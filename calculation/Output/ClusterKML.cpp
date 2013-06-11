//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ClusterKML.h"
#include "SaTScanData.h"
#include <fstream>
#include "UtilityFunctions.h"
#include "RandomNumberGenerator.h"
#include "ZipUtils.h"
#include "GisUtils.h"

const char * ClusterKML::KMZ_FILE_EXT = ".kmz";
const char * ClusterKML::KML_FILE_EXT = ".kml";
double ClusterKML::_minRatioToReport=0.001;

/** constructor */
ClusterKML::ClusterKML(const CSaTScanData& dataHub, const MostLikelyClustersContainer& clusters, const SimulationVariables& simVars) 
           :_dataHub(dataHub), _clusters(clusters), _simVars(simVars), _visibleLocations(false) {
    if (_dataHub.GetParameters().getIncludeLocationsKML()) {
        // Calculate the number of locations that will be reported. If there are many, we'll make them to NetworkLink links.
        tract_t tNumClustersToDisplay(_simVars.get_sim_count() == 0 ? std::min(10, _clusters.GetNumClustersRetained()) : _clusters.GetNumClustersRetained());
        unsigned int numLocationsAllClusters=0;
        //first iterate through all location coordinates to determine largest X and Y
        for (int i=0; i < _clusters.GetNumClustersRetained(); ++i) {
            const CCluster& cluster = _clusters.GetCluster(i);
            if (cluster.GetClusterType() == PURELYTEMPORALCLUSTER)
                continue;
            if (!(i == 0 || (i < tNumClustersToDisplay && cluster.m_nRatio >= _minRatioToReport && (_simVars.get_sim_count() == 0 || cluster.GetRank() <= _simVars.get_sim_count()))))
                break;
            if (cluster.m_nRatio >= _minRatioToReport) {
                numLocationsAllClusters += static_cast<unsigned int>(_clusters.GetCluster(i).GetNumTractsInCluster());
            }
        }
        _separateLocationsKML = numLocationsAllClusters > _dataHub.GetParameters().getLocationsThresholdKML();
    }
}

/** Creates the primary KML file and returns filename. */
ClusterKML::file_collection_t& ClusterKML::createKMLFiles(file_collection_t& fileCollection) const {
    std::string buffer, color,legend;

    try {
        fileCollection.resize(fileCollection.size() + 1);
        fileCollection.back().setFullPath(_dataHub.GetParameters().GetOutputFileName().c_str());
        fileCollection.back().setExtension(KML_FILE_EXT);

        std::ofstream KMLout;
        //open output file
        KMLout.open(fileCollection.back().getFullPath(buffer).c_str());
        if (!KMLout) throw resolvable_error("Error: Could not create file '%s'.\n", fileCollection.back().getFullPath(buffer).c_str());

        KMLout << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
        KMLout << "<kml xmlns=\"http://www.opengis.net/kml/2.2\">" << std::endl << "<Document>" << std::endl << std::endl;
        KMLout << "\t<Style id=\"high-rate-placemark\"><IconStyle><Icon><href>https://maps.google.com/mapfiles/kml/shapes/placemark_circle.png</href><scale>0.25</scale></Icon></IconStyle></Style>" << std::endl;
        KMLout << "\t<Style id=\"low-rate-placemark\"><IconStyle><Icon><href>https://maps.google.com/mapfiles/kml/shapes/placemark_circle.png</href><scale>0.25</scale></Icon></IconStyle></Style>" << std::endl;
        KMLout << std::endl << "\t<name>SaTScan Clusters Detected</name>" << std::endl << std::endl;
        //if  no replications requested, attempt to display up to top 10 clusters
        tract_t tNumClustersToDisplay(_simVars.get_sim_count() == 0 ? std::min(10, _clusters.GetNumClustersRetained()) : _clusters.GetNumClustersRetained());
        //first iterate through all location coordinates to determine largest X and Y
        for (int i=0; i < _clusters.GetNumClustersRetained(); ++i) {
            //get reference to i'th top cluster
            const CCluster& cluster = _clusters.GetCluster(i);
            //skip purely temporal clusters
            if (cluster.GetClusterType() == PURELYTEMPORALCLUSTER)
                continue;
            if (!(i == 0 || (i < tNumClustersToDisplay && cluster.m_nRatio >= _minRatioToReport && (_simVars.get_sim_count() == 0 || cluster.GetRank() <= _simVars.get_sim_count()))))
                break;
            //write cluster details to 'cluster information' file
            if (cluster.m_nRatio >= _minRatioToReport) {
                writeCluster(fileCollection, KMLout, cluster, i);
            }
        }
        KMLout << "</Document>" << std::endl << "</kml>" << std::endl;
        KMLout.close();
    } catch (prg_exception& x) {
        x.addTrace("createKMLFiles()","ClusterKML");
        throw;
    }
    return fileCollection;
}

/** Render scatter chart to html page. */
void ClusterKML::generateKML() {
    file_collection_t kmlFiles;

    try {
        createKMLFiles(kmlFiles);
        if (_dataHub.GetParameters().getCompressClusterKML()) {
            FileName kmzFile;
            std::string kmz, kml;
            kmzFile.setFullPath(_dataHub.GetParameters().GetOutputFileName().c_str());
            kmzFile.setExtension(KMZ_FILE_EXT);
            kmzFile.getFullPath(kmz);
            for (file_collection_t::const_iterator itr=kmlFiles.begin(); itr != kmlFiles.end(); ++itr) {
                addZip(kmz, itr->getFullPath(kml), itr!=kmlFiles.begin());
                remove(kml.c_str());
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("generateKML()","ClusterKML");
        throw;
    }
}

void ClusterKML::writeCluster(file_collection_t& fileCollection, std::ofstream& outKML, const CCluster& cluster, int iCluster) const {
    std::string                                legend, locations, buffer, buffer2;
    std::vector<double>                        vCoordinates;
    std::pair<double, double>                  prLatitudeLongitude;
    TractHandler::Location::StringContainer_t  vTractIdentifiers;
    const double radius_km = cluster.GetLatLongRadius();
    bool isHighRate = cluster.getAreaRateForCluster(_dataHub) == HIGH;

    try {
        outKML << getClusterStyleTags(cluster, iCluster, buffer, isHighRate).c_str() << std::endl; 
        outKML << "\t<Placemark>" << std::endl; 
        outKML << "\t\t<name>" << (iCluster + 1) << "</name>" << std::endl;
        outKML << "\t\t<snippet>SaTScan Cluster #" << (iCluster + 1) << "</snippet>" << std::endl;
        // set popup window text

        outKML << "\t\t<TimeSpan><begin>" << cluster.GetStartDate(buffer, _dataHub, "-") << "T00:00:00Z</begin><end>" << cluster.GetEndDate(buffer2, _dataHub, "-") << "T23:59:59Z</end></TimeSpan>" << std::endl;
        outKML << "\t\t<styleUrl>#cluster-" << (iCluster + 1) << "-stylemap</styleUrl>" << std::endl;
        outKML << "\t\t" << getClusterExtendedData(cluster, iCluster, buffer).c_str() << std::endl;
        outKML << "\t\t<MultiGeometry>" << std::endl;

        GisUtils::pointpair_t clusterSegment = GisUtils::getClusterRadiusSegmentPoints(_dataHub, cluster);
        // create boundary circle placemark
        outKML << "\t\t\t<Polygon><outerBoundaryIs><LinearRing><extrude>1</extrude><tessellate>1</tessellate><coordinates>";
        // calculate the points of a linear ring around the cluster and write them to kml file
        GisUtils::points_t circlePoints = GisUtils::getPointsOnCircleCircumference(clusterSegment.first, clusterSegment.second);
        for (GisUtils::points_t::const_iterator itr=circlePoints.begin(); itr != circlePoints.end(); ++itr) {
            outKML << itr->first << "," << itr->second << ",500 ";
        }
        outKML << "</coordinates></LinearRing></outerBoundaryIs></Polygon>" << std::endl;
        // create centroid placemark
        prLatitudeLongitude = clusterSegment.first;
        outKML << "\t\t\t<Point><extrude>1</extrude><altitudeMode>relativeToGround</altitudeMode><coordinates>" << prLatitudeLongitude.second << "," << prLatitudeLongitude.first << ",0" << "</coordinates></Point>" << std::endl;
        outKML << "\t\t</MultiGeometry>" << std::endl << "\t</Placemark>" << std::endl; 
        // add cluster locations if requested
        if (_dataHub.GetParameters().getIncludeLocationsKML()) {
            std::stringstream  clusterPlacemarks;
            // create locations folder and locations within cluster placemarks
            for (tract_t t=1; t <= cluster.GetNumTractsInCluster(); ++t) {
                tract_t tTract = _dataHub.GetNeighbor(cluster.GetEllipseOffset(), cluster.GetCentroidIndex(), t, cluster.GetCartesianRadius());
                if (!_dataHub.GetIsNullifiedLocation(tTract)) {
                    _dataHub.GetTInfo()->retrieveAllIdentifiers(tTract, vTractIdentifiers);
                    CentroidNeighborCalculator::getTractCoordinates(_dataHub, cluster, tTract,vCoordinates);
                    prLatitudeLongitude = ConvertToLatLong(vCoordinates);
                    clusterPlacemarks << "\t\t<Placemark><name>" <<  vTractIdentifiers[0] << "</name>" << (_visibleLocations ? "" : "<visibility>0</visibility>") 
                                      << "<description></description><styleUrl>";
                    if (_separateLocationsKML) {
                        // If creating separate KML files for locations, styles of primary kml need to be qualified in sub-kml files.
                        clusterPlacemarks << fileCollection.front().getFileName() << fileCollection.front().getExtension();
                    }
                    clusterPlacemarks << "#" << (isHighRate ? "high" : "low") << "-rate-placemark</styleUrl>"
                                      << "<Point><coordinates>" << prLatitudeLongitude.second << "," << prLatitudeLongitude.first << ",0"
                                      << "</coordinates></Point></Placemark>" << std::endl;
                }
            }
            if (clusterPlacemarks.str().size()) {
                if (_separateLocationsKML) {
                    // Create separate kml for this clusters locations, then reference in primary cluster.
                    fileCollection.resize(fileCollection.size() + 1);
                    fileCollection.back().setFullPath(_dataHub.GetParameters().GetOutputFileName().c_str());
                    printString(buffer, "cluster%u_locations", (iCluster + 1));
                    fileCollection.back().setFileName(buffer.c_str());
                    fileCollection.back().setExtension(KML_FILE_EXT);

                    std::ofstream clusterKML;
                    clusterKML.open(fileCollection.back().getFullPath(buffer).c_str());
                    if (!clusterKML) throw resolvable_error("Error: Could not create file '%s'.\n", fileCollection.back().getFullPath(buffer).c_str());
                    clusterKML << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
                    clusterKML << "<kml xmlns=\"http://www.opengis.net/kml/2.2\">" << std::endl << "<Document>" << std::endl << std::endl;
                    clusterKML << "<name>Cluster " << (iCluster + 1) << " Locations</name>" << std::endl << clusterPlacemarks.str() << "</Document>" << std::endl << "</kml>" << std::endl;
                    clusterKML.close();
                    // Now reference this kml file in NetworkLink tag of primary kml.
                    outKML << "\t<NetworkLink><name>Cluster " << (iCluster + 1) << " Locations</name><visibility>0</visibility><refreshVisibility>0</refreshVisibility><Link><href>"
                           << "cluster" << (iCluster + 1) << "_locations" << KML_FILE_EXT << "</href></Link></NetworkLink>" << std::endl << std::endl;
                } else { // Insert locations into primary kml.
                    outKML << "\t<Folder><name>Cluster " << (iCluster + 1) << " Locations</name><description></description>" << std::endl << clusterPlacemarks.str() << "\t</Folder>"<< std::endl << std::endl;
                }
            }
        } else {
            outKML << std::endl; 
        }

    } catch (prg_exception& x) {
        x.addTrace("writeCluster()","ClusterKML");
        throw;
    }
}

/** Returns style and stylemap tags for cluster. */
std::string & ClusterKML::getClusterStyleTags(const CCluster& cluster, int iCluster, std::string& styleString, bool isHighRate) const {
    std::stringstream  lines;
    std::string buffer;

    lines << "\t<Style id=\"cluster-" << (iCluster + 1) << "-style\"><IconStyle><Icon></Icon></IconStyle><LabelStyle><scale>1.0</scale></LabelStyle>";
    lines << "<LineStyle><color>" << (isHighRate ? "ff0000aa" : "ffff0000") << "</color></LineStyle><PolyStyle><color>" << (isHighRate ? "400000aa" : "40ff0000") << "</color></PolyStyle>";
    lines << "<BalloonStyle><text>" << getClusterBalloonTemplate(cluster, buffer).c_str() << "</text></BalloonStyle></Style>"<< std::endl;
    lines << "\t<StyleMap id=\"cluster-" << (iCluster + 1) << "-stylemap\"><Pair><key>normal</key><styleUrl>#cluster-" << (iCluster + 1) << "-style</styleUrl></Pair><Pair><key>highlight</key><styleUrl>#cluster-" << (iCluster + 1) << "-style</styleUrl></Pair></StyleMap>";
    styleString = lines.str();
    return styleString;
}

/* Returns cluster balloon template. */
std::string & ClusterKML::getClusterBalloonTemplate(const CCluster& cluster, std::string& templateString) const {
    std::string buffer;
    std::stringstream  templateLines;
    const CParameters& parameters = _dataHub.GetParameters();
    const char * rowFormat = "<tr><th style=\"text-align:left;white-space:nowrap;padding-right:5px;\">%s</th><td style=\"white-space:nowrap;\">$[%s]</td></tr>";

    templateLines << "<![CDATA[<b>$[snippet]</b><br/><table border=\"0\">";
    CCluster::ReportCache_t::const_iterator itr=cluster.getReportLinesCache().begin(), itr_end=cluster.getReportLinesCache().end();
    for (; itr != itr_end; ++itr) {
        templateLines << printString(buffer, rowFormat, itr->first.c_str(), itr->first.c_str()).c_str();
    }
    templateLines << "</table>]]>";
    templateString = templateLines.str();
    return templateString;
}

/** XML encodes string */
std::string& ClusterKML::encode(const std::string& data, std::string& buffer) const {
    using boost::algorithm::replace_all;
    buffer = data;
    replace_all(buffer, "&",  "&amp;");
    replace_all(buffer, "\"", "&quot;");
    replace_all(buffer, "\'", "&apos;");
    replace_all(buffer, "<",  "&lt;");
    replace_all(buffer, ">",  "&gt;");
    return buffer;
}

/** Returns ExtendedData tags for this cluster. */
std::string & ClusterKML::getClusterExtendedData(const CCluster& cluster, int iCluster, std::string& buffer) const {
    std::stringstream lines;
    CCluster::ReportCache_t::const_iterator itr=cluster.getReportLinesCache().begin(), itr_end=cluster.getReportLinesCache().end();
    lines << "<ExtendedData>";
    for (; itr != itr_end; ++itr) {
        lines << "<Data name=\"" << itr->first.c_str() << "\"><value>" << encode(itr->second, buffer).c_str() << "</value></Data>";
    }
    lines << "</ExtendedData>";
    buffer = lines.str();
    return buffer;
}

/** Return legend of cluster information to be used as popup in html page. */
std::string & ClusterKML::getClusterLegend(const CCluster& cluster, int iCluster, std::string& legend) const {
    std::stringstream  lines;
    CCluster::ReportCache_t::const_iterator itr=cluster.getReportLinesCache().begin(), itr_end=cluster.getReportLinesCache().end();

    lines << "<![CDATA[" << std::endl << "<table style=\"font-size:12px;\">";
    for (; itr != itr_end; ++itr) {
        lines << "<tr><th style=\"text-align:left;white-space:nowrap;padding-right:5px;\">" << itr->first << "</th><td style=\"white-space:nowrap;\">" << itr->second << "</td></tr>";
    }
    lines << "</table>" << std::endl << "]]>";
    legend = lines.str();
    std::replace(legend.begin(), legend.end(), '\n', ' ');
    return legend;
}

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
#include "DataDemographics.h"
#include "DataSource.h"
#include "DateStringParser.h"
#include "Toolkit.h"
#include <boost/dynamic_bitset.hpp>
#include <boost/algorithm/string.hpp>

/////////////////////////////////// BaseClusterKML ////////////////////////////////////////

const char * BaseClusterKML::KMZ_FILE_EXT = ".kmz";
const char * BaseClusterKML::KML_FILE_EXT = ".kml";
double BaseClusterKML::_minRatioToReport=0.001;

BaseClusterKML::BaseClusterKML(const CSaTScanData& dataHub) : _dataHub(dataHub), _visibleLocations(false) {
    _cluster_locations.resize(_dataHub.getLocationsManager().locations().size());
    _separateLocationsKML = _dataHub.getLocationsManager().locations().size() > _dataHub.GetParameters().getLocationsThresholdKML();
}

void BaseClusterKML::createKMZ(const file_collection_t& fileCollection, bool removefiles) {
    try {
        if (_dataHub.GetParameters().getCompressClusterKML()) {
            FileName kmzFile;
            std::string kmz, kml;
            kmzFile.setFullPath(_dataHub.GetParameters().GetOutputFileName().c_str());
            kmzFile.setExtension(KMZ_FILE_EXT);
            kmzFile.getFullPath(kmz);
            for (file_collection_t::const_iterator itr = fileCollection.begin(); itr != fileCollection.end(); ++itr) {
                addZip(kmz, itr->getFullPath(kml), itr != fileCollection.begin());
                if (removefiles)
                    remove(kml.c_str());
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("createKMZ()", "BaseClusterKML");
        throw;
    }
}

/* Returns html color as KML color. */
std::string BaseClusterKML::toKmlColor(const std::string& htmlColor, const std::string& alpha) const {
    if (htmlColor.size() < 6) throw prg_error("Can not convert html color %s to kml color.", htmlColor.c_str());
    unsigned int offset(htmlColor[0] == '#' ? 1 : 0);
    std::stringstream s;
    s << alpha << htmlColor.substr(4 + offset, 2) << htmlColor.substr(2 + offset, 2) << htmlColor.substr(0 + offset, 2);
    return s.str();
}

void BaseClusterKML::writeCluster(file_collection_t& fileCollection, std::ofstream& outKML, const CCluster& cluster, int iCluster, const SimulationVariables& simVars) const {
    std::string                                legend, locations, buffer, buffer2;
    std::vector<double>                        vCoordinates;
    std::pair<double, double>                  prLatitudeLongitude;
	Identifier::CombinedIdentifierNames_t  vTractIdentifiers;
    bool isHighRate = cluster.getAreaRateForCluster(_dataHub) == HIGH;

    try {
        outKML << getClusterStyleTags(cluster, iCluster, buffer, isHighRate).c_str() << std::endl;
        outKML << "\t<Placemark>" << std::endl;
        if (_dataHub.GetParameters().getClusterMonikerPrefix().size()) {
            printString(buffer2, " (%sC%u)", _dataHub.GetParameters().getClusterMonikerPrefix().c_str(), (iCluster + 1));
        }
        outKML << "\t\t<name>#" << (iCluster + 1) << buffer2.c_str() << "</name>" << std::endl;
        outKML << "\t\t<snippet>Cluster #" << (iCluster + 1) << buffer2.c_str() << "</snippet>" << std::endl;
        outKML << "\t\t<visibility>" << (iCluster == 0 ? "1" : "0") << "</visibility>" << std::endl;
        //outKML << "\t\t<TimeSpan><begin>" << cluster.GetStartDate(buffer, _dataHub, "-") << "T00:00:00Z</begin><end>" << cluster.GetEndDate(buffer2, _dataHub, "-") << "T23:59:59Z</end></TimeSpan>" << std::endl;
        outKML << "\t\t<styleUrl>#cluster-" << (iCluster + 1) << "-stylemap</styleUrl>" << std::endl;
        outKML << "\t\t" << getClusterExtendedData(cluster, iCluster, buffer).c_str() << std::endl;
        outKML << "\t\t<MultiGeometry>" << std::endl;

        GisUtils::pointpair_t clusterSegment = GisUtils::getClusterRadiusSegmentPoints(_dataHub, cluster);
        // create boundary circle placemark
        outKML << "\t\t\t<Polygon><outerBoundaryIs><LinearRing><extrude>1</extrude><tessellate>1</tessellate><coordinates>";
        // calculate the points of a linear ring around the cluster and write them to kml file
        GisUtils::points_t circlePoints = GisUtils::getPointsOnCircleCircumference(clusterSegment.first, clusterSegment.second);
        for (GisUtils::points_t::const_iterator itr = circlePoints.begin(); itr != circlePoints.end(); ++itr) {
            outKML << itr->first << "," << itr->second << ",500 ";
        }
        outKML << "</coordinates></LinearRing></outerBoundaryIs></Polygon>" << std::endl;
        // create centroid placemark
        prLatitudeLongitude = clusterSegment.first;
        outKML << "\t\t\t<Point><extrude>1</extrude><altitudeMode>relativeToGround</altitudeMode><coordinates>" << prLatitudeLongitude.second << "," << prLatitudeLongitude.first << ",0" << "</coordinates></Point>" << std::endl;
        outKML << "\t\t</MultiGeometry>" << std::endl << "\t</Placemark>" << std::endl;

        // When using a network file, we only draw a small circle around central location then drawn connections/edges between locations in cluster.
        NetworkLocationContainer_t networkLocations;
        if (_dataHub.GetParameters().getUseLocationsNetworkFile()) {
            _dataHub.getClusterNetworkLocations(cluster, networkLocations);
            Network::Connection_Details_t connections = GisUtils::getClusterConnections(networkLocations);
            outKML << "\t\t<Folder><name>Cluster " << (iCluster + 1) << " Edges</name>";
            for (auto itr = connections.begin(); itr != connections.end(); ++itr) {
                itr->get<0>()->coordinates()->retrieve(vCoordinates);
                std::pair<double, double> prLatitudeLongitude(ConvertToLatLong(vCoordinates));
                outKML << "\t\t\t<Placemark><styleUrl>#" << (isHighRate ? "high" : "low") << "-line-edge</styleUrl><LineString><coordinates>";
                outKML << prLatitudeLongitude.second << "," << prLatitudeLongitude.first << ",0  ";
                itr->get<1>()->coordinates()->retrieve(vCoordinates);
                prLatitudeLongitude = ConvertToLatLong(vCoordinates);
                outKML << prLatitudeLongitude.second << "," << prLatitudeLongitude.first << ",0</coordinates></LineString></Placemark>" << std::endl;
            }
            outKML << "\t\t</Folder>" << std::endl;
        }

        // add cluster locations if requested
        if (_dataHub.GetParameters().getIncludeLocationsKML()) {
            std::stringstream  clusterPlacemarks;
            // create locations folder and locations within cluster placemarks
            const auto& locations = _dataHub.getLocationsManager().locations();
            boost::dynamic_bitset<> clusterLocations;
            CentroidNeighborCalculator::getLocationsAboutCluster(_dataHub, cluster, &clusterLocations);
            size_t index = clusterLocations.find_first();
            while (index != clusterLocations.npos) {
                locations[index].get()->coordinates()->retrieve(vCoordinates);
                _cluster_locations.set(index);
                std::pair<double, double> prLatitudeLongitude(ConvertToLatLong(vCoordinates));
                clusterPlacemarks << "\t\t<Placemark><name></name>" << (_visibleLocations ? "" : "<visibility>0</visibility>") << "<description></description><styleUrl>";
                if (_separateLocationsKML) {
                    // If creating separate KML files for locations, styles of primary kml need to be qualified in sub-kml files.
                    clusterPlacemarks << fileCollection.front().getFileName() << fileCollection.front().getExtension();
                }
                clusterPlacemarks << "#" << (isHighRate ? "high" : "low") << "-rate-placemark</styleUrl>"
                    << "<Point><coordinates>" << prLatitudeLongitude.second << "," << prLatitudeLongitude.first << ",0"
                    << "</coordinates></Point></Placemark>" << std::endl;
                index = clusterLocations.find_next(index);
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
                    clusterKML << "<kml xmlns=\"http://www.opengis.net/kml/2.2\" xmlns:gx=\"http://www.google.com/kml/ext/2.2\">" << std::endl << "<Document>" << std::endl << std::endl;
                    clusterKML << "<name>Cluster " << (iCluster + 1) << " Locations</name>" << std::endl << clusterPlacemarks.str() << "</Document>" << std::endl << "</kml>" << std::endl;
                    clusterKML.close();
                    // Now reference this kml file in NetworkLink tag of primary kml.
                    outKML << "\t<NetworkLink><name>Cluster " << (iCluster + 1) << " Locations</name><visibility>0</visibility><refreshVisibility>0</refreshVisibility><Link><href>"
                        << "cluster" << (iCluster + 1) << "_locations" << KML_FILE_EXT << "</href></Link></NetworkLink>" << std::endl << std::endl;
                }
                else { // Insert locations into primary kml.
                    outKML << "\t<Folder><name>Cluster " << (iCluster + 1) << " Locations</name><description></description>" << std::endl << clusterPlacemarks.str() << "\t</Folder>" << std::endl << std::endl;
                }
            }
        } else {
            outKML << std::endl;
        }

    } catch (prg_exception& x) {
        x.addTrace("writeCluster()", "BaseClusterKML");
        throw;
    }
}

/* Returns the kml color - including opacity. Full opacity is 100% otherwise 25%.  The label and colors change by drilldown index. */
std::string& BaseClusterKML::getStyleColor(bool isHighRate, bool fullOpacity, std::string& buffer) const {
    std::stringstream styleColor;
    styleColor << (fullOpacity ? "ff" : "40");
    if (!_dataHub.isDrilldown())
        styleColor  << (isHighRate ? "0000aa" : "ff0000");
    else if (_dataHub.getDrilldownLevel() == 1)
        styleColor << (isHighRate ? "00aaff" : "005500");
    else if (_dataHub.getDrilldownLevel() == 2) 
        styleColor << (isHighRate ? "00ffff" : "000055");
    else if (_dataHub.getDrilldownLevel() == 3)
        styleColor << (isHighRate ? "147ab2" : "005555");
    else
        styleColor << (isHighRate ? "1232b0" : "555555");
    buffer = styleColor.str();
    return buffer;
}

std::string BaseClusterKML::toHex(const std::string& data) const {
    std::stringstream ss;
    for (const auto &item : data) {
        ss << std::hex << int(item);
    }
    return ss.str();
}

/** Returns style and stylemap tags for cluster. */
std::string & BaseClusterKML::getClusterStyleTags(const CCluster& cluster, int iCluster, std::string& styleString, bool isHighRate) const {
    std::stringstream lines;
    std::string buffer;

    // Calculate the label scale -- decreasing as we drilldown but no lower than 0.7 -- empirically determined to be best minimum.
    double labelScale = std::max(_dataHub.isDrilldown() ? (1.0 - (static_cast<double>(_dataHub.getDrilldownLevel()) * 0.1)) : 1.0, 0.7);
    // Calculate the line width -- increasing as we drilldown but no greater than 5 -- empirically determined to be best maximum.
    unsigned int lineWidth = std::min(_dataHub.isDrilldown() ? _dataHub.getDrilldownLevel() + 2 : (unsigned int)1, (unsigned int)5);
    lines << std::setprecision(1) << "\t<Style id=\"cluster-" << (iCluster + 1) << "-style\"><IconStyle><Icon></Icon></IconStyle><LabelStyle><scale>" << labelScale << "</scale></LabelStyle>";
    lines << "<LineStyle><color>" << getStyleColor(isHighRate, true, buffer) << "</color><width>" << lineWidth << "</width></LineStyle>";
    lines << "<PolyStyle><color>" << getStyleColor(isHighRate, false, buffer) << "</color></PolyStyle>";
    lines << "<BalloonStyle><text>" << getClusterBalloonTemplate(cluster, buffer).c_str() << "</text></BalloonStyle></Style>" << std::endl;
    lines << "\t<StyleMap id=\"cluster-" << (iCluster + 1) << "-stylemap\"><Pair><key>normal</key><styleUrl>#cluster-" << (iCluster + 1) << "-style</styleUrl></Pair><Pair><key>highlight</key><styleUrl>#cluster-" << (iCluster + 1) << "-style</styleUrl></Pair></StyleMap>";
    styleString = lines.str();
    return styleString;
}

/* Returns cluster balloon template. */
std::string & BaseClusterKML::getClusterBalloonTemplate(const CCluster& cluster, std::string& templateString) const {
    std::string buffer, buffer2, buffer3, buffer4;
    std::stringstream  templateLines;
    const CParameters& parameters = _dataHub.GetParameters();
    unsigned int currSetIdx = std::numeric_limits<unsigned int>::max(), numFilesSets = parameters.getNumFileSets();

    templateLines << "<![CDATA[<u><b>$[snippet]</b></u><br/>";
    if (numFilesSets == 1) {
        templateLines << "<table border=\"0\" style=\"width:100%;\">";
        for (auto itr=cluster.getReportLinesCache().begin(); itr != cluster.getReportLinesCache().end(); ++itr) {
            templateLines << printString(buffer,
                "<tr><th style=\"text-align:left;white-space:nowrap;padding-right:5px;\">%s:</th><td style=\"white-space:nowrap;text-align:left;\">$[%s]</td></tr>",
                encode(itr->first, buffer2).c_str(), encode(itr->first, buffer3).c_str()
            );
        }
        templateLines << "</table>";
    } else {
        std::stringstream clusterLines, clusterDataSetLines;
        clusterLines << "<table border=\"0\" style=\"width:100%;\">";
        for (auto itr=cluster.getReportLinesCache().begin(); itr != cluster.getReportLinesCache().end(); ++itr) {
            if (itr->second.second == std::numeric_limits<unsigned int>::max()) { // cluster level
                clusterLines << printString(buffer,
                    "<tr><th style=\"text-align:left;white-space:nowrap;padding-right:5px;color:#333;font-weight:400;\">%s</th><td style=\"white-space:nowrap;text-align:right;\">$[%s]</td></tr>",
                    encode(itr->first, buffer2).c_str(), encode(itr->first, buffer3).c_str()
                );
            } else { // cluster data set level
                if (currSetIdx != itr->second.second) {
                    if (currSetIdx != std::numeric_limits<unsigned int>::max()) clusterDataSetLines << "</table>";
                    clusterDataSetLines << "<table border=\"0\" style=\"width:100%;\">";
                    clusterDataSetLines << "<caption style=\"text-align:left;white-space:nowrap;padding:2px 0 2px 0;text-decoration:underline;font-weight:bold;color:#555;\">";
                    clusterDataSetLines << getWrappedText(
                        encode(printString(buffer4, "%s (set %i)", parameters.getDataSourceNames()[
                        _dataHub.GetDataSetHandler().getDataSetRelativeIndex(itr->second.second)].c_str(), (itr->second.second + 1)), buffer2), 0, 40, "<br>", buffer3
                    );
                    clusterDataSetLines << "</caption>";
                    currSetIdx = itr->second.second;
                }
                clusterDataSetLines << printString(buffer,
                    "<tr><th style=\"text-align:left;white-space:nowrap;padding-right:5px;color:#333;font-weight:400;\">%s:</th><td style=\"white-space:nowrap;text-align:right;\">$[%s%s]</td></tr>",
                    encode(itr->first, buffer2).c_str(), encode(itr->first, buffer3).c_str(), printString(buffer4, " set%u", itr->second.second).c_str()
                );
            }
        }
        templateLines << clusterLines.str() << "</table>" << clusterDataSetLines.str() << "</table>";
    }
    templateLines << "]]>";
    templateString = templateLines.str();
    return templateString;
}

/** XML encodes string */
std::string& BaseClusterKML::encode(const std::string& data, std::string& buffer) const {
    using boost::algorithm::replace_all;
    buffer = data;
    replace_all(buffer, "&", "&amp;");
    replace_all(buffer, "\"", "&quot;");
    replace_all(buffer, "\'", "&apos;");
    replace_all(buffer, "<", "&lt;");
    replace_all(buffer, ">", "&gt;");
    return buffer;
}

/** Returns ExtendedData tags for this cluster. */
std::string & BaseClusterKML::getClusterExtendedData(const CCluster& cluster, int iCluster, std::string& buffer) const {
    const CParameters& parameters = _dataHub.GetParameters();
    std::stringstream lines;
    CCluster::ReportCache_t::const_iterator itr = cluster.getReportLinesCache().begin(), itr_end = cluster.getReportLinesCache().end();
    std::string bufferSetIdx;
    unsigned int numDataSets = parameters.getNumFileSets();

    lines << "<ExtendedData>";
    for (; itr != itr_end; ++itr) {
        lines << "<Data name=\"" << itr->first.c_str()
            << (numDataSets > 1 && itr->second.second != std::numeric_limits<unsigned int>::max() ? printString(bufferSetIdx, " set%u", itr->second.second).c_str() : "")
            << "\"><value>" << encode(itr->second.first, buffer).c_str() << "</value></Data>";
    }
    lines << "</ExtendedData>";
    buffer = lines.str();
    return buffer;
}

/** Write the opening block to the KML file. */
void BaseClusterKML::writeOpenBlockKML(std::ofstream& outKML) const {
    std::string buffer;
    outKML << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
    outKML << "<kml xmlns=\"http://www.opengis.net/kml/2.2\">" << std::endl << "<Document>" << std::endl << std::endl;
    outKML << "\t<Style id=\"high-rate-placemark\"><IconStyle><color>" << getStyleColor(true, true, buffer) << "</color><Icon><href>https://maps.google.com/mapfiles/kml/shapes/placemark_circle.png</href><scale>0.25</scale></Icon></IconStyle></Style>" << std::endl;
    outKML << "\t<Style id=\"low-rate-placemark\"><IconStyle><color>" << getStyleColor(false, true, buffer) << "</color><Icon><href>https://maps.google.com/mapfiles/kml/shapes/placemark_circle.png</href><scale>0.25</scale></Icon></IconStyle></Style>" << std::endl;
    outKML << "\t<Style id=\"location-placemark\"><IconStyle><color>ff019399</color><Icon><href>https://maps.google.com/mapfiles/kml/shapes/placemark_circle.png</href><scale>0.25</scale></Icon></IconStyle></Style>" << std::endl;
    outKML << "\t<StyleMap id=\"high-line-edge\"><Pair><key>normal</key><Style><LineStyle><color>" << getStyleColor(true, true, buffer) << "</color><width>3</width><scale>1.0</scale></LineStyle></Style></Pair><Pair><key>highlight</key><Style id=\"line-edge1\"><LineStyle><color>" << getStyleColor(true, false, buffer) << "</color><width>3</width><scale>1.0</scale></LineStyle></Style></Pair></StyleMap>" << std::endl;
    outKML << "\t<StyleMap id=\"low-line-edge\"><Pair><key>normal</key><Style><LineStyle><color>" << getStyleColor(false, true, buffer) << "</color><width>3</width><scale>1.0</scale></LineStyle></Style></Pair><Pair><key>highlight</key><Style id=\"line-edge2\"><LineStyle><color>" << getStyleColor(false, false, buffer) << "</color><width>3</width><scale>1.0</scale></LineStyle></Style></Pair></StyleMap>" << std::endl;
    outKML << "\t<StyleMap id=\"line-edge\"><Pair><key>normal</key><Style><LineStyle><color>" << toKmlColor("#F39C12", "ff") << "</color><width>1.5</width><scale>1.0</scale></LineStyle></Style></Pair><Pair><key>highlight</key><Style id=\"line-edge3\"><LineStyle><color>" << toKmlColor("#F39C12", "ff") << "</color><width>1.5</width><scale>1.0</scale></LineStyle></Style></Pair></StyleMap>" << std::endl;

    FileName filename(_dataHub.GetParameters().GetOutputFileName().c_str());
    outKML << std::endl << "\t<name>SaTScan: " << filename.getFileName() << "</name>" << std::endl << std::endl;
}

/** Adds clusters of MostLikelyClustersContainer collection to KML file(s). Returns the number of caddClusterslusters written. */
unsigned int BaseClusterKML::addClusters(const MostLikelyClustersContainer& clusters, const SimulationVariables& simVars, std::ofstream& outKML, file_collection_t& fileCollection, unsigned int clusterOffset) {
    unsigned int writtenClusters = 0;
    //if  no replications requested, attempt to display up to top 10 clusters
    tract_t tNumClustersToDisplay(simVars.get_sim_count() == 0 ? std::min(10, clusters.GetNumClustersRetained()) : clusters.GetNumClustersRetained());
    for (int i = 0; i < clusters.GetNumClustersRetained(); ++i) {
        //get reference to i'th top cluster
        const CCluster& cluster = clusters.GetCluster(i);
        //skip purely temporal clusters
        if (cluster.GetClusterType() == PURELYTEMPORALCLUSTER)
            continue;
        if (!(i == 0 || (i < tNumClustersToDisplay && cluster.m_nRatio >= _minRatioToReport && (simVars.get_sim_count() == 0 || cluster.GetRank() <= simVars.get_sim_count()))))
            break;
        //write cluster details to 'cluster information' file
        if (cluster.m_nRatio >= _minRatioToReport) {
            writeCluster(fileCollection, outKML, cluster, i + clusterOffset, simVars);
            ++writtenClusters;
        }
    }
    return writtenClusters;
}

/////////////////////// ClusterKML //////////////////////////////

ClusterKML::ClusterKML(const CSaTScanData& dataHub) : BaseClusterKML(dataHub), _clusters_written(0), _locations_written(0) {
    _kml_files.resize(_kml_files.size() + 1);
    _kml_files.back().setFullPath(_dataHub.GetParameters().GetOutputFileName().c_str());
    _kml_files.back().setExtension(KML_FILE_EXT);

    std::string buffer;
    //open output file
    _kml_out.open(_kml_files.back().getFullPath(buffer).c_str());
    if (!_kml_out) throw resolvable_error("Error: Could not create file '%s'.\n", _kml_files.back().getFullPath(buffer).c_str());
    writeOpenBlockKML(_kml_out);
}

void ClusterKML::add(const MostLikelyClustersContainer& clusters, const SimulationVariables& simVars) {
    if (_dataHub.GetParameters().getIncludeLocationsKML()) {
        // Calculate the number of locations that will be reported. If there are many, we'll make them to NetworkLink links.
        tract_t tNumClustersToDisplay(simVars.get_sim_count() == 0 ? std::min(10, clusters.GetNumClustersRetained()) : clusters.GetNumClustersRetained());
        for (int i = 0; i < clusters.GetNumClustersRetained(); ++i) {
            const CCluster& cluster = clusters.GetCluster(i);
            if (cluster.GetClusterType() == PURELYTEMPORALCLUSTER)
                continue;
            if (!(i == 0 || (i < tNumClustersToDisplay && cluster.m_nRatio >= _minRatioToReport && (simVars.get_sim_count() == 0 || cluster.GetRank() <= simVars.get_sim_count()))))
                break;
            if (cluster.m_nRatio >= _minRatioToReport)
                _locations_written += static_cast<unsigned int>(clusters.GetCluster(i).getNumIdentifiers());
        }
    }
    _clusters_written += addClusters(clusters, simVars, _kml_out, _kml_files, _clusters_written);
}

/* Adds individual level data to the Google Earth file. */
void ClusterKML::add(const DataDemographicsProcessor& demographics) {
    // First test whether any of the data sets include indivdual and descriptive coordinates.
    if (!demographics.hasIndividualGeographically()) {
        _dataHub.GetPrintDirection().Printf(
            "Descriptive coordinates were not found in line list data. Line list individuals will not be added to Google Earth KML file.\n", BasePrint::P_WARNING
        );
        return;
    }
    // Create collection of event types that we'll be grouping the individuals into.
    // The event types don't have to be matching between data sets - they can match, partially match or not match at all.
    std::vector<std::string> event_types;
    for (size_t idx = 0; idx < _dataHub.GetNumDataSets(); ++idx) {
        const auto& demographic_set = demographics.getDataSetDemographics(idx);
        if (!demographic_set.hasIndividualGeographically()) continue; // skip data sets w/o individual and descriptive lat/long.
        for (auto const &demographic : demographic_set.getAttributes()) {
            if (demographic.second->gettype() <= DESCRIPTIVE_COORD_X) continue; // Only want attributes, not individual id or descripive coordinates.
            if (!demographic.second->reportedInVisualizations()) { //skip if excluded from visualizations.
                _dataHub.GetPrintDirection().Printf(
                    "Excluding line list attribute '%s' from Google Earth KML file.\nAttribute has too many group values for reporting in KML output.\n",
                    BasePrint::P_WARNING, demographic.second->label().c_str()
                );
                continue;
            }
            if (std::find_if(event_types.begin(), event_types.end(), [&demographic](const std::string& et) { return et == demographic.first.second; }) == event_types.end())
                event_types.push_back(demographic.first.second);
        }
    }
    if (event_types.size() == 0) {
        _dataHub.GetPrintDirection().Printf("No line list attributes to display individuals by. Individuals were not added to Google KML file.\n", BasePrint::P_WARNING);
        return;
    }
    // Generating kml files for event type/grouping.
    _dataHub.GetPrintDirection().SetImpliedInputFileType(BasePrint::CASEFILE, true);
    bool storeWarn = _dataHub.GetPrintDirection().isSuppressingWarnings(); // prevent re-printing case file warnings
    _dataHub.GetPrintDirection().SetSuppressWarnings(true);
    std::sort(event_types.begin(), event_types.end());
    for (auto const&event_type : event_types) {
        _dataHub.GetPrintDirection().Printf("Adding line list data by '%s' to Google Earth file ...\n", BasePrint::P_STDOUT, event_type.c_str());
        add(demographics, event_type);
    }
    _dataHub.GetPrintDirection().SetSuppressWarnings(storeWarn);
}

/* Creates KML file for event type / grouping attribute. */
void ClusterKML::add(const DataDemographicsProcessor& demographics, const std::string& group_by) {
    const CParameters& parameters = _dataHub.GetParameters();
    std::string buffer, buffer2;
    // Create separate kml for events, then reference in primary cluster. (This data can technically be used independent of the cluster KML file.)
    _kml_files.resize(_kml_files.size() + 1);
    _kml_files.back().setFullPath(parameters.GetOutputFileName().c_str());
    _kml_files.back().setFileName(printString(buffer, "%s_individuals_group%u", _kml_files.back().getFileName().c_str(), _kml_files.size() - 1).c_str());
    _kml_files.back().setExtension(KML_FILE_EXT);
    std::ofstream kml_out;
    kml_out.open(_kml_files.back().getFullPath(buffer).c_str());
    if (!kml_out) throw resolvable_error("Error: Could not create file '%s'.\n", _kml_files.back().getFullPath(buffer).c_str());
    kml_out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
    kml_out << "<kml xmlns=\"http://www.opengis.net/kml/2.2\" xmlns:gx=\"http://www.google.com/kml/ext/2.2\">" << std::endl << "<Document>" << std::endl << std::endl;
    std::map<std::string, boost::shared_ptr<std::stringstream>> category_placemarks; // store placemarks of same category together
    std::map<std::string, unsigned int> category_frequency; // category frequency tracker
    std::map<size_t, boost::shared_ptr<std::stringstream>> dataset_ballonstyle;
    // Iterate over data sets
    for (size_t idx=0; idx < _dataHub.GetNumDataSets(); ++idx) {
        // skip data sets which don't include line list individuals at descriptive coordinates
        if (!demographics.getDataSetDemographics(idx).hasIndividualGeographically()) continue;
        std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(
            getFilenameFormatTime(parameters.GetCaseFileName(idx + 1), parameters.getTimestamp(), true),
            parameters.getInputSource(CASEFILE, idx + 1), _dataHub.GetPrintDirection()
        ));
        if (Source->getLinelistFieldsMap().size() == 0) continue; // skip data sources w/o line list mappings
        // Define the ballonstyle based on the line-list field maps of current data set - also test that this data source includes group by attribute.
        bool groupInSet = false;
        dataset_ballonstyle[idx] = boost::shared_ptr<std::stringstream>(new std::stringstream());
        *(dataset_ballonstyle[idx]) << "<BalloonStyle><text><![CDATA[<b style=\"white-space:nowrap;\">$[snippet]</b><br/><table border=\"0\">";
        if (parameters.GetIsSpaceTimeAnalysis())
            *(dataset_ballonstyle[idx]) << "<tr><th style=\"text-align:left;white-space:nowrap;padding-right:5px;\">Date</th><td style=\"white-space:nowrap;\">$[eventcasedate]</td></tr>";
        for (const auto& llfm : Source->getLinelistFieldsMap()) {
            if (llfm.get<1>() > DESCRIPTIVE_COORD_X) {
                *(dataset_ballonstyle[idx]) << "<tr><th style=\"text-align:left;white-space:nowrap;padding-right:5px;\">" << llfm.get<2>()
                    << "</th><td style=\"white-space:nowrap;\">$[" << llfm.get<2>() << "]</td></tr>";
            }
            groupInSet |= boost::iequals(llfm.get<2>(), group_by);
        }
        *(dataset_ballonstyle[idx]) << "</table>]]></text></BalloonStyle>";
        // If this group by attribute isn't present in this data source, skip reading it.
        if (!groupInSet) continue;
        // Iterate over the records of the case file - creating placemarks for each record.
        const char * value = 0;
        std::stringstream placemark, extended, coordinates;
        std::string category, event_date, end_date, individual, latitude, longitude;
        if (parameters.GetIsSpaceTimeAnalysis())
            JulianToString(end_date, _dataHub.GetStudyPeriodEndDate(), parameters.GetPrecisionOfTimesType(), "-", true, false, true);
		tract_t tid; count_t count; Julian case_date;
        while (Source->ReadRecord()) {
			DataSetHandler::RecordStatusType readStatus = _dataHub.GetDataSetHandler().RetrieveIdentifierIndex(*Source, tid); //read and validate that tract identifier
			if (readStatus != DataSetHandler::Accepted) continue; // should only be either Accepted or Ignored since we have already read this file
			readStatus = _dataHub.GetDataSetHandler().RetrieveCaseCounts(*Source, count);
			if (readStatus != DataSetHandler::Accepted) continue; // should only be either Accepted or Ignored since we have already read this file
			readStatus = _dataHub.GetDataSetHandler().RetrieveCountDate(*Source, case_date);
			if (readStatus != DataSetHandler::Accepted) continue; // should only be either Accepted or Ignored since we have already read this file
            category = ""; individual = ""; latitude = ""; longitude = "";
            placemark.str(""); extended.str(""); coordinates.str("");
            placemark << "<Placemark>" << std::endl;
            for (const auto& llfm : Source->getLinelistFieldsMap()) {
                value = Source->GetValueAtUnmapped(llfm.get<0>());
                value = value == 0 ? "" : value;
                if (llfm.get<1>() == INDIVIDUAL_ID) {
                    placemark << "<name>Individual: " << value << "</name>" << std::endl;
                    placemark << "<snippet>Individual: " << value << "</snippet>" << std::endl;
                    individual = value;
                } else if (llfm.get<1>() == DESCRIPTIVE_COORD_Y) {
                    latitude = value;
                } else if (llfm.get<1>() == DESCRIPTIVE_COORD_X) {
                    longitude = value;
                } else {
                    value = strlen(value) == 0 ? "~ blank ~" : value;
                    extended << "<Data name=\"" << encode(llfm.get<2>(), buffer) << "\"><value>" << encode(value, buffer2) << "</value></Data>";
                    if (boost::iequals(llfm.get<2>(), group_by)) category = value;
                }
            }
            if (category.length() == 0) continue; // skip this record if it does not contain the grouping attribute
            if (individual.length() == 0 || latitude.length() == 0 || longitude.length() == 0) { // warn then skip if we don't have enough information in the record
                _dataHub.GetPrintDirection().Printf("Unable to placemark individual of record %ld in case file to KML.\n", BasePrint::P_WARNING, Source->GetCurrentRecordIndex());
                continue;
            }
            double dlat, dlong; // warn and skip record if descriptive coordinates don't cast or are invalid
            if (!string_to_type<double>(latitude.c_str(), dlat) || !string_to_type<double>(longitude.c_str(), dlong) || fabs(dlat) > 90.0 && fabs(dlong) > 180.0) {
                _dataHub.GetPrintDirection().Printf("Unable to placemark individual of record %ld in case file to KML.\n", BasePrint::P_WARNING, Source->GetCurrentRecordIndex());
                continue;
            }
            if (_dataHub.GetParameters().GetIsSpaceTimeAnalysis()) { // set time span of individual if this is a space-time analysis
                placemark << "<TimeSpan>" << "<begin>" << JulianToString(event_date, case_date, parameters.GetPrecisionOfTimesType(), "-", false, false, true) << "</begin><end>" << end_date << "</end>" << "</TimeSpan>" << std::endl;
                extended << "<Data name=\"eventcasedate\"><value>" << JulianToString(event_date, case_date, parameters.GetPrecisionOfTimesType(), "/") << "</value></Data>";
            }
            placemark << "<Point><coordinates>" << longitude << " , " << latitude << ", 500</coordinates></Point>" << std::endl;
            placemark << "<ExtendedData>" << extended.str() << "</ExtendedData>" << std::endl;
            placemark << "<styleUrl>#events-" << toHex(category) << idx; // define the style for this category and current set index
            if (demographics.isNewIndividual(individual)) // individual status is also part of the style
                placemark << "-new";
            else if (demographics.inCluster(tid, case_date) && demographics.isExistingIndividual(individual))
                placemark << "-ongoing";
            else
                placemark << "-outside";
            placemark << "-stylemap</styleUrl>" << std::endl << "</Placemark>" << std::endl;
            auto pgroup = category_placemarks.find(category); // add placemark to correct placemark collection and update frequency
            if (pgroup == category_placemarks.end()) {
                category_placemarks[category] = boost::shared_ptr<std::stringstream>(new std::stringstream());
                category_frequency[category] = 0;
            }
            *(category_placemarks[category]) << placemark.str();
            category_frequency[category] += 1;
        }
    }
    // Now that we've read all the data, write groups to KML file
    std::vector<std::pair<std::string, unsigned int> > ordered_frequency;
    for (const auto& gr : category_frequency) ordered_frequency.push_back(gr); // create new collection which sorts the categories by frequency
    std::sort(ordered_frequency.begin(), ordered_frequency.end(), [](const std::pair<std::string, unsigned int> &left, const std::pair<std::string, unsigned int> &right) { return left.second > right.second; });
    // write the screen overlay / legend
    kml_out << "<ScreenOverlay><visibility>1</visibility><name><div style='text-decoration:underline;min-width:250px;'>Legend: " << encode(group_by, buffer);
    kml_out << "</div></name><Snippet></Snippet><description><div style='border: 1px solid black;background-color:#E5E4E2;padding-top:3px;padding-bottom:3px;'><div>" << std::endl;
    kml_out << "<ul style='padding-left:3px;margin-left:5px;margin-top:5px;padding-right: 5px;margin-bottom: 3px;'>" << std::endl;
    kml_out << "<li style='list-style-type:none;white-space:nowrap;'><div style='width:30px;height:10px;border:1px solid black; margin:0;padding:0;background-color:#FF0000;display:inline-block;'></div><span style='font-weight:bold;margin-left:5px;'>Inside Cluster, new entry</span></li>" << std::endl;
    kml_out << "<li style='list-style-type:none;white-space:nowrap;'><div style='width:30px;height:10px;border:1px solid black; margin:0;padding:0;background-color:#971D03;display:inline-block;'></div><span style='font-weight:bold;margin-left:5px;'>Inside Cluster, not new entry</span></li>" << std::endl;
    kml_out << "<li style='list-style-type:none;white-space:nowrap;'><div style='width:30px;height:10px;border:1px solid black; margin:0;padding:0;background-color:#FFFFFF;display:inline-block;'></div><span style='font-weight:bold;margin-left:5px;'>Outside Clusters</span></li>" << std::endl;
    kml_out << "</ul></div><hr style='border-top: 1px solid black;margin-top:10px;margin-bottom:5px;margin-left:10px;margin-right:10px;'/>" << std::endl;
    kml_out << "<ul style='padding-left:0'>" << std::endl;
    std::string website = AppToolkit::getToolkit().GetWebSite();
    // write the legend items for the categories - we'll squash together less frequent categories if there are too many
    std::stringstream squasheditems;
    auto itrShape = _visual_utils.getShapes().begin();
    for (auto const& pgroup : ordered_frequency) {
        if (itrShape != _visual_utils.getShapes().end()) {
            kml_out << "<li style='list-style-type:none;white-space:nowrap;margin-bottom: 5px;'><img style='vertical-align:middle;margin-left:5px;margin-right:6px;'"
                << " width='16' height='16' src='" << website << "images/events/" << *itrShape << "-whitecenter.png'/><span style='font-weight:bold;margin-left:5px;'>"
                << encode(pgroup.first, buffer) << "</span></li>" << std::endl;
            ++itrShape;
        } else
            squasheditems << (squasheditems.rdbuf()->in_avail() ? ", " : "") << pgroup.first;
    }
    if (squasheditems.rdbuf()->in_avail()) {
        kml_out << "<li style='list-style-type:none;white-space:nowrap;margin-bottom: 5px;' title='" << squasheditems.str() << "'><img style='vertical-align:middle;margin-left:5px;margin-right:6px;'"
            << " width='16' height='16' src='" << website << "images/events/" << _visual_utils.getAggregationShape() << "-whitecenter.png'/><span style='font-weight:bold;margin-left:5px;'>Other(s)</span></li>" << std::endl;
    }
    kml_out << "</ul></div></description>" << std::endl;
    kml_out << "<overlayXY x=\"1\" y=\"1\" xunits=\"fraction\" yunits=\"fraction\"/><screenXY x=\"1\" y=\"1\" xunits=\"fraction\" yunits=\"fraction\"/>";
    kml_out << "<rotationXY x=\"0\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/><size x=\"0\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/></ScreenOverlay>" << std::endl;
    kml_out << "<name>Individuals By " << encode(group_by, buffer) << "</name>" << std::endl;
    itrShape = itrShape = _visual_utils.getShapes().begin();
    for (auto const& pgroup: ordered_frequency) { // write the styles, balloon style, and placesmarks
        const auto& shape = itrShape != _visual_utils.getShapes().end() ? *itrShape : std::string(_visual_utils.getAggregationShape());
        const auto& groupHex = toHex(pgroup.first);
        for (size_t idx = 0; idx < _dataHub.GetNumDataSets(); ++idx) {
            if (!demographics.getDataSetDemographics(idx).hasIndividualGeographically()) continue;
            std::stringstream styleHex;
            styleHex << groupHex << idx;
            kml_out << "<Style id=\"events-" << styleHex.str() << "-new-style\"><IconStyle><color>" << toKmlColor("FF0000") << "</color><Icon><href>" << website << "images/events/" << shape << "-whitecenter.png"
                << "</href></Icon><scale>0.5</scale></IconStyle><LabelStyle><scale>0</scale></LabelStyle>" << dataset_ballonstyle[idx]->str() << "</Style>" << std::endl;
            kml_out << "<StyleMap id=\"events-" << styleHex.str() << "-new-stylemap\"><Pair><key>normal</key><styleUrl>#events-" << styleHex.str()
                << "-new-style</styleUrl></Pair><Pair><key>highlight</key><styleUrl>#events-" << styleHex.str() << "-new-style</styleUrl></Pair></StyleMap>" << std::endl;
            kml_out << "<Style id=\"events-" << styleHex.str() << "-ongoing-style\"><IconStyle><color>" << toKmlColor("971D03") << "</color><Icon><href>" << website << "images/events/" << shape << "-whitecenter.png"
                << "</href></Icon><scale>0.5</scale></IconStyle><LabelStyle><scale>0</scale></LabelStyle>" << dataset_ballonstyle[idx]->str() << "</Style>" << std::endl;
            kml_out << "<StyleMap id=\"events-" << styleHex.str() << "-ongoing-stylemap\"><Pair><key>normal</key><styleUrl>#events-" << styleHex.str()
                << "-ongoing-style</styleUrl></Pair><Pair><key>highlight</key><styleUrl>#events-" << styleHex.str() << "-ongoing-style</styleUrl></Pair></StyleMap>" << std::endl;
            kml_out << "<Style id=\"events-" << styleHex.str() << "-outside-style\"><IconStyle><color>" << toKmlColor("FFFFFF") << "</color><Icon><href>" << website << "images/events/" << shape << "-whitecenter.png"
                << "</href></Icon><scale>0.5</scale></IconStyle><LabelStyle><scale>0</scale></LabelStyle>" << dataset_ballonstyle[idx]->str() << "</Style>" << std::endl;
            kml_out << "<StyleMap id=\"events-" << styleHex.str() << "-outside-stylemap\"><Pair><key>normal</key><styleUrl>#events-" << styleHex.str()
                << "-outside-style</styleUrl></Pair><Pair><key>highlight</key><styleUrl>#events-" << styleHex.str() << "-outside-style</styleUrl></Pair></StyleMap>" << std::endl;
        }
        kml_out << "<Folder><name>" << encode(pgroup.first, buffer) << " (" << pgroup.second << ")</name>" << std::endl << category_placemarks[pgroup.first]->str() << "</Folder>";
        if (itrShape != _visual_utils.getShapes().end()) ++itrShape;
    }
    kml_out << "</Document>" << std::endl << "</kml>" << std::endl;
    kml_out.close();
    // Now reference this event kml file in a NetworkLink tag of primary kml.
    _kml_out << "\t<NetworkLink><name>Individuals By " << encode(group_by, buffer) << "</name><visibility>0</visibility><refreshVisibility>0</refreshVisibility>";
    _kml_out << "<Link><href>"<< _kml_files.back().getFileName() << KML_FILE_EXT << "</href></Link></NetworkLink>" << std::endl << std::endl;
}

void ClusterKML::finalize() {
    try {
        if (_dataHub.GetParameters().getIncludeLocationsKML() && !_cluster_locations.all()) {
            /* Create collection of locations which are not within a cluster. */
            std::string buffer;
            std::vector<double> vCoordinates;
            std::stringstream  locationPlacemarks;
            // create locations folder and locations within cluster placemarks
			for (auto itrGroup = _dataHub.getIdentifierInfo().getIdentifiers().begin(); itrGroup != _dataHub.getIdentifierInfo().getIdentifiers().end(); ++itrGroup) {
                for (unsigned int loc = 0; loc < itrGroup->get()->getLocations().size(); ++loc) {
                    if (!_cluster_locations.test(itrGroup->get()->getLocations()[loc]->index())) {
                        itrGroup->get()->getLocations()[loc]->coordinates()->retrieve(vCoordinates);
                        std::pair<double, double> prLatitudeLongitude(ConvertToLatLong(vCoordinates));
                        locationPlacemarks << "\t\t<Placemark>" << (_visibleLocations ? "" : "<visibility>0</visibility>") << "<description></description><styleUrl>";
                        if (_separateLocationsKML) {
                            // If creating separate KML files for locations, styles of primary kml need to be qualified in sub-kml files.
                            locationPlacemarks << _kml_files.front().getFileName() << _kml_files.front().getExtension();
                        }
                        locationPlacemarks << "#location-placemark</styleUrl><Point><coordinates>" << prLatitudeLongitude.second << ",";
                        locationPlacemarks << prLatitudeLongitude.first << ",0" << "</coordinates></Point></Placemark>" << std::endl;
                    }
                }
            }
            if (locationPlacemarks.str().size()) {
                if (_separateLocationsKML) {
                    // Create separate kml for this clusters locations, then reference in primary cluster.
                    _kml_files.resize(_kml_files.size() + 1);
                    _kml_files.back().setFullPath(_dataHub.GetParameters().GetOutputFileName().c_str());
                    _kml_files.back().setFileName("locations_outside_clusters");
                    _kml_files.back().setExtension(KML_FILE_EXT);

                    std::ofstream clusterKML;
                    clusterKML.open(_kml_files.back().getFullPath(buffer).c_str());
                    if (!clusterKML) throw resolvable_error("Error: Could not create file '%s'.\n", _kml_files.back().getFullPath(buffer).c_str());
                    clusterKML << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
                    clusterKML << "<kml xmlns=\"http://www.opengis.net/kml/2.2\">" << std::endl << "<Document>" << std::endl << std::endl;
                    clusterKML << "<name>Locations Outside Clusters</name>" << std::endl << locationPlacemarks.str() << "</Document>" << std::endl << "</kml>" << std::endl;
                    clusterKML.close();
                    // Now reference this kml file in NetworkLink tag of primary kml.
                    _kml_out << "\t<NetworkLink><name>Locations Outside Clusters</name><visibility>0</visibility><refreshVisibility>0</refreshVisibility><Link><href>"
                        << "locations_outside_clusters" << KML_FILE_EXT << "</href></Link></NetworkLink>" << std::endl << std::endl;
                } else { // Insert locations into primary kml.
                    _kml_out << "\t<Folder><name>Locations Outside Clusters</name><description></description>" << std::endl << locationPlacemarks.str() << "\t</Folder>" << std::endl << std::endl;
                }
            }
        }
        if (_dataHub.GetParameters().getUseLocationsNetworkFile()) {
            // Add edges of entire network when using a network file.
            std::string buffer;
            std::stringstream edges;
            std::vector<double> vCoordinates;

            Network::Connection_Details_t connections = GisUtils::getNetworkConnections(_dataHub.refLocationNetwork());
            for (const auto& connection : GisUtils::getNetworkConnections(_dataHub.refLocationNetwork())) {
                std::pair<double, double> prLatitudeLongitude(ConvertToLatLong(connection.get<0>()->coordinates()->retrieve(vCoordinates)));
                edges << "\t\t\t<Placemark><visibility>0</visibility><styleUrl>#line-edge</styleUrl><LineString><coordinates>";
                edges << prLatitudeLongitude.second << "," << prLatitudeLongitude.first << ",0  ";
                prLatitudeLongitude = ConvertToLatLong(connection.get<1>()->coordinates()->retrieve(vCoordinates));
                edges << prLatitudeLongitude.second << "," << prLatitudeLongitude.first << ",0</coordinates></LineString></Placemark>" << std::endl;
            }
            if (_separateLocationsKML) {
                // Create separate kml for this clusters locations, then reference in primary cluster.
                _kml_files.resize(_kml_files.size() + 1);
                _kml_files.back().setFullPath(_dataHub.GetParameters().GetOutputFileName().c_str());
                _kml_files.back().setFileName("network_edges");
                _kml_files.back().setExtension(KML_FILE_EXT);

                std::ofstream kmlnetwork;
                kmlnetwork.open(_kml_files.back().getFullPath(buffer).c_str());
                if (!kmlnetwork) throw resolvable_error("Error: Could not create file '%s'.\n", _kml_files.back().getFullPath(buffer).c_str());
                kmlnetwork << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
                kmlnetwork << "<kml xmlns=\"http://www.opengis.net/kml/2.2\">" << std::endl << "<Document>" << std::endl << std::endl;
                kmlnetwork << "\t<StyleMap id=\"line-edge\"><Pair><key>normal</key><Style><LineStyle><color>" << toKmlColor("#F39C12", "ff") << "</color><width>1.5</width><scale>1.0</scale></LineStyle></Style></Pair><Pair><key>highlight</key><Style id=\"line-edge3\"><LineStyle><color>" << toKmlColor("#F39C12", "ff") << "</color><width>1.5</width><scale>1.0</scale></LineStyle></Style></Pair></StyleMap>" << std::endl;

                kmlnetwork << "<name>Network Edges</name>" << std::endl << edges.str() << "</Document>" << std::endl << "</kml>" << std::endl;
                kmlnetwork.close();
                // Now reference this kml file in NetworkLink tag of primary kml.
                _kml_out << "\t<NetworkLink><name>Network Edges</name><visibility>0</visibility><refreshVisibility>0</refreshVisibility><Link><href>"
                    << "network_edges" << KML_FILE_EXT << "</href></Link></NetworkLink>" << std::endl << std::endl;
            }  else { // Insert edges into primary kml.
                _kml_out << "\t<Folder><name>Network Edges</name><description></description>" << std::endl << edges.str() << "\t</Folder>" << std::endl << std::endl;
            }
        }
        /* Finish writing the KML file. */
        _kml_out << "</Document>" << std::endl << "</kml>" << std::endl;
        _kml_out.close();
        if (_dataHub.GetParameters().getCompressClusterKML())
            createKMZ(_kml_files);
    } catch (prg_exception& x) {
        x.addTrace("finalize()", "ClusterKML");
        throw;
    }
}

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
#include <boost/dynamic_bitset.hpp>
#include <boost/algorithm/string.hpp>

/////////////////////////////////// BaseClusterKML ////////////////////////////////////////

const char * BaseClusterKML::KMZ_FILE_EXT = ".kmz";
const char * BaseClusterKML::KML_FILE_EXT = ".kml";
double BaseClusterKML::_minRatioToReport=0.001;

BaseClusterKML::BaseClusterKML(const CSaTScanData& dataHub) : _dataHub(dataHub), _visibleLocations(false) {
    _cluster_locations.resize(_dataHub.GetNumTracts() + _dataHub.GetTInfo()->getMetaManagerProxy().getNumMetaLocations());
    _separateLocationsKML = dataHub.GetTInfo()->getLocations().size() > _dataHub.GetParameters().getLocationsThresholdKML();
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

void BaseClusterKML::writeCluster(file_collection_t& fileCollection, std::ofstream& outKML, const CCluster& cluster, int iCluster, const SimulationVariables& simVars) const {
    std::string                                legend, locations, buffer, buffer2;
    std::vector<double>                        vCoordinates;
    std::pair<double, double>                  prLatitudeLongitude;
    TractHandler::Location::StringContainer_t  vTractIdentifiers;
    const double radius_km = cluster.GetLatLongRadius();
    bool isHighRate = cluster.getAreaRateForCluster(_dataHub) == HIGH;

    try {
        outKML << getClusterStyleTags(cluster, iCluster, buffer, isHighRate).c_str() << std::endl;
        outKML << "\t<Placemark>" << std::endl;
        if (_dataHub.GetParameters().getClusterMonikerPrefix().size()) {
            printString(buffer2, " (%sC%u)", _dataHub.GetParameters().getClusterMonikerPrefix().c_str(), (iCluster + 1));
        }
        outKML << "\t\t<name>#" << (iCluster + 1) << buffer2.c_str() << "</name>" << std::endl;
        outKML << "\t\t<snippet>#" << (iCluster + 1) << buffer2.c_str() << "</snippet>" << std::endl;
        outKML << "\t\t<visibility>" << (iCluster == 0 || cluster.isSignificant(_dataHub, iCluster, simVars) ? "1" : "0") << "</visibility>" << std::endl;
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
        if (_dataHub.GetParameters().getUseLocationsNetworkFile()) {
            outKML << "\t\t<Folder><name>Cluster " << (iCluster + 1) << " Edges</name>";
            Network::Connection_Details_t connections = _dataHub.refLocationNetwork().getClusterConnections(cluster, _dataHub);
            Network::Connection_Details_t::const_iterator itr = connections.begin(), end = connections.end();
            for (; itr != end; ++itr) {
                CentroidNeighborCalculator::getTractCoordinates(_dataHub, cluster, itr->get<0>(), vCoordinates);
                std::pair<double, double> prLatitudeLongitude(ConvertToLatLong(vCoordinates));
                outKML << "\t\t\t<Placemark><styleUrl>#" << (isHighRate ? "high" : "low") << "-line-edge</styleUrl><LineString><coordinates>";
                outKML << prLatitudeLongitude.second << "," << prLatitudeLongitude.first << ",0  ";
                CentroidNeighborCalculator::getTractCoordinates(_dataHub, cluster, itr->get<1>(), vCoordinates);
                prLatitudeLongitude = ConvertToLatLong(vCoordinates);
                outKML << prLatitudeLongitude.second << "," << prLatitudeLongitude.first << ",0</coordinates></LineString></Placemark>" << std::endl;
            }
            outKML << "\t\t</Folder>" << std::endl;
        }

        // add cluster locations if requested
        if (_dataHub.GetParameters().getIncludeLocationsKML()) {
            std::stringstream  clusterPlacemarks;
            // create locations folder and locations within cluster placemarks
            for (tract_t t=1; t <= cluster.GetNumTractsInCluster(); ++t) {
                tract_t tTract = _dataHub.GetNeighbor(cluster.GetEllipseOffset(), cluster.GetCentroidIndex(), t, cluster.GetCartesianRadius());
                if (!_dataHub.GetIsNullifiedLocation(tTract)) {
                    _dataHub.GetTInfo()->retrieveAllIdentifiers(tTract, vTractIdentifiers);
                    _cluster_locations.set(tTract);
                    CentroidNeighborCalculator::getTractCoordinates(_dataHub, cluster, tTract, vCoordinates);
                    prLatitudeLongitude = ConvertToLatLong(vCoordinates);
                    clusterPlacemarks << "\t\t<Placemark><name>" << vTractIdentifiers[0] << "</name>" << (_visibleLocations ? "" : "<visibility>0</visibility>")
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
                }
                else { // Insert locations into primary kml.
                    outKML << "\t<Folder><name>Cluster " << (iCluster + 1) << " Locations</name><description></description>" << std::endl << clusterPlacemarks.str() << "\t</Folder>" << std::endl << std::endl;
                }
            }
        } else {
            outKML << std::endl;
        }

    }
    catch (prg_exception& x) {
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

std::string BaseClusterKML::getRandomKmlColor() const {
    // The order of expression is aabbggrr, where aa = alpha(00 to ff); bb = blue(00 to ff); gg = green(00 to ff); rr = red(00 to ff).
    long bb = Equilikely(static_cast<long>(0), static_cast<long>(255), _rng),
         gg = Equilikely(static_cast<long>(0), static_cast<long>(255), _rng),
         rr = Equilikely(static_cast<long>(0), static_cast<long>(255), _rng);
    std::stringstream ss;
    ss << "ff" << std::hex << std::setw(2) << std::setfill('0') << bb << std::setw(2) << std::setfill('0') << gg << std::setw(2) << std::setfill('0') << rr;
    return ss.str();
}

std::string BaseClusterKML::convertKmlColorToHTMLColor(const std::string& colorKML) const {
    if (colorKML.size() != 8) throw prg_error("Can not convert kml color %s to html color.", colorKML.c_str());
    std::stringstream s;
    s << colorKML.substr(6, 2) << colorKML.substr(4, 2) << colorKML.substr(2, 2);
    return s.str();
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
    std::string buffer, bufferSetIdx;
    std::stringstream  templateLines;
    const CParameters& parameters = _dataHub.GetParameters();
    const char * rowFormat = "<tr><th style=\"text-align:left;white-space:nowrap;padding-right:5px;%s\">%s</th><td style=\"white-space:nowrap;\">$[%s%s]</td></tr>";
    const char * setRowFormat = "<tr><th style=\"text-align:left;white-space:nowrap;padding-right:5px;\">%s</th><td style=\"white-space:nowrap;\"></td></tr>";
    unsigned int currSetIdx = std::numeric_limits<unsigned int>::max(), numFilesSets = _dataHub.GetParameters().getNumFileSets();

    templateLines << "<![CDATA[<b>$[snippet]</b><br/><table border=\"0\">";
    CCluster::ReportCache_t::const_iterator itr = cluster.getReportLinesCache().begin(), itr_end = cluster.getReportLinesCache().end();
    for (; itr != itr_end; ++itr) {
        if (numFilesSets > 1 && itr->second.second > 0 && currSetIdx != itr->second.second) {
            // add table row for data set label
            printString(bufferSetIdx, "Data Set %u", itr->second.second);
            templateLines << printString(buffer, setRowFormat, bufferSetIdx.c_str()).c_str();
            currSetIdx = itr->second.second;
        }
        templateLines << printString(buffer,
            rowFormat,
            numFilesSets == 1 || itr->second.second == 0 ? "" : "padding-left:10px;",
            itr->first.c_str(),
            itr->first.c_str(),
            numFilesSets == 1 || itr->second.second == 0 ? "" : printString(bufferSetIdx, " set%u", itr->second.second).c_str()).c_str();
    }
    templateLines << "</table>]]>";
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
            << (numDataSets > 1 && itr->second.second != 0 ? printString(bufferSetIdx, " set%u", itr->second.second).c_str() : "")
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
    outKML << "\t<StyleMap id=\"low-line-edge\"><Pair><key>normal</key><Style><LineStyle><color>" << getStyleColor(false, true, buffer) << "</color><width>3</width><scale>1.0</scale></LineStyle></Style></Pair><Pair><key>highlight</key><Style id=\"line-edge1\"><LineStyle><color>" << getStyleColor(false, false, buffer) << "</color><width>3</width><scale>1.0</scale></LineStyle></Style></Pair></StyleMap>" << std::endl;

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

ClusterKML::ClusterKML(const CSaTScanData& dataHub) : BaseClusterKML(dataHub), _clusters_written(0), _locations_written(0){
    _fileCollection.resize(_fileCollection.size() + 1);
    _fileCollection.back().setFullPath(_dataHub.GetParameters().GetOutputFileName().c_str());
    _fileCollection.back().setExtension(KML_FILE_EXT);

    std::string buffer;
    //open output file
    _kml_out.open(_fileCollection.back().getFullPath(buffer).c_str());
    if (!_kml_out) throw resolvable_error("Error: Could not create file '%s'.\n", _fileCollection.back().getFullPath(buffer).c_str());
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
                _locations_written += static_cast<unsigned int>(clusters.GetCluster(i).GetNumTractsInCluster());
        }
    }
    _clusters_written += addClusters(clusters, simVars, _kml_out, _fileCollection, _clusters_written);
}

/* Adds event level data to the KML output - if case data defines line-list data and event-id information. */
void ClusterKML::add(const DataDemographicsProcessor& demographics) {
    const CParameters& parameters = _dataHub.GetParameters();
    // First test whether any of the data sets report event-id level and it's coordinates.
    bool anySet = false;
    for (size_t idx = 0; idx < _dataHub.GetNumDataSets() && anySet == false; ++idx)
        anySet |= demographics.getEventStatus(idx).get<0>() && demographics.getEventStatus(idx).get<1>();
    if (!anySet) {
        _dataHub.GetPrintDirection().Printf(
            "Event coordinates were no found in line list data. Event placements will not be added to KML output.\n",
            BasePrint::P_WARNING);
        return; // nothing to report in terms of event level data
    }
    // Find the grouping charateristic in mapping.
    std::string group_by = parameters.getKmlEventGroupAttribute();
    bool found = false;
    for (auto demographic: demographics.getDataSetDemographics().getAttributes()) {
        if (demographic.second->gettype() <= EVENT_COORD_Y) continue;
        if (boost::iequals(demographic.first, group_by)) {
            found = true; break;
        }
    }
    if (!found) {
        _dataHub.GetPrintDirection().Printf("Unable to find grouping charateristic '%s' in line list.\nEvent placements will not be added to KML output.\n", BasePrint::P_WARNING, group_by.c_str());
        return;
    }
    std::string buffer, buffer2;
    // Create separate kml for events, then reference in primary cluster. (This data can technically be used independent of the cluster KML file.)
    _fileCollection.resize(_fileCollection.size() + 1);
    _fileCollection.back().setFullPath(parameters.GetOutputFileName().c_str());
    _fileCollection.back().setFileName(printString(buffer, "%s_events", _fileCollection.back().getFileName().c_str()).c_str());
    _fileCollection.back().setExtension(KML_FILE_EXT);
    std::ofstream eventKML;
    eventKML.open(_fileCollection.back().getFullPath(buffer).c_str());
    if (!eventKML) throw resolvable_error("Error: Could not create file '%s'.\n", _fileCollection.back().getFullPath(buffer).c_str());
    eventKML << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
    eventKML << "<kml xmlns=\"http://www.opengis.net/kml/2.2\">" << std::endl << "<Document>" << std::endl << std::endl;
    // Iterate over data sets
    std::map<std::string, boost::shared_ptr<std::stringstream> > _group_placemarks;
    std::stringstream ballonstyle;
    for (size_t idx=0; idx < _dataHub.GetNumDataSets(); ++idx) {
        // First test whether this data set reported event id and coordinates.
        if (!(demographics.getEventStatus(idx).get<0>() && demographics.getEventStatus(idx).get<1>()))
            continue;
        // Open the data source and read all the records of the case file again.
        _dataHub.GetPrintDirection().SetImpliedInputFileType(BasePrint::CASEFILE);
        std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(
            getFilenameFormatTime(parameters.GetCaseFileName(idx + 1), parameters.getTimestamp(), true),
            parameters.getInputSource(CASEFILE, idx + 1), _dataHub.GetPrintDirection()
        ));
        if (Source->getLinelistFieldsMap().size() == 0) continue; // no mappings defined for this data set.
        // Define the ballonstyle based on the line-list field maps of primary data set.
        // We'd expect all sets to be same in line-list columns but we aren't checking ... just assume for now.
        if (idx == 0) {
            ballonstyle << "<BalloonStyle><text><![CDATA[<b style=\"white-space:nowrap;\">$[snippet]</b><br/><table border=\"0\">";
            if (_dataHub.GetParameters().GetIsSpaceTimeAnalysis())
                ballonstyle << "<tr><th style=\"text-align:left;white-space:nowrap;padding-right:5px;\">Event Date</th><td style=\"white-space:nowrap;\">$[eventcasedate]</td></tr>";
            for (auto llfm : Source->getLinelistFieldsMap()) {
                if (!(llfm.second.get<0>() == EVENT_ID || llfm.second.get<0>() == EVENT_COORD_Y || llfm.second.get<0>() == EVENT_COORD_X)) {
                    ballonstyle << "<tr><th style=\"text-align:left;white-space:nowrap;padding-right:5px;\">" << llfm.second.get<1>()
                                << "</th><td style=\"white-space:nowrap;\">$[" << llfm.second.get<1>() << "]</td></tr>";
                }
            }
            ballonstyle << "</table>]]></text></BalloonStyle>";
        }
        // Iterate over the records of the case file - creating placemarks for each event;
        const char * value = 0;
        std::stringstream placemark, extended, coordinates;
        std::string group_value, event_date, end_date, event_id, latitude, longitude;
        if (_dataHub.GetParameters().GetIsSpaceTimeAnalysis())
            end_date = gregorianToString(gregorianFromString(parameters.GetStudyPeriodEndDate()), "%Y-%m-%d");
        while (Source->ReadRecord()) {
            Julian case_date;
            if (_dataHub.GetDataSetHandler().RetrieveCountDate(*Source, case_date) != DataSetHandler::Accepted)
                return throw prg_error("Failed to read case file date in data set %d.", "ClusterKML::add(const DataDemographicsProcessor&)", idx + 1);
            group_value = ""; event_id = ""; latitude = ""; longitude = "";
            placemark.str(""); extended.str(""); coordinates.str("");
            placemark << "<Placemark>" << std::endl;
            for (auto itr = Source->getLinelistFieldsMap().begin(); itr != Source->getLinelistFieldsMap().end(); ++itr) {
                value = Source->GetValueAtUnmapped(itr->first);
                if (itr->second.get<0>() == EVENT_ID) {
                    placemark << "<name>Event: " << value << "</name>" << std::endl;
                    placemark << "<snippet>Event: " << value << "</snippet>" << std::endl;
                    event_id = value;
                } else if (itr->second.get<0>() == EVENT_COORD_Y) {
                    latitude = value;
                } else if (itr->second.get<0>() == EVENT_COORD_X) {
                    longitude = value;
                } else {
                    extended << "<Data name=\"" << itr->second.get<1>() << "\"><value>" << value << "</value></Data>";
                    if (boost::iequals(itr->second.get<1>(), group_by)) group_value = value;
                }
            }
            // At least the minimal checking - confirm that event_id, coordinates and group value are present in record.
            if (event_id.length() == 0 || latitude.length() == 0 || longitude.length() == 0 || group_value.length() == 0) {
                _dataHub.GetPrintDirection().Printf("Unable to placemark event in record %ld if case file.\n", BasePrint::P_WARNING, Source->GetCurrentRecordIndex());
                continue;
            }
            if (_dataHub.GetParameters().GetIsSpaceTimeAnalysis()) { // Set time span of event if this is a space-time analysis.
                placemark << "<TimeSpan>" << "<begin>" << JulianToString(event_date, case_date, parameters.GetPrecisionOfTimesType(), "-") << "</begin>"
                    << "<end>" << end_date << "</end>" << "</TimeSpan>";
                extended << "<Data name=\"eventcasedate\"><value>" << JulianToString(event_date, case_date, parameters.GetPrecisionOfTimesType(), "/") << "</value></Data>";
            }
            placemark << "<styleUrl>#events-"<< toHex(group_value) << (demographics.isNewEvent(event_id) ? "-new" : "") << "-stylemap</styleUrl>" << std::endl;
            placemark << "<Point><coordinates>" << longitude << " , " << latitude << ", 500</coordinates></Point>" << std::endl;
            placemark << "<ExtendedData>" << extended.str() << "</ExtendedData>" << std::endl << "</Placemark>" << std::endl;
            // Add placemark to correct placemark group.
            auto pgroup = _group_placemarks.find(group_value);
            if (pgroup == _group_placemarks.end())
                _group_placemarks[group_value] = boost::shared_ptr<std::stringstream>(new std::stringstream());
            *(_group_placemarks[group_value]) << placemark.str();
        }
    }
    // Now that we've read all the data, write groups to KML file.
    std::vector<std::string> event_color_defaults = { // seperate colors for each group.
        "ff0c8ff7", "ffb2834d", "ff00ccf9", "ff86bc73", "ff4016ca", "ffc1c34d", "ff9ed0d1", "ff4f493c", "ff53dbc5",
        "ff25306c", "ff5a6666", "ff476c10", "ff5979f2", "ff4d7cb2", "ff310a58", "ff0c9fa1", "ffa3d0e1"
    };
    std::string event_icon = "https://maps.google.com/mapfiles/kml/shapes/placemark_square.png";
    std::string new_event_icon = "http://maps.google.com/mapfiles/kml/shapes/square.png";
    // First create the ScreenOverlay, which is the legend.
    eventKML << "<ScreenOverlay><visibility>1</visibility><name><div style=\"text-decoration:underline;\">Legend: " << parameters.getKmlEventGroupAttribute();
    eventKML << "</div></name><Snippet></Snippet><description>";
    eventKML << "<div><img style=\"vertical-align:middle;padding-right:6px\" width=\"22\" height=\"22\" src=\"" << new_event_icon << "\"/> <span style=\"font-weight:bold;padding-right:8px;\">New</span>";
    eventKML << "<img width=\"32\" height=\"32\" style=\"vertical-align:middle;padding-right:0px;\" src=\"" << event_icon << "\"/> <span style=\"font-weight:bold;\">Ongoing</span></div>";
    eventKML << "<ul style=\"padding-left:0\">" << std::endl;
    std::vector<std::pair<std::string, std::string> > group_colors;
    auto itrColor = event_color_defaults.begin();
    for (auto pgroup : _group_placemarks) {
        auto colorKML = (itrColor == event_color_defaults.end() ? getRandomKmlColor() : *itrColor);
        group_colors.push_back(std::make_pair(colorKML, convertKmlColorToHTMLColor(colorKML)));
        eventKML << "<li style=\"list-style-type:none;white-space:nowrap;\"><div style=\"width:30px;height:10px;border:1px solid black; margin:0;padding:0;background-color:#";
        eventKML << group_colors.back().second << ";display:inline-block;\"></div><span style=\"font-weight:bold;margin-left:5px;\">" << pgroup.first << "</span></li>" << std::endl;
        if (itrColor != event_color_defaults.end()) ++itrColor;
    }
    eventKML << "</ul></description>" << std::endl;
    eventKML << "<overlayXY x=\"1\" y=\"1\" xunits=\"fraction\" yunits=\"fraction\"/><screenXY x=\"1\" y=\"1\" xunits=\"fraction\" yunits=\"fraction\"/>";
    eventKML << "<rotationXY x=\"0\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/><size x=\"0\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/></ScreenOverlay>" << std::endl;
    auto itrColors = group_colors.begin();
    eventKML << "<name>Events By " << parameters.getKmlEventGroupAttribute() << "</name>" << std::endl;
    for (auto pgroup: _group_placemarks) {
        auto color = itrColors->first;
        auto groupHex = toHex(pgroup.first);
        eventKML << "<Style id=\"events-" << groupHex << "-style\"><IconStyle><color>" << color << "</color><Icon><href>" << event_icon
            << "</href><scale>0.25</scale></Icon></IconStyle><LabelStyle><scale>0</scale></LabelStyle>";
        eventKML << ballonstyle.str() << "</Style>" << std::endl;
        eventKML << "<StyleMap id=\"events-" << groupHex << "-stylemap\"><Pair><key>normal</key><styleUrl>#events-" << groupHex
            << "-style</styleUrl></Pair><Pair><key>highlight</key><styleUrl>#events-" << groupHex << "-style</styleUrl></Pair></StyleMap>" << std::endl;
        eventKML << "<Style id=\"events-" << groupHex << "-new-style\"><IconStyle><color>" << color << "</color><Icon><href>" << new_event_icon
            << "</href><scale>0.25</scale></Icon></IconStyle><LabelStyle><scale>0</scale></LabelStyle>";
        eventKML << ballonstyle.str() << "</Style>" << std::endl;
        eventKML << "<StyleMap id=\"events-" << groupHex << "-new-stylemap\"><Pair><key>normal</key><styleUrl>#events-" << groupHex
            << "-new-style</styleUrl></Pair><Pair><key>highlight</key><styleUrl>#events-" << groupHex << "-new-style</styleUrl></Pair></StyleMap>" << std::endl;
        eventKML << "<Folder><name>" << pgroup.first << "</name>" << std::endl << pgroup.second->str() << "</Folder>";
        ++itrColors;
    }
    eventKML << "</Document>" << std::endl << "</kml>" << std::endl;
    eventKML.close();
    // Now reference this event kml file in a NetworkLink tag of primary kml.
    _kml_out << "\t<NetworkLink><name>Events By " << group_by << "</name><visibility>0</visibility><refreshVisibility>0</refreshVisibility>"
        << "<Link><href>"<< _fileCollection.back().getFileName() << KML_FILE_EXT << "</href></Link></NetworkLink>" << std::endl << std::endl;
}

void ClusterKML::finalize() {
    try {
        if (_dataHub.GetParameters().getIncludeLocationsKML() && !_cluster_locations.all()) {
            /* Create collection of locations which are not within a cluster. */
            std::string buffer;
            std::vector<double> vCoordinates;
            std::stringstream  locationPlacemarks;
            const TractHandler::LocationsContainer_t & locations = _dataHub.GetTInfo()->getLocations();
            // create locations folder and locations within cluster placemarks
            for (size_t t=0; t < locations.size(); ++t) {
                if (!_cluster_locations.test(t)) {
                    locations[t]->getCoordinates()[locations[t]->getCoordinates().size() - 1]->retrieve(vCoordinates);
                    std::pair<double, double> prLatitudeLongitude(ConvertToLatLong(vCoordinates));
                    locationPlacemarks << "\t\t<Placemark>" << (_visibleLocations ? "" : "<visibility>0</visibility>") << "<description></description><styleUrl>";
                    if (_separateLocationsKML) {
                        // If creating separate KML files for locations, styles of primary kml need to be qualified in sub-kml files.
                        locationPlacemarks << _fileCollection.front().getFileName() << _fileCollection.front().getExtension();
                    }
                    locationPlacemarks << "#location-placemark</styleUrl><Point><coordinates>" << prLatitudeLongitude.second << ","
                        << prLatitudeLongitude.first << ",0" << "</coordinates></Point></Placemark>" << std::endl;
                }
            }
            if (locationPlacemarks.str().size()) {
                if (_separateLocationsKML) {
                    // Create separate kml for this clusters locations, then reference in primary cluster.
                    _fileCollection.resize(_fileCollection.size() + 1);
                    _fileCollection.back().setFullPath(_dataHub.GetParameters().GetOutputFileName().c_str());
                    _fileCollection.back().setFileName("locations_outside_clusters");
                    _fileCollection.back().setExtension(KML_FILE_EXT);

                    std::ofstream clusterKML;
                    clusterKML.open(_fileCollection.back().getFullPath(buffer).c_str());
                    if (!clusterKML) throw resolvable_error("Error: Could not create file '%s'.\n", _fileCollection.back().getFullPath(buffer).c_str());
                    clusterKML << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
                    clusterKML << "<kml xmlns=\"http://www.opengis.net/kml/2.2\">" << std::endl << "<Document>" << std::endl << std::endl;
                    clusterKML << "<name>Locations Outside Clusters</name>" << std::endl << locationPlacemarks.str() << "</Document>" << std::endl << "</kml>" << std::endl;
                    clusterKML.close();
                    // Now reference this kml file in NetworkLink tag of primary kml.
                    _kml_out << "\t<NetworkLink><name>Locations Outside Clusters</name><visibility>0</visibility><refreshVisibility>0</refreshVisibility><Link><href>"
                        << "locations_outside_clusters" << KML_FILE_EXT << "</href></Link></NetworkLink>" << std::endl << std::endl;
                }
                else { // Insert locations into primary kml.
                    _kml_out << "\t<Folder><name>Locations Outside Clusters</name><description></description>" << std::endl << locationPlacemarks.str() << "\t</Folder>" << std::endl << std::endl;
                }
            }
        }
        /* Finish writing the KML file. */
        _kml_out << "</Document>" << std::endl << "</kml>" << std::endl;
        _kml_out.close();
        if (_dataHub.GetParameters().getCompressClusterKML())
            createKMZ(_fileCollection);
    } catch (prg_exception& x) {
        x.addTrace("finalize()", "ClusterKML");
        throw;
    }
}

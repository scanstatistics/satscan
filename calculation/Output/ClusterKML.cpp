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
                _locations_written += static_cast<unsigned int>(clusters.GetCluster(i).getNumIdentifiers());
        }
    }
    _clusters_written += addClusters(clusters, simVars, _kml_out, _fileCollection, _clusters_written);
}

/* Adds event level data to the KML output for all event attributes. */
void ClusterKML::add(const DataDemographicsProcessor& demographics) {
    std::vector<std::string> g_values; // Get the values specified by user.
    csv_string_to_typelist<std::string>(_dataHub.GetParameters().getKmlEventGroupAttribute().c_str(), g_values);
    std::set<std::string> grouping_by; // All the columns which we'll be generating an events kml file.
    for (auto const &demographic : demographics.getDataSetDemographics().getAttributes()) {
        if (demographic.second->gettype() <= DESCRIPTIVE_COORD_Y) continue;
        if (std::find(g_values.begin(), g_values.end(), demographic.first) == g_values.end()) continue;
        grouping_by.emplace(demographic.first);
    }
    if (grouping_by.size() == 0) {
        _dataHub.GetPrintDirection().Printf(
            "No characteristics to group by. Event placements will not be added to KML output.\n",
            BasePrint::P_WARNING
        );
        return;
    }
    _dataHub.GetPrintDirection().SetImpliedInputFileType(BasePrint::CASEFILE, true);
    int storeMax = _dataHub.GetPrintDirection().getMaximumReadErrors();
    for (auto const&group_by : grouping_by) {
        _dataHub.GetPrintDirection().Printf("Adding event data by '%s' to Google Earth file ...\n", BasePrint::P_STDOUT, group_by.c_str());
        add(demographics, group_by);
        _dataHub.GetPrintDirection().SetMaximumReadErrors(0); // Don't repeat same read errors.
    }
    _dataHub.GetPrintDirection().SetMaximumReadErrors(storeMax);
}

/* Adds event level data to the KML output - if case data defines line-list data and event-id information. */
void ClusterKML::add(const DataDemographicsProcessor& demographics, const std::string& group_by) {
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
    bool found = false;
    for (const auto& demographic: demographics.getDataSetDemographics().getAttributes()) {
        if (demographic.second->gettype() <= DESCRIPTIVE_COORD_Y) continue;
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
    std::string file_name = group_by;
    std::transform(std::begin(group_by), std::end(group_by), std::begin(file_name), [](char ch) {
        return (ch == '<' || ch == '>' || ch == ':' || ch == '"' || ch == '/' || ch == '\\' || ch == '|' || ch == '?' || ch == '*' || ch == ' ') ? '_' : ch;
    });
    _fileCollection.resize(_fileCollection.size() + 1);
    _fileCollection.back().setFullPath(parameters.GetOutputFileName().c_str());
    _fileCollection.back().setFileName(printString(buffer, "%s_events%u_by_%s", _fileCollection.back().getFileName().c_str(), _fileCollection.size(), file_name.c_str()).c_str());
    _fileCollection.back().setExtension(KML_FILE_EXT);
    std::ofstream eventKML;
    eventKML.open(_fileCollection.back().getFullPath(buffer).c_str());
    if (!eventKML) throw resolvable_error("Error: Could not create file '%s'.\n", _fileCollection.back().getFullPath(buffer).c_str());
    eventKML << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
    eventKML << "<kml xmlns=\"http://www.opengis.net/kml/2.2\" xmlns:gx=\"http://www.google.com/kml/ext/2.2\">" << std::endl << "<Document>" << std::endl << std::endl;
    // Iterate over data sets
    std::map<std::string, boost::shared_ptr<std::stringstream> > group_placemarks;
    std::map<std::string, unsigned int> group_frequency;
    std::stringstream ballonstyle;
    for (size_t idx=0; idx < _dataHub.GetNumDataSets(); ++idx) {
        // First test whether this data set reported event id and coordinates.
        if (!(demographics.getEventStatus(idx).get<0>() && demographics.getEventStatus(idx).get<1>()))
            continue;
        // Open the data source and read all the records of the case file again.
        std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(
            getFilenameFormatTime(parameters.GetCaseFileName(idx + 1), parameters.getTimestamp(), true),
            parameters.getInputSource(CASEFILE, idx + 1), _dataHub.GetPrintDirection()
        ));
        if (Source->getLinelistFieldsMap().size() == 0) continue; // no mappings defined for this data set.
        // Define the ballonstyle based on the line-list field maps of primary data set.
        // We'd expect all sets to be same in line-list columns but we aren't checking ... just assume for now.
        if (idx == 0) {
            ballonstyle << "<BalloonStyle><text><![CDATA[<b style=\"white-space:nowrap;\">$[snippet]</b><br/><table border=\"0\">";
            if (parameters.GetIsSpaceTimeAnalysis())
                ballonstyle << "<tr><th style=\"text-align:left;white-space:nowrap;padding-right:5px;\">Event Date</th><td style=\"white-space:nowrap;\">$[eventcasedate]</td></tr>";
            for (const auto& llfm : Source->getLinelistFieldsMap()) {
                if (!(llfm.second.get<0>() == DESCRIPTIVE_COORD_Y || llfm.second.get<0>() == DESCRIPTIVE_COORD_X)) {
                    ballonstyle << "<tr><th style=\"text-align:left;white-space:nowrap;padding-right:5px;\">" << llfm.second.get<1>()
                                << "</th><td style=\"white-space:nowrap;\">$[" << llfm.second.get<1>() << "]</td></tr>";
                }
            }
            ballonstyle << "</table>]]></text></BalloonStyle>";
        }
        // Iterate over the records of the case file - creating placemarks for each record.
        const char * value = 0;
        std::stringstream placemark, extended, coordinates;
        std::string group_value, event_date, end_date, event_id, latitude, longitude, identifier;
        if (parameters.GetIsSpaceTimeAnalysis()) {
            JulianToString(end_date, _dataHub.GetStudyPeriodEndDate(), parameters.GetPrecisionOfTimesType(), "-", true, false, true);
        }
		tract_t tid; count_t count; Julian case_date;
        while (Source->ReadRecord()) {
			DataSetHandler::RecordStatusType readStatus = _dataHub.GetDataSetHandler().RetrieveIdentifierIndex(*Source, tid); //read and validate that tract identifier

            // TODO: What if count greater than zero? Create multiple of the same record?
            //       If not, what happens if this record is split into 2 or more identical records?

            _dataHub.getIdentifierInfo().getIdentifierNameAtIndex(tid, identifier);
			if (readStatus != DataSetHandler::Accepted) continue; // Should only be either Accepted or Ignored.
			readStatus = _dataHub.GetDataSetHandler().RetrieveCaseCounts(*Source, count);
			if (readStatus != DataSetHandler::Accepted) continue; // Should only be either Accepted or Ignored.

			readStatus = _dataHub.GetDataSetHandler().RetrieveCountDate(*Source, case_date);
			if (readStatus != DataSetHandler::Accepted) continue; // Should only be either Accepted or Ignored.
            group_value = ""; event_id = ""; latitude = ""; longitude = "";
            placemark.str(""); extended.str(""); coordinates.str("");
            placemark << "<Placemark>" << std::endl;
            placemark << "<name>Identifier: " << identifier << "</name>" << std::endl;
            placemark << "<snippet>Identifier: " << identifier << "</snippet>" << std::endl;
            for (auto itr = Source->getLinelistFieldsMap().begin(); itr != Source->getLinelistFieldsMap().end(); ++itr) {
                value = Source->GetValueAtUnmapped(itr->first);
                value = value == 0 ? "" : value;
                /*if (itr->second.get<0>() == INDIVIDUAL_ID) {
                    placemark << "<name>Event: " << value << "</name>" << std::endl;
                    placemark << "<snippet>Event: " << value << "</snippet>" << std::endl;
                    event_id = value;
                } else*/ if (itr->second.get<0>() == DESCRIPTIVE_COORD_Y) {
                    latitude = value;
                } else if (itr->second.get<0>() == DESCRIPTIVE_COORD_X) {
                    longitude = value;
                } else {
                    extended << "<Data name=\"" << itr->second.get<1>() << "\"><value>" << value << "</value></Data>";
                    if (boost::iequals(itr->second.get<1>(), group_by)) group_value = value;
                }
            }
            // At least the minimal checking - confirm that event_id, coordinates and group value are present in record.
            if (/*event_id.length() == 0 ||*/ latitude.length() == 0 || longitude.length() == 0 || group_value.length() == 0) {
                _dataHub.GetPrintDirection().Printf("Unable to placemark event of record %ld in case file to KML.\n", BasePrint::P_READERROR, Source->GetCurrentRecordIndex());
                continue;
            }
            double dlat, dlong;
            if (!string_to_type<double>(latitude.c_str(), dlat) || !string_to_type<double>(longitude.c_str(), dlong) || fabs(dlat) > 90.0 && fabs(dlong) > 180.0) {
                _dataHub.GetPrintDirection().Printf("Unable to placemark event of record %ld in case file to KML.\n", BasePrint::P_READERROR, Source->GetCurrentRecordIndex());
                continue;
            }
            if (_dataHub.GetParameters().GetIsSpaceTimeAnalysis()) { // Set time span of event if this is a space-time analysis.
                placemark << "<TimeSpan>" << "<begin>" << JulianToString(event_date, case_date, parameters.GetPrecisionOfTimesType(), "-", false, false, true) << "</begin><end>" << end_date << "</end>" << "</TimeSpan>" << std::endl;
                extended << "<Data name=\"eventcasedate\"><value>" << JulianToString(event_date, case_date, parameters.GetPrecisionOfTimesType(), "/") << "</value></Data>";
            }
            placemark << "<Point><coordinates>" << longitude << " , " << latitude << ", 500</coordinates></Point>" << std::endl;
            placemark << "<ExtendedData>" << extended.str() << "</ExtendedData>" << std::endl;
            placemark << "<styleUrl>#events-" << toHex(group_value);
            if (demographics.isNewEvent(event_id))
                placemark << "-new";
            else if (demographics.isExistingEvent(event_id))
                placemark << "-ongoing";
            else
                placemark << "-outside";
            placemark << "-stylemap</styleUrl>" << std::endl << "</Placemark>" << std::endl;
            // Add placemark to correct placemark group.
            auto pgroup = group_placemarks.find(group_value);
            if (pgroup == group_placemarks.end()) {
                group_placemarks[group_value] = boost::shared_ptr<std::stringstream>(new std::stringstream());
                group_frequency[group_value] = 0;
            }
            *(group_placemarks[group_value]) << placemark.str();
            group_frequency[group_value] += 1;
        }
    }
    // Now that we've read all the data, write groups to KML file.
    // First create the ScreenOverlay, which is the legend.
    std::vector<std::pair<std::string, unsigned int> > ordered_frequency;
    for (const auto& gr : group_frequency) {
        ordered_frequency.push_back(gr);
    }
    std::sort(ordered_frequency.begin(), ordered_frequency.end(), [](const std::pair<std::string, unsigned int> &left, const std::pair<std::string, unsigned int> &right) { return left.second > right.second; });
    eventKML << "<ScreenOverlay><visibility>1</visibility><name><div style='text-decoration:underline;min-width:250px;'>Legend: " << group_by;
    eventKML << "</div></name><Snippet></Snippet><description><div style='border: 1px solid black;background-color:#E5E4E2;padding-top:3px;padding-bottom:3px;'><div>" << std::endl;
    eventKML << "<ul style='padding-left:3px;margin-left:5px;margin-top:5px;padding-right: 5px;margin-bottom: 3px;'>" << std::endl;
    eventKML << "<li style='list-style-type:none;white-space:nowrap;'><div style='width:30px;height:10px;border:1px solid black; margin:0;padding:0;background-color:#FF0000;display:inline-block;'></div><span style='font-weight:bold;margin-left:5px;'>Inside Cluster, new entry</span></li>" << std::endl;
    eventKML << "<li style='list-style-type:none;white-space:nowrap;'><div style='width:30px;height:10px;border:1px solid black; margin:0;padding:0;background-color:#971D03;display:inline-block;'></div><span style='font-weight:bold;margin-left:5px;'>Inside Cluster, not new entry</span></li>" << std::endl;
    eventKML << "<li style='list-style-type:none;white-space:nowrap;'><div style='width:30px;height:10px;border:1px solid black; margin:0;padding:0;background-color:#FFFFFF;display:inline-block;'></div><span style='font-weight:bold;margin-left:5px;'>Outside Clusters</span></li>" << std::endl;
    eventKML << "</ul></div><hr style='border-top: 1px solid black;margin-top:10px;margin-bottom:5px;margin-left:10px;margin-right:10px;'/>" << std::endl;
    eventKML << "<ul style='padding-left:0'>" << std::endl;
    std::string website = AppToolkit::getToolkit().GetWebSite();
    std::stringstream squasheditems;
    auto itrShape = _visual_utils.getShapes().begin();
    for (auto const& pgroup : ordered_frequency) {
        if (itrShape != _visual_utils.getShapes().end()) {
            eventKML << "<li style='list-style-type:none;white-space:nowrap;margin-bottom: 5px;'><img style='vertical-align:middle;margin-left:5px;margin-right:6px;'"
                << " width='16' height='16' src='" << website << "images/events/" << *itrShape << "-whitecenter.png'/><span style='font-weight:bold;margin-left:5px;'>"
                << pgroup.first << "</span></li>" << std::endl;
            ++itrShape;
        } else {
            squasheditems << (squasheditems.rdbuf()->in_avail() ? ", " : "") << pgroup.first;
        }
    }
    if (squasheditems.rdbuf()->in_avail()) {
        eventKML << "<li style='list-style-type:none;white-space:nowrap;margin-bottom: 5px;' title='" << squasheditems.str() << "'><img style='vertical-align:middle;margin-left:5px;margin-right:6px;'"
            << " width='16' height='16' src='" << website << "images/events/" << _visual_utils.getAggregationShape() << "-whitecenter.png'/><span style='font-weight:bold;margin-left:5px;'>Other(s)</span></li>" << std::endl;
    }
    eventKML << "</ul></div></description>" << std::endl;
    eventKML << "<overlayXY x=\"1\" y=\"1\" xunits=\"fraction\" yunits=\"fraction\"/><screenXY x=\"1\" y=\"1\" xunits=\"fraction\" yunits=\"fraction\"/>";
    eventKML << "<rotationXY x=\"0\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/><size x=\"0\" y=\"0\" xunits=\"fraction\" yunits=\"fraction\"/></ScreenOverlay>" << std::endl;
    eventKML << "<name>Events By " << group_by << "</name>" << std::endl;
    itrShape = itrShape = _visual_utils.getShapes().begin();
    for (auto const& pgroup: ordered_frequency) {
        const auto& shape = itrShape != _visual_utils.getShapes().end() ? *itrShape : std::string(_visual_utils.getAggregationShape());
        const auto& groupHex = toHex(pgroup.first);
        eventKML << "<Style id=\"events-" << groupHex << "-new-style\"><IconStyle><color>" << toKmlColor("FF0000") << "</color><Icon><href>" << website << "images/events/" << shape << "-whitecenter.png"
            << "</href></Icon><scale>0.5</scale></IconStyle><LabelStyle><scale>0</scale></LabelStyle>" << ballonstyle.str() << "</Style>" << std::endl;
        eventKML << "<StyleMap id=\"events-" << groupHex << "-new-stylemap\"><Pair><key>normal</key><styleUrl>#events-" << groupHex
            << "-new-style</styleUrl></Pair><Pair><key>highlight</key><styleUrl>#events-" << groupHex << "-new-style</styleUrl></Pair></StyleMap>" << std::endl;
        eventKML << "<Style id=\"events-" << groupHex << "-ongoing-style\"><IconStyle><color>" << toKmlColor("971D03") << "</color><Icon><href>" << website << "images/events/" << shape << "-whitecenter.png"
            << "</href></Icon><scale>0.5</scale></IconStyle><LabelStyle><scale>0</scale></LabelStyle>" << ballonstyle.str() << "</Style>" << std::endl;
        eventKML << "<StyleMap id=\"events-" << groupHex << "-ongoing-stylemap\"><Pair><key>normal</key><styleUrl>#events-" << groupHex
            << "-ongoing-style</styleUrl></Pair><Pair><key>highlight</key><styleUrl>#events-" << groupHex << "-ongoing-style</styleUrl></Pair></StyleMap>" << std::endl;
        eventKML << "<Style id=\"events-" << groupHex << "-outside-style\"><IconStyle><color>" << toKmlColor("FFFFFF") << "</color><Icon><href>" << website << "images/events/" << shape << "-whitecenter.png"
            << "</href></Icon><scale>0.5</scale></IconStyle><LabelStyle><scale>0</scale></LabelStyle>" << ballonstyle.str() << "</Style>" << std::endl;
        eventKML << "<StyleMap id=\"events-" << groupHex << "-outside-stylemap\"><Pair><key>normal</key><styleUrl>#events-" << groupHex
            << "-outside-style</styleUrl></Pair><Pair><key>highlight</key><styleUrl>#events-" << groupHex << "-outside-style</styleUrl></Pair></StyleMap>" << std::endl;

        eventKML << "<Folder><name>" << pgroup.first << " (" << pgroup.second << ")</name>" << std::endl << group_placemarks[pgroup.first]->str() << "</Folder>";
        if (itrShape != _visual_utils.getShapes().end()) ++itrShape;
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
            // create locations folder and locations within cluster placemarks
			for (auto itrGroup = _dataHub.getIdentifierInfo().getIdentifiers().begin(); itrGroup != _dataHub.getIdentifierInfo().getIdentifiers().end(); ++itrGroup) {
                for (unsigned int loc = 0; loc < itrGroup->get()->getLocations().size(); ++loc) {
                    if (!_cluster_locations.test(itrGroup->get()->getLocations()[loc]->index())) {
                        itrGroup->get()->getLocations()[loc]->coordinates()->retrieve(vCoordinates);
                        std::pair<double, double> prLatitudeLongitude(ConvertToLatLong(vCoordinates));
                        locationPlacemarks << "\t\t<Placemark>" << (_visibleLocations ? "" : "<visibility>0</visibility>") << "<description></description><styleUrl>";
                        if (_separateLocationsKML) {
                            // If creating separate KML files for locations, styles of primary kml need to be qualified in sub-kml files.
                            locationPlacemarks << _fileCollection.front().getFileName() << _fileCollection.front().getExtension();
                        }
                        locationPlacemarks << "#location-placemark</styleUrl><Point><coordinates>" << prLatitudeLongitude.second << ",";
                        locationPlacemarks << prLatitudeLongitude.first << ",0" << "</coordinates></Point></Placemark>" << std::endl;
                    }
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
                edges << "\t\t\t<Placemark><styleUrl>#line-edge</styleUrl><LineString><coordinates>";
                edges << prLatitudeLongitude.second << "," << prLatitudeLongitude.first << ",0  ";
                prLatitudeLongitude = ConvertToLatLong(connection.get<1>()->coordinates()->retrieve(vCoordinates));
                edges << prLatitudeLongitude.second << "," << prLatitudeLongitude.first << ",0</coordinates></LineString></Placemark>" << std::endl;
            }
            if (_separateLocationsKML) {
                // Create separate kml for this clusters locations, then reference in primary cluster.
                _fileCollection.resize(_fileCollection.size() + 1);
                _fileCollection.back().setFullPath(_dataHub.GetParameters().GetOutputFileName().c_str());
                _fileCollection.back().setFileName("network_edges");
                _fileCollection.back().setExtension(KML_FILE_EXT);

                std::ofstream kmlnetwork;
                kmlnetwork.open(_fileCollection.back().getFullPath(buffer).c_str());
                if (!kmlnetwork) throw resolvable_error("Error: Could not create file '%s'.\n", _fileCollection.back().getFullPath(buffer).c_str());
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
            createKMZ(_fileCollection);
    } catch (prg_exception& x) {
        x.addTrace("finalize()", "ClusterKML");
        throw;
    }
}

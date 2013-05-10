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

        KMLout << "\t<Style id=\"high-rate-style\"><IconStyle><Icon></Icon></IconStyle><LabelStyle><scale>0.5</scale></LabelStyle><LineStyle><color>ff0000aa</color></LineStyle><PolyStyle><color>400000aa</color></PolyStyle></Style>" << std::endl;
        KMLout << "\t<StyleMap id=\"high-rate-stylemap\"><Pair><key>normal</key><styleUrl>#high-rate-style</styleUrl></Pair><Pair><key>highlight</key><styleUrl>#high-rate-style</styleUrl></Pair></StyleMap>" << std::endl;
        KMLout << "\t<Style id=\"low-rate-style\"><IconStyle><Icon></Icon></IconStyle><LabelStyle><scale>0.5</scale></LabelStyle><LineStyle><color>ffff0000</color></LineStyle><PolyStyle><color>40ff0000</color></PolyStyle></Style>" << std::endl;
        KMLout << "\t<StyleMap id=\"low-rate-stylemap\"><Pair><key>normal</key><styleUrl>#low-rate-style</styleUrl></Pair><Pair><key>highlight</key><styleUrl>#low-rate-style</styleUrl></Pair></StyleMap>" << std::endl;
        KMLout << "\t<Style id=\"high-rate-placemark\"><IconStyle><Icon><href>https://maps.google.com/mapfiles/kml/shapes/placemark_circle.png</href><scale>0.25</scale></Icon></IconStyle></Style>" << std::endl;
        KMLout << "\t<Style id=\"low-rate-placemark\"><IconStyle><Icon><href>https://maps.google.com/mapfiles/kml/shapes/placemark_circle.png</href><scale>0.25</scale></Icon></IconStyle></Style>" << std::endl;

        KMLout << std::endl << "\t<name>SaTScan Clusters Detected</name><description>Spatial clusters detected in your analysis.</description>" << std::endl << std::endl;
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

/* Returns a collection of sequential points that define a linear ring around center point. 
   This code was addapted from php source at KML Circle Generator (http://dev.bt23.org/keyhole/circlegen/output.phps)
   and Google Earth Circle Generator (http://www.techidiots.net/autoit-scripts/google-earth-circle-generator/circle%20generator.au3/view) */
ClusterKML::points_t ClusterKML::getPointsOnCircleCircumference(point_t centerPoint, point_t pointOnCircumference) const {
    double EARTH_RADIUS_METERS = 6378137;

    // convert coordinates to radians
    double lat1 = degrees2radians(centerPoint.first);
    double long1 = degrees2radians(centerPoint.second);
    double lat2 = degrees2radians(pointOnCircumference.first);
    double long2 = degrees2radians(pointOnCircumference.second);

    // get the difference between lat/long coords
    double dlat = lat2 - lat1;
    double dlong = long2 - long1;

    // calculate the radius by computing distance of great circle
    double a = std::pow((std::sin(dlat/2.0)), 2) + std::cos(lat1) * std::cos(lat2) * std::pow((std::sin(dlong/2.0)), 2);
    double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
    // get distance between points (in meters)
    double d = EARTH_RADIUS_METERS * c;
    double d_rad = d/EARTH_RADIUS_METERS;

    points_t circlePoints;
    for (int i=0; i <= 360; ++i) {
        double radial = degrees2radians(i);
        double lat_rad = std::asin(std::sin(lat1) * std::cos(d_rad) + std::cos(lat1) * std::sin(d_rad) * std::cos(radial));
        double dlon_rad = std::atan2(std::sin(radial) * std::sin(d_rad) * std::cos(lat1), std::cos(d_rad) - std::sin(lat1) * std::sin(lat_rad));
        double lon_rad = std::fmod((long1 + dlon_rad + PI), 2 * PI) - PI;
        circlePoints.push_back(point_t(radians2degrees(lon_rad), radians2degrees(lat_rad)));
    }
    return circlePoints;
}

void ClusterKML::writeCluster(file_collection_t& fileCollection, std::ofstream& outKML, const CCluster& cluster, int iCluster) const {
    std::string                                legend, locations, clusterCentroidLabel, buffer;
    std::vector<double>                        vCoordinates;
    std::pair<double, double>                  prLatitudeLongitude;
    TractHandler::Location::StringContainer_t  vTractIdentifiers;
    const double radius_km = cluster.GetLatLongRadius();
    bool isHighRate = cluster.getAreaRateForCluster(_dataHub) == HIGH;

    try {
        if (!_dataHub.GetParameters().UseSpecialGrid()) {
            printString(clusterCentroidLabel, "Cluster %d Centroid (%s)", (iCluster + 1), _dataHub.GetTInfo()->getIdentifier(cluster.GetMostCentralLocationIndex()));
        } else {
            printString(clusterCentroidLabel, "Cluster %d Centroid", (iCluster + 1));
        }
        outKML << "\t<Placemark>" << std::endl; 
        outKML << "\t\t<name>" << (iCluster + 1) << "</name>" << std::endl;
        // set popup window text
        getClusterLegend(cluster, iCluster, legend);
        outKML << "\t\t<description>" << legend << "</description>" << std::endl;
        outKML << "\t\t<gx:balloonVisibility>1</gx:balloonVisibility>" << std::endl;
        outKML << "\t\t<styleUrl>#" << (isHighRate ? "high-rate-stylemap" : "low-rate-stylemap") << "</styleUrl>" << std::endl;
        outKML << "\t\t<MultiGeometry>" << std::endl;
        //set focal point of this cluster - cluster centriod
        _dataHub.GetGInfo()->retrieveCoordinates(cluster.GetCentroidIndex(), vCoordinates);
        prLatitudeLongitude = ConvertToLatLong(vCoordinates);
        // get the coordinates of location farthest away from center
        std::vector<double> TractCoords;
        CentroidNeighborCalculator::getTractCoordinates(_dataHub, cluster, _dataHub.GetNeighbor(cluster.GetEllipseOffset(), cluster.GetCentroidIndex(), cluster.GetNumTractsInCluster()), TractCoords);
        std::pair<double, double> pointOnCircumference = ConvertToLatLong(TractCoords);
        if (prLatitudeLongitude == pointOnCircumference) {
            /* If the cluster only has one location, the centroid, then get the nearest neighbor - per Martin.
               This is more difficult than I initially thought. First, I'm not sure he meant nearest point in 
               terms of any point defined. I think he meant in terms of clusters reported in kml. I'm also thinking
               that this includes points on drawn circles -- otherwise if the cluster locations of neighboring cluster
               are on it's farside, then the clusters could overlap.
            */
            // Temporary implementation until we figure out the best course here.  Move north 5 degrees to create a point on circumference.
            pointOnCircumference.first += (0.000278 * 5.0);
        }
        // create boundary circle placemark
        outKML << "\t\t\t<Polygon><outerBoundaryIs><LinearRing><extrude>1</extrude><tessellate>1</tessellate><coordinates>";
        // calculate the points of a linear ring around the cluster and write them to kml file
        ClusterKML::points_t circlePoints = getPointsOnCircleCircumference(prLatitudeLongitude, pointOnCircumference);
        for (ClusterKML::points_t::const_iterator itr=circlePoints.begin(); itr != circlePoints.end(); ++itr) {
            outKML << itr->first << "," << itr->second << ",500 ";
        }
        outKML << "</coordinates></LinearRing></outerBoundaryIs></Polygon>" << std::endl;
        // create centroid placemark
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
                    clusterPlacemarks << "\t<Placemark><name>" <<  vTractIdentifiers[0] << "</name>" << (_visibleLocations ? "" : "<visibility>0</visibility>") 
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
                    outKML << "\t<NetworkLink><name>" << (iCluster + 1) << " Locations</name><visibility>0</visibility><refreshVisibility>0</refreshVisibility><Link><href>"
                           << "cluster" << (iCluster + 1) << "_locations" << KML_FILE_EXT << "</href></Link></NetworkLink>" << std::endl << std::endl;
                } else { // Insert locations into primary kml.
                    outKML << "\t<Folder><name>" << (iCluster + 1) << " Locations</name><description></description>" << std::endl << clusterPlacemarks.str() << "</Folder>" << std::endl;
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

/** Return legend of cluster information to be used as popup in html page. */
std::string & ClusterKML::getClusterLegend(const CCluster& cluster, int iCluster, std::string& legend) const {
    std::stringstream  lines;
    CCluster::ReportCache_t::const_iterator itr=cluster.getReportLinesCache().begin(), itr_end=cluster.getReportLinesCache().end();

    lines << "<![CDATA[" << std::endl << "<table style=\"font-size:12px;\">";
    for (; itr != itr_end; ++itr) {
        lines << "<tr><th style=\"text-align:left;white-space:nowrap;padding-right:5px;\">" << itr->first << "</th><td>" << itr->second << "</td></tr>";
    }
    lines << "</table>" << std::endl << "]]>";

    legend = lines.str();
    std::replace(legend.begin(), legend.end(), '\n', ' ');
    return legend;
}

/** Returns random HTML color. */
std::string& ClusterKML::changeColor(std::string& s, long i, RandomNumberGenerator & rng) {
  static long border = 360, border2 = 270;
  long r, g, b; 
  bool lights, darks;

  // switch base color based upon iteration
  switch (i % 6) {
    case 1 : r=256; g=40; b=40; lights=false; darks=false; break;
    case 2 : r=40; g=256; b=40; lights=false; darks=false; break;
    case 3 : r=40; g=40; b=256; lights=false; darks=false; break;
    case 4 : r=256; g=256; b=256; lights=true; darks=false; break;
    case 5 : r=256; g=256; b=256; lights=false; darks=true; break;
    default : r=256; g=256; b=256; lights=false; darks=false; break;
  }

  long r1 = static_cast<long>(floor((rand()/(float(RAND_MAX)+1)) * r));
  long r2 = static_cast<long>(floor((rand()/(float(RAND_MAX)+1)) * g));
  long r3 = static_cast<long>(floor((rand()/(float(RAND_MAX)+1)) * b));
  long sum = r1 + r2 + r3;
			
  if (lights) {			
    while (sum < border) {				
        long choose = static_cast<long>(floor((rand()/(float(RAND_MAX)+1)) * 3));
        switch (choose) {
            case 0: r1 += (border - sum); break;
            case 1: r3 += (border - sum); break;
            case 2: r2 += (border - sum); break;
        }			
        sum = r1 + r2 + r3;								
    }
  } else if (darks) {
    r1 = std::min(r1, (long)170);
    r2 = std::min(r2, (long)170);
    r3 = std::min(r3, (long)170);						
    while (sum >= border2) {
        long choose = static_cast<long>(floor((rand()/(float(RAND_MAX)+1)) * 3));
        switch (choose) {
            case 0:	r1 = std::max(r1 - 10, (long)1); break;
            case 1: r2 = std::max(r2 - 10, (long)1); break;
            case 2: r3 = std::max(r3 - 10, (long)1); break;
        }				
		sum = r1 + r2 + r3;											
    }			
  }
  printString(s, "%s%x%s%x%s%x", (r1 < 16 ? "0" : ""), r1, (r2 < 16 ? "0" : ""), r2, (r3 < 16 ? "0" : ""), r3);
  return s;
}

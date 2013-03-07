//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ClusterKML.h"
#include "SaTScanData.h"
#include <fstream>
#include "UtilityFunctions.h"
#include "RandomNumberGenerator.h"

const char * ClusterKML::KML_FILE_EXT = ".kml";

/** constructor */
ClusterKML::ClusterKML(const CSaTScanData& dataHub, const MostLikelyClustersContainer& clusters, const SimulationVariables& simVars) 
           :_dataHub(dataHub), _clusters(clusters), _simVars(simVars), _visibleLocations(false), _includeLocations(true) {}

/** Render scatter chart to html page. */
void ClusterKML::renderKML() {
  std::string              buffer, color,legend;
  RandomNumberGenerator    rng;
  double                   gdMinRatioToReport=0.001;
  FileName                 fileName;

  try {
    fileName.setFullPath(_dataHub.GetParameters().GetOutputFileName().c_str());
    fileName.setExtension(KML_FILE_EXT);

    std::ofstream KMLout;
    //open output file
    KMLout.open(fileName.getFullPath(buffer).c_str());
    if (!KMLout) throw resolvable_error("Error: Could not open file '%s'.\n", fileName.getFullPath(buffer).c_str());

    KMLout << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
    KMLout << "<kml xmlns=\"http://www.opengis.net/kml/2.2\">" << std::endl << "<Document>" << std::endl << std::endl;

    // TODO: How to detect that a cluster is high or low? Oberserved vs expected? ... but how about ordinal model?
	KMLout << "<Style id=\"cluster-centriod-high-rate\"><IconStyle><Icon><href>https://maps.google.com/mapfiles/kml/pushpin/red-pushpin.png</href></Icon></IconStyle><LineStyle><color>ff0000aa</color></LineStyle><PolyStyle><color>400000aa</color></PolyStyle></Style>" << std::endl;
    KMLout << "<Style id=\"cluster-boundary-high-rate\"><LineStyle><color>ff0000aa</color><width>2</width></LineStyle><PolyStyle><color>400000aa</color></PolyStyle></Style>" << std::endl;
    KMLout << "<Style id=\"cluster-placemark-high-rate\"><IconStyle><Icon><href>https://maps.google.com/mapfiles/kml/shapes/placemark_circle.png</href><scale>0.25</scale></Icon></IconStyle></Style>" << std::endl;

	KMLout << "<Style id=\"cluster-centriod-low-rate\"><IconStyle><Icon><href>https://maps.google.com/mapfiles/kml/pushpin/blue-pushpin.png</href></Icon></IconStyle><LineStyle><color>ffff0000</color></LineStyle><PolyStyle><color>40ff0000</color></PolyStyle></Style>" << std::endl;
    KMLout << "<Style id=\"cluster-boundary-low-rate\"><LineStyle><color>ffff0000</color><width>2</width></LineStyle><PolyStyle><color>40ff0000</color></PolyStyle></Style>" << std::endl;
    KMLout << "<Style id=\"cluster-placemark-low-rate\"><IconStyle><Icon><href>https://maps.google.com/mapfiles/kml/shapes/placemark_circle.png</href><scale>0.25</scale></Icon></IconStyle></Style>" << std::endl;

    KMLout << "<name>SaTScan Clusters Detected</name><description>Spatial clusters detected in your analysis.</description>" << std::endl << std::endl;

    //if  no replications requested, attempt to display up to top 10 clusters
    tract_t tNumClustersToDisplay(_simVars.get_sim_count() == 0 ? std::min(10, _clusters.GetNumClustersRetained()) : _clusters.GetNumClustersRetained());
    //first iterate through all location coordinates to determine largest X and Y
    for (int i=0; i < _clusters.GetNumClustersRetained(); ++i) {
       //get reference to i'th top cluster
       const CCluster& cluster = _clusters.GetCluster(i);
       //skip purely temporal clusters
       if (cluster.GetClusterType() == PURELYTEMPORALCLUSTER)
           continue;
       if (!(i == 0 || (i < tNumClustersToDisplay && cluster.m_nRatio >= gdMinRatioToReport && (_simVars.get_sim_count() == 0 || cluster.GetRank() <= _simVars.get_sim_count()))))
           break;
       //write cluster details to 'cluster information' file
       if (cluster.m_nRatio >= gdMinRatioToReport) {
           writeCluster(KMLout, cluster, i);
       }
    }
    KMLout << "</Document>" << std::endl << "</kml>" << std::endl;
	KMLout.close();
  } catch (prg_exception& x) {
    x.addTrace("renderKML()","ClusterKML");
    throw;
  }
}

void ClusterKML::writeCluster(std::ofstream& outKML, const CCluster& cluster, int iCluster) const {
  std::string                                legend, locations, clusterCentroidLabel;
  std::vector<double>                        vCoordinates;
  std::pair<double, double>                  prLatitudeLongitude;
  TractHandler::Location::StringContainer_t  vTractIdentifiers;
  double radius = 2 * EARTH_RADIUS_km * asin(cluster.GetCartesianRadius()/(2 * EARTH_RADIUS_km));
  const double GLOBE = 10018.0; //distance from equator to pole in km
  bool isHighRate = cluster.getAreaRateForCluster(_dataHub) == HIGH;

  try {
      if (!_dataHub.GetParameters().UseSpecialGrid()) {
          printString(clusterCentroidLabel, "Cluster %d Centroid (%s)", (iCluster + 1), _dataHub.GetTInfo()->getIdentifier(cluster.GetMostCentralLocationIndex()));
      } else {
          printString(clusterCentroidLabel, "Cluster %d Centroid", (iCluster + 1));
      }
      outKML << "<Folder>" << std::endl; 
      outKML << "<name>Cluster " << (iCluster + 1) << "</name>" << std::endl;
      // set popup window text
      getClusterLegend(cluster, iCluster, legend);
      outKML << "<description>" << legend << "</description>" << std::endl;
      //set focal point of this cluster - cluster centriod
      _dataHub.GetGInfo()->retrieveCoordinates(cluster.GetCentroidIndex(), vCoordinates);
      prLatitudeLongitude = ConvertToLatLong(vCoordinates);
      outKML << "<LookAt><longitude>" << prLatitudeLongitude.second << "</longitude><latitude>" << prLatitudeLongitude.first
             << "</latitude><altitude>1500</altitude><altitudeMode>relativeToGround</altitudeMode></LookAt>" << std::endl;
      // create boundary cirle placemark -- TODO: what about ellipses?
      outKML << "<Placemark><name>Boundary</name><styleUrl>#cluster-boundary-" << (isHighRate ? "high" : "low") << "-rate</styleUrl><Polygon><outerBoundaryIs><LinearRing><coordinates>";
      double radian = prLatitudeLongitude.first/180.0*PI;
      for (int i=0; i <= 360; i += 6) {
        outKML << prLatitudeLongitude.second + radius * 90.0/GLOBE * cos(i/180.0*PI)/cos(radian) << ",";
        outKML << prLatitudeLongitude.first + radius * 90.0/GLOBE * sin(i/180.0*PI) << " ";
      }
      outKML << "</coordinates></LinearRing></outerBoundaryIs></Polygon></Placemark>" << std::endl;
      // create centroid placemark
      outKML << "<Placemark><name>" << clusterCentroidLabel.c_str() <<"</name><description></description><styleUrl>#cluster-centriod-"<< (isHighRate ? "high" : "low") << "-rate</styleUrl>"
             << "<Point><coordinates>" << prLatitudeLongitude.second << "," << prLatitudeLongitude.first << ",0" << "</coordinates></Point></Placemark>" << std::endl;

	  if (_includeLocations) {
		// create locations folder and locations within cluster placemarks
		outKML << "<Folder><name>Locations</name><description></description>" << std::endl;
		for (tract_t t=1; t <= cluster.GetNumTractsInCluster(); ++t) {
			tract_t tTract = _dataHub.GetNeighbor(cluster.GetEllipseOffset(), cluster.GetCentroidIndex(), t, cluster.GetCartesianRadius());
			if (!_dataHub.GetIsNullifiedLocation(tTract)) {
				_dataHub.GetTInfo()->retrieveAllIdentifiers(tTract, vTractIdentifiers);
				CentroidNeighborCalculator::getTractCoordinates(_dataHub, cluster, tTract,vCoordinates);
				prLatitudeLongitude = ConvertToLatLong(vCoordinates);
				outKML << "<Placemark><name>" <<  vTractIdentifiers[0] << "</name>" << (_visibleLocations ? "" : "<visibility>0</visibility>") << "<description></description><styleUrl>#cluster-placemark-" << (isHighRate ? "high" : "low") << "-rate</styleUrl>"
					   << "<Point><coordinates>" << prLatitudeLongitude.second << "," << prLatitudeLongitude.first << ",0"
                       << "</coordinates></Point></Placemark>" << std::endl;
			}
		}
		outKML << "</Folder>" << std::endl;
	  }
      outKML << "</Folder>" << std::endl << std::endl; 
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

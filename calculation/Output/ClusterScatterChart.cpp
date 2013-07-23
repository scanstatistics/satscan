//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ClusterScatterChart.h"
#include "SaTScanData.h"
#include <fstream>
#include "UtilityFunctions.h"
#include "RandomNumberGenerator.h"


/** constructor */
ClusterScatterChart::ClusterScatterChart(const CSaTScanData& dataHub, 
                                         const MostLikelyClustersContainer& clusters, 
                                         const SimulationVariables& simVars) 
                    :_dataHub(dataHub), _clusters(clusters), _simVars(simVars) {}

/** Render scatter chart to html page. */
void ClusterScatterChart::renderScatterChart() {
  double                   gdMinRatioToReport=0.001;
  double                   largestXValue=std::numeric_limits<double>::min(), largestYValue=std::numeric_limits<double>::min();
  double                   smallestXValue=std::numeric_limits<double>::max(), smallestYValue=std::numeric_limits<double>::max();
  std::vector<std::string> chartClusters, chartPoints;
  std::string              color,legend;
  std::vector<double>      vCoordinates;
  RandomNumberGenerator    rng;

  try {
    //if  no replications requested, attempt to display up to top 10 clusters
    tract_t tNumClustersToDisplay(_simVars.get_sim_count() == 0 ? std::min(10, _clusters.GetNumClustersRetained()) : _clusters.GetNumClustersRetained());

    //first iterate through all location coordinates to determine largest X and Y
    for (int i=0; i < _clusters.GetNumClustersRetained(); ++i) {
       //get reference to i'th top cluster
       const CCluster& cluster = _clusters.GetCluster(i);
       if (!(i == 0 || (i < tNumClustersToDisplay && cluster.m_nRatio >= gdMinRatioToReport && (_simVars.get_sim_count() == 0 || cluster.GetRank() <= _simVars.get_sim_count()))))
           break;
       //write cluster details to 'cluster information' file
       if (cluster.m_nRatio >= gdMinRatioToReport) {

         changeColor(color, i, rng);
         getClusterLegend(cluster, i, legend);
         _dataHub.GetGInfo()->retrieveCoordinates(cluster.GetCentroidIndex(), vCoordinates);
         chartClusters.resize(chartClusters.size() + 1);

         if (cluster.GetEllipseOffset() == 0) {
           printString(chartClusters.back(), "myChart.addBubble(%d, %.2lf, %.2lf, %.2lf, '#%s', '%s');", 
               i, //cluster index
               vCoordinates.at(0), // X coordinate
               vCoordinates.at(1), // Y coordinate
               std::max(cluster.GetCartesianRadius(),1.0), // cluster radius -- TODO: what about decimals values?
               color.c_str(), // cluster color
               legend.c_str()); // cluster details for popup
           largestXValue = std::max(largestXValue, vCoordinates.at(0) + cluster.GetCartesianRadius() + 5);
           smallestXValue = std::min(smallestXValue, vCoordinates.at(0) - cluster.GetCartesianRadius() - 5/* left-margin buffer*/);
           largestYValue = std::max(largestYValue, vCoordinates.at(1) + cluster.GetCartesianRadius() + 5);
           smallestYValue = std::min(smallestYValue, vCoordinates.at(1) - cluster.GetCartesianRadius() - 5/* bottom-margin buffer*/);
         } else {
           double semi_major = cluster.GetCartesianRadius() * _dataHub.GetEllipseShape(cluster.GetEllipseOffset());
           double degrees = 180.0 * (_dataHub.GetEllipseAngle(cluster.GetEllipseOffset()) / (double)M_PI);
           degrees = 180.0 - degrees; // invert degrees to lower quadrands for canvas rotate
           printString(chartClusters.back(), "myChart.addEllipticBubble(%d, %.2lf, %.2lf, %.2lf, %.2lf, %.2lf, %.2lf, '#%s', '%s');",
               i, //cluster index
               vCoordinates.at(0), // X coordinate
               vCoordinates.at(1), // Y coordinate
               std::max(cluster.GetCartesianRadius(),1.0), // semi-minor axis -- TODO: what about decimals values?
               std::max(semi_major,1.0), // semi-major axis -- TODO: what about decimals values?
               degrees, // cluster angle
               //cluster.ConvertAngleToDegrees(_dataHub.GetEllipseAngle(cluster.GetEllipseOffset())), // cluster angle
               _dataHub.GetEllipseShape(cluster.GetEllipseOffset()), // cluster shape
               color.c_str(), // cluster color
               legend.c_str()); // cluster details for popup
           // to make it simplier, just pretend that semi-major extends along X and Y axis  
           largestXValue = std::max(largestXValue, vCoordinates.at(0) + semi_major + 5/* left-margin buffer*/);
           smallestXValue = std::min(smallestXValue, vCoordinates.at(0) - semi_major - 5/* left-margin buffer*/);
           largestYValue = std::max(largestYValue, vCoordinates.at(1) + semi_major + 5/* left-margin buffer*/);
           smallestYValue = std::min(smallestYValue, vCoordinates.at(1) - semi_major - 5/* bottom-margin buffer*/);
         }
         for (tract_t t=1; t <= cluster.GetNumTractsInCluster(); ++t) {
             tract_t tTract = _dataHub.GetNeighbor(cluster.GetEllipseOffset(), cluster.GetCentroidIndex(), t, cluster.GetCartesianRadius());
             if (!_dataHub.GetIsNullifiedLocation(tTract)) {
                CentroidNeighborCalculator::getTractCoordinates(_dataHub, cluster, tTract,vCoordinates);
                largestXValue = std::max(largestXValue, vCoordinates.at(0));
                smallestXValue = std::min(smallestXValue, vCoordinates.at(0));
                largestYValue = std::max(largestYValue, vCoordinates.at(1)); 
                smallestYValue = std::min(smallestYValue, vCoordinates.at(1));
                chartPoints.resize(chartPoints.size() + 1);
                printString(chartPoints.back(), "myChart.addPoint(%d, %.2lf, %.2lf, '#%s');", i, vCoordinates.at(0), vCoordinates.at(1), "FFFFFF");
             }
         }
       }
    }

    //for (size_t tt=0; tt < _dataHub.GetTInfo()->getCoordinates().size(); ++tt) {
    //  _dataHub.GetTInfo()->getCoordinates()[tt]->retrieve(vCoordinates);
    //  chartPoints.resize(chartPoints.size() + 1);
    //  printString(chartPoints.back(), "myChart.addPoint(%d, %.2lf, %.2lf, '#%s');", 0, vCoordinates.at(0), vCoordinates.at(1), "000000");
    //}

    std::string src_dir("C:/prj/satscan.development/mootools/");

    std::ofstream HTMLout;
    //open output file
    std::string name(_dataHub.GetParameters().GetOutputFileName());
    name += ".html";
    HTMLout.open(name.c_str());
    if (!HTMLout) throw resolvable_error("Error: Could not open file '%s'.\n", name.c_str());

    // need to keep x,y ranges equal for proper scaling
    double xrange = fabs(ceil(largestXValue) - floor(smallestXValue));
    double yrange = fabs(ceil(largestYValue) - floor(smallestYValue));
    long xmax = static_cast<long>(std::max(xrange,yrange) + smallestXValue);
    long ymax = static_cast<long>(std::max(xrange,yrange) + smallestYValue);

    HTMLout << "<html>\n<head>\n";
    HTMLout << "<script type=\"text/javascript\" src=\"file:///" << src_dir << "mootools-1.2.4-core-nc.js\"></script>\n";
    HTMLout << "<script type=\"text/javascript\" src=\"file:///" << src_dir << "moochart-0.1b1-nc.js\"></script>\n";
    HTMLout << "<script type=\"text/javascript\" src=\"file:///" << src_dir << "mootoolsMore.js\"></script>\n";
    HTMLout << "<script type=\"text/javascript\">\n";
    HTMLout << "    var myChart = null;\n";
    HTMLout << "    window.addEvent('domready', function(){\n";
    HTMLout << "       myChart = new Chart.Bubble('chartContainer', {zmin:" << 0 << ",zmax:" << 1 
        << ",xsteps:" << 10 << ",ysteps:" << 10 << ",xmin:" << floor(smallestXValue) << ",xmax:" << xmax << ",ymin:" << floor(smallestYValue) 
        << ",ymax:" << ymax << ",width:" << 800 << ",height:" << 800 << ",bubbleSize:" << 8
        << ",tipImage:'file:///" << src_dir << "tip-background.png'});\n";
    HTMLout << "       showPoints(document.getElementById('points').checked);\n";
    HTMLout << "    });\n";

    HTMLout << "    function showPoints(show) {\n";
    HTMLout << "       myChart.empty();\n";
    std::vector<std::string>::const_iterator itr=chartClusters.begin(), itr_end=chartClusters.end();
    for (;itr != itr_end; ++itr) {
       HTMLout << "       " << *itr << "\n";
    }
    HTMLout << "       if (show) {\n";
    itr=chartPoints.begin(), itr_end=chartPoints.end();
    for (;itr != itr_end; ++itr) {
       HTMLout << "          " << *itr << "\n";
    }
    HTMLout << "       }\n";
    HTMLout << "       myChart.redraw();\n";
    HTMLout << "    }\n";

    HTMLout << "</script>\n</head>\n";
    HTMLout << "<body style=\"background-color: #f0f8ff;/*background-image: url('file:///" << src_dir << "bg.png'); background-repeat:repeat-x;*/\">\n";
    HTMLout << "<!--[if IE]>\n<div id=\"ie\" style=\"z-index:255;border-top:5px solid #fff;border-bottom:5px solid #fff;background-color:#c00; color:#fff;\">\n";
    HTMLout << "  <div class=\"iewrap\" style=\"border-top:5px solid #e57373;border-bottom:5px solid #e57373;\">\n";
    HTMLout << "     <div class=\"iehead\" style=\"margin: 14px 14px;font-size: 20px;\">Notice to Internet Explorer users!</div>\n";
    HTMLout << "     <div class=\"iebody\" style=\"font-size: 14px;line-height: 14px;margin: 14px 28px;\">It appears that you are using Internet Explorer, <strong>this page may not display correctly with versions 8 or earlier of this browser</strong>.<br /><br />\n";
    HTMLout << "         <i>This page is known to display correctly with the following browsers: Safari 4+, Firefox 3+, Opera 10+ and Google Chrome 5+.</i>\n";
    HTMLout << "     </div>\n  </div>\n</div>\n<![endif]-->\n";
    HTMLout << "<div id='chartContainer' name='chartContainer'></div>\n";
    HTMLout << "<input type=\"checkbox\" id=\"points\" value=\"points\" onclick=\"showPoints(this.checked)\"> Show Points<br>\n";
    HTMLout << "</body>\n</html>\n";
	HTMLout.close();
  } catch (prg_exception& x) {
    x.addTrace("renderScatterChart()","ClusterScatterChart");
    throw;
  }
}

// TODO: How to handle purely temporal output?
/*
      |             ___________
 LLR  |             |         |
      |        _____|_____    |
      |        |         |    |
      |        |         |    | 
      |________|_________|____|__________
       1990  1992  1994  1996  1998  2000     
*/

/** Return legend of cluster information to be used as popup in html page. */
std::string & ClusterScatterChart::getClusterLegend(const CCluster& cluster, int iCluster, std::string& legend) const {
  std::stringstream  lines;
  CCluster::ReportCache_t::const_iterator itr=cluster.getReportLinesCache().begin(), itr_end=cluster.getReportLinesCache().end();

  lines << "Cluster " << iCluster + 1 << "<br>";
  for (; itr != itr_end; ++itr) {
      lines << itr->first << " : " << itr->second.first << "<br>";
  }
  legend = lines.str();
  std::replace(legend.begin(), legend.end(), '\n', ' ');
  return legend;
}

/** Returns random HTML color. */
std::string& ClusterScatterChart::changeColor(std::string& s, long i, RandomNumberGenerator & rng) {

  static const char * firstColors[] = {"ffd700", "DFDF20", "9ACD32", "DF9020", "DF4020", "DF2020", "008000", "008080", "0000ff", "154890"};
  //http://www.colorcombos.com/combotester.html?color0=ffd700&color1=DFDF20&color2=9ACD32&color3=DF9020&color4=DF4020&color5=DF2020&color6=008000&color7=008080%20&color8=0000ff%20&color9=154890

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

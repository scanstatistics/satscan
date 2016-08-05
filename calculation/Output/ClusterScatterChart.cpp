//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ClusterScatterChart.h"
#include "SaTScanData.h"
#include <fstream>
#include "UtilityFunctions.h"
#include "RandomNumberGenerator.h"
#include "Toolkit.h"

const char * CartesianGraph::HTML_FILE_EXT = ".html";
const char * CartesianGraph::FILE_SUFFIX_EXT = ".cluster";

const char * CartesianGraph::TEMPLATE = " \
<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\"> \n \
<html lang=\"en\"> \n \
    <head> \n \
        <title>Cluster Graph</title> \n \
        <meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"> \n \
        <link href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/css/bootstrap.min.css\" rel=\"stylesheet\"> \n \
        <style type=\"text/css\"> \n \
        body {background-color: #f0f8ff;} \n \
        #chartContainer { overflow: hidden; } \n \
        .chart-options{ display:none } \n \
        .chart-options{ padding:10px 0 10px 0; background-color:#e6eef2; border:1px solid silver } \n \
        .options-row{ margin:0 10px 10px 10px } \n \
        .options-row>label:first-child, .options-row detail{ color:#13369f; font-weight:bold } \n \
        .options-row input[type='radio']{ margin:5px } \n \
        p.help-block{ font-size:11px; color:#666; font-style:oblique; margin-top:0; } \n \
        .main-content{ margin: 5px; } \n \
        .options-row label{ font-weight: normal; } \n \
        input[type=checkbox]{ margin-right:5px; } \n \
        label.option-section{ border-bottom: solid 1px #e6e9eb; width: 100% ; } \n \
        .chart-column{ padding-top: 20px; padding-bottom: 30px; border-left: 1px solid #ddd; } \n \
        .print-section a{ padding-right: 20px; text-decoration: none; } \n \
        .cluster-selection{ border-bottom: dashed 1px #e6e9eb; } \n \
        .cluster-selection label{ white-space: nowrap; color: #313030; } \n \
        @media print{ title, #banner,.chart-options-section{ display: none; } #chartContainer{ margin: 20px; }.chart-column{ border-left: 0; } } \n \
        </style> \n \
        <script type='text/javascript' src='--resource-path--javascript/jquery/jquery-1.12.4/jquery-1.12.4.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/clustercharts/jQuery.resizeEnd.js'></script> \n \
        <script type=\"text/javascript\"> \n \
            jQuery.noConflict(); \n \
        </script> \n \
        <script type='text/javascript' src='--resource-path--javascript/clustercharts/mootools-1.6.0/MooTools-Core-1.6.0.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/clustercharts/clusterchart-1.0.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/clustercharts/mootools-1.6.0/MooTools-More-1.6.0.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/clustercharts/FileSaver-2014-06-24.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/clustercharts/Blob-2014-07-24.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/clustercharts/canvas-toBlob-2016-05-26.js'></script> \n \
        <script type='text/javascript' src='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/js/bootstrap.min.js'></script> \n \
        <script type='text/javascript'> \n \
            var chart = null; \n \
            function showGraph() { \n \
               var row = jQuery('.row'); \n \
               var options_div = jQuery('.chart-options-section'); \n \
               var dimension = Math.max(jQuery(row).width() - jQuery(options_div).width() - 100, jQuery(options_div).width() - 50); \n \
               jQuery('.chart-column').html(\"<div id = 'chartContainer' name = 'chartContainer' style = 'margin-left: 20px;'></div>\"); \n \
               chart = new Chart.Bubble('chartContainer', {zmin:--zmin--,zmax:--zmax--,xsteps:--xsteps--,ysteps:--ysteps--,bubbleSize:--bubbleSize--,xmin:--xmin--,xmax:--xmax--,ymin:--ymin--,ymax:--ymax--,width:dimension,height:dimension,title: jQuery('.title-setter').val() || 'Cluster Graph',points_mouseoveronly: document.getElementById('points_mouseover').checked}); \n \
               --chart--bubbles-- \n \
               chart.drawLabels(); \n \
               chart.redraw(); \n \
            } \n \
            window.addEvent('domready', function(){ \n \
               try { \n \
                  showGraph(); \n \
                  jQuery('.cluster-selection input[type=checkbox], #points_mouseover').on('click', function(){ showGraph(); }); \n \
                  jQuery('.title-setter').keyup(function() { showGraph(); }); \n \
                  jQuery(window).resizeend(function() { showGraph(); }); \n \
                  jQuery('#print_png').on('click', function(){ \n \
                     var filename = chart.options.title || \"cluster-graph\"; \n \
                     chart.canvas.toBlob(function(blob) { \n \
                        saveAs(blob, filename + '.png'); \n \
                     }); \n \
                     return false; \n \
                  }); \n \
               } catch (error) { \n \
				   jQuery('#load_error').html('There was a problem loading the graph. Please <a href=\"mailto:--tech-support-email--?Subject=Graph%20Error\" target=\"_top\">email</a> technical support and attach the file:<br/>' + window.location.href.replace('file:///', '').replace(/%20/g, ' ') ).show(); \n \
             	   throw( error ); \n \
                } \n \
            }); \n \
        </script> \n \
    </head> \n \
    <body style=\"margin:0;background-color: #fff;\"> \n \
        <table width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" bgcolor=\"#F8FAFA\" style=\"border-bottom: 3px double navy;\"> \n \
        <tbody><tr> \n \
        <td width=\"120\" align=\"center\" bgcolor=\"#DBD7DB\"><img src=\"--resource-path--images/swe2.jpg\" alt=\"&Ouml;stersund map\" title=\"Östersund map\" width=\"120\" height=\"115\" hspace=\"1\" border=\"0\"></td> \n \
        <td align=\"right\" bgcolor=\"#D4DCE5\"><img src=\"--resource-path--images/satscan_title2.jpg\" alt=\"SaTScan&#0153; - Software for the spatial, temporal, and space-time scan statistics\" title=\"SaTScan&#0153; - Software for the spatial, temporal, and space-time scan statistics\" width=\"470\" height=\"115\"></td> \n \
        <td width=\"25%\" bgcolor=\"#F8FAFA\" align=\"right\"><img src=\"--resource-path--images/nyc2.jpg\" alt=\"New York City map\" title=\"New York City map\" width=\"112\" height=\"115\" hspace=\"1\" border=\"0\" align=\"middle\"></td> \n \
        </tr></tbody></table> \n \
		<div id=\"load_error\" style=\"color:#101010; text-align: center;font-size: 1.2em; padding: 20px;background-color: #ece1e1; border: 1px solid #e49595; display:none;\"></div> \n \
    <div class=\"container-fluid main-content\"> \n \
    <div class=\"row\"> \n \
    <div class=\"col-md-3 chart-options-section\"> \n \
    <fieldset> \n \
    <h3 style=\"font-size: 13px; font-weight: 700;\">Graph Options : </h3> \n \
    <div class=\"options-row\"> \n \
    <label class=\"option-section\" for=\"title_obs\">Title</label> \n \
    <div><input type=\"text\" style=\"width:95%;padding:1px;\" class=\"title-setter\" id=\"title_obs\" value=\"Cluster Graph\"> \n \
    <p class=\"help-block\">Title can be updated by editing this text.</p> \n \
    </div> \n \
    </div> \n \
    <div class=\"options-row\"> \n \
    <label class=\"option-section\" for = \"title_obs\">Print</label> \n \
    <div class=\"print-section\"> \n \
    <a href=\"#\" onclick = \"javascript:window.print();return false;\"><span class=\"glyphicon glyphicon-print\" aria-hidden=\"true\"></span> Print</a> \n \
    <a href=\"#\" id=\"print_png\"><span class=\"glyphicon glyphicon-picture\" aria-hidden = \"true\"></span> Save Image</a> \n \
    </div> \n \
    </div> \n \
    <div class=\"options-row\"> \n \
    <label class=\"option-section\">Clusters</label> \n \
    <p class=\"help-block\">Toggle display of clusters and locations.</p> \n \
    <div class=\"container-fluid\"> \n \
    --cluster-sections-- \n \
    </div> </div> \n \
    <div class=\"options-row\"> \n \
    <label class=\"option-section\">Additional</label> \n \
    <label><input type=\"checkbox\" id=\"points_mouseover\" value=\"points\" />Locations on mouse - over only</label> \n \
    <p class=\"help-block\">Only display cluster locations when mouse over cluster.</p> \n \
    </div> \n \
    </fieldset> \n \
    </div> \n \
    <div class=\"col-md-9 chart-column\"> \n \
    <div id='chartContainer' name='chartContainer'></div> \n \
    </div> \n \
    </div> \n \
    </div> \n \
    </body> \n \
</html> \n";

/** Alters pass Filename to include suffix and extension. */
FileName& CartesianGraph::getFilename(FileName& filename) {
    std::string buffer;
    printString(buffer, "%s%s", filename.getFileName().c_str(), FILE_SUFFIX_EXT);
    filename.setFileName(buffer.c_str());
    filename.setExtension(HTML_FILE_EXT);
    return filename;
}

/** Replaces 'replaceStub' text in passed stringstream 'templateText' with text of 'replaceWith'. */
std::stringstream & CartesianGraph::templateReplace(std::stringstream& templateText, const std::string& replaceStub, const std::string& replaceWith) {
    boost::regex to_be_replaced(replaceStub);
    std::string changed(boost::regex_replace(templateText.str(), to_be_replaced, replaceWith));
    templateText.str(std::string());
    templateText << changed;
    return templateText;
}

/** Return legend of cluster information to be used as popup in html page. */
std::string & CartesianGraph::getClusterLegend(const CCluster& cluster, int iCluster, std::string& legend) const {
    std::stringstream  lines;
    CCluster::ReportCache_t::const_iterator itr = cluster.getReportLinesCache().begin(), itr_end = cluster.getReportLinesCache().end();

    lines << "<div style=\"text-decoration:underline; \">Cluster " << iCluster + 1 << "</div>";
    for (; itr != itr_end; ++itr) {
        lines << itr->first << " : " << itr->second.first << "<br>";
    }
    legend = lines.str();
    std::replace(legend.begin(), legend.end(), '\n', ' ');
    return legend;
}

/** Render scatter chart to html page. */
void CartesianGraph::generateChart() {
    double                   gdMinRatioToReport = 0.001;
    double                   largestXValue = -std::numeric_limits<double>::max(), largestYValue = -std::numeric_limits<double>::max();
    double                   smallestXValue = std::numeric_limits<double>::max(), smallestYValue = std::numeric_limits<double>::max();
    //std::vector<std::string> chartClusters, chartPoints;
    std::string              color, legend;
    std::vector<double>      vCoordinates;
    RandomNumberGenerator    rng;
    std::string              buffer, buffer2;
    std::stringstream        html, cluster_html, cluster_sections, worker;
    FileName fileName;

    try {
        fileName.setFullPath(_dataHub.GetParameters().GetOutputFileName().c_str());
        getFilename(fileName);

        std::ofstream HTMLout;
        //open output file
        HTMLout.open(fileName.getFullPath(buffer).c_str());
        if (!HTMLout) throw resolvable_error("Error: Could not open file '%s'.\n", fileName.getFullPath(buffer).c_str());

        // read template into stringstream
        html << TEMPLATE << std::endl;
        // site resource link path
        templateReplace(html, "--resource-path--", AppToolkit::getToolkit().GetWebSite());
        // site resource link path
        templateReplace(html, "--tech-support-email--", AppToolkit::getToolkit().GetTechnicalSupportEmail());

        //if  no replications requested, attempt to display up to top 10 clusters
        tract_t tNumClustersToDisplay(_simVars.get_sim_count() == 0 ? std::min(10, _clusters.GetNumClustersRetained()) : _clusters.GetNumClustersRetained());

        //first iterate through all location coordinates to determine largest X and Y
        for (int i = 0; i < _clusters.GetNumClustersRetained(); ++i) {
            //get reference to i'th top cluster
            const CCluster& cluster = _clusters.GetCluster(i);
            if (!(i == 0 || (i < tNumClustersToDisplay && cluster.m_nRatio >= gdMinRatioToReport && (_simVars.get_sim_count() == 0 || cluster.GetRank() <= _simVars.get_sim_count()))))
                break;
            //write cluster details to 'cluster information' file
            if (cluster.m_nRatio >= gdMinRatioToReport) {


                color = cluster.getAreaRateForCluster(_dataHub) == HIGH ? "F13C3F" : "5F8EBD";
                //changeColor(color, i, rng);

                getClusterLegend(cluster, i, legend);
                _dataHub.GetGInfo()->retrieveCoordinates(cluster.GetCentroidIndex(), vCoordinates);
                //chartClusters.resize(chartClusters.size() + 1);

                if (cluster.GetEllipseOffset() == 0) {
                    printString(buffer, "chart.addBubble(%d, %.2lf, %.2lf, %.2lf, '#%s', '%s');",
                        i + 1, //cluster index
                        vCoordinates.at(0), // X coordinate
                        vCoordinates.at(1), // Y coordinate
                        cluster.GetCartesianRadius(), // cluster radius
                        color.c_str(), // cluster color
                        legend.c_str()); // cluster details for popup
                    largestXValue = std::max(largestXValue, vCoordinates.at(0) + cluster.GetCartesianRadius() + 1);
                    smallestXValue = std::min(smallestXValue, vCoordinates.at(0) - cluster.GetCartesianRadius() - 1/* left-margin buffer*/);
                    largestYValue = std::max(largestYValue, vCoordinates.at(1) + cluster.GetCartesianRadius() + 1);
                    smallestYValue = std::min(smallestYValue, vCoordinates.at(1) - cluster.GetCartesianRadius() - 1/* bottom-margin buffer*/);
                }
                else {
                    double semi_major = cluster.GetCartesianRadius() * _dataHub.GetEllipseShape(cluster.GetEllipseOffset());
                    double degrees = 180.0 * (_dataHub.GetEllipseAngle(cluster.GetEllipseOffset()) / (double)M_PI);
                    degrees = 180.0 - degrees; // invert degrees to lower quadrands for canvas rotate
                    printString(buffer, "chart.addEllipticBubble(%d, %.2lf, %.2lf, %.2lf, %.2lf, %.2lf, %.2lf, '#%s', '%s');",
                        i + 1, //cluster index
                        vCoordinates.at(0), // X coordinate
                        vCoordinates.at(1), // Y coordinate
                        cluster.GetCartesianRadius(), // semi-minor axis
                        semi_major, // semi-major axis
                        degrees, // cluster angle
                                 //cluster.ConvertAngleToDegrees(_dataHub.GetEllipseAngle(cluster.GetEllipseOffset())), // cluster angle
                        _dataHub.GetEllipseShape(cluster.GetEllipseOffset()), // cluster shape
                        color.c_str(), // cluster color
                        legend.c_str()); // cluster details for popup
                                         // to make it simplier, just pretend that semi-major extends along X and Y axis  
                    largestXValue = std::max(largestXValue, vCoordinates.at(0) + semi_major + 1/* left-margin buffer*/);
                    smallestXValue = std::min(smallestXValue, vCoordinates.at(0) - semi_major - 1/* left-margin buffer*/);
                    largestYValue = std::max(largestYValue, vCoordinates.at(1) + semi_major + 1/* left-margin buffer*/);
                    smallestYValue = std::min(smallestYValue, vCoordinates.at(1) - semi_major - 1/* bottom-margin buffer*/);
                }

                cluster_sections << "<div class=\"row cluster-selection\">"
                    << "<div class=\"col-md-6\"><label><input type=\"checkbox\" id=\"cluster_" << (i + 1) << "\" value=\"cluster_" << (i + 1) << "\" checked=checked />Cluster #" << (i + 1) << "</label></div>"
                    << "<div class=\"col-md-6\"><label><input type=\"checkbox\" id=\"points_" << (i + 1) << "\" value=\"points_" << (i + 1) << "\" checked=checked />#" << (i + 1) << " Locations</label></div>"
                    << "</div>" << std::endl;

                cluster_html << "if (document.getElementById('cluster_" << (i + 1) << "').checked) " << buffer.c_str() << std::endl;
                cluster_html << "if (document.getElementById('points_" << (i + 1) << "').checked) ";
                cluster_html << "chart.addPoints(" << (i + 1) << ",[";

                worker.str("");
                for (tract_t t = 1; t <= cluster.GetNumTractsInCluster(); ++t) {
                    tract_t tTract = _dataHub.GetNeighbor(cluster.GetEllipseOffset(), cluster.GetCentroidIndex(), t, cluster.GetCartesianRadius());
                    if (!_dataHub.GetIsNullifiedLocation(tTract)) {
                        CentroidNeighborCalculator::getTractCoordinates(_dataHub, cluster, tTract, vCoordinates);
                        largestXValue = std::max(largestXValue, vCoordinates.at(0));
                        smallestXValue = std::min(smallestXValue, vCoordinates.at(0));
                        largestYValue = std::max(largestYValue, vCoordinates.at(1));
                        smallestYValue = std::min(smallestYValue, vCoordinates.at(1));
                        //chartPoints.resize(chartPoints.size() + 1);
                        worker << printString(buffer2, "[%.2lf, %.2lf]", vCoordinates.at(0), vCoordinates.at(1)).c_str() << ",";
                    }
                }                
                cluster_html << trimString(worker.str(), ",").c_str() << "], '#FFFFFF');" << std::endl;

            }
        }

        // site resource link path
        templateReplace(html, "--cluster-sections--", cluster_sections.str().c_str());
        templateReplace(html, "--chart--bubbles--", cluster_html.str().c_str());

        // need to keep x,y ranges equal for proper scaling
        double xrange = fabs(ceil(largestXValue) - floor(smallestXValue));
        double yrange = fabs(ceil(largestYValue) - floor(smallestYValue));
        long xmax = static_cast<long>(std::max(xrange, yrange) + smallestXValue);
        long ymax = static_cast<long>(std::max(xrange, yrange) + smallestYValue);

        templateReplace(html, "--bubbleSize--", "8");
        templateReplace(html, "--zmin--", "0");
        templateReplace(html, "--zmax--", "1");
        worker.str(""); worker << floor(smallestXValue);
        templateReplace(html, "--xmin--", worker.str().c_str());
        worker.str(""); worker << xmax;
        templateReplace(html, "--xmax--", worker.str().c_str());
        worker.str(""); worker << floor(smallestYValue);
        templateReplace(html, "--ymin--", worker.str().c_str());
        worker.str(""); worker << ymax;
        templateReplace(html, "--ymax--", worker.str().c_str());

        long xstep = static_cast<long>(std::min(20.0, std::abs(xmax - floor(smallestXValue) + 1.0)));
        worker.str(""); worker << xstep;
        templateReplace(html, "--xsteps--", worker.str().c_str());
        long ystep = static_cast<long>(std::min(20.0, std::abs(ymax - floor(smallestYValue) + 1.0)));
        worker.str(""); worker << ystep;
        templateReplace(html, "--ysteps--", worker.str().c_str());

        HTMLout << html.str() << std::endl;
        HTMLout.close();
    }
    catch (prg_exception& x) {
        x.addTrace("renderScatterChart()", "ClusterScatterChart");
        throw;
    }
}

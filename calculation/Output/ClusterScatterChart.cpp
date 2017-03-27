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
        body {background-color: #f0f8ff;}\n \
        #chartContainer{ overflow: hidden; }\n \
        .chart-options{ display:none; }\n \
        .chart-options{ padding:10px 0 10px 0; background-color:#e6eef2; border:1px solid silver; }\n \
        .options-row{ margin:0 10px 10px 10px }\n \
        .options-row>label:first-child, .options-row detail{ color:#13369f; font-weight:bold; }\n \
        .options-row input[type='radio']{ margin:5px }\n \
        p.help-block{ font-size:11px; color:#666; font-style:oblique; margin-top:0; }\n \
        .main-content{ margin: 5px; }\n \
        .options-row label{ font-weight: normal; }\n \
        input[type=checkbox]{ margin-right:5px; }\n \
        label.option-section{ border-bottom: solid 1px #e6e9eb; width: 100 % ; }\n \
        .chart-column{ padding-top: 20px; padding-bottom: 30px; border-left: 1px solid #ddd; }\n \
        .print-section a{ padding-right: 20px; text-decoration: none; }\n \
        .cluster-selection{ border-bottom: dashed 1px #e6e9eb; }\n \
        .cluster-selection label{ white-space: nowrap; color: #313030; }\n \
        #id_display_count { margin:10px; }\n \
        fieldset { margin-top: 10px; }\n \
        @media print{ title, #banner,.chart-options-section{ display: none; } #chartContainer{ margin: 20px; }.chart-column{ border-left: 0; } }\n \
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
            var clusters = [ \n \
            --cluster-definitions-- \n \
            ]; \n \
            var chart = null; \n \
            var parameters = {--parameters--};\n \
            var cluster_region = {--cluster-region-hash--}; \n \
            var cluster_region_points = [--cluster-region-points--]; \n \
            var entire_region = {--entire-region-hash--}; \n \
            var entire_region_points = [--entire-region-points--]; \n \
            var display_stats = {};\n \
			function inViewport(el) {\n \
                var elH = jQuery(el).outerHeight(), H = jQuery(window).height(), r = jQuery(el)[0].getBoundingClientRect(), t = r.top, b = r.bottom;\n \
                return Math.max(0, t > 0 ? Math.min(elH, H - t) : (b < H ? b : H));\n \
            }\n \
            function showGraph() { \n \
               var region = jQuery('#id_zoom_cluster_region').is(':checked') ? jQuery.extend({}, cluster_region) : jQuery.extend({}, entire_region); \n \
               var row = jQuery('.row'); \n \
               var options_div = jQuery('.chart-options-section'); \n \
               var dimension;\n \
               if (jQuery('#id_fit_graph_viewport').is(':checked'))\n \
                  dimension = Math.max(jQuery(window).height() - jQuery('#id_banner').height(), inViewport('.row')) - 50;\n \
               else\n \
                 dimension = Math.max(jQuery(row).width() - jQuery(options_div).width() - 100, jQuery(options_div).width() - 50);\n \
               jQuery('.chart-column').html(\"<div id = 'chartContainer' name = 'chartContainer' style = 'margin-left: 20px;'></div>\"); \n \
               chart = new Chart.Bubble('chartContainer', {zmin:region.zmin,zmax:region.zmax,xsteps:region.xsteps,ysteps:region.ysteps,bubbleSize:region.bubbleSize,xmin:region.xmin,xmax:region.xmax,ymin:region.ymin,ymax:region.ymax,width:dimension,height:dimension,\n \
                                        title: jQuery('.title-setter').val() || 'Cluster Graph',\n \
                                        points_mouseoveronly: false, \n \
                                        showGrid:document.getElementById('id_show_grid_lines').checked, \n \
                                        showAxes:document.getElementById('id_show_axes').checked});  \n \
                display_stats = {displayed_clusters: 0, displayed_cluster_points: 0, displayed_points: 0};\n \
                jQuery.each(clusters, function(i, c) { \n \
                    var add = true;\n \
                    if (parameters.scanrate == 3) {\n \
                        if (jQuery('#id_view_high').is(':checked')) add &= (c.highrate == true);\n \
                        if (jQuery('#id_view_low').is(':checked')) add &= (c.highrate == false);\n \
                    }\n \
                    if (jQuery('#id_hierarchical').is(':checked') && jQuery('#id_gini').is(':checked')) add &= (c.hierarchical == true && c.gini == true);\n \
                    else if (jQuery('#id_hierarchical').is(':checked')) add &= (c.hierarchical == true);\n \
                    else if (jQuery('#id_gini').is(':checked')) add &= (c.gini == true);\n \
                    else add = false;\n \
                    if (jQuery('#id_view_significant').is(':checked')) add &= (c.significant == true);\n \
                    if (add && jQuery('#id_cluster_circles').is(':checked')) {\n \
                        if (c.ellipse)\n \
                            chart.addEllipticBubble(c.id, c.x, c.y, c.z, c.semimajor, c.angle, c.shape, c.color, c.tip);\n \
                        else\n \
                            chart.addBubble(c.id, c.x, c.y, c.z, c.color, c.tip);\n \
                        display_stats.displayed_clusters++;\n \
                    }\n \
                    if (add && jQuery('#id_cluster_locations').is(':checked')) {\n \
                        chart.addPoints(c.id, c.points, c.pointscolor);\n \
                        display_stats.displayed_cluster_points += c.points.length;\n \
                        display_stats.displayed_points += c.points.length;\n \
                    } else if (jQuery('#id_show_location_points').is(':checked')) {\n \
                        chart.addPoints(0, c.points, '#00001a');\n \
                        display_stats.displayed_points += c.points.length;\n \
                    }\n \
                });\n \
                if (jQuery('#id_show_location_points').is(':checked')) {\n \
                    if (jQuery('#id_zoom_cluster_region').is(':checked')) {\n \
                        chart.addPoints(0, cluster_region_points, '#00001a'); \n \
                        display_stats.displayed_points += cluster_region_points.length; \n \
                    } else {\n \
                        chart.addPoints(0, entire_region_points, '#00001a');\n \
                        display_stats.displayed_points += entire_region_points.length;\n \
                    }\n \
                }\n \
               jQuery('#id_cluster_count').html(display_stats.displayed_clusters); \n \
               jQuery('#id_cluster_point_count').html(display_stats.displayed_cluster_points); \n \
               jQuery('#id_point_count').html(display_stats.displayed_points); \n \
               chart.drawLabels(); \n \
               chart.redraw(); \n \
            } \n \
            window.addEvent('domready', function(){ \n \
               try { \n \
                    if ( clusters.every(function(c) { return c.significant == false; }))\n \
                        jQuery('#id_view_all').prop('checked', true);\n \
                    if ( clusters.every(function(c) { return c.hierarchical == false; }))\n \
                        jQuery('#id_hierarchical').prop('checked', false);\n \
                    if (clusters.every(function(c) { return c.gini == false; }))\n \
                        jQuery('#id_gini').prop('checked', false);\n \
                    jQuery('#id_rates_option').toggle(parameters.scanrate == 3);\n \
                    jQuery('#id_secondary_clusters_option').toggle(parameters.giniscan);\n \
                    showGraph();\n \
                    jQuery('.options-row :input').on('click', function(){ showGraph(); });\n \
                    jQuery('.title-setter').keyup(function() { showGraph(); });\n \
                    jQuery(window).resizeend(function() { showGraph(); });\n \
                    jQuery('#print_png').on('click', function(){\n \
                        var filename = chart.options.title || 'cluster-graph';\n \
                        chart.canvas.toBlob(function(blob) {\n \
                            saveAs(blob, filename + '.png');\n \
                        });\n \
                        return false;\n \
                    });\n \
               } catch (error) { \n \
				   jQuery('#load_error').html('There was a problem loading the graph. Please <a href=\"mailto:--tech-support-email--?Subject=Graph%20Error\" target=\"_top\">email</a> technical support and attach the file:<br/>' + window.location.href.replace('file:///', '').replace(/%20/g, ' ') ).show(); \n \
             	   throw( error ); \n \
                } \n \
            }); \n \
        </script> \n \
    </head> \n \
    <body style=\"margin:0;background-color: #fff;\"> \n \
        <table id=\"id_banner\" width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" bgcolor=\"#F8FAFA\" style=\"border-bottom: 3px double navy;\"> \n \
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
                <div class=\"options-row\"> \n \
                    <label class=\"option-section\" for=\"title_obs\">Title</label> \n \
                    <div>\n \
                        <input type=\"text\" style=\"width:95%;padding:1px;\" class=\"title-setter\" id=\"title_obs\" value=\"Cluster Graph\"> \n \
                        <p class=\"help-block\">Title can be updated by editing this text.</p> \n \
                    </div> \n \
                </div> \n \
                <div class=\"options-row\"> \n \
                    <label class=\"option-section\" for=\"title_obs\">Print</label> \n \
                    <div class=\"print-section\"> \n \
                        <a href=\"#\" onclick=\"javascript:window.print();return false;\"><span class=\"glyphicon glyphicon-print\" aria-hidden=\"true\"></span> Print</a> \n \
                        <a href=\"#\" id=\"print_png\"><span class=\"glyphicon glyphicon-picture\" aria-hidden = \"true\"></span> Save Image</a> \n \
                    </div> \n \
                </div> \n \
                <div class=\"options-row\"> \n \
                    <div id=\"id_significance_option\">\n \
                        <label><input type=\"radio\" name=\"view_significance\" id=\"id_view_significant\" value=\"entire\" checked=checked />Significant clusters</label>\n \
                        <label><input type=\"radio\" name=\"view_significance\" id=\"id_view_all\" value=\"cluster\" />All clusters</label>\n \
                        <p class=\"help-block\">Toggle display between significant and all clusters.</p>\n \
                    </div>\n \
                    <div id=\"id_rates_option\"> \n \
                        <label><input type=\"radio\" name=\"view_rate\" id=\"id_view_highlow\" value=\"entire\" checked=checked />High and low clusters</label>\n \
                        <label><input type=\"radio\" name=\"view_rate\" id=\"id_view_high\" value=\"cluster\"/>High only</label>\n \
                        <label><input type=\"radio\" name=\"view_rate\" id=\"id_view_low\" value=\"cluster\"/>Low only</label>\n \
                        <p class=\"help-block\">Toggle display of clusters for scan rate.</p>\n \
                    </div> \n \
                    <div id=\"id_secondary_clusters_option\"> \n \
                        <div>Secondary Clusters:</div>\n \
                        <label style=\"margin-left:15px;\"><input type=\"checkbox\" id=\"id_hierarchical\" value=\"secondary\" checked=checked />Hierarchical</label>\n \
                        <label style=\"margin-left:15px;\"><input type=\"checkbox\" id=\"id_gini\" value=\"secondary\" checked=checked />Gini</label>\n \
                        <p class=\"help-block\">Display options for secondary clusters.</p>\n \
                    </div> \n \
                    <div>Show clusters using:</div>\n \
                    <label style=\"margin-left:15px;\"><input type=\"checkbox\" id=\"id_cluster_circles\" value=\"cluster\" checked=checked />Circles / Ellipses</label>\n \
                    <label style=\"margin-left:15px;\"><input type=\"checkbox\" id=\"id_cluster_locations\" value=\"cluster\" />Locations</label>\n \
                    <p class=\"help-block\">Display options for clusters.</p>\n \
                    <div id=\"id_zoom_cluster_option\" style=\"display:--display-zoom-cluster--;\">\n \
                    <label><input type=\"checkbox\" id=\"id_zoom_cluster_region\"/>Zoom in on cluster region</label>\n \
                    <p class=\"help-block\">Focus grid region on reported clusters.</p>\n \
                    </div>\n \
                    <label><input type=\"checkbox\" id=\"id_show_grid_lines\" checked=checked />Show grid lines</label>\n \
                    <p class=\"help-block\">Toggle display of graph grid lines.</p>\n \
                    <label><input type=\"checkbox\" id=\"id_show_axes\" checked=checked />Show x and y axes</label>\n \
                    <p class=\"help-block\">Toggle display of graph x / y axes.</p>\n \
                    <label><input type=\"checkbox\" id=\"id_show_location_points\" checked=checked />Show location points</label>\n \
                    <p class=\"help-block\">Toggle display of location points.</p>\n \
                    <label><input type=\"checkbox\" id=\"id_fit_graph_viewport\" />Fit graph to viewport</label>\n \
                    <p class=\"help-block\">Attempts to keep entire graph in view.</p>\n \
                </div> \n \
                <div id=\"id_display_count\">\n \
                    <fieldset>\n \
                            <legend style=\"font-size:14px; margin-bottom:0;\">Display Data:</legend>\n \
                            <div><span id=\"id_cluster_count\"></span> Clusters</div>\n \
                            <div><span id=\"id_cluster_point_count\"></span> Cluster Locations</div>\n \
                            <div><span id=\"id_point_count\"></span> Total Locations</div> \n \
                    </fieldset>\n \
                </div>\n \
                </fieldset> \n \
                <div style=\"font-style:italic; font-size:smaller;\">Generated with SaTScan v--satscan-version--</div>\n \
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
    RegionSettings           clusterRegion, entireRegion;
    std::string              buffer, buffer2, legend, clusterColor, pointsColor;
    std::vector<double>      vCoordinates;
    std::stringstream        html, cluster_html, cluster_sections, worker, worker2, cluster_definitions;
    FileName fileName;
    std::vector<tract_t>     clusterLocations;

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
        for (int i=0; i < _clusters.GetNumClustersRetained(); ++i) {
            //get reference to i'th top cluster
            const CCluster& cluster = _clusters.GetCluster(i);
            if (!(i == 0 || (i < tNumClustersToDisplay && cluster.m_nRatio >= gdMinRatioToReport && (_simVars.get_sim_count() == 0 || cluster.GetRank() <= _simVars.get_sim_count()))))
                break;
            //write cluster details to 'cluster information' file
            if (cluster.m_nRatio >= gdMinRatioToReport) {
                clusterColor = cluster.getAreaRateForCluster(_dataHub) == HIGH ? "#F13C3F" : "#5F8EBD";
                pointsColor = cluster.getAreaRateForCluster(_dataHub) == HIGH ? "#FF1A1A" : "#1AC6FF";
                getClusterLegend(cluster, i, legend);
                worker.str("");
                for (tract_t t = 1; t <= cluster.GetNumTractsInCluster(); ++t) {
                    tract_t tTract = _dataHub.GetNeighbor(cluster.GetEllipseOffset(), cluster.GetCentroidIndex(), t, cluster.GetCartesianRadius());
                    if (tTract < _dataHub.GetNumTracts()) {// is tract atomic?
                        if (!_dataHub.GetIsNullifiedLocation(tTract)) {
                            CentroidNeighborCalculator::getTractCoordinates(_dataHub, cluster, tTract, vCoordinates);
                            worker << printString(buffer2, "[%f, %f],", vCoordinates.at(0), vCoordinates.at(1)).c_str();
                            clusterLocations.push_back(tTract);
                        }
                    } else {
                        std::vector<tract_t> indexes;
                        _dataHub.GetTInfo()->getMetaManagerProxy().getIndexes(tTract - _dataHub.GetNumTracts(), indexes);
                        for (std::vector<tract_t>::const_iterator itr = indexes.begin(); itr != indexes.end(); ++itr) {
                            if (!_dataHub.GetIsNullifiedLocation(*itr)) {
                                CentroidNeighborCalculator::getTractCoordinates(_dataHub, cluster, *itr, vCoordinates);
                                worker << printString(buffer2, "[%f, %f],", vCoordinates.at(0), vCoordinates.at(1)).c_str();
                                clusterLocations.push_back(tTract);
                            }
                        }
                    }
                }

                std::string points = worker.str();
                trimString(points, ",");
                const char * cluster_def_format = "x : %f, y : %f, z : %f, semimajor : %f, angle : %.2lf, shape : %.2lf";
                _dataHub.GetGInfo()->retrieveCoordinates(cluster.GetCentroidIndex(), vCoordinates);
                if (cluster.GetEllipseOffset() == 0) {
                    printString(buffer, cluster_def_format, vCoordinates.at(0), vCoordinates.at(1), cluster.GetCartesianRadius(), 0.0, 0.0, 0.0);
                    clusterRegion._largestX = std::max(clusterRegion._largestX, vCoordinates.at(0) + cluster.GetCartesianRadius() + 0.25);
                    clusterRegion._smallestX = std::min(clusterRegion._smallestX, vCoordinates.at(0) - cluster.GetCartesianRadius() - 0.25/* left-margin buffer*/);
                    clusterRegion._largestY = std::max(clusterRegion._largestY, vCoordinates.at(1) + cluster.GetCartesianRadius() + 0.25);
                    clusterRegion._smallestY = std::min(clusterRegion._smallestY, vCoordinates.at(1) - cluster.GetCartesianRadius() - 0.25/* bottom-margin buffer*/);
                } else {
                    double semi_major = cluster.GetCartesianRadius() * _dataHub.GetEllipseShape(cluster.GetEllipseOffset());
                    double degrees = 180.0 * (_dataHub.GetEllipseAngle(cluster.GetEllipseOffset()) / (double)M_PI);
                    degrees = 180.0 - degrees; // invert degrees to lower quadrands for canvas rotate
                    printString(buffer, cluster_def_format, vCoordinates.at(0), vCoordinates.at(1), cluster.GetCartesianRadius(), semi_major, degrees, _dataHub.GetEllipseShape(cluster.GetEllipseOffset()));
                    clusterRegion._largestX = std::max(clusterRegion._largestX, vCoordinates.at(0) + semi_major + 0.25/* left-margin buffer*/);
                    clusterRegion._smallestX = std::min(clusterRegion._smallestX, vCoordinates.at(0) - semi_major - 0.25/* left-margin buffer*/);
                    clusterRegion._largestY = std::max(clusterRegion._largestY, vCoordinates.at(1) + semi_major + 0.25/* left-margin buffer*/);
                    clusterRegion._smallestY = std::min(clusterRegion._smallestY, vCoordinates.at(1) - semi_major - 0.25/* bottom-margin buffer*/);
                }
                cluster_definitions << "{ id: " << (i + 1) << ", significant : " << (cluster.isSignificant(_dataHub, i, _simVars) ? "true" : "false")
                                    << ", highrate : " << (cluster.getAreaRateForCluster(_dataHub) == HIGH ? "true" : "false")
                                    << ", hierarchical : " << (cluster.isHierarchicalCluster() ? "true" : "false") << ", gini : " << (cluster.isGiniCluster() ? "true" : "false")
                                    << ", ellipse : " << (cluster.GetEllipseOffset() != 0 ? "true" : "false") << ", " << buffer
                                    << ", color : '" << clusterColor.c_str() << "', pointscolor : '" << pointsColor.c_str() << "', tip : '" << legend.c_str() << "', points : [" << points << "] },\n";
            }
        }

        // Update cluster region to keep x and y ranges equal -- for proper scaling in graph
        clusterRegion.setproportional();
        entireRegion = clusterRegion;
        std::sort(clusterLocations.begin(), clusterLocations.end());
        std::stringstream cluster_region_points, entire_region_points;
        const TractHandler::LocationsContainer_t & locations = _dataHub.GetTInfo()->getLocations();
        worker.str("");
        worker2.str("");
        for (size_t t = 0; t < locations.size(); ++t) {
            std::vector<tract_t>::iterator itr = std::lower_bound(clusterLocations.begin(), clusterLocations.end(), t);
            if (itr == clusterLocations.end() || *itr != t) {
                double * p = locations[t]->getCoordinates()[locations[t]->getCoordinates().size() - 1]->getCoordinates();
                if (clusterRegion.in(p[0], p[1])) {
                    worker2 << printString(buffer2, "[%f, %f],", p[0], p[1]).c_str();
                }
                worker << printString(buffer2, "[%f, %f],", p[0], p[1]).c_str();
                entireRegion._largestX = std::max(entireRegion._largestX, p[0]);
                entireRegion._smallestX = std::min(entireRegion._smallestX, p[0]);
                entireRegion._largestY = std::max(entireRegion._largestY, p[1]);
                entireRegion._smallestY = std::min(entireRegion._smallestY, p[1]);
            }
        }
        std::string entire_points = worker.str();
        std::string cluster_points = worker2.str();
        // site resource link path
        buffer = cluster_definitions.str();
        templateReplace(html, "--cluster-definitions--", trimString(buffer, ",\n").c_str());
        templateReplace(html, "--entire-region-points--", trimString(trimString(entire_points, ","), ",").c_str());
        templateReplace(html, "--cluster-region-points--", trimString(trimString(cluster_points, ","), ",").c_str());

        // TODO: try to get clusterRegion and entireRegion to better numbers/ticks in graph

        // replace parameters hash
        printString(buffer, "scanrate:%d/*high=1,low=2,highorlow=3*/,giniscan:%s", _dataHub.GetParameters().GetAreaScanRateType(),(_dataHub.GetParameters().getReportGiniOptimizedClusters() ? "true": "false"));
        templateReplace(html, "--parameters--", buffer.c_str());

        // replace region hashes in template
        const char * region_hash = "zmin:0,zmax:1,xsteps:%d,ysteps:%d,bubbleSize:8,xmin:%f,xmax:%f,ymin:%f,ymax:%f";
        long xstep = static_cast<long>(std::min(20.0, std::abs(clusterRegion._largestX - clusterRegion._smallestX + 1.0)));
        long ystep = static_cast<long>(std::min(20.0, std::abs(clusterRegion._largestY - clusterRegion._smallestY + 1.0)));
        printString(buffer, region_hash, xstep, ystep, clusterRegion._smallestX, clusterRegion._largestX, clusterRegion._smallestY, clusterRegion._largestY);
        templateReplace(html, "--cluster-region-hash--", buffer.c_str());
        entireRegion.setproportional(); // need to keep x,y ranges equal for proper scaling
        xstep = static_cast<long>(std::min(20.0, std::abs(entireRegion._largestX - floor(entireRegion._smallestX) + 1.0)));
        ystep = static_cast<long>(std::min(20.0, std::abs(entireRegion._largestY - floor(entireRegion._smallestY) + 1.0)));
        printString(buffer, region_hash, xstep, ystep, entireRegion._smallestX, entireRegion._largestX, entireRegion._smallestY, entireRegion._largestY);
        templateReplace(html, "--entire-region-hash--", buffer.c_str());

        // hide the option to switch between entire region and cluster region if cluster region is greater than 90% of entire region.
        double cluster_region_area = clusterRegion.area();
        double entire_region_area = entireRegion.area();
        templateReplace(html, "--display-zoom-cluster--", ((cluster_region_area / entire_region_area) < 0.9) ? "block" : "none");

        templateReplace(html, "--satscan-version--", AppToolkit::getToolkit().GetVersion());

        HTMLout << html.str() << std::endl;
        HTMLout.close();
    } catch (prg_exception& x) {
        x.addTrace("renderScatterChart()", "ClusterScatterChart");
        throw;
    }
}

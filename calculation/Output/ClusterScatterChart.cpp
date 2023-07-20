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
#include "GisUtils.h"

const char * CartesianGraph::HTML_FILE_EXT = ".html";
const char * CartesianGraph::FILE_SUFFIX_EXT = ".cluster";

const char * CartesianGraph::TEMPLATE = " \
<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\"> \n \
<html lang=\"en\"> \n \
    <head> \n \
        <title>Cartesian Coordinates Map</title> \n \
        <meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"> \n \
        <link href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/css/bootstrap.min.css\" rel=\"stylesheet\"> \n \
        <style type=\"text/css\"> \n \
        body {background-color: #f0f8ff;}\n \
        #chartContainer{ overflow: hidden; }\n \
        .chart-options{ display:none; }\n \
        .chart-options{ padding:10px 0 10px 0; background-color:#e6eef2; border:1px solid silver; }\n \
        .options-row{ margin:0 5px 5px 5px }\n \
        .options-row>label:first-child, .options-row detail{ color:#13369f; font-weight:bold; }\n \
        .options-row input[type='radio']{ margin:5px }\n \
        p.help-block{ font-size:11px; color:#666; font-style:oblique; margin-top:0; margin-bottom:1px;}\n \
        .main-content{ margin: 5px; }\n \
        .options-row label{ font-weight: normal; margin-bottom: 1px;}\n \
        input[type=checkbox]{ margin-right:5px; }\n \
        label.option-section{ border-bottom: solid 1px #e6e9eb; width: 100 % ; }\n \
        .chart-column{ padding-top: 20px; padding-bottom: 30px; border-left: 1px solid #ddd; }\n \
        .print-section a{ padding-right: 20px; text-decoration: none; }\n \
        .cluster-selection{ border-bottom: dashed 1px #e6e9eb; }\n \
        .cluster-selection label{ white-space: nowrap; color: #313030; }\n \
        #id_display_count { margin:10px; }\n \
        /*fieldset { margin-top: 10px; }*/ \n \
        @media print{ title, #banner,.chart-options-section{ display: none; } #chartContainer{ margin: 20px; }.chart-column{ border-left: 0; } }\n \
        </style> \n \
        <script type='text/javascript' src='--resource-path--javascript/jquery/jquery-1.12.4/jquery-1.12.4.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/clustercharts/jQuery.resizeEnd.js'></script> \n \
        <script type=\"text/javascript\"> \n \
            jQuery.noConflict(); \n \
        </script> \n \
        <script type='text/javascript' src='--resource-path--javascript/clustercharts/mootools-1.6.0/MooTools-Core-1.6.0.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/clustercharts/clusterchart-1.3.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/clustercharts/mootools-1.6.0/MooTools-More-1.6.0.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/clustercharts/FileSaver-2014-06-24.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/clustercharts/Blob-2014-07-24.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/clustercharts/canvas-toBlob-2016-05-26.js'></script> \n \
        <script type='text/javascript' src='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/js/bootstrap.min.js'></script> \n \
        <script type='text/javascript'> \n"
"          var clusters = [ \n \
            --cluster-definitions-- \n \
            ]; \n \
            var chart = null; \n \
            var parameters = {--parameters--};\n \
            var cluster_region = {--cluster-region-hash--}; \n \
            var cluster_region_points = [--cluster-region-points--]; \n \
            var entire_region = {--entire-region-hash--}; \n \
            var entire_region_points = [--entire-region-points--]; \n \
            var entire_region_edges = [--entire-region-edges--]; \n \
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
                                        title: jQuery('.title-setter').val() || 'Cartesian Coordinates Map',\n \
                                        points_mouseoveronly: false, \n \
                                        x_sinusoidal_factor: --x_sinusoidal_factor--, \n \
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
                    else add &= true;\n \
                    if (jQuery('#id_view_significant').is(':checked')) add &= (c.significant == true);\n \
                    if (add && jQuery('#id_cluster_circles').is(':checked')) {\n \
                        if (c.ellipse)\n \
                            chart.addEllipticBubble(c.id, c.x, c.y, c.z, c.semimajor, c.angle, c.shape, c.color, c.tip);\n \
                        else\n \
                            chart.addBubble(c.id, c.x, c.y, c.z, c.color, c.tip, c.edges);\n \
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
                if (jQuery('#id_show_all_edges').is(':checked')) { chart.addEdges(entire_region_edges, '#F39C12'); } \n \
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
                    //if ( clusters.every(function(c) { return c.hierarchical == false; }))\n \
                    //    jQuery('#id_hierarchical').prop('checked', false);\n \
                    //if (clusters.every(function(c) { return c.gini == false; }))\n \
                    //    jQuery('#id_gini').prop('checked', false);\n \
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
                    jQuery('#id_show_all_edges').on('click', function(){ showGraph(); }); \n \
                    if (entire_region_edges.length == 0) { jQuery('#id_show_all_edges').parent().hide(); jQuery('#id_show_all_edges').parent().next('p.help-block').hide(); } \n \
               } catch (error) { \n \
				   jQuery('#load_error').html('There was a problem loading the graph. Please <a href=\"mailto:--tech-support-email--?Subject=Graph%20Error\" target=\"_top\">email</a> technical support and attach the file:<br/>' + window.location.href.replace('file:///', '').replace(/%20/g, ' ') ).show(); \n \
             	   throw( error ); \n \
                } \n \
            }); \n \
        </script> \n \
    </head> \n \
    <body style=\"margin:0;background-color: #fff;\"> \n \
		<div id=\"load_error\" style=\"color:#101010; text-align: center;font-size: 1.2em; padding: 20px;background-color: #ece1e1; border: 1px solid #e49595; display:none;\"></div> \n \
    <div class=\"container-fluid main-content\"> \n \
        <div class=\"row\"> \n \
            <div class=\"col-md-3 chart-options-section\"> \n \
                <fieldset> \n \
                <div class=\"options-row\"> \n \
                    <div style='font-style:italic;'>Generated with SaTScan v--satscan-version--</div>\n \
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
                        <label style=\"margin-left:15px;\"><input type=\"checkbox\" id=\"id_hierarchical\" value=\"secondary\" />Hierarchical</label>\n \
                        <label style=\"margin-left:15px;\"><input type=\"checkbox\" id=\"id_gini\" value=\"secondary\" />Gini</label>\n \
                        <p class=\"help-block\">Display options for secondary clusters.</p>\n \
                    </div> \n \
                    <div>Show clusters using:</div>\n \
                    <label style=\"margin-left:15px;\"><input type=\"checkbox\" id=\"id_cluster_circles\" value=\"cluster\" checked=checked />--cluster-input-label--</label>\n \
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
                    <label><input type=\"checkbox\" id=\"id_show_location_points\" />Show all location points</label>\n \
                    <p class=\"help-block\">Toggle display of location points.</p>\n \
                    <label><input type=\"checkbox\" id=\"id_show_all_edges\" />Show all edges</label> \n \
                    <p class=\"help-block\">Toggle display of all edges.</p> \n \
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
                <div class=\"options-row\"> \n \
                    <label class='option-section' for='title_obs' style='display:none;'>Title</label> \n \
                    <div>\n \
                        <input type=\"text\" style=\"width:95%;padding:1px;\" class=\"title-setter\" id=\"title_obs\" value=\"Cartesian Coordinates Map\"> \n \
                        <p class=\"help-block\">Title can be changed by editing this text.</p> \n \
                    </div> \n \
                </div> \n \
                <div class=\"options-row\"> \n \
                    <label class='option-section' for='title_obs' style='display:none;'>Print</label> \n \
                    <div class=\"print-section\"> \n \
                        <a href=\"#\" onclick=\"javascript:window.print();return false;\"><span class=\"glyphicon glyphicon-print\" aria-hidden=\"true\"></span> Print</a> \n \
                        <a href=\"#\" id=\"print_png\"><span class=\"glyphicon glyphicon-picture\" aria-hidden = \"true\"></span> Save Image</a> \n \
                    </div> \n \
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

CartesianGraph::CartesianGraph(const CSaTScanData& dataHub) : _dataHub(dataHub), _clusters_written(0), _median_parallel(0.0) {
    _cluster_locations.resize(_dataHub.GetGroupInfo().getLocationsManager().locations().size());
    if (_dataHub.GetParameters().GetCoordinatesType() == LATLON) {
        // Calculate the median parallel among all points.
        std::vector<double> parallels, vCoordinates;
		for (auto itrGroup = _dataHub.GetGroupInfo().getObservationGroups().begin(); itrGroup != _dataHub.GetGroupInfo().getObservationGroups().end(); ++itrGroup) {
            for (unsigned int loc = 0; loc < itrGroup->get()->getLocations().size(); ++loc) {
                itrGroup->get()->getLocations()[loc]->coordinates()->retrieve(vCoordinates);
                std::pair<double, double> latlong = ConvertToLatLong(vCoordinates);
                parallels.push_back(latlong.first);
            }
		}
        std::sort(parallels.begin(), parallels.end());
        if (parallels.size() % 2 == 0)
            _median_parallel = (parallels[parallels.size() / 2 - 1] + parallels[parallels.size() / 2]) / 2;
        else
            _median_parallel = parallels[parallels.size() / 2];
    }
}

/** Alters pass Filename to include suffix and extension. */
FileName& CartesianGraph::getFilename(FileName& filename) {
    std::string buffer;
    printString(buffer, "%s%s", filename.getFileName().c_str(), FILE_SUFFIX_EXT);
    filename.setFileName(buffer.c_str());
    filename.setExtension(HTML_FILE_EXT);
    return filename;
}

/** Return legend of cluster information to be used as popup in html page. */
std::string & CartesianGraph::getClusterLegend(const CCluster& cluster, int iCluster, std::string& legend) const {
    std::stringstream  lines;
    CCluster::ReportCache_t::const_iterator itr = cluster.getReportLinesCache().begin(), itr_end = cluster.getReportLinesCache().end();
    unsigned int currSetIdx = std::numeric_limits<unsigned int>::max(), numFilesSets = _dataHub.GetParameters().getNumFileSets();

    lines << "<div style=\"text-decoration:underline; \">Cluster " << iCluster + 1 << "</div>";
    for (; itr != itr_end; ++itr) {
        if (numFilesSets > 1 && itr->second.second > 0 && currSetIdx != itr->second.second) {
            lines << "Data Set " << itr->second.second << "<br>";
            currSetIdx = itr->second.second;
        }
        lines << itr->first << " : " << itr->second.first << "<br>";
    }
    legend = lines.str();
    std::replace(legend.begin(), legend.end(), '\n', ' ');
    return legend;
}

/* If the coordinates system in settings is latitude/longitude, transform coordinates back to latitude/longitude. */
std::vector<double>& CartesianGraph::transform(std::vector<double>& vCoordinates) {
    if (_dataHub.GetParameters().GetCoordinatesType() == LATLON) {
        std::pair<double, double> latlong = ConvertToLatLong(vCoordinates);
        vCoordinates.clear();
        vCoordinates.push_back(latlong.second);
        vCoordinates.push_back(latlong.first);
    }
    return vCoordinates;
}

void CartesianGraph::add(const MostLikelyClustersContainer& clusters, const SimulationVariables& simVars) {
    double gdMinRatioToReport = 0.001, radius = 0.0;
    std::vector<double> vCoordinates;
    std::string buffer, buffer2, legend, points, edges;
    std::stringstream  worker;
    unsigned int clusterOffset = _clusters_written;

    //if  no replications requested, attempt to display up to top 10 clusters
    tract_t tNumClustersToDisplay(simVars.get_sim_count() == 0 ? std::min(10, clusters.GetNumClustersRetained()) : clusters.GetNumClustersRetained());
    //first iterate through all location coordinates to determine largest X and Y
    for (int i=0; i < clusters.GetNumClustersRetained(); ++i) {
        //get reference to i'th top cluster
        const CCluster& cluster = clusters.GetCluster(i);
        //skip purely temporal clusters
        if (cluster.GetClusterType() == PURELYTEMPORALCLUSTER)
            continue;
        if (!(i == 0 || (i < tNumClustersToDisplay && cluster.m_nRatio >= gdMinRatioToReport && (simVars.get_sim_count() == 0 || cluster.GetRank() <= simVars.get_sim_count()))))
            break;
        std::vector<double> clusterCenterCoordinates;
        //write cluster details to 'cluster information' file
        if (cluster.m_nRatio >= gdMinRatioToReport) {
            NetworkLocationContainer_t networkLocations;
            if (_dataHub.GetParameters().getUseLocationsNetworkFile()) {
                // Create collections of connections between the cluster nodes - we'll use them to create edges in display.
                _dataHub.getClusterNetworkLocations(cluster, networkLocations);
                Network::Connection_Details_t connections = GisUtils::getClusterConnections(networkLocations);
                worker.str("");
                for (auto itr = connections.begin(); itr != connections.end(); ++itr) {
                    itr->get<0>()->coordinates()->retrieve(vCoordinates);
                    transform(vCoordinates);
                    worker << printString(buffer2, "[[%f, %f],", vCoordinates.at(0), vCoordinates.at(1)).c_str();
                    itr->get<1>()->coordinates()->retrieve(vCoordinates);
                    transform(vCoordinates);
                    worker << printString(buffer2, "[%f, %f]],", vCoordinates.at(0), vCoordinates.at(1)).c_str();
                }
                edges = worker.str();
                trimString(edges, ",");
            }
            // Compile collection of cluster locations.
            worker.str("");
            auto locations = _dataHub.GetGroupInfo().getLocationsManager().locations();
            std::vector<tract_t> vLocations;
            CentroidNeighborCalculator::getLocationsAboutCluster(_dataHub, cluster, 0, &vLocations);
            for (auto index: vLocations) {
                locations[index].get()->coordinates()->retrieve(vCoordinates);
                transform(vCoordinates);
                if (clusterCenterCoordinates.empty()) clusterCenterCoordinates = vCoordinates;
                worker << printString(buffer2, "[%f, %f],", vCoordinates.at(0), vCoordinates.at(1)).c_str();
                _cluster_locations.set(index);
            }
            points = worker.str();
            trimString(points, ",");
            if (_dataHub.GetParameters().getUseLocationsNetworkFile())
                radius = 0.0;
            else if (_dataHub.GetParameters().GetCoordinatesType() == LATLON) // Calculate the radius from centroid to outer most location in cluster.
                radius = Coordinates::distanceBetween(clusterCenterCoordinates, vCoordinates);
            else
                radius = cluster.GetCartesianRadius();
            // Add cluster definition to javascript hash collection.
            getClusterLegend(cluster, i + clusterOffset, legend);
            const char * cluster_def_format = "x : %f, y : %f, z : %f, semimajor : %f, angle : %.2lf, shape : %.2lf";
            _dataHub.GetGInfo()->retrieveCoordinates(cluster.GetCentroidIndex(), vCoordinates);
            transform(vCoordinates);
            if (cluster.GetEllipseOffset() == 0) {
                printString(buffer, cluster_def_format, vCoordinates.at(0), vCoordinates.at(1), radius, 0.0, 0.0, 0.0);
                _clusterRegion._largestX = std::max(_clusterRegion._largestX, vCoordinates.at(0) + radius + 0.25);
                _clusterRegion._smallestX = std::min(_clusterRegion._smallestX, vCoordinates.at(0) - radius - 0.25/* left-margin buffer*/);
                _clusterRegion._largestY = std::max(_clusterRegion._largestY, vCoordinates.at(1) + radius + 0.25);
                _clusterRegion._smallestY = std::min(_clusterRegion._smallestY, vCoordinates.at(1) - radius - 0.25/* bottom-margin buffer*/);
            } else {
                double semi_major = cluster.GetCartesianRadius() * _dataHub.GetEllipseShape(cluster.GetEllipseOffset());
                double degrees = 180.0 * (_dataHub.GetEllipseAngle(cluster.GetEllipseOffset()) / (double)M_PI);
                degrees = 180.0 - degrees; // invert degrees to lower quadrands for canvas rotate
                printString(buffer, cluster_def_format, vCoordinates.at(0), vCoordinates.at(1), cluster.GetCartesianRadius(), semi_major, degrees, _dataHub.GetEllipseShape(cluster.GetEllipseOffset()));
                _clusterRegion._largestX = std::max(_clusterRegion._largestX, vCoordinates.at(0) + semi_major + 0.25/* left-margin buffer*/);
                _clusterRegion._smallestX = std::min(_clusterRegion._smallestX, vCoordinates.at(0) - semi_major - 0.25/* left-margin buffer*/);
                _clusterRegion._largestY = std::max(_clusterRegion._largestY, vCoordinates.at(1) + semi_major + 0.25/* left-margin buffer*/);
                _clusterRegion._smallestY = std::min(_clusterRegion._smallestY, vCoordinates.at(1) - semi_major - 0.25/* bottom-margin buffer*/);
            }
			_cluster_definitions << "{ id: " << (i + 1) << ", significant : " << (cluster.isSignificant(_dataHub, i, simVars) ? "true" : "false")
				<< ", highrate : " << (cluster.getAreaRateForCluster(_dataHub) == HIGH ? "true" : "false")
				<< ", hierarchical : " << (cluster.isHierarchicalCluster() ? "true" : "false") << ", gini : " << (cluster.isGiniCluster() ? "true" : "false")
				<< ", ellipse : " << (cluster.GetEllipseOffset() != 0 ? "true" : "false") << ", " << buffer
				<< ", color : '" << (cluster.getAreaRateForCluster(_dataHub) == HIGH ? "#F13C3F" : "#5F8EBD")
                << "', pointscolor : '" << (cluster.getAreaRateForCluster(_dataHub) == HIGH ? "#FF1A1A" : "#1AC6FF") 
                << "', tip : '" << legend.c_str() << "', edges : [" << edges << "], points : [" << points << "] },\n";
        }
        ++_clusters_written;
    }

}

/** Render scatter chart to html page. */
void CartesianGraph::finalize() {
    std::string buffer, buffer2;
    std::stringstream html, worker, worker2;
    FileName fileName;
    std::vector<double> vCoordinates;

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
		templateReplace(html, "--cluster-input-label--", _dataHub.GetParameters().getUseLocationsNetworkFile() ? "Circles / Edges" : "Circles / Ellipses");

        // Update cluster region to keep x and y ranges equal -- for proper scaling in graph
        _clusterRegion.setproportional();
        _entireRegion = _clusterRegion;
        std::stringstream cluster_region_points, entire_region_points;
        worker.str("");
        worker2.str("");
        for (auto location : _dataHub.GetGroupInfo().getLocationsManager().locations()) {
            if (!_cluster_locations.test(location->index())) {
                location.get()->coordinates()->retrieve(vCoordinates);
                transform(vCoordinates);
                if (_clusterRegion.in(vCoordinates[0], vCoordinates[1]))
                    worker2 << printString(buffer2, "[%f, %f],", vCoordinates[0], vCoordinates[1]).c_str();
                worker << printString(buffer, "[%f, %f],", vCoordinates[0], vCoordinates[1]).c_str();
                _entireRegion._largestX = std::max(_entireRegion._largestX, vCoordinates[0]);
                _entireRegion._smallestX = std::min(_entireRegion._smallestX, vCoordinates[0]);
                _entireRegion._largestY = std::max(_entireRegion._largestY, vCoordinates[1]);
                _entireRegion._smallestY = std::min(_entireRegion._smallestY, vCoordinates[1]);
            }
        }
        std::string entire_points = worker.str();
        std::string cluster_points = worker2.str();
        // site resource link path
        buffer = _cluster_definitions.str();
        templateReplace(html, "--cluster-definitions--", trimString(buffer, ",\n").c_str());
        templateReplace(html, "--entire-region-points--", trimString(trimString(entire_points, ","), ",").c_str());
        templateReplace(html, "--cluster-region-points--", trimString(trimString(cluster_points, ","), ",").c_str());

        // TODO: try to get clusterRegion and entireRegion to better numbers/ticks in graph

        // replace parameters hash
        printString(buffer, "scanrate:%d/*high=1,low=2,highorlow=3*/,giniscan:%s", _dataHub.GetParameters().GetAreaScanRateType(),(_dataHub.GetParameters().getReportGiniOptimizedClusters() ? "true": "false"));
        templateReplace(html, "--parameters--", buffer.c_str());

        std::vector<double> vCoordinates;
        // Create collections of connections between the cluster nodes - we'll use them to create edges in display.
        worker.str("");
        if (_dataHub.GetParameters().getUseLocationsNetworkFile()) {
            Network::Connection_Details_t connections = GisUtils::getNetworkConnections(_dataHub.refLocationNetwork());
            for (auto connection : GisUtils::getNetworkConnections(_dataHub.refLocationNetwork())) {
                connection.get<0>()->coordinates()->retrieve(vCoordinates);
                transform(vCoordinates);
                worker << printString(buffer, "[[%f, %f],", vCoordinates[0], vCoordinates[1]).c_str();
                connection.get<1>()->coordinates()->retrieve(vCoordinates);
                transform(vCoordinates);
                worker << printString(buffer, "[%f, %f]],", vCoordinates[0], vCoordinates[1]).c_str();
            }
        }
        std::string all_edges = worker.str();
        templateReplace(html, "--entire-region-edges--", trimString(trimString(all_edges, ","), ",").c_str());

        // replace region hashes in template
        const char * region_hash = "zmin:0,zmax:1,xsteps:%d,ysteps:%d,bubbleSize:8,xmin:%f,xmax:%f,ymin:%f,ymax:%f";
        long xstep = static_cast<long>(std::min(20.0, std::abs(_clusterRegion._largestX - _clusterRegion._smallestX + 1.0)));
        long ystep = static_cast<long>(std::min(20.0, std::abs(_clusterRegion._largestY - _clusterRegion._smallestY + 1.0)));
        printString(buffer, region_hash, xstep, ystep, _clusterRegion._smallestX, _clusterRegion._largestX, _clusterRegion._smallestY, _clusterRegion._largestY);
        templateReplace(html, "--cluster-region-hash--", buffer.c_str());
        _entireRegion.setproportional(); // need to keep x,y ranges equal for proper scaling
        xstep = static_cast<long>(std::min(20.0, std::abs(_entireRegion._largestX - floor(_entireRegion._smallestX) + 1.0)));
        ystep = static_cast<long>(std::min(20.0, std::abs(_entireRegion._largestY - floor(_entireRegion._smallestY) + 1.0)));
        printString(buffer, region_hash, xstep, ystep, _entireRegion._smallestX, _entireRegion._largestX, _entireRegion._smallestY, _entireRegion._largestY);
        templateReplace(html, "--entire-region-hash--", buffer.c_str());

        // hide the option to switch between entire region and cluster region if cluster region is greater than 90% of entire region.
        double cluster_region_area = _clusterRegion.area();
        double entire_region_area = _entireRegion.area();
        templateReplace(html, "--display-zoom-cluster--", ((cluster_region_area / entire_region_area) < 0.9) ? "block" : "none");
        printString(buffer, "%.6f", (_dataHub.GetParameters().GetCoordinatesType() == CARTESIAN ? 1.0 : std::cos(degrees2radians(_median_parallel))));
        templateReplace(html, "--x_sinusoidal_factor--", buffer.c_str());
        templateReplace(html, "--satscan-version--", AppToolkit::getToolkit().GetVersion());

        HTMLout << html.str() << std::endl;
        HTMLout.close();
    } catch (prg_exception& x) {
        x.addTrace("renderScatterChart()", "ClusterScatterChart");
        throw;
    }
}

//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ClusterMap.h"
#include "SaTScanData.h"
#include <fstream>
#include "UtilityFunctions.h"
#include "RandomNumberGenerator.h"
#include "Toolkit.h"
#include "SSException.h"
#include "GisUtils.h"

const char * ClusterMap::HTML_FILE_EXT = ".html";
const char * ClusterMap::FILE_SUFFIX_EXT = ".clustermap";
const char * ClusterMap::API_KEY = "AIzaSyDFPvXhvQIG9SoJfHpsogahmlFZbrePWgc"; // API Key under spatovich@gmail.com - satscan

const char * ClusterMap::TEMPLATE = " \
<!DOCTYPE html> \n \
<html lang=\"en\"> \n \
    <head> \n \
        <title>Cluster Map</title> \n \
        <meta name=\"viewport\" content=\"initial-scale=1.0, user-scalable=no\" charset=\"utf-8\"> \n \
        <link href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/css/bootstrap.min.css\" rel=\"stylesheet\"> \n \
        <style type=\"text/css\"> \n \
         body {background-color: #f0f8ff;} \n \
         #chartContainer{ overflow: hidden; } \n \
         .chart-options{ display:none; } \n \
         .chart-options{ padding:10px 0 10px 0; background-color:#e6eef2; border:1px solid silver; } \n \
         .options-row{ margin:0 10px 10px 10px } \n \
         .options-row>label:first-child, .options-row detail{ color:#13369f; font-weight:bold; } \n \
         .options-row input[type='radio']{ margin:5px } \n \
         p.help-block{ font-size:11px; color:#666; font-style:oblique; margin-top:0; } \n \
         .main-content{ margin: 5px; } \n \
         .options-row label{ font-weight: normal; } \n \
         input[type=checkbox]{ margin-right:5px; } \n \
         label.option-section{ border-bottom: solid 1px #e6e9eb; width: 100 % ; } \n \
         .chart-column{ padding-top: 20px; padding-bottom: 30px; border-left: 1px solid #ddd; } \n \
         .print-section a{ padding-right: 20px; text-decoration: none; } \n \
         .cluster-selection{ border-bottom: dashed 1px #e6e9eb; } \n \
         .cluster-selection label{ white-space: nowrap; color: #313030; } \n \
         #id_display_count { margin:10px; } \n \
         fieldset { margin-top: 10px; } \n \
         @media print{ title{ display: none; } #id_banner { display: none; } .chart-options-section{ display: none; } #chartContainer{ margin: 20px; } .chart-column{ border-left: 0; } } \n \
         @media print{ img { max-width: none !important; } a[href]:after { content: \"\"; } } \n \
         #map-outer { height: 440px; padding: 20px; } \n \
         #map { height: 400px; box-shadow: 1px 1px 8px #999;} \n \
         @media all and (max-width: 991px) { #map-outer  { height: 650px } } \n \
         table.info-window td { padding: 3px; } \n \
        </style> \n \
        <script type='text/javascript' src='--resource-path--javascript/jquery/jquery-1.12.4/jquery-1.12.4.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/clustercharts/jQuery.resizeEnd.js'></script> \n \
        <script type='text/javascript' src='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/js/bootstrap.min.js'></script> \n \
    </head> \n \
    <body> \n \
        <table id=\"id_banner\" width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" bgcolor=\"#F8FAFA\" style=\"border-bottom: 3px double navy;\"> \n \
        <tbody><tr> \n \
        <td width=\"120\" align=\"center\" bgcolor=\"#DBD7DB\"><img src=\"--resource-path--images/swe2.jpg\" alt=\"&Ouml;stersund map\" title=\"ճtersund map\" width=\"120\" height=\"115\" hspace=\"1\" border=\"0\"></td> \n \
        <td align=\"right\" bgcolor=\"#D4DCE5\"><img src=\"--resource-path--images/satscan_title2.jpg\" alt=\"SaTScan&#0153; - Software for the spatial, temporal, and space-time scan statistics\" title=\"SaTScan&#0153; - Software for the spatial, temporal, and space-time scan statistics\" width=\"470\" height=\"115\"></td> \n \
        <td width=\"25%\" bgcolor=\"#F8FAFA\" align=\"right\"><img src=\"--resource-path--images/nyc2.jpg\" alt=\"New York City map\" title=\"New York City map\" width=\"112\" height=\"115\" hspace=\"1\" border=\"0\" align=\"middle\"></td> \n \
        </tr></tbody></table> \n \
		<div id=\"load_error\" style=\"color:#101010; text-align: center;font-size: 1.2em; padding: 20px;background-color: #ece1e1; border: 1px solid #e49595; display:none;\"></div> \n \
    <div class=\"container-fluid main-content\"> \n \
        <div class=\"row\"> \n \
            <div id=\"map-outer\" class=\"col-md-12\"> \n \
            <div class=\"col-md-3 chart-options-section\"> \n \
                <fieldset> \n \
                <!-- \n \
                <div class=\"options-row\"> \n \
                    <label class=\"option-section\" for=\"title_obs\">Title</label> \n \
                    <div>\n \
                        <input type=\"text\" style=\"width:95%;padding:1px;\" class=\"title-setter\" id=\"title_obs\" value=\"Cartesian Coordinates Map\"> \n \
                        <p class=\"help-block\">Title can be changed by editing this text.</p> \n \
                    </div> \n \
                </div> \n \
                --> \n \
                <div class=\"options-row\"> \n \
                    <label class=\"option-section\" for=\"title_obs\">Print</label> \n \
                    <div class=\"print-section\"> \n \
                        <a href=\"#\" onclick=\"javascript:window.print();return false;\"><span class=\"glyphicon glyphicon-print\" aria-hidden=\"true\"></span> Print</a> \n \
                        <!-- <a href=\"#\" id=\"print_png\"><span class=\"glyphicon glyphicon-picture\" aria-hidden = \"true\"></span> Save Image</a> --> \n \
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
                        <label style=\"margin-left:15px;\"><input type=\"checkbox\" id=\"id_hierarchical\" value=\"secondary\" />Hierarchical</label>\n \
                        <label style=\"margin-left:15px;\"><input type=\"checkbox\" id=\"id_gini\" value=\"secondary\" />Gini</label>\n \
                        <p class=\"help-block\">Display options for secondary clusters.</p>\n \
                    </div> \n \
                    <div>Show clusters using:</div>\n \
                    <label style=\"margin-left:15px;\"><input type=\"checkbox\" id=\"id_cluster_circles\" value=\"cluster\" checked=checked />Circles</label>\n \
                    <label style=\"margin-left:15px;\"><input type=\"checkbox\" id=\"id_cluster_locations\" value=\"cluster\" checked=checked />Locations</label>\n \
                    <p class=\"help-block\">Display options for clusters.</p>\n \
                    <!-- <label><input type=\"checkbox\" id=\"id_show_grid_lines\" checked=checked />Show grid lines</label> \n \
                    <p class=\"help-block\">Toggle display of graph grid lines.</p> --> \n \
                    <label><input type=\"checkbox\" id=\"id_show_location_points\" />Show all location points</label>\n \
                    <p class=\"help-block\">Toggle display of location points.</p>\n \
                    <label><input type=\"checkbox\" id=\"id_fit_graph_viewport\" checked=checked />Fit map to viewport</label>\n \
                    <p class=\"help-block\">Attempts to keep entire map in view.</p>\n \
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
            <div class=\"xx-col-md-9 chart-column\" id=\"map\"></div> \n \
            </div> \n \
        </div> \n \
     </div> \n \
        <script type='text/javascript'> \n \
            var parameters = {--parameters--};\n \
            if (parameters.scanrate != 3) { $('#id_rates_option').hide(); }\n \
            if (!parameters.giniscan) { $('#id_secondary_clusters_option').hide(); }\n \
            var entire_region_points = [--entire-region-points--]; \n \
            var display_stats = {};\n \
            var clusters = [ \n \
            --cluster-definitions-- \n \
            ]; \n \
            clusters.reverse();\n \
            var resource_path = '--resource-path--'; \n \
    </script> \n \
    <script src='--resource-path--javascript/clustercharts/mapgoogle-1.0.js'></script> \n \
  </body> \n \
</html> \n";

ClusterMap::ClusterMap(const CSaTScanData& dataHub) :_dataHub(dataHub), _clusters_written(0) { 
    _cluster_locations.resize(_dataHub.GetTInfo()->getLocations().size());
}

/** Alters pass Filename to include suffix and extension. */
FileName& ClusterMap::getFilename(FileName& filename) {
    std::string buffer;
    printString(buffer, "%s%s", filename.getFileName().c_str(), FILE_SUFFIX_EXT);
    filename.setFileName(buffer.c_str());
    filename.setExtension(HTML_FILE_EXT);
    return filename;
}

/** Replaces 'replaceStub' text in passed stringstream 'templateText' with text of 'replaceWith'. */
std::stringstream & ClusterMap::templateReplace(std::stringstream& templateText, const std::string& replaceStub, const std::string& replaceWith) {
    boost::regex to_be_replaced(replaceStub);
    std::string changed(boost::regex_replace(templateText.str(), to_be_replaced, replaceWith));
    templateText.str(std::string());
    templateText << changed;
    return templateText;
}

/** Return legend of cluster information to be used as popup in html page. */
std::string & ClusterMap::getClusterLegend(const CCluster& cluster, int iCluster, std::string& legend) const {
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

void ClusterMap::add(const MostLikelyClustersContainer& clusters, const SimulationVariables& simVars) {
    double gdMinRatioToReport = 0.001;
    std::vector<double> vCoordinates;
    std::string buffer, buffer2, legend;
    std::stringstream  worker;
    unsigned int clusterOffset = _clusters_written;

    //if no replications requested, attempt to display up to top 10 clusters
    tract_t tNumClustersToDisplay(simVars.get_sim_count() == 0 ? std::min(10, clusters.GetNumClustersRetained()) : clusters.GetNumClustersRetained());
    for (int i=0; i < clusters.GetNumClustersRetained(); ++i) {
        //get reference to i'th top cluster
        const CCluster& cluster = clusters.GetCluster(i);
        //skip purely temporal clusters
        if (cluster.GetClusterType() == PURELYTEMPORALCLUSTER)
            continue;
        if (!(i == 0 || (i < tNumClustersToDisplay && cluster.m_nRatio >= gdMinRatioToReport && (simVars.get_sim_count() == 0 || cluster.GetRank() <= simVars.get_sim_count()))))
            break;
        if (cluster.m_nRatio >= gdMinRatioToReport) {
            worker.str("");
            for (tract_t t=1; t <= cluster.GetNumTractsInCluster(); ++t) {
                tract_t tTract = _dataHub.GetNeighbor(cluster.GetEllipseOffset(), cluster.GetCentroidIndex(), t, cluster.GetCartesianRadius());
                if (!_dataHub.GetIsNullifiedLocation(tTract)) {
                    CentroidNeighborCalculator::getTractCoordinates(_dataHub, cluster, tTract, vCoordinates);
                    std::pair<double, double> prLatitudeLongitude(ConvertToLatLong(vCoordinates));
                    worker << printString(buffer2, "[%f, %f],", prLatitudeLongitude.second, prLatitudeLongitude.first).c_str();
                    _cluster_locations.set(tTract);
                }
            }

            if (cluster.GetEllipseOffset() != 0)
                throw prg_error("Not implemented for elliptical clusters", "ClusterMap::add()");

            GisUtils::pointpair_t clusterSegment = GisUtils::getClusterRadiusSegmentPoints(_dataHub, cluster);
            double radius = GisUtils::getRadiusInMeters(clusterSegment.first, clusterSegment.second);

            std::string points = worker.str();
            trimString(points, ",");
            _dataHub.GetGInfo()->retrieveCoordinates(cluster.GetCentroidIndex(), vCoordinates);
            std::pair<double, double> prLatitudeLongitude(ConvertToLatLong(vCoordinates));
            printString(buffer, "lat : %f, lng : %f, radius : %f", prLatitudeLongitude.first, prLatitudeLongitude.second, radius);
            _cluster_definitions << "{ id: " << (i + 1) << ", significant : " << (cluster.isSignificant(_dataHub, i, simVars) ? "true" : "false")
                << ", highrate : " << (cluster.getAreaRateForCluster(_dataHub) == HIGH ? "true" : "false") << ", " << buffer
                << ", hierarchical : " << (cluster.isHierarchicalCluster() ? "true" : "false") << ", gini : " << (cluster.isGiniCluster() ? "true" : "false")                
                << ", color : '" << (cluster.getAreaRateForCluster(_dataHub) == HIGH ? "#F13C3F" : "#5F8EBD") << "', pointscolor : '" << (cluster.getAreaRateForCluster(_dataHub) == HIGH ? "#FF1A1A" : "#1AC6FF") 
                << "', tip : '" << getClusterLegend(cluster, i + clusterOffset, legend).c_str() << "', points : [" << points << "] },\n";
        }
        ++_clusters_written;
    }

}

/** Render scatter chart to html page. */
void ClusterMap::finalize() {
    std::string buffer;
    std::stringstream html, worker;
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

        std::vector<double> vCoordinates;
        const TractHandler::LocationsContainer_t & locations = _dataHub.GetTInfo()->getLocations();
        worker.str("");
        for (size_t t=0; t < locations.size(); ++t) {
            if (!_cluster_locations.test(t)) {
                locations[t]->getCoordinates()[locations[t]->getCoordinates().size() - 1]->retrieve(vCoordinates);
                std::pair<double, double> prLatitudeLongitude(ConvertToLatLong(vCoordinates));
                worker << printString(buffer, "[%f, %f],", prLatitudeLongitude.second, prLatitudeLongitude.first).c_str();
            }
        }
        std::string entire_points = worker.str();
        // site resource link path
        buffer = _cluster_definitions.str();
        templateReplace(html, "--cluster-definitions--", trimString(buffer, ",\n").c_str());
        templateReplace(html, "--entire-region-points--", trimString(trimString(entire_points, ","), ",").c_str());

        // replace parameters hash
        printString(buffer, "scanrate:%d/*high=1,low=2,highorlow=3*/,giniscan:%s", _dataHub.GetParameters().GetAreaScanRateType(),(_dataHub.GetParameters().getReportGiniOptimizedClusters() ? "true": "false"));
        templateReplace(html, "--parameters--", buffer.c_str());
        templateReplace(html, "--satscan-version--", AppToolkit::getToolkit().GetVersion());
        templateReplace(html, "--api-key--", std::string(API_KEY));

        HTMLout << html.str() << std::endl;
        HTMLout.close();
    } catch (prg_exception& x) {
        x.addTrace("renderScatterChart()", "ClusterScatterChart");
        throw;
    }
}

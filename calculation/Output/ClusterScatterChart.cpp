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
        <title>Clusters Graph</title> \n \
        <meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"> \n \
        <link href=\"--resource-path--javascript/bootstrap/3.4.1/bootstrap.min.css\" rel=\"stylesheet\"> \n \
        <link rel=\"stylesheet\" href=\"--resource-path--javascript/clustercharts/nouislider.css\"> \n \
        <link rel=\"stylesheet\" href=\"--resource-path--javascript/clustercharts/clusterchart-1.0.css\"> \n \
        <script type='text/javascript' src='--resource-path--javascript/jquery/jquery-1.12.4/jquery-1.12.4.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/clustercharts/jQuery.resizeEnd.js'></script> \n \
        <script type=\"text/javascript\">jQuery.noConflict();</script> \n \
        <script type='text/javascript' src='--resource-path--javascript/clustercharts/mootools-1.6.0/MooTools-Core-1.6.0.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/clustercharts/clusterchart-1.3.2.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/clustercharts/mootools-1.6.0/MooTools-More-1.6.0.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/clustercharts/FileSaver-2014-06-24.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/clustercharts/Blob-2014-07-24.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/clustercharts/canvas-toBlob-2016-05-26.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/bootstrap/3.4.1/bootstrap.min.js'></script> \n \
        <link rel=\"stylesheet\" href=\"--resource-path--javascript/bootstrap/bootstrap-multiselect/bootstrap-multiselect.css\"> \n \
        <script src=\"--resource-path--javascript/bootstrap/bootstrap-multiselect/bootstrap-multiselect.js\"></script> \n \
         <script src=\"--resource-path--javascript/clustercharts/nouislider.js\"></script> \n \
        <script type='text/javascript'> \n \
            var clusters = [ \n \
            --cluster-definitions-- \n \
            ]; \n \
            var chart = null; \n \
            const parameters = {--parameters--};\n \
            const slider_range = {--slider-range--};\n \
            const slider_range_start = --slider-range-start--;\n \
            var cluster_region = {--cluster-region-hash--}; \n \
            var cluster_region_points = [--cluster-region-points--]; \n \
            var entire_region = {--entire-region-hash--}; \n \
            var entire_region_points = [--entire-region-points--]; \n \
            var entire_region_edges = [--entire-region-edges--]; \n \
            var x_sinusoidal_factor = --x_sinusoidal_factor--; \n \
        </script> \n \
    </head> \n \
    <body> \n \
		<div id='load_error'></div> \n \
    <div class='container-fluid main-content'> \n \
        <div class='row'> \n \
            <div class='col-md-3 chart-options-section'> \n \
                <fieldset> \n \
                <div class='options-row'> \n \
                    <div class='version'>Generated with SaTScan v--satscan-version--</div>\n \
                </div> \n \
                <div class='options-row'> \n \
                    <div id='id_clusters'>\n \
                        <div><label>Display Clusters By:</label></div> \n \
                        <input type='radio' name='cluster_display_type' id='cl_value' value='value' checked=checked/><label for='cl_value'>P-Value</label> \n \
                        <label for='slider_clusters'>P-Value</label> \n \
                        <div class='slider-styled slider-round' id='slider_clusters'></div> \n \
                        <div id='id_slider_value'></div> \n \
                        <input type='radio' name='cluster_display_type' id='cl_list' value='list' /><label for='cl_list'>Index Number</label> \n \
                        <label for='id_select_clusters'>Selections:</label> \n \
                        <select name='select_clusters' id='id_select_clusters' multiple='multiple' class='clusters-select'> \n \
                            --cluster-options--\n \
                        </select> \n \
                        <p class='help-block'>Toggle display of clusters.</p> \n \
                    </div>\n \
                    <div id='id_rates_option'> \n \
                        <label><input type='radio' name='view_rate' id='id_view_highlow' value='entire' checked=checked />High and low clusters</label>\n \
                        <label><input type='radio' name='view_rate' id='id_view_high' value='cluster'/>High only</label>\n \
                        <label><input type='radio' name='view_rate' id='id_view_low' value='cluster'/>Low only</label>\n \
                        <p class='help-block'>Toggle display of clusters for scan rate.</p>\n \
                    </div> \n \
                    <div id='id_secondary_clusters_option'> \n \
                        <div>Secondary Clusters:</div>\n \
                        <label class='cluster-show-opts'><input type='checkbox' id='id_hierarchical' value='secondary'/>Hierarchical</label>\n \
                        <label class='cluster-show-opts'><input type='checkbox' id='id_gini' value='secondary'/>Gini</label>\n \
                        <p class='help-block'>Display options for secondary clusters.</p>\n \
                    </div> \n \
                    <div>Show clusters using:</div> \n \
                    <label class='cluster-show-opts'><input type='checkbox' id='id_cluster_circles' value='cluster' checked=checked/>--cluster-input-label--</label>\n \
                    <label class='cluster-show-opts'><input type='checkbox' id='id_cluster_locations' value='cluster'/>Locations</label>\n \
                    <p class='help-block'>Display options for clusters.</p>\n \
                    <div id='id_zoom_cluster_option' style='display:--display-zoom-cluster--;'>\n \
                    <label><input type='checkbox' id='id_zoom_cluster_region'/>Zoom in on cluster region</label>\n \
                    <p class='help-block'>Focus grid region on reported clusters.</p>\n \
                    </div>\n \
                    <label><input type='checkbox' id='id_show_grid_lines' checked=checked />Show grid lines</label>\n \
                    <p class='help-block'>Toggle display of graph grid lines.</p>\n \
                    <label><input type='checkbox' id='id_show_axes' checked=checked />Show x and y axes</label>\n \
                    <p class='help-block'>Toggle display of graph x / y axes.</p>\n \
                    <label><input type='checkbox' id='id_show_location_points' />Show all location points</label>\n \
                    <p class='help-block'>Toggle display of location points.</p>\n \
                    <label><input type='checkbox' id='id_show_all_edges' />Show all edges</label> \n \
                    <p class='help-block'>Toggle display of all edges.</p> \n \
                    <label><input type='checkbox' id='id_fit_graph_viewport' />Fit graph to viewport</label>\n \
                    <p class='help-block'>Attempts to keep entire graph in view.</p>\n \
                </div> \n \
                <div id=\"id_display_count\">\n \
                    <fieldset>\n \
                            <legend>Display Data:</legend>\n \
                            <div><span id='id_cluster_count'></span> Clusters</div>\n \
                            <div><span id='id_cluster_point_count'></span> Cluster Locations</div>\n \
                            <div><span id='id_point_count'></span> Total Locations</div> \n \
                    </fieldset>\n \
                </div>\n \
                <div class='options-row'> \n \
                    <label class='option-section' for='title_obs' style='display:none;'>Title</label> \n \
                    <div>\n \
                        <input type='text' style='width:95%;padding:1px;' class='title-setter' id='title_obs' value='Clusters Graph'> \n \
                        <p class='help-block'>Title can be changed by editing this text.</p> \n \
                    </div> \n \
                </div> \n \
                <div class='options-row'> \n \
                    <label class='option-section' for='title_obs' style='display:none;'>Print</label> \n \
                    <div class='print-section'> \n \
                        <a href='#' onclick='javascript:window.print();return false;'><span aria-hidden='true'></span> Print</a> \n \
                        <a href='#' id='print_png'><span aria-hidden='true'></span> Save Image</a> \n \
                    </div> \n \
                </div> \n \
                </fieldset> \n \
            </div> \n \
            <div class='col-md-9 chart-column'> \n \
                <div id='chartContainer' name='chartContainer'></div> \n \
            </div> \n \
        </div> \n \
     </div> \n \
     </body> \n \
</html> \n";

CartesianGraph::CartesianGraph(const CSaTScanData& dataHub) : _dataHub(dataHub), _clusters_written(0), _median_parallel(0.0) {
    _cluster_locations.resize(_dataHub.getLocationsManager().locations().size());
    if (_dataHub.GetParameters().GetCoordinatesType() == LATLON) {
        // Calculate the median parallel among all points.
        std::vector<double> parallels, vCoordinates;
		for (const auto& identifier: _dataHub.getIdentifierInfo().getIdentifiers()) {
            for (unsigned int loc = 0; loc < identifier->getLocations().size(); ++loc) {
                identifier->getLocations()[loc]->coordinates()->retrieve(vCoordinates);
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

void CartesianGraph::add(const MostLikelyClustersContainer& clusters, const SimulationVariables& simVars, unsigned int iteration) {
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
            const auto& locations = _dataHub.getLocationsManager().locations();
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
            VisualizationUtils::getHtmlClusterLegend(cluster, i + clusterOffset, _dataHub, legend);
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
			_cluster_definitions << "{ id: " << (i + iteration) << ", slider_value : " << VisualizationUtils::getSliderValue(_dataHub, cluster, i + iteration, simVars)
				<< ", highrate : " << (cluster.getAreaRateForCluster(_dataHub) == HIGH ? "true" : "false")
				<< ", hierarchical : " << (cluster.isHierarchicalCluster() ? "true" : "false") << ", gini : " << (cluster.isGiniCluster() ? "true" : "false")
				<< ", ellipse : " << (cluster.GetEllipseOffset() != 0 ? "true" : "false") << ", " << buffer
				<< ", color : '" << (cluster.getAreaRateForCluster(_dataHub) == HIGH ? "#F13C3F" : "#5F8EBD")
                << "', pointscolor : '" << (cluster.getAreaRateForCluster(_dataHub) == HIGH ? "#FF1A1A" : "#1AC6FF") 
                << "', tip : '" << legend.c_str() << "', edges : [" << edges << "], points : [" << points << "] },\n";
           _cluster_options << "<option value=" << (i + iteration) << " " << (i == 0 ? "selected" : "") << ">Cluster " << (i + iteration) << "</option>";
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
    const auto& parameters = _dataHub.GetParameters();

    try {
        fileName.setFullPath(parameters.GetOutputFileName().c_str());
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
		templateReplace(html, "--cluster-input-label--", parameters.getUseLocationsNetworkFile() ? "Circles / Edges" : "Circles / Ellipses");

        // Update cluster region to keep x and y ranges equal -- for proper scaling in graph
        _clusterRegion.setproportional();
        _entireRegion = _clusterRegion;
        std::stringstream cluster_region_points, entire_region_points;
        worker.str("");
        worker2.str("");
        for (const auto& location : _dataHub.getLocationsManager().locations()) {
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

        // write clusters option
        worker.str("");
        if (_cluster_options.tellp())
            worker << _cluster_options.str();
        templateReplace(html, "--cluster-options--", worker.str());

        // replace parameters hash
        printString(buffer, "scanrate:%d/*high=1,low=2,highorlow=3*/,giniscan:%s,prospective:%s", 
            parameters.GetAreaScanRateType(), parameters.getReportGiniOptimizedClusters() ? "true": "false", parameters.GetIsProspectiveAnalysis() && !_dataHub.isDrilldown() ? "true" : "false"
        );
        templateReplace(html, "--parameters--", buffer.c_str());
        templateReplace(html, "--slider-range--", VisualizationUtils::getSliderRange(_dataHub));
        if (parameters.GetIsProspectiveAnalysis()) {
            unsigned int default_RI = parameters.getReadingLineDataFromCasefile() ? static_cast<unsigned int>(parameters.getCutoffLineListCSV()) : 365;
            templateReplace(html, "--slider-range-start--", printString(buffer, "%u", default_RI));
        } else {
            double default_PV = parameters.getReadingLineDataFromCasefile() ? parameters.getCutoffLineListCSV() : 1.0;
            templateReplace(html, "--slider-range-start--", printString(buffer, "%g", default_PV));
        }

        std::vector<double> vCoordinates;
        // Create collections of connections between the cluster nodes - we'll use them to create edges in display.
        worker.str("");
        if (parameters.getUseLocationsNetworkFile()) {
            Network::Connection_Details_t connections = GisUtils::getNetworkConnections(_dataHub.refLocationNetwork());
            for (const auto& connection : GisUtils::getNetworkConnections(_dataHub.refLocationNetwork())) {
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
        printString(buffer, "%.6f", (parameters.GetCoordinatesType() == CARTESIAN ? 1.0 : std::cos(degrees2radians(_median_parallel))));
        templateReplace(html, "--x_sinusoidal_factor--", buffer.c_str());
        templateReplace(html, "--satscan-version--", AppToolkit::getToolkit().GetVersion());

        HTMLout << html.str() << std::endl;
        HTMLout.close();
    } catch (prg_exception& x) {
        x.addTrace("renderScatterChart()", "ClusterScatterChart");
        throw;
    }
}

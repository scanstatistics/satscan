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
#include "DataDemographics.h"

unsigned int EventType::_counter = 0;

std::string EventType::getCategoryColor(unsigned int offset) const {
    if (offset > _visual_utils.getColors().size() - 1) {
        return _visual_utils.getRandomHtmlColor();
    } else {
        return _visual_utils.getColors()[offset];
    }
}

std::string EventType::toJson(const std::string& resource_path) {
    std::stringstream json;
    json << "{ event_class: '" << _class << "', " << "event_type: '" << _type << "', " << "event_name: '" << _name << "', ";
    json << "categories: [";
    for (auto itr = _categories.begin(); itr != _categories.end(); ++itr) {
        json << (itr == _categories.begin() ? "" : ", ") << "{type: '" << itr->get<0>() << "', label: '" << itr->get<1>() << "', color: '" << itr->get<2>() << "'}";
    }
    json << "] }";
    return json.str();
}

const char * ClusterMap::HTML_FILE_EXT = ".html";
const char * ClusterMap::FILE_SUFFIX_EXT = ".clustermap";

const char * ClusterMap::TEMPLATE = " \
<!DOCTYPE html> \n \
<html lang=\"en\"> \n \
    <head> \n \
        <title>Cluster Map</title> \n \
        <meta name=\"viewport\" content=\"initial-scale=1.0, user-scalable=no\" charset=\"utf-8\"> \n \
        <link href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/css/bootstrap.min.css\" rel=\"stylesheet\"> \n \
        <link rel=\"stylesheet\" href=\"--resource-path--javascript/clustercharts/nouislider.css\"> \n \
        <style type=\"text/css\"> \n \
         body {background-color: #f0f8ff;} \n \
         #chartContainer{ overflow: hidden; } \n \
         .chart-options{ display:none; } \n \
         .chart-options{ padding:10px 0 10px 0; background-color:#e6eef2; border:1px solid silver; } \n \
         .options-row{ margin:0 5px 5px 5px } \n \
         .options-row>label:first-child, .options-row detail{ color:#13369f; font-weight:bold; } \n \
         input.standard[type='radio']{ margin:5px } \n \
         p.help-block{ font-size:11px; color:#666; font-style:oblique; margin-top:0; margin-bottom:1px;} \n \
         .main-content{ margin: 5px; } \n \
         .options-row label{ font-weight: normal; margin-bottom: 1px;} \n \
         input[type=checkbox]{ margin-right:5px; } \n \
         label.option-section{ border-bottom: solid 1px #e6e9eb; width: 100 % ; } \n \
         .chart-column{ padding-top: 20px; padding-bottom: 30px; border-left: 1px solid #ddd; } \n \
         .print-section a{ padding-right: 20px; text-decoration: none; } \n \
         .cluster-selection{ border-bottom: dashed 1px #e6e9eb; } \n \
         .cluster-selection label{ white-space: nowrap; color: #313030; } \n \
         #id_display_count { margin:10px; } \n \
         @media print{ title{ display: none; } #id_banner { display: none; } .chart-options-section{ display: none; } #chartContainer{ margin: 20px; } .chart-column{ border-left: 0; } } \n \
         @media print{ img { max-width: none !important; } a[href]:after { content: \"\"; } } \n \
         #map-outer { height: 75.0rem !important; padding: 20px; } \n \
         #map { height:75.0rem !important; box-shadow: 1px 1px 8px #999;} \n \
         @media all and (max-width: 991px) { #map-outer  { height: 650px } } \n \
         table.info-window td { padding: 3px; }\n \
         #legend { display:none; font-family: Arial, sans-serif; background: #fff; padding: 10px; margin: 10px; border: 1px solid #000; } \n \
         .slider-round { height: 10px; margin: 3px 5px 5px 5px; } \n \
         .slider-round .noUi-connect { background: #387bbe; } \n \
         .slider-round .noUi-handle { height: 18px; width: 18px; top: -5px; right: -9px; /* half the width */ border-radius: 9px; background: #0052A3; border: 1px solid #0052A3; } \n \
         .slider-round-small { height: 8px; margin: 6px 5px 2px 0px; } \n \
         .slider-round-small .noUi-connect { background: #387bbe; } \n \
         .slider-round-small .noUi-handle { height: 12px; width: 12px; top: -3px; right: -7px; /* half the width */ border-radius: 9px; background: #0052A3; border: 1px solid #0052A3; } \n \
        </style> \n \
        <script type=\"text/javascript\" src=\"--resource-path--javascript/jquery/jquery-1.12.4/jquery-1.12.4.js\"></script> \n \
        <script type=\"text/javascript\" src=\"--resource-path--javascript/clustercharts/jQuery.resizeEnd.js\"></script> \n \
        <script type=\"text/javascript\" src=\"--resource-path--javascript/bootstrap/3.3.6/bootstrap.min.js\"></script> \n \
        <link rel=\"stylesheet\" href=\"--resource-path--javascript/bootstrap/bootstrap-multiselect/bootstrap-multiselect.css\"> \n \
        <script src=\"--resource-path--javascript/bootstrap/bootstrap-multiselect/bootstrap-multiselect.js\"></script> \n \
        <script src=\"--resource-path--javascript/clustercharts/nouislider.js\"></script> \n \
        <script src=\"--resource-path--javascript/clustercharts/OverlappingMarkerSpiderfier_1.0.3_oms.min.js\"></script> \n \
    </head> \n \
    <body> \n \
		<div id='load_error' style='color:#101010; text-align: center;font-size: 1.2em; padding: 20px;background-color: #ece1e1; border: 1px solid #e49595; display:none;'></div> \n \
    <div class='container-fluid main-content'> \n \
        <div class='row'> \n \
            <div id='map-outer' class='col-md-12'> \n \
            <div class='col-md-2 chart-options-section' style='padding-left:0;'> \n \
                <fieldset> \n \
                <div class='options-row'> \n \
                    <div style='font-style:italic;'>Generated with SaTScan v--satscan-version--</div>\n \
                </div>\n \
                <div class='options-row'> \n \
                    <div id='id_clusters'>\n \
                        <label for='id_select_clusters'>Display Clusters:</label> \n \
                        <select name='select_clusters' id='id_select_clusters' multiple='multiple' class='clusters-select'> \n \
                            --significant-cluster-options--\n \
                            --non-significant-cluster-options--\n \
                            </select> \n \
                            <p class='help-block'>Toggle display of clusters.</p> \n \
                    </div>\n \
                    <div id='id_rates_option'> \n \
                        <label><input type='radio' class='standard' name='view_rate' id='id_view_highlow' value='entire' checked=checked />High and low clusters</label>\n \
                        <label><input type='radio' class='standard' name='view_rate' id='id_view_high' value='cluster'/>High only</label>\n \
                        <label><input type='radio' class='standard' name='view_rate' id='id_view_low' value='cluster'/>Low only</label>\n \
                        <p class='help-block'>Toggle display of clusters for scan rate.</p>\n \
                    </div> \n \
                    <div id='id_secondary_clusters_option'> \n \
                        <div>Secondary Clusters:</div>\n \
                        <label style='margin-left:15px;'><input type='checkbox' id='id_hierarchical' value='secondary' />Hierarchical</label>\n \
                        <label style='margin-left:15px;'><input type='checkbox' id='id_gini' value='secondary' />Gini</label>\n \
                        <p class='help-block'>Display options for secondary clusters.</p>\n \
                    </div> \n \
                    <div>Show clusters using:</div>\n \
                    <label style='margin-left:15px;'><input type='checkbox' id='id_cluster_circles' value='cluster' checked=checked />--cluster-display--</label>\n \
                    <label style='margin-left:15px;'><input type='checkbox' id='id_cluster_locations' value='cluster' />Locations</label>\n \
                    <p class='help-block'>Display options for clusters.</p>\n \
                    <!-- <label><input type='checkbox' id='id_show_grid_lines' checked=checked />Show grid lines</label> \n \
                    <p class='help-block'>Toggle display of graph grid lines.</p> --> \n \
                    <label><input type='checkbox' id='id_show_location_points' />Show all location points</label>\n \
                    <p class='help-block'>Toggle display of location points.</p>\n \
                    <!--<label><input type='checkbox' id='id_fit_graph_viewport' checked=checked />Fit map to viewport</label>\n \
                    <p class='help-block'>Attempts to keep entire map in view.</p> --> \n \
                </div> \n \
                <div class='options-row event-controls'> \n \
                    <div id='id_display_events'> \n \
                        <label for='id_select_event_type'>Display Events By:</label> \n \
                        <select name='select_event_type' id='id_select_event_type' multiple='multiple' class='events-select'> \n \
                        --select-groups-event-type-display-- \n \
                        </select> \n \
                    </div> \n \
                    <p class='help-block'>Displays markers for selected events.</p> \n \
                    <div id='id_group_events'>  \n \
                    <label for='id_group_event_type'>Separate Icons:</label> \n \
                    <select name='group_event_type' id='id_group_event_type' multiple='multiple' class='events-group'></select> \n \
                    </div> \n \
                    <p class='help-block'>Maximum of 8 can be distinguished by icon.</p> \n \
                    <div id='id_filter_events'> \n \
                        <label for='id_filter_event_type'>Exclude Events:</label> \n \
                        <select name='filter_event_type' id='id_filter_event_type' multiple='multiple' class='events-filter'> \n \
                        --select-groups-event-type-exclude-- \n \
                        </select> \n \
                    </div> \n \
                    <p class='help-block'>Filter to exclude markers of displayed events.</p> \n \
                    <label for='slider_display'>Events In Study Period</label> \n \
                    <div class='slider-styled slider-round' id='slider_display'></div> \n \
                    <div style='font-size: small;padding-bottom: 5px;'><span id='id_range_startdate'>11/23/2021</span> to <span id='id_range_enddate'>3/2/2022</span></div> \n \
                    <div class='pull-left'> \n \
                    <button type='button' class='btn btn-success btn-sm' id='id_run_timeline'>Run</button> \n \
                    <button type='button' class='btn btn-primary btn-sm' id='id_pause_timeline'>Pause</button> \n \
                    <button type='button' class='btn btn-secondary btn-sm btn-warning' id='id_run_timeline_reset'>Reset</button> \n \
                    </div> \n \
                    <div class='clearfix'></div> \n \
                    <label for='slider_speed' style='font-size:12px;margin-bottom:0;margin-left:2px;'>Run Delay: <span id='id_timeline_rate'>50</span> milliseconds</label> \n \
                    <div class='slider-styled slider-round-small' id='slider_speed'></div> \n \
                    <label for='slider_display' style='margin-top:5px;'>Recent Events</label> \n \
                    <div class='slider-styled slider-round' id='slider_recent'></div> \n \
                    <div style='font-size: small;padding-bottom: 5px;'>Recent as of <span id='id_range_recent'>11/23/2021</span></div> \n \
                    <!-- <p class='help-block'>Show events by study period.</p> --> \n \
                    <label><input type='checkbox' id='id_show_legend' checked=checked />Display Event Legend</label> \n \
                    <div style='margin-top: 1px;'><label>Events Size</label></div> \n \
                     <div class='slider-styled slider-round' id='slider_size' ></div> \n \
                     <div style='font-size:small;text-align:center;'><span style='margin-right:15px;'>&#129048; smaller</span><span style='margin-left:15px;'>larger &#x1F81A;</span></div> \n \
                    <div class='clearfix'></div> \n \
                </div> \n \
                <div id='id_display_count'>\n \
                    <fieldset>\n \
                            <legend style='font-size:14px; margin-bottom:0;'>Display Data:</legend>\n \
                            <div><span id='id_cluster_count'></span> Clusters</div>\n \
                            <div><span id='id_cluster_point_count'></span> Cluster Locations</div>\n \
                            <div><span id='id_point_count'></span> Total Locations</div> \n \
                            <div><span id='id_event_count'>0</span> Total Events</div> \n \
                    </fieldset>\n \
                </div>\n \
                <div class='options-row'> \n \
                    <label class='option-section' style='display:none;' for='title_obs'>Print</label> \n \
                    <div class='print-section'> \n \
                        <a href='#' onclick='javascript:window.print();return false;'><span class='glyphicon glyphicon-print' aria-hidden='true'></span> Print</a> \n \
                        <!-- <a href='#' id='print_png'><span class='glyphicon glyphicon-picture' aria-hidden = 'true'></span> Save Image</a> --> \n \
                    </div> \n \
                </div> \n \
                </fieldset> \n \
            </div> \n \
            <div class='xx-col-md-10 chart-column' id='map'></div> \n \
            <div id='legend'><h3>Legend</h3></div> \n \
            </div> \n \
        </div> \n \
     </div> \n \
        <script type='text/javascript'> \n \
            const event_range_begin = --event-range-begin--; \n \
            const event_range_start = --event-range-start--; \n \
            const event_range_end = --event-range-end--; \n \
            const true_dates = --true-dates--; \n \
            var event_types = [--event-types-definitions--]; \n \
            var events = [--event-definitions--]; \n \
            var parameters = {--parameters--};\n \
            if (parameters.scanrate != 3) { $('#id_rates_option').hide(); }\n \
            if (!parameters.giniscan) { $('#id_secondary_clusters_option').hide(); }\n \
            var entire_region_points = [--entire-region-points--]; \n \
            var display_stats = {};\n \
            var clusters = [--cluster-definitions--]; \n \
            clusters.reverse();\n \
            var resource_path = '--resource-path--'; \n \
    </script> \n \
    <script src=\"--resource-path--javascript/clustercharts/mapgoogle-1.3.js\"></script> \n \
  </body> \n \
</html> \n";

ClusterMap::ClusterMap(const CSaTScanData& dataHub) :_dataHub(dataHub), _clusters_written(0),
    _recent_startdate(dataHub.GetParameters().GetIsProspectiveAnalysis() ? dataHub.GetTimeIntervalStartTimes().at(dataHub.getDataInterfaceIntervalStartIndex()) : dataHub.GetTimeIntervalStartTimes().front()) { 
    _cluster_locations.resize(_dataHub.GetGroupInfo().getLocationsManager().locations().size());
}

/** Alters pass Filename to include suffix and extension. */
FileName& ClusterMap::getFilename(FileName& filename) {
    std::string buffer;
    printString(buffer, "%s%s", filename.getFileName().c_str(), FILE_SUFFIX_EXT);
    filename.setFileName(buffer.c_str());
    filename.setExtension(HTML_FILE_EXT);
    return filename;
}

/** Return legend of cluster information to be used as popup in html page. */
std::string & ClusterMap::getClusterLegend(const CCluster& cluster, int iCluster, std::string& legend) const {
    std::stringstream  lines;
    CCluster::ReportCache_t::const_iterator itr = cluster.getReportLinesCache().begin(), itr_end = cluster.getReportLinesCache().end();
    unsigned int currSetIdx = std::numeric_limits<unsigned int>::max(), numFilesSets = _dataHub.GetParameters().getNumFileSets();

    lines << "<div style=\"text-decoration:underline;\">Cluster " << iCluster + 1 << "</div>";
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

/* Conditionally adds clusters of most likely cluster collection to Google Map. */
void ClusterMap::add(const MostLikelyClustersContainer& clusters, const SimulationVariables& simVars, unsigned int iteration) {
    double gdMinRatioToReport = 0.001;
    std::vector<double> vCoordinates;
    std::string buffer, buffer2, legend, points, edges;
    std::stringstream  worker;
    unsigned int clusterOffset = _clusters_written;
    const CParameters& parameters = _dataHub.GetParameters();

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
            NetworkLocationContainer_t networkLocations;
            if (_dataHub.GetParameters().getUseLocationsNetworkFile()) {
                // Create collections of connections between the cluster nodes - we'll use them to create edges in display.
                _dataHub.getClusterNetworkLocations(cluster, networkLocations);
                Network::Connection_Details_t connections = GisUtils::getClusterConnections(networkLocations);
                worker.str("");
                for (auto itr = connections.begin(); itr != connections.end(); ++itr) {
                    itr->get<0>()->coordinates()->retrieve(vCoordinates);
                    std::pair<double, double> prLatitudeLongitude(ConvertToLatLong(vCoordinates));
                    worker << printString(buffer2, "[[%f, %f],", prLatitudeLongitude.second, prLatitudeLongitude.first).c_str();
                    itr->get<1>()->coordinates()->retrieve(vCoordinates);
                    prLatitudeLongitude = ConvertToLatLong(vCoordinates);
                    worker << printString(buffer2, "[%f, %f]],", prLatitudeLongitude.second, prLatitudeLongitude.first).c_str();
                }
                edges = worker.str();
                trimString(edges, ",");
            }
            // Compile collection of cluster points to display through known tracts of cluster.
            worker.str("");
            auto locations = _dataHub.GetGroupInfo().getLocationsManager().locations();
            boost::dynamic_bitset<> clusterLocations;
            CentroidNeighborCalculator::getLocationsAboutCluster(_dataHub, cluster, &clusterLocations);
            size_t index = clusterLocations.find_first();
            while (index != clusterLocations.npos) {
                locations[index].get()->coordinates()->retrieve(vCoordinates);
                std::pair<double, double> prLatitudeLongitude(ConvertToLatLong(vCoordinates));
                worker << printString(buffer2, "[%f, %f],", prLatitudeLongitude.second, prLatitudeLongitude.first).c_str();
                _cluster_locations.set(index);
                index = clusterLocations.find_next(index);
            }
            points = worker.str();
            trimString(points, ",");
            // Add cluster definition to javascript hash collection.
            if (cluster.GetEllipseOffset() != 0)
                throw prg_error("Not implemented for elliptical clusters", "ClusterMap::add()");

            GisUtils::pointpair_t clusterSegment = GisUtils::getClusterRadiusSegmentPoints(_dataHub, cluster);
            double radius = GisUtils::getRadiusInMeters(clusterSegment.first, clusterSegment.second);
            // Record window start of MLC with prospective analyses - we'll potentially use this with events display.
            if (i == 0 && parameters.GetIsProspectiveAnalysis()) _recent_startdate = _dataHub.GetTimeIntervalStartTimes()[cluster.m_nFirstInterval];
            _dataHub.GetGInfo()->retrieveCoordinates(cluster.GetCentroidIndex(), vCoordinates);
            std::pair<double, double> prLatitudeLongitude(ConvertToLatLong(vCoordinates));
            printString(buffer, "lat : %f, lng : %f, radius : %f", prLatitudeLongitude.first, prLatitudeLongitude.second, radius);
            _cluster_definitions << "{ id: " << (i + iteration) << ", significant : " << (cluster.isSignificant(_dataHub, i, simVars) ? "true" : "false")
                << ", highrate : " << (cluster.getAreaRateForCluster(_dataHub) == HIGH ? "true" : "false") << ", " << buffer
                << ", hierarchical : " << (cluster.isHierarchicalCluster() ? "true" : "false") << ", gini : " << (cluster.isGiniCluster() ? "true" : "false")                
                << ", color : '" << (cluster.getAreaRateForCluster(_dataHub) == HIGH ? "#F13C3F" : "#5F8EBD") << "', pointscolor : '" << (cluster.getAreaRateForCluster(_dataHub) == HIGH ? "#FF1A1A" : "#1AC6FF") 
                << "', tip : '" << getClusterLegend(cluster, i + clusterOffset, legend).c_str() << "', edges : [" << edges << "], points : [" << points << "] },\n";

            if (cluster.isSignificant(_dataHub, i, simVars)) {
                _cluster_options_significant << "<option value=" << (i + iteration) << " class='significant_clusters' selected>Cluster " << (i + iteration) << "</option>";
            } else {
                _cluster_options_non_significant << "<option value=" << (i + iteration) << " class='non_significant_clusters'>Cluster " << (i + iteration) << "</option>";
            }
        }
        ++_clusters_written;
    }
}

/* Adds events/case demographic markers to Google Map. */
void ClusterMap::add(const DataDemographicsProcessor& demographics) {
    const CParameters& parameters = _dataHub.GetParameters();
    // Create collection of event types that we'll be grouping the events into.
    std::vector<std::string> g_values;
    csv_string_to_typelist<std::string>(parameters.getKmlEventGroupAttribute().c_str(), g_values);
    for (auto const &demographic : demographics.getDataSetDemographics().getAttributes()) {
        if (demographic.second->gettype() <= EVENT_COORD_Y) continue; // Only want attributes, no event id or coordinates.
        if (std::find(g_values.begin(), g_values.end(), demographic.first) == g_values.end()) continue;
        if (std::find_if(_event_types.begin(), _event_types.end(), [&demographic](const EventType& et) { return et.name() == demographic.first; }) == _event_types.end())
            _event_types.push_back(EventType(demographic.first));
    }
    if (_event_types.size() == 0) {
        _dataHub.GetPrintDirection().Printf("No characteristics to group by. Event placements will not be added to Google Maps output.\n",  BasePrint::P_WARNING);
    } else {
        // Sort so event types are reported alphabetically.
        std::sort(_event_types.begin(), _event_types.end(), [](const EventType &left, const EventType &right) { return left.name() < right.name(); });
        // Create a map for quick event-type to EventType object.
        std::map<std::string, EventType*> eventtype_map;
        for (auto eventtype = _event_types.begin(); eventtype != _event_types.end(); ++eventtype) eventtype_map[eventtype->name()] = &(*eventtype);
        // Determine lowest date for displayed events - for prospective space-time, we're excluding those below cluster window.
        UInt jyear, jmonth, jday;
        //std::map<std::string, boost::shared_ptr<std::stringstream>> group_events;
        for (size_t idx=0; idx < _dataHub.GetNumDataSets(); ++idx) {
            // First test whether this data set reported event id and coordinates.
            if (!(demographics.getEventStatus(idx).get<0>() && demographics.getEventStatus(idx).get<1>())) continue;
            // Open the data source and read all the records of the case file again.
            std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(
                getFilenameFormatTime(parameters.GetCaseFileName(idx + 1), parameters.getTimestamp(), true),
                parameters.getInputSource(CASEFILE, idx + 1), _dataHub.GetPrintDirection()
            ));
            if (Source->getLinelistFieldsMap().size() == 0) continue; // no mappings defined for this data set.
            // Iterate over the records of the case file - creating event types and event entries;
            const char * value = 0;
            tract_t tid; count_t count; Julian case_date;
            std::string event_date, event_id, latitude, longitude, status;
            std::stringstream eventtypes;
            std::vector<std::pair<std::string, std::string>> eventAttrs;
            while (Source->ReadRecord()) {
                DataSetHandler::RecordStatusType readStatus = _dataHub.GetDataSetHandler().RetrieveLocationIndex(*Source, tid); //read and validate that tract identifier
                if (readStatus != DataSetHandler::Accepted) continue; // Should only be either Accepted or Ignored.
                readStatus = _dataHub.GetDataSetHandler().RetrieveCaseCounts(*Source, count);
                if (readStatus != DataSetHandler::Accepted) continue; // Should only be either Accepted or Ignored.
                readStatus = _dataHub.GetDataSetHandler().RetrieveCountDate(*Source, case_date);
                if (readStatus != DataSetHandler::Accepted) continue; // Should only be either Accepted or Ignored.
                // Compile event attributes from this record.
                eventtypes.str("");
                eventAttrs.clear();
                for (auto itr=Source->getLinelistFieldsMap().begin(); itr != Source->getLinelistFieldsMap().end(); ++itr) {
                    value = Source->GetValueAtUnmapped(itr->first);
                    value = value == 0 ? "" : value;
                    if (itr->second.get<0>() == EVENT_ID)
                        event_id = value;
                    else if (itr->second.get<0>() == EVENT_COORD_Y)
                        latitude = value;
                    else if (itr->second.get<0>() == EVENT_COORD_X)
                        longitude = value;
                    else {
                        // If this event column is one of those being grouped, add the value to the event-types categories.
                        auto eventtype = eventtype_map.find(itr->second.get<1>());
                        if (eventtype != eventtype_map.end()) {
                            auto category = eventtype->second->addCategory(value);
                            if (eventtypes.tellp()) eventtypes << ",";
                            eventtypes << "'" << eventtype->second->className() << "': '" << category.get<0>() << "'";
                        }
                        eventAttrs.push_back(std::make_pair(itr->second.get<1>(), value));
                    }
                }
                // At least the minimal checking - confirm that event_id, coordinates and group value are present in record.
                if (event_id.length() == 0 || latitude.length() == 0 || longitude.length() == 0) {
                    _dataHub.GetPrintDirection().Printf("Unable to place event of record %ld in case file to Google Map.\n", BasePrint::P_READERROR, Source->GetCurrentRecordIndex());
                    continue;
                }
                double dlat, dlong;
                if (!string_to_type<double>(latitude.c_str(), dlat) || !string_to_type<double>(longitude.c_str(), dlong) || fabs(dlat) > 90.0 && fabs(dlong) > 180.0) {
                    _dataHub.GetPrintDirection().Printf("Unable to placemark event of record %ld in case file to Google Map.\n", BasePrint::P_READERROR, Source->GetCurrentRecordIndex());
                    continue;
                }
                // Determine status of this event.
                if (demographics.isNewEvent(event_id))
                    status = "new";
                else if (demographics.isExistingEvent(event_id))
                    status = "ongoing";
                else
                    status = "outside";
                // Write event definition to collection of events.
                if (_event_definitions.tellp()) _event_definitions << ",";
                _event_definitions << std::endl << "{ eventid: '" << event_id << "', status: '" << status << "', marker: null, coordinates: [" << longitude << "," << latitude << "], ";
                if (parameters.GetPrecisionOfTimesType() == GENERIC) {
                    _event_definitions << "date: " << JulianToString(event_date, case_date, parameters.GetPrecisionOfTimesType(), "-", false, false, true) << ",";
                } else {
                    JulianToMDY(&jmonth, &jday, &jyear, case_date);
                    _event_definitions << "date: new Date(" << jyear << ", " << (jmonth - 1) << ", " << jday << "),";
                }
                if (eventtypes.tellp()) { _event_definitions << eventtypes.str(); }
                _event_definitions << ", info: '<div style=\"padding:5px;\"><div style=\"text-decoration:underline;margin-bottom:3px;\">" << event_id << "</div>Event Date: " << JulianToString(event_date, case_date, parameters.GetPrecisionOfTimesType()) << "<br>";
                std::sort(eventAttrs.begin(), eventAttrs.end(), [](const std::pair<std::string, std::string> &left, const std::pair<std::string, std::string> &right) {
                    return left.first < right.first;
                });
                for (auto attr = eventAttrs.begin(); attr != eventAttrs.end(); ++attr) {
                    _event_definitions << attr->first << ": " << attr->second << "<br>";
                }
                _event_definitions << "</div>'}";
            }
        }
    }
}

/** Render scatter chart to html page. */
void ClusterMap::finalize() {
    std::string buffer;
    std::stringstream html, worker;
    FileName fileName;
    const CParameters& params = _dataHub.GetParameters();

    try {
        fileName.setFullPath(params.GetOutputFileName().c_str());
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
        worker.str("");
		for (auto location: _dataHub.GetGroupInfo().getLocationsManager().locations()) {
            if (!_cluster_locations.test(location->index())) {
                std::pair<double, double> prLatitudeLongitude(ConvertToLatLong(location.get()->coordinates()->retrieve(vCoordinates)));
                worker << printString(buffer, "[%f, %f],", prLatitudeLongitude.second, prLatitudeLongitude.first).c_str();
            }
        }
        std::string entire_points = worker.str();
        // site resource link path
        buffer = _cluster_definitions.str();
        templateReplace(html, "--cluster-definitions--", trimString(buffer, ",\n").c_str());
        templateReplace(html, "--entire-region-points--", trimString(trimString(entire_points, ","), ",").c_str());
        std::stringstream options_group;
        if (_cluster_options_significant.tellp()) {
            options_group << "<optgroup label='Significant' class='significant_clusters'>" << _cluster_options_significant.str() << "</optgroup>";
        }
        templateReplace(html, "--significant-cluster-options--", options_group.str());
        options_group.str("");
        if (_cluster_options_non_significant.tellp()) {
            options_group << "<optgroup label='Non-Significant' class='non_significant_clusters'>" << _cluster_options_non_significant.str() << "</optgroup>";
        }
        templateReplace(html, "--non-significant-cluster-options--", options_group.str());
        // Write any event types and definitions.
        std::stringstream selecteventgroups;
        for (auto eventtype = _event_types.begin(); eventtype != _event_types.end(); ++eventtype) {
            selecteventgroups << "<optgroup label='" << eventtype->name() << "' class='" << eventtype->className() << "'>";
            eventtype->sortCategories();
            for (auto category = eventtype->getCategories().begin(); category != eventtype->getCategories().end(); ++category)
                selecteventgroups << "<option value='" << category->get<0>() << "' class='" << eventtype->className() << "'>" << category->get<1>() << " (" << category->get<3>() << ")" << "</option>";
            selecteventgroups << "</optgroup>";
        }
        templateReplace(html, "--select-groups-event-type-display--", selecteventgroups.str());
        templateReplace(html, "--select-groups-event-type-exclude--", selecteventgroups.str());
        std::stringstream event_type_definitions;
        std::string website = AppToolkit::getToolkit().GetWebSite();
        for (auto eventtype=_event_types.begin(); eventtype != _event_types.end(); ++eventtype) {
            event_type_definitions << std::endl << eventtype->toJson(website) << ((eventtype + 1) == _event_types.end() ? "" : ",");
        } event_type_definitions << std::endl;
        templateReplace(html, "--event-types-definitions--", event_type_definitions.str());
        templateReplace(html, "--event-definitions--", _event_definitions.str());

        UInt jyear, jmonth, jday;
        if (params.GetPrecisionOfTimesType() == GENERIC) {
            templateReplace(html, "--event-range-begin--", JulianToString(buffer, _dataHub.GetStudyPeriodStartDate(), params.GetPrecisionOfTimesType()));
            templateReplace(html, "--event-range-start--", JulianToString(buffer, _recent_startdate, params.GetPrecisionOfTimesType()));
            templateReplace(html, "--event-range-end--", JulianToString(buffer, _dataHub.GetStudyPeriodEndDate(), params.GetPrecisionOfTimesType()));
            templateReplace(html, "--true-dates--", "false");
        } else {
            JulianToMDY(&jmonth, &jday, &jyear, _dataHub.GetStudyPeriodStartDate());
            templateReplace(html, "--event-range-begin--", printString(buffer, "new Date(%u, %u, %u)", jyear, jmonth - 1, jday));
            JulianToMDY(&jmonth, &jday, &jyear, _recent_startdate);
            templateReplace(html, "--event-range-start--", printString(buffer, "new Date(%u, %u, %u)", jyear, jmonth - 1, jday));
            JulianToMDY(&jmonth, &jday, &jyear, _dataHub.GetStudyPeriodEndDate());
            templateReplace(html, "--event-range-end--", printString(buffer, "new Date(%u, %u, %u)", jyear, jmonth - 1, jday));
            templateReplace(html, "--true-dates--", "true");
        }

        // replace parameters hash
        printString(buffer, "scanrate:%d/*high=1,low=2,highorlow=3*/,giniscan:%s", _dataHub.GetParameters().GetAreaScanRateType(),(_dataHub.GetParameters().getReportGiniOptimizedClusters() ? "true": "false"));
        templateReplace(html, "--parameters--", buffer.c_str());
        templateReplace(html, "--satscan-version--", AppToolkit::getToolkit().GetVersion());
		templateReplace(html, "--cluster-display--", std::string(_dataHub.GetParameters().getUseLocationsNetworkFile() ? "Circles/Edges" : "Circles"));

        HTMLout << html.str() << std::endl;
        HTMLout.close();
    } catch (prg_exception& x) {
        x.addTrace("renderScatterChart()", "ClusterScatterChart");
        throw;
    }
}

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

////////////////////////////// EventType ///////////////////////////

unsigned int EventType::_counter = 0;

/* EventType constructor */
EventType::EventType(const std::string& name) {
    ++_counter; // increment counter to tract number of event types
    unsigned int idx = _counter; // since _counter is static, concurrent analyses could cause strange behavior
    _name = name;
    printString(_class, "event_type_%u", idx);
    _type = formatTypeString(name, idx);
}

// Attempts to add new category to event collection.
const EventType::CategoryTuple_t& EventType::addCategory(const std::string& category_label) {
    for (auto& category: _categories) { // does this category label already exist?
        if (category.get<1>() == category_label) {
            category.get<2>() += 1; // increment the number of times this category was encountered
            return category;
        }
    }
    _categories.push_back(CategoryTuple_t(formatTypeString(category_label, _categories.size() + 1), category_label, 1));
    return _categories.back();
}

/* Formats string for use as javascript hash string. */
std::string EventType::formatTypeString(const std::string& str, unsigned int offset) {
    std::stringstream text;
    for (size_t pos = 0; pos != str.size(); ++pos) {
        if (std::ispunct(static_cast<unsigned char>(str[pos])) || str[pos] == ' ')
            text << "_";
        else
            text << str[pos];
    }
    text << "_" << offset; // just to prevent possible conflicts with replacement (e.g. 't@est' -> 't_st' and 't%est' -> 't_st')
    std::string return_value = text.str();
    return lowerString(return_value);
}

/* Returns event type as json formatted string. */
std::string& EventType::toJson(std::string& json_str) {
    std::string buffer;
    std::stringstream json;
    json << "{ event_class: '" << _class << "', " << "event_type: '" << _type << "', " << "event_name: '" << htmlencode(_name, buffer) << "', ";
    json << "categories: [";
    for (auto itr = _categories.begin(); itr != _categories.end(); ++itr) {
        json << (itr == _categories.begin() ? "" : ", ") << "{type: '" << _class << "-" << itr->get<0>() << "', label: '" << htmlencode(itr->get<1>(), buffer) << "'}";
    }
    json << "] }";
    json_str = json.str();
    return json_str;
}

/* Sorts categories by times encountered, falling back to label as tie breaker. */
const EventType::CategoriesContainer_t& EventType::sortCategories() {
    std::sort(_categories.begin(), _categories.end(), [](const CategoryTuple_t &left, const CategoryTuple_t &right) {
        if (left.get<2>() == right.get<2>()) return left.get<1>() < right.get<1>();
        return left.get<2>() > right.get<2>();
    });
    return _categories;
}


////////////////////////////// ClusterMap ///////////////////////////

const char * ClusterMap::HTML_FILE_EXT = ".html";
const char * ClusterMap::FILE_SUFFIX_EXT = ".clustermap";

const char * ClusterMap::TEMPLATE = " \
<!DOCTYPE html> \n \
<html lang=\"en\"> \n \
    <head> \n \
        <title>Cluster Map</title> \n \
        <meta name=\"viewport\" content=\"initial-scale=1.0, user-scalable=no\" charset=\"utf-8\"> \n \
        <link href=\"--resource-path--javascript/bootstrap/3.4.1/bootstrap.min.css\" rel=\"stylesheet\"> \n \
        <link rel=\"stylesheet\" href=\"--resource-path--javascript/clustercharts/nouislider.css\"> \n \
        <link rel=\"stylesheet\" href=\"--resource-path--javascript/clustercharts/mapgoogle-1.0.css\"> \n \
        <script type=\"text/javascript\" src=\"--resource-path--javascript/jquery/jquery-1.12.4/jquery-1.12.4.js\"></script> \n \
        <script type=\"text/javascript\" src=\"--resource-path--javascript/clustercharts/jQuery.resizeEnd.js\"></script> \n \
        <script type=\"text/javascript\" src=\"--resource-path--javascript/bootstrap/3.4.1/bootstrap.min.js\"></script> \n \
        <link rel=\"stylesheet\" href=\"--resource-path--javascript/bootstrap/bootstrap-multiselect/bootstrap-multiselect.css\"> \n \
        <script src=\"--resource-path--javascript/bootstrap/bootstrap-multiselect/bootstrap-multiselect.js\"></script> \n \
        <script src=\"--resource-path--javascript/clustercharts/nouislider.js\"></script> \n \
        <script src=\"--resource-path--javascript/clustercharts/OverlappingMarkerSpiderfier_1.0.3_oms.min.js\"></script> \n \
    </head> \n \
    <body> \n \
		<div id='load_error'></div> \n \
    <div class='container-fluid main-content'> \n \
        <div class='row'> \n \
            <div id='map-outer' class='col-md-12'> \n \
            <div class='col-md-2 chart-options-section'> \n \
                <fieldset> \n \
                <div class='options-row'> \n \
                    <div class='version'>Generated with SaTScan v--satscan-version--</div>\n \
                </div>\n \
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
                        <label><input type='radio' class='standard' name='view_rate' id='id_view_highlow' value='entire' checked=checked />High and low clusters</label>\n \
                        <label><input type='radio' class='standard' name='view_rate' id='id_view_high' value='cluster'/>High only</label>\n \
                        <label><input type='radio' class='standard' name='view_rate' id='id_view_low' value='cluster'/>Low only</label>\n \
                        <p class='help-block'>Toggle display of clusters for scan rate.</p>\n \
                    </div> \n \
                    <div id='id_secondary_clusters_option'> \n \
                        <div>Secondary Clusters:</div>\n \
                        <label><input type='checkbox' id='id_hierarchical' value='secondary' />Hierarchical</label>\n \
                        <label><input type='checkbox' id='id_gini' value='secondary' />Gini</label>\n \
                        <p class='help-block'>Display options for secondary clusters.</p>\n \
                    </div> \n \
                    <div>Show clusters using:</div>\n \
                    <label class='cluster-show-opts'><input type='checkbox' id='id_cluster_circles' value='cluster' checked=checked />--cluster-display--</label>\n \
                    <label class='cluster-show-opts'><input type='checkbox' id='id_cluster_locations' value='cluster' />Locations</label>\n \
                    <p class='help-block'>Display options for clusters.</p>\n \
                    <label><input type='checkbox' id='id_show_location_points' />Show all location points</label>\n \
                    <p class='help-block'>Toggle display of location points.</p>\n \
                    <label><input type='checkbox' id='id_show_all_edges' />Show all network edges</label>\n \
                    <p class='help-block'>Toggle display of all network edges.</p>\n \
                </div> \n \
                <div class='options-row event-controls'> \n \
                    <div id='id_display_events'> \n \
                        <label for='id_select_event_type'>Display Individuals By:</label> \n \
                        <select name='select_event_type' id='id_select_event_type' multiple='multiple' class='events-select'> \n \
                        --select-groups-event-type-display-- \n \
                        </select> \n \
                    </div> \n \
                    <p class='help-block'>Displays markers for selected groups.</p> \n \
                    <div id='id_group_events'>  \n \
                    <label for='id_group_event_type'>Separate Icons:</label> \n \
                    <select name='group_event_type' id='id_group_event_type' multiple='multiple' class='events-group'></select> \n \
                    </div> \n \
                    <p class='help-block'>Maximum of 8 can be distinguished by icon.</p> \n \
                    <div id='id_filter_events'> \n \
                        <label for='id_filter_event_type'>Exclude Individuals:</label> \n \
                        <select name='filter_event_type' id='id_filter_event_type' multiple='multiple' class='events-filter'> \n \
                        --select-groups-event-type-exclude-- \n \
                        </select> \n \
                    </div> \n \
                    <p class='help-block'>Filter to exclude markers of displayed individuals.</p> \n \
                    <label for='slider_display'>Individuals In Period</label> \n \
                    <div class='slider-styled slider-round' id='slider_display'></div> \n \
                    <div class='slider_display_range'><span id='id_range_startdate'>11/23/2021</span> to <span id='id_range_enddate'>3/2/2022</span></div> \n \
                    <div class='pull-left'> \n \
                    <button type='button' class='btn btn-success btn-sm' id='id_run_timeline'>Run</button> \n \
                    <button type='button' class='btn btn-primary btn-sm' id='id_pause_timeline'>Pause</button> \n \
                    <button type='button' class='btn btn-secondary btn-sm btn-warning' id='id_run_timeline_reset'>Reset</button> \n \
                    </div> \n \
                    <div class='clearfix'></div> \n \
                    <label for='slider_speed'>Run Delay: <span id='id_timeline_rate'>50</span> milliseconds</label> \n \
                    <div class='slider-styled slider-round-small' id='slider_speed'></div> \n \
                    <label for='slider_display'>Recent Individuals</label> \n \
                    <div class='slider-styled slider-round' id='slider_recent'></div> \n \
                    <div id='id_recent_range'>Recent as of <span id='id_range_recent'>11/23/2021</span></div> \n \
                    <label><input type='checkbox' id='id_show_legend' checked=checked />Display Individuals Legend</label> \n \
                    <div id='id_icon_size'><label>Individuals Icon Size</label></div> \n \
                     <div class='slider-styled slider-round' id='slider_size' ></div> \n \
                     <div id='id_icon_range'><span>&#129048; smaller</span><span>larger &#x1F81A;</span></div> \n \
                    <div class='clearfix'></div> \n \
                </div> \n \
                <div id='id_display_count'>\n \
                    <fieldset>\n \
                            <legend>Display Data:</legend>\n \
                            <div><span id='id_cluster_count'></span> Clusters</div>\n \
                            <div><span id='id_cluster_point_count'></span> Cluster Locations</div>\n \
                            <div><span id='id_point_count'></span> Total Locations</div> \n \
                            <div><span id='id_event_count'>0</span> Total Individuals</div> \n \
                    </fieldset>\n \
                </div>\n \
                <div class='options-row'> \n \
                    <label class='option-section print' for='title_obs'>Print</label> \n \
                    <div class='print-section'> \n \
                        <a href='#' onclick='javascript:window.print();return false;'><span aria-hidden='true'></span> Print</a> \n \
                    </div> \n \
                </div> \n \
                </fieldset> \n \
            </div> \n \
            <div class='xx-col-md-10 chart-column' id='map'></div> \n \
            <div id='legend'><h3>Legend</h3></div> \n \
            <div id='cursor_position'></div> \n \
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
            var entire_region_edges = [--entire-region-edges--]; \n \
            var display_stats = {};\n \
            var clusters = [--cluster-definitions--]; \n \
            clusters.reverse();\n \
            var resource_path = '--resource-path--'; \n \
    </script> \n \
    <script src=\"--resource-path--javascript/clustercharts/mapgoogle-1.3.2.js\"></script> \n \
  </body> \n \
</html> \n";

ClusterMap::ClusterMap(const CSaTScanData& dataHub) :_dataHub(dataHub), _clusters_written(0),
    _recent_startdate(dataHub.GetParameters().GetIsProspectiveAnalysis() ? dataHub.GetTimeIntervalStartTimes().at(dataHub.getDataInterfaceIntervalStartIndex()) : dataHub.GetTimeIntervalStartTimes().front()) { 
    _cluster_locations.resize(_dataHub.getLocationsManager().locations().size());
}

/** Updates passed FileName object to name of file generated by this class. */
FileName& ClusterMap::getFilename(FileName& filename) {
    std::string buffer;
    filename.setFileName(printString(buffer, "%s%s", filename.getFileName().c_str(), FILE_SUFFIX_EXT).c_str());
    filename.setExtension(HTML_FILE_EXT);
    return filename;
}

/** Returns cluster legend to be used as popup in html page. */
std::string & ClusterMap::getClusterLegend(const CCluster& cluster, int iCluster, std::string& legend) const {
    std::stringstream  lines;
    unsigned int currSetIdx = std::numeric_limits<unsigned int>::max(), numFilesSets = _dataHub.GetParameters().getNumFileSets();

    lines << "<div style=\"text-decoration:underline;\">Cluster " << iCluster + 1 << "</div>";
    for (const auto& ci: cluster.getReportLinesCache()) {
        if (numFilesSets > 1 && ci.second.second > 0 && currSetIdx != ci.second.second) {
            lines << "Data Set " << ci.second.second << "<br>";
            currSetIdx = ci.second.second;
        }
        lines << ci.first << " : " << ci.second.first << "<br>";
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
            const auto& locations = _dataHub.getLocationsManager().locations();
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
            _cluster_definitions << "{ id: " << (i + iteration) << ", slider_value : " << VisualizationUtils::getSliderValue(_dataHub, cluster, i + iteration, simVars)
                << ", highrate : " << (cluster.getAreaRateForCluster(_dataHub) == HIGH ? "true" : "false") << ", " << buffer
                << ", hierarchical : " << (cluster.isHierarchicalCluster() ? "true" : "false") << ", gini : " << (cluster.isGiniCluster() ? "true" : "false")                
                << ", color : '" << (cluster.getAreaRateForCluster(_dataHub) == HIGH ? "#F13C3F" : "#5F8EBD") << "', pointscolor : '" << (cluster.getAreaRateForCluster(_dataHub) == HIGH ? "#FF1A1A" : "#1AC6FF") 
                << "', tip : '" << getClusterLegend(cluster, i + clusterOffset, legend).c_str() << "', edges : [" << edges << "], points : [" << points << "] },\n";
            _cluster_options << "<option value=" << (i + iteration) << " " << (i == 0 ? "selected" : "") << ">Cluster " << (i + iteration) << "</option>";
        }
        ++_clusters_written;
    }
}

/* Adds case demographic markers to Google Map. */
void ClusterMap::add(const DataDemographicsProcessor& demographics) {
    // First test whether any of the data sets include indivdual and descriptive coordinates.
    if (!demographics.hasIndividualGeographically()) {
        _dataHub.GetPrintDirection().Printf(
            "Descriptive coordinates were not found in line list data. Line list individuals will not be added to Google Map file.\n", BasePrint::P_WARNING
        );
        return;
    }
    // Create collection of event types which we'll be reporting in KML file.
    // The event types don't have to be matching between data sets - they can match completely, partially, or not at all.
    for (size_t idx = 0; idx < _dataHub.GetNumDataSets(); ++idx) {
        const auto& demographic_set = demographics.getDataSetDemographics(idx);
        if (!demographic_set.hasIndividualGeographically()) continue; // skip data sets w/o individual and descriptive lat/long.
        for (auto const &demographic : demographic_set.getAttributes()) {
            if (demographic.second->gettype() <= DESCRIPTIVE_COORD_X) continue; // skip individual id and descriptive coordinates.
            if (!demographic.second->reportedInVisualizations()) { //skip if excluded from visualizations.
                _dataHub.GetPrintDirection().Printf(
                    "Excluding line list attribute '%s' from Google Map file.\nAttribute has too many group values for reporting in Map output.\n",
                    BasePrint::P_WARNING, demographic.second->label().c_str()
                );
                continue;
            }
            if (std::find_if(_event_types.begin(), _event_types.end(), [&demographic](const EventType& et) { return et.name() == demographic.first.second; }) == _event_types.end())
                _event_types.push_back(EventType(demographic.first.second));
        }
    }
    if (_event_types.size() == 0) {
        _dataHub.GetPrintDirection().Printf("No line list attributes to display individuals by. Individuals were not added to Google Map file.\n",  BasePrint::P_WARNING);
        return;
    }
    bool storeWarn = _dataHub.GetPrintDirection().isSuppressingWarnings(); // prevent re-printing case file warnings
    _dataHub.GetPrintDirection().SetSuppressWarnings(true);
    const CParameters& parameters = _dataHub.GetParameters();
    // Sort so event types are reported alphabetically.
    std::sort(_event_types.begin(), _event_types.end(), [](const EventType &left, const EventType &right) { return left.name() < right.name(); });
    // Create a map for quick event-type to EventType object access.
    std::map<std::string, EventType*> eventtype_map;
    for (auto eventtype = _event_types.begin(); eventtype != _event_types.end(); ++eventtype) eventtype_map[eventtype->name()] = &(*eventtype);
    UInt jyear, jmonth, jday;
    for (size_t idx=0; idx < _dataHub.GetNumDataSets(); ++idx) {
        if (!demographics.getDataSetDemographics(idx).hasIndividualGeographically()) continue; // skip data sets w/o individual and descriptive lat/long.
        std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(
            getFilenameFormatTime(parameters.GetCaseFileName(idx + 1), parameters.getTimestamp(), true),
            parameters.getInputSource(CASEFILE, idx + 1), _dataHub.GetPrintDirection()
        ));
        if (Source->getLinelistFieldsMap().size() == 0) continue; // skip this data set if no line-list mappings defined
        // Iterate over the records of the case file - creating event types and event entries.
        const char * value = 0;
        tract_t tid; count_t count; Julian case_date;
        std::string event_date, individual, latitude, longitude, status, buffer;
        std::stringstream event_types;
        std::vector<std::pair<std::string, std::string>> event_attrs;
        std::set<const EventType*> unseen_events; // tracks which event types haven't been seen in this record
        while (Source->ReadRecord()) {
            DataSetHandler::RecordStatusType readStatus = _dataHub.GetDataSetHandler().RetrieveIdentifierIndex(*Source, tid); //read and validate that tract identifier
            if (readStatus != DataSetHandler::Accepted) continue; // should only be either Accepted or Ignored since we have already read this file
            readStatus = _dataHub.GetDataSetHandler().RetrieveCaseCounts(*Source, count);
            if (readStatus != DataSetHandler::Accepted) continue; // should only be either Accepted or Ignored since we have already read this file
            readStatus = _dataHub.GetDataSetHandler().RetrieveCountDate(*Source, case_date);
            if (readStatus != DataSetHandler::Accepted) continue; // should only be either Accepted or Ignored since we have already read this file
            // Compile individual attributes from this record.
            event_types.str("");
            event_attrs.clear();
            unseen_events.clear(); // clear and reset unseen attributes
            for (auto eventtype = _event_types.begin(); eventtype != _event_types.end(); ++eventtype) unseen_events.emplace(&(*eventtype));
            // iterate over data source line list mappings
            for (const auto& llfm: Source->getLinelistFieldsMap()) {
                value = Source->GetValueAtUnmapped(llfm.get<0>());
                value = value == 0 ? "" : value;
                if (llfm.get<1>() == INDIVIDUAL_ID)
                    individual = value;
                else if (llfm.get<1>() == DESCRIPTIVE_COORD_Y)
                    latitude = value;
                else if (llfm.get<1>() == DESCRIPTIVE_COORD_X)
                    longitude = value;
                else {
                    // If this column is one of those being grouped, add the value to the event-type category.
                    value = strlen(value) == 0 ? "~ blank ~" : value;
                    auto eventtype = eventtype_map.find(llfm.get<2>());
                    if (eventtype != eventtype_map.end()) {
                        const auto& category = eventtype->second->addCategory(value);
                        if (event_types.tellp()) event_types << ",";
                        event_types << "'" << eventtype->second->className() << "': '" << eventtype->second->className() << "-" << category.get<0>() << "'";
                        unseen_events.erase(eventtype->second); // mark this event type as seen in this record
                    }
                    event_attrs.push_back(std::make_pair(llfm.get<2>(), value));
                }
            }
            // At least the minimal checking - confirm that individual, coordinates and group value are present in record.
            if (individual.length() == 0 || latitude.length() == 0 || longitude.length() == 0) { // warn and skip record if descriptive coordinates don't cast or are invalid
                _dataHub.GetPrintDirection().Printf("Unable to place individual of record %ld in case file to Google Map.\n", BasePrint::P_WARNING, Source->GetCurrentRecordIndex());
                continue;
            }
            double dlat, dlong; // warn and skip record if descriptive coordinates don't cast or are invalid
            if (!string_to_type<double>(latitude.c_str(), dlat) || !string_to_type<double>(longitude.c_str(), dlong) || fabs(dlat) > 90.0 && fabs(dlong) > 180.0) {
                _dataHub.GetPrintDirection().Printf("Unable to place individual of record %ld in case file to Google Map.\n", BasePrint::P_WARNING, Source->GetCurrentRecordIndex());
                continue;
            }
            // Determine status of this individual.
            if (demographics.isNewIndividual(individual))
                status = "new";
            else if (demographics.inCluster(tid, case_date) && demographics.isExistingIndividual(individual))
                status = "ongoing";
            else
                status = "outside";
            // write the individual/event hash to the javascript collection
            if (_event_definitions.tellp()) _event_definitions << ","; // add comma if this isn't the first one
            _event_definitions << std::endl << "{ eventid: '" << individual << "', status: '" << status << "', marker: null, coordinates: [" << longitude << "," << latitude << "], ";
            if (parameters.GetPrecisionOfTimesType() == GENERIC) {
                _event_definitions << "date: " << JulianToString(event_date, case_date, parameters.GetPrecisionOfTimesType(), "-", false, false, true) << ",";
            } else {
                JulianToMDY(&jmonth, &jday, &jyear, case_date);
                _event_definitions << "date: new Date(" << jyear << ", " << (jmonth - 1) << ", " << jday << "),";
            }
            for (auto& unseen : unseen_events) { // now add unseen event types so that each record in the javascript hash is complete
                if (event_types.tellp()) event_types << ",";
                event_types << "'" << unseen->className() << "': '" << unseen->className() << "-not-applicable-'";
            }
            if (event_types.tellp()) { _event_definitions << event_types.str(); }
            _event_definitions << ", info: '<div style=\"padding:5px;\"><div style=\"text-decoration:underline;margin-bottom:3px;\">" << individual << "</div>Date: " << JulianToString(event_date, case_date, parameters.GetPrecisionOfTimesType()) << "<br>";
            std::sort(event_attrs.begin(), event_attrs.end(), [](const std::pair<std::string, std::string> &left, const std::pair<std::string, std::string> &right) {
                return left.first < right.first;
            });
            for (auto& attr: event_attrs) {
                _event_definitions << htmlencode(attr.first, buffer) << ": " << htmlencode(attr.second, buffer) << "<br>";
            }
            _event_definitions << "</div>'}";
        }
    }
    _dataHub.GetPrintDirection().SetSuppressWarnings(storeWarn);
}

/** Finalize generating the html page which will contain the Google Map, along with supporting html inputs and javascript variables. */
void ClusterMap::finalize() {
    std::string str_buffer, website(AppToolkit::getToolkit().GetWebSite());
    std::stringstream html, stream_buffer;
    const CParameters& params = _dataHub.GetParameters();
    std::vector<double> coordinates;
    std::ofstream html_out;

    try {
        FileName fileName(params.GetOutputFileName().c_str());
        getFilename(fileName);
        //open output file
        html_out.open(fileName.getFullPath(str_buffer).c_str());
        if (!html_out) throw resolvable_error("Error: Could not open file '%s'.\n", fileName.getFullPath(str_buffer).c_str());
        html << TEMPLATE << std::endl; // read template into stringstream
        templateReplace(html, "--resource-path--", AppToolkit::getToolkit().GetWebSite()); // site resource link path
        templateReplace(html, "--tech-support-email--", AppToolkit::getToolkit().GetTechnicalSupportEmail()); // site resource link path
        str_buffer = _cluster_definitions.str(); // write cluster definitions
        templateReplace(html, "--cluster-definitions--", trimString(str_buffer, ",\n").c_str());
        // If we're using a network, create collection of network connections - we'll use them to display edges in the entire network.
        if (params.getUseLocationsNetworkFile()) {
            Network::Connection_Details_t connections = GisUtils::getNetworkConnections(_dataHub.refLocationNetwork());
            for (const auto& connection : GisUtils::getNetworkConnections(_dataHub.refLocationNetwork())) {
                connection.get<0>()->coordinates()->retrieve(coordinates);
                std::pair<double, double> prLatitudeLongitude(ConvertToLatLong(coordinates));
                stream_buffer << printString(str_buffer, "[[%f, %f],", prLatitudeLongitude.second, prLatitudeLongitude.first).c_str();
                connection.get<1>()->coordinates()->retrieve(coordinates);
                prLatitudeLongitude = ConvertToLatLong(coordinates);
                stream_buffer << printString(str_buffer, "[%f, %f]],", prLatitudeLongitude.second, prLatitudeLongitude.first).c_str();
            }
        }
        str_buffer = stream_buffer.str();
        templateReplace(html, "--entire-region-edges--", trimString(trimString(str_buffer, ","), ",").c_str());
        // create collection of all the location in the analysis
        stream_buffer.str("");
        for (const auto& location: _dataHub.getLocationsManager().locations()) {
            if (_cluster_locations.test(location->index())) continue; // skip locations which will already be represented with a cluster
            std::pair<double, double> prLatitudeLongitude(ConvertToLatLong(location.get()->coordinates()->retrieve(coordinates)));
            stream_buffer << printString(str_buffer, "[%f, %f],", prLatitudeLongitude.second, prLatitudeLongitude.first).c_str();
        }
        str_buffer = stream_buffer.str();
        templateReplace(html, "--entire-region-points--", trimString(trimString(str_buffer, ","), ",").c_str());
        // write clusters option
        stream_buffer.str("");
        if (_cluster_options.tellp())
            stream_buffer << _cluster_options.str();
        templateReplace(html, "--cluster-options--", stream_buffer.str());
        // write individual (event) option groups
        stream_buffer.str("");
        for (auto& eventtype: _event_types) {
            stream_buffer << "<optgroup label='" << htmlencode(eventtype.name(), str_buffer) << "' class='" << eventtype.className() << "'>";
            for (const auto& category: eventtype.sortCategories()) {
                stream_buffer << "<option value='" << eventtype.className() << "-" << category.get<0>() << "' class='" << eventtype.className() << "'>";
                stream_buffer << htmlencode(category.get<1>(), str_buffer) << " (" << category.get<2>() << ")" << "</option>";
            }
            stream_buffer << "</optgroup>";
        }
        templateReplace(html, "--select-groups-event-type-display--", stream_buffer.str());
        templateReplace(html, "--select-groups-event-type-exclude--", stream_buffer.str());
        stream_buffer.str("");
        for (auto eventtype=_event_types.begin(); eventtype != _event_types.end(); ++eventtype) {
            stream_buffer << std::endl << eventtype->toJson(str_buffer) << ((eventtype + 1) == _event_types.end() ? "" : ",");
        } stream_buffer << std::endl;
        templateReplace(html, "--event-types-definitions--", stream_buffer.str());
        templateReplace(html, "--event-definitions--", _event_definitions.str());
        UInt jyear, jmonth, jday; // write supporting javascript variables
        if (params.GetPrecisionOfTimesType() == GENERIC) {
            templateReplace(html, "--event-range-begin--", JulianToString(str_buffer, _dataHub.GetStudyPeriodStartDate(), params.GetPrecisionOfTimesType()));
            templateReplace(html, "--event-range-start--", JulianToString(str_buffer, _recent_startdate, params.GetPrecisionOfTimesType()));
            templateReplace(html, "--event-range-end--", JulianToString(str_buffer, _dataHub.GetStudyPeriodEndDate(), params.GetPrecisionOfTimesType()));
            templateReplace(html, "--true-dates--", "false");
        } else {
            JulianToMDY(&jmonth, &jday, &jyear, _dataHub.GetStudyPeriodStartDate());
            templateReplace(html, "--event-range-begin--", printString(str_buffer, "new Date(%u, %u, %u)", jyear, jmonth - 1, jday));
            JulianToMDY(&jmonth, &jday, &jyear, _recent_startdate);
            templateReplace(html, "--event-range-start--", printString(str_buffer, "new Date(%u, %u, %u)", jyear, jmonth - 1, jday));
            JulianToMDY(&jmonth, &jday, &jyear, _dataHub.GetStudyPeriodEndDate());
            templateReplace(html, "--event-range-end--", printString(str_buffer, "new Date(%u, %u, %u)", jyear, jmonth - 1, jday));
            templateReplace(html, "--true-dates--", "true");
        }
        templateReplace(html, "--parameters--", printString( // replace parameters hash
            str_buffer, "scanrate:%d/*high=1,low=2,highorlow=3*/,giniscan:%s,prospective:%s",
            params.GetAreaScanRateType(), params.getReportGiniOptimizedClusters() ? "true" : "false", params.GetIsProspectiveAnalysis() && !_dataHub.isDrilldown() ? "true" : "false"
        ));
        templateReplace(html, "--satscan-version--", AppToolkit::getToolkit().GetVersion());
		templateReplace(html, "--cluster-display--", std::string(_dataHub.GetParameters().getUseLocationsNetworkFile() ? "Circles/Edges" : "Circles"));
        html_out << html.str() << std::endl;
        html_out.close();
    } catch (prg_exception& x) {
        x.addTrace("finalize()", "ClusterMap");
        throw;
    }
}

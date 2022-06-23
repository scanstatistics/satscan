//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "DataDemographics.h"
#include "SimulationVariables.h"
#include "MostLikelyClustersContainer.h"
#include "DataSetHandler.h"
#include "FileName.h"
#include <numeric>
#include <algorithm>

///////////////////////// CategoricalDemographicAttribute //////////////////////////////////////////////////////////////////////

void CategoricalDemographicAttribute::add(const std::string& value, unsigned int times) {
    auto result = _category_counts.emplace(value, times);
    if (!result.second) { result.first->second += times; }
}

void CategoricalDemographicAttribute::print() const {
    unsigned int total = std::accumulate(
        std::begin(_category_counts), std::end(_category_counts), 0, 
        [](unsigned int value, const std::map<std::string, unsigned int>::value_type& p) { return value + p.second; }
    );
    printf("Name: %s, Total: %u\n", _label.c_str(), total);
    for (auto itr = _category_counts.begin(); itr != _category_counts.end(); ++itr)
        printf("'%s' - %u (%.0f%%)\n", itr->first.c_str(), itr->second, (static_cast<double>(itr->second) / static_cast<double>(total)) * 100);
}

///////////////////////// ContinuousDemographicAttribute //////////////////////////////////////////////////////////////////////

void ContinuousDemographicAttribute::add(const std::string& value, unsigned int times) {
    double d;
    if (!string_to_type<double>(value.c_str(), d))
        throw resolvable_error("Unable to cast '%s' to continuous variable type (double).", value.c_str());
    for (auto i=0; i < times; ++i) _values.push_back(d);
}

void ContinuousDemographicAttribute::print() const {
    if (_values.size() == 0) return;
    const auto minmaxpair = std::minmax_element(std::begin(_values), std::end(_values));
    const auto average = std::accumulate(std::begin(_values), std::end(_values), 0.0) / _values.size();
    printf("Name: %s\n", _label.c_str());
    printf("Average: %g (Min: %g, Max: %g)\n", average, *minmaxpair.first, *minmaxpair.second);
}

///////////////////////// DemographicAttributeSet //////////////////////////////////////////////////////////////////////

DemographicAttributeSet::DemographicAttributeSet(const LineListFieldMapContainer_t& llmap) {
    for (auto itr = llmap.begin(); itr != llmap.end(); ++itr) {
        switch (itr->second.get<0>()) {
            case EVENT_ID: _attributes_set[itr->second.get<1>()] = boost::shared_ptr<DemographicAttribute>(new GeneralDemographicAttribute(itr->second.get<1>(), EVENT_ID)); break;
            case EVENT_COORD_X: _attributes_set[itr->second.get<1>()] = boost::shared_ptr<DemographicAttribute>(new GeneralDemographicAttribute(itr->second.get<1>(), EVENT_COORD_X)); break;
            case EVENT_COORD_Y: _attributes_set[itr->second.get<1>()] = boost::shared_ptr<DemographicAttribute>(new GeneralDemographicAttribute(itr->second.get<1>(), EVENT_COORD_Y)); break;
            case GENERAL_DATA: _attributes_set[itr->second.get<1>()] = boost::shared_ptr<DemographicAttribute>(new GeneralDemographicAttribute(itr->second.get<1>())); break;
            case CATEGORICAL_DATA: _attributes_set[itr->second.get<1>()] = boost::shared_ptr<DemographicAttribute>(new CategoricalDemographicAttribute(itr->second.get<1>())); break;
            case CONTINUOUS_DATA: _attributes_set[itr->second.get<1>()] = boost::shared_ptr<DemographicAttribute>(new ContinuousDemographicAttribute(itr->second.get<1>())); break;
            default: throw prg_error("Unsupported line list type '%d'.", "DemographicAttributeSet()", itr->second.get<0>());
        }
    }
}

/* Attempts to retrieve DemographicAttribute for LinelistTuple_t. */
boost::shared_ptr<DemographicAttribute> DemographicAttributeSet::get(LinelistTuple_t llt) {
    auto itr = _attributes_set.find(llt.get<1>());
    if (itr == _attributes_set.end())
        throw prg_error("Line-list tuple not defined (%d, %s).", "DemographicAttributeSet()", llt.get<0>(), llt.get<1>().c_str());
    return itr->second;
}

bool DemographicAttributeSet::hasEventAttribute() const {
    for (auto attr : _attributes_set)
        if (attr.second->gettype() == EVENT_ID)
            return true;
    return false;
}

bool DemographicAttributeSet::hasEventCoordinatesAttributes() const {
    bool x = false, y = false;
    for (auto attr : _attributes_set) {
        x |= attr.second->gettype() == EVENT_COORD_X;
        y |= attr.second->gettype() == EVENT_COORD_Y;
    }
    return x && y;
}

///////////////////////////////////// DataDemographicsProcessor //////////////////////////////////////////

DataDemographicsProcessor::DataDemographicsProcessor(const DataSetHandler& handler) :
    _handler(handler), _clusters(0), _sim_vars(0), _parameters(handler.gDataHub.GetParameters()) {}

DataDemographicsProcessor::DataDemographicsProcessor(const DataSetHandler& handler, const MostLikelyClustersContainer& clusters, const SimulationVariables& sim_vars)
    :_handler(handler), _clusters(&clusters), _sim_vars(&sim_vars), _parameters(handler.gDataHub.GetParameters()) {

    // Iterate over the most likely clusters, creating structures that will facilitate compiling data.
    std::string buffer;
    for (int i = 0; i < _clusters->GetNumClustersRetained(); ++i) {
        _cluster_new_events[i] = std::make_pair(0, 0);
        const CCluster& cluster = _clusters->GetCluster(i);
        if (cluster.isSignificant(handler.gDataHub, i + 1, sim_vars)) {
            // Defined which locations are in each cluster, using bitset for quick search while iterating over case line list data rows.
            boost::dynamic_bitset<> locations(handler.gDataHub.GetNumTracts());
            std::vector<tract_t> tractIndexes;
            if (cluster.GetClusterType() != PURELYTEMPORALCLUSTER) {
                for (auto tractIdx : cluster.getLocationIndexes(handler.gDataHub, tractIndexes, true))
                    locations.set(tractIdx);
            }
            _cluster_locations[i] = locations;
            // Create collection of demographic attributes for this cluster.
            _cluster_demographics_by_dataset[i] = std::deque<DemographicAttributeSet>();
            // Create unique temporary filename to store linelist data for this cluster - we want the records grouped by cluster.
            _cluster_location_files[i] = GetUserTemporaryFilename(buffer);
        }
    }
    // Read event ids from file cache - these are events that signalled in prior analyses.
    if (boost::filesystem::exists(_parameters.getEventCacheFileName().c_str())) {
        std::ifstream event_stream;
        if (!event_stream.is_open()) event_stream.open(_parameters.getEventCacheFileName().c_str());
        if (!event_stream) throw resolvable_error("Error: Could not open file '%s' to read the prior signalling events.\n", _parameters.getEventCacheFileName().c_str());
        while (!event_stream.eof()) {
            std::getline(event_stream, buffer);
            if (trimString(buffer).size())
                _existing_event_ids.emplace(buffer);
        }
        event_stream.close();
    }
}

DataDemographicsProcessor::~DataDemographicsProcessor() {
    // Delete any temporary cluster files.
    for (auto cfiles : _cluster_location_files) {
        try {
            remove(cfiles.second.c_str());
        } catch (...) {}
    }
}

/* Appends data record to temporary cluster file. */
void DataDemographicsProcessor::appendLinelistData(int clusterIdx, std::vector<std::string>& data, boost::optional<int> first, unsigned int times) {
    std::ofstream temp_file(_cluster_location_files[clusterIdx].c_str(), std::ios_base::app | std::ios_base::binary);
    std::string buffer;
    std::stringstream line;
    line << (clusterIdx + 1) << ",";
    if (first) line << (first.get() == clusterIdx ? "Primary" : "Secondary") << ",";
    line << typelist_to_csv_string<std::string>(data, buffer) << std::endl;
    for (int i=0; i < times; ++i) 
        temp_file << line.str();
}

/* Add the header row to the final output file. */
void DataDemographicsProcessor::createHeadersFile(std::ofstream& linestream, const DataSource::OrderedLineListField_t& llmap) {
    std::vector<std::string> v = {"Cluster"};
    for (auto const& itr : llmap) {
        if (itr.second.get<0>() == EVENT_ID) {
            v.push_back("Hierarchy");
            v.push_back("New Event");
        }
        v.push_back(itr.second.get<1>());
    }
    std::string buffer;
    typelist_to_csv_string<std::string>(v, buffer);
    linestream << buffer << std::endl;
}

/* Re-reads cases file to accumulate line-list data inconjuction with detected clusters. */
bool DataDemographicsProcessor::processCaseFileLinelist(const RealDataSet& DataSet) {
    try {
        std::string buffer, buffer2;
        /* Open case file data source - input source should now have ncessary line-list mappings since they were assigned during initial case file read. */
        _handler.gPrint.SetImpliedInputFileType(BasePrint::CASEFILE);
        std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(
            getFilenameFormatTime(_handler.gParameters.GetCaseFileName(DataSet.getSetIndex()), _handler.gParameters.getTimestamp(), true),
            _handler.gParameters.getInputSource(CASEFILE, DataSet.getSetIndex()), _handler.gPrint)
        );
        // Create new demographics attrbite set for entire data set.
        _demographics_by_dataset.push_back(DemographicAttributeSet(Source->getLinelistFieldsMap()));
        // Record whether data source has event attributes.
        _events_by_dataset.push_back(boost::tuple<bool, bool>(Source->hasEventIdLinelistMapping(), Source->hasEventCoordinatesLinelistMapping()));
        /* Create temporary events cache to record all event discovered in this analysis. If the baseline period moves over time for this analysis
           then the event cache could become bloated with event ids no longer in baseline period. */
        std::ofstream events_stream;
        if (_events_by_dataset.back().get<0>() && _temp_events_cache_filename.empty()) {
            _temp_events_cache_filename = GetUserTemporaryFilename(buffer);
            events_stream.open(_temp_events_cache_filename, std::ios_base::app);
            _events_filter = getNewBloomFilter(_handler._approximate_case_records);
        }
        // Create  new demographics attrbite set for each cluster being reported.
        for (auto cluster : _cluster_demographics_by_dataset)
            _cluster_demographics_by_dataset[cluster.first].push_back(DemographicAttributeSet(Source->getLinelistFieldsMap()));

        // Iterate over case file records of this data set.
        boost::dynamic_bitset<> applicable(_cluster_locations.size());
        bool storeSetting = _handler.gPrint.isSuppressingWarnings();
        _handler.gPrint.SetSuppressWarnings(true);
        DataSource::OrderedLineListField_t linelistFieldsMap;
        Source->getOrderedLinelistFieldsMap(linelistFieldsMap);
        while (!_handler.gPrint.GetMaximumReadErrorsPrinted() && Source->ReadRecord()) {
            tract_t tid;
            if (_handler.RetrieveLocationIndex(*Source, tid) != DataSetHandler::Accepted) continue;
            count_t nCount;
            if (Source->GetValueAt(_handler.guCountIndex) == 0)
                return false;
            if (!string_to_type<count_t>(Source->GetValueAt(_handler.guCountIndex), nCount) || nCount < 0)
                return false;
            if (nCount == 0) continue;
            Julian nDate;
            if (_handler.RetrieveCountDate(*Source, nDate) != DataSetHandler::Accepted) continue;
            int startIdx = _handler.gDataHub.GetTimeIntervalOfDate(nDate);
            int endIdx = _handler.gDataHub.GetTimeIntervalOfEndDate(nDate);
            // Determine which clusters this record applys to.
            applicable.reset();
            for (auto cluster_locs : _cluster_locations) {
                const CCluster& cluster = _clusters->GetCluster(cluster_locs.first);
                if (cluster.GetClusterType() != PURELYTEMPORALCLUSTER && !cluster_locs.second.test(tid))
                    continue;
                if (startIdx >= cluster.m_nFirstInterval && endIdx <= cluster.m_nLastInterval)
                    applicable.set(cluster_locs.first);
            }
            boost::logic::tribool is_new_event(boost::logic::indeterminate);
            const char * value = 0; std::vector<std::string> values; std::string eventid;
            for (auto const& fieldMap: linelistFieldsMap) {
                // Retrieve the value to report for this demographic attribute.
                value = Source->GetValueAtUnmapped(fieldMap.first);
                value = value == 0 ? "" : value;
                values.push_back(value);
                trimString(values.back());
                // Add attribute to data set demographics.
                _demographics_by_dataset.back().get(fieldMap.second)->add(values.back(), static_cast<unsigned int>(nCount));
                // Special behavior for event id linelist column.
                if (fieldMap.second.get<0>() == EVENT_ID) {
                    if (_events_filter->contains(values.back())) {
                        _handler.gDataHub.GetPrintDirection().PrintWarning(
                            printString(buffer,
                            "Warning: The event id '%s' appears to be defined more than once in case file data yet event id is expected to be unique.\n",
                            values.back().c_str()).c_str()
                        );
                    } else
                        _events_filter->insert(values.back());
                    is_new_event = _existing_event_ids.find(value) == _existing_event_ids.end();
                    values.insert(values.end() - 1, (is_new_event ? "New" : ""));
                    eventid = value;
                }
                // Add attribute to cluster data set demographics.
                for (size_t t=0; t < applicable.size(); ++t) {
                    const CCluster& cluster = _clusters->GetCluster(static_cast<tract_t>(t));
                    // Add to cluster demographics only if location and time overlap.
                    if (applicable.test(t))
                        _cluster_demographics_by_dataset[static_cast<int>(t)].back().get(fieldMap.second)->add(values.back(), static_cast<unsigned int>(nCount));
                    else if (fieldMap.second.get<0>() == LinelistType::CATEGORICAL_DATA)
                        // Always add the categorical attribute label though, just so we have a complete set with each cluster.
                        _cluster_demographics_by_dataset[static_cast<int>(t)].back().get(fieldMap.second)->add(values.back(), 0);
                }
            }
            // Write values to temporary cluster file - depending on geographical overlap -- this could be more than one cluster.
            boost::optional<int> first(_events_by_dataset.back().get<0>() ? boost::make_optional(applicable.find_first()) : boost::none);
            for (boost::dynamic_bitset<>::size_type b= applicable.find_first(); b != boost::dynamic_bitset<>::npos; b=applicable.find_next(b)) {
                // typcically nCount == 1 but if not event data, could be aggregated count - hmm, what if nCount is large? (count columm might be better?)
                appendLinelistData(static_cast<int>(b), values, first, nCount);
                if (is_new_event) {
                    _new_event_ids.emplace(eventid);
                    _cluster_new_events[static_cast<int>(b)].first += nCount;
                }
                _cluster_new_events[static_cast<int>(b)].second += nCount;
            }
            // Maintain the event id cache. Add if:
            // 1) new event id signalled in significant cluster of this analysis.
            // 1) event id signalled in prior analysis iteration (event id in current case file and current event cache).
            if ((is_new_event && applicable.count()) || is_new_event == boost::logic::tribool(false))
                events_stream << eventid << std::endl;
        }
        _handler.gPrint.SetSuppressWarnings(storeSetting);
        if (events_stream.is_open()) events_stream.close();
        // Create the output file and concatenate cluster data files to it.
        FileName linelist(_parameters.GetOutputFileName().c_str()); // TODO -- should this be made into a AbstractDataFileWriter class?
        linelist.setExtension(printString(buffer, ".linelist%s.csv", (_handler.GetNumDataSets() > 1 ? printString(buffer2, ".dataset%u", DataSet.getSetIndex()).c_str() : "")).c_str());
        std::ofstream lineliststream;
        lineliststream.open(linelist.getFullPath(buffer).c_str());
        createHeadersFile(lineliststream, linelistFieldsMap);
        for (auto cfiles : _cluster_location_files) {
            std::ifstream filestream(cfiles.second.c_str(), std::ios_base::binary);
            lineliststream << filestream.rdbuf();
        }
        lineliststream.close();
    } catch (prg_exception& x) {
        x.addTrace("processCaseFileLinelist()", "DataDemographicAccumulator");
        throw;
    }
    return true;
}

/* Finalize usage of this object - overwriting events signal file. */
void DataDemographicsProcessor::finalize() {
    // Write signalling events to cache file if this data set includes event id in line list data - nothing to write otherwise.
    if (_events_by_dataset.back().get<0>()) {
        std::string filepath = _parameters.getEventCacheFileName();
        if (filepath.empty()) {
            FileName signalled(_parameters.GetOutputFileName().c_str());
            signalled.setExtension(".event-cache.txt");
            signalled.getFullPath(filepath);
            _handler.gDataHub.GetPrintDirection().Printf(
                "An event cache file is being created for this analysis, since one was not specified in the parameter settings.\n"
                "If you wish to maintain this file in future analyses, you should update the event cache parameter setting to:\n%s\n",
                BasePrint::P_WARNING, filepath.c_str()
            );
        }
        boost::filesystem::path from = _temp_events_cache_filename, to = filepath;
        boost::filesystem::detail::copy_file(from, to, boost::filesystem::detail::overwrite_if_exists);
        remove(_temp_events_cache_filename.c_str());
    }
}

void DataDemographicsProcessor::process() {
    for (const auto dataset : _handler.getDataSets()) {
        processCaseFileLinelist(*dataset);
    }
}

void DataDemographicsProcessor::print() {
    for (auto attrpair : _demographics_by_dataset.back().getAttributes())
        attrpair.second->print();
    for (auto cluster_dem : _cluster_demographics_by_dataset) {
        for (auto attrpair : _cluster_demographics_by_dataset[cluster_dem.first].back().getAttributes())
            attrpair.second->print();
    }
}

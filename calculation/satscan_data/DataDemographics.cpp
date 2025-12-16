//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "DataDemographics.h"
#include "SimulationVariables.h"
#include "MostLikelyClustersContainer.h"
#include "DataSetHandler.h"
#include "FileName.h"
#include "SaTScanData.h"
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
        auto mapKey = std::make_pair(itr->get<1>(), itr->get<2>());
        switch (itr->get<1>()) {
            case INDIVIDUAL_ID: _attributes_set[mapKey] = boost::shared_ptr<DemographicAttribute>(new GeneralDemographicAttribute(itr->get<2>(), INDIVIDUAL_ID)); break;
            case DESCRIPTIVE_COORD_X: _attributes_set[mapKey] = boost::shared_ptr<DemographicAttribute>(new GeneralDemographicAttribute(itr->get<2>(), DESCRIPTIVE_COORD_X)); break;
            case DESCRIPTIVE_COORD_Y: _attributes_set[mapKey] = boost::shared_ptr<DemographicAttribute>(new GeneralDemographicAttribute(itr->get<2>(), DESCRIPTIVE_COORD_Y)); break;
            //case GENERAL_DATA: _attributes_set[mapKey] = boost::shared_ptr<DemographicAttribute>(new GeneralDemographicAttribute(itr->get<2>())); break;
            case GENERAL_DATA: _attributes_set[mapKey] = boost::shared_ptr<DemographicAttribute>(new CategoricalDemographicAttribute(itr->get<2>())); break;
            case CATEGORICAL_DATA: _attributes_set[mapKey] = boost::shared_ptr<DemographicAttribute>(new CategoricalDemographicAttribute(itr->get<2>())); break;
            case CONTINUOUS_DATA: _attributes_set[mapKey] = boost::shared_ptr<DemographicAttribute>(new ContinuousDemographicAttribute(itr->get<2>())); break;
            default: throw prg_error("Unsupported line list type '%d'.", "DemographicAttributeSet()", itr->get<1>());
        }
    }
}

/* Attempts to retrieve DemographicAttribute for LinelistTuple_t. */
boost::shared_ptr<DemographicAttribute> DemographicAttributeSet::get(LinelistTuple_t llt) {
    auto itr = _attributes_set.find(std::make_pair(llt.get<1>(), llt.get<2>()));
    if (itr == _attributes_set.end())
        throw prg_error("Line-list tuple not defined (%d, %s).", "DemographicAttributeSet()", llt.get<1>(), llt.get<2>().c_str());
    return itr->second;
}

/* Returns whether demographic set includes individuals. */
bool DemographicAttributeSet::hasIndividual() const {
    for (const auto& attr : _attributes_set)
        if (attr.second->gettype() == INDIVIDUAL_ID) return true;
    return false;
}

/* Returns whether demographic set includes individuals and both descriptive latitiude and longitude. */
bool DemographicAttributeSet::hasIndividualGeographically() const {
    return hasIndividual() && hasDescriptiveCoordinates();
}

/* Returns whether demographic set includes both descriptive latitiude and longitude. */
bool DemographicAttributeSet::hasDescriptiveCoordinates() const {
    bool x = false, y = false;
    for (const auto& attr : _attributes_set) {
        x |= attr.second->gettype() == DESCRIPTIVE_COORD_X;
        y |= attr.second->gettype() == DESCRIPTIVE_COORD_Y;
        if (x && y) return true;
    }
    return x && y;
}

///////////////////////////////////// DataDemographicsProcessor //////////////////////////////////////////

DataDemographicsProcessor::DataDemographicsProcessor(const DataSetHandler& handler) :
    _handler(handler), _parameters(handler.gDataHub.GetParameters()) {
    _new_events_timestamp << boost::posix_time::second_clock::local_time();
    // Read individual ids from file cache - these signalled in significant clusters of previous analysis(es).
    if (boost::filesystem::exists(_parameters.getLinelistIndividualsCacheFileName().c_str())) {
		std::string buffer;
        std::ifstream event_stream;
        event_stream.open(_parameters.getLinelistIndividualsCacheFileName().c_str());
        if (!event_stream)
            throw resolvable_error(
                "Error: Could not open file '%s' to read the prior signalling individuals.\n", _parameters.getLinelistIndividualsCacheFileName().c_str()
            );
        while (!event_stream.eof()) {
            std::getline(event_stream, buffer);
            if (!trimString(buffer).size()) continue;
            boost::tokenizer<boost::escaped_list_separator<char>> values(buffer, boost::escaped_list_separator<char>(std::string(""), ",", "\""));
            for (auto itr = values.begin(); itr != values.end(); ++itr) {
                buffer = *itr;
                if (trimString(buffer).size()) _existing_individuals.emplace(buffer);
                // The first token is the individual id. Earlier versions of the file didn't include a timestamp column, so just ignore any extra tokens.
                break;
            }
        }
        event_stream.close();
    }
}

/* Destructor */
DataDemographicsProcessor::~DataDemographicsProcessor() {
    removeTempClusterFiles();
}

/** Added clusters to accumulation, which the demographics will be reported in the context of. */
void DataDemographicsProcessor::addClusters(MostLikelyClustersContainer& clusters, const SimulationVariables& sim_vars, unsigned int iteration) {
    // Iterate over the most likely clusters, creating structures that will facilitate compiling data.
    auto recordClusterLocations = [this, iteration](MostLikelyClustersContainer::Cluster_t& cluster, bool isReportedCsv) {
        // Define which locations are in each cluster, using bitset for quick search while iterating over case line list data rows.
        if (cluster->GetClusterType() != PURELYTEMPORALCLUSTER) {
            auto clusterIdx = _reporting_clusters.size();
            _reporting_clusters.emplace_back(ReportCluster(
                cluster, boost::dynamic_bitset<>(_handler.gDataHub.GetNumIdentifiers()), isReportedCsv, iteration, clusterIdx
            ));
            std::vector<tract_t> tractIndexes;
            boost::dynamic_bitset<>& locations = _reporting_clusters.back()._locations;
            for (auto tractIdx : cluster->getIdentifierIndexes(_handler.gDataHub, tractIndexes, true))
                locations.set(tractIdx);
        }
    };
    for (int i = 0; i < clusters.GetNumClustersRetained(); ++i) {
        auto& cluster = clusters.GetClusterRef(i);
        if (isReportedInCsv(_handler.gDataHub, *cluster, i, sim_vars)) {
            recordClusterLocations(cluster, true);
        } else if (meetsMainResultsCutoff(*cluster, i, sim_vars))
            recordClusterLocations(cluster, false);
    }
}

/* Appends data record to temporary cluster file. */
void DataDemographicsProcessor::appendLinelistData(int clusterIdx, std::vector<std::string>& data, boost::optional<int> first, unsigned int times) {
    auto& cluster = _reporting_clusters[clusterIdx];
    std::ofstream temp_file(
        cluster._temporary_filename.c_str(),
        std::ios_base::app | std::ios_base::binary
    );
    std::string buffer;
    std::stringstream line;
    line << (cluster._iteration > 1 ? cluster._iteration : (clusterIdx + 1)) << ",";
    if (first) {
        if (cluster._iteration > 1)
            line << "Iterative";
        else
            line << (first.get() == clusterIdx ? "Primary" : "Secondary");
        line << ",";
    }
    line << typelist_to_csv_string<std::string>(data, buffer) << std::endl;
    // typcically times == 1 but if not individual data, could be aggregated count
    for (int i=0; i < times; ++i) 
        temp_file << line.str();
}

/** Returns whether a cluster is reported in the cluster line-list output file. */
bool DataDemographicsProcessor::isReportedInCsv(const CSaTScanData& Data, const CCluster& cluster, unsigned int iReportedCluster, const SimulationVariables& simVars) {
    const auto& parameters = Data.GetParameters();
    if (cluster.reportableRecurrenceInterval(parameters, simVars) && !Data.isDrilldown())
        return parameters.getCutoffLineListCSV() <= std::round(cluster.GetRecurrenceInterval(Data, iReportedCluster + 1, simVars).second) // round RI to whole days
        && meetsMainResultsCutoff(cluster, iReportedCluster, simVars);
    if (cluster.reportablePValue(parameters, simVars))
        return macro_less_than_or_equal(cluster.getReportingPValue(parameters, simVars, parameters.GetIsIterativeScanning() || (iReportedCluster + 1) == 1), parameters.getCutoffLineListCSV(), DBL_CMP_TOLERANCE) 
        && meetsMainResultsCutoff(cluster, iReportedCluster, simVars);
    // Otherwise match reporting criteria of main results file.
    return meetsMainResultsCutoff(cluster, iReportedCluster, simVars);
}

/** Returns whether cluster meets main results file cutoff. */
bool DataDemographicsProcessor::meetsMainResultsCutoff(const CCluster& cluster, unsigned int iReportedCluster, const SimulationVariables& simVars) {
    return iReportedCluster == 0 || (cluster.m_nRatio >= MIN_CLUSTER_LLR_REPORT && (simVars.get_sim_count() == 0 || cluster.GetRank() <= simVars.get_sim_count()));
}

/* Returns bitset which indicates the clusters indexes which contain location/date in their cluster window. */
boost::dynamic_bitset<>& DataDemographicsProcessor::getApplicableClusters(tract_t tid, Julian nDate, boost::dynamic_bitset<>& applicable_clusters, bool reportedOnly) const {
    if (applicable_clusters.empty())
        applicable_clusters.resize(_reporting_clusters.size());
    else
        applicable_clusters.reset();
    int startIdx = _handler.gDataHub.GetTimeIntervalOfDate(nDate), endIdx = _handler.gDataHub.GetTimeIntervalOfEndDate(nDate);
    // Determine which clusters this record applys to.
    for (const auto& clusterR : _reporting_clusters) {
        if (reportedOnly && !clusterR._reportedInCsv) continue;
        const auto& cluster = clusterR._cluster;
        if (clusterR._cluster->GetClusterType() != PURELYTEMPORALCLUSTER && !clusterR._locations.test(tid))
            continue;
        if (startIdx >= cluster->m_nFirstInterval && endIdx <= cluster->m_nLastInterval)
            applicable_clusters.set(clusterR._index);
    }
    return applicable_clusters;
}

/* Re-reads cases file to accumulate line-list data inconjuction with detected clusters. */
bool DataDemographicsProcessor::processCaseFileLinelist(const RealDataSet& DataSet) {
    try {
        removeTempClusterFiles(); // remove temporary cluster files from prior data set
        std::string buffer;
        /* Open case file data source - input source should now have ncessary line-list mappings since they were assigned during initial case file read. */
        _handler.gPrint.SetImpliedInputFileType(BasePrint::CASEFILE);
        std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(
            getFilenameFormatTime(_handler.gParameters.GetCaseFileName(DataSet.getSetIndex()), _handler.gParameters.getTimestamp(), true),
            _handler.gParameters.getInputSource(CASEFILE, DataSet.getSetIndex()), _handler.gPrint)
        );
        // Create new demographics attributes set for this data set
        _demographics_by_dataset.push_back(DemographicAttributeSet(Source->getLinelistFieldsMap()));
        // Create temporary individuals cache to record all individuals discovered in this analysis.
        std::ofstream individuals_stream;
        if (_demographics_by_dataset.back().hasIndividual() && _temp_individuals_cache_filename.empty()) {
            _temp_individuals_cache_filename = GetUserTemporaryFilename(buffer);
            individuals_stream.open(_temp_individuals_cache_filename, std::ios_base::app);
            _individuals_filter = getNewBloomFilter(_handler._approximate_case_records);
        }
        // Create a new demographics attributes set for each cluster being reported.
        for (auto& cluster : _reporting_clusters)
			cluster._demographics_by_dataset.push_back(DemographicAttributeSet(Source->getLinelistFieldsMap()));
        // Iterate over case file records of this data set.
        boost::dynamic_bitset<> applicable_clusters(_reporting_clusters.size());
        bool storeSetting = _handler.gPrint.isSuppressingWarnings();
        _handler.gPrint.SetSuppressWarnings(true);
        DataSource::OrderedLineListField_t linelistFieldsMap;
        Source->getOrderedLinelistFieldsMap(linelistFieldsMap);
		tract_t tid; count_t nCount; Julian nDate;
        while (!_handler.gPrint.GetMaximumReadErrorsPrinted() && Source->ReadRecord()) {
			DataSetHandler::RecordStatusType readStatus = _handler.RetrieveIdentifierIndex(*Source, tid);
			if (readStatus != DataSetHandler::Accepted) continue; // Should only be either Accepted or Ignored.
			readStatus = _handler.RetrieveCaseCounts(*Source, nCount);
			if (readStatus != DataSetHandler::Accepted) continue; // Should only be either Accepted or Ignored.
			readStatus = _handler.RetrieveCountDate(*Source, nDate);
			if (readStatus != DataSetHandler::Accepted) continue; // Should only be either Accepted or Ignored.
            int startIdx = _handler.gDataHub.GetTimeIntervalOfDate(nDate);
            int endIdx = _handler.gDataHub.GetTimeIntervalOfEndDate(nDate);
            // Determine which clusters this record applys to.
            getApplicableClusters(tid, nDate, applicable_clusters, true);
            boost::logic::tribool is_new_event(boost::logic::indeterminate); // unknown, false, true
            const char * value = 0; std::vector<std::string> values; std::string individual;
            for (auto const& fieldMap: linelistFieldsMap) {
                // Retrieve the value to report for this demographic attribute.
                value = Source->GetValueAtUnmapped(fieldMap.get<0>());
                values.push_back((value == 0 ? "" : value));
                trimString(values.back());
                // Add attribute to data set demographics.
                _demographics_by_dataset.back().get(fieldMap)->add(values.back(), static_cast<unsigned int>(nCount));
                // Special behavior for event id linelist column.
                if (fieldMap.get<1>() == INDIVIDUAL_ID) {
                    if (!values.back().size())
                        throw resolvable_error(
                            "Error: Case file line list data references an individual with a blank value (record %ld, data set %u).\n",
                            Source->GetCurrentRecordIndex(), DataSet.getSetIndex()
                        );
                    if (_individuals_filter->contains(values.back())) {
                        _handler.gDataHub.GetPrintDirection().PrintWarning(printString(buffer,
                            "Warning: The individual id '%s' appears to be defined more than once in case file data yet individual id is expected to be unique.\n",
                            values.back().c_str()).c_str()
                        );
                    } else
                        _individuals_filter->insert(values.back());
                    is_new_event = _existing_individuals.find(value) == _existing_individuals.end();
                    values.insert(values.end() - 1, (is_new_event ? "New" : ""));
                    individual = value;
                }
                // Add attribute to cluster data set demographics.
                for (size_t t=0; t < applicable_clusters.size(); ++t) {
                    // Add to cluster demographics only if location and time overlap.
                    if (applicable_clusters.test(t))
                        _reporting_clusters[t]._demographics_by_dataset.back().get(fieldMap)->add(values.back(), static_cast<unsigned int>(nCount));
                    else if (fieldMap.get<1>() == LinelistType::CATEGORICAL_DATA)
                        // Always add the categorical attribute label though, just so we have a complete set with each cluster.
                        _reporting_clusters[t]._demographics_by_dataset.back().get(fieldMap)->add(values.back(), 0);
                }
            }
            // Write values to temporary cluster file - depending on geographical overlap -- this could be more than one cluster.
            boost::optional<int> first(_demographics_by_dataset.back().hasIndividual() ? boost::make_optional(applicable_clusters.find_first()) : boost::none);
            for (boost::dynamic_bitset<>::size_type b= applicable_clusters.find_first(); b != boost::dynamic_bitset<>::npos; b=applicable_clusters.find_next(b)) {
                appendLinelistData(static_cast<int>(b), values, first, nCount);
                if (is_new_event) {
                    _new_individuals.emplace(individual);
                    _reporting_clusters[b]._event_totals.first += nCount;
                }
                _reporting_clusters[b]._event_totals.second += nCount;
            }
            // Maintain the individuals cache. Add if it's new individual that signalled in significant cluster of this analysis.
            // Individuals which signalled in previous analyses are already included in primary cache file.
            if ((is_new_event && applicable_clusters.count())) 
                individuals_stream << individual << "," << _new_events_timestamp.str() << std::endl;
        }
        if (individuals_stream.is_open()) individuals_stream.close();
        // Create the cluster line list file and concatenate each cluster line list file.
        writeClusterLineListFile(linelistFieldsMap, DataSet.getSetIndex());
    } catch (prg_exception& x) {
        x.addTrace("processCaseFileLinelist()", "DataDemographicsProcessor");
        throw;
    }
    return true;
}

/* Returns whether any data set includes individuals in line list data. */
bool DataDemographicsProcessor::hasIndividualAttribute() const {
    for (const auto& dbd : _demographics_by_dataset) {
        if (dbd.hasIndividual()) return true;
    } return false;
}

/* Returns whether any data set can place line list individuals geographically. */
bool DataDemographicsProcessor::hasIndividualGeographically() const {
    for (const auto& dbd : _demographics_by_dataset) {
        if (dbd.hasIndividualGeographically()) return true;
    } return false;
}

/** Returns whether a record (tid and nDate) is within a cluster. */
bool DataDemographicsProcessor::inCluster(tract_t tid, Julian nDate) const {
    boost::dynamic_bitset<> applicable_clusters(_reporting_clusters.size());
    return getApplicableClusters(tid, nDate, applicable_clusters, true).any();
}

/* Finalize usage of this object - overwriting events signal file. */
void DataDemographicsProcessor::finalize() {
    // Write signalling individuals from this analysis to the cache file - excluding drilldowns, only the primary analysis writes to cache.
    if (_temp_individuals_cache_filename.size()) {
        if (_parameters.getLinelistIndividualsCacheFileName().size() && !_handler.gDataHub.isDrilldown()) {
            _handler.gDataHub.GetPrintDirection().Printf(
                "A cache file is being maintained for this analysis to track line list individuals seen in clusters which meet line list cutoff.\n"
                "This cache will be used in subsequent runs of this analysis to determine whether an individual is considered 'new' or 'ongoing'.\n",
                BasePrint::P_WARNING, _parameters.getLinelistIndividualsCacheFileName().c_str()
            );
            // Append new signalling individuals to the primary cache file.
            std::ifstream temp_new_cache(_temp_individuals_cache_filename, std::ios::binary);
            if (!temp_new_cache)
                throw prg_error(
                    "Error: Failed to reopen temporary individuals cache file for reading prior to writing line-list cache file:\n%s\n",
                    _temp_individuals_cache_filename.c_str()
                );
            std::ofstream final_cache(
                _parameters.getLinelistIndividualsCacheFileName().c_str(), std::ios_base::app | std::ios_base::binary
            );
            if (!final_cache)
                throw resolvable_error(
                    "Error: Could not open line-list individuals cache file for writing:\n%s\n",
                    _parameters.getLinelistIndividualsCacheFileName().c_str()
                );
            final_cache << temp_new_cache.rdbuf();
            temp_new_cache.close();
            final_cache.close();
        }
        remove(_temp_individuals_cache_filename.c_str());
    }
}

/* Process line list data from each data set. */
void DataDemographicsProcessor::process() {
    bool storeSetting = _handler.gPrint.isSuppressingWarnings(); // prevent re-printing case file warnings
    _handler.gPrint.SetSuppressWarnings(true);
    for (const auto& dataset : _handler.getDataSets()) {
        processCaseFileLinelist(*dataset);
    }
    _handler.gPrint.SetSuppressWarnings(storeSetting);
}

void DataDemographicsProcessor::print() {
    for (const auto& attrpair : _demographics_by_dataset.back().getAttributes())
        attrpair.second->print();
    for (auto& cluster: _reporting_clusters) {
        for (const auto& attrpair : cluster._demographics_by_dataset.back().getAttributes())
            attrpair.second->print();
	}
}

/* Attempts to remove any temporary cluster files. */
void DataDemographicsProcessor::removeTempClusterFiles() {
    for (const auto& cluster : _reporting_clusters) {
        try {
            remove(cluster._temporary_filename.c_str());
        } catch (...) {}
    }
}

/* Creates the final clusters line list file - adds header row then concatenates temporary cluster files into one. */
void DataDemographicsProcessor::writeClusterLineListFile(const DataSource::OrderedLineListField_t& llmap, unsigned int idxDataSet) {
    std::vector<std::string> v = { "Cluster" };
    for (auto const& itr : llmap) {
        if (itr.get<1>() == INDIVIDUAL_ID) {
            v.push_back("Hierarchy");
            v.push_back("New Individual");
        }
        v.push_back(itr.get<2>());
    }
    std::string buffer, buffer2;
    FileName linelist(_parameters.GetOutputFileName().c_str());
    linelist.setExtension(printString(
        buffer, ".linelist%s.csv", (_handler.GetNumDataSets() > 1 ? printString(buffer2, ".dataset%u", idxDataSet).c_str() : "")
    ).c_str());
    std::ofstream lineliststream;
    lineliststream.open(linelist.getFullPath(buffer).c_str(), std::ios_base::trunc | std::ios_base::binary);
    if (!lineliststream)
        throw resolvable_error(
            "Error: The line list csv file could not be opened for writing:\n'%s'\n", buffer.c_str()
        );
    // Write header row -- skip if secondary scans (iterative scan)
    lineliststream << typelist_to_csv_string<std::string>(v, buffer) << std::endl; // write header row
    for (const auto& cluster : _reporting_clusters) {
        std::ifstream filestream(cluster._temporary_filename.c_str(), std::ios_base::binary);
        lineliststream << filestream.rdbuf();
    }
    lineliststream.close();
}

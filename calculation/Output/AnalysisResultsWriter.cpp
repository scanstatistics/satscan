//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "AnalysisResultsWriter.h"
#include "SSException.h"
#include "SaTScanData.h"
#include "ParametersPrint.h"
#include "Toolkit.h"
#include "AsciiPrintFormat.h"
#include "cluster.h"

AnalysisResultsWriter::AnalysisResultsWriter(const CSaTScanData& dataHub) : _dataHub(dataHub),  _parameters(dataHub.GetParameters()) {
    _textOutputName = _parameters.GetOutputFileName();
    FileName(_parameters.GetOutputFileName().c_str()).setExtension(".html").getFullPath(_htmlOutputName);
    if ((_text_out = fopen(_textOutputName.c_str(), "w")) == NULL)
        throw resolvable_error("Error: Unable to create output file '%s'.\n", _textOutputName.c_str());
    _html_out.open(_htmlOutputName);
    if (!_html_out) throw resolvable_error("Error: Unable to create output file '%s'.\n", _htmlOutputName.c_str());
    _html_out << "<!DOCTYPE html>" << std::endl;
    _html_out << "<?xml version='1.0' encoding='UTF-8'?> " << std::endl;
    _html_out << "<html xmlns='http://www.w3.org/1999/xhtml' xml:lang='en'>" << std::endl;
    _html_out << "<head>" << std::endl;
    _html_out << "<link rel='stylesheet' href='" << AppToolkit::getToolkit().GetWebSite() << "javascript/bootstrap/4.1.1/bootstrap.4.1.1.css'>" << std::endl;
    _html_out << "<link rel='stylesheet' href='" << AppToolkit::getToolkit().GetWebSite() << "javascript/datatables.1.10.16/css/jquery.dataTables.min.css'>" << std::endl;
    _html_out << "<link rel='stylesheet' href='" << AppToolkit::getToolkit().GetWebSite() << "html-results/satscan-results.1.0.css'>" << std::endl;
    _html_out << "<link rel='stylesheet' href='http://maxcdn.bootstrapcdn.com/font-awesome/4.7.0/css/font-awesome.min.css'>" << std::endl;
    _html_out << "</head>" << std::endl;
    _html_out << "<script src='" << AppToolkit::getToolkit().GetWebSite() << "javascript/jquery/jquery-3.7.0.min.js' type='text/javascript'></script>" << std::endl;
    _html_out << "<script src='" << AppToolkit::getToolkit().GetWebSite() << "javascript/datatables.1.10.16/js/jquery.dataTables.min.js' type='text/javascript'></script>" << std::endl;
    _html_out << "<script src='" << AppToolkit::getToolkit().GetWebSite() << "javascript/bootstrap/4.1.1/popper.4.1.1.js' type='text/javascript'></script>" << std::endl;
    _html_out << "<script src='" << AppToolkit::getToolkit().GetWebSite() << "javascript/bootstrap/4.1.1/bootstrap.4.1.1.js' type='text/javascript'></script>" << std::endl;
    _html_out << "<script src='" << AppToolkit::getToolkit().GetWebSite() << "html-results/satscan-results.1.0.js' type='text/javascript'></script>" << std::endl;
    _html_out << "<body>" << std::endl;
}

std::string& AnalysisResultsWriter::getTotalRunningTime(time_t start, time_t end, std::string& buffer) const {
    double nTotalTime = difftime(end, start);
    double nHours = floor(nTotalTime / (60 * 60));
    double nMinutes = floor((nTotalTime - nHours * 60 * 60) / 60);
    double nSeconds = nTotalTime - (nHours * 60 * 60) - (nMinutes * 60);
    const char* szHours = (0 < nHours && nHours < 1.5 ? "hour" : "hours");
    const char* szMinutes = (0 < nMinutes && nMinutes < 1.5 ? "minute" : "minutes");
    const char* szSeconds = (0.5 <= nSeconds && nSeconds < 1.5 ? "second" : "seconds");
    if (nHours > 0) printString(buffer, "%.0f %s %.0f %s %.0f %s", nHours, szHours, nMinutes, szMinutes, nSeconds, szSeconds);
    else if (nMinutes > 0) printString(buffer, "%.0f %s %.0f %s", nMinutes, szMinutes, nSeconds, szSeconds);
    else printString(buffer, "%.0f %s", nSeconds, szSeconds);
    return buffer;
}

AnalysisResultsWriter::~AnalysisResultsWriter() {
    if (_text_out) fclose(_text_out);
    if (_html_out.is_open()) _html_out.close();
}

/** Writes the cluster information to an HTML table row. 
    There is a precise coupling with headers defined in writeHtmlTableStart() and the cluster attributes reported here. */
void AnalysisResultsWriter::writeClusterToHtmlTable(const CCluster& cluster, const ClusterSupplementInfo& supplementInfo, const SimulationVariables& simVars) {
    std::string rowId, buffer, work, work2;
    printString(rowId, "tr-ID_%u", (supplementInfo.getClusterReportIndex(cluster)));
    std::vector<std::pair<std::string, std::string>> columnValues(_html_columns.size(), { "-","" }); // display value, data-order value
    size_t valueIdx = 0;
    // Create map to cluster cached values - easier and format will match that of other output files.
    std::map<std::string, CCluster::FieldCache> clusterAttributes;
    std::set<unsigned int> datasets;
    for (const auto& entry : cluster.getReportLinesCache()) {
        // Store dataset attributes for later use - we want to write these at the cluster level when only 1 dataset,
        // but at the dataset level when multiple datasets (with subrows for each dataset).
        if (entry._set_idx != std::numeric_limits<unsigned int>::max()) datasets.emplace(entry._set_idx);
        if (entry._set_idx > 0 && entry._set_idx != std::numeric_limits<unsigned int>::max()) continue;
        if (clusterAttributes.find(entry._label) != clusterAttributes.end())
            throw prg_error("Duplicate cluster attribute label found in cache: '%s'", "AnalysisResultsWriter", entry._label.c_str());
        clusterAttributes.insert(std::make_pair(entry._label, entry));
    }
    // Write dataset level attributes for each dataset represented in cluster cache (if more than 1 dataset).
    // With multiple data sets, we'll have subrows detailing the data set specific values.
    std::vector<std::map<std::string, CCluster::FieldCache>> datasetAttributes;
    if (_parameters.getNumFileSets() > 1) {
        for (auto setIdx : datasets) {
            datasetAttributes.emplace_back();
            for (const auto& entry : cluster.getReportLinesCache()) {
                if (entry._set_idx != setIdx) continue; // only process entries for this dataset
                if (datasetAttributes.back().find(entry._label) != datasetAttributes.back().end())
                    throw prg_error("Duplicate cluster attribute label found in cache: '%s'", "AnalysisResultsWriter", entry._label.c_str());
                datasetAttributes.back().insert(std::make_pair(entry._label, entry));
            }
        }
    }
    printString(columnValues[valueIdx].first, "%u", supplementInfo.getClusterReportIndex(cluster));
    if (_parameters.getNumFileSets() > 1)
        printString(columnValues[++valueIdx].first, "<a href='#' class='cut-node-w-children'>%s</a>", cluster.GetClusterLocation(buffer, _dataHub).c_str());
    else 
        cluster.GetClusterLocation(columnValues[++valueIdx].first, _dataHub);
    // Set columnn which reports the number of locations in the cluster - potentially with popover information detailing locations and overlap.
    _cluster_locations.emplace_back(boost::dynamic_bitset<>());
    if (cluster.GetClusterType() == PURELYTEMPORALCLUSTER)
        printString(columnValues[++valueIdx].first, "%u", _dataHub.getLocationsManager().locations().size());
    else {
        std::vector<tract_t> clusterLocations;
        CentroidNeighborCalculator::getLocationsAboutCluster(_dataHub, cluster, &_cluster_locations.back(), &clusterLocations);
        std::stringstream printStringBuffer, popoverText;
        const auto& locationData = _dataHub.getLocationsManager().locations();
        for (auto index : clusterLocations) {
            if (popoverText.rdbuf()->in_avail()) popoverText << ", ";
            popoverText << locationData[index].get()->name();
            // check for overlap with previous cluster locations and if so, add to buffer for display in popover
            std::stringstream overlaps;
            for (size_t t=0; t < _cluster_locations.size() - 1; ++t) {
                if (_cluster_locations[t].test(index))
                    overlaps << (overlaps.rdbuf()->in_avail() ? "," : "(") << "#" << (t + 1);
            }
            if (overlaps.rdbuf()->in_avail()) overlaps << ")";
            popoverText << overlaps.str();
        }
        printStringBuffer << "<a class='location-popover' href='#' cluster-id='" << supplementInfo.getClusterReportIndex(cluster)
			<< "' cluster-locations='" << htmlencode(popoverText.str(), buffer) << "'>" << _cluster_locations.back().count() << "</a>";
		columnValues[++valueIdx].first = printStringBuffer.str();
        columnValues[valueIdx].second = std::to_string(_cluster_locations.back().count());
    }
    if (_parameters.getClusterMonikerPrefix().size())
        printString(
            columnValues[++valueIdx].first, "%sC%u", _parameters.getClusterMonikerPrefix().c_str(), supplementInfo.getClusterReportIndex(cluster)
        );
    if (_parameters.getReportGiniOptimizedClusters())
        columnValues[++valueIdx].first = (supplementInfo.getOverlappingClusters(cluster, buffer).size() == 0 ? "No Overlap": buffer);
    // Set radius, coordinates, and related columns - when appropriate.
    if (!(_parameters.UseLocationNeighborsFile() || cluster.GetClusterType() == PURELYTEMPORALCLUSTER ||
        (_parameters.getUseLocationsNetworkFile() && !_dataHub.networkCanReportLocationCoordinates()))) {
        std::vector<double> vCoordinates;
        _dataHub.GetGInfo()->retrieveCoordinates(cluster.GetCentroidIndex(), vCoordinates);
        if (_parameters.GetCoordinatesType() == CARTESIAN) {
            for (size_t i = 0; i < vCoordinates.size() - 1; ++i)
                buffer += printString(work, "%g,", vCoordinates[i]);
            buffer += printString(work, "%g", vCoordinates.back());
            columnValues[++valueIdx].first = buffer;
            if (_parameters.GetSpatialWindowType() == ELLIPTIC) {
                columnValues[++valueIdx].first = clusterAttributes["Semiminor axis"]._formatted_value;
                columnValues[++valueIdx].first = clusterAttributes["Semimajor axis"]._formatted_value;
                columnValues[++valueIdx].first = clusterAttributes["Angle (degrees)"]._formatted_value;
                columnValues[++valueIdx].first = clusterAttributes["Shape"]._formatted_value;
            } else {
                if (!_parameters.getUseLocationsNetworkFile())
                    getValueAsString(cluster.GetCartesianRadius(), columnValues[++valueIdx].first);
                if (_parameters.getUseLocationsNetworkFile()) {
                    if (cluster.GetCartesianRadius() != -1.0)
                        columnValues[++valueIdx].first = getValueAsString(cluster.GetCartesianRadius(), work);
                    else ++valueIdx;
                }
            }
        } else {
            std::pair<double, double> prLatitudeLongitude;
            prLatitudeLongitude = ConvertToLatLong(vCoordinates);
            printString(columnValues[++valueIdx].first, "%.6f %c, %.6f %c",
                fabs(prLatitudeLongitude.first), prLatitudeLongitude.first >= 0 ? 'N' : 'S',
                fabs(prLatitudeLongitude.second), prLatitudeLongitude.second >= 0 ? 'E' : 'W'
            );
            columnValues[valueIdx].second = printString(buffer, "[%g,%g]", prLatitudeLongitude.first, prLatitudeLongitude.second);
            if (!_parameters.getUseLocationsNetworkFile()) {
                printString(columnValues[++valueIdx].first, "%s km", getValueAsString(cluster.GetLatLongRadius(), work).c_str());
                getValueAsString(cluster.GetLatLongRadius(), columnValues[valueIdx].second);
            }
            if (_parameters.getUseLocationsNetworkFile()) {
                if (cluster.GetCartesianRadius() != -1.0) {
                    printString(columnValues[++valueIdx].first, "%s km", getValueAsString(cluster.GetLatLongRadius(), work).c_str());
                    getValueAsString(cluster.GetLatLongRadius(), columnValues[valueIdx].second);
                } else ++valueIdx;
            }
        }
        columnValues[++valueIdx].first = clusterAttributes["Span"]._formatted_value;
        columnValues[valueIdx].second = clusterAttributes["Span-raw"]._formatted_value;
    }
    if (_parameters.getReportGiniOptimizedClusters())
        columnValues[++valueIdx].first = clusterAttributes["Gini Cluster"]._formatted_value;
    if (_parameters.GetProbabilityModelType() != HOMOGENEOUSPOISSON && !_parameters.GetIsPurelySpatialAnalysis() && _parameters.GetAnalysisType() != SPATIALVARTEMPTREND) {
        columnValues[++valueIdx].first = clusterAttributes["Time frame"]._formatted_value;;
        printString(columnValues[valueIdx].second, "[%u,%u]", 
            _dataHub.GetTimeIntervalStartTimes()[cluster.m_nFirstInterval], _dataHub.GetTimeIntervalStartTimes()[cluster.m_nLastInterval] - 1
        );
    }
    // This lambda is used to write the cluster level column data in the appropriate order depending on number of data sets.
    auto writeClusterLevelColumnData = [&](const CCluster& cluster) {
        if (_parameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
            if (_parameters.getTimeTrendType() == LINEAR) {
                columnValues[++valueIdx].first = clusterAttributes["Inside time trend"]._formatted_value;
                columnValues[++valueIdx].first = clusterAttributes["Outside time trend"]._formatted_value;
            } else if (_parameters.getTimeTrendType() == QUADRATIC) {
                columnValues[++valueIdx].first = clusterAttributes["Inside Risk Function"]._formatted_value;
                columnValues[++valueIdx].first = clusterAttributes["Outside Risk Function"]._formatted_value;
            }
        }
        if (_parameters.IsTestStatistic(false)) {
            columnValues[++valueIdx].first = clusterAttributes["Test statistic"]._formatted_value;
        } else {
            columnValues[++valueIdx].first = clusterAttributes["Log likelihood ratio"]._formatted_value;
            if (_parameters.GetSpatialWindowType() == ELLIPTIC)
                columnValues[++valueIdx].first = clusterAttributes["Test statistic"]._formatted_value;
        }
        if (_parameters.GetNumReplicationsRequested() >= MIN_SIMULATION_RPT_PVALUE) {
            if (_parameters.getReportClusterRank())
                columnValues[++valueIdx].first = clusterAttributes["Monte Carlo rank"]._formatted_value;
            if (cluster.reportablePValue(_parameters, simVars)) {
                columnValues[++valueIdx].first = clusterAttributes["P-value"]._formatted_value;
                columnValues[valueIdx].second = clusterAttributes["P-value-raw"]._formatted_value;
            }
            if (cluster.reportableRecurrenceInterval(_parameters, simVars)) {
                columnValues[++valueIdx].first = clusterAttributes["Recurrence interval"]._formatted_value;
                columnValues[valueIdx].second = clusterAttributes["Recurrence interval (days)"]._formatted_value;
            }
            if (_parameters.getIsReportingGumbelAsAddon()) {
                columnValues[++valueIdx].first = clusterAttributes["Gumbel P-value"]._formatted_value;
                columnValues[valueIdx].second = clusterAttributes["Gumbel P-value-raw"]._formatted_value;
            }
        }
    };
    // Now write data sets level fields, but dependent on number of data sets.
    if (_parameters.getNumFileSets() > 1)
        writeClusterLevelColumnData(cluster);
    auto setDataRowValues = [&]( // lambda to set data row values from attributes
        const std::vector<std::string>& attributeNames,
        const std::map<std::string, CCluster::FieldCache>& attributes,
        std::vector<std::pair<std::string, std::string>>& datarow
    ) {
        auto setValuesFromAttributes = [&](
            const std::map<std::string, CCluster::FieldCache>& attributes, std::vector<std::pair<std::string, std::string>>& datarow, size_t& valIdx
        ) {
            for (size_t t = 0; t < attributeNames.size(); ++t) {
                auto it = attributes.find(attributeNames[t]);
                if (it != attributes.end())
                    datarow[++valIdx] = std::make_pair(it->second._formatted_value, ""); // no data-order value for now
            }
        };
        if (_parameters.getNumFileSets() > 1) { // If multiple sets, need special routine to do per data set.
            size_t setAttrIdx = 0;
            if (_html_sub_rows.rdbuf()->in_avail()) _html_sub_rows << "," << std::endl;
            _html_sub_rows << "'" << rowId << "' : {" << std::endl;
            for (auto setIdx : datasets) {
                std::vector<std::pair<std::string, std::string>> subrowValues(_html_columns.size(), { "-","" }); // display value, data-order value
                printString(subrowValues[0].first, "%u_%u", supplementInfo.getClusterReportIndex(cluster), setIdx + 1);
                htmlencode(_dataHub.getDatasetLabel(setIdx, buffer, false), subrowValues[1].first);
                size_t temp = valueIdx; setValuesFromAttributes(datasetAttributes[setAttrIdx], subrowValues, temp);
                if (setAttrIdx) _html_sub_rows << "," << std::endl;
                _html_sub_rows << "'tr-set" << (setIdx + 1) << "':[";
                for (size_t t=0; t < subrowValues.size(); ++t)
                    _html_sub_rows << "'" << subrowValues[t].first << "'" << (t < subrowValues.size() - 1 ? "," : "");
                _html_sub_rows << "]";
                ++setAttrIdx;
            }
            _html_sub_rows << std::endl << "}";
        } else // Otherwise just from cluster attributes
            setValuesFromAttributes(clusterAttributes, columnValues, valueIdx);
    };
    // Now set the data row values depending on probability model type.
    if (_parameters.GetProbabilityModelType() == ORDINAL || _parameters.GetProbabilityModelType() == CATEGORICAL) {
        setDataRowValues({
            "Total cases", "Category", "Number of cases", "Expected cases", "Observed / expected", "Relative risk", "Percent cases in area"
            }, clusterAttributes, columnValues
        );
    } else if (_parameters.GetProbabilityModelType() == EXPONENTIAL) {
        setDataRowValues({ "Total individuals", "Number of cases", "Expected cases", "Observed / expected" }, clusterAttributes, columnValues);
    } else if (_parameters.GetProbabilityModelType() == NORMAL &&_parameters.getIsWeightedNormal()) {
        setDataRowValues({
            "Number of cases", "Total weights", "Mean inside", "Mean outside", "Variance", "Standard deviation",
            "Weighted mean inside", "Weighted mean outside", "Weighted variance", "Weighted std deviation"
        }, clusterAttributes, columnValues);
    } else if (_parameters.GetProbabilityModelType() == NORMAL && !_parameters.getIsWeightedNormal()) {
        setDataRowValues({
            "Number of cases", "Mean inside", "Mean outside", "Variance", "Standard deviation"
        }, clusterAttributes, columnValues);
    } else if (_parameters.GetProbabilityModelType() == RANK) {
        setDataRowValues({ "Number of cases", "Average Rank Inside", "Average Rank Outside" }, clusterAttributes, columnValues);
    } else if (_parameters.GetProbabilityModelType() == BATCHED) {
        setDataRowValues({
            "Number of batches", "Obs. positive batches", "Exp. positive batches",
            "Number of individuals", "Positive individuals inside", "Positive individuals outside"
        }, clusterAttributes, columnValues);
    } else {
        std::vector<std::string> attributeNames;
        if ((_parameters.GetProbabilityModelType() == POISSON && _parameters.UsePopulationFile() && cluster.GetClusterType() != PURELYTEMPORALCLUSTER) ||
            _parameters.GetProbabilityModelType() == BERNOULLI)
            attributeNames.emplace_back("Population");
        attributeNames.emplace_back("Number of cases");
        attributeNames.emplace_back("Expected cases");
        if (_parameters.GetProbabilityModelType() == POISSON && _parameters.UsePopulationFile() && _parameters.GetTimeTrendAdjustmentType() != TEMPORAL_STRATIFIED_RANDOMIZATION)
            attributeNames.emplace_back(printString(buffer, "Annual cases / %.0f", _dataHub.GetAnnualRatePop()));
        attributeNames.emplace_back("Observed / expected");
        if (_parameters.GetProbabilityModelType() == POISSON || _parameters.GetProbabilityModelType() == BERNOULLI)
            attributeNames.emplace_back("Relative risk");
        if (_parameters.GetProbabilityModelType() == BERNOULLI)
            attributeNames.emplace_back("Percent cases in area");
        setDataRowValues(attributeNames, clusterAttributes, columnValues);
    }
    if (_parameters.getNumFileSets() == 1)
        writeClusterLevelColumnData(cluster);
    // Finally, write the HTML table row.
    _html_out << "<tr id='" << rowId << "'>";
    for (const auto& colValue : columnValues) {
        if (colValue.second.size())
            _html_out << "<td data-order=" << colValue.second << ">" << colValue.first << "</td>";
        else
            _html_out << "<td>" << colValue.first << "</td>";
    }
    _html_out << "</tr>" << std::endl;
}


/** Writes run information to results files. */
void AnalysisResultsWriter::writeComputationCompletion(time_t startTime, time_t completedTime) {
    std::string runningTime, version, buffer;
    getTotalRunningTime(startTime, completedTime, runningTime);
    printString(version, "SaTScan %s.%s%s%s%s%s",
        VERSION_MAJOR, VERSION_MINOR, (!strcmp(VERSION_RELEASE, "0") ? "" : "."),
        (!strcmp(VERSION_RELEASE, "0") ? "" : VERSION_RELEASE), (strlen(VERSION_PHASE) ? " " : ""), VERSION_PHASE
    );
    PairsContainer_t pairs;
    pairs.emplace_back("Version", version);
    pairs.emplace_back("Program run on", ctime(&startTime)); pairs.back().second.pop_back();
    pairs.emplace_back("Program completed", ctime(&completedTime)); pairs.back().second.pop_back();
    pairs.emplace_back("Total running time", runningTime);
    if (_parameters.GetNumParallelProcessesToExecute() > 1) 
        pairs.emplace_back("Processor Usage", 
            printString(buffer, "%u processors", _parameters.GetNumParallelProcessesToExecute())
        );
    size_t tMaxLabel = 0; //first calculate maximum label length
    for (const auto& s : pairs) tMaxLabel = std::max(tMaxLabel, s.first.size());

    AsciiPrintFormat::PrintSectionSeparatorString(_text_out, 0, 1);
    fprintf(_text_out, "\nRUN INFORMATION\n\n");
    _html_out << "<div class='hr' style='margin-top:5px;margin-bottom:5px;'></div>" << std::endl;
    _html_out << "<div class='program-info run-info'><h4>RUN INFORMATION</h4><table style='text-align: left;'><tbody>" << std::endl;
    for (const auto& p : pairs) { //print settings
        fprintf(_text_out, "%s", p.first.c_str());
        for (size_t t = p.first.size(); t < tMaxLabel; ++t)
            fprintf(_text_out, " ");
        fprintf(_text_out, " : %s\n", p.second.c_str());
        _html_out << "<tr><th>" << p.first << ":</th><td>" << p.second << "</td></tr>" << std::endl;
    }
    _html_out << "</table></div>" << std::endl;
}

void AnalysisResultsWriter::writeHeaderAndSummary(time_t startTime) {
    AsciiPrintFormat PrintFormat;
    PrintFormat.PrintVersionHeader(_text_out);
    if (_parameters.GetTitleName() != "") {
        PrintFormat.PrintAlignedMarginsDataString(_text_out, _parameters.GetTitleName());
    }
    fprintf(_text_out, "\nProgram run on: %s\n", ctime(&startTime));

    std::vector<std::string> statements;
    ParametersPrint(_parameters).getAnalysisSummaryStatements(statements, _dataHub.GetDataSetHandler());
    PrintFormat.SetMarginsAsOverviewSection();
    for (auto& line : statements)
        PrintFormat.PrintAlignedMarginsDataString(_text_out, line);

    CSaTScanData::SummaryPairs_t summaryEntries;
    _dataHub.getSummaryPairs(summaryEntries, true);

    PrintFormat.PrintSectionSeparatorString(_text_out, 0, 2);
    fprintf(_text_out, "SUMMARY OF DATA\n\n");
    PrintFormat.PrintSummaryEntries(_text_out, summaryEntries);
    PrintFormat.PrintSectionSeparatorString(_text_out, 0, 1);

    // Print to same stuff to HTML output
    if (_parameters.GetTitleName() != "") {
        _html_out << "<div class='hr' style='margin-top: 5px;'></div><div class='information'>" << _parameters.GetTitleName() << "</div>" << std::endl;
    }
    _html_out << "<div class='hr' style='margin-top: 5px;'></div><div class='program-info'>" << std::endl;
    _html_out << "<p style='font-size:15px;font-weight:bold;'>";
    for (auto& line : statements)
        _html_out << line << "<br/>";
    _html_out << "</p>";
    _html_out << "<h2 style='font-size:14px;margin:8px 0 5px 0;font-weight:bold;'>Summary Statistics</h2>" << std::endl;

    _html_out << "<table class='analysis-summary'><tbody>" << std::endl;
    for (auto& entry: summaryEntries)
        _html_out << "<tr><th>" << entry.first << ":</th><td>" << entry.second << "</td></tr>" << std::endl;
    _html_out << "</tbody></table></div>" << std::endl;
    _html_out << "<div class='hr'></div>" << std::endl;

    writeHtmlTableStart();
}

/** Start HTML table for cluster reporting. */
void AnalysisResultsWriter::writeHtmlTableStart() {
    auto addColumns = [&](const std::vector<HtmlColumn>& columns) { // lamdba to add columns
        for (const auto& column : columns) _html_columns.emplace_back(column);
    };
    addColumns({ {"No.", true},{"Centroid", true},{"Locations", true} });
    if (_parameters.getClusterMonikerPrefix().size())
        addColumns({ {"Moniker", true} });
    if (_parameters.getReportGiniOptimizedClusters())
        addColumns({ {"Overlap with clusters", true} });
    if (!(_parameters.GetIsPurelyTemporalAnalysis() || _parameters.UseLocationNeighborsFile() ||
          (_parameters.getUseLocationsNetworkFile() && !_dataHub.networkCanReportLocationCoordinates())
    )) {
        addColumns({ {"Coordinates", true, false} });
        if (_parameters.GetCoordinatesType() == CARTESIAN) {
            if (_parameters.GetSpatialWindowType() == ELLIPTIC) {
                addColumns({ {"Semiminor axis", true},{"Semimajor axis", true},{"Angle (degrees)", true},{"Shape", true} });
            } else {
                if (!_parameters.getUseLocationsNetworkFile())
                    addColumns({ {"Radius", true} });
                if (_parameters.getUseLocationsNetworkFile())
                    addColumns({ {"Ctr to utmost location", true} });
            }
        } else if (_parameters.GetCoordinatesType() == LATLON) {
            if (!_parameters.getUseLocationsNetworkFile())
                addColumns({ {"Radius", true} });
            if (_parameters.getUseLocationsNetworkFile())
                addColumns({ {"Ctr to utmost location", true} });
        }
        addColumns({ {"Span", true} });
    }
    if (_parameters.getReportGiniOptimizedClusters())
        addColumns({ {"Gini Cluster", true} });
    if (_parameters.GetProbabilityModelType() != HOMOGENEOUSPOISSON && !_parameters.GetIsPurelySpatialAnalysis() && _parameters.GetAnalysisType() != SPATIALVARTEMPTREND)
        addColumns({ {"Time frame", true} });
    auto defineClusterLevelColumns = [&]() { // lamdba to add cluster level columns
        if (_parameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
            if (_parameters.getTimeTrendType() == LINEAR)
                addColumns({ {"Inside time trend", true, false},{"Outside time trend", true, false} });
            else if (_parameters.getTimeTrendType() == QUADRATIC)
                addColumns({ {"Inside Risk Function", true, false},{"Outside Risk Function", true, false} });
        }
        if (_parameters.IsTestStatistic(false)) {
            addColumns({ {"Test statistic", true} });
        } else {
            addColumns({ {"Log likelihood ratio", true} });
            if (_parameters.GetSpatialWindowType() == ELLIPTIC)
                addColumns({ {"Test statistic", true} });
        }
        if (_parameters.GetNumReplicationsRequested() >= MIN_SIMULATION_RPT_PVALUE) {
            if (_parameters.getReportClusterRank())
                addColumns({ {"Monte Carlo rank", true} });
            addColumns({ {"P-value", true} });
            if (_parameters.GetIsProspectiveAnalysis())
                addColumns({ {"Recurrence interval", true} });
            if (_parameters.getIsReportingGumbelAsAddon())
                addColumns({ {"Gumbel P-value", true} });
        }
    };
    if (_parameters.getNumFileSets() > 1) defineClusterLevelColumns();
    if (_parameters.GetProbabilityModelType() == ORDINAL || _parameters.GetProbabilityModelType() == CATEGORICAL) {
        addColumns({
            {"Total cases", false}, {"Category", false}, {"Number of cases", false}, {"Expected cases", false},
            {"Observed/expected", false}, {"Relative risk", false}, {"Percent cases in area", false} 
        });
    } else if (_parameters.GetProbabilityModelType() == EXPONENTIAL) {
        addColumns({ {"Total individuals", false}, {"Number of cases", false}, {"Expected cases", false}, {"Observed/expected", false} });
    } else if (_parameters.GetProbabilityModelType() == NORMAL) {
        if (_parameters.getIsWeightedNormal()) {
            addColumns({ 
                {"Number of cases", false}, {"Total weights", false}, {"Mean inside", false}, {"Mean outside", false},
                {"Variance", false}, {"Standard deviation", false}, {"Weighted mean inside", false}, {"Weighted mean outside", false},
                {"Weighted variance", false}, {"Weighted std deviation", false} 
             });
        } else {
            addColumns({ 
                {"Number of cases", false}, {"Mean inside", false}, {"Mean outside", false}, {"Variance", false}, {"Standard deviation", false} 
            });
        }
    } else if (_parameters.GetProbabilityModelType() == RANK) {
        addColumns({ {"Number of cases", false}, {"Average Rank Inside", false}, {"Average Rank Outside", false} });
    } else if (_parameters.GetProbabilityModelType() == BATCHED) {
        addColumns({ 
            {"Number of batches", false}, {"Obs. positive batches", false}, {"Exp. positive batches", false},
            {"Number of individuals", false}, {"Positive individuals inside", false}, {"Positive individuals outside", false} 
        });
    } else {
        if ((_parameters.GetProbabilityModelType() == POISSON && _parameters.UsePopulationFile() && !_parameters.GetIsPurelyTemporalAnalysis()) || 
            _parameters.GetProbabilityModelType() == BERNOULLI)
            addColumns({ { "Population", false} });
        addColumns({ { "Number of cases", false}, { "Expected cases", false} });
        if (_parameters.GetProbabilityModelType() == POISSON && _parameters.UsePopulationFile() && _parameters.GetTimeTrendAdjustmentType() != TEMPORAL_STRATIFIED_RANDOMIZATION) {
            std::string buffer;
            addColumns({ { printString(buffer, "Annual cases / %.0f", _dataHub.GetAnnualRatePop()), false} });
        }
        addColumns({ { "Observed/expected", false} });
        if (_parameters.GetProbabilityModelType() == POISSON || _parameters.GetProbabilityModelType() == BERNOULLI)
            addColumns({ { "Relative risk", false} });
        if (_parameters.GetProbabilityModelType() == BERNOULLI)
            addColumns({ { "Percent cases in area", false} });
    }
    if (_parameters.getNumFileSets() == 1) defineClusterLevelColumns();
    _html_out << "<div id='cuts'><h3>CLUSTERS DETECTED</h3><div>" << std::endl << "<table id='id_cuts' class='display' style='width:100%'>" << std::endl << "<thead><tr>";
    for (const auto& column: _html_columns)
        _html_out << "<th" << (_parameters.getNumFileSets() > 1 && !column._cluster_level || !column._sortable ? " class='no-sort'" : "") << ">" << column._header << "</th>";
    _html_out << "</tr></thead>" << std::endl << "<tbody>" << std::endl;
}

void AnalysisResultsWriter::writeHtmlTableEnd() {
    _html_out << "</tbody>" << std::endl << "</table>" << std::endl << "</div></div>";
    // Write any sub rows data
    if (_html_sub_rows.rdbuf()->in_avail()) {
        _html_out << std::endl << "<script type='text/javascript'>" << std::endl
            << "var sub_rows = {" << std::endl<< _html_sub_rows.str() << std::endl << "};" << std::endl 
            << "</script>" << std::endl;
    }
}

void AnalysisResultsWriter::writeMessage(
    const std::string& message, const std::string& divClass
) {
    AsciiPrintFormat PrintFormat;
    PrintFormat.SetMarginsAsOverviewSection();
    PrintFormat.PrintSectionSeparatorString(_text_out, 0, 2, '_');
    PrintFormat.PrintAlignedMarginsDataString(_text_out, message);
    _html_messages << "<div class='hr' style='margin-top:5px;margin-bottom:5px;'></div>" << std::endl;
    _html_messages << "<div class='" << divClass << "'>" << message << "</div>" << std::endl;
}

void AnalysisResultsWriter::writeMessage(const std::vector<std::string>& message, const std::string& header, const std::string& divClass) {
    AsciiPrintFormat PrintFormat;
    PrintFormat.SetMarginsAsOverviewSection();
    if (header.size()) {
        PrintFormat.PrintSectionSeparatorString(_text_out, 0, 2, '_');
        PrintFormat.PrintAlignedMarginsDataString(_text_out, header, 1);
    }
    fprintf(_text_out, "\n");
    for (const auto& line : message)
        PrintFormat.PrintAlignedMarginsDataString(_text_out, line);

    writeMessageHtml(message, header, divClass);
}

void AnalysisResultsWriter::writeMessageHtml(const std::vector<std::string>& message, const std::string& header, const std::string& divClass) {
    _html_messages << "<div class='hr' style='margin-top:5px;margin-bottom:5px;'></div>" << std::endl;
    _html_messages << "<div class='" << divClass << "'>" << std::endl;
    if (header.size()) _html_messages << header << "<br/>" << std::endl;
    for (const auto& line : message)
        _html_messages << line << "<br/>" << std::endl;
    _html_messages << "</div>" << std::endl;
}

void AnalysisResultsWriter::writeMessageListStart(const std::string& message, const std::string& divClass, unsigned int iPreNewlines) {
    AsciiPrintFormat PrintFormat;
    PrintFormat.SetMarginsAsOverviewSection();
    //while (iPreNewlines--) fprintf(_text_out, "\n");
    PrintFormat.PrintSectionSeparatorString(_text_out, iPreNewlines, 2, '_');
    PrintFormat.PrintAlignedMarginsDataString(_text_out, message);

    _html_messages << "<div class='hr' style='margin-top:5px;margin-bottom:5px;'></div>" << std::endl;
    _html_messages << "<div class='toggle-message show-chart-options " << divClass << "'><a href='#'>" << message << "</a></div>" << std::endl;
    _html_messages << "<div class='chart-options program-info' style='display:none'>" << std::endl;
}

void AnalysisResultsWriter::writeMessageListLine(const std::string& message) {
    AsciiPrintFormat PrintFormat;
    PrintFormat.SetMarginsAsOverviewSection();
    PrintFormat.PrintAlignedMarginsDataString(_text_out, message);

    _html_messages << "<div>" << message << "</div>" << std::endl;
}

void AnalysisResultsWriter::writeMessageListEnd(unsigned int iPostNewlines) {
    while (iPostNewlines--) fprintf(_text_out, "\n");
    _html_messages << "</div>" << std::endl;
}


void AnalysisResultsWriter::writeParameters(bool isDrilldown) {
    // First write any messages
    if (_html_messages.rdbuf()->in_avail())
        _html_out << std::endl << _html_messages.str() << std::endl;
    ParametersPrint(_parameters).Print(*this, false);
}
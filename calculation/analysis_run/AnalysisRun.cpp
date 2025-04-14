//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#if defined(__APPLE__)
#include <sys/sysctl.h>
#endif

#include "AnalysisRun.h"
#include "PurelySpatialData.h"
#include "PurelyTemporalData.h"
#include "SpaceTimeData.h"
#include "SVTTData.h"
#include "ClosedLoopData.h"
#include "stsRunHistoryFile.h"
#include "LoglikelihoodRatioWriter.h"
#include "ClusterInformationWriter.h"
#include "PurelySpatialAnalysis.h"
#include "PurelySpatialMonotoneAnalysis.h"
#include "PurelyTemporalAnalysis.h"
#include "SpaceTimeAnalysis.h"
#include "SpaceTimeIncludePurelySpatialAnalysis.h"
#include "SpaceTimeIncludePurelyTemporalAnalysis.h"
#include "SpaceTimeIncludePureAnalysis.h"
#include "SVTTAnalysis.h"
#include "PrintQueue.h"
#include "stsMonteCarloSimFunctor.h"
#include "stsMCSimJobSource.h"
#include "stsCentricAlgoJobSource.h"
#include "stsCentricAlgoFunctor.h"
#include "contractor.h"
#include "PurelySpatialCentricAnalysis.h"
#include "SpaceTimeCentricAnalysis.h"
#include "SpaceTimeIncludePurelySpatialCentricAnalysis.h"
#include "ParametersPrint.h"
#include "SSException.h" 
#include "SVTTCentricAnalysis.h"
#include "PurelySpatialBruteForceAnalysis.h"
#include "ClusterKML.h"
#include "ChartGenerator.h"
#include "PoissonRandomizer.h"
#include "OliveiraJobSource.h"
#include "OliveiraFunctor.h"
#include "DataSource.h"
#include "ParametersValidate.h"
#include "OrdinalLikelihoodCalculation.h"
#include "CategoricalClusterData.h"
#include "MultipleAnalyses.h"
#include <boost/assign/std/vector.hpp>
#include <algorithm>
using namespace boost::assign;
using boost::algorithm::ireplace_all;

const double AbstractAnalysisDrilldown::DEFAULT_CUTOFF_PVALUE = 0.05;
const int BernoulliAnalysisDrilldown::DEFAULT_NUM_ITERATIVE_SCANS = 10;
const double BernoulliAnalysisDrilldown::DEFAULT_ITERATIVE_CUTOFF_PVALUE = 0.05;

//////////////////////////// AbstractAnalysisDrilldown ///////////////////////////////////

bool AbstractAnalysisDrilldown::shouldDrilldown(const CCluster& cluster, unsigned int clusterRptIdx, const CSaTScanData& data, const CParameters& parameters, const SimulationVariables& simvars) {
    bool should = static_cast<unsigned int>(cluster.getNumIdentifiers()) >= parameters.getDrilldownMinimumLocationsCluster() && cluster.GetClusterType() != PURELYTEMPORALCLUSTER;
    if (parameters.GetIsProspectiveAnalysis() && !data.isDrilldown()) {
        should &= cluster.reportableRecurrenceInterval(parameters, simvars) &&
            std::round(cluster.GetRecurrenceInterval(data, clusterRptIdx, simvars).second) >= parameters.getDrilldownCutoff(); // round RI to whole days
    } else {
        should &= cluster.reportablePValue(parameters, simvars) &&
            cluster.getReportingPValue(parameters, simvars, clusterRptIdx == 0 || parameters.GetIsIterativeScanning()) <= parameters.getDrilldownCutoff();
    }
    // One additional check for Bernoulli analysis -- there must be controls as well.
    if (parameters.GetProbabilityModelType() == BERNOULLI) {
        bool anyControls = false;
        for (size_t t=0; t < data.GetNumDataSets(); ++t)
            anyControls |= (static_cast<count_t>(data.GetProbabilityModel().GetPopulation(t, cluster, data)) - cluster.GetObservedCount()) > 0;
        should &= anyControls;
    }
    // Check that the number of cases, across all data sets, meets the minimum number of cases restriction.
    count_t totalCasesAllSets = 0;
    if (parameters.GetProbabilityModelType() == ORDINAL || parameters.GetProbabilityModelType() == CATEGORICAL) {
        // Specialized behavior for ordinal and multinomial models - exclude category with the most cases.
        const AbstractCategoricalClusterData * pClusterData = 0;
        if ((pClusterData = dynamic_cast<const AbstractCategoricalClusterData*>(cluster.GetClusterData())) == 0)
            throw prg_error("Cluster data object could not be dynamically casted to AbstractCategoricalClusterData type.\n", "shouldDrilldown()");
        OrdinalLikelihoodCalculator Calculator(data);
        std::vector<OrdinalCombinedCategory> vCategoryContainer;
        for (size_t t=0; t < data.GetNumDataSets(); ++t) {
            pClusterData->GetOrdinalCombinedCategories(Calculator, vCategoryContainer, t);
            count_t maxCategory = 0, totalCases = 0;
            for (std::vector<OrdinalCombinedCategory>::iterator itrCategory=vCategoryContainer.begin(); itrCategory != vCategoryContainer.end(); ++itrCategory) {
                count_t tObserved = 0;
                for (size_t m=0; m < itrCategory->GetNumCombinedCategories(); ++m)
                    tObserved += cluster.GetObservedCountOrdinal(t, itrCategory->GetCategoryIndex(m));
                totalCases += tObserved;
                maxCategory = std::max(maxCategory, tObserved);
            }
            totalCasesAllSets += totalCases - maxCategory;
        }
    } else {
        for (size_t t=0; t < data.GetNumDataSets(); ++t)
            totalCasesAllSets += cluster.GetObservedCount(t);
    }
    should &= static_cast<unsigned int>(totalCasesAllSets) >= parameters.getDrilldownMinimumCasesCluster();
    return should;
}

//////////////////////////// AnalysisExecution ///////////////////////////////////

AnalysisExecution::AnalysisExecution(CSaTScanData& data_hub, ExecutionType executing_type, time_t start, unsigned int& drilldowns)
    :_print_direction(data_hub.GetPrintDirection()), _parameters(data_hub.GetParameters()), _data_hub(data_hub), _start_time(start), _clustersReported(false),
    _executing_type(executing_type), _analysis_count(0), _significant_at005(0), _significant_clusters(0), _reportClusters(0), _drilldowns(drilldowns){
    try {
        for (std::vector<double>::const_iterator itr = _parameters.getExecuteSpatialWindowStops().begin(); itr != _parameters.getExecuteSpatialWindowStops().end(); ++itr)
            _top_clusters_containers.push_back(MostLikelyClustersContainer(*itr));
        if (_parameters.GetReportCriticalValues() || (_parameters.getPerformPowerEvaluation() && _parameters.getPowerEvaluationCriticalValueType() == CV_MONTECARLO))
            _significant_ratios.reset(new SignificantRatios(_parameters.GetNumReplicationsRequested()));
        _relevance_tracker.reset(new LocationRelevance());
        _base_output = _parameters.GetOutputFileName(); // Default
    } catch (prg_exception& x) {
        x.addTrace("constructor()", "AnalysisExecution");
        throw;
    }
}

/** If the user requested gini clusters, add those clusters to passed MLC container. */
void AnalysisExecution::addGiniClusters(MLC_Collections_t& mlc_collections, MostLikelyClustersContainer& mlc, double p_value_cutoff) {
    if (_parameters.getReportGiniOptimizedClusters() && mlc_collections.size() > 0) {
        // cluster reporting for index based cluster collections can either be only the optimal collection or all collections
        if (_parameters.getGiniIndexReportType() == OPTIMAL_ONLY) {
            const MostLikelyClustersContainer * optimal = getOptimalGiniContainerByPValue(mlc_collections, p_value_cutoff);
            if (optimal) mlc.combine(*optimal, _data_hub, _sim_vars, true);
        }
        else {
            // combine clusters from each maxima collection with reporting collection
            for (MLC_Collections_t::const_iterator itrMLC = mlc_collections.begin(); itrMLC != mlc_collections.end(); ++itrMLC)
                mlc.combine(*itrMLC, _data_hub, _sim_vars, true);
        }
        // now sort combined cluster collection by LLR
        mlc.sort(_data_hub);
    }
}

/** calculates most likely clusters in real data */
void AnalysisExecution::calculateMostLikelyClusters() {
    try {
        //display process heading
        printFindClusterHeading();
        //allocate date gateway object
        std::auto_ptr<AbstractDataSetGateway> pDataSetGateway(_data_hub.GetDataSetHandler().GetNewDataGatewayObject());
        _data_hub.GetDataSetHandler().GetDataGateway(*pDataSetGateway);
        //get analysis object
        std::auto_ptr<CAnalysis> pAnalysis(getNewAnalysisObject(_print_direction));
        //allocate objects used in 'FindTopClusters()' process
        pAnalysis->AllocateTopClustersObjects(*pDataSetGateway);
        //calculate most likely clusters
        _data_hub.SetActiveNeighborReferenceType(CSaTScanData::REPORTED);
        pAnalysis->FindTopClusters(*pDataSetGateway, _top_clusters_containers);
        //display the loglikelihood of most likely cluster
        if (!_print_direction.GetIsCanceled() && _top_clusters_containers.size() > 0) {
            rankClusterCollections(_top_clusters_containers, _reportClusters, &_clusterRanker, _print_direction);
            // Note: If we're not reporting hierarchical, then this might report a cluster that is not displayed in final output.
            //       We can't perform index based ordering here since we need to perform simulations first ... correct?
            printTopClusterLogLikelihood(getLargestMaximaClusterCollection());
        }
    } catch (prg_exception& x) {
        x.addTrace("calculateMostLikelyClusters()", "AnalysisExecution");
        throw;
    }
}

/** Iterates through all reporting clusters to determine which overlap with other clusters geographically.
    Overlapping added a cluster information, to be reported in results file. */
void AnalysisExecution::calculateOverlappingClusters(const MostLikelyClustersContainer& mlc, ClusterSupplementInfo& clusterSupplement) {
    if (_parameters.GetCriteriaSecondClustersType() == NOGEOOVERLAP && !_parameters.getReportGiniOptimizedClusters()) return;

    size_t numClusters = std::min(static_cast<size_t>(mlc.GetNumClustersRetained()), clusterSupplement.size());
    // allocate bit sets that will track which clusters overlap with cluster at vector index
    std::vector<boost::dynamic_bitset<> > overlappingClusters;
    for (size_t t = 0; t < numClusters; ++t)
        overlappingClusters.push_back(boost::dynamic_bitset<>(numClusters));
    // iterate over all reporting clusters, checking for overlapping locations
    for (size_t i = 0; i < numClusters; ++i) {
        const CCluster& cluster = mlc.GetCluster(i);
        if (cluster.GetClusterType() == PURELYTEMPORALCLUSTER) continue;
        boost::dynamic_bitset<>& clusterSet = overlappingClusters[i];
        for (size_t j = 1; j < numClusters; ++j) {
            if (i == j) continue; // skip self
            if (overlappingClusters[j].test(i)) // other cluster already signals overlap with this cluster
                clusterSet.set(j);
            else if (mlc.HasAnyTractsInCommon(_data_hub, cluster, mlc.GetCluster(j))) {
                clusterSet.set(j);
                overlappingClusters[j].set(i);
            }
        }
    }
    // add to extra report lines which clusters each cluster overlaps with
    for (size_t t = 0; t < numClusters; ++t)
        clusterSupplement.setOverlappingClusters(mlc.GetCluster(t), overlappingClusters[t]);
}

/* Finds the collections of clusters about requested number of data sets for Oliveira's F calculation. */
void AnalysisExecution::calculateOliveirasF() {
    {
        _print_direction.Printf("Calculating Oliveira's F ...\n", BasePrint::P_STDOUT);
        // create oliveira data sets -- the cases from real data set are used as the measure in these sets
        const RealDataContainer_t& oliveira_datasets = _data_hub.GetDataSetHandler().buildOliveiraDataSets();

        PrintQueue lclPrintDirection(_print_direction, _parameters.GetSuppressingWarnings());
        OliveiraJobSource jobSource(*this, ::GetCurrentTime_HighResolution(), lclPrintDirection);
        typedef contractor<OliveiraJobSource> contractor_type;
        contractor_type theContractor(jobSource);
        PrintNull nullPrint;

        //run threads:
        boost::thread_group tg;
        boost::mutex thread_mutex;
        unsigned long ulParallelProcessCount = std::min(_parameters.GetNumParallelProcessesToExecute(), _parameters.getNumRequestedOliveiraSets());
        for (unsigned u = 0; u < ulParallelProcessCount; ++u) {
            try {
                OliveiraFunctor oliveiraf(oliveira_datasets, *this, boost::shared_ptr<CAnalysis>(getNewAnalysisObject(nullPrint)));
                tg.create_thread(subcontractor<contractor_type, OliveiraFunctor>(theContractor, oliveiraf));
            }
            catch (std::bad_alloc &b) {
                if (u == 0) throw; // if this is the first thread, re-throw exception
                _print_direction.Printf("Notice: Insufficient memory to create %u%s parallel simulation ... continuing analysis with %u parallel simulations.\n",
                    BasePrint::P_NOTICE, u + 1, (u == 1 ? "nd" : (u == 2 ? "rd" : "th")), u);
                break;
            }
            catch (prg_exception& x) {
                if (u == 0) throw; // if this is the first thread, re-throw exception
                _print_direction.Printf("Error: Program exception occurred creating %u%s parallel simulation ... continuing analysis with %u parallel simulations.\nException:%s\n",
                    BasePrint::P_ERROR, u + 1, (u == 1 ? "nd" : (u == 2 ? "rd" : "th")), u, x.what());
                break;
            }
            catch (...) {
                if (u == 0) throw prg_error("Unknown program error occurred.\n", "PerformSuccessiveSimulations_Parallel()"); // if this is the first thread, throw exception
                _print_direction.Printf("Error: Unknown program exception occurred creating %u%s parallel simulation ... continuing analysis with %u parallel simulations.\n",
                    BasePrint::P_ERROR, u + 1, (u == 1 ? "nd" : (u == 2 ? "rd" : "th")), u);
                break;
            }
        }
        tg.join_all();

        //propagate exceptions if needed:
        theContractor.throw_unhandled_exception();
        jobSource.Assert_NoExceptionsCaught();
        if (jobSource.GetUnregisteredJobCount() > 0)
            throw prg_error("At least %d jobs remain uncompleted.", "AnalysisExecution", jobSource.GetUnregisteredJobCount());
    }
}

/* Creates executes analysis and writes results to output files. */
void AnalysisExecution::execute() {
    try {
        createReport();
        // conditionally perform standard analysis based upon power evaluation settings
        if (!_parameters.getPerformPowerEvaluation() || (_parameters.getPerformPowerEvaluation() && _parameters.getPowerEvaluationMethod() == PE_WITH_ANALYSIS)) {
            switch (_executing_type) {
                case CENTRICALLY: executeCentricEvaluation(); break;
                case SUCCESSIVELY:
                default: executeSuccessively();
            }
        }
        // conditionally perform power evaluation
        if (_parameters.getPerformPowerEvaluation())
            executePowerEvaluations();
    } catch (std::bad_alloc &b) {
        std::string additional;
        //Potentially provide detailed options given user parameter settings:
        if (_executing_type == SUCCESSIVELY && _parameters.GetPermitsCentricExecution(true) &&
            _parameters.GetPValueReportingType() == TERMINATION_PVALUE && _parameters.GetNumReplicationsRequested() >= MIN_SIMULATION_RPT_PVALUE) {
            additional = "\nNote: SaTScan could not utilize the alternative memory allocation for\n"
                "this analysis because of the P-Value reporting setting (sequential Monte Carlo).\n"
                "Consider changing this setting, which will enable analysis to utilize the\n"
                "alternative memory allocation and possibly execute without memory issues.\n";
        }
        throw resolvable_error("\nSaTScan is unable to perform analysis due to insufficient memory.\n"
            "Please see 'Memory Requirements' in user guide for suggested solutions.\n%s", additional.c_str());
  } catch (prg_exception& x) {
      x.addTrace("execute()", "AnalysisExecution");
      throw;
  }
}

void AnalysisExecution::finalize() {
    FILE * fp = 0;
    try {
        // Finalize the data demograhics process now -- updating cache file for this analysis.
        if (_data_demographic_processor.get()) _data_demographic_processor->finalize();

        /** Finalizes the reporting to result output file.
        - indicates whether clusters were found
        - indicates whether no clusters were reported because their loglikelihood
        ratios are less than defined minimum value
        - if the number of simulations are less 98, reported that the reported
        clusters intentially do not contain p-values */
        
        time_t CompletionTime;
        double nTotalTime, nSeconds, nMinutes, nHours;
        const char * szHours = "hours", * szMinutes = "minutes", * szSeconds = "seconds";
        AsciiPrintFormat PrintFormat;
        std::string buffer;

        _print_direction.Printf("Printing analysis settings to the results file ...\n", BasePrint::P_STDOUT);
        openReportFile(fp, true);
        PrintFormat.SetMarginsAsOverviewSection();
        if (_clustersReported && _parameters.GetNumReplicationsRequested() == 0) {
            fprintf(fp, "\n");
            buffer = "Note: As the number of Monte Carlo replications was set to zero, no hypothesis testing was done and no p-values are reported.";
            PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
        }
        if (_clustersReported && _parameters.GetNumReplicationsRequested() > 0 && _parameters.GetNumReplicationsRequested() < MIN_SIMULATION_RPT_PVALUE) {
            fprintf(fp, "\n");
            buffer = "Note: The number of Monte Carlo replications was set too low, "
            "and a meaningful hypothesis test cannot be done. Consequently, no p-values are reported.";
            PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
        }
        if (_parameters.GetProbabilityModelType() == POISSON && !_data_hub.isDrilldown())
            _data_hub.GetDataSetHandler().ReportZeroPops(_data_hub, fp, &_print_direction);
        if (!_data_hub.isDrilldown() && _parameters.GetMultipleCoordinatesType() == ONEPERLOCATION)
            _data_hub.getIdentifierInfo().reportCombinedIdentifiers(fp);
        ParametersPrint(_parameters).Print(fp, _data_hub.isDrilldown());
        macroRunTimeManagerPrint(fp);
        time(&CompletionTime);
        nTotalTime = difftime(CompletionTime, _start_time);
        nHours = floor(nTotalTime / (60 * 60));
        nMinutes = floor((nTotalTime - nHours * 60 * 60) / 60);
        nSeconds = nTotalTime - (nHours * 60 * 60) - (nMinutes * 60);
        fprintf(fp, "\nRUN INFORMATION\n\nProgram completed  : %s", ctime(&CompletionTime));
        if (0 < nHours && nHours < 1.5) szHours = "hour";
        if (0 < nMinutes && nMinutes < 1.5) szMinutes = "minute";
        if (0.5 <= nSeconds && nSeconds < 1.5) szSeconds = "second";
        if (nHours > 0) fprintf(fp, "Total Running Time : %.0f %s %.0f %s %.0f %s", nHours, szHours, nMinutes, szMinutes, nSeconds, szSeconds);
        else if (nMinutes > 0) fprintf(fp, "Total Running Time : %.0f %s %.0f %s", nMinutes, szMinutes, nSeconds, szSeconds);
        else fprintf(fp, "Total Running Time : %.0f %s", nSeconds, szSeconds);
        if (_parameters.GetNumParallelProcessesToExecute() > 1) fprintf(fp, "\nProcessor Usage    : %u processors", _parameters.GetNumParallelProcessesToExecute());
        fclose(fp); fp = 0;
        // Create a temporary file which will be used to generate the multiple analyses summary email.
        if (!_data_hub.isDrilldown() && _parameters.getCreateEmailSummaryFile()) {
            CCluster::RecurrenceInterval_t ri_cutoff(1, _parameters.getEmailSummaryValue());
            unsigned int metCutoff = 0;
            for (tract_t i=0; i < _reportClusters.GetNumClustersRetained(); ++i) {
                if (!_reportClusters.GetCluster(i).meetsCutoff(_data_hub, 1, _sim_vars, ri_cutoff, _parameters.getEmailSummaryValue()))
                    break;
                ++metCutoff;
            }
            // Write temp file that will be used when creating the multiple analysis summary email.
            printString(buffer, "%s%s", _parameters.GetOutputFileName().c_str(), MultipleAnalyses::_EMAIL_TEMP_EXTENSION.c_str());
            std::ofstream summary_tmp(buffer.c_str(), std::ios_base::trunc);
            summary_tmp << metCutoff;
        }

        // Send email per user settings.
        bool clusterMeetsCutoff = false;
        if ((_parameters.getAlwaysEmailSummary() || _parameters.getCutoffEmailSummary()) && !_data_hub.isDrilldown() && _sim_vars.get_sim_count() != 0) {
            std::stringstream messageSubjectLine, messageBody, messagePlain, messageHTML, summaryParagraph;
            // Create a summary paragraph of the analysis results. 
            if (_reportClusters.GetNumClustersRetained() == 0) {
                summaryParagraph << "No clusters were found by this analysis.";
            } else {
                CCluster::RecurrenceInterval_t ri_cutoff(1, _parameters.getCutoffEmailValue());
                const auto& cluster = _reportClusters.GetCluster(0); // Get the most likely cluster
                clusterMeetsCutoff = (bool)cluster.meetsCutoff(_data_hub, 1, _sim_vars, ri_cutoff, _parameters.getCutoffEmailValue());
                summaryParagraph << "SaTScan found a most likely cluster ";
                if (cluster.GetClusterType() != PURELYTEMPORALCLUSTER)
                    summaryParagraph << " centered on " << cluster.GetClusterLocation(buffer, _data_hub);
                if (!(_parameters.GetIsPurelyTemporalAnalysis() || _parameters.UseLocationNeighborsFile() ||
                    (_parameters.getUseLocationsNetworkFile() && !_data_hub.networkCanReportLocationCoordinates()))) {
                    double span = cluster.getLocationsSpan(_data_hub);
                    if (_parameters.GetCoordinatesType() == CARTESIAN && _parameters.GetSpatialWindowType() == ELLIPTIC) {
                        double radius = cluster.GetCartesianRadius();
                        summaryParagraph << EmailText::LINEBREAK << "Semiminor Axis: " << getValueAsString(radius, buffer);
                        summaryParagraph << EmailText::LINEBREAK << "Semimajor Axis: " << getValueAsString(radius * _data_hub.GetEllipseShape(cluster.GetEllipseOffset()), buffer);
                        if (span >= 0.0) summaryParagraph << EmailText::LINEBREAK << "Span: " << getValueAsString(span, buffer);
                    } else if (_parameters.GetCoordinatesType() == CARTESIAN && _parameters.GetSpatialWindowType() == CIRCULAR) {
                        summaryParagraph << EmailText::LINEBREAK << "Radius: " << getValueAsString(cluster.GetCartesianRadius(), buffer);
                        if (span >= 0.0) summaryParagraph << EmailText::LINEBREAK << "Span: " << getValueAsString(span, buffer);
                    } else if (_parameters.GetCoordinatesType() == LATLON && !_parameters.getUseLocationsNetworkFile()) {
                        summaryParagraph << EmailText::LINEBREAK << "Radius: " << getValueAsString(cluster.GetLatLongRadius(), buffer) << " km";
                        if (span >= 0.0) summaryParagraph << EmailText::LINEBREAK << "Span: " << getValueAsString(span, buffer) << " km";
                    } else if (span >= 0.0) 
                        summaryParagraph << EmailText::LINEBREAK << "Span: " << getValueAsString(span, buffer) << " km";
                }
                // Create map to cluster cached values - easier and format will match that of other output files.
                std::map<std::string, std::string> clusterAttributes;
                for (const auto& entry : cluster.getReportLinesCache()) {
                    if (clusterAttributes.find(entry.first) == clusterAttributes.end())
                        clusterAttributes.insert(std::make_pair(entry.first, entry.second.first));
                    else
                        clusterAttributes[entry.first] += printString(buffer, ", %s", entry.second.first.c_str());
                }
                if (clusterAttributes.find("Time frame") != clusterAttributes.end())
                    summaryParagraph << EmailText::LINEBREAK << "Time frame: " << clusterAttributes["Time frame"];
                if (clusterAttributes.find("Number of cases") != clusterAttributes.end())
                    summaryParagraph << EmailText::LINEBREAK << "Number of cases: " << clusterAttributes["Number of cases"];
                if (clusterAttributes.find("Relative risk") != clusterAttributes.end())
                    summaryParagraph << EmailText::LINEBREAK << "Relative risk: " << clusterAttributes["Relative risk"];
                else if (clusterAttributes.find("Observed / expected") != clusterAttributes.end())
                    summaryParagraph << EmailText::LINEBREAK << "Observed / expected: " << clusterAttributes["Observed / expected"];
                if (clusterAttributes.find("Recurrence interval") != clusterAttributes.end())
                    summaryParagraph << EmailText::LINEBREAK << "Recurrence Interval: " << clusterAttributes["Recurrence interval"];
                else if (clusterAttributes.find("Gumbel P-value") != clusterAttributes.end())
                    summaryParagraph << EmailText::LINEBREAK << "Gumbel P-value: " << clusterAttributes["Gumbel P-value"];
                else if (clusterAttributes.find("P-value") != clusterAttributes.end())
                    summaryParagraph << EmailText::LINEBREAK << "P-value: " << clusterAttributes["P-value"];
                if (_parameters.getCutoffEmailSummary() && clusterMeetsCutoff) {
                    // If cluster meets cut-off, count the number of other clusters that also met it.
                    unsigned int othersCutoff = 0;
                    for (tract_t i=1; i < _reportClusters.GetNumClustersRetained(); ++i) {
                        if (!_reportClusters.GetCluster(i).meetsCutoff(_data_hub, i + 1, _sim_vars, ri_cutoff, _parameters.getCutoffEmailValue()))
                            continue;
                        ++othersCutoff;
                    }
                    if (othersCutoff) {
                        summaryParagraph << EmailText::LINEBREAK << EmailText::LINEBREAK << "There " << (othersCutoff == 1 ? "was" : "were")
                            << " " << othersCutoff << " additional cluster" << (othersCutoff == 1 ? "" : "s") << " with a ";
                        if (_parameters.GetIsProspectiveAnalysis())
                            summaryParagraph << "recurrence interval >= " << ri_cutoff.second << " day" << (ri_cutoff.second == 1 ? "" : "s") << ".";
                        else
                            summaryParagraph << "p-value <= " << _parameters.getCutoffEmailValue() << ".";
                    }
                }
            }
            // If the user defined a custom message, build that message now.
            if (_parameters.getEmailCustom()) {
                messageSubjectLine << _parameters.getEmailCustomSubject();
                messageBody << _parameters.getEmailCustomMessageBody();
                bool usingCache = !_parameters.getLinelistIndividualsCacheFileName().empty();
                std::stringstream signaltext;
                std::string customMessageBody(messageBody.str());
                /* If the user specified line-list data in the case file, we might also have individuals to help distinguish if a
                    cluster should be considered when emailing -- otherwise repeated noise i.e.same cluster as yesterdays run. */
                if (_parameters.getReadingLineDataFromCasefile() && _data_demographic_processor.get() && _data_demographic_processor->hasIndividualAttribute()) {
                    const auto& cluster_counts = _data_demographic_processor->getClusterEventTotals();
                    for (tract_t i = 0; i < _reportClusters.GetNumClustersRetained(); ++i) {
                        if (!DataDemographicsProcessor::isReportedInCsv(_data_hub, _reportClusters.GetCluster(i), i + 1, _sim_vars))
                            continue; // Use same cutoff as line list cluster csv
                        unsigned int newcases = cluster_counts.at(static_cast<int>(i)).first;
                        signaltext << (signaltext.str().size() ? "" : "All clusters in the line list file:") << EmailText::LINEBREAK << "Cluster #" << (i + 1);
                        count_t totalcases = cluster_counts.at(static_cast<int>(i)).second;
                        if (newcases == totalcases)
                            signaltext << " is a " << (usingCache ? "new " : "") << "signal, with " << newcases << " case" << (newcases == 1 ? "" : "s");
                        else
                            signaltext << " is an ongoing signal, with " << (totalcases - newcases) << " old and " << newcases << " new case" << (newcases == 1 ? "" : "s");
                        // Report RI or p-value of cluster.
                        std::stringstream clustersigtext;
                        const auto& cluster = _reportClusters.GetCluster(i);
                        for (const auto& entry : cluster.getReportLinesCache()) {
                            if (_parameters.GetIsProspectiveAnalysis() && entry.first == "Recurrence interval") {
                                clustersigtext << "RI=" << entry.second.first; break;
                            } else if (!_parameters.GetIsProspectiveAnalysis() && (entry.first == "Gumbel P-value" || entry.first == "P-value")) {
                                clustersigtext << "P-value=" << entry.second.first; break;
                            }
                        }
                        if (clustersigtext.rdbuf()->in_avail()) signaltext << " (" << clustersigtext.str() << ")";
                        signaltext << ".";
                    }
                }
                ireplace_all(customMessageBody, EmailText::LINELIST_PAR, (signaltext.str().size() ? signaltext.str() : std::string("No clusters signaled in this analysis.")));
                ireplace_all(customMessageBody, EmailText::SUMMARY_PAR, summaryParagraph.str());
                messageBody.str("");
                messageBody << customMessageBody;
            } else { // Otherwise create the standard email.
                messageSubjectLine << _parameters.GetTitleName(); // Create the email subject line from either title parameter or default string.
                messageSubjectLine << "SaTScan Results" << " - " << FileName(_parameters.GetOutputFileName().c_str()).getFileName();
                messageBody << summaryParagraph.rdbuf();
                // Add output summaries, if user requested it.
                if (_parameters.getEmailIncludeResultsDirectory()) messageBody << EmailText::LINEBREAK << EmailText::LINEBREAK << EmailText::LOCATION_PAR;
                messageBody << EmailText::LINEBREAK << EmailText::LINEBREAK << EmailText::FOOTER_PAR;
            }
            // Create the message in plain text and html fomrats.
            messagePlain << EmailText::getFormattedText(messageBody.str(), _parameters.GetOutputFileName(), false);
            messageHTML << EmailText::getFormattedText(messageBody.str(), _parameters.GetOutputFileName(), true);

            // Build the recipients list.
            std::vector<std::string> recipients;
            if (_parameters.getAlwaysEmailSummary())
                for (const auto& r : _parameters.getEmailAlwaysRecipientsList()) recipients.push_back(r);
            if (_parameters.getCutoffEmailSummary() && clusterMeetsCutoff)
                for (const auto& r : _parameters.getEmailCutoffRecipientsList()) recipients.push_back(r);
            // Remove any duplicate recipients.
            std::sort(recipients.begin(), recipients.end());
            recipients.erase(std::unique(recipients.begin(), recipients.end()), recipients.end());
            if (recipients.size())
                sendMail( // Send the message.
                    AppToolkit::getToolkit().mail_from, recipients, {}, AppToolkit::getToolkit().mail_reply,
                    EmailText::getFormattedText(messageSubjectLine.str(), _parameters.GetOutputFileName(), false), messagePlain, messageHTML,
                    (_parameters.getEmailAttachResults() ? _parameters.GetOutputFileName() : std::string("")),
                    AppToolkit::getToolkit().mail_servername, _print_direction, false, AppToolkit::getToolkit().mail_additional
                );
        }
    } catch (prg_exception& x) {
        if (fp) fclose(fp);
        x.addTrace("finalize()", "AnalysisExecution");
        throw;
    }
}

/** Creates/overwrites result file specified by user in parameter settings. Only header summary information is printed. File pointer does not remain open. */
void AnalysisExecution::createReport() {
    macroRunTimeStartSerial(SerialRunTimeComponent::PrintingResults);
    FILE              * fp = 0;
    std::string         sStartTime;
    std::string         sTitleName;
    AsciiPrintFormat    PrintFormat;

    try {
        openReportFile(fp, false);
        AsciiPrintFormat::PrintVersionHeader(fp);
        if (_parameters.GetTitleName() != "") {
            sTitleName = _parameters.GetTitleName().c_str();
            PrintFormat.PrintAlignedMarginsDataString(fp, sTitleName);
        }
        sStartTime = ctime(&_start_time);
        fprintf(fp, "\nProgram run on: %s\n", sStartTime.c_str());
        ParametersPrint(_parameters).PrintAnalysisSummary(fp, _data_hub.GetDataSetHandler());
        _data_hub.DisplaySummary(fp, "SUMMARY OF DATA", true);
        fclose(fp); fp = 0;
    } catch (prg_exception& x) {
        if (fp) fclose(fp);
        x.addTrace("CreateReport()", "AnalysisExecution");
        throw;
    }
    macroRunTimeStopSerial();
}

/** starts analysis execution */
void AnalysisExecution::executeCentricEvaluation() {
    unsigned long         ulParallelProcessCount = std::min(_parameters.GetNumParallelProcessesToExecute(), static_cast<unsigned>(_data_hub.m_nGridTracts));
    DataSetHandler      & DataHandler = _data_hub.GetDataSetHandler();

    try {
        do { //start analyzing data
            ++_analysis_count;
            _significant_at005 = 0;
            _sim_vars.reset(0.0);

            //simualtion data randomizer
            RandomizerContainer_t                       RandomizationContainer;
            //allocate a simulation data set for each requested replication
            std::vector<SimulationDataContainer_t>      vRandomizedDataSets(_parameters.GetNumReplicationsRequested());
            //allocate a data gateway for each requested replication
            ptr_vector<AbstractDataSetGateway>          vSimDataGateways(_parameters.GetNumReplicationsRequested());
            //allocate an array to contain simulation llr values
            AbstractCentricAnalysis::CalculatedRatioContainer_t SimulationRatios;
            //data gateway object for real data
            std::auto_ptr<AbstractDataSetGateway>        DataSetGateway(DataHandler.GetNewDataGatewayObject());
            std::auto_ptr<LoglikelihoodRatioWriter>      RatioWriter;
            std::auto_ptr<AbstractDataSetWriter>         DataSetWriter;
            std::string simulation_out;

            //get data randomizers
            DataHandler.GetRandomizerContainer(RandomizationContainer);
            //set data gateway object
            DataHandler.GetDataGateway(*DataSetGateway);
            if (_parameters.GetNumReplicationsRequested())
                _print_direction.Printf("Calculating simulation data for %u simulations\n\n", BasePrint::P_STDOUT, _parameters.GetNumReplicationsRequested());
            if (_parameters.GetOutputSimulationData()) {
                DataSetWriter.reset(AbstractDataSetWriter::getNewDataSetWriter(_parameters));
                simulation_out = getFilenameFormatTime(_parameters.GetSimulationDataOutputFilename(), _parameters.getTimestamp(), true);
                remove(simulation_out.c_str());
            }
            //create simulation data sets -- randomize each and set corresponding data gateway object
            for (unsigned int i = 0; i < _parameters.GetNumReplicationsRequested() && !_print_direction.GetIsCanceled(); ++i) {
                SimulationDataContainer_t& thisDataCollection = vRandomizedDataSets[i];
                //create new simulation data set object for each data set of this simulation
                for (unsigned int j = 0; j < DataHandler.GetNumDataSets(); ++j)
                    thisDataCollection.push_back(new DataSet(_data_hub.GetNumTimeIntervals(), _data_hub.GetNumIdentifiers(), _data_hub.GetNumMetaIdentifiersReferenced(), _parameters, j + 1));
                //allocate appropriate data structure for given data set handler (probablility model)
                DataHandler.AllocateSimulationData(thisDataCollection);
                //randomize data
                macroRunTimeStartSerial(SerialRunTimeComponent::RandomDataGeneration);
                _data_hub.RandomizeData(RandomizationContainer, thisDataCollection, i + 1);
                macroRunTimeStopSerial();
                //print simulation data to file, if requested
                if (_parameters.GetOutputSimulationData()) {
                    for (size_t t = 0; t < thisDataCollection.size(); ++t)
                        DataSetWriter->write(*thisDataCollection[t], _parameters, simulation_out);
                }
                //allocate and set data gateway object
                vSimDataGateways[i] = DataHandler.GetNewDataGatewayObject();
                DataHandler.GetSimulationDataGateway(*vSimDataGateways[i], thisDataCollection, RandomizationContainer);
            }
            //detect user cancellation
            if (_print_direction.GetIsCanceled()) return;

            //construct centric-analyses and centroid calculators for each thread:
            std::deque<boost::shared_ptr<AbstractCentricAnalysis> > seqCentricAnalyses(ulParallelProcessCount);
            std::deque<boost::shared_ptr<CentroidNeighborCalculator> > seqCentroidCalculators(ulParallelProcessCount);
            for (unsigned u = 0; u<ulParallelProcessCount; ++u) {
                seqCentricAnalyses[u].reset(getNewCentricAnalysisObject(*DataSetGateway, vSimDataGateways));
                seqCentroidCalculators[u].reset(new CentroidNeighborCalculator(_data_hub, _print_direction));
            }

            //analyze real and simulation data about each centroid
            {
                stsCentricAlgoJobSource::result_type purelyTemporalExecutionExceptionStatus;//if (.first) then (.second) is the exception message and callpath.
                PrintQueue tmpPrintDirection(_print_direction, _parameters.GetSuppressingWarnings());
                AsynchronouslyAccessible<PrintQueue> tmpThreadsafePrintDirection(tmpPrintDirection);
                stsCentricAlgoJobSource jobSource(_data_hub.m_nGridTracts, ::GetCurrentTime_HighResolution(), tmpThreadsafePrintDirection);
                typedef contractor<stsCentricAlgoJobSource> contractor_type;
                contractor_type theContractor(jobSource);
                //run threads:
                boost::thread_group tg;
                boost::mutex        thread_mutex;
                unsigned uThreadIdx = 0;
                if (_parameters.GetIncludePurelyTemporalClusters()) {
                    //launch specialized first thread:
                    stsPurelyTemporal_Plus_CentricAlgoThreadFunctor Functor(theContractor, jobSource, purelyTemporalExecutionExceptionStatus,
                        tmpThreadsafePrintDirection, *(seqCentricAnalyses[uThreadIdx]),
                        *(seqCentroidCalculators[uThreadIdx]), *DataSetGateway, vSimDataGateways);
                    tg.create_thread(Functor);
                    ++uThreadIdx;
                }
                //launch the remaining threads:
                for (; uThreadIdx < ulParallelProcessCount; ++uThreadIdx) {
                    stsCentricAlgoFunctor mcsf(*(seqCentricAnalyses[uThreadIdx]), *(seqCentroidCalculators[uThreadIdx]), *DataSetGateway, vSimDataGateways);
                    tg.create_thread(subcontractor<contractor_type, stsCentricAlgoFunctor>(theContractor, mcsf));
                }
                tg.join_all();

                // Since we are evaluating real and simulation data simultaneuosly, there is no early termination option. 
                _sim_vars.set_sim_count_explicit(_parameters.GetNumReplicationsRequested());

                //propagate exceptions if needed:
                if (_parameters.GetIncludePurelyTemporalClusters() && purelyTemporalExecutionExceptionStatus.bExceptional) {
                    if (purelyTemporalExecutionExceptionStatus.eException_type == stsCentricAlgoJobSource::result_type::memory)
                        throw memory_exception(purelyTemporalExecutionExceptionStatus.Exception.what());
                    throw purelyTemporalExecutionExceptionStatus.Exception;
                }
                jobSource.Assert_NoExceptionsCaught();
                if (jobSource.GetUnregisteredJobCount() > 0)
                    throw prg_error("At least %d jobs remain uncompleted.", "AnalysisExecution", jobSource.GetUnregisteredJobCount());
            }
            if (_print_direction.GetIsCanceled()) return;
            //retrieve top clusters and simulated loglikelihood ratios from analysis object
            for (unsigned u = 0; u<ulParallelProcessCount; ++u) {
                seqCentricAnalyses[u]->RetrieveClusters(_top_clusters_containers);
                seqCentricAnalyses[u]->RetrieveLoglikelihoodRatios(SimulationRatios);
            }
            //free memory of objects that will no longer be used
            // - we might need the memory for recalculating neighbors in geographical overlap code
            vRandomizedDataSets.clear();
            seqCentricAnalyses.clear();
            seqCentroidCalculators.clear();
            vSimDataGateways.killAll();
            //detect user cancellation
            if (_print_direction.GetIsCanceled())
                return;
            //rank top clusters and apply criteria for reporting secondary clusters
            rankClusterCollections(_top_clusters_containers, _reportClusters, &_clusterRanker, _print_direction);
            // Note: If we're not reporting hierarchical, then this might report a cluster that is not displayed in final output.
            //       We can't perform index based ordering here since we need to perform simulations first ... correct?
            printTopClusterLogLikelihood(getLargestMaximaClusterCollection());
            //report calculated simulation llr values
            if (SimulationRatios) {
                if (getIsCalculatingSignificantRatios()) _significant_ratios->initialize();
                RatioWriter.reset(new LoglikelihoodRatioWriter(_parameters, _analysis_count > 1, false));
                std::vector<double>::const_iterator itr = SimulationRatios->begin(), itr_end = SimulationRatios->end();
                for (; itr != itr_end; ++itr) {
                    //update most likely clusters given latest simulated loglikelihood ratio
                    _clusterRanker.update(*itr);
                    //update significance indicator
                    updateSignificantRatiosList(*itr);
                    //update power calculations
                    //update simulation variables
                    _sim_vars.add_llr(*itr);
                    //update simulated loglikelihood record buffer
                    if (RatioWriter.get()) RatioWriter->Write(*itr);
                }
                SimulationRatios.reset();
            }
            //report clusters to output files
            reportClusters();
            //log history for first analysis run
            if (_analysis_count == 1) {
                // report relative risk estimates for each location
                if (_parameters.GetOutputRelativeRisksFiles()) {
                    macroRunTimeStartSerial(SerialRunTimeComponent::PrintingResults);
                    _print_direction.Printf("Reporting relative risk estimates ...\n", BasePrint::P_STDOUT);
                    _data_hub.DisplayRelativeRisksForEachTract(*_relevance_tracker, _reportClusters);
                    macroRunTimeStopSerial();
                }
                logRunHistory();
            }
            //report additional output file: 'relative risks for each location'
            if (_print_direction.GetIsCanceled()) return;
        } while (repeatAnalysis() == true); //repeat analysis - iterative scan
                                            // Finalize cluster graph writer if it was allocated.
        if (_cluster_graph.get()) _cluster_graph->finalize();
        // Finalize kml writer if it was allocated.
        if (_cluster_kml.get()) _cluster_kml->finalize();
        // Finalize google writer if it was allocated.
        if (_cluster_map.get()) _cluster_map->finalize();
    }
    catch (prg_exception& x) {
        x.addTrace("executeCentricEvaluation()", "AnalysisExecution");
        throw;
    }
}

/* perform power evaluations */
void AnalysisExecution::executePowerEvaluations() {
    try {
        // If performing drilldown, store current simulation variables - since the power evaluation modifies that structure.
        std::auto_ptr<SimulationVariables> storeSimVars;
        if (_parameters.getPerformBernoulliDrilldown() || _parameters.getPerformStandardDrilldown())
            storeSimVars.reset(new SimulationVariables(_sim_vars));

        FILE * fp = 0;
        std::string buffer;
        openReportFile(fp, true);
        fprintf(fp, "\nESTIMATED POWER\n");
        _clusterRanker.sort(_data_hub); // need to sort otherwise simulation process of ranking clusters will fail
        boost::shared_ptr<RandomizerContainer_t> randomizers(new RandomizerContainer_t());
        // if simulations not already done is analysis stage, perform them now
        if (!_sim_vars.get_sim_count()) {
            // Do standard replications
            _data_hub.SetActiveNeighborReferenceType(CSaTScanData::MAXIMUM);
            _print_direction.Printf("Doing the Monte Carlo replications\n", BasePrint::P_STDOUT);
            // if writing simulation data to file, delete file now
            std::string simulation_out;
            if (_parameters.GetOutputSimulationData()) {
                simulation_out = getFilenameFormatTime(_parameters.GetSimulationDataOutputFilename(), _parameters.getTimestamp(), true);
                remove(simulation_out.c_str());
            }
            runSuccessiveSimulations(randomizers, _parameters.GetNumReplicationsRequested(), simulation_out, false, 1);
        }
        else
            AsciiPrintFormat::PrintSectionSeparatorString(fp, 0, 1);
        double critical05, critical01, critical001;
        switch (_parameters.getPowerEvaluationCriticalValueType()) {
        case CV_MONTECARLO:
            critical05 = _significant_ratios->getAlpha05().second;
            critical01 = _significant_ratios->getAlpha01().second;
            critical001 = _significant_ratios->getAlpha001().second;
            break;
        case CV_GUMBEL:
            critical05 = calculateGumbelCriticalValue(_sim_vars, 0.05).first;
            critical01 = calculateGumbelCriticalValue(_sim_vars, 0.01).first;
            critical001 = calculateGumbelCriticalValue(_sim_vars, 0.001).first;
            break;
        case CV_POWER_VALUES:
            critical05 = _parameters.getPowerEvaluationCriticalValue05();
            critical01 = _parameters.getPowerEvaluationCriticalValue01();
            critical001 = _parameters.getPowerEvaluationCriticalValue001();
            break;
        default: throw prg_error("Unknown type '%d'.", "executePowerEvaluations()", _parameters.getPowerEvaluationCriticalValueType());
        };
        // if power estimation is monte carlo, then set sim vars to track those LLRs
        if (_parameters.getPowerEstimationType() == PE_MONTECARLO) {
            _sim_vars.reset(critical05);
            _sim_vars.add_additional_mlc(critical01);
            _sim_vars.add_additional_mlc(critical001);
        }
        if (_print_direction.GetIsCanceled()) return;

        // Do power replications
        SaTScanDataReader::RiskAdjustmentsContainer_t riskAdjustments;
        size_t number_randomizations = 0;
        switch (_parameters.GetPowerEvaluationSimulationType()) {
        case STANDARD: {
            // read power evaluations adjustments
            SaTScanDataReader reader(_data_hub);
            if (!reader.ReadAdjustmentsByRelativeRisksFile(_parameters.getPowerEvaluationAltHypothesisFilename(), riskAdjustments, false))
                throw resolvable_error("There were problems reading the hypothesis alternative file.", "ExecutePowerEvaluations()");
            if (!riskAdjustments.size())
                throw resolvable_error("Power evaluations can not be performed. No adjustments found in the hypothesis alternative file.", "executePowerEvaluations()");
            number_randomizations = riskAdjustments.size();
        } break;
        case FILESOURCE: {
            // determine the number of randomization iterations by counting lines in source file
            std::ifstream inFile(getFilenameFormatTime(_parameters.getPowerEvaluationSimulationDataSourceFilename(), _parameters.getTimestamp(), true).c_str());
            size_t count = std::count(std::istreambuf_iterator<char>(inFile), std::istreambuf_iterator<char>(), '\n');
            number_randomizations = count / _parameters.getNumPowerEvalReplicaPowerStep();
        } break;
        default: throw prg_error("Unknown power evalaution simulation type '%d'.", "executePowerEvaluations()", _parameters.GetPowerEvaluationSimulationType());
        }

        std::string simulation_out;
        if (_parameters.getOutputPowerEvaluationSimulationData()) {
            simulation_out = getFilenameFormatTime(_parameters.getPowerEvaluationSimulationDataOutputFilename(), _parameters.getTimestamp(), true);
            remove(simulation_out.c_str());
        }
        if (number_randomizations == 0) {
            fprintf(fp, "\nNo alternative hypothesis sets found in source files.\n");
        }
        else {
            // report critical values gathered from simulations and/or user specified
            AsciiPrintFormat::printPadRight(fp, "\nAlpha", 25);
            AsciiPrintFormat::printPadRight(fp, "0.05", 20);
            AsciiPrintFormat::printPadRight(fp, "0.01", 20);
            AsciiPrintFormat::printPadRight(fp, "0.001", 20);
            AsciiPrintFormat::printPadRight(fp, "\n-----", 25);
            AsciiPrintFormat::printPadRight(fp, "----", 20);
            AsciiPrintFormat::printPadRight(fp, "----", 20);
            AsciiPrintFormat::printPadRight(fp, "-----", 20);
        }
        SimulationVariables simVarsCopy(_sim_vars);
        for (size_t t = 0; t < number_randomizations; ++t) {
            ++_analysis_count;
            // create adjusted randomizers
            randomizers->killAll();
            randomizers->resize(_data_hub.GetNumDataSets(), 0);
            switch (_parameters.GetPowerEvaluationSimulationType()) {
            case STANDARD:
                randomizers->at(0) = new AlternateHypothesisRandomizer(_data_hub, riskAdjustments.at(t), _parameters.GetRandomizationSeed());
                break;
            case FILESOURCE: {
                std::auto_ptr<FileSourceRandomizer> randomizer(new FileSourceRandomizer(_parameters, getFilenameFormatTime(_parameters.getPowerEvaluationSimulationDataSourceFilename(), _parameters.getTimestamp(), true), _parameters.GetRandomizationSeed()));
                // set file line offset for the current iteration of power step
                randomizer->setLineOffset(t * _parameters.getNumPowerEvalReplicaPowerStep());
                randomizers->at(0) = randomizer.release();
            } break;
            default: throw prg_error("Unknown power evalaution simulation type '%d'.", "executePowerEvaluations()", _parameters.GetPowerEvaluationSimulationType());
            }
            // create more if needed
            for (size_t r = 1; r < _data_hub.GetNumDataSets(); ++r)
                randomizers->at(r) = randomizers->at(0)->Clone();
            _print_direction.Printf("\nDoing the alternative replications for power set %d\n", BasePrint::P_STDOUT, t + 1);
            runSuccessiveSimulations(randomizers, _parameters.getNumPowerEvalReplicaPowerStep(), simulation_out, true, t + 1);

            double power05, power01, power001;
            switch (_parameters.getPowerEstimationType()) {
            case PE_MONTECARLO:
                power05 = static_cast<double>(_sim_vars.get_llr_counters().at(0).second) / static_cast<double>(_parameters.getNumPowerEvalReplicaPowerStep());
                power01 = static_cast<double>(_sim_vars.get_llr_counters().at(1).second) / static_cast<double>(_parameters.getNumPowerEvalReplicaPowerStep());
                power001 = static_cast<double>(_sim_vars.get_llr_counters().at(2).second) / static_cast<double>(_parameters.getNumPowerEvalReplicaPowerStep());
                break;
            case PE_GUMBEL:
                power05 = calculateGumbelPValue(_sim_vars, critical05).first;
                power01 = calculateGumbelPValue(_sim_vars, critical01).first;
                power001 = calculateGumbelPValue(_sim_vars, critical001).first;
                break;
            default: throw prg_error("Unknown type '%d'.", "executePowerEvaluations()", _parameters.getPowerEstimationType());
            }
            AsciiPrintFormat::printPadRight(fp, printString(buffer, "\nAlternative #%d", t + 1).c_str(), 25);
            AsciiPrintFormat::printPadRight(fp, getRoundAsString(power05, buffer, 3).c_str(), 20);
            AsciiPrintFormat::printPadRight(fp, getRoundAsString(power01, buffer, 3).c_str(), 20);
            AsciiPrintFormat::printPadRight(fp, getRoundAsString(power001, buffer, 3).c_str(), 20);

            // reset simualtion variables for next power estimation interation
            _sim_vars = simVarsCopy;
        }
        fprintf(fp, "\n");
        fclose(fp); fp = 0;
        if (storeSimVars.get()) _sim_vars = *storeSimVars.get();
    } catch (prg_exception& x) {
        x.addTrace("executePowerEvaluations()", "AnalysisExecution");
        throw;
    }
}


/** starts analysis execution - evaluating real data then replications */
void AnalysisExecution::executeSuccessively() {
    try {
        do { //start analyzing data
            ++_analysis_count;
            _significant_at005 = 0;
            //calculate most likely clusters
            macroRunTimeStartSerial(SerialRunTimeComponent::RealDataAnalysis);
            calculateMostLikelyClusters();
            macroRunTimeStopSerial();
            //detect user cancellation
            if (_print_direction.GetIsCanceled()) return;
            _sim_vars.reset(_parameters.GetNumReplicationsRequested() > 0 && getLargestMaximaClusterCollection().GetNumClustersRetained() > 0 ? getLargestMaximaClusterCollection().GetTopRankedCluster().GetRatio() : 0.0);

            //Do Monte Carlo replications.
            if (getLargestMaximaClusterCollection().GetNumClustersRetained())
                executeSuccessiveSimulations();

            //detect user cancellation
            if (_print_direction.GetIsCanceled()) return;
            // report clusters to output files
            reportClusters();

            //log history for first analysis run
            if (_analysis_count == 1) {
                // report relative risk estimates for each location
                if (_parameters.GetOutputRelativeRisksFiles()) {
                    macroRunTimeStartSerial(SerialRunTimeComponent::PrintingResults);
                    _print_direction.Printf("Reporting relative risk estimates ...\n", BasePrint::P_STDOUT);
                    _data_hub.DisplayRelativeRisksForEachTract(*_relevance_tracker, _reportClusters);
                    macroRunTimeStopSerial();
                }
                logRunHistory();
            }
            if (_print_direction.GetIsCanceled()) return;
        } while (repeatAnalysis()); //repeat analysis - iterative scan
                                    // Finalize cluster graph writer if it was allocated.
        if (_cluster_graph.get()) _cluster_graph->finalize();
        // Finalize kml writer if it was allocated.
        if (_cluster_kml.get()) _cluster_kml->finalize();
        // Finalize google writer if it was allocated.
        if (_cluster_map.get()) _cluster_map->finalize();
    } catch (prg_exception& x) {
        x.addTrace("executeSuccessively()", "AnalysisExecution");
        throw;
    }
}

/** Prepares data for simulations and contracts simulation process. */
void AnalysisExecution::executeSuccessiveSimulations() {
    try {
        if (_parameters.GetNumReplicationsRequested() > 0) {
            //recompute neighbors if settings indicate that smaller clusters are reported
            _data_hub.SetActiveNeighborReferenceType(CSaTScanData::MAXIMUM);
            _print_direction.Printf("Doing the Monte Carlo replications\n", BasePrint::P_STDOUT);
            //set/reset loglikelihood ratio significance indicator
            if (getIsCalculatingSignificantRatios())
                _significant_ratios->initialize();
            //if writing simulation data to file, delete file now
            std::string simulation_out;
            if (_parameters.GetOutputSimulationData()) {
                simulation_out = getFilenameFormatTime(_parameters.GetSimulationDataOutputFilename(), _parameters.getTimestamp(), true);
                remove(simulation_out.c_str());
            }
            boost::shared_ptr<RandomizerContainer_t> randomizers(new RandomizerContainer_t());
            runSuccessiveSimulations(randomizers, _parameters.GetNumReplicationsRequested(), simulation_out, false, _analysis_count);
        }
    }
    catch (prg_exception& x) {
        x.addTrace("ExecuteSuccessiveSimulations()", "AnalysisExecution");
        throw;
    }
}

/** Returns the most likely cluster collection associated with largest spatial maxima. */
const MostLikelyClustersContainer & AnalysisExecution::getLargestMaximaClusterCollection() const {
    if (_reportClusters.GetNumClustersRetained() > 0)
        return _reportClusters;
    else if (_top_clusters_containers.size() > 0)
        return _top_clusters_containers.back();
    else
        /* Default to the _reportClusters collection, since we must return a reference to something. */
        return _reportClusters;
}

/** returns new CAnalysis object */
CAnalysis * AnalysisExecution::getNewAnalysisObject(BasePrint& print) const {
    try {
        switch (_parameters.GetAnalysisType()) {
        case PURELYSPATIAL:
            if (_parameters.GetRiskType() == STANDARDRISK) {
                if (_parameters.GetProbabilityModelType() == HOMOGENEOUSPOISSON)
                    return new CPurelySpatialBruteForceAnalysis(_parameters, _data_hub, print);
                else
                    return new CPurelySpatialAnalysis(_parameters, _data_hub, print);
            }
            else
                return new CPSMonotoneAnalysis(_parameters, _data_hub, print);
        case PURELYTEMPORAL:
        case PROSPECTIVEPURELYTEMPORAL:
        case SEASONALTEMPORAL:
            return new CPurelyTemporalAnalysis(_parameters, _data_hub, print);
        case SPACETIME:
        case PROSPECTIVESPACETIME:
            if (_parameters.GetIncludePurelySpatialClusters() && _parameters.GetIncludePurelyTemporalClusters())
                return new C_ST_PS_PT_Analysis(_parameters, _data_hub, print);
            else if (_parameters.GetIncludePurelySpatialClusters())
                return new C_ST_PS_Analysis(_parameters, _data_hub, print);
            else if (_parameters.GetIncludePurelyTemporalClusters())
                return new C_ST_PT_Analysis(_parameters, _data_hub, print);
            else
                return new CSpaceTimeAnalysis(_parameters, _data_hub, print);
        case SPATIALVARTEMPTREND:
            return new CSpatialVarTempTrendAnalysis(_parameters, _data_hub, print);
        default:
            throw prg_error("Unknown analysis type '%d'.\n", "getNewAnalysisObject()", _parameters.GetAnalysisType());
        };
    } catch (prg_exception& x) {
        x.addTrace("getNewAnalysisObject()", "AnalysisExecution");
        throw;
    }
}

/** returns new AbstractCentricAnalysis object */
AbstractCentricAnalysis * AnalysisExecution::getNewCentricAnalysisObject(const AbstractDataSetGateway& RealDataGateway,	const ptr_vector<AbstractDataSetGateway>& vSimDataGateways) const {
    try {
        switch (_parameters.GetAnalysisType()) {
        case PURELYSPATIAL:
            if (_parameters.GetRiskType() == STANDARDRISK)
                return new PurelySpatialCentricAnalysis(_parameters, _data_hub, _print_direction, RealDataGateway, vSimDataGateways);
            else
                throw prg_error("No implementation for purely spatial analysis with isotonic scan for centric evaluation.\n", "getNewCentricAnalysisObject()");
        case PURELYTEMPORAL:
        case PROSPECTIVEPURELYTEMPORAL:
            throw prg_error("No implementation for purely temporal analysis for centric evaluation.\n", "getNewCentricAnalysisObject()");
        case SPACETIME:
        case PROSPECTIVESPACETIME:
            if (_parameters.GetIncludePurelySpatialClusters() && _parameters.GetIncludePurelyTemporalClusters())
                return new SpaceTimeIncludePureCentricAnalysis(_parameters, _data_hub, _print_direction, RealDataGateway, vSimDataGateways);
            else if (_parameters.GetIncludePurelySpatialClusters())
                return new SpaceTimeIncludePurelySpatialCentricAnalysis(_parameters, _data_hub, _print_direction, RealDataGateway, vSimDataGateways);
            else if (_parameters.GetIncludePurelyTemporalClusters())
                return new SpaceTimeIncludePurelyTemporalCentricAnalysis(_parameters, _data_hub, _print_direction, RealDataGateway, vSimDataGateways);
            else
                return new SpaceTimeCentricAnalysis(_parameters, _data_hub, _print_direction, RealDataGateway, vSimDataGateways);
        case SPATIALVARTEMPTREND:
            return new SpatialVarTempTrendCentricAnalysis(_parameters, _data_hub, _print_direction, RealDataGateway, vSimDataGateways);
        default:
            throw prg_error("Unknown analysis type '%d'.\n", "getNewCentricAnalysisObject()", _parameters.GetAnalysisType());
        };
    } catch (prg_exception& x) {
        x.addTrace("getNewCentricAnalysisObject()", "AnalysisExecution");
        throw;
    }
}

/* Calculates the optimal cluster collection, limiting GINI coefficient by passed cutoff value. When no optimal collection is found,
returns the collection with the largest maxima and have any clusters. Otherwise returns null. */
const MostLikelyClustersContainer * AnalysisExecution::getOptimalGiniContainerByPValue(const MLC_Collections_t& mlc_collections, double p_value_cutoff) const {
    // iterate through cluster collections, finding the collection with the greatest GINI coeffiecent
    const MostLikelyClustersContainer* maximizedCollection = &(mlc_collections.front());
    double maximizedGINI = mlc_collections.front().getGiniCoefficient(_data_hub, _sim_vars, p_value_cutoff);
    for (MLC_Collections_t::const_iterator itrMLC = mlc_collections.begin() + 1; itrMLC != mlc_collections.end(); ++itrMLC) {
        double thisGini = itrMLC->getGiniCoefficient(_data_hub, _sim_vars, p_value_cutoff);
        if (maximizedGINI < thisGini) {
            maximizedCollection = &(*itrMLC);
            maximizedGINI = thisGini;
        }
    }

    // combine clusters from maximized GINI collection with reporting collection
    if (maximizedGINI > 0.0)
        return maximizedCollection;
    else { //if (_reportClusters.GetNumClustersRetained() == 0) {
           /* When reporting only Gini coefficients (optimal only) when no significant gini collection found,
           then report cluster collection from largest maxima with clusters. */
        MLC_Collections_t::const_reverse_iterator rev(mlc_collections.end()), rev_end(mlc_collections.begin());
        for (; rev != rev_end; rev++) {
            if (rev->GetNumClustersRetained()) {
                return &(*rev);
            }
        }
    }
    return 0;
}

/* Returns optimal gini cluster collection -- limiting the number of clusters used in gini coefficient calculation by passed collection vector -- not p-value. */
AnalysisExecution::OptimalGiniByLimit_t AnalysisExecution::getOptimalGiniContainerByLimit(const MLC_Collections_t& mlc_collections, const std::vector<unsigned int>& atmost) const {
    assert(atmost.size() == mlc_collections.size());
    OptimalGiniByLimit_t maximized(&(mlc_collections.front()), atmost.front());
    //MostLikelyClustersContainer* maximizedCollection =  &(mlc_collections.front());
    double maximizedGINI = mlc_collections.front().getGiniCoefficient(_data_hub, _sim_vars, boost::optional<double>(), atmost.front());
    MLC_Collections_t::const_iterator itrMLC = mlc_collections.begin() + 1;
    std::vector<unsigned int>::const_iterator itrMost = atmost.begin() + 1;
    // iterate through cluster collections, finding the collection with the greatest GINI coeffiecent
    for (; itrMLC != mlc_collections.end(); ++itrMLC, ++itrMost) {
        double thisGini = itrMLC->getGiniCoefficient(_data_hub, _sim_vars, boost::optional<double>(), *itrMost);
        if (maximizedGINI < thisGini) {
            maximized.first = &(*itrMLC);
            maximized.second = *itrMost;
            maximizedGINI = thisGini;
        }
    }
    // combine clusters from maximized GINI collection with reporting collection
    if (maximizedGINI > 0.0)
        return maximized;
    else { //if (_reportClusters.GetNumClustersRetained() == 0) {
           /* When reporting only Gini coefficients (optimal only) when no significant gini collection found,
           then report cluster collection from largest maxima with clusters. */
        MLC_Collections_t::const_reverse_iterator rev(mlc_collections.end()), rev_end(mlc_collections.begin());
        std::vector<unsigned int>::const_reverse_iterator itrMost(atmost.end());
        for (; rev != rev_end; rev++, ++itrMost) {
            if (rev->GetNumClustersRetained()) {
                return std::make_pair(&(*rev), *itrMost);
            }
        }
    }
    return std::make_pair((MostLikelyClustersContainer*)0, 0U);
}


/** Logs run to history file. */
void  AnalysisExecution::logRunHistory() {
    try {
        if (_parameters.GetIsLoggingHistory()) {
            _print_direction.Printf("Logging run history ...\n", BasePrint::P_STDOUT);
            macroRunTimeStartSerial(SerialRunTimeComponent::PrintingResults);
            stsRunHistoryFile(_parameters, _print_direction).LogNewHistory(*this);
            macroRunTimeStopSerial();
        }
    } catch (...) {
        // If fails for any reason, notify user and continue ...
        _print_direction.Printf("Notice: Logging run history failed.\n", BasePrint::P_NOTICE);
    }
}

/** Attempts to open result output file stream and assign to passed file pointer address. Open mode is determined to boolean paramter. */
void AnalysisExecution::openReportFile(FILE*& fp, bool bOpenAppend) {
    try {
        if ((fp = fopen(_parameters.GetOutputFileName().c_str(), (bOpenAppend ? "a" : "w"))) == NULL) {
            if (!bOpenAppend)
                throw resolvable_error("Error: Results file '%s' could not be created.\n", _parameters.GetOutputFileName().c_str());
            else if (bOpenAppend)
                throw resolvable_error("Error: Results file '%s' could not be opened.\n", _parameters.GetOutputFileName().c_str());
        }
    } catch (prg_exception& x) {
        x.addTrace("openReportFile()", "AnalysisExecution");
        throw;
    }
}

/** Displays progress information to print direction indicating that analysis
is calculating the most likely clusters in data. If iterative scan option
was requested, the message printed reflects which iteration of the scan it
is performing. */
void AnalysisExecution::printFindClusterHeading() {
    if (!_parameters.GetIsIterativeScanning())
        _print_direction.Printf("Finding the most likely clusters\n", BasePrint::P_STDOUT);
    else {
        switch (_analysis_count) {
        case  1: _print_direction.Printf("Finding the most likely cluster.\n", BasePrint::P_STDOUT); break;
        case  2: _print_direction.Printf("Finding the second most likely cluster.\n", BasePrint::P_STDOUT); break;
        case  3: _print_direction.Printf("Finding the third most likely cluster.\n", BasePrint::P_STDOUT); break;
        default: _print_direction.Printf("Finding the %ith most likely cluster.\n", BasePrint::P_STDOUT, _analysis_count);
        }
    }
}

/** Prints calculated critical values to report file. */
void AnalysisExecution::printCriticalValuesStatus(FILE* fp) {
    AsciiPrintFormat      PrintFormat;
    std::string           buffer;

    // Martin is not sure that critical values should be reported if early termination occurs.
    if (_sim_vars.get_sim_count() != _parameters.GetNumReplicationsRequested()) return;

    if (_parameters.GetReportCriticalValues() && getIsCalculatingSignificantRatios() && _sim_vars.get_sim_count() >= 19) {
        PrintFormat.SetMarginsAsOverviewSection();
        AsciiPrintFormat::PrintSectionSeparatorString(fp, 0, 2);
        fprintf(fp, "CRITICAL VALUES\n\n");
        printString(buffer, "A cluster is statistically significant when its %s "
            "is greater than the critical value, which is, for significance level:",
            (_parameters.IsTestStatistic() ? "test statistic" : "log likelihood ratio"));
        PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
        bool printSelectiveGumbel = _parameters.GetPValueReportingType() == DEFAULT_PVALUE && _parameters.getCanReportGumbelInDefaultCombination();
        if (_parameters.GetPValueReportingType() == GUMBEL_PVALUE || _parameters.getIsReportingGumbelAsAddon()) {
            if (printSelectiveGumbel && (_sim_vars.get_sim_count() > 0 && _sim_vars.get_sim_count() < 99999)) {
                fprintf(fp, "\nGumbel Critical Values:\n");
            }
            if (printSelectiveGumbel && _sim_vars.get_sim_count() < 19) {
                std::pair<double, double> cv = calculateGumbelCriticalValue(_sim_vars, (double)0.05);
                fprintf(fp, "...... 0.05: %f\n", cv.first);
            }
            if (printSelectiveGumbel && _sim_vars.get_sim_count() < 99) {
                std::pair<double, double> cv = calculateGumbelCriticalValue(_sim_vars, (double)0.01);
                fprintf(fp, "...... 0.01: %f\n", cv.first);
            }
            if (printSelectiveGumbel && _sim_vars.get_sim_count() < 999) {
                std::pair<double, double> cv = calculateGumbelCriticalValue(_sim_vars, (double)0.001);
                fprintf(fp, "..... 0.001: %f\n", cv.first);
            }
            if (printSelectiveGumbel && _sim_vars.get_sim_count() < 9999) {
                std::pair<double, double> cv = calculateGumbelCriticalValue(_sim_vars, (double)0.0001);
                fprintf(fp, ".... 0.0001: %f\n", cv.first);
            }
            if (printSelectiveGumbel && _sim_vars.get_sim_count() < 99999) {
                std::pair<double, double> cv = calculateGumbelCriticalValue(_sim_vars, (double)0.00001);
                fprintf(fp, "... 0.00001: %f\n", cv.first);
            }
        }
        // skip reporting standard critical values if p-value reporting is gumbel
        if (_parameters.GetPValueReportingType() != GUMBEL_PVALUE) {
            if (_sim_vars.get_sim_count() >= 19)
                fprintf(fp, "\nStandard Monte Carlo Critical Values:\n");
            if (_sim_vars.get_sim_count() >= 19) {
                SignificantRatios::alpha_t alpha05(_significant_ratios->getAlpha05());
                fprintf(fp, "...... 0.05: %f\n", alpha05.second);
            }
            if (_sim_vars.get_sim_count() >= 99) {
                SignificantRatios::alpha_t alpha01(_significant_ratios->getAlpha01());
                fprintf(fp, "...... 0.01: %f\n", alpha01.second);
            }
            if (_sim_vars.get_sim_count() >= 999) {
                SignificantRatios::alpha_t alpha001(_significant_ratios->getAlpha001());
                fprintf(fp, "..... 0.001: %f\n", alpha001.second);
            }
            if (_sim_vars.get_sim_count() >= 9999) {
                SignificantRatios::alpha_t alpha0001(_significant_ratios->getAlpha0001());
                fprintf(fp, ".... 0.0001: %f\n", alpha0001.second);
            }
            if (_sim_vars.get_sim_count() >= 99999) {
                SignificantRatios::alpha_t alpha00001(_significant_ratios->getAlpha00001());
                fprintf(fp, "... 0.00001: %f\n", alpha00001.second);
            }
        }
    }
}

/** Prints early termination status to report file. */
void AnalysisExecution::printEarlyTerminationStatus(FILE* fp) {
    bool anyClusters = false;
    for (MLC_Collections_t::const_iterator itrMLC = _top_clusters_containers.begin(); itrMLC != _top_clusters_containers.end() && !anyClusters; ++itrMLC)
        anyClusters = itrMLC->GetNumClustersRetained() > 0;
    if (anyClusters && _sim_vars.get_sim_count() < _parameters.GetNumReplicationsRequested()) {
        std::string buffer;
        AsciiPrintFormat printFormat;
        printString(buffer, "\nNOTE: The sequential Monte Carlo procedure was used to terminate the calculations after %u replications.", _sim_vars.get_sim_count());
        printFormat.PrintAlignedMarginsDataString(fp, buffer);
    }
}

/* Reports dataset that were ignored by analysis. */
void AnalysisExecution::printIgnoredDataSets(FILE* fp) {
    if (getDataHub().GetDataSetHandler().getRemovedDataSetDetails().size()) {
        std::vector<int> noCases, noControls;
        for (auto const& removed : getDataHub().GetDataSetHandler().getRemovedDataSetDetails()) {
            if (removed.get<1>()) noCases.push_back(removed.get<0>());
            if (removed.get<2>()) noControls.push_back(removed.get<0>());
        }
        std::stringstream s;
        AsciiPrintFormat printFormat;
        if (noCases.size()) {
            s << std::endl << "NOTE: The following data sets have zero cases";
            if (getDataHub().isDrilldown()) s << " in the drilldown area";
            s << ", hence they are uninformative and do not contribute to the " << (getDataHub().isDrilldown() ?  "drilldown " : "") << "analysis: " << std::endl;
            for (int i = noCases.size() - 1; i >= 0; --i) {
                s << _parameters.getDataSourceNames()[noCases[i]].c_str() << (i == 0 ? "" : ", ");
            }
            printFormat.PrintAlignedMarginsDataString(fp, s.str());
        }
        if (noControls.size()) {
            s.str("");
            s << (noCases.size() ? "" : "\n") << "NOTE: The following data sets have zero controls";
            if (getDataHub().isDrilldown()) s << " in the drilldown area";
            s << ", hence they are uninformative and do not contribute to the " << (getDataHub().isDrilldown() ?  "drilldown " : "") << "analysis: " << std::endl;
            for (int i = noControls.size() - 1; i >= 0; --i) {
                s << _parameters.getDataSourceNames()[noControls[i]].c_str() << (i == 0 ? "" : ", ");
            }
            printFormat.PrintAlignedMarginsDataString(fp, s.str());
        }
    }
}

/** Print GINI coefficients */
void AnalysisExecution::printGiniCoefficients(FILE* fp) {
    if (!(_parameters.getReportGiniOptimizedClusters() && _parameters.getReportGiniIndexCoefficents())) return;
    AsciiPrintFormat printFormat;
    std::string buffer;
    printFormat.SetMarginsAsClusterSection(0);
    printFormat.PrintSectionSeparatorString(fp, 0, 2);
    printString(buffer, "Gini Indexes");
    printFormat.PrintNonRightMarginedDataString(fp, buffer, false);
    printString(buffer, "----------------------------------------------------------");
    printFormat.PrintNonRightMarginedDataString(fp, buffer, false);
    double minSize = std::numeric_limits<double>::max(), maxSize = 0, minGINI = std::numeric_limits<double>::max(), maxGINI = 0;
    const MostLikelyClustersContainer* maximizedCollection = 0;
    for (MLC_Collections_t::const_iterator itrMLC = _top_clusters_containers.begin(); itrMLC != _top_clusters_containers.end(); ++itrMLC) {
        printString(buffer, "%g percent", itrMLC->getMaximumWindowSize());
        printFormat.PrintSectionLabel(fp, buffer.c_str(), false, false);
        double gini = itrMLC->getGiniCoefficient(_data_hub, _sim_vars, _parameters.getGiniIndexPValueCutoff());
        printFormat.PrintAlignedMarginsDataString(fp, getValueAsString(gini, buffer, 4));
        if (gini > maxGINI) { maximizedCollection = &(*itrMLC); maxGINI = gini; }
        minSize = std::min(minSize, itrMLC->getMaximumWindowSize());
        maxSize = std::max(maxSize, itrMLC->getMaximumWindowSize());
        minGINI = std::min(minGINI, gini);
    }
    if (maximizedCollection) {
        printString(buffer, "Optimal Gini coefficient found at %g%% maxima.", maximizedCollection->getMaximumWindowSize());
        printFormat.PrintNonRightMarginedDataString(fp, buffer, false);
    }
    if (_parameters.GetNumReplicationsRequested() >= MIN_SIMULATION_RPT_PVALUE) {
        std::string buffer2;
        printString(buffer, "Coefficients based on clusters with p<%s.", getValueAsString(_parameters.getGiniIndexPValueCutoff(), buffer).c_str());
        printFormat.PrintNonRightMarginedDataString(fp, buffer, false);
    }

    // create gini html chart file
    GiniChartGenerator giniGenerator(_top_clusters_containers, _data_hub, _sim_vars);
    giniGenerator.generateChart();
}


/** Prints indication of whether no clusters were retained nor reported. */
void AnalysisExecution::printRetainedClustersStatus(FILE* fp, bool bClusterReported) {
    AsciiPrintFormat    PrintFormat;
    std::string         buffer;

    PrintFormat.SetMarginsAsOverviewSection();
    //if zero clusters retained in real data, then no clusters of significance were retained.

    bool anyClusters = false;
    for (MLC_Collections_t::const_iterator itrMLC = _top_clusters_containers.begin(); itrMLC != _top_clusters_containers.end() && !anyClusters; ++itrMLC)
        anyClusters = itrMLC->GetNumClustersRetained() > 0;
    if (!anyClusters) {
        if (_parameters.GetIsIterativeScanning() && _analysis_count > 1)
            fprintf(fp, "\nNo further clusters were found.\n");
        else
            fprintf(fp, "\nNo clusters were found.\n");
        switch (_parameters.GetProbabilityModelType()) {
        case POISSON:
            if (_parameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
                switch (_parameters.GetAreaScanRateType()) {
                case HIGH: buffer = "All potential cluster areas scanned had either only one case or a lower trend inside than outside the area."; break;
                case LOW: buffer = "All potential cluster areas scanned had either only one case or a higher trend inside than outside the area."; break;
                case HIGHANDLOW: buffer = "All potential cluster areas scanned had either only one case or the same time trend inside and outside the area."; break;
                default: throw prg_error("Unknown area scan rate type '%d'.\n", "printRetainedClustersStatus()", _parameters.GetAreaScanRateType());
                }
                break;
            }
        case BATCHED:
        case RANK:
        case BERNOULLI:
        case SPACETIMEPERMUTATION:
        case HOMOGENEOUSPOISSON:
        case UNIFORMTIME:
            switch (_parameters.GetAreaScanRateType()) {
            case HIGH: buffer = "All potential cluster areas scanned had either only one case or fewer observed cases than expected."; break;
            case LOW: buffer = "All potential cluster areas scanned had either only one case or more observed cases than expected."; break;
            case HIGHANDLOW: buffer = "All potential cluster areas scanned had either only one case or an equal number of observed and expected cases."; break;
            default: throw prg_error("Unknown area scan rate type '%d'.\n", "printRetainedClustersStatus()", _parameters.GetAreaScanRateType());
            }
            break;
        case CATEGORICAL:
            buffer = "All areas scanned had either only one case or an equal number of low or high value cases to expected for any cut-off."; break;
            break;
        case ORDINAL:
            switch (_parameters.GetAreaScanRateType()) {
            case HIGH: buffer = "All areas scanned had either only one case or an equal or lower number of high value cases than expected for any cut-off."; break;
            case LOW: buffer = "All areas scanned had either only one case or an equal or higher number of low value cases than expected for any cut-off."; break;
            case HIGHANDLOW: buffer = "All areas scanned had either only one case or an equal number of low or high value cases to expected for any cut-off."; break;
            default: throw prg_error("Unknown area scan rate type '%d'.\n", "printRetainedClustersStatus()", _parameters.GetAreaScanRateType());
            }
            break;
        case NORMAL:
            switch (_parameters.GetAreaScanRateType()) {
            case HIGH: buffer = "All areas scanned had either only one case or an equal or lower mean than outside the area."; break;
            case LOW: buffer = "All areas scanned had either only one case or an equal or higher mean than outside the area."; break;
            case HIGHANDLOW: buffer = "All areas scanned had either only one case or an equal mean to outside the area."; break;
            default: throw prg_error("Unknown area scan rate type '%d'.\n", "printRetainedClustersStatus()", _parameters.GetAreaScanRateType());
            }
            break;
        case EXPONENTIAL:
            switch (_parameters.GetAreaScanRateType()) {
            case HIGH: buffer = "All areas scanned had either only one case or equal or longer survival than outside the area."; break;
            case LOW: buffer = "All areas scanned had either only one case or equal or shorter survival than outside the area."; break;
            case HIGHANDLOW: buffer = "All areas scanned had either only one case or equal survival to outside the area."; break;
            default: throw prg_error("Unknown area scan rate type '%d'.\n", "printRetainedClustersStatus()", _parameters.GetAreaScanRateType());
            }
            break;
        default: throw prg_error("Unknown probability model '%d'.", "printRetainedClustersStatus()", _parameters.GetProbabilityModelType());
        }
        PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
    }
    else if (!bClusterReported) {
        fprintf(fp, "\nNo clusters reported.\n");
        anyClusters = false;
        for (MLC_Collections_t::const_iterator itrMLC = _top_clusters_containers.begin(); itrMLC != _top_clusters_containers.end() && !anyClusters; ++itrMLC)
            anyClusters = itrMLC->GetNumClustersRetained() && itrMLC->GetTopRankedCluster().GetRatio() < MIN_CLUSTER_LLR_REPORT;
        if (!anyClusters)
            printString(buffer, "All clusters had a %s less than %g.", (_parameters.IsTestStatistic() ? "test statistic" : "log likelihood ratio"), MIN_CLUSTER_LLR_REPORT);
        else
            printString(buffer, "All clusters had a rank greater than %i.", _parameters.GetNumReplicationsRequested());
        PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
    }
}

/** Displays most likely clusters loglikelihood ratio(test statistic) to print
direction. If no clusters were retained, indicating message is printed. */
void AnalysisExecution::printTopClusterLogLikelihood(const MostLikelyClustersContainer& mlc) {
    //if any clusters were retained, display either loglikelihood or test statistic
    if (mlc.GetNumClustersRetained())
        _print_direction.Printf("SaTScan %s for the most likely cluster: %7.2lf\n", BasePrint::P_STDOUT,
            _parameters.IsTestStatistic() ? "test statistic" : "log likelihood ratio",
            mlc.GetTopRankedCluster().m_nRatio);
    else
        _print_direction.Printf("No clusters retained.\n", BasePrint::P_STDOUT);
}

/** Calculates simulated loglikelihood ratios and updates:
- most likely clusters rank
- significant loglikelihood ratio indicator
- power calculation data, if requested by user
- additional output file(s)
*****************************************************
*/
void AnalysisExecution::runSuccessiveSimulations(boost::shared_ptr<RandomizerContainer_t>& randomizers, unsigned int num_relica, const std::string& writefile, bool isPowerStep, unsigned int iteration) {
    try {
        {
            PrintQueue lclPrintDirection(_print_direction, _parameters.GetSuppressingWarnings());
            stsMCSimJobSource jobSource(_parameters, ::GetCurrentTime_HighResolution(), lclPrintDirection, *this, num_relica, isPowerStep, iteration);
            typedef contractor<stsMCSimJobSource> contractor_type;
            contractor_type theContractor(jobSource);
            PrintNull nullPrint;

            //run threads:
            boost::thread_group tg;
            boost::mutex thread_mutex;
            unsigned long ulParallelProcessCount = std::min(_parameters.GetNumParallelProcessesToExecute(), num_relica);
            for (unsigned u = 0; u < ulParallelProcessCount; ++u) {
                try {
                    stsMCSimSuccessiveFunctor mcsf(thread_mutex, _data_hub, boost::shared_ptr<CAnalysis>(getNewAnalysisObject(nullPrint)), boost::shared_ptr<SimulationDataContainer_t>(new SimulationDataContainer_t()), randomizers, writefile, u == 0);
                    tg.create_thread(subcontractor<contractor_type, stsMCSimSuccessiveFunctor>(theContractor, mcsf));
                }
                catch (std::bad_alloc &b) {
                    if (u == 0) throw; // if this is the first thread, re-throw exception
                    _print_direction.Printf("Notice: Insufficient memory to create %u%s parallel simulation ... continuing analysis with %u parallel simulations.\n",
                        BasePrint::P_NOTICE, u + 1, (u == 1 ? "nd" : (u == 2 ? "rd" : "th")), u);
                    break;
                }
                catch (prg_exception& x) {
                    if (u == 0) throw; // if this is the first thread, re-throw exception
                    _print_direction.Printf("Error: Program exception occurred creating %u%s parallel simulation ... continuing analysis with %u parallel simulations.\nException:%s\n",
                        BasePrint::P_ERROR, u + 1, (u == 1 ? "nd" : (u == 2 ? "rd" : "th")), u, x.what());
                    break;
                }
                catch (...) {
                    if (u == 0) throw prg_error("Unknown program error occurred.\n", "PerformSuccessiveSimulations_Parallel()"); // if this is the first thread, throw exception
                    _print_direction.Printf("Error: Unknown program exception occurred creating %u%s parallel simulation ... continuing analysis with %u parallel simulations.\n",
                        BasePrint::P_ERROR, u + 1, (u == 1 ? "nd" : (u == 2 ? "rd" : "th")), u);
                    break;
                }
            }

            tg.join_all();

            //propagate exceptions if needed:
            theContractor.throw_unhandled_exception();
            jobSource.Assert_NoExceptionsCaught();
            if (jobSource.GetUnregisteredJobCount() > 0)
                throw prg_error("At least %d jobs remain uncompleted.", "AnalysisExecution", jobSource.GetUnregisteredJobCount());
        }
    } catch (prg_exception& x) {
        x.addTrace("runSuccessiveSimulations()", "AnalysisExecution");
        throw;
    }
}

/** Performs ranking on each collection of clusters. */
void AnalysisExecution::rankClusterCollections(MLC_Collections_t& mlc_collection, MostLikelyClustersContainer& mlc, ClusterRankHelper * ranker, BasePrint& print) const {
    mlc.Empty();
    if (ranker) ranker->clear();

    if (mlc_collection.empty()) {
        // We need to sort the ranker so the object is closed to additions.
        if (ranker) ranker->sort(_data_hub);
        return;
    }

    if (!(_parameters.getReportHierarchicalClusters() || _parameters.getReportGiniOptimizedClusters()) || _parameters.GetIsPurelyTemporalAnalysis()) {
        // Not performing hierarchical nor gini clusters, then we're only grabbing the top ranked cluster.
        mlc = mlc_collection.back();
        mlc.rankClusters(_data_hub, _parameters.GetCriteriaSecondClustersType(), print, 1);
        mlc.setClustersHierarchical();
        // don't need to add clusters to cluster ranker if not performing simulations
        if (_parameters.GetNumReplicationsRequested() && ranker) ranker->add(mlc);
    }
    else {
        // If reporting hierarchical clusters, clone the collection of clusters associated with the greatest maxima.
        // We need to maintain a copy since geographical overlap might be different than index based ranking (no overlap).
        if (_parameters.getReportHierarchicalClusters()) {
            mlc = mlc_collection.back();
            mlc.rankClusters(_data_hub, _parameters.GetCriteriaSecondClustersType(), print);
            mlc.setClustersHierarchical();
            // don't need to add clusters to cluster ranker if not performing simulations
            if (_parameters.GetNumReplicationsRequested() && ranker) ranker->add(mlc);
        }
        if (_parameters.getReportGiniOptimizedClusters()) {
            // Index based clusters always use 'No Geographical Overlap'.
            for (MLC_Collections_t::iterator itr = mlc_collection.begin(); itr != mlc_collection.end(); ++itr) {
                itr->rankClusters(_data_hub, NOGEOOVERLAP, print);
                // don't need to add clusters to cluster ranker if not performing simulations
                if (_parameters.GetNumReplicationsRequested() && ranker) ranker->add(*itr);
            }
        }
    }
    // cause the cluster ranker to sort clusters by LLR for ranking during simulations
    if (ranker) ranker->sort(_data_hub);
}

/** Set drilldown attributes of this AnalysisExecution - these are needed with iterative analyses. */
void AnalysisExecution::setDrilldownAttributes(const std::string& base_output, const std::string& cluster_path) {
    _base_output = base_output; 
    _cluster_path = cluster_path;
}


/** Returns indication of whether analysis repeats.
Indication of true is returned if user requested iterative scan option and:
- analysis type is purely spatial or monotone purely spatial
- a most likely cluster was retained
- most likely cluster's p-value is not less than user specified cutoff p- value
- after removing most likely cluster's identifiers, there are still more idenifiers
- the number of requested iterative scans has not been already reached
- last iteration of simulations did not terminate early
Indication of false is returned if user did not request iterative scan option. */
bool AnalysisExecution::repeatAnalysis() {
    //NOTE: Still in the air as to the minimum for STP model, set to 2 for now.
    count_t tMinCases = (_parameters.GetProbabilityModelType() == ORDINAL || _parameters.GetProbabilityModelType() == CATEGORICAL ? 4 : 2);

    try {
        if (!_parameters.GetIsIterativeScanning()) return false;
        if (_analysis_count >= _parameters.GetNumIterativeScansRequested()) return false;
        if (!getLargestMaximaClusterCollection().GetNumClustersRetained()) return false;
        const CCluster& cluster = getLargestMaximaClusterCollection().GetTopRankedCluster();
        //if user requested replications, validate that cluster does not exceed user defined cutoff 
        if (_parameters.GetNumReplicationsRequested()) {
            if (_parameters.GetIsProspectiveAnalysis()) {
                if (!cluster.reportableRecurrenceInterval(_parameters, _sim_vars) ||
                    std::round(cluster.GetRecurrenceInterval(_data_hub, 0, _sim_vars).second) < _parameters.GetIterativeCutOffPValue()) { // round RI to whole days
                    return false;
                }
            } else if (!cluster.reportablePValue(_parameters, _sim_vars) || cluster.getReportingPValue(_parameters, _sim_vars, true) > _parameters.GetIterativeCutOffPValue()) {
                return false;
            }
        }
        //potentially drilldown on most likely cluster of this iteration
        if (_parameters.getPerformStandardDrilldown() || _parameters.getPerformBernoulliDrilldown())
            AbstractAnalysisDrilldown::drilldownCluster(*this, cluster, 0, _drilldowns);
        //now we need to modify the data sets - removing data of locations in top cluster
        _data_hub.RemoveClusterSignificance(cluster);
        //for SVTT analyses, are data set global time trends converging?
        if (_parameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
            for (unsigned int i = 0; i < _data_hub.GetDataSetHandler().GetNumDataSets(); ++i)
                if (_data_hub.GetDataSetHandler().GetDataSet(i).getTimeTrend().GetStatus() != AbstractTimeTrend::CONVERGED)
                    return false;
        }
        //does the minimum number of cases remain in all data sets?
        unsigned int iSetWithMinimumCases = 0;
        for (unsigned int i = 0; i < _data_hub.GetDataSetHandler().GetNumDataSets(); ++i)
            if (_data_hub.GetDataSetHandler().GetDataSet(i).getTotalCases() < tMinCases) ++iSetWithMinimumCases;
        if (_data_hub.GetDataSetHandler().GetNumDataSets() == iSetWithMinimumCases)
            return false;
        //are there locations left?
        if (!_parameters.GetIsPurelyTemporalAnalysis() && ((size_t)_data_hub.GetNumIdentifiers() + _data_hub.GetNumMetaIdentifiersReferenced() - _data_hub.GetNumNullifiedIdentifiers()) < 2)
            return false;
        //is the minimum number of cases per data set remaining, as required by probability model?
        if (_parameters.GetProbabilityModelType() == ORDINAL || _parameters.GetProbabilityModelType() == CATEGORICAL) {
            int iCategoriesWithCases = 0;
            for (unsigned int i = 0; i < _data_hub.GetDataSetHandler().GetNumDataSets(); ++i) {
                const PopulationData& Population = _data_hub.GetDataSetHandler().GetDataSet(i).getPopulationData();
                for (size_t t = 0; t < Population.GetNumOrdinalCategories(); ++t)
                    if (Population.GetNumCategoryTypeCases(static_cast<int>(t)) > 0)
                        ++iCategoriesWithCases;
                if (iCategoriesWithCases < 2)
                    return false;
            }
        }
        if (!_parameters.GetIsPurelyTemporalAnalysis())
            _data_hub.AdjustNeighborCounts(_executing_type); //recalculate neighbors
        //clear top clusters container
        std::for_each(_top_clusters_containers.begin(), _top_clusters_containers.end(), [](MostLikelyClustersContainer& m) { m.Empty(); });
        _reportClusters.Empty();
        _clusterRanker.clear();
        _data_hub.clearClusterLocationsCache();
    } catch (prg_exception& x) {
        x.addTrace("repeatAnalysis()", "AnalysisExecution");
        throw;
    }
    return true;
}

/** Updates results output file.
- prints most likely cluster(s) (optionally for iterative scan)
- significant loglikelihood ratio indicator
- power calculation results, if option requested by user
- indication of when simulations terminated early */
void AnalysisExecution::reportClusters() {
    macroRunTimeStartSerial(SerialRunTimeComponent::PrintingResults);
    try {
        if (_parameters.getCalculateOliveirasF())
            calculateOliveirasF();

        _print_direction.Printf("Printing analysis results to file ...\n", BasePrint::P_STDOUT);
        // since the simulations have been completed, we can calculate the gini index and add to collections
        if (_parameters.getReportGiniOptimizedClusters())
            addGiniClusters(_top_clusters_containers, _reportClusters, _parameters.getGiniIndexPValueCutoff());

        // Process line list data if selected.
        if (_parameters.getReadingLineDataFromCasefile()) {
            _print_direction.Printf("Processing line list data and adding results to cluster line list file ...\n", BasePrint::P_STDOUT);
            _data_demographic_processor.reset(new DataDemographicsProcessor(_data_hub.GetDataSetHandler(), _reportClusters, _sim_vars));
            _data_demographic_processor->process(_analysis_count);
        }

        // report clusters accordingly
        if (_parameters.GetIsIterativeScanning()) {
            printTopIterativeScanCluster(_reportClusters);
        } else {
            printTopClusters(_reportClusters);
            // create temporal graph
            if ((_parameters.GetIsPurelyTemporalAnalysis() || _parameters.GetIsSpaceTimeAnalysis() || _parameters.GetAnalysisType() == SPATIALVARTEMPTREND) 
                && _parameters.getOutputTemporalGraphFile()) {
                _print_direction.Printf("Adding analysis results to temporal graph map ...\n", BasePrint::P_STDOUT);
                TemporalChartGenerator generator(_data_hub, _reportClusters, _sim_vars);
                generator.generateChart();
            }
        }

        // Create Cartesian graph, if requested.
        if (_parameters.getOutputCartesianGraph() && !_parameters.GetIsPurelyTemporalAnalysis() &&
            ((_parameters.GetCoordinatesType() == CARTESIAN && _data_hub.getLocationsManager().expectedDimensions() == 2) || _parameters.GetCoordinatesType() == LATLON)) {
            // If first iteration of analyses, create the ClusterKML object -- this is both with and without iterative scan.
            _print_direction.Printf("Adding analysis results to Cartesian map file ...\n", BasePrint::P_STDOUT);
            if (_analysis_count == 1) _cluster_graph.reset(new CartesianGraph(_data_hub));
            _cluster_graph->add(_reportClusters, _sim_vars, _analysis_count);
        }

        // Create Google Maps file if requested.
        if (_parameters.getOutputGoogleMapsFile()) {
            // If first iteration of analyses, create the ClusterMap object -- this is both with and without iterative scan.
            _print_direction.Printf("Adding analysis results to Google map file ...\n", BasePrint::P_STDOUT);
            if (_analysis_count == 1) _cluster_map.reset(new ClusterMap(_data_hub));
            _cluster_map->add(_reportClusters, _sim_vars, _analysis_count);
            if (_data_demographic_processor.get() && _analysis_count == 1) _cluster_map->add(*_data_demographic_processor.get());
        }

        // Create KML file if requested.
        if (_parameters.getOutputKMLFile()) {
            // If first iteration of analyses, create the ClusterKML object -- this is both with and without iterative scan.
            _print_direction.Printf("Adding analysis results to Google Earth file ...\n", BasePrint::P_STDOUT);
            if (_analysis_count == 1) _cluster_kml.reset(new ClusterKML(_data_hub));
            if (_data_demographic_processor.get() && _analysis_count == 1) _cluster_kml->add(*_data_demographic_processor);
            _cluster_kml->add(_reportClusters, _sim_vars);
        }
    } catch (prg_exception& x) {
        x.addTrace("reportClusters()", "AnalysisExecution");
        throw;
    }
    macroRunTimeStopSerial();
}

/** Prints most likely cluster information, if any retained, to result file.
If user requested 'location information' output file(s), they are created
simultaneously with reported clusters. */
void AnalysisExecution::printTopClusters(const MostLikelyClustersContainer& mlc) {
    std::auto_ptr<LocationInformationWriter> ClusterLocationWriter;
    std::auto_ptr<ClusterInformationWriter>  ClusterWriter;
    boost::posix_time::ptime StartTime = ::GetCurrentTime_HighResolution();
    FILE * fp = 0;
    _clusterSupplement.reset(new ClusterSupplementInfo());

    try {
        //if creating 'location information' files, create record data buffers
        if (_parameters.GetOutputAreaSpecificFiles())
            ClusterLocationWriter.reset(new LocationInformationWriter(_data_hub, _analysis_count > 1));
        //if creating 'cluster information' files, create record data buffers
        if (_parameters.GetOutputClusterLevelFiles() || _parameters.GetOutputClusterCaseFiles() || _parameters.getOutputShapeFiles())
            ClusterWriter.reset(new ClusterInformationWriter(_data_hub));
        //open result output file
        openReportFile(fp, true);
        // determine how many clusters are being reported and define supplement information for each
        // if no replications requested, attempt to display up to top 10 clusters
        tract_t maxDisplay = _sim_vars.get_sim_count() == 0 ? std::min(10, mlc.GetNumClustersRetained()) : mlc.GetNumClustersRetained();
        for (int i = 0; i < maxDisplay; ++i) {
            const CCluster& cluster = mlc.GetCluster(i);
            if (i == 0 || (cluster.m_nRatio >= MIN_CLUSTER_LLR_REPORT && (_sim_vars.get_sim_count() == 0 || cluster.GetRank() <= _sim_vars.get_sim_count())))
                _clusterSupplement->addCluster(cluster, i + 1);
        }
        // calculate geographical overlap of clusters
        calculateOverlappingClusters(mlc, *_clusterSupplement);
        for (size_t i = 0; i < _clusterSupplement->size(); ++i) {
            _print_direction.Printf("Reporting cluster %i of %i\n", BasePrint::P_STDOUT, i + 1, _clusterSupplement->size());
            if (i == 9) //report estimate of time to report all clusters
                ReportTimeEstimate(StartTime, _clusterSupplement->size(), i, _print_direction);
            //get reference to i'th top cluster
            const CCluster& TopCluster = mlc.GetCluster(i);
            //write cluster details to 'cluster information' file
            if (ClusterWriter.get() && TopCluster.m_nRatio >= MIN_CLUSTER_LLR_REPORT)
                ClusterWriter->Write(TopCluster, i + 1, _sim_vars);
            //write cluster details to results file and 'location information' files -- always report most likely cluster but only report
            //secondary clusters if loglikelihood ratio is greater than defined minimum and it's rank is not lower than all simulated ratios
            switch (i) {
                case 0: fprintf(fp, "\nCLUSTERS DETECTED\n\n"); break;
                //case 1  : fprintf(fp, "\nSECONDARY CLUSTERS\n\n"); break;
                default: fprintf(fp, "\n"); break;
            }
            //print cluster definition to file stream
            TopCluster.Display(fp, _data_hub, *_clusterSupplement, _sim_vars);
            //check track of whether this cluster was significant in top five percentage

            if (getIsCalculatingSignificantRatios() && macro_less_than(_significant_ratios->getAlpha05().second, TopCluster.m_nRatio, DBL_CMP_TOLERANCE))
                ++_significant_at005;
            if (TopCluster.isSignificant(_data_hub, _clusterSupplement->getClusterReportIndex(TopCluster), _sim_vars))
                ++_significant_clusters;

            //print cluster definition to 'location information' record buffer
            if (_parameters.GetOutputAreaSpecificFiles())
                ClusterLocationWriter->WriteClusterLocations(TopCluster, _data_hub, i + 1, _sim_vars, *_relevance_tracker);
            _clustersReported = true;
        }

        printRetainedClustersStatus(fp, _clustersReported);
        printCriticalValuesStatus(fp);
        printEarlyTerminationStatus(fp);
        printGiniCoefficients(fp);
        printIgnoredDataSets(fp);
        fclose(fp); fp = 0;
    }
    catch (prg_exception& x) {
        if (fp) fclose(fp);
        x.addTrace("printTopClusters()", "AnalysisExecution");
        throw;
    }
}

/** Updates list of significant ratio, if structure allocated. */
void AnalysisExecution::updateSignificantRatiosList(double dRatio) {
    if (_significant_ratios.get()) _significant_ratios->add(dRatio);
}


/** Prints most likely cluster information, if retained, to result file. This
function only prints THE most likely cluster, as part of reporting with
the iterative scan option. So printing is directed by the particular
iteration of the iterative scan.
If user requested 'location information' output file(s), they are created
simultaneously with reported clusters. */
void AnalysisExecution::printTopIterativeScanCluster(const MostLikelyClustersContainer& mlc) {
    FILE * fp = 0;
    std::string buffer;
    _clusterSupplement.reset(new ClusterSupplementInfo());

    try {
        //open result output file
        openReportFile(fp, true);
        if (mlc.GetNumClustersRetained()) {
            if (_analysis_count > 1) {
                if (_analysis_count == 2)
                    _data_hub.DisplaySummary(fp, printString(buffer, "REMAINING DATA AFTER TOP CLUSTER REMOVED"), false);
                else
                    _data_hub.DisplaySummary(fp, printString(buffer, "REMAINING DATA AFTER TOP %d CLUSTERS REMOVED", _analysis_count - 1), false);
            }

            //get most likely cluster
            const CCluster& TopCluster = mlc.GetTopRankedCluster();
            _clusterSupplement->addCluster(TopCluster, _analysis_count);
            fprintf(fp, "\nCLUSTER DETECTED\n\n");
            //print cluster definition to file stream
            TopCluster.Display(fp, _data_hub, *_clusterSupplement, _sim_vars);
            //print cluster definition to 'cluster information' record buffer
            if (_parameters.GetOutputClusterLevelFiles() || _parameters.GetOutputClusterCaseFiles())
                ClusterInformationWriter(_data_hub, _analysis_count > 1).Write(TopCluster, _analysis_count, _sim_vars);
            //print cluster definition to 'location information' record buffer
            if (_parameters.GetOutputAreaSpecificFiles())
                LocationInformationWriter(_data_hub, _analysis_count > 1).WriteClusterLocations(TopCluster, _data_hub, _analysis_count, _sim_vars, *_relevance_tracker);
            //check track of whether this cluster was significant in top five percentage
            if (getIsCalculatingSignificantRatios() && macro_less_than(_significant_ratios->getAlpha05().second, TopCluster.m_nRatio, DBL_CMP_TOLERANCE))
                ++_significant_at005;
            if (TopCluster.isSignificant(_data_hub, _clusterSupplement->getClusterReportIndex(TopCluster), _sim_vars))
                ++_significant_clusters;
            _clustersReported = true;
        }

        //if no clusters reported in this iteration but clusters were reported previuosly, print spacer
        if (!_clusterSupplement->size() && _clustersReported)
            fprintf(fp, "                  _____________________________\n\n");

        printRetainedClustersStatus(fp, _clusterSupplement->size() > 0);
        printCriticalValuesStatus(fp);
        printEarlyTerminationStatus(fp);
        printIgnoredDataSets(fp);
        fclose(fp); fp = 0;
    }
    catch (prg_exception& x) {
        if (fp) fclose(fp);
        x.addTrace("PrintTopIterativeScanCluster()", "AnalysisExecution");
        throw;
    }
}

/////////////////////////////// AbstractAnalysisDrilldown /////////////////////////////

void AbstractAnalysisDrilldown::setOutputFilename(const CCluster& detectedCluster, const ClusterSupplementInfo& supplementInfo, unsigned int& drilldowns) {
    std::string buffer;
    _cluster_path = printString(
        buffer, "%sC%u", _cluster_path.c_str(), supplementInfo.getClusterReportIndex(detectedCluster)
    );
    FileName resultsFile(_base_output.c_str());
    resultsFile.setFileName(printString(buffer, "%s-drilldown-%s-%s(run%u)", 
        resultsFile.getFileName().c_str(), _cluster_path.c_str(), getTypeIdentifier().c_str(), drilldowns).c_str()
    );
    _parameters.SetOutputFileNameSetting(resultsFile.getFullPath(buffer).c_str());
    _parameters.setClusterMonikerPrefix(_cluster_path);
}

AbstractAnalysisDrilldown::AbstractAnalysisDrilldown(
    const CParameters& source_parameters, const std::string& base_output, ExecutionType executing_type,
    BasePrint& print, unsigned int downlevel, unsigned int parent_runid,
    unsigned int& drilldowns,
    boost::optional<const std::string&> cluster_path
): _parameters(source_parameters), _print_direction(print), _executing_type(executing_type), _downlevel(downlevel), 
    _base_output(base_output), _significant_clusters(0), _drilldowns(drilldowns) {
    // Record start time of drilldown start -- of course this excludes time reading data.
    time(&_start_time);
    _cluster_path = (cluster_path ? cluster_path.get() : "");
    ++drilldowns;
    // Never email from drill down analyses.
    _parameters.setAlwaysEmailSummary(false);
    _parameters.setCutoffEmailSummary(false);
    _parameters.setCreateEmailSummaryFile(false);
}

AbstractAnalysisDrilldown::~AbstractAnalysisDrilldown() {
    // Attempt to cleanup any temporary files.
    for (std::vector<std::string>::const_iterator itr=_temp_files.begin(); itr != _temp_files.end(); ++itr) {
        try {
            remove(itr->c_str());
        } catch (...) {}
    }
}

std::string& AbstractAnalysisDrilldown::createTempFilename(const CCluster& detectedCluster, const ClusterSupplementInfo& supplementInfo,  const char * extension, std::string& filename) {
    std::stringstream temp_coordinates_filename;
    std::string temp_directory, buffer;
    temp_coordinates_filename << GetUserTemporaryDirectory(temp_directory).c_str();
    buffer = boost::filesystem::path::preferred_separator;
    temp_coordinates_filename << buffer << "drilldown-" << getTypeIdentifier() << "-" << _cluster_path;
    temp_coordinates_filename << "-" << RandomNumberGenerator(reinterpret_cast<long>(&detectedCluster)).GetRandomInteger() << extension;
    filename = temp_coordinates_filename.str();
    return filename;
}

void AbstractAnalysisDrilldown::createReducedCoodinatesFile(const CCluster& detectedCluster, const ClusterSupplementInfo& supplementInfo, const CSaTScanData& source_data_hub, unsigned int downlevel) {
    std::string buffer;
    // Drilldown analysis is always 50% of population at risk.
    _parameters.SetMaxSpatialSizeForType(PERCENTOFPOPULATION, 50.0, false);
    _parameters.SetMaxSpatialSizeForType(PERCENTOFPOPULATION, 50.0, true);
    // Relax coordinates data checking to exclude data outside new retricted geographical area.
    _parameters.SetCoordinatesDataCheckingType(RELAXEDCOORDINATES);

    /* Create a collection locations in detected cluster and re-read coordinates file - ignoring records which reference locations not in cluster. */
    std::set<std::string> clusterLocations, clusterLocations2;
	Identifier::CombinedIdentifierNames_t tract_identifiers;
    for (tract_t i = 1; i <= detectedCluster.getNumIdentifiers(); ++i) {
        tract_t tTract = source_data_hub.GetNeighbor(detectedCluster.GetEllipseOffset(), detectedCluster.GetCentroidIndex(), i, detectedCluster.GetCartesianRadius());
        source_data_hub.getIdentifierInfo().retrieveAll(tTract, tract_identifiers);
        for (unsigned int idx = 0; idx < tract_identifiers.size(); ++idx) {
            clusterLocations.insert(tract_identifiers[idx]);
        }
    }
    if (_parameters.GetMultipleCoordinatesType() != ONEPERLOCATION) {
        std::vector<tract_t> vLocations;
        CentroidNeighborCalculator::getLocationsAboutCluster(source_data_hub, detectedCluster, 0, &vLocations);
        for (auto idx : vLocations) {
            clusterLocations2.insert(source_data_hub.getLocationsManager().locations()[idx]->name());
        }
    }
    std::set<std::string> * coordinateLocations = _parameters.GetMultipleCoordinatesType() == ONEPERLOCATION ? &clusterLocations : &clusterLocations2;


    if (_parameters.UseLocationNeighborsFile()) {
        std::ofstream neighbors_file;
        neighbors_file.open(createTempFilename(detectedCluster, supplementInfo, ".nei", buffer).c_str());
        if (!neighbors_file) throw resolvable_error("Error: Could not create neighbors file '%s'.\n", buffer.c_str());
		_print_direction.SetImpliedInputFileType(BasePrint::LOCATION_NEIGHBORS_FILE);
        std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(
            getFilenameFormatTime(_parameters.GetLocationNeighborsFileName(), _parameters.getTimestamp(), true),
            _parameters.getInputSource(LOCATION_NEIGHBORS_FILE), _print_direction)
        );
        while (Source->ReadRecord()) {
            unsigned int written = 0;
            for (long idx = 0; idx < Source->GetNumValues(); ++idx) {
                std::string identifier(Source->GetValueAt(idx));
                if (std::find(coordinateLocations->begin(), coordinateLocations->end(), identifier) != coordinateLocations->end()) {
                    neighbors_file << (written == 0 ? "" : ",") << identifier;
                    ++written;
                }
            }
            if (written) neighbors_file << std::endl;
        }
        CParameters::InputSource source(CSV, ",", "\"", 0, false);
        _parameters.defineInputSource(LOCATION_NEIGHBORS_FILE, source);
        _parameters.SetLocationNeighborsFileName(buffer.c_str());
        neighbors_file.close();
        _temp_files.push_back(buffer);
    } else if (_parameters.GetCoordinatesFileName().size()) {
        std::ofstream coordinates_file;
        coordinates_file.open(createTempFilename(detectedCluster, supplementInfo, ".geo", buffer).c_str());
        if (!coordinates_file) throw resolvable_error("Error: Could not create coordinates file '%s'.\n", buffer.c_str());
		_print_direction.SetImpliedInputFileType(BasePrint::COORDFILE);
		std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(
            getFilenameFormatTime(_parameters.GetCoordinatesFileName(), _parameters.getTimestamp(), true),
            _parameters.getInputSource(COORDFILE), _print_direction)
        );
        while (Source->ReadRecord()) {
            std::string identifier(Source->GetValueAt(0));
            if (std::find(coordinateLocations->begin(), coordinateLocations->end(), identifier) != coordinateLocations->end()) {
                coordinates_file << identifier;
                for (long idx = 1; idx < Source->GetNumValues(); ++idx)
                    coordinates_file << "," << Source->GetValueAt(idx);
                coordinates_file << std::endl;
            }
        }
        CParameters::InputSource source(CSV, ",", "\"", 0, false);
        _parameters.defineInputSource(COORDFILE, source);
        _parameters.SetCoordinatesFileName(buffer.c_str());
        coordinates_file.close();
        _temp_files.push_back(buffer);
    }
    if (_parameters.getUseLocationsNetworkFile()) {
        std::ofstream network_file;
        network_file.open(createTempFilename(detectedCluster, supplementInfo, ".ntk", buffer).c_str());
        if (!network_file) throw resolvable_error("Error: Could not create locations network file '%s'.\n", buffer.c_str());
		_print_direction.SetImpliedInputFileType(BasePrint::NETWORK_FILE);
        std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(
            getFilenameFormatTime(_parameters.getLocationsNetworkFilename(), _parameters.getTimestamp(), true),
            _parameters.getInputSource(NETWORK_FILE), _print_direction)
        );
        while (Source->ReadRecord()) {
            std::string identifier(Source->GetValueAt(0));
            if (std::find(coordinateLocations->begin(), coordinateLocations->end(), identifier) == coordinateLocations->end())
                continue;
            if (Source->GetNumValues() > 1) {
                identifier = Source->GetValueAt(1);
                if (std::find(coordinateLocations->begin(), coordinateLocations->end(), identifier) == coordinateLocations->end())
                    continue;
            }
            for (long idx = 0; idx < Source->GetNumValues(); ++idx)
                network_file << (idx == 0 ? "" : ",") << Source->GetValueAt(idx);
            network_file << std::endl;
        }
        CParameters::InputSource source(CSV, ",", "\"", 0, false);
        _parameters.defineInputSource(NETWORK_FILE, source);
        _parameters.setLocationsNetworkFilename(buffer.c_str());
        network_file.close();
        _temp_files.push_back(buffer);
    }

    if (_parameters.GetMultipleCoordinatesType() != ONEPERLOCATION && _parameters.getMultipleLocationsFile().size()) {
        std::ofstream ml_file;
        ml_file.open(createTempFilename(detectedCluster, supplementInfo, ".ml", buffer).c_str());
        if (!ml_file) throw resolvable_error("Error: Could not create multiple locations file '%s'.\n", buffer.c_str());
        _print_direction.SetImpliedInputFileType(BasePrint::MULTIPLE_LOCATIONS);
        std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(
            getFilenameFormatTime(_parameters.getMultipleLocationsFile(), _parameters.getTimestamp(), true),
            _parameters.getInputSource(MULTIPLE_LOCATIONS_FILE), _print_direction)
        );
        while (Source->ReadRecord()) {
            std::string value(Source->GetValueAt(0));
            if (std::find(clusterLocations.begin(), clusterLocations.end(), value) == clusterLocations.end())
                continue;
            value = Source->GetValueAt(1);
            if (std::find(clusterLocations2.begin(), clusterLocations2.end(), value) == clusterLocations2.end())
                continue;
            for (long idx = 0; idx < Source->GetNumValues(); ++idx)
                ml_file << (idx == 0 ? "" : ",") << Source->GetValueAt(idx);
            ml_file << std::endl;
        }
        CParameters::InputSource source(CSV, ",", "\"", 0, false);
        _parameters.defineInputSource(MULTIPLE_LOCATIONS_FILE, source);
        _parameters.setMultipleLocationsFile(buffer.c_str());
        ml_file.close();
        _temp_files.push_back(buffer);
    }
}

void AbstractAnalysisDrilldown::createReducedGridFile(const CCluster& detectedCluster, const ClusterSupplementInfo& supplementInfo, const CSaTScanData& source_data_hub, unsigned int downlevel) {
    if (_parameters.UseSpecialGrid()) {
        std::string buffer;
        // Read through grid file and find coordinates that are within the radius of detected cluster, there must be at least one - the center of detected cluster.
        std::ofstream grid_file;
        grid_file.open(createTempFilename(detectedCluster, supplementInfo, ".grd", buffer).c_str());
        if (!grid_file) throw resolvable_error("Error: Could not create grid file '%s'.\n", buffer.c_str());
		_print_direction.SetImpliedInputFileType(BasePrint::GRIDFILE);
        std::auto_ptr<DataSource> Source(DataSource::GetNewDataSourceObject(
            getFilenameFormatTime(_parameters.GetSpecialGridFileName(), _parameters.getTimestamp(), true),
            _parameters.getInputSource(GRIDFILE), _print_direction)
        );
        std::vector<double> gridCoordinates(source_data_hub.getLocationsManager().expectedDimensions(), 0.0), clusterCoordinates;
        short iScanCount;
        source_data_hub.GetGInfo()->retrieveCoordinates(detectedCluster.GetCentroidIndex(), clusterCoordinates);
        stsClusterCentroidGeometry clusterCentroid(clusterCoordinates);
        double clusterRadius = MostLikelyClustersContainer::GetClusterRadius(source_data_hub, detectedCluster);
        while (Source->ReadRecord()) {
            if (_parameters.GetCoordinatesType() == LATLON)
                SaTScanDataReader::ReadLatitudeLongitudeCoordinates(*Source, _print_direction, gridCoordinates, 0, "grid");
            else
                SaTScanDataReader::ReadCartesianCoordinates(*Source, _print_direction, gridCoordinates, iScanCount, 0);
            stsClusterCentroidGeometry gridPoint(gridCoordinates);
            if (
                (detectedCluster.GetEllipseOffset() == 0 && MostLikelyClustersContainer::CentroidLiesWithinSphereRegion(gridPoint, clusterCentroid, clusterRadius)) ||
                (detectedCluster.GetEllipseOffset() > 0 &&
                    MostLikelyClustersContainer::PointLiesWithinEllipseArea(
                        gridPoint.GetCoordinates()[0], gridPoint.GetCoordinates()[1],
                        clusterCentroid.GetCoordinates()[0], clusterCentroid.GetCoordinates()[1],
                        clusterRadius, source_data_hub.GetEllipseAngle(detectedCluster.GetEllipseOffset()), source_data_hub.GetEllipseShape(detectedCluster.GetEllipseOffset())
                    )
                    )
                ) {
                for (long idx=0; idx < Source->GetNumValues(); ++idx)
                    grid_file << (idx == 0 ? "" : ",") << Source->GetValueAt(idx);
                grid_file << std::endl;
            }
        }
        CParameters::InputSource source(CSV, ",", "\"", 0, false);
        _parameters.defineInputSource(GRIDFILE, source);
        _parameters.SetSpecialGridFileName(buffer.c_str());
        grid_file.close();
        _temp_files.push_back(buffer);
    }
}

/** Potentially performs drilldown on detected cluster. */
void AbstractAnalysisDrilldown::drilldownCluster(const AnalysisExecution& execution, const CCluster& cluster, unsigned int clusterRptIdx, unsigned int& drilldowns) {
    const CSaTScanData& datahub = execution.getDataHub();
    BasePrint& print = datahub.GetPrintDirection();
    const CParameters& parameters = execution.getParameters();
    const ClusterSupplementInfo& suppleInfo = execution.getClusterSupplement();
    const SimulationVariables& simVars = execution.getSimVariables();
    
    if (suppleInfo.test(cluster)/*reported?*/ && shouldDrilldown(cluster, clusterRptIdx + 1, datahub, parameters, simVars)) {
        if (parameters.getPerformStandardDrilldown()) {
            try {
                print.Printf(
                    "Performing main analysis %u%s level drilldown on %u%s detected cluster\n", BasePrint::P_STDOUT,
                    datahub.getDrilldownLevel() + 1, ordinal_suffix(datahub.getDrilldownLevel() + 1),
                    suppleInfo.getClusterReportIndex(cluster), ordinal_suffix(suppleInfo.getClusterReportIndex(cluster))
                );
                AnalysisDrilldown drilldown(
                    cluster, suppleInfo, datahub, execution.getDrilldownBaseOutput(), execution.getExecutioningType(),
                    datahub.getDrilldownLevel() + 1, drilldowns, boost::optional<const std::string&>(execution.getDrilldownClusterPath())
                );
                drilldown.execute(parameters.GetOutputFileName());
                const_cast<CParameters&>(parameters).addDrilldownResultFilename(drilldown.getParameters().GetOutputFileName());
            }
            catch (drilldown_exception& x) {
                print.Printf("The main analysis %u%s level drilldown did not execute on %u%s detected cluster:\n%s\n",
                    BasePrint::P_WARNING, datahub.getDrilldownLevel() + 1, ordinal_suffix(datahub.getDrilldownLevel() + 1),
                    suppleInfo.getClusterReportIndex(cluster), ordinal_suffix(suppleInfo.getClusterReportIndex(cluster)),
                    x.what()
                );
            }
            catch (resolvable_error& x) {
                print.Printf("The main analysis %u%s level drilldown stopped execution on %u%s detected cluster:\n%s\n",
                    BasePrint::P_WARNING, datahub.getDrilldownLevel() + 1, ordinal_suffix(datahub.getDrilldownLevel() + 1),
                    suppleInfo.getClusterReportIndex(cluster), ordinal_suffix(suppleInfo.getClusterReportIndex(cluster)),
                    x.what()
                );
            }
        }
        if (parameters.getPerformBernoulliDrilldown() && 
            (cluster.GetClusterType() == SPACETIMECLUSTER /* one additional restriction for Bernoulli */ ||
            (parameters.GetIsPurelySpatialAnalysis() && parameters.GetProbabilityModelType() == BERNOULLI) )) {
            try {
                print.Printf("Performing purely spatial Bernoulli %u%s level drilldown on %u%s detected cluster\n",
                    BasePrint::P_STDOUT, datahub.getDrilldownLevel() + 1, ordinal_suffix(datahub.getDrilldownLevel() + 1),
                    suppleInfo.getClusterReportIndex(cluster), ordinal_suffix(suppleInfo.getClusterReportIndex(cluster))
                );
                BernoulliAnalysisDrilldown drilldown(
                    cluster, suppleInfo, datahub, execution.getDrilldownBaseOutput(), execution.getExecutioningType(),
                    datahub.getDrilldownLevel() + 1, drilldowns, boost::optional<const std::string&>(execution.getDrilldownClusterPath())
                );
                drilldown.execute(parameters.GetOutputFileName());
                const_cast<CParameters&>(parameters).addDrilldownResultFilename(drilldown.getParameters().GetOutputFileName());
            }
            catch (drilldown_exception& x) {
                print.Printf("The purely spatial Bernoulli %u%s level drilldown did not execute on %u%s detected cluster:\n%s\n",
                    BasePrint::P_WARNING, datahub.getDrilldownLevel() + 1, ordinal_suffix(datahub.getDrilldownLevel() + 1),
                    suppleInfo.getClusterReportIndex(cluster), ordinal_suffix(suppleInfo.getClusterReportIndex(cluster)),
                    x.what()
                );
            }
            catch (resolvable_error& x) {
                print.Printf("The purely spatial Bernoulli %u%s level drilldown stopped execution on %u%s detected cluster:\n%s\n",
                    BasePrint::P_WARNING, datahub.getDrilldownLevel() + 1, ordinal_suffix(datahub.getDrilldownLevel() + 1),
                    suppleInfo.getClusterReportIndex(cluster), ordinal_suffix(suppleInfo.getClusterReportIndex(cluster)),
                    x.what()
                );
            }
        }
    }
}

/** Executes the analysis drilldown. */
void AbstractAnalysisDrilldown::execute(const std::string& parentOutputFilename) {
    // Sanity check - ensure that parameter settings are correct.
    if (!ParametersValidate(_parameters).Validate(_print_direction, true, _data_hub->isDrilldown()))
        throw prg_error("Drilldown parameter settings are invalid.", "execute()");
    if (_print_direction.GetIsCanceled()) return;
    //calculate number of neighboring locations about each centroid
    if (_executing_type == SUCCESSIVELY && _parameters.GetProbabilityModelType() != HOMOGENEOUSPOISSON) {
        macroRunTimeStartSerial(SerialRunTimeComponent::NeighborCalcuation);
        _data_hub->FindNeighbors();
        macroRunTimeStopSerial();
    }
    if (_print_direction.GetIsCanceled()) return;
    // Execute the analysis.
    AnalysisExecution execution(*_data_hub, _executing_type, _start_time, _drilldowns);
    execution.setDrilldownAttributes(_base_output, _cluster_path);
    execution.execute();
    if (_print_direction.GetIsCanceled()) return;
    _significant_clusters = execution.getNumSignificantClusters();
    // Register this drilldowns results.
    _print_direction.ReportDrilldownResults(_parameters.GetOutputFileName().c_str(), parentOutputFilename.c_str(), _significant_clusters);
    // Attempt to drilldown on most likely clusters, unless the analysis is an iterative scan. Iterative scans drilldown on clusters
    // at the end of each scan iteration.
    if ((_parameters.getPerformStandardDrilldown() || _parameters.getPerformBernoulliDrilldown()) && !_parameters.GetIsIterativeScanning()) {
        const MostLikelyClustersContainer & mlc = execution.getLargestMaximaClusterCollection();
        for (tract_t c = 0; c < mlc.GetNumClustersRetained(); ++c) {
            drilldownCluster(execution, mlc.GetCluster(c), c, _drilldowns);
        }
    }
    execution.finalize();
    _print_direction.Printf(
        "Drilldown anaylsis completed successfully.\nThe results have been written to:\n%s\n\n",
        BasePrint::P_STDOUT, _parameters.GetOutputFileName().c_str()
    );
}

/////////////////////////// AnalysisDrilldown ////////////////////////////////////

std::string AnalysisDrilldown::TYPE_IDENTIFIER = "std";

AnalysisDrilldown::AnalysisDrilldown(
    const CCluster& detectedCluster, const ClusterSupplementInfo& supplementInfo, const CSaTScanData& source_data_hub, const std::string& base_output,
    ExecutionType executing_type, unsigned int downlevel, unsigned int& drilldowns, boost::optional<const std::string&> cluster_path
): AbstractAnalysisDrilldown(
    source_data_hub.GetParameters(), base_output, executing_type, source_data_hub.GetPrintDirection(),
    downlevel, source_data_hub.getDrilldownRunId(), drilldowns, cluster_path
) {
    // Restrict to purely spatial or space-time analyses - ParametersValidate should be guarding most invalid parameter settings.
    if (!(_parameters.GetIsPurelySpatialAnalysis() || _parameters.GetIsSpaceTimeAnalysis() || _parameters.GetAnalysisType() == SPATIALVARTEMPTREND))
        throw prg_error("AnalysisDrilldown is not implemented for Analysis Type '%d'.", "constructor()", _parameters.GetAnalysisType());
    // Create new data hub that is will be only data from detected cluster.
    _data_hub.reset(AnalysisRunner::getNewCSaTScanData(_parameters, _print_direction));
    _data_hub->setIsDrilldownLevel(downlevel);
    _data_hub->setDrilldownRunId(drilldowns);

    // If parent analysis is prospective and it's top level analysis, then we want to switch cutoffs from recurrence interval to p-value in the drillown.
    if (_parameters.GetIsProspectiveAnalysis() && downlevel == 1) {
        _parameters.setDrilldownCutoff(DEFAULT_CUTOFF_PVALUE);
        _parameters.setCutoffLineListCSV(DEFAULT_CUTOFF_PVALUE);
        _parameters.setTemporalGraphSignificantCutoff(DEFAULT_CUTOFF_PVALUE);
    }

    // Assign output file for this drilldown analysis.
    setOutputFilename(detectedCluster, supplementInfo, drilldowns);
    // Create new grid and coordinates file from locations defined in detected cluster.
    createReducedGridFile(detectedCluster, supplementInfo, source_data_hub, downlevel);
    createReducedCoodinatesFile(detectedCluster, supplementInfo, source_data_hub, downlevel);
    // Read files again but using the new coordinates file plus ignoring locations outside geographical area.
    macroRunTimeStartSerial(SerialRunTimeComponent::DataRead);
    _data_hub->ReadDataFromFiles();
    macroRunTimeStopSerial();
}

/////////////////////////// BernoulliAnalysisDrilldown ////////////////////////////////////

std::string BernoulliAnalysisDrilldown::TYPE_IDENTIFIER = "bin";

BernoulliAnalysisDrilldown::BernoulliAnalysisDrilldown(
    const CCluster& detectedCluster, const ClusterSupplementInfo& supplementInfo, const CSaTScanData& source_data_hub,
    const std::string& base_output, ExecutionType executing_type, unsigned int downlevel, unsigned int& drilldowns, boost::optional<const std::string&> cluster_path
) : AbstractAnalysisDrilldown(
    source_data_hub.GetParameters(), base_output, executing_type, source_data_hub.GetPrintDirection(), 
    downlevel, source_data_hub.getDrilldownRunId(), drilldowns, cluster_path
) {
    std::string buffer;
    const CParameters& source_parameters = source_data_hub.GetParameters();
    // This is a drilldown on a most likely cluster of an iterative scan from a purely spatial Bernoulli drilldown (This class calling itself).
    if (downlevel > 1 && source_parameters.getIsBernoulliIterativeDrilldown()) {
        // Create new data hub that is only the data from detected cluster.
        _data_hub.reset(AnalysisRunner::getNewCSaTScanData(_parameters, _print_direction));
        _data_hub->setIsDrilldownLevel(downlevel);
        _data_hub->setDrilldownRunId(drilldowns);
        // Assign output file for this drilldown analysis.
        setOutputFilename(detectedCluster, supplementInfo, drilldowns);
        // Create new grid and coordinates file from locations defined in detected cluster.
        createReducedGridFile(detectedCluster, supplementInfo, source_data_hub, downlevel);
        createReducedCoodinatesFile(detectedCluster, supplementInfo, source_data_hub, downlevel);
        // Read files again but using the new coordinates file plus ignoring locations outside geographical area. Exclude reading case and control data.
        SaTScanDataReader(*_data_hub).ReadBernoulliDrilldown();
        // Carry down knowledge of which sets have been removed in parent analysis.
        for (auto details : source_data_hub.GetDataSetHandler().getRemovedDataSetDetails())
            _data_hub->GetDataSetHandler().removeDataSet(details.get<0>());
        // Allocate the case and measure structures in new data hub - this is needed for the rare situation one
        // of the original data sets was removed because it contained no data.
        for (size_t d = 0; d < _data_hub->GetDataSetHandler().GetNumDataSets(); ++d) {
            _data_hub->GetDataSetHandler().GetDataSet(d).allocateCaseData();
            _data_hub->GetDataSetHandler().GetDataSet(d).allocateControlData();
        }
        // Reduce the data from calling analysis to this purely spatial Bernoulli.
        for (size_t d = 0; d < source_data_hub.GetNumDataSets(); ++d) {
            count_t ** ppCases = _data_hub->GetDataSetHandler().GetDataSet(d).getCaseData().GetArray(),
                ** ppControls = _data_hub->GetDataSetHandler().GetDataSet(d).getControlData().GetArray();
            count_t ** ppSourceCases = source_data_hub.GetDataSetHandler().GetDataSet(d).getCaseData().GetArray(),
                ** ppSourceControls = source_data_hub.GetDataSetHandler().GetDataSet(d).getControlData().GetArray();
            for (tract_t i = 1; i <= detectedCluster.getNumIdentifiers(); ++i) {
                tract_t tTract = source_data_hub.GetNeighbor(detectedCluster.GetEllipseOffset(), detectedCluster.GetCentroidIndex(), i, detectedCluster.GetCartesianRadius());
                tract_t drilldown_tract = _data_hub->getIdentifierInfo().getIdentifierIndex(source_data_hub.getIdentifierInfo().getIdentifierNameAtIndex(tTract, buffer)).get();
                ppCases[0][drilldown_tract] = ppSourceCases[0][tTract];
                ppControls[0][drilldown_tract] = ppSourceControls[0][tTract];
            }
        }
        // It's possible now that some of the data sets have zero cases or zero controls. Remove those that don't have both.
        size_t numSets = _data_hub->GetDataSetHandler().GetNumDataSets();
        for (int d = numSets - 1; d >= 0; --d) {
            unsigned int num_locations = _data_hub->GetDataSetHandler().GetDataSet(d).getCaseData().Get2ndDimension();
            count_t** ppCases = _data_hub->GetDataSetHandler().GetDataSet(d).getCaseData().GetArray(),
                ** ppControls = _data_hub->GetDataSetHandler().GetDataSet(d).getControlData().GetArray();
            if (std::all_of(ppCases[0], ppCases[0] + num_locations, [](count_t i) { return i == 0; }) ||
                std::all_of(ppControls[0], ppControls[0] + num_locations, [](count_t i) { return i == 0; }))
                _data_hub->GetDataSetHandler().removeDataSet(d);
        }
        if (_data_hub->GetDataSetHandler().GetNumDataSets() == 0)
            throw drilldown_exception("While attempting to perform a purely spatial Bernoulli drilldown, no data set was found to have both cases and controls.");
        // Run any post data read operations now that cases and controls are populated.
        _data_hub->PostDataRead();
    } else if (source_parameters.GetIsSpaceTimeAnalysis()) {
        // Switch analysis type to purely spatial Bernoulli.
        _parameters.SetAnalysisType(PURELYSPATIAL);
        _parameters.SetProbabilityModelType(BERNOULLI);
        _parameters.SetControlFileName("", false);
        // Only one level with this drilldown type.
        _parameters.setPerformStandardDrilldown(false);
        _parameters.setPerformBernoulliDrilldown(true);
        _parameters.setDrilldownAdjustWeeklyTrends(false);
        // Perform scan iteratively with default settings
        _parameters.SetIterativeScanning(true);
        _parameters.SetNumIterativeScans(DEFAULT_NUM_ITERATIVE_SCANS);
        _parameters.SetIterativeCutOffPValue(DEFAULT_ITERATIVE_CUTOFF_PVALUE);
        // Toggle off all parameter settings copied from primary analysis that are invalid for purely spatial Beronulli.
        _parameters.setCalculateOliveirasF(false);
        _parameters.SetSimulationType(STANDARD);
        _parameters.setPerformPowerEvaluation(false);
        _parameters.SetAdjustForEarlierAnalyses(false);
        _parameters.setAdjustForWeeklyTrends(false);
        _parameters.SetIncludePurelySpatialClusters(false);
        _parameters.SetIncludePurelyTemporalClusters(false);
        _parameters.SetSpatialAdjustmentType(SPATIAL_NOTADJUSTED);
        _parameters.SetTimeTrendAdjustmentType(TEMPORAL_NOTADJUSTED);
        _parameters.setOutputTemporalGraphFile(false);
        // If parent analysis is prospective and it's a top level analysis, then we want to switch cutoffs from recurrence interval to p-value in the drillown.
        if (source_parameters.GetIsProspectiveAnalysis() && downlevel == 1) {
            _parameters.setDrilldownCutoff(DEFAULT_CUTOFF_PVALUE);
            _parameters.setCutoffLineListCSV(DEFAULT_CUTOFF_PVALUE);
            _parameters.setTemporalGraphSignificantCutoff(DEFAULT_CUTOFF_PVALUE);
        }
        // If performing day of week adjustment on drilldown, potentially define multiple data sets.
        if (source_parameters.getDrilldownAdjustWeeklyTrends()) {
            /* The primary analysis is restricted to having a time aggregation length of 1 day and a study period of at least 14 days. But there is the possiblity
               that the detected cluster is less than 7 days long and therefore there wouldn't be cases/controls in all 7 data sets. */
            _parameters.setNumFileSets(std::min(7, detectedCluster.getClusterLength()));
            // Set default data set names since we're introducing 6 new data sets.
            std::vector<std::string> sourceNames = _parameters.getDataSourceNames();
            for (size_t t=sourceNames.size(); t < _parameters.getNumFileSets(); ++t)
                sourceNames.push_back(printString(buffer, "Data Set #%u", t + 1));
            _parameters.setDataSourceNames(sourceNames);
        } else {
            _parameters.setNumFileSets(source_parameters.getNumFileSets());
        }
        // Certain parameters become invalidate once we switch to multiple data sets.
        if (_parameters.getNumFileSets() > 1) {
            _parameters.setRiskLimitHighClusters(false);
            _parameters.setRiskLimitLowClusters(false);
        }
        // Create new data hub that is only the data from detected cluster.
        _data_hub.reset(AnalysisRunner::getNewCSaTScanData(_parameters, _print_direction));
        _data_hub->setIsDrilldownLevel(downlevel);
        _data_hub->setDrilldownRunId(drilldowns);
        // Assign output file for this drilldown analysis.
        setOutputFilename(detectedCluster, supplementInfo, drilldowns);
        // Create new grid and coordinates file from locations defined in detected cluster.
        createReducedGridFile(detectedCluster, supplementInfo, source_data_hub, downlevel);
        createReducedCoodinatesFile(detectedCluster, supplementInfo, source_data_hub, downlevel);
        // Read files again but using the new coordinates file plus ignoring locations outside geographical area. Exclude reading case and control data.
        SaTScanDataReader(*_data_hub).ReadBernoulliDrilldown();
        // Carry down knowledge of which sets have been removed in parent analysis.
        for (auto details : source_data_hub.GetDataSetHandler().getRemovedDataSetDetails())
            _data_hub->GetDataSetHandler().removeDataSet(details.get<0>());
        // Get detected clusters case data.
        if (!source_parameters.getDrilldownAdjustWeeklyTrends()) {
            // Allocate the case and measure structures in new data hub - this is needed for the rare situation one
            // of the original data sets was removed because it contained no data.
            for (size_t d = 0; d < _data_hub->GetDataSetHandler().GetNumDataSets(); ++d) {
                _data_hub->GetDataSetHandler().GetDataSet(d).allocateCaseData();
                _data_hub->GetDataSetHandler().GetDataSet(d).allocateControlData();
            }
            // Transform the data from calling analysis to purely spatial Bernoulli.
            for (size_t d = 0; d < source_data_hub.GetNumDataSets(); ++d) {
                count_t** ppCases = _data_hub->GetDataSetHandler().GetDataSet(d).getCaseData().GetArray(),
                    ** ppControls = _data_hub->GetDataSetHandler().GetDataSet(d).getControlData().GetArray();
                for (tract_t i = 1; i <= detectedCluster.getNumIdentifiers(); ++i) {
                    tract_t tTract = source_data_hub.GetNeighbor(detectedCluster.GetEllipseOffset(), detectedCluster.GetCentroidIndex(), i, detectedCluster.GetCartesianRadius());
                    tract_t drilldown_tract = _data_hub->getIdentifierInfo().getIdentifierIndex(source_data_hub.getIdentifierInfo().getIdentifierNameAtIndex(tTract, buffer)).get();
                    ppCases[0][drilldown_tract] = detectedCluster.GetObservedCountForTract(tTract, source_data_hub, d);
                    ppControls[0][drilldown_tract] = detectedCluster.GetCountForTractOutside(tTract, source_data_hub, d);
                }
            }
            // It's possible now that some of the data sets have zero cases or zero controls. Remove those that don't have both.
            size_t numSets = _data_hub->GetDataSetHandler().GetNumDataSets();
            for (int d = numSets - 1; d >= 0; --d) {
                unsigned int num_locations = _data_hub->GetDataSetHandler().GetDataSet(d).getCaseData().Get2ndDimension();
                count_t** ppCases = _data_hub->GetDataSetHandler().GetDataSet(d).getCaseData().GetArray(),
                    ** ppControls = _data_hub->GetDataSetHandler().GetDataSet(d).getControlData().GetArray();
                if (std::all_of(ppCases[0], ppCases[0] + num_locations, [](count_t i) { return i == 0; }) ||
                    std::all_of(ppControls[0], ppControls[0] + num_locations, [](count_t i) { return i == 0; }))
                    _data_hub->GetDataSetHandler().removeDataSet(d);
            }
            if (_data_hub->GetDataSetHandler().GetNumDataSets() == 0)
                throw drilldown_exception("While attempting to perform a purely spatial Bernoulli drilldown, no data set was found to have both cases and controls.");
        } else {
            // Transform the data from calling analysis to purely spatial Bernoulli - stratifying by day of week.
            size_t numSets = _data_hub->GetDataSetHandler().GetNumDataSets();
            // Allocate data structures for cases and controls.
            std::vector<count_t**> setCases, setControls;
            for (size_t d = 0; d < numSets; ++d) {
                setCases.push_back(_data_hub->GetDataSetHandler().GetDataSet(d).allocateCaseData().GetArray());
                setControls.push_back(_data_hub->GetDataSetHandler().GetDataSet(d).allocateControlData().GetArray());
            }
            count_t** ppClusterCases = source_data_hub.GetDataSetHandler().GetDataSet(0).getCaseData().GetArray();
            for (tract_t i = 1; i <= detectedCluster.getNumIdentifiers(); ++i) {
                tract_t tTract = source_data_hub.GetNeighbor(detectedCluster.GetEllipseOffset(), detectedCluster.GetCentroidIndex(), i, detectedCluster.GetCartesianRadius());
                tract_t drilldown_tract = _data_hub->getIdentifierInfo().getIdentifierIndex(source_data_hub.getIdentifierInfo().getIdentifierNameAtIndex(tTract, buffer)).get();
                // record number of cases by interval for current tract - stratifying by day of week.
                for (int interval = detectedCluster.m_nFirstInterval; interval < detectedCluster.m_nLastInterval; ++interval)
                    setCases[interval % numSets][0][drilldown_tract] += ppClusterCases[interval][tTract] - (interval + 1 == source_data_hub.GetNumTimeIntervals() ? 0 : ppClusterCases[interval + 1][tTract]);
                // record number of controls by interval for current tract - stratifying by day of week.
                for (int interval = 0; interval < detectedCluster.m_nFirstInterval; ++interval)
                    setControls[interval % numSets][0][drilldown_tract] += ppClusterCases[interval][tTract] - (interval + 1 == source_data_hub.GetNumTimeIntervals() ? 0 : ppClusterCases[interval + 1][tTract]);
                // record number of controls by interval for current tract - stratifying by day of week
                for (int interval = detectedCluster.m_nLastInterval + 1; interval < source_data_hub.GetNumTimeIntervals(); ++interval)
                    setControls[interval % numSets][0][drilldown_tract] += ppClusterCases[interval][tTract] - (interval + 1 == source_data_hub.GetNumTimeIntervals() ? 0 : ppClusterCases[interval + 1][tTract]);
            }
            // It's possible now that some of the data sets have zero cases or zero controls.
            unsigned int num_locations = _data_hub->GetDataSetHandler().GetDataSet(0).getCaseData().Get2ndDimension();
            for (int d = numSets - 1; d >= 0; --d) {
                if (std::all_of(setCases[d][0], setCases[d][0] + num_locations, [](count_t i) { return i == 0; }) ||
                    std::all_of(setControls[d][0], setControls[d][0] + num_locations, [](count_t i) { return i == 0; }))
                    _data_hub->GetDataSetHandler().removeDataSet(d);
            }
            if (_data_hub->GetDataSetHandler().GetNumDataSets() == 0)
                throw drilldown_exception("While attempting to perform a purely spatial Bernoulli drilldown, with the day of week adjustment, no data set was found to have both cases and controls.");
        }
        // Run any post data read operations now that cases and controls are populated.
        _data_hub->PostDataRead();
    } else
        throw prg_error("BernoulliAnalysisDrilldown is not implemented for Analysis Type '%d'.", "BernoulliAnalysisDrilldown()", source_parameters.GetAnalysisType());
}

//////////////////////////// AnalysisRunner //////////////////////////////////////

CSaTScanData * AnalysisRunner::getNewCSaTScanData(const CParameters& parameters, BasePrint& print) {
    switch (parameters.GetAnalysisType()) {
        case PURELYSPATIAL: return new CPurelySpatialData(parameters, print);
        case PURELYTEMPORAL:
        case PROSPECTIVEPURELYTEMPORAL: return new CPurelyTemporalData(parameters, print);
        case SPACETIME:
        case PROSPECTIVESPACETIME: return new CSpaceTimeData(parameters, print);
        case SPATIALVARTEMPTREND: return new CSVTTData(parameters, print); 
        case SEASONALTEMPORAL: return new ClosedLoopData(parameters, print);
        default: throw prg_error("Unknown Analysis Type '%d'.", "getNewCSaTScanData()", parameters.GetAnalysisType());
    };
    return 0;
}

/** constructor */
AnalysisRunner::AnalysisRunner(const CParameters& parameters, time_t start_time, BasePrint& print_direction)
               :_parameters(parameters), _start_time(start_time), _print_direction(print_direction), _executing_type(parameters.GetExecutionType()), _drilldowns(0)
 {
  try {
    macroRunTimeManagerInit();
    //create data hub
    _data_hub.reset(getNewCSaTScanData(_parameters, _print_direction));
  } catch (prg_exception& x) {
    x.addTrace("constructor()","AnalysisRunner");
    throw;
  }
}

/** Executes analysis - conditionally running successive or centric processes. */
void AnalysisRunner::run() {
  try {
    //read data
    macroRunTimeStartSerial(SerialRunTimeComponent::DataRead);
    _data_hub->ReadDataFromFiles();
    macroRunTimeStopSerial();
    if (_print_direction.GetIsCanceled()) return;
    //calculation approxiate amount of memory required to run analysis
    std::pair<double, double> prMemory = getMemoryApproxiation(_parameters, *_data_hub);
    if (_executing_type == AUTOMATIC) //prefer successive execution if: enough RAM, or memory needs less than centric, or centric execution not a valid option given parameters
        _executing_type = (prMemory.first < getAvailablePhysicalMemory() || prMemory.first < prMemory.second || !_parameters.GetPermitsCentricExecution()) ? SUCCESSIVELY : CENTRICALLY;
    try {
        // sanity check for invalidate paramater configurations
        if (_parameters.getPerformPowerEvaluation() && _parameters.getPowerEvaluationMethod() != PE_ONLY_SPECIFIED_CASES && !_parameters.GetCaseFileName().size())
            throw prg_error("Error: Power evaluation is not implemented such that both the number of power cases and the case file(s) are unspecified.\n","run()");
        //calculate number of neighboring locations about each centroid
        if (_executing_type == SUCCESSIVELY && _parameters.GetProbabilityModelType() != HOMOGENEOUSPOISSON) {
            macroRunTimeStartSerial(SerialRunTimeComponent::NeighborCalcuation);
            _data_hub->FindNeighbors();
            macroRunTimeStopSerial();
        }
        if (_print_direction.GetIsCanceled()) return;

        boost::shared_ptr<AnalysisExecution> execution(getAnalysisExecution());
        execution->execute();
        if (_print_direction.GetIsCanceled()) return;

        // Perform analysis drilldowns - if requested by user.
        if ((_parameters.getPerformStandardDrilldown() || _parameters.getPerformBernoulliDrilldown()) && !_parameters.GetIsIterativeScanning()) {
            const MostLikelyClustersContainer & mlc = execution->getLargestMaximaClusterCollection();
            for (tract_t c = 0; c < mlc.GetNumClustersRetained(); ++c) {
                const CCluster& cluster = mlc.GetCluster(c);
                if (execution->getClusterSupplement().test(cluster)/*reported?*/ && AbstractAnalysisDrilldown::shouldDrilldown(cluster, c + 1, *_data_hub, _parameters, execution->getSimVariables())) {
                    if (_parameters.getPerformStandardDrilldown()) {
                        try {
                            _print_direction.Printf(
                                "Performing main analysis %u%s level drilldown on %u%s detected cluster\n", BasePrint::P_STDOUT, 1, ordinal_suffix(1),
                                execution->getClusterSupplement().getClusterReportIndex(cluster), ordinal_suffix(execution->getClusterSupplement().getClusterReportIndex(cluster))
                            );
                            AnalysisDrilldown drilldown(
                                cluster, execution->getClusterSupplement(), *_data_hub, _parameters.GetOutputFileName(), _executing_type, 1, _drilldowns
                            );
                            drilldown.execute(_parameters.GetOutputFileName());
                            const_cast<CParameters&>(_parameters).addDrilldownResultFilename(drilldown.getParameters().GetOutputFileName());
                        } catch (drilldown_exception& x) {
                            _print_direction.Printf(
                                "The main analysis %u%s level drilldown did not execute on %u%s detected cluster:\n%s\n", BasePrint::P_WARNING,
                                1, ordinal_suffix(1),
                                execution->getClusterSupplement().getClusterReportIndex(cluster), ordinal_suffix(execution->getClusterSupplement().getClusterReportIndex(cluster)), x.what()
                            );
                        } catch (resolvable_error& x) {
                            _print_direction.Printf(
                                "The main analysis %u%s level drilldown stopped execution on %u%s detected cluster:\n%s\n", BasePrint::P_WARNING,
                                1, ordinal_suffix(1),
                                execution->getClusterSupplement().getClusterReportIndex(cluster), ordinal_suffix(execution->getClusterSupplement().getClusterReportIndex(cluster)), x.what()
                            );
                        }
                    }
                    if (_parameters.getPerformBernoulliDrilldown() && cluster.GetClusterType() == SPACETIMECLUSTER /* one additional restriction for Bernoulli */ ) {
                        try {
                            _print_direction.Printf(
                                "Performing purely spatial Bernoulli %u%s level drilldown on %u%s detected cluster\n", BasePrint::P_STDOUT, 1, ordinal_suffix(1),
                                execution->getClusterSupplement().getClusterReportIndex(cluster), ordinal_suffix(execution->getClusterSupplement().getClusterReportIndex(cluster))
                            );
                            BernoulliAnalysisDrilldown drilldown(
                                cluster, execution->getClusterSupplement(), *_data_hub, _parameters.GetOutputFileName(), _executing_type, 1, _drilldowns
                            );
                            drilldown.execute(_parameters.GetOutputFileName());
                            const_cast<CParameters&>(_parameters).addDrilldownResultFilename(drilldown.getParameters().GetOutputFileName());
                        } catch (drilldown_exception& x) {
                            _print_direction.Printf(
                                "The purely spatial Bernoulli %u%s level drilldown did not execute on %u%s detected cluster:\n%s\n", BasePrint::P_WARNING, 1, ordinal_suffix(1),
                                execution->getClusterSupplement().getClusterReportIndex(cluster), ordinal_suffix(execution->getClusterSupplement().getClusterReportIndex(cluster)), x.what()
                            );
                        } catch (resolvable_error& x) {
                            _print_direction.Printf(
                                "The purely spatial Bernoulli %u%s level drilldown stopped execution on %u%s detected cluster:\n%s\n", BasePrint::P_WARNING, 1, ordinal_suffix(1),
                                execution->getClusterSupplement().getClusterReportIndex(cluster), ordinal_suffix(execution->getClusterSupplement().getClusterReportIndex(cluster)), x.what()
                            );
                        }
                    }
                }
            }
        }
        // Finalize analysis execution.
        execution->finalize();
    } catch (std::bad_alloc &b) {
        std::string additional;
        //Potentially provide detailed options given user parameter settings:
        if (_executing_type == SUCCESSIVELY && _parameters.GetPermitsCentricExecution(true) &&
            _parameters.GetPValueReportingType() == TERMINATION_PVALUE && _parameters.GetNumReplicationsRequested() >= MIN_SIMULATION_RPT_PVALUE) {
            additional = "\nNote: SaTScan could not utilize the alternative memory allocation for\n"
                         "this analysis because of the P-Value reporting setting (sequential Monte Carlo).\n"
                         "Consider changing this setting, which will enable analysis to utilize the\n"
                         "alternative memory allocation and possibly execute without memory issues.\n";
        }
        throw resolvable_error("\nSaTScan is unable to perform analysis due to insufficient memory.\n"
                               "Please see 'Memory Requirements' in user guide for suggested solutions.\n"
                               "Note that memory needs are on the order of %.0lf MB.\n%s",
                               (_executing_type == SUCCESSIVELY ? prMemory.first : prMemory.second), additional.c_str());
    }
  } catch (prg_exception& x) {
    x.addTrace("run()","AnalysisRunner");
    throw;
  }
}

/** Returns available random access memory om mega-bytes. */
double AnalysisRunner::getAvailablePhysicalMemory() {
  double /*dTotalPhysicalMemory(0),*/ dAvailablePhysicalMemory(0);

  //need process for handling failure from system call

#ifdef _WINDOWS_
  MEMORYSTATUS stat;
  GlobalMemoryStatus (&stat);
  //dTotalPhysicalMemory = stat.dwTotalPhys;
  dAvailablePhysicalMemory = static_cast<double>(stat.dwAvailPhys);
#elif defined(__APPLE__)
    int physmem;
    size_t len = sizeof physmem;
    static int mib[2] = { CTL_HW, HW_USERMEM };
    if (sysctl (mib, 2, &physmem, &len, NULL, 0) == 0 && len == sizeof (physmem)) {
        dAvailablePhysicalMemory = static_cast<double>(physmem);
    }
#else
  //dTotalPhysicalMemory = sysconf(_SC_PHYS_PAGES);
  //dTotalPhysicalMemory *= sysconf(_SC_PAGESIZE);
  dAvailablePhysicalMemory = sysconf(_SC_AVPHYS_PAGES);
  dAvailablePhysicalMemory *= sysconf(_SC_PAGESIZE);
#endif

  return std::ceil(dAvailablePhysicalMemory/1000000);
}

/** Approxiates the amount of memory (in MB) that will be required to run this ananlysis,
    both Standard Memory Allocation and Special Memory Allocation. Note that these formulas
    are largely identical to that of user guide. */
std::pair<double, double> AnalysisRunner::getMemoryApproxiation(const CParameters& parameters, const CSaTScanData& data_hub) {
  std::pair<double, double>  prMemoryAppoxiation;

   //the number of location IDs in the coordinates file
  double L = data_hub.GetNumIdentifiers();
  //the number of coordinates in the grid file (G=L if no grid file is specified)
  //double G = gpDataHub->GetGInfo()->getNumGridPoints();
  //maximum geographical cluster size, as a proportion of the population ( 0 < mg = ½ , mg=1 for a purely temporal analysis)
  double mg=0;
  if (parameters.GetIsPurelyTemporalAnalysis()) mg = 0.0;
  else if (parameters.GetAnalysisType() == PROSPECTIVESPACETIME && parameters.GetAdjustForEarlierAnalyses()) {
    if (parameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false))
      mg = parameters.GetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false);
    else
      mg = parameters.GetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false);
  }
  else if (parameters.UseLocationNeighborsFile()) mg = 0.25; // pure guess
  else mg = parameters.GetMaxSpatialSizeForType(PERCENTOFPOPULATION, false)/100.0;
  //number of time intervals into which the temporal data is aggregated (TI=1 for a purely spatial analysis)               
  double TI = data_hub.GetNumTimeIntervals();
  //read data structures
  double b=1;
  switch (parameters.GetProbabilityModelType()) {
    case POISSON:
    case SPACETIMEPERMUTATION:
    case UNIFORMTIME:
    case EXPONENTIAL:  b = sizeof(count_t) + sizeof(measure_t); break;
    case BERNOULLI: b = 2 * sizeof(count_t) + sizeof(measure_t); break;
    case CATEGORICAL:
    case ORDINAL: b = sizeof(count_t); break;
    case BATCHED:
    case NORMAL: b = sizeof(count_t) + sizeof(measure_t) + sizeof(measure_t); break;
    case HOMOGENEOUSPOISSON: b = 1; break; // ??
    case RANK: b = sizeof(count_t) + sizeof(measure_t); break;
    default : throw prg_error("Unknown model type '%d'.\n", "getMemoryApproxiation()", parameters.GetProbabilityModelType());
  };
  //the number of categories in the ordinal model (CAT=1 for other models)
  double CAT = (parameters.GetProbabilityModelType() == ORDINAL || parameters.GetProbabilityModelType() == CATEGORICAL ? 0 : 1);
  for (size_t i=0; i < data_hub.GetDataSetHandler().GetNumDataSets(); ++i)
     CAT += data_hub.GetDataSetHandler().GetDataSet(i).getPopulationData().GetNumOrdinalCategories();
  //for exponential model, EXP =1 one for all other models
  double EXP = 1; //EXP is multiplied by 4 bytes
  switch (parameters.GetProbabilityModelType()) {
    case POISSON:
    case SPACETIMEPERMUTATION:
    case BERNOULLI:
    case HOMOGENEOUSPOISSON: 
    case CATEGORICAL:
    case RANK:
    case UNIFORMTIME:
    case ORDINAL: EXP = 1; break;
    case EXPONENTIAL: EXP = 3; break; //cases and measure
    case BATCHED:
    case NORMAL: EXP = 4; break; //cases, measure and measure squared
    default : throw prg_error("Unknown model type '%d'.\n", "getMemoryApproxiation()", parameters.GetProbabilityModelType());
  };
  //the total number of cases (for the ordinal model or multiple data sets, C=0)
  double C = (parameters.GetProbabilityModelType() == ORDINAL || parameters.GetProbabilityModelType() == CATEGORICAL || data_hub.GetDataSetHandler().GetNumDataSets() > 1 ? 0 : data_hub.GetDataSetHandler().GetDataSet(0).getTotalCases());
  //1 when scanning for high rates only or low rates only, R=2 when scanning for either high or low rates
  double R = (parameters.GetAreaScanRateType() == HIGHANDLOW ? 2 : 1);
  //number of data sets
  double D = static_cast<double>(data_hub.GetDataSetHandler().GetNumDataSets());
  //number of processors available on the computer for SaTScan use
  double P = parameters.GetNumParallelProcessesToExecute();
  //is the number of Monte Carlo simulations
  double MC = parameters.GetNumReplicationsRequested();
  //sort array data type size
  double SortedDataTypeSize(data_hub.GetNumIdentifiers() < (int)std::numeric_limits<unsigned short>::max() ? static_cast<double>(sizeof(unsigned short)) : static_cast<double>(sizeof(int)));
  //size of sorted array -- this formula deviates from the user guide slightly
  double SortedNeighborsArray = (parameters.GetIsPurelyTemporalAnalysis() ? 0 :
                                 (double)sizeof(void**) * (parameters.GetNumTotalEllipses()+1) +
                                 (double)sizeof(void*) * (parameters.GetNumTotalEllipses()+1) * data_hub.m_nGridTracts +
                                 (double)(parameters.GetNumTotalEllipses()+1) * data_hub.m_nGridTracts * SortedDataTypeSize * data_hub.GetNumIdentifiers() * mg);
  //Standard Memory Allocation
  prMemoryAppoxiation.first = std::ceil((SortedNeighborsArray + (b + 4.0 * EXP * P) * L * TI * CAT * D + sizeof(measure_t) * C * R * P)/1000000);
  //Special Memory Allocation
  prMemoryAppoxiation.second = std::ceil((4 * L * TI * CAT * EXP * D * MC + sizeof(measure_t) * MC * C * R * P)/1000000);
  return prMemoryAppoxiation;
}

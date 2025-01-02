//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "PurelyTemporalAnalysis.h"
#include "MostLikelyClustersContainer.h"
#include "SSException.h"

/** Constructor */
CPurelyTemporalAnalysis::CPurelyTemporalAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
:CAnalysis(Parameters, DataHub, PrintDirection) {}

/** Allocates objects used during simulations, instead of repeated allocations
    for each simulation. Which objects that are allocated depends on whether
    the simluations process uses same process as real data or uses measure list. */
void CPurelyTemporalAnalysis::AllocateSimulationObjects(const AbstractDataSetGateway& DataGateway) {
    try {
        _time_intervals.reset(GetNewTemporalDataEvaluatorObject(
            _parameters.GetIsProspectiveAnalysis() ? ALLCLUSTERS : _parameters.GetIncludeClustersType(), SUCCESSIVELY
        ));
        if (_replica_process_type == MeasureListEvaluation)
            _measure_list.reset(GetNewMeasureListObject());
        _cluster_data.reset(_cluster_data_factory->GetNewTemporalClusterData(DataGateway));
    } catch (prg_exception& x) {
        x.addTrace("AllocateSimulationObjects()","CPurelyTemporalAnalysis");
        throw;
    }
}

const SharedClusterVector_t CPurelyTemporalAnalysis::CalculateTopClusters(tract_t, const AbstractDataSetGateway&) {
    throw prg_error("CalculateTopCluster() can not be called for CPurelyTemporalAnalysis.","CPurelyTemporalAnalysis");
}

/** Calculates most likely cluster(s) for a purely temporal scan. */
void CPurelyTemporalAnalysis::FindTopClusters(const AbstractDataSetGateway& DataGateway, MLC_Collections_t& TopClustersContainers) {
    try {
        // Determine the type of clusters to compare
        IncludeClustersType includeType = _parameters.GetIsProspectiveAnalysis() ? ALIVECLUSTERS : _parameters.GetIncludeClustersType();
        // Create cluster objects
        CPurelyTemporalCluster clusterCompare(_cluster_data_factory.get(), DataGateway, includeType, _data_hub);
        boost::shared_ptr<CTimeIntervals> timeIntervals(GetNewTemporalDataEvaluatorObject(includeType, SUCCESSIVELY));
        boost::shared_ptr<CClusterSet> clusterSet(CClusterSet::getNewCClusterSetObject(clusterCompare, _data_hub));
        CClusterObject clusterObject(clusterCompare);
        clusterSet->add(clusterObject);
        // Iterate through time intervals, finding top cluster(s).
        timeIntervals->CompareClusterSet(clusterCompare, *clusterSet);
        // If there is a most likely cluster, possibly search for other clusters then add to MLC container.
        if (clusterSet->getSet().front().getCluster().ClusterDefined()) {
            CClusterSet::ClusterContainer_t clusters;
            clusters.push_back(clusterSet->getSet().front()); // Add MLC to the collection of purely temporal clusters.
            // Search for other purely temporal clusters that don't overlap temporally with the MLC, or each other.
            if (_parameters.GetAnalysisType() == PURELYTEMPORAL && !_parameters.GetIsIterativeScanning()) // retrospective purely temporal only
                CClusterSetTemporalOverlap::findOtherNotTemporalOverlapping(clusters, *clusterSet, _data_hub);
            assert(TopClustersContainers.size() == 1);
            for (auto& clusterObj : clusters)
                TopClustersContainers.front().Add(clusterObj.getCluster());
        }
    } catch (prg_exception& x) {
        x.addTrace("FindTopClusters()","CPurelyTemporalAnalysis");
        throw;
    }
}

/** Returns maximized value for Monte Carlo replication. Depending on the parameter settings,
    value returned might either be the maximizing value or full loglikelihood ratio/test statistic. */
double CPurelyTemporalAnalysis::MonteCarlo(tract_t, const AbstractDataSetGateway&) {
    _cluster_data->InitializeData();
    //iterate through time intervals, finding top cluster
    double dMaximumingValue = _time_intervals->ComputeMaximizingValue(*_cluster_data);
    if (_data_hub.GetDataSetHandler().GetNumDataSets() == 1)
        dMaximumingValue = _likelihood_calculator->CalculateFullStatistic(dMaximumingValue);
    return std::max(0.0, dMaximumingValue);  
}

/** Returns log likelihood ratio for Monte Carlo replication. */
double CPurelyTemporalAnalysis::MonteCarlo(const DataSetInterface&) {
    _measure_list->Reset();
    macroRunTimeStartFocused(FocusRunTimeComponent::MeasureListScanningAdding);
    _time_intervals->CompareMeasures(*_cluster_data, *_measure_list);
    macroRunTimeStopFocused(FocusRunTimeComponent::MeasureListScanningAdding);
    return _measure_list->GetMaximumLogLikelihoodRatio();
}


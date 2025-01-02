//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SpaceTimeAnalysis.h"
#include "MakeNeighbors.h"
#include "ClusterData.h"
#include "SSException.h"

#include "BatchedLikelihoodCalculation.h"

/** Constructor */
CSpaceTimeAnalysis::CSpaceTimeAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
:CAnalysis(Parameters, DataHub, PrintDirection), _top_clusters(DataHub) {}

/** Allocates objects used during simulations, instead of repeated allocations
    for each simulation. Which objects that are allocated depends on whether
    the simluations process uses same process as real data or uses measure list. */
void CSpaceTimeAnalysis::AllocateSimulationObjects(const AbstractDataSetGateway& DataGateway) {
    try {
        //create new time intervals object - delete existing object used during real data process
        _time_intervals.reset(GetNewTemporalDataEvaluatorObject(
            _parameters.GetAnalysisType() == PROSPECTIVESPACETIME ? ALLCLUSTERS : _parameters.GetIncludeClustersType(), SUCCESSIVELY
        ));
        //create simulation objects based upon which process used to perform simulations
        if (_replica_process_type == MeasureListEvaluation)
            _measure_list.reset(GetNewMeasureListObject());
        _cluster_data.reset(_cluster_data_factory->GetNewSpaceTimeClusterData(DataGateway));
    } catch (prg_exception& x) {
        x.addTrace("AllocateSimulationObjects()","CSpaceTimeAnalysis");
        throw;
    }
}

/** Allocates objects used during calculation of most likely clusters, instead of repeated allocations for each grid point. */
void CSpaceTimeAnalysis::AllocateTopClustersObjects(const AbstractDataSetGateway& DataGateway) {
    try {
        //create new time intervals object - delete existing object used during real data process
        _time_intervals.reset(GetNewTemporalDataEvaluatorObject(
            _parameters.GetAnalysisType() == PROSPECTIVESPACETIME ? ALIVECLUSTERS : _parameters.GetIncludeClustersType(), SUCCESSIVELY
        ));
        //create cluster object used as comparator when iterating over centroids and time intervals
        _cluster_compare.reset(new CSpaceTimeCluster(_cluster_data_factory.get(), DataGateway));
        //initialize list of top circle/ellipse clusters
        _top_clusters.setTopClusters(*_cluster_compare);
    } catch (prg_exception& x) {
        x.addTrace("AllocateTopClustersObjects()","CSpaceTimeAnalysis");
        throw;
    }
}

/** Calculates the top clusters about centroid and returns collection for each spatial expansion stop. */
const SharedClusterVector_t CSpaceTimeAnalysis::CalculateTopClusters(tract_t tCenter, const AbstractDataSetGateway& DataGateway) {
    _top_clusters.resetAboutCentroid(tCenter);
    _time_intervals->setIntervalRange(tCenter);
    //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
    for (tract_t k=0; k <= _parameters.GetNumTotalEllipses(); ++k) {
        CentroidNeighbors CentroidDef(k, _data_hub, tCenter);
        _top_clusters.resetNeighborCounts(k);
        _cluster_compare->Initialize(tCenter);
        _cluster_compare->SetEllipseOffset(k, _data_hub);
        _cluster_compare->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef, _top_clusters.getClusterSet(k), *_time_intervals);
    }
    SharedClusterVector_t clusters;
    return _top_clusters.getTopClusters(clusters);
}

/** Returns loglikelihood ratio for Monte Carlo replication using same algorithm as real data. */
double CSpaceTimeAnalysis::MonteCarlo(tract_t tCenter, const AbstractDataSetGateway& DataGateway) {
    tract_t                       t, tNumNeighbors, * pIntegerArray;
    unsigned short              * pUnsignedShortArray;
    double                        dMaximizingValue;
    std::vector<double>           vMaximizingValues(_parameters.GetNumTotalEllipses() + 1, -std::numeric_limits<double>::max());
    std::vector<double>::iterator itr, itr_end;

    _time_intervals->setIntervalRange(tCenter);
    for (int j=0; j <= _parameters.GetNumTotalEllipses(); ++j) {
        double& dShapeMaxValue = vMaximizingValues[j];
        _cluster_data->InitializeData();
        CentroidNeighbors CentroidDef(j, _data_hub);
        CentroidDef.Set(tCenter);
        tNumNeighbors = CentroidDef.GetNumNeighbors();
        pUnsignedShortArray = CentroidDef.GetRawUnsignedShortArray();
        pIntegerArray = CentroidDef.GetRawIntegerArray();
        for (t=0; t < tNumNeighbors; ++t) {
            _cluster_data->AddNeighborData((pUnsignedShortArray ? (tract_t)pUnsignedShortArray[t] : pIntegerArray[t]), DataGateway);
            dMaximizingValue = _time_intervals->ComputeMaximizingValue(*_cluster_data);
            if (dMaximizingValue > dShapeMaxValue) dShapeMaxValue = dMaximizingValue;
        }
    }
    //if maximizing value is not a ratio/test statistic, convert them now
    if (_data_hub.GetDataSetHandler().GetNumDataSets() == 1)
        for (itr=vMaximizingValues.begin(),itr_end=vMaximizingValues.end(); itr != itr_end; ++itr)
            *itr = _likelihood_calculator->CalculateFullStatistic(*itr);
    //determine which ratio/test statistic is the greatest, be sure to apply compactness correction
    double dPenalty = _data_hub.GetParameters().GetNonCompactnessPenaltyPower();
    dMaximizingValue = vMaximizingValues.front() * CalculateNonCompactnessPenalty(_data_hub.GetEllipseShape(0), dPenalty);
    for (t=1,itr=vMaximizingValues.begin()+1,itr_end=vMaximizingValues.end(); itr != itr_end; ++itr, ++t) {
        *itr *= CalculateNonCompactnessPenalty(_data_hub.GetEllipseShape(t), dPenalty);
        dMaximizingValue = std::max(*itr, dMaximizingValue);
    }
    return dMaximizingValue;
}

/** Returns log likelihood ratio for Monte Carlo replication. */
double CSpaceTimeAnalysis::MonteCarlo(const DataSetInterface& Interface) {
    SpaceTimeData * pSpaceTimeData=0;
    if ((pSpaceTimeData = dynamic_cast<SpaceTimeData*>(_cluster_data.get())) == 0)
        throw prg_error("Cluster data object could not be dynamically casted to SpaceTimeData type.\n", "MonteCarlo()");
    _measure_list->Reset();
    //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
    for (tract_t k=0; k <= _parameters.GetNumTotalEllipses(); ++k) {
        CentroidNeighbors CentroidDef(k, _data_hub);
        for (tract_t i=0; i < _data_hub.m_nGridTracts; ++i) {
            CentroidDef.Set(i);
            _time_intervals->setIntervalRange(i);
            pSpaceTimeData->AddNeighborDataAndCompare(CentroidDef, Interface, *_time_intervals, *_measure_list);
        }
        _measure_list->SetForNextIteration(k);
    }
    return _measure_list->GetMaximumLogLikelihoodRatio();
}

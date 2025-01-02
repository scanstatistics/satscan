//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "PurelySpatialAnalysis.h"
#include "ClusterData.h"
#include "SSException.h"

/** Constructor */
CPurelySpatialAnalysis::CPurelySpatialAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
:CAnalysis(Parameters, DataHub, PrintDirection), _top_clusters(DataHub) {}

/** Allocates objects used during Monte Carlo simulations instead of repeated
    allocations for each simulation. This method must be called prior to MonteCarlo(). */
void CPurelySpatialAnalysis::AllocateSimulationObjects(const AbstractDataSetGateway& DataGateway) {
    try {
        //create simulation objects based upon which process used to perform simulations
        if (_replica_process_type == MeasureListEvaluation)
            _measure_list.reset(GetNewMeasureListObject());
        _cluster_data.reset(_cluster_data_factory->GetNewSpatialClusterData(DataGateway));
    } catch (prg_exception& x) {
        x.addTrace("AllocateSimulationObjects()","CPurelySpatialAnalysis");
        throw;
    }
}

/** Allocates objects used during calculation of most likely clusters, instead
    of repeated allocations. This method must be called prior to CalculateTopCluster(). */
void CPurelySpatialAnalysis::AllocateTopClustersObjects(const AbstractDataSetGateway& DataGateway) {
    try {
        _cluster_compare.reset(new CPurelySpatialCluster(_cluster_data_factory.get(), DataGateway));
        _top_clusters.setTopClusters(*_cluster_compare.get());
    } catch (prg_exception& x) {
        x.addTrace("AllocateTopClustersObjects()","CPurelySpatialAnalysis");
        throw;
    }
}

/** Calculates the top clusters about centroid and returns collection for each spatial expansion stop. */
const SharedClusterVector_t CPurelySpatialAnalysis::CalculateTopClusters(tract_t tCenter, const AbstractDataSetGateway& DataGateway) {
    _top_clusters.resetAboutCentroid(tCenter); // re-initialize top clusters objects
    for (int j=0; j <= _parameters.GetNumTotalEllipses(); ++j) {
        CentroidNeighbors CentroidDef(j, _data_hub, tCenter);
        _top_clusters.resetNeighborCounts(j);
        _cluster_compare->Initialize(tCenter);
        _cluster_compare->SetEllipseOffset(j, _data_hub);
        _cluster_compare->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef, _top_clusters.getClusterSet(j), *_likelihood_calculator);
    }
    SharedClusterVector_t topClusters;
    return _top_clusters.getTopClusters(topClusters);
}

/** Returns loglikelihood ratio for Monte Carlo replication using same algorithm as real data. */
double CPurelySpatialAnalysis::MonteCarlo(tract_t tCenter, const AbstractDataSetGateway & DataGateway) {
    tract_t                       t, tNumNeighbors, * pIntegerArray;
    unsigned short              * pUnsignedShortArray;
    double                        dMaximizingValue;
    std::vector<double>           vMaximizingValues(_parameters.GetNumTotalEllipses() + 1, -std::numeric_limits<double>::max());
    std::vector<double>::iterator itr, itr_end;

    // for weighted normal model with covariates, we need to reload matrix data at the beginning of each simulation
    if (tCenter == 0 && _parameters.getIsWeightedNormalCovariates()) 
        _cluster_data->InitializeData(DataGateway);
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
            dMaximizingValue = _cluster_data->GetMaximizingValue(*_likelihood_calculator);
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

/** Returns loglikelihood ratio for Monte Carlo replication utilizing measure list structure. */
double CPurelySpatialAnalysis::MonteCarlo(const DataSetInterface& Interface) {
    SpatialData * pSpatialData = dynamic_cast<SpatialData*>(_cluster_data.get());
    _measure_list->Reset();
    for (tract_t k=0; k <= _parameters.GetNumTotalEllipses(); ++k) {
        CentroidNeighbors CentroidDef(k, _data_hub);
        for (tract_t i=0; i < _data_hub.m_nGridTracts; ++i) {
            CentroidDef.Set(i);
            pSpatialData->AddMeasureList(CentroidDef, Interface, _measure_list.get());
        }
        _measure_list->SetForNextIteration(k);
    }
    return _measure_list->GetMaximumLogLikelihoodRatio();
}


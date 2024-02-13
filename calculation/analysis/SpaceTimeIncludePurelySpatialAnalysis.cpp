//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "SpaceTimeIncludePurelySpatialAnalysis.h"
#include "ClusterData.h"
#include "SSException.h"

/** Constructor */
C_ST_PS_Analysis::C_ST_PS_Analysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
                 :CSpaceTimeAnalysis(Parameters, DataHub, PrintDirection), _ps_top_clusters(DataHub), _psp_top_clusters(DataHub) {}

/** Allocates objects used during simulations, instead of repeated allocations
    for each simulation. Which objects that are allocated depends on whether
    the simluations process uses same process as real data or uses measure list. */
void C_ST_PS_Analysis::AllocateSimulationObjects(const AbstractDataSetGateway & DataGateway) {
  try {
    CSpaceTimeAnalysis::AllocateSimulationObjects(DataGateway);
    //create simulation objects based upon which process used to perform simulations
    if (_parameters.GetAnalysisType() == PROSPECTIVESPACETIME)
      _psp_cluster_data.reset(_cluster_data_factory->GetNewProspectiveSpatialClusterData(_data_hub, DataGateway));
    else
      _ps_cluster_data.reset(_cluster_data_factory->GetNewSpatialClusterData(DataGateway));
  }
  catch (prg_exception& x) {
    x.addTrace("AllocateSimulationObjects()","C_ST_PS_Analysis");
    throw;
  }
}

/** Allocates objects used during calculation of most likely clusters, instead
    of repeated allocations for each simulation.                                */
void C_ST_PS_Analysis::AllocateTopClustersObjects(const AbstractDataSetGateway & DataGateway) {
    try {
        //create top cluster objects for space-time portion
        CSpaceTimeAnalysis::AllocateTopClustersObjects(DataGateway);
        //create comparator cluster for purely spatial cluster
        _ps_cluster_compare.reset(new CPurelySpatialCluster(_cluster_data_factory, DataGateway));
        _ps_top_clusters.setTopClusters(*_ps_cluster_compare);
    } catch (prg_exception& x) {
        x.addTrace("AllocateTopClustersObjects()","C_ST_PS_Analysis");
        throw;
    }
}

/** Returns cluster centered at grid point nCenter, with the greatest log likelihood
    ratio. Caller should not assume that returned reference is persistent, but should
    either call Clone() method or overloaded assignment operator. */
const SharedClusterVector_t C_ST_PS_Analysis::CalculateTopClusters(tract_t tCenter, const AbstractDataSetGateway & DataGateway) {
    //re-initialize clusters
    _top_clusters.resetAboutCentroid(tCenter);
    if (_ps_cluster_compare.get()) _ps_top_clusters.resetAboutCentroid(tCenter);
    if (_psp_cluster_compare.get()) _psp_top_clusters.resetAboutCentroid(tCenter);
    _time_intervals->setIntervalRange(tCenter);
    for (int j=0 ;j <= _parameters.GetNumTotalEllipses(); ++j) {
        CentroidNeighbors CentroidDef(j, _data_hub, tCenter);
        _top_clusters.resetNeighborCounts(j);
        if (_ps_cluster_compare.get()) _ps_top_clusters.resetNeighborCounts(j);
        if (_psp_cluster_compare.get()) _psp_top_clusters.resetNeighborCounts(j);
        _cluster_compare->Initialize(tCenter);
        _cluster_compare->SetEllipseOffset(j, _data_hub);
        if (_ps_cluster_compare.get()) {
            _ps_cluster_compare->Initialize(tCenter);
            _ps_cluster_compare->SetEllipseOffset(j, _data_hub);
            _ps_cluster_compare->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef, _ps_top_clusters.getClusterSet(j), *_likelihood_calculator);
        } else {
            _psp_cluster_compare->Initialize(tCenter);
            _psp_cluster_compare->SetEllipseOffset(j, _data_hub);
            _psp_cluster_compare->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef, _psp_top_clusters.getClusterSet(j), *_likelihood_calculator);
        }
        _cluster_compare->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef, _top_clusters.getClusterSet(j), *_time_intervals);
    }
    SharedClusterVector_t stClusters, psClusters;
    _top_clusters.getTopClusters(stClusters);
    if (_ps_cluster_compare.get()) _ps_top_clusters.getTopClusters(psClusters);
    if (_psp_cluster_compare.get()) _psp_top_clusters.getTopClusters(psClusters);
    if (!psClusters.front()->ClusterDefined()) return stClusters; // Skip if purely spatial cluster is not defined - no gini since space-time.
    if (!stClusters.size() || !stClusters.front()->ClusterDefined()) return psClusters; // no space-time clusters, only purely spatial
    // Figure out whether the space-time cluster(s) or purely spatial cluster is the best for this centroid.
    // If this is a retrospective space-time analysis, there could be more than one space-time cluster, with greatest first in list.
    return macro_less_than(stClusters.front()->m_nRatio, psClusters.front()->m_nRatio, DBL_CMP_TOLERANCE) ? psClusters : stClusters;
}

/** Returns loglikelihood for Monte Carlo replication. */
double C_ST_PS_Analysis::MonteCarlo(const DataSetInterface & Interface) {
  if (_parameters.GetAnalysisType() == PROSPECTIVESPACETIME)
    return MonteCarloProspective(Interface);

  tract_t               i, k;
  SpatialData         * pSpatialData = dynamic_cast<SpatialData*>(_ps_cluster_data.get());
  SpaceTimeData       * pSpaceTimeData = dynamic_cast<SpaceTimeData*>(_cluster_data.get());

  _measure_list->Reset();
  //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
  for (k=0; k <= _parameters.GetNumTotalEllipses(); ++k) {
     CentroidNeighbors CentroidDef(k, _data_hub);
     for (i=0; i < _data_hub.m_nGridTracts; ++i) {
        CentroidDef.Set(i);
        _time_intervals->setIntervalRange(i);
        pSpatialData->AddMeasureList(CentroidDef, Interface, _measure_list.get());
        pSpaceTimeData->AddNeighborDataAndCompare(CentroidDef, Interface, *_time_intervals, *_measure_list);
     }
     _measure_list->SetForNextIteration(k);
  }
  return _measure_list->GetMaximumLogLikelihoodRatio();
}

/** Returns cluster centered at grid point nCenter, with the greatest log
    likelihood ratio . Caller should not assume that returned reference is
    persistent, but should either call Clone() method or overloaded assignment
    operator. */
double C_ST_PS_Analysis::MonteCarlo(tract_t tCenter, const AbstractDataSetGateway& DataGateway) {
  if (_parameters.GetAnalysisType() == PROSPECTIVESPACETIME)
    return MonteCarloProspective(tCenter, DataGateway);
    
  tract_t                       t, tNumNeighbors, * pIntegerArray;
  unsigned short              * pUnsignedShortArray;
  double                        dMaximizingValue;
  std::vector<double>           vMaximizingValues(_parameters.GetNumTotalEllipses() + 1, -std::numeric_limits<double>::max());
  std::vector<double>::iterator itr, itr_end;

  _time_intervals->setIntervalRange(tCenter);
  for (int j=0; j <= _parameters.GetNumTotalEllipses(); ++j) {
     double& dShapeMaxValue = vMaximizingValues[j];
     _ps_cluster_data->InitializeData();
     _cluster_data->InitializeData();
     CentroidNeighbors CentroidDef(j, _data_hub);
     CentroidDef.Set(tCenter);
     tNumNeighbors = CentroidDef.GetNumNeighbors();                     
     pUnsignedShortArray = CentroidDef.GetRawUnsignedShortArray();
     pIntegerArray = CentroidDef.GetRawIntegerArray();
     for (t=0; t < tNumNeighbors; ++t) {
        _ps_cluster_data->AddNeighborData((pUnsignedShortArray ? (tract_t)pUnsignedShortArray[t] : pIntegerArray[t]), DataGateway);
        dMaximizingValue = _ps_cluster_data->GetMaximizingValue(*_likelihood_calculator);
        if (dMaximizingValue > dShapeMaxValue) dShapeMaxValue = dMaximizingValue;
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

/** Returns loglikelihood for Monte Carlo Prospective replication. */
double C_ST_PS_Analysis::MonteCarloProspective(const DataSetInterface & Interface) {
  tract_t                  k, i;
  ProspectiveSpatialData * pPSSpatialData = dynamic_cast<ProspectiveSpatialData*>(_psp_cluster_data.get());
  SpaceTimeData          * pSpaceTimeData = dynamic_cast<SpaceTimeData*>(_cluster_data.get());

  _measure_list->Reset();
  //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
  for (k=0; k <= _parameters.GetNumTotalEllipses(); ++k) {
     CentroidNeighbors CentroidDef(k, _data_hub);
     for (i=0; i < _data_hub.m_nGridTracts; ++i) {
        CentroidDef.Set(i);
        _time_intervals->setIntervalRange(i);
        pPSSpatialData->AddMeasureList(CentroidDef, Interface, _measure_list.get());
        pSpaceTimeData->AddNeighborDataAndCompare(CentroidDef, Interface, *_time_intervals, *_measure_list);
     }
     _measure_list->SetForNextIteration(k);
  }
  return _measure_list->GetMaximumLogLikelihoodRatio();
}

/** Returns cluster centered at grid point nCenter, with the greatest log
    likelihood ratio . Caller should not assume that returned reference is
    persistent, but should either call Clone() method or overloaded assignment
    operator. */
double C_ST_PS_Analysis::MonteCarloProspective(tract_t tCenter, const AbstractDataSetGateway& DataGateway) {
  tract_t                       t, tNumNeighbors, * pIntegerArray;
  unsigned short              * pUnsignedShortArray;
  double                        dMaximizingValue;
  std::vector<double>           vMaximizingValues(_parameters.GetNumTotalEllipses() + 1, -std::numeric_limits<double>::max());
  std::vector<double>::iterator itr, itr_end;
  AbstractProspectiveSpatialClusterData * pPSPSpatialData = dynamic_cast<AbstractProspectiveSpatialClusterData*>(_psp_cluster_data.get());

  _time_intervals->setIntervalRange(tCenter);
  for (int j=0; j <= _parameters.GetNumTotalEllipses(); ++j) {
     double& dShapeMaxValue = vMaximizingValues[j];
     _psp_cluster_data->InitializeData();
     _cluster_data->InitializeData();
     CentroidNeighbors CentroidDef(j, _data_hub);
     CentroidDef.Set(tCenter);
     tNumNeighbors = CentroidDef.GetNumNeighbors();
     pUnsignedShortArray = CentroidDef.GetRawUnsignedShortArray();
     pIntegerArray = CentroidDef.GetRawIntegerArray();
     for (t=0; t < tNumNeighbors; ++t) {
        _psp_cluster_data->AddNeighborData((pUnsignedShortArray ? (tract_t)pUnsignedShortArray[t] : pIntegerArray[t]), DataGateway);
        dMaximizingValue = pPSPSpatialData->GetMaximizingValue(*_likelihood_calculator);
        if (dMaximizingValue > dShapeMaxValue) dShapeMaxValue = dMaximizingValue;
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


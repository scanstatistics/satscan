//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "SpaceTimeIncludePureAnalysis.h"
#include "ClusterData.h"
#include "MostLikelyClustersContainer.h"
#include "SSException.h"

/** Constructor */
C_ST_PS_PT_Analysis::C_ST_PS_PT_Analysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
                    :C_ST_PS_Analysis(Parameters, DataHub, PrintDirection) {}

/** Allocates objects used during simulations, instead of repeated allocations
    for each simulation. Which objects that are allocated depends on whether
    the simluations process uses same process as real data or uses measure list. */
void C_ST_PS_PT_Analysis::AllocateSimulationObjects(const AbstractDataSetGateway& DataGateway) {
    try {
        //allocate objects for space-time part of simulations
        C_ST_PS_Analysis::AllocateSimulationObjects(DataGateway);
        _pt_cluster_data.reset(_cluster_data_factory->GetNewTemporalClusterData(DataGateway));
    } catch (prg_exception& x) {
        x.addTrace("AllocateSimulationObjects()","C_ST_PS_PT_Analysis");
        throw;
    }
}

/** Calculates most likely cluster(s) for space-time and purely temporal scans. */
void C_ST_PS_PT_Analysis::FindTopClusters(const AbstractDataSetGateway& DataGateway, MLC_Collections_t& TopClustersContainers) {
    // Calculate top cluster over all space-time 
    C_ST_PS_Analysis::FindTopClusters(DataGateway, TopClustersContainers);
    if (_print.GetIsCanceled()) return; // Detect user cancellation before continuing.
    // Now calculate top purely temporal cluster(s).
    //create cluster objects
    CPurelyTemporalCluster ClusterComparator(
        _cluster_data_factory.get(), DataGateway,
        _parameters.GetAnalysisType() == PROSPECTIVESPACETIME ? ALIVECLUSTERS : _parameters.GetIncludeClustersType(), _data_hub
    );
    boost::shared_ptr<CClusterSet> clusterSet(CClusterSet::getNewCClusterSetObject(ClusterComparator, _data_hub));
    CClusterObject clusterObject(ClusterComparator);
    clusterSet->add(clusterObject);
    //iterate through time intervals - looking for top purely temporal cluster
    _time_intervals->resetIntervalRange();
    _time_intervals->CompareClusterSet(ClusterComparator, *clusterSet);
    // If there is a most likely cluster, possibly search for other clusters then add to MLC container.
    if (clusterSet->getSet().front().getCluster().ClusterDefined()) {
        CClusterSet::ClusterContainer_t clusters;
        clusters.push_back(clusterSet->getSet().front()); // Add MLC to the collection of purely temporal clusters.
        // Search for other purely temporal clusters that don't overlap temporally with the MLC, or each other.
        if (_parameters.GetAnalysisType() == SPACETIME && !_parameters.GetIsIterativeScanning()) // retrospective space-time only
            CClusterSetTemporalOverlap::findOtherNotTemporalOverlapping(clusters, *clusterSet, _data_hub);
        assert(TopClustersContainers.size() == 1);
        for (auto& clusterObj : clusters)
            TopClustersContainers.front().Add(clusterObj.getCluster());
    }
}

/** Returns loglikelihood for Monte Carlo replication. */
double C_ST_PS_PT_Analysis::MonteCarlo(const DataSetInterface & Interface) {
  if (_parameters.GetAnalysisType() == PROSPECTIVESPACETIME)
    return MonteCarloProspective(Interface);

  tract_t               k, i;
  SpatialData         * pSpatialData = dynamic_cast<SpatialData*>(_ps_cluster_data.get());
  SpaceTimeData       * pSpaceTimeData = dynamic_cast<SpaceTimeData*>(_cluster_data.get());

  _measure_list->Reset();
  //Add measure values for purely space first - so that this cluster's values
  //will be calculated with circle's measure values.
  macroRunTimeStartFocused(FocusRunTimeComponent::MeasureListScanningAdding);
  _time_intervals->resetIntervalRange();
  _time_intervals->CompareMeasures(*_pt_cluster_data, *_measure_list);
  macroRunTimeStopFocused(FocusRunTimeComponent::MeasureListScanningAdding);
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

double C_ST_PS_PT_Analysis::MonteCarlo(tract_t tCenter, const AbstractDataSetGateway & DataGateway) {
  if (_parameters.GetAnalysisType() == PROSPECTIVESPACETIME)
    return MonteCarloProspective(tCenter, DataGateway);

  tract_t                       t, tNumNeighbors, * pIntegerArray;
  unsigned short              * pUnsignedShortArray;
  double                        dMaximizingValue;
  std::vector<double>           vMaximizingValues(_parameters.GetNumTotalEllipses() + 1, -std::numeric_limits<double>::max());
  std::vector<double>::iterator itr, itr_end;

  _pt_cluster_data->InitializeData();
  //iterate through time intervals, finding top cluster
  _time_intervals->resetIntervalRange();
  dMaximizingValue = _time_intervals->ComputeMaximizingValue(*_pt_cluster_data);
  if (dMaximizingValue > vMaximizingValues[0]) vMaximizingValues[0] = dMaximizingValue;

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
double C_ST_PS_PT_Analysis::MonteCarloProspective(const DataSetInterface & Interface) {
  tract_t                  k, i;
  ProspectiveSpatialData * pPSSpatialData = dynamic_cast<ProspectiveSpatialData*>(_psp_cluster_data.get());
  SpaceTimeData          * pSpaceTimeData = dynamic_cast<SpaceTimeData*>(_cluster_data.get());

  _measure_list->Reset();
  //Add measure values for purely space first - so that this cluster's values
  //will be calculated with circle's measure values.
  macroRunTimeStartFocused(FocusRunTimeComponent::MeasureListScanningAdding);
  _time_intervals->resetIntervalRange();
  _time_intervals->CompareMeasures(*_pt_cluster_data, *_measure_list);
  macroRunTimeStopFocused(FocusRunTimeComponent::MeasureListScanningAdding);
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

double C_ST_PS_PT_Analysis::MonteCarloProspective(tract_t tCenter, const AbstractDataSetGateway & DataGateway) {
  tract_t                       t, tNumNeighbors, * pIntegerArray;
  unsigned short              * pUnsignedShortArray;
  double                        dMaximizingValue;
  std::vector<double>           vMaximizingValues(_parameters.GetNumTotalEllipses() + 1, -std::numeric_limits<double>::max());
  std::vector<double>::iterator itr, itr_end;
  AbstractProspectiveSpatialClusterData * pPSPSpatialData = dynamic_cast<AbstractProspectiveSpatialClusterData*>(_psp_cluster_data.get());

  _pt_cluster_data->InitializeData();
  //iterate through time intervals, finding top cluster
  _time_intervals->resetIntervalRange();
  dMaximizingValue = _time_intervals->ComputeMaximizingValue(*_pt_cluster_data);
  if (dMaximizingValue > vMaximizingValues[0]) vMaximizingValues[0] = dMaximizingValue;  

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

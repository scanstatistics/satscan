//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SpaceTimeIncludePurelyTemporalAnalysis.h"
#include "ClusterData.h"
#include "MostLikelyClustersContainer.h"
#include "SSException.h"

/** Constructor */
C_ST_PT_Analysis::C_ST_PT_Analysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
                 :CSpaceTimeAnalysis(Parameters, DataHub, PrintDirection) {}

/** Allocates objects used during simulations, instead of repeated allocations
    for each simulation. Which objects that are allocated depends on whether
    the simluations process uses same process as real data or uses measure list. */
void C_ST_PT_Analysis::AllocateSimulationObjects(const AbstractDataSetGateway & DataGateway) {
    try {
        //allocate objects for space-time part of simulations
        CSpaceTimeAnalysis::AllocateSimulationObjects(DataGateway);
        //create simulation objects based upon which process used to perform simulations
        _pt_cluster_data.reset(_cluster_data_factory->GetNewTemporalClusterData(DataGateway));
    } catch (prg_exception& x) {
        x.addTrace("AllocateSimulationObjects()","C_ST_PT_Analysis");
        throw;
    }
}

/** calculates top cluster about each centroid/grid point - iterating through
    all possible time intervals - populates top cluster array with most likely
    cluster about each grid point plus , possible, one more for purely temporal
    cluster. */
void C_ST_PT_Analysis::FindTopClusters(const AbstractDataSetGateway & DataGateway, MLC_Collections_t& TopClustersContainers) {
    try {
        //calculate top cluster over all space-time
        CSpaceTimeAnalysis::FindTopClusters(DataGateway, TopClustersContainers);
        //detect user cancellation
        if (_print.GetIsCanceled()) return;
        //create comparator cluster
        CPurelyTemporalCluster ClusterComparator(
            _cluster_data_factory.get(), DataGateway,
            _parameters.GetAnalysisType() == PROSPECTIVESPACETIME ? ALIVECLUSTERS : _parameters.GetIncludeClustersType(), _data_hub
        );
        boost::shared_ptr<CClusterSet> clusterSet(CClusterSet::getNewCClusterSetObject(ClusterComparator, _data_hub));
        CClusterObject clusterObject(ClusterComparator);
        clusterSet->add(clusterObject);
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
    } catch (prg_exception& x) {
        x.addTrace("FindTopClusters()","C_ST_PT_Analysis");
        throw;
    }
}

/** Returns loglikelihood ratio for Monte Carlo replication using same algorithm as real data. */
double C_ST_PT_Analysis::MonteCarlo(tract_t tCenter, const AbstractDataSetGateway & DataGateway) {
  tract_t                       t, tNumNeighbors, * pIntegerArray;
  unsigned short              * pUnsignedShortArray;
  double                        dMaximizingValue;
  std::vector<double>           vMaximizingValues(_parameters.GetNumTotalEllipses() + 1, -std::numeric_limits<double>::max());
  std::vector<double>::iterator itr, itr_end;

  _pt_cluster_data->InitializeData();
  //iterate through time intervals, finding top maximizing value
  _time_intervals->resetIntervalRange();
  dMaximizingValue = _time_intervals->ComputeMaximizingValue(*_pt_cluster_data);
  if (dMaximizingValue > vMaximizingValues[0]) vMaximizingValues[0] = dMaximizingValue;

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

/** Returns loglikelihood for Monte Carlo replication. */
double C_ST_PT_Analysis::MonteCarlo(const DataSetInterface & Interface) {
  tract_t               k, i;
  SpaceTimeData       * pSpaceTimeData = dynamic_cast<SpaceTimeData*>(_cluster_data.get());

  _measure_list->Reset();
  //compare purely temporal cluster in same ratio correction as circle
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
        pSpaceTimeData->AddNeighborDataAndCompare(CentroidDef, Interface, *_time_intervals, *_measure_list);
     }
     _measure_list->SetForNextIteration(k);
  }
  return _measure_list->GetMaximumLogLikelihoodRatio();
}


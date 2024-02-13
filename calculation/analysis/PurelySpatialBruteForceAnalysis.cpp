//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "PurelySpatialBruteForceAnalysis.h"
#include "HomogeneousPoissonRandomizer.h"

#include "ClusterData.h"
#include "SSException.h"

/** Constructor */
CPurelySpatialBruteForceAnalysis::CPurelySpatialBruteForceAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
                       :AbstractBruteForceAnalysis(Parameters, DataHub, PrintDirection), _top_clusters(DataHub) {}

/** Allocates objects used during Monte Carlo simulations instead of repeated
    allocations for each simulation. This method must be called prior to MonteCarlo(). */
void CPurelySpatialBruteForceAnalysis::AllocateSimulationObjects(const AbstractDataSetGateway& DataGateway) {
  try {
    //create simulation objects based upon which process used to perform simulations
    if (_replica_process_type == MeasureListEvaluation)
      _measure_list.reset(GetNewMeasureListObject());
    _cluster_data.reset(new SpatialHomogeneousData(DataGateway));
  }
  catch (prg_exception& x) {
    x.addTrace("AllocateSimulationObjects()","CPurelySpatialBruteForceAnalysis");
    throw;
  }
}

/** Allocates objects used during calculation of most likely clusters, instead
    of repeated allocations. This method must be called prior to CalculateTopCluster(). */
void CPurelySpatialBruteForceAnalysis::AllocateTopClustersObjects(const AbstractDataSetGateway& DataGateway) {
  try {
    _cluster_compare.reset(new PurelySpatialHomogeneousPoissonCluster(DataGateway));
    _top_clusters.setTopClusters(*_cluster_compare);
    _centroid_calculator.reset(new CentroidNeighborCalculator(_data_hub, _print));
  }
  catch (prg_exception& x) {
    x.addTrace("AllocateTopClustersObjects()","CPurelySpatialBruteForceAnalysis");
    throw;
  }
}

/** Returns cluster centered at grid point nCenter, with the greatest loglikelihood ratio.
    Caller should not assume that returned reference is persistent, but should either call
    Clone() method or overloaded assignment operator. */
const SharedClusterVector_t CPurelySpatialBruteForceAnalysis::CalculateTopClusters(tract_t tCenter, const AbstractDataSetGateway& DataGateway) {
  _top_clusters.resetAboutCentroid(tCenter);
  _neighbor_info.killAll();
  const CentroidNeighborCalculator::DistanceContainer_t& locDist = _centroid_calculator->getLocationDistances();
  for (int j=0; j <= _parameters.GetNumTotalEllipses(); ++j) {
     _neighbor_info.push_back(new CentroidNeighbors());
     _centroid_calculator->CalculateNeighborsAboutCentroid(j, tCenter, *_neighbor_info.back());
     _top_clusters.resetNeighborCounts(*_neighbor_info.back());
     _cluster_compare->Initialize(tCenter);
     _cluster_compare->SetEllipseOffset(j, _data_hub);
     _cluster_compare->CalculateTopClusterAboutCentroidDefinition(DataGateway, *_neighbor_info.back(), locDist, _top_clusters.getClusterSet(j), *_likelihood_calculator);
  }
  SharedClusterVector_t topClusters;
  return _top_clusters.getTopClusters(topClusters);
}

/** Returns loglikelihood ratio for Monte Carlo replication using same algorithm as real data. */
double CPurelySpatialBruteForceAnalysis::MonteCarlo(tract_t tCenter, const AbstractDataSetGateway & DataGateway) {
    return MonteCarlo(DataGateway.GetDataSetInterface(0));
}

/** Returns loglikelihood ratio for Monte Carlo replication utilizing measure list structure. */
double CPurelySpatialBruteForceAnalysis::MonteCarlo(const DataSetInterface& Interface) {
  SpatialHomogeneousData * pSpatialData = _cluster_data.get();

  _measure_list->Reset();
  const CentroidNeighborCalculator::DistanceContainer_t& locDist = _centroid_calculator->getLocationDistances();
  for (tract_t k=0; k <= _parameters.GetNumTotalEllipses(); ++k) {
     for (tract_t i=0; i < _data_hub.m_nGridTracts; ++i) {
        CentroidNeighbors centroidDef;
        _centroid_calculator->CalculateNeighborsAboutCentroid(k, i, centroidDef);
        centroidDef.SetMaximumClusterSize_SimulatedData();
        pSpatialData->AddMeasureList(centroidDef, locDist,Interface, _measure_list.get());
     }
     _measure_list->SetForNextIteration(k);
  }
  return _measure_list->GetMaximumLogLikelihoodRatio();
}

/** Allocates additional objects used during randomization. */
void CPurelySpatialBruteForceAnalysis::AllocateAdditionalSimulationObjects(RandomizerContainer_t& Container) {
   try {
     //Ensure that only one randomizer exists in container -- only one data set implemented for H. Poisson.
     if (Container.size() != 1)
        throw prg_error("Expecting exactly one randomizer.","AllocateAdditionalSimulationObjects()");

     //Cast container object to expected HomogeneousPoissonRandomizer.
     HomogeneousPoissonRandomizer * pRandomizer = dynamic_cast<HomogeneousPoissonRandomizer*>(*Container.begin());
     if (!pRandomizer)
        throw prg_error("Randomizer cound not be cast to HomogeneousPoissonRandomizer type.","AllocateAdditionalSimulationObjects()");

     //If using grid file, get handler from data hub; else get separate object held by randomizer.
     const GInfo& ginfo = (_parameters.UseSpecialGrid() ? *_data_hub.GetGInfo() : pRandomizer->getCentroidHandler());
     //Allocate CentroidNeighborCalculator object used during randomization.
     _centroid_calculator.reset(new CentroidNeighborCalculator(_data_hub, pRandomizer->getIdentifierInfo(), ginfo, _print));
   }
   catch (prg_exception& x) {
     x.addTrace("AllocateAdditionalSimulationObjects()","CPurelySpatialBruteForceAnalysis");
     throw;
   }
}

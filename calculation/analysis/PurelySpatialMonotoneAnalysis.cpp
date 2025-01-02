//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "PurelySpatialMonotoneAnalysis.h"
#include "SSException.h"

/** constructor */
CPSMonotoneAnalysis::CPSMonotoneAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
                    :CAnalysis(Parameters, DataHub, PrintDirection), _top_clusters(DataHub) {}

/** Allocates objects used during simulations, instead of repeated allocations
    for each simulation.
    NOTE: This analysis has not been optimized to 'pre' allocate objects used in
          simulation process. This function is only a shell.                     */
void CPSMonotoneAnalysis::AllocateSimulationObjects(const AbstractDataSetGateway& DataGateway) {
  _top_clusters.setTopClusters(CPSMonotoneCluster(_cluster_data_factory.get(), DataGateway, _parameters.GetExecuteScanRateType()));
  _compare_cluster.reset(new CPSMonotoneCluster(_cluster_data_factory.get(), DataGateway, _parameters.GetExecuteScanRateType() == HIGHANDLOW ? HIGH : _parameters.GetExecuteScanRateType()));
  if (_parameters.GetExecuteScanRateType() == HIGHANDLOW)
    _aux_compare_cluster.reset(new CPSMonotoneCluster(_cluster_data_factory.get(), DataGateway, LOW));
}

/** Allocates objects used during calculation of most likely clusters, instead
    of repeated allocations for each simulation.
    NOTE: This analysis has not been optimized to 'pre' allocate objects used in
          process of finding most likely clusters. This function is only a shell. */
void CPSMonotoneAnalysis::AllocateTopClustersObjects(const AbstractDataSetGateway& DataGateway) {
  _top_clusters.setTopClusters(CPSMonotoneCluster(_cluster_data_factory.get(), DataGateway, _parameters.GetExecuteScanRateType()));
  _compare_cluster.reset(new CPSMonotoneCluster(_cluster_data_factory.get(), DataGateway, _parameters.GetExecuteScanRateType() == HIGHANDLOW ? HIGH : _parameters.GetExecuteScanRateType()));
  if (_parameters.GetExecuteScanRateType() == HIGHANDLOW)
    _aux_compare_cluster.reset(new CPSMonotoneCluster(_cluster_data_factory.get(), DataGateway, LOW));
}

/** Returns cluster centered at grid point nCenter, with the greatest loglikelihood ratio.
    Caller should not assume that returned reference is persistent, but should either call
    Clone() method or overloaded assignment operator. */
const SharedClusterVector_t CPSMonotoneAnalysis::CalculateTopClusters(tract_t tCenter, const AbstractDataSetGateway& DataGateway) {
  try {
    _top_clusters.resetAboutCentroid(tCenter);
    CentroidNeighbors CentroidDef(0, _data_hub, tCenter);
    _top_clusters.resetNeighborCounts(0);
    if (_parameters.GetExecuteScanRateType() == HIGHANDLOW) {
      _compare_cluster->AllocateForMaxCircles(_data_hub.GetNeighborCountArray()[0][tCenter]+1);
      _compare_cluster->Initialize(tCenter);
      _compare_cluster->SetCenter(tCenter);
      _compare_cluster->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef, _top_clusters.getClusterSet(0), *_likelihood_calculator);

      _aux_compare_cluster->AllocateForMaxCircles(_data_hub.GetNeighborCountArray()[0][tCenter]+1);
      _aux_compare_cluster->Initialize(tCenter);
      _aux_compare_cluster->SetCenter(tCenter);
      _aux_compare_cluster->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef, _top_clusters.getClusterSet(0), *_likelihood_calculator);
    } else {
      _compare_cluster->Initialize(tCenter);
      _compare_cluster->SetCenter(tCenter);
      _compare_cluster->AllocateForMaxCircles(_data_hub.GetNeighborCountArray()[0][tCenter]+1);
      _compare_cluster->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef, _top_clusters.getClusterSet(0), *_likelihood_calculator);
    }
    SharedClusterVector_t topClusters;
    return _top_clusters.getTopClusters(topClusters);
  }
  catch (prg_exception& x) {
    x.addTrace("CalculateTopCluster()","CPSMonotoneAnalysis");
    throw;
  }
}

/** Not implemented in this analysis class. */
double CPSMonotoneAnalysis::MonteCarlo(const DataSetInterface& Interface) {
  throw prg_error("MonteCarlo(const DataSetInterface&) not implemented.","CPSMonotoneAnalysis");
}

/** Returns calculates log likelihood ratio about centroid. Currently this function calls CalculateTopCluster()
    but will likely be updated in the future. */
double CPSMonotoneAnalysis::MonteCarlo(tract_t tCenter, const AbstractDataSetGateway & DataGateway) {
  //TODO: refactor for index based cluster reporting
  return CalculateTopClusters(tCenter, DataGateway).back()->m_nRatio;
}


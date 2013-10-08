//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "PurelySpatialMonotoneAnalysis.h"
#include "SSException.h"

/** constructor */
CPSMonotoneAnalysis::CPSMonotoneAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
                    :CAnalysis(Parameters, DataHub, PrintDirection), _topClusters(DataHub) {}

/** destructor */
CPSMonotoneAnalysis::~CPSMonotoneAnalysis() {}

/** Allocates objects used during simulations, instead of repeated allocations
    for each simulation.
    NOTE: This analysis has not been optimized to 'pre' allocate objects used in
          simulation process. This function is only a shell.                     */
void CPSMonotoneAnalysis::AllocateSimulationObjects(const AbstractDataSetGateway& DataGateway) {
  _topClusters.setTopClusters(CPSMonotoneCluster(gpClusterDataFactory, DataGateway, gParameters.GetExecuteScanRateType()));
  gComparatorCluster.reset(new CPSMonotoneCluster(gpClusterDataFactory, DataGateway, gParameters.GetExecuteScanRateType() == HIGHANDLOW ? HIGH : gParameters.GetExecuteScanRateType()));
  if (gParameters.GetExecuteScanRateType() == HIGHANDLOW)
    gAuxComparatorCluster.reset(new CPSMonotoneCluster(gpClusterDataFactory, DataGateway, LOW));
}

/** Allocates objects used during calculation of most likely clusters, instead
    of repeated allocations for each simulation.
    NOTE: This analysis has not been optimized to 'pre' allocate objects used in
          process of finding most likely clusters. This function is only a shell. */
void CPSMonotoneAnalysis::AllocateTopClustersObjects(const AbstractDataSetGateway& DataGateway) {
  _topClusters.setTopClusters(CPSMonotoneCluster(gpClusterDataFactory, DataGateway, gParameters.GetExecuteScanRateType()));
  gComparatorCluster.reset(new CPSMonotoneCluster(gpClusterDataFactory, DataGateway, gParameters.GetExecuteScanRateType() == HIGHANDLOW ? HIGH : gParameters.GetExecuteScanRateType()));
  if (gParameters.GetExecuteScanRateType() == HIGHANDLOW)
    gAuxComparatorCluster.reset(new CPSMonotoneCluster(gpClusterDataFactory, DataGateway, LOW));
}

/** Returns cluster centered at grid point nCenter, with the greatest loglikelihood ratio.
    Caller should not assume that returned reference is persistent, but should either call
    Clone() method or overloaded assignment operator. */
const SharedClusterVector_t CPSMonotoneAnalysis::CalculateTopClusters(tract_t tCenter, const AbstractDataSetGateway& DataGateway) {
  try {
    _topClusters.reset(tCenter);
    CentroidNeighbors CentroidDef(0, gDataHub, tCenter);
    _topClusters.resetNeighborCounts(0);
    if (gParameters.GetExecuteScanRateType() == HIGHANDLOW) {
      gComparatorCluster->AllocateForMaxCircles(gDataHub.GetNeighborCountArray()[0][tCenter]+1);
      gComparatorCluster->Initialize(tCenter);
      gComparatorCluster->SetCenter(tCenter);
      gComparatorCluster->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef, _topClusters.getClusterSet(0), *gpLikelihoodCalculator);

      gAuxComparatorCluster->AllocateForMaxCircles(gDataHub.GetNeighborCountArray()[0][tCenter]+1);
      gAuxComparatorCluster->Initialize(tCenter);
      gAuxComparatorCluster->SetCenter(tCenter);
      gAuxComparatorCluster->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef, _topClusters.getClusterSet(0), *gpLikelihoodCalculator);
    }
    else {
      gComparatorCluster->Initialize(tCenter);
      gComparatorCluster->SetCenter(tCenter);
      gComparatorCluster->AllocateForMaxCircles(gDataHub.GetNeighborCountArray()[0][tCenter]+1);
      gComparatorCluster->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef, _topClusters.getClusterSet(0), *gpLikelihoodCalculator);
    }
    SharedClusterVector_t topClusters;
    return _topClusters.getTopClusters(topClusters);
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


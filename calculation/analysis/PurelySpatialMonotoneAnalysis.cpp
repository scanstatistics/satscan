//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "PurelySpatialMonotoneAnalysis.h"
#include "SSException.h"

/** constructor */
CPSMonotoneAnalysis::CPSMonotoneAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
                    :CAnalysis(Parameters, DataHub, PrintDirection) {}

/** destructor */
CPSMonotoneAnalysis::~CPSMonotoneAnalysis() {}

/** Allocates objects used during simulations, instead of repeated allocations
    for each simulation.
    NOTE: This analysis has not been optimized to 'pre' allocate objects used in
          simulation process. This function is only a shell.                     */
void CPSMonotoneAnalysis::AllocateSimulationObjects(const AbstractDataSetGateway& DataGateway) {
  gMaxCluster.reset(new CPSMonotoneCluster(gpClusterDataFactory, DataGateway, gParameters.GetExecuteScanRateType()));
  gComparatorCluster.reset(new CPSMonotoneCluster(gpClusterDataFactory, DataGateway, gParameters.GetExecuteScanRateType() == HIGHANDLOW ? HIGH : gParameters.GetExecuteScanRateType()));
  if (gParameters.GetExecuteScanRateType() == HIGHANDLOW)
    gAuxComparatorCluster.reset(new CPSMonotoneCluster(gpClusterDataFactory, DataGateway, LOW));
}

/** Allocates objects used during calculation of most likely clusters, instead
    of repeated allocations for each simulation.
    NOTE: This analysis has not been optimized to 'pre' allocate objects used in
          process of finding most likely clusters. This function is only a shell. */
void CPSMonotoneAnalysis::AllocateTopClustersObjects(const AbstractDataSetGateway& DataGateway) {
  gMaxCluster.reset(new CPSMonotoneCluster(gpClusterDataFactory, DataGateway, gParameters.GetExecuteScanRateType()));
  gComparatorCluster.reset(new CPSMonotoneCluster(gpClusterDataFactory, DataGateway, gParameters.GetExecuteScanRateType() == HIGHANDLOW ? HIGH : gParameters.GetExecuteScanRateType()));
  if (gParameters.GetExecuteScanRateType() == HIGHANDLOW)
    gAuxComparatorCluster.reset(new CPSMonotoneCluster(gpClusterDataFactory, DataGateway, LOW));
}

/** Returns cluster centered at grid point nCenter, with the greatest loglikelihood ratio.
    Caller should not assume that returned reference is persistent, but should either call
    Clone() method or overloaded assignment operator. */
const CCluster & CPSMonotoneAnalysis::CalculateTopCluster(tract_t tCenter, const AbstractDataSetGateway& DataGateway) {
  try {
    gMaxCluster->AllocateForMaxCircles(gDataHub.GetNeighborCountArray()[0][tCenter]+1);
    gMaxCluster->Initialize(tCenter);
    CentroidNeighbors CentroidDef(0, gDataHub);
    CentroidDef.Set(tCenter);
    if (gParameters.GetExecuteScanRateType() == HIGHANDLOW) {
      gComparatorCluster->AllocateForMaxCircles(gDataHub.GetNeighborCountArray()[0][tCenter]+1);
      gComparatorCluster->Initialize(tCenter);
      gComparatorCluster->SetCenter(tCenter);
      gComparatorCluster->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef, *gMaxCluster, *gpLikelihoodCalculator);

      gAuxComparatorCluster->AllocateForMaxCircles(gDataHub.GetNeighborCountArray()[0][tCenter]+1);
      gAuxComparatorCluster->Initialize(tCenter);
      gAuxComparatorCluster->SetCenter(tCenter);
      gAuxComparatorCluster->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef, *gMaxCluster, *gpLikelihoodCalculator);
    }
    else {
      gComparatorCluster->Initialize(tCenter);
      gComparatorCluster->SetCenter(tCenter);
      gComparatorCluster->AllocateForMaxCircles(gDataHub.GetNeighborCountArray()[0][tCenter]+1);
      gComparatorCluster->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef, *gMaxCluster, *gpLikelihoodCalculator);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("CalculateTopCluster()","CPSMonotoneAnalysis");
    throw;
  }
  return *gMaxCluster;
}

/** Not implemented in this analysis class. */
double CPSMonotoneAnalysis::MonteCarlo(const DataSetInterface& Interface) {
  throw prg_error("MonteCarlo(const DataSetInterface&) not implemented.","CPSMonotoneAnalysis");
}

/** Returns calculates log likelihood ratio about centroid. Currently this function calls CalculateTopCluster()
    but will likely be updated in the future. */
double CPSMonotoneAnalysis::MonteCarlo(tract_t tCenter, const AbstractDataSetGateway & DataGateway) {
  return CalculateTopCluster(tCenter, DataGateway).m_nRatio;
}


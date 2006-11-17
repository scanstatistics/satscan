//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SVTTAnalysis.h"
#include "SSException.h"

/** constructor */
CSpatialVarTempTrendAnalysis::CSpatialVarTempTrendAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
                             :CAnalysis(Parameters, DataHub, PrintDirection), gTopClusters(DataHub) {
}

/** destructor */
CSpatialVarTempTrendAnalysis::~CSpatialVarTempTrendAnalysis() {}

/** Allocates objects used during simulations, instead of repeated allocations
    for each simulation.
    NOTE: This analysis has not been optimized to 'pre' allocate objects used in
          simulation process. This function is only a shell.                     */
void CSpatialVarTempTrendAnalysis::AllocateSimulationObjects(const AbstractDataSetGateway & DataGateway) {
  gClusterData.reset(new SVTTClusterData(gDataHub.GetNumTimeIntervals()));
}

/** Allocates objects used during calculation of most likely clusters, instead
    of repeated allocations for each simulation.
    NOTE: This analysis has not been optimized to 'pre' allocate objects used in
          process of finding most likely clusters. */
void CSpatialVarTempTrendAnalysis::AllocateTopClustersObjects(const AbstractDataSetGateway & DataGateway) {
  try {
    gClusterComparator.reset(new CSVTTCluster(gpClusterDataFactory, DataGateway));
    gTopClusters.SetTopClusters(*gClusterComparator);
  }
  catch (prg_exception& x) {
    x.addTrace("AllocateTopClustersObjects()","CSpatialVarTempTrendAnalysis");
    throw;
  }
}

/** calculates most likely cluster about central location 'tCenter' */
const CCluster & CSpatialVarTempTrendAnalysis::CalculateTopCluster(tract_t tCenter, const AbstractDataSetGateway & DataGateway) {
  try {
    gClusterComparator->InitializeSVTT(0, DataGateway);
    gTopClusters.SetTopClusters(*gClusterComparator);
    //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
    for (int k=0; k <= gParameters.GetNumTotalEllipses(); ++k) {
       CentroidNeighbors CentroidDef(k, gDataHub);
       CentroidDef.Set(tCenter);
       gClusterComparator->InitializeSVTT(tCenter, DataGateway);
       gClusterComparator->SetEllipseOffset(k, gDataHub);
       gClusterComparator->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef,
                                                                      gTopClusters.GetTopCluster(k),
                                                                      *gpLikelihoodCalculator);
    }
    CSVTTCluster& TopCluster = gTopClusters.GetTopCluster();
    if (gTopClusters.GetTopCluster().ClusterDefined())
      TopCluster.SetTimeTrend(gParameters.GetTimeAggregationUnitsType(), gParameters.GetTimeAggregationLength());
    return TopCluster;
  }
  catch (prg_exception& x) {
    x.addTrace("CalculateTopCluster()","CSpatialVarTempTrendAnalysis");
    throw;
  }
}

/** calculates loglikelihood ratio for simulated data pointed to by DataSetInterface
    in a retrospective manner */
double CSpatialVarTempTrendAnalysis::MonteCarlo(const DataSetInterface& Interface) {
  throw prg_error("MonteCarlo(const DataSetInterface&) not implemented.","CSpatialVarTempTrendAnalysis");
}

/** Returns calculates log likelihood ratio about centroid. Currently this function calls CalculateTopCluster()
    but will likely be updated in the future when this analysis type is made public. */
double CSpatialVarTempTrendAnalysis::MonteCarlo(tract_t tCenter, const AbstractDataSetGateway & DataGateway) {
  double                        dMaximumLogLikelihoodRatio;
  tract_t                       t, tNumNeighbors, * pIntegerArray;
  unsigned short              * pUnsignedShortArray;
  std::vector<double>           vMaximizingValues(gParameters.GetNumTotalEllipses() + 1, 0);
  std::vector<double>::iterator itr, itr_end;

  try {
    //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
    for (int k=0; k <= gParameters.GetNumTotalEllipses(); k++) {
       double& dShapeMaxValue = vMaximizingValues[k];
       CentroidNeighbors CentroidDef(k, gDataHub);
       CentroidDef.Set(tCenter);
       tNumNeighbors = CentroidDef.GetNumNeighbors();
       pUnsignedShortArray = CentroidDef.GetRawUnsignedShortArray();
       pIntegerArray = CentroidDef.GetRawIntegerArray();
       gClusterData->InitializeSVTTData(DataGateway);
       for (tract_t j=0; j < tNumNeighbors; j++) {
          gClusterData->AddNeighborData((pUnsignedShortArray ? (tract_t)pUnsignedShortArray[j] : pIntegerArray[j]), DataGateway);
          dShapeMaxValue = std::max(dShapeMaxValue , gClusterData->CalculateSVTTLoglikelihoodRatio(*gpLikelihoodCalculator, DataGateway));
       }
    }
    //determine which ratio/test statistic is the greatest, be sure to apply compactness correction
    double dPenalty = gDataHub.GetParameters().GetNonCompactnessPenaltyPower();
    dMaximumLogLikelihoodRatio = vMaximizingValues.front() * CalculateNonCompactnessPenalty(gDataHub.GetEllipseShape(0), dPenalty);
    for (t=1,itr=vMaximizingValues.begin()+1,itr_end=vMaximizingValues.end(); itr != itr_end; ++itr, ++t) {
       *itr *= CalculateNonCompactnessPenalty(gDataHub.GetEllipseShape(t), dPenalty);
       dMaximumLogLikelihoodRatio = std::max(*itr, dMaximumLogLikelihoodRatio);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("MonteCarlo()","CSpatialVarTempTrendAnalysis");
    throw;
  }
  return dMaximumLogLikelihoodRatio;
}


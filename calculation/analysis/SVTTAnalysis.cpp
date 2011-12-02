//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SVTTAnalysis.h"
#include "SSException.h"

/** constructor */
CSpatialVarTempTrendAnalysis::CSpatialVarTempTrendAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
                             :CAnalysis(Parameters, DataHub, PrintDirection), _topClusters(DataHub) {
}

/** destructor */
CSpatialVarTempTrendAnalysis::~CSpatialVarTempTrendAnalysis() {}

/** Allocates objects used during simulations, instead of repeated allocations
    for each simulation.
    NOTE: This analysis has not been optimized to 'pre' allocate objects used in
          simulation process. This function is only a shell.                     */
void CSpatialVarTempTrendAnalysis::AllocateSimulationObjects(const AbstractDataSetGateway & DataGateway) {
  gClusterData.reset(new SVTTClusterData(DataGateway));
}

/** Allocates objects used during calculation of most likely clusters, instead
    of repeated allocations for each simulation.
    NOTE: This analysis has not been optimized to 'pre' allocate objects used in
          process of finding most likely clusters. */
void CSpatialVarTempTrendAnalysis::AllocateTopClustersObjects(const AbstractDataSetGateway & DataGateway) {
  try {
    gClusterComparator.reset(new CSVTTCluster(gpClusterDataFactory, DataGateway));
    _topClusters.setTopClusters(*gClusterComparator);
  }
  catch (prg_exception& x) {
    x.addTrace("AllocateTopClustersObjects()","CSpatialVarTempTrendAnalysis");
    throw;
  }
}

/** calculates most likely cluster about central location 'tCenter' */
const SharedClusterVector_t CSpatialVarTempTrendAnalysis::CalculateTopClusters(tract_t tCenter, const AbstractDataSetGateway & DataGateway) {
  try {
    gClusterComparator->InitializeSVTT(0, DataGateway);
    _topClusters.resetSVTT(tCenter, DataGateway);
    //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
    for (int k=0; k <= gParameters.GetNumTotalEllipses(); ++k) {
       CentroidNeighbors CentroidDef(k, gDataHub, tCenter);
       _topClusters.resetNeighborCounts(k);
       gClusterComparator->InitializeSVTT(tCenter, DataGateway);
       gClusterComparator->SetEllipseOffset(k, gDataHub);
       gClusterComparator->CalculateTopClusterAboutCentroidDefinition(DataGateway, CentroidDef, _topClusters.getClusterSet(k), *gpLikelihoodCalculator);
    }
    SharedClusterVector_t topClusters;
    _topClusters.getTopClusters(topClusters);
    for (size_t t=0; t < topClusters.size(); ++t) {
      if (topClusters[t]->ClusterDefined())
        dynamic_cast<CSVTTCluster*>(topClusters[t].get())->SetTimeTrend(gParameters.GetTimeAggregationUnitsType(), gParameters.GetTimeAggregationLength());
    }
    return topClusters;
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


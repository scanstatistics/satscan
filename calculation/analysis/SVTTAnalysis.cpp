//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "SVTTAnalysis.h"

/** constructor */
CSpatialVarTempTrendAnalysis::CSpatialVarTempTrendAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
                             :CAnalysis(Parameters, DataHub, PrintDirection), gTopClusters(DataHub) {}

/** destructor */
CSpatialVarTempTrendAnalysis::~CSpatialVarTempTrendAnalysis() {}

/** Allocates objects used during simulations, instead of repeated allocations
    for each simulation.
    NOTE: This analysis has not been optimized to 'pre' allocate objects used in
          simulation process. This function is only a shell.                     */
void CSpatialVarTempTrendAnalysis::AllocateSimulationObjects(const AbstractDataSetGateway & DataGateway) {
}

/** Allocates objects used during calculation of most likely clusters, instead
    of repeated allocations for each simulation.
    NOTE: This analysis has not been optimized to 'pre' allocate objects used in
          process of finding most likely clusters. */
void CSpatialVarTempTrendAnalysis::AllocateTopClustersObjects(const AbstractDataSetGateway & DataGateway) {
  try {
    CSVTTCluster thisCluster(DataGateway, gDataHub.GetNumTimeIntervals());
    thisCluster.InitializeSVTT(0, DataGateway);
    gTopClusters.SetTopClusters(thisCluster);
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateTopClustersObjects()","CSpatialVarTempTrendAnalysis");
    throw;
  }
}

/** calculates most likely cluster about central location 'tCenter' */
const CCluster & CSpatialVarTempTrendAnalysis::CalculateTopCluster(tract_t tCenter, const AbstractDataSetGateway & DataGateway) {
  int                   k;
  tract_t               i, iNumNeighbors;

  try {
    gTopClusters.Reset(tCenter);
    //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
    for (k=0; k <= gParameters.GetNumTotalEllipses(); ++k) {
       CSVTTCluster thisCluster(DataGateway, gDataHub.GetNumTimeIntervals());
       thisCluster.SetCenter(tCenter);
//$       thisCluster.SetRate(gParameters.GetExecuteScanRateType());
       thisCluster.SetEllipseOffset(k, gDataHub);
      // CSVTTCluster & TopShapeCluster = (CSVTTCluster&)(gpTopShapeClusters->GetTopCluster(k));
       iNumNeighbors = gDataHub.GetNeighborCountArray()[k][tCenter];
       for (i=1; i <= iNumNeighbors; ++i) {
          thisCluster.AddNeighbor(gDataHub.GetNeighbor(k, tCenter, i), DataGateway);
          //TODO: -- Calculate loglikelihood ratio for all datasets.
          thisCluster.m_nRatio = gpLikelihoodCalculator->CalcSVTTLogLikelihood(0, &thisCluster, *(DataGateway.GetDataSetInterface(0).GetTimeTrend()));
          if (thisCluster.m_nRatio > gTopClusters.GetTopCluster(k).m_nRatio)
           gTopClusters.GetTopCluster(k) = thisCluster;
       }
    }

    // the ratio needs to be calculated for each circle/cylinder, instead of here !!!!
    CSVTTCluster& Cluster = gTopClusters.GetTopCluster();
    Cluster.m_nRatio -= gpLikelihoodCalculator->GetLogLikelihoodForTotal();
    Cluster.SetTimeTrend(gParameters.GetTimeAggregationUnitsType(), gParameters.GetTimeAggregationLength());
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateTopCluster()","CSpatialVarTempTrendAnalysis");
    throw;
  }
  return gTopClusters.GetTopCluster();
}

/** calculates loglikelihood ratio for simulated data pointed to by DataSetInterface
    in a retrospective manner */
double CSpatialVarTempTrendAnalysis::MonteCarlo(const DataSetInterface & Interface) {
  int           k;
  tract_t       i, j, iNumNeighbors;
  double        dMaximumLogLikelihoodRatio;

  try {
    gTopClusters.SetTopClusters(CSVTTCluster(Interface, gDataHub.GetNumTimeIntervals()));
  
    //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
    for (k=0; k <= gParameters.GetNumTotalEllipses(); k++) {
       CSVTTCluster thisCluster(Interface, gDataHub.GetNumTimeIntervals());
//$       thisCluster.SetRate(gParameters.GetExecuteScanRateType());
       thisCluster.SetEllipseOffset(k, gDataHub);
       for (i=0; i < gDataHub.m_nGridTracts; ++i) {
          thisCluster.InitializeSVTT(i, Interface);
          iNumNeighbors = gDataHub.GetNeighborCountArray()[k][i];	
          for (j=1; j <= iNumNeighbors; j++) {
             thisCluster.AddNeighbor(gDataHub.GetNeighbor(k, i, j), Interface, 0);
             thisCluster.m_nRatio = gpLikelihoodCalculator->CalcSVTTLogLikelihood(0, &thisCluster, *(Interface.GetTimeTrend()));
             if (thisCluster.m_nRatio > gTopClusters.GetTopCluster(k).m_nRatio)
               gTopClusters.GetTopCluster(k) = thisCluster;
          }
       }
    }
    //get copy of best cluster over all iterations
    dMaximumLogLikelihoodRatio = gTopClusters.GetTopCluster().m_nRatio;
    dMaximumLogLikelihoodRatio -= gpLikelihoodCalculator->GetLogLikelihoodForTotal();
  }
  catch (ZdException &x) {
    x.AddCallpath("MonteCarlo()","CSpatialVarTempTrendAnalysis");
    throw;
  }
  return dMaximumLogLikelihoodRatio;
}


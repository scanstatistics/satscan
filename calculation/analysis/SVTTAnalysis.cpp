//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "SVTTAnalysis.h"
#include "SVTTCluster.h"

/** constructor */
CSpatialVarTempTrendAnalysis::CSpatialVarTempTrendAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
                             :CAnalysis(Parameters, DataHub, PrintDirection) {
  try {
    Init();
    Setup();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CSpatialVarTempTrendAnalysis");
    throw;
  }
}

/** destructor */
CSpatialVarTempTrendAnalysis::~CSpatialVarTempTrendAnalysis() {
  try {
    delete gpTopShapeClusters;
  }
  catch(...){}
}

/** Allocates objects used during simulations, instead of repeated allocations
    for each simulation.
    NOTE: This analysis has not been optimized to 'pre' allocate objects used in
          simulation process. This function is only a shell.                     */
void CSpatialVarTempTrendAnalysis::AllocateSimulationObjects(const AbtractDataSetGateway & DataGateway) {
}

/** Allocates objects used during calculation of most likely clusters, instead
    of repeated allocations for each simulation.
    NOTE: This analysis has not been optimized to 'pre' allocate objects used in
          process of finding most likely clusters. */
void CSpatialVarTempTrendAnalysis::AllocateTopClustersObjects(const AbtractDataSetGateway & DataGateway) {
  try {
    CSVTTCluster thisCluster(DataGateway, gDataHub.GetNumTimeIntervals());
    thisCluster.InitializeSVTT(0, DataGateway);
    gpTopShapeClusters->SetTopClusters(thisCluster);
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateTopClustersObjects()","CSpatialVarTempTrendAnalysis");
    throw;
  }
}

/** calculates most likely cluster about central location 'tCenter' */
const CCluster & CSpatialVarTempTrendAnalysis::CalculateTopCluster(tract_t tCenter, const AbtractDataSetGateway & DataGateway) {
  int                   k;
  tract_t               i, iNumNeighbors;

  try {
    gpTopShapeClusters->Reset(tCenter);
    //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
    for (k=0; k <= gParameters.GetNumTotalEllipses(); ++k) {
       CSVTTCluster thisCluster(DataGateway, gDataHub.GetNumTimeIntervals());
       thisCluster.SetCenter(tCenter);
       thisCluster.SetRate(gParameters.GetAreaScanRateType());
       thisCluster.SetEllipseOffset(k);
       thisCluster.SetNonCompactnessPenalty((k == 0 || !gParameters.GetNonCompactnessPenalty() ? 1 : gDataHub.GetEllipseShape(k)));
       CSVTTCluster & TopShapeCluster = (CSVTTCluster&)(gpTopShapeClusters->GetTopCluster(k));
       iNumNeighbors = gDataHub.GetNeighborCountArray()[k][tCenter];
       for (i=1; i <= iNumNeighbors; ++i) {
          thisCluster.AddNeighbor(gDataHub.GetNeighbor(k, tCenter, i), DataGateway);
          //TODO: -- Calculate loglikelihood ratio for all datasets.
          thisCluster.m_nRatio = gpLikelihoodCalculator->CalcSVTTLogLikelihood(0, &thisCluster, *(DataGateway.GetDataSetInterface(0).GetTimeTrend()));
          if (thisCluster.m_nRatio > TopShapeCluster.m_nRatio)
           TopShapeCluster = thisCluster;
       }
    }

    // the ratio needs to be calculated for each circle/cylinder, instead of here !!!!
    CSVTTCluster & Cluster = (CSVTTCluster&)(gpTopShapeClusters->GetTopCluster());
    Cluster.m_nRatio -= gpLikelihoodCalculator->GetLogLikelihoodForTotal();
    Cluster.SetTimeTrend(gParameters.GetTimeAggregationUnitsType(), gParameters.GetTimeAggregationLength());
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateTopCluster()","CSpatialVarTempTrendAnalysis");
    throw;
  }
  return gpTopShapeClusters->GetTopCluster();
}

/** calculates loglikelihood ratio for simulated data pointed to by DataSetInterface
    in a retrospective manner */
double CSpatialVarTempTrendAnalysis::MonteCarlo(const DataSetInterface & Interface) {
  int           k;
  tract_t       i, j, iNumNeighbors;
  double        dMaximumLogLikelihoodRatio;

  try {
    gpTopShapeClusters->SetTopClusters(CSVTTCluster(Interface, gDataHub.GetNumTimeIntervals()));
  
    //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
    for (k=0; k <= gParameters.GetNumTotalEllipses(); k++) {
       CSVTTCluster thisCluster(Interface, gDataHub.GetNumTimeIntervals());
       thisCluster.SetRate(gParameters.GetAreaScanRateType());
       thisCluster.SetEllipseOffset(k);
       thisCluster.SetNonCompactnessPenalty((k == 0 || !gParameters.GetNonCompactnessPenalty() ? 1 : gDataHub.GetEllipseShape(k)));
       CSVTTCluster & TopShapeCluster = (CSVTTCluster&)(gpTopShapeClusters->GetTopCluster(k));
       for (i=0; i < gDataHub.m_nGridTracts; ++i) {
          thisCluster.InitializeSVTT(i, Interface);
          iNumNeighbors = gDataHub.GetNeighborCountArray()[k][i];	
          for (j=1; j <= iNumNeighbors; j++) {
             thisCluster.AddNeighbor(gDataHub.GetNeighbor(k, i, j), Interface, 0);
             thisCluster.m_nRatio = gpLikelihoodCalculator->CalcSVTTLogLikelihood(0, &thisCluster, *(Interface.GetTimeTrend()));
             if (thisCluster.m_nRatio > TopShapeCluster.m_nRatio)
               TopShapeCluster = thisCluster;
          }
       }
    }
    //get copy of best cluster over all iterations
    dMaximumLogLikelihoodRatio = gpTopShapeClusters->GetTopCluster().m_nRatio;
    dMaximumLogLikelihoodRatio -= gpLikelihoodCalculator->GetLogLikelihoodForTotal();
  }
  catch (ZdException &x) {
    x.AddCallpath("MonteCarlo()","CSpatialVarTempTrendAnalysis");
    throw;
  }
  return dMaximumLogLikelihoodRatio;
}

/** internal setup function */
void CSpatialVarTempTrendAnalysis::Setup() {
  try {
    gpTopShapeClusters = new TopClustersContainer(gDataHub);
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","CSpatialVarTempTrendAnalysis");
    delete gpTopShapeClusters;
    throw;
  }
}


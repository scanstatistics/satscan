#include "SaTScan.h"
#pragma hdrstop
#include "SVTTAnalysis.h"
#include "SVTTCluster.h"

/** constructor */
CSpatialVarTempTrendAnalysis::CSpatialVarTempTrendAnalysis(CParameters*  pParameters, CSaTScanData* pData, BasePrint *pPrintDirection)
                             :CAnalysis(pParameters, pData, pPrintDirection) {
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
void CSpatialVarTempTrendAnalysis::AllocateSimulationObjects(const AbtractDataStreamGateway & DataGateway) {
}

/** Allocates objects used during calculation of most likely clusters, instead
    of repeated allocations for each simulation.
    NOTE: This analysis has not been optimized to 'pre' allocate objects used in
          process of finding most likely clusters. */
void CSpatialVarTempTrendAnalysis::AllocateTopClustersObjects(const AbtractDataStreamGateway & DataGateway) {
  try {
    CSVTTCluster thisCluster(DataGateway, m_pData->GetNumTimeIntervals());
    thisCluster.InitializeSVTT(0, DataGateway);
    gpTopShapeClusters->SetTopClusters(thisCluster);
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateTopClustersObjects()","CSpatialVarTempTrendAnalysis");
    throw;
  }
}

/** calculates most likely cluster about central location 'tCenter' */
const CCluster & CSpatialVarTempTrendAnalysis::CalculateTopCluster(tract_t tCenter, const AbtractDataStreamGateway & DataGateway) {
  int                   k;
  tract_t               i, iNumNeighbors;
  CModel              & ProbModel(m_pData->GetProbabilityModel());

  try {
    gpTopShapeClusters->Reset(tCenter);
    //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
    for (k=0; k <= m_pParameters->GetNumTotalEllipses(); ++k) {
       CSVTTCluster thisCluster(DataGateway, m_pData->GetNumTimeIntervals());
       thisCluster.SetCenter(tCenter);
       thisCluster.SetRate(m_pParameters->GetAreaScanRateType());
       thisCluster.SetEllipseOffset(k);
       thisCluster.SetDuczmalCorrection((k == 0 || !m_pParameters->GetDuczmalCorrectEllipses() ? 1 : m_pData->GetShapesArray()[k - 1]));
       CSVTTCluster & TopShapeCluster = (CSVTTCluster&)(gpTopShapeClusters->GetTopCluster(k));
       iNumNeighbors = m_pData->GetNeighborCountArray()[k][tCenter];
       for (i=1; i <= iNumNeighbors; ++i) {
          thisCluster.AddNeighbor(m_pData->GetNeighbor(k, tCenter, i), DataGateway);
          thisCluster.m_nRatio = 0;
          for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
            thisCluster.m_nRatio += gpLikelihoodCalculator->CalcSVTTLogLikelihoodRatio(t, &thisCluster, *(DataGateway.GetDataStreamInterface(t).GetTimeTrend()));
          if (thisCluster.m_nRatio > TopShapeCluster.m_nRatio)
           TopShapeCluster = thisCluster;
       }
    }

    // the ratio needs to be calculated for each circle/cylinder, instead of here !!!!
    CSVTTCluster & Cluster = (CSVTTCluster&)(gpTopShapeClusters->GetTopCluster());
    Cluster.m_nRatio = Cluster.m_nLogLikelihood - gpLikelihoodCalculator->GetLogLikelihoodForTotal();
    Cluster.SetTimeTrend(m_pParameters->GetTimeIntervalUnitsType(), m_pParameters->GetTimeIntervalLength());
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateTopCluster()","CSpatialVarTempTrendAnalysis");
    throw;
  }
  return gpTopShapeClusters->GetTopCluster();
}

/** calculates loglikelihood ratio for simulated data pointed to by DataStreamInterface
    in a retrospective manner */
double CSpatialVarTempTrendAnalysis::MonteCarlo(const DataStreamInterface & Interface) {
  int           k;
  tract_t       i, j, iNumNeighbors;
  double        dMaximumLogLikelihoodRatio;
  CModel      & ProbModel(m_pData->GetProbabilityModel());  

  try {
    //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
    for (k=0; k <= m_pParameters->GetNumTotalEllipses(); k++) {
       CSVTTCluster thisCluster(Interface, m_pData->GetNumTimeIntervals());
       thisCluster.SetRate(m_pParameters->GetAreaScanRateType());
       thisCluster.SetEllipseOffset(k);
       thisCluster.SetDuczmalCorrection((k == 0 || !m_pParameters->GetDuczmalCorrectEllipses() ? 1 : m_pData->GetShapesArray()[k - 1]));
       CSVTTCluster & TopShapeCluster = (CSVTTCluster&)(gpTopShapeClusters->GetTopCluster(k));
       for (i=0; i < m_pData->m_nGridTracts; ++i) {
          thisCluster.InitializeSVTT(i, Interface);
          iNumNeighbors = m_pData->GetNeighborCountArray()[k][i];	
          for (j=1; j <= iNumNeighbors; j++) {
             thisCluster.AddNeighbor(m_pData->GetNeighbor(k, i, j), Interface, 0);
             thisCluster.m_nRatio = gpLikelihoodCalculator->CalcSVTTLogLikelihoodRatio(0, &thisCluster, *(Interface.GetTimeTrend()));
             if (thisCluster.m_nRatio > TopShapeCluster.m_nRatio)
               TopShapeCluster = thisCluster;
          }
       }
    }
    //get copy of best cluster over all iterations
    dMaximumLogLikelihoodRatio = gpTopShapeClusters->GetTopCluster().m_nRatio;
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
    gpTopShapeClusters = new TopClustersContainer(*m_pData);
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","CSpatialVarTempTrendAnalysis");
    delete gpTopShapeClusters;
    throw;
  }
}


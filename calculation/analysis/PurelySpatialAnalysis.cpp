#include "SaTScan.h"
#pragma hdrstop
#include "PurelySpatialAnalysis.h"

/** Constructor */
CPurelySpatialAnalysis::CPurelySpatialAnalysis(CParameters*  pParameters, CSaTScanData* pData, BasePrint *pPrintDirection)
                       :CAnalysis(pParameters, pData, pPrintDirection) {
  try {
    Init();
    Setup();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CPurelySpatialAnalysis");
    throw;
  }
}

/** Desctructor */
CPurelySpatialAnalysis::~CPurelySpatialAnalysis(){
  try {
    delete gpTopShapeClusters;
  }
  catch(...){}
}

/** Returns cluster centered at grid point nCenter, with the greatest loglikelihood.
    Caller is responsible for deleting returned cluster. */
void CPurelySpatialAnalysis::CalculateTopCluster(tract_t tCenter, const DataStreamGateway & DataGateway, bool bSimulation) {
  int                           i, j;
  tract_t                       iNumNeighbors;
  CModel                      & ProbModel(m_pData->GetProbabilityModel());

  try {
    gpTopShapeClusters->Reset(tCenter);
    for (j=0; j <= m_pParameters->GetNumTotalEllipses(); ++j) {   //circle is 0 offset... (always there)
       CPurelySpatialCluster thisCluster(*m_pData, gpPrintDirection);
       thisCluster.SetCenter(tCenter);
       thisCluster.SetRate(m_pParameters->GetAreaScanRateType());
       thisCluster.SetEllipseOffset(j);                       // store the ellipse link in the cluster obj
       thisCluster.SetDuczmalCorrection((j == 0 || !m_pParameters->GetDuczmalCorrectEllipses() ? 1 : m_pData->GetShapesArray()[j - 1]));
       CPurelySpatialCluster & TopShapeCluster = (CPurelySpatialCluster&)(gpTopShapeClusters->GetTopCluster(j));
       iNumNeighbors = m_pData->GetNeighborCountArray()[j][tCenter];
       for (i=1; i <= iNumNeighbors; ++i) {
          thisCluster.AddNeighbor(m_pData->GetNeighbor(j, tCenter, i), DataGateway);
          thisCluster.CompareTopCluster(TopShapeCluster, m_pData);
       }
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateTopCluster()","CPurelySpatialAnalysis");
    throw;
  }
}

CCluster & CPurelySpatialAnalysis::GetTopCalculatedCluster() {
  return gpTopShapeClusters->GetTopCluster();
}

/** Returns loglikelihood for Monte Carlo replication. */
double CPurelySpatialAnalysis::MonteCarlo(const DataStreamInterface & Interface) {
  CMeasureList                  * pMeasureList=0;
  CPurelySpatialCluster           C(*m_pData, gpPrintDirection);
  double                          dMaxLogLikelihoodRatio;
  tract_t                         i, j, iNumNeighbors;
  int                             k;
  tract_t                      ** ppNeighbors(m_pData->GetNeighborCountArray());

  try {
    C.SetRate(m_pParameters->GetAreaScanRateType());
    switch (m_pParameters->GetAreaScanRateType()) {
     case HIGH       : pMeasureList = new CMinMeasureList(*m_pData, *gpPrintDirection);
                       break;
     case LOW        : pMeasureList = new CMaxMeasureList(*m_pData, *gpPrintDirection);
                       break;
     case HIGHANDLOW : pMeasureList = new CMinMaxMeasureList(*m_pData, *gpPrintDirection);
                       break;
     default         : ZdGenerateException("Unknown incidence rate specifier \"%d\".","MonteCarlo()",
                                           m_pParameters->GetAreaScanRateType());
    }

    for (k=0; k <= m_pParameters->GetNumTotalEllipses(); k++) { //circle is 0 offset... (always there)
       for (i=0; i < m_pData->m_nGridTracts; i++) {
          C.Initialize(i);
          iNumNeighbors = ppNeighbors[k][i];
          for (j=1; j <= iNumNeighbors; j++) {
             C.AddNeighbor(m_pData->GetNeighbor(k, i, j), Interface);
             C.ComputeBestMeasures(*pMeasureList);
          }
       }
       pMeasureList->SetForNextIteration(k);
    }
    dMaxLogLikelihoodRatio = pMeasureList->GetMaximumLogLikelihoodRatio();
    delete pMeasureList;
  }
  catch (ZdException & x) {
    delete pMeasureList;
    x.AddCallpath("MonteCarlo()", "CPurelySpatialAnalysis");
    throw;
  }
  return dMaxLogLikelihoodRatio;
}

/** Prospective monte carlo not valid for purely spatial analysis. */
double CPurelySpatialAnalysis::MonteCarloProspective(const DataStreamInterface & Interface) {
  ZdGenerateException("MonteCarloProspective() not implemented for CPurelySpatialAnalysis.","MonteCarloProspective()");
  return 0;
}

void CPurelySpatialAnalysis::SetTopClusters(const DataStreamGateway & DataGateway, bool bSimulation) {
  try {
    gpTopShapeClusters->SetTopClusters(CPurelySpatialCluster(*m_pData, gpPrintDirection));
  }
  catch (ZdException &x) {
    x.AddCallpath("SetTopClusters()","CPurelySpatialAnalysis");
    throw;
  }
}

void CPurelySpatialAnalysis::Setup() {
  try {
    gpTopShapeClusters = new TopClustersContainer(*m_pData);
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()", "CPurelySpatialAnalysis");
    delete gpTopShapeClusters;
    throw;
  }
}


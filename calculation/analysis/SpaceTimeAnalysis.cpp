#include "SaTScan.h"
#pragma hdrstop
#include "SpaceTimeAnalysis.h"

/** Constructor */
CSpaceTimeAnalysis::CSpaceTimeAnalysis(CParameters*  pParameters, CSaTScanData* pData, BasePrint *pPrintDirection)
                   :CAnalysis(pParameters, pData, pPrintDirection) {
  try {
    Init();
    Setup();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CSpaceTimeAnalysis");
    throw;
  }
}

/** Destructor */
CSpaceTimeAnalysis::~CSpaceTimeAnalysis() {
  try {
    delete gpTopShapeClusters;
  }
  catch(...){}
}

/** Returns cluster centered at grid point nCenter, with the greatest loglikelihood.
    Caller is responsible for deleting returned cluster. */
void CSpaceTimeAnalysis::CalculateTopCluster(tract_t tCenter, const DataStreamGateway & DataGateway, bool bSimulation) {
  IncludeClustersType           eIncludeClustersType;
  int                           k;
  tract_t                       i, iNumNeighbors;

  try {
    // if Prospective Space-Time then Alive Clusters Only.
    if (m_pParameters->GetAnalysisType() == PROSPECTIVESPACETIME)
      eIncludeClustersType = (bSimulation ? ALLCLUSTERS : ALIVECLUSTERS);
    else
      eIncludeClustersType = m_pParameters->GetIncludeClustersType();

    gpTopShapeClusters->Reset(i);
    //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
    for (k=0; k <= m_pParameters->GetNumTotalEllipses(); ++k) {
       CSpaceTimeCluster thisCluster(eIncludeClustersType, *m_pData, *gpPrintDirection);
       thisCluster.SetCenter(tCenter);
       thisCluster.SetRate(m_pParameters->GetAreaScanRateType());
       thisCluster.SetEllipseOffset(k);
       thisCluster.SetDuczmalCorrection((k == 0 || !m_pParameters->GetDuczmalCorrectEllipses() ? 1 : m_pData->GetShapesArray()[k - 1]));
       CSpaceTimeCluster & TopShapeCluster = (CSpaceTimeCluster&)(gpTopShapeClusters->GetTopCluster(k));
       iNumNeighbors = m_pData->GetNeighborCountArray()[k][tCenter];
       for (i=1; i <= iNumNeighbors; ++i) {
          thisCluster.AddNeighbor(m_pData->GetNeighbor(k, tCenter, i), DataGateway);
          thisCluster.CompareTopCluster(TopShapeCluster, *m_pData);
       }
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("CalculateTopCluster()", "CSpaceTimeAnalysis");
    throw;
  }
}

CCluster & CSpaceTimeAnalysis::GetTopCalculatedCluster() {
  return gpTopShapeClusters->GetTopCluster();
}

/** Returns loglikelihood for Monte Carlo replication. */
double CSpaceTimeAnalysis::MonteCarlo(const DataStreamInterface & Interface) {
  CMeasureList  * pMeasureList=0;
  double          dMaxLogLikelihoodRatio;
  int             k;
  tract_t         i, j, iNumNeighbors;

  try {
    CSpaceTimeCluster C(m_pParameters->GetIncludeClustersType(), *m_pData, *gpPrintDirection);
    C.SetRate(m_pParameters->GetAreaScanRateType());
    switch (m_pParameters->GetAreaScanRateType()) {
     case HIGH      : pMeasureList = new CMinMeasureList(*m_pData, *gpPrintDirection);
                      break;
     case LOW       : pMeasureList = new CMaxMeasureList(*m_pData, *gpPrintDirection);
                      break;
     case HIGHANDLOW: pMeasureList = new CMinMaxMeasureList(*m_pData, *gpPrintDirection);
                      break;
     default        : ZdGenerateException("Unknown incidence rate specifier \"%d\".","MonteCarlo()",
                                          m_pParameters->GetAreaScanRateType());
    }

    //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
    for (k=0; k <= m_pParameters->GetNumTotalEllipses(); ++k) {
       for (i=0; i < m_pData->m_nGridTracts; ++i) {
          C.Initialize(i);
          iNumNeighbors = m_pData->GetNeighborCountArray()[k][i];
          for (j=1; j <= iNumNeighbors; ++j) {
             C.AddNeighbor(m_pData->GetNeighbor(k, i, j), Interface, 0);
             C.ComputeBestMeasures(*pMeasureList);
          }
       }
       pMeasureList->SetForNextIteration(k);
    }
    dMaxLogLikelihoodRatio = pMeasureList->GetMaximumLogLikelihoodRatio();
    delete pMeasureList;
  }
  catch (ZdException & x) {
    x.AddCallpath("MonteCarlo()", "CSpaceTimeAnalysis");
    delete pMeasureList;
    throw;
  }
  return dMaxLogLikelihoodRatio;
}

double CSpaceTimeAnalysis::MonteCarloProspective(const DataStreamInterface & Interface) {
  CMeasureList  * pMeasureList=0;
  double          dMaxLogLikelihoodRatio;
  int             k;
  tract_t         iNumNeighbors;

  try {
    //for prospective Space-Time, m_bAliveClustersOnly should be false..
    //m_bAliveClustersOnly is the first parameter into the CSpaceTimeCluster class
    CSpaceTimeCluster C(ALLCLUSTERS, *m_pData, *gpPrintDirection);
    C.SetRate(m_pParameters->GetAreaScanRateType());
    switch (m_pParameters->GetAreaScanRateType()) {
     case HIGH       : pMeasureList = new CMinMeasureList(*m_pData, *gpPrintDirection);
                       break;
     case LOW        : pMeasureList = new CMaxMeasureList(*m_pData, *gpPrintDirection);
                       break;
     case HIGHANDLOW : pMeasureList = new CMinMaxMeasureList(*m_pData, *gpPrintDirection);
                       break;
     default         : ZdGenerateException("Unknown incidence rate specifier \"%d\".","MonteCarloProspective()",
                       m_pParameters->GetAreaScanRateType());
    }

    for (k=0; k <= m_pParameters->GetNumTotalEllipses(); ++k) {  //circle is 0 offset... (always there)
       for (tract_t i = 0; i<m_pData->m_nGridTracts; ++i) {
          C.Initialize(i);
          iNumNeighbors = m_pData->GetNeighborCountArray()[k][i];
          for (tract_t j=1; j <= iNumNeighbors; ++j) {
             C.AddNeighbor(m_pData->GetNeighbor(k, i, j), Interface, 0);
             C.ComputeBestMeasures(*pMeasureList);
          }
       }
       pMeasureList->SetForNextIteration(k);
    }
    dMaxLogLikelihoodRatio = pMeasureList->GetMaximumLogLikelihoodRatio();
    delete pMeasureList;
  }
  catch (ZdException & x) {
    x.AddCallpath("MonteCarloProspective()", "CSpaceTimeAnalysis");
    delete pMeasureList;
    throw;
  }
  return dMaxLogLikelihoodRatio;
}

void CSpaceTimeAnalysis::SetTopClusters(const DataStreamGateway & DataGateway, bool bSimulation) {
  IncludeClustersType   eIncludeClustersType;

  try {
    // if Prospective Space-Time then Alive Clusters Only.
    if (m_pParameters->GetAnalysisType() == PROSPECTIVESPACETIME)
      eIncludeClustersType = (bSimulation ? ALLCLUSTERS : ALIVECLUSTERS);
    else
      eIncludeClustersType = m_pParameters->GetIncludeClustersType();

    gpTopShapeClusters->SetTopClusters(CSpaceTimeCluster(eIncludeClustersType, *m_pData, *gpPrintDirection));
  }
  catch (ZdException &x) {
    x.AddCallpath("SetTopClusters()","CSpaceTimeAnalysis");
    throw;
  }
}

void CSpaceTimeAnalysis::Setup() {
  try {
    gpTopShapeClusters = new TopClustersContainer(*m_pData);
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()", "CSpaceTimeAnalysis");
    delete gpTopShapeClusters;
    throw;
  }
}


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
CCluster* CPurelySpatialAnalysis::GetTopCluster(tract_t nCenter) {
  int                           i, j;
  CPurelySpatialCluster       * pTopCluster=0;

  try {
    pTopCluster = new CPurelySpatialCluster(gpPrintDirection);
    pTopCluster->SetLogLikelihood(m_pData->m_pModel->GetLogLikelihoodForTotal());
    gpTopShapeClusters->SetTopClusters(*pTopCluster);

    //***************************************************************************
    // Need to create a new cluster object for each circle or ellipse.
    // If you do not, then AddNeighbor creates invalid case counts as you tally
    // over multiple objects (i.e. the circle and ellipses.
    //***************************************************************************
    for (j = 0; j <= m_pParameters->GetNumTotalEllipses(); j++) {   //circle is 0 offset... (always there)
       CPurelySpatialCluster thisCluster(gpPrintDirection);
       thisCluster.SetCenter(nCenter);
       thisCluster.SetRate(m_pParameters->GetAreaScanRateType());
       thisCluster.SetEllipseOffset(j);                       // store the ellipse link in the cluster obj
       thisCluster.SetDuczmalCorrection((j == 0 || !m_pParameters->GetDuczmalCorrectEllipses() ? 1 : m_pData->mdE_Shapes[j - 1]));
       CPurelySpatialCluster & TopShapeCluster = (CPurelySpatialCluster&)(gpTopShapeClusters->GetTopCluster(j));       
       for (i=1; i <= m_pData->m_NeighborCounts[j][nCenter]; i++) {
          thisCluster.AddNeighbor(j, *m_pData, m_pData->m_pCases, i);
          if (thisCluster.RateIsOfInterest(m_pData->m_nTotalCases, m_pData->m_nTotalMeasure)) {
            thisCluster.m_nLogLikelihood = m_pData->m_pModel->CalcLogLikelihood(thisCluster.m_nCases, thisCluster.m_nMeasure);
            if (thisCluster.m_nLogLikelihood > TopShapeCluster.m_nLogLikelihood)
              TopShapeCluster = thisCluster;
          }
       }
    }
    //get copy of best cluster over all iterations
    *pTopCluster = (CPurelySpatialCluster&)(gpTopShapeClusters->GetTopCluster());
  }
  catch (ZdException & x) {
    x.AddCallpath("GetTopCluster()", "CPurelySpatialAnalysis");
    delete pTopCluster;
    throw;
  }
  return pTopCluster;
}

/** Returns loglikelihood for Monte Carlo replication. */
double CPurelySpatialAnalysis::MonteCarlo() {
  CMeasureList                  * pMeasureList=0;
  CPurelySpatialCluster           C(gpPrintDirection);
  double                          dMaxLogLikelihoodRatio;
  tract_t                         i, j;
  int                             k;

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
          for (j=1; j <= m_pData->m_NeighborCounts[k][i]; j++) {
             C.AddNeighbor(k, *m_pData, m_pData->m_pSimCases, j);
             pMeasureList->AddMeasure(C.m_nCases, C.m_nMeasure);
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

/** For purely spatial analysis, prospective monte carlo is the same as monte carlo. */
double CPurelySpatialAnalysis::MonteCarloProspective() {
  CMeasureList                * pMeasureList=0;
  CPurelySpatialCluster         C(gpPrintDirection);
  double                        dMaxLogLikelihoodRatio;
  tract_t                       i, j;
  int                           k;

  try {
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

    for (k=0; k <= m_pParameters->GetNumTotalEllipses(); k++) {  //circle is 0 offset... (always there)
       for (tract_t i = 0; i<m_pData->m_nGridTracts; i++) {
          C.Initialize(i);
          for (tract_t j=1; j<=m_pData->m_NeighborCounts[k][i]; j++) {
             C.AddNeighbor(k, *m_pData, m_pData->m_pSimCases, j);
             pMeasureList->AddMeasure(C.m_nCases, C.m_nMeasure);
          }
       }
       pMeasureList->SetForNextIteration(k);
    }
    dMaxLogLikelihoodRatio = pMeasureList->GetMaximumLogLikelihoodRatio();
    delete pMeasureList;
  }
  catch (ZdException & x) {
    delete pMeasureList;
    x.AddCallpath("MonteCarloProspective()", "CPurelySpatialAnalysis");
    throw;
  }
  return dMaxLogLikelihoodRatio;
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


#include "SaTScan.h"
#pragma hdrstop
#include "SVTTAnalysis.h"
#include "SVTTCluster.h"

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

CSpatialVarTempTrendAnalysis::~CSpatialVarTempTrendAnalysis() {
  try {
    delete gpTopShapeClusters;
  }
  catch(...){}
}

CCluster* CSpatialVarTempTrendAnalysis::GetTopCluster(tract_t nCenter) {
  CSVTTCluster        * pTopCluster=0;
  int                   k;
  tract_t               i;

  try {
    pTopCluster = new CSVTTCluster(*m_pData, m_pData->m_pCases_TotalByTimeInt, gpPrintDirection);
    pTopCluster->SetLogLikelihood(0.0);
    gpTopShapeClusters->SetTopClusters(*pTopCluster);

    //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
    for (k=0; k <= m_pParameters->GetNumTotalEllipses(); k++) {
       CSVTTCluster thisCluster(*m_pData, m_pData->m_pCases_TotalByTimeInt, gpPrintDirection);
       thisCluster.SetCenter(nCenter);
       thisCluster.SetRate(m_pParameters->GetAreaScanRateType());
       thisCluster.SetEllipseOffset(k);
       thisCluster.SetDuczmalCorrection((k == 0 || !m_pParameters->GetDuczmalCorrectEllipses() ? 1 : m_pData->mdE_Shapes[k - 1]));
       CSVTTCluster & TopShapeCluster = (CSVTTCluster&)(gpTopShapeClusters->GetTopCluster(k));
       for (i=1; i <= m_pData->m_NeighborCounts[k][nCenter]; i++) {
          thisCluster.AddNeighbor(k, *m_pData, m_pData->m_pCases_NC, i);
          thisCluster.m_nLogLikelihood = m_pData->m_pModel->CalcSVTTLogLikelihood(&thisCluster, m_pData->m_nTimeTrend);
          if (/*i==1 ||*/ thisCluster.m_nLogLikelihood > TopShapeCluster.m_nLogLikelihood)
           TopShapeCluster = thisCluster;
       }
    }
    //get copy of best cluster over all iterations
    *pTopCluster = (CSVTTCluster&)(gpTopShapeClusters->GetTopCluster());
    //NOTE --> make sure that this alteration is correct!
    pTopCluster->m_nTimeTrend.SetAnnualTimeTrend(m_pParameters->GetTimeIntervalUnitsType(), m_pParameters->GetTimeIntervalLength());
  }
  catch (ZdException &x) {
    delete pTopCluster;
    x.AddCallpath("GetTopCluster()","CSpatialVarTempTrendAnalysis");
    throw;
  }
  return pTopCluster;
}

double CSpatialVarTempTrendAnalysis::MonteCarlo() {
  int           k;
  tract_t       i, j;
  double        dMaximumLogLikelihoodRatio;

  try {
    gpTopShapeClusters->SetTopClusters(CSVTTCluster(*m_pData, m_pData->m_pCases_TotalByTimeInt, gpPrintDirection));

    //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
    for (k=0; k <= m_pParameters->GetNumTotalEllipses(); k++) {
       CSVTTCluster thisCluster(*m_pData, m_pData->m_pCases_TotalByTimeInt, gpPrintDirection);
       thisCluster.SetRate(m_pParameters->GetAreaScanRateType());
       thisCluster.SetEllipseOffset(k);
       thisCluster.SetDuczmalCorrection((k == 0 || !m_pParameters->GetDuczmalCorrectEllipses() ? 1 : m_pData->mdE_Shapes[k - 1]));
       CSVTTCluster & TopShapeCluster = (CSVTTCluster&)(gpTopShapeClusters->GetTopCluster(k));
       for (i=0; i < m_pData->m_nGridTracts; i++) {
          thisCluster.InitializeSVTT(i, *m_pData, m_pData->m_pSimCases_TotalByTimeInt);
          for (j=1; j <= m_pData->m_NeighborCounts[k][i]; j++) {
             thisCluster.AddNeighbor(k,*m_pData, m_pData->m_pSimCases_NC, j);
             thisCluster.m_nLogLikelihood = m_pData->m_pModel->CalcSVTTLogLikelihood(&thisCluster, m_pData->m_nTimeTrend_Sim);
             if (/*i==1 ||*/ thisCluster.m_nLogLikelihood > TopShapeCluster.m_nLogLikelihood)
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

double CSpatialVarTempTrendAnalysis::MonteCarloProspective() {
  ZdGenerateException("MonteCarloProspective() not implemented.","CSpatialVarTempTrendAnalysis");
  return 0;
}

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


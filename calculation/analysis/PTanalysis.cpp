// PTAnalysis.cpp

#include "PTanalysis.h"
#include "PTCluster.h"
#include "PTData.h"
#include "MeasureList.h"
//#include "MakeData.h"
#include "TimeEstimate.h"
#include "display.h"
#include "model.h"

CPurelyTemporalAnalysis::CPurelyTemporalAnalysis(CParameters*  pParameters,
                                                 CSaTScanData* pData)
                        :CAnalysis(pParameters, pData)
{
}

CPurelyTemporalAnalysis::~CPurelyTemporalAnalysis()
{
}

void CPurelyTemporalAnalysis::FindTopClusters()
{
#if DEBUGANALYSIS
m_pData->CSaTScanData::DisplayCases(m_pDebugFile);
m_pData->CSaTScanData::DisplayMeasure(m_pDebugFile);
#endif

  printf("Find top cluster.\n");

  m_pTopClusters[0] = GetTopCluster(0);

  if (m_pTopClusters[0]->ClusterDefined())
     m_nClustersRetained++;
  else
  {
    delete m_pTopClusters[0];
    m_pTopClusters[0] = NULL;
  }

}

CCluster* CPurelyTemporalAnalysis::GetTopCluster(tract_t nCenter)
{
  printf("Get Top P.T. Cluster.\n");
  CPurelyTemporalCluster* MaxCluster = new CPurelyTemporalCluster
                                           (m_pParameters->m_bAliveClustersOnly,
                                            m_pData->m_nTimeIntervals,
                                            m_pData->m_nIntervalCut);
  CPurelyTemporalCluster* C          = new CPurelyTemporalCluster
                                           (m_pParameters->m_bAliveClustersOnly,
                                            m_pData->m_nTimeIntervals,
                                            m_pData->m_nIntervalCut);

  MaxCluster->CCluster::SetLogLikelihood(m_pData->m_pModel->GetLogLikelihoodForTotal());

  C->SetCenter(nCenter);
  C->SetRate(m_pParameters->m_nAreas);
  C->InitTimeIntervalIndeces();

  while (C->SetNextTimeInterval(((CPurelyTemporalData*)(m_pData))->m_pPTCases,
                                ((CPurelyTemporalData*)(m_pData))->m_pPTMeasure))
  {
    if (C->RateIsOfInterest(m_pData->m_nTotalCases, m_pData->m_nTotalMeasure))
    {
      C->m_nLogLikelihood = m_pData->m_pModel->CalcLogLikelihood
                                               (C->m_nCases, C->m_nMeasure);
      //C->SetLogLikelihood(m_pData->m_nTotalCases, m_pData->m_nTotalMeasure);
      if (C->m_nLogLikelihood > MaxCluster->m_nLogLikelihood)
        *MaxCluster = *C;
    }

  }

  delete C;

  MaxCluster->SetRatioAndDates(*m_pData);

  return MaxCluster;
}

/*void CPurelyTemporalAnalysis::MakeData()
{
  CAnalysis::MakeData();
  m_pData->SetPurelyTemporalSimCases();
}
*/

/*void CPurelyTemporalAnalysis::MakeData()
{
    MakePurelyTemporalData(m_pData->m_pPTMeasure,
                           m_pData->m_nTimeIntervals,
                           m_pData->m_nTotalCases,
                           m_pData->m_nTotalMeasure,
                           m_pData->m_pPTSimCases);
}
*/
double CPurelyTemporalAnalysis::MonteCarlo()
{
  CMeasureList* pMeasureList;

  CPurelyTemporalCluster C(m_pParameters->m_bAliveClustersOnly,
                           m_pData->m_nTimeIntervals,
                           m_pData->m_nIntervalCut);

  C.SetRate(m_pParameters->m_nAreas);

  double nMaxLogLikelihood = m_pData->m_pModel->GetLogLikelihoodForTotal();

  switch (m_pParameters->m_nAreas)
  {
    case HIGH      : pMeasureList = new CMinMeasureList(m_pData->m_nTotalCases, m_pData->m_nTotalMeasure);
                     break;
    case LOW       : pMeasureList = new CMaxMeasureList(m_pData->m_nTotalCases, m_pData->m_nTotalMeasure);
                     break;
    case HIGHANDLOW: pMeasureList = new CMinMaxMeasureList(m_pData->m_nTotalCases, m_pData->m_nTotalMeasure);
                     break;
    default        : ;
  }

  C.Initialize(0);
  C.InitTimeIntervalIndeces();

  while (C.SetNextTimeInterval(/*((CPurelyTemporalData*)(*/m_pData/*))*/->m_pPTSimCases,
                               /*((CPurelyTemporalData*)(*/m_pData/*))*/->m_pPTMeasure))
    pMeasureList->AddMeasure(C.m_nCases, C.m_nMeasure);

  nMaxLogLikelihood = pMeasureList->GetMaxLogLikelihood(*m_pData);

  delete pMeasureList;
  return (nMaxLogLikelihood - m_pData->m_pModel->GetLogLikelihoodForTotal());
}



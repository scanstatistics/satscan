// ST+PTanalysis.cpp

#include "ST+PTanalysis.h"
#include "STCluster.h"
#include "PTData.h"
#include "STData.h"
#include "display.h"
#include "MeasureList.h"
#include "TimeEstimate.h"
#include "Model.h"

C_ST_PT_Analysis::C_ST_PT_Analysis(CParameters*  pParameters,
                                   CSaTScanData* pData)
                 :CSpaceTimeAnalysis(pParameters, pData)
{
}

C_ST_PT_Analysis::~C_ST_PT_Analysis()
{
}

void C_ST_PT_Analysis::SetMaxNumClusters()
{
  m_nMaxClusters = m_pData->m_nGridTracts+1;
}

void C_ST_PT_Analysis::FindTopClusters()
{
  CSpaceTimeAnalysis::FindTopClusters();

  tract_t nLastClusterIndex = m_nClustersRetained;
  m_pTopClusters[nLastClusterIndex] = GetTopPTCluster();

  SortTopClusters();
}

CPurelyTemporalCluster* C_ST_PT_Analysis::GetTopPTCluster()
{
  CPurelyTemporalCluster* MaxCluster = new CPurelyTemporalCluster
                                           (m_pParameters->m_bAliveClustersOnly,
                                            m_pData->m_nTimeIntervals,
                                            m_pData->m_nIntervalCut);

  CPurelyTemporalCluster* C_PT = new CPurelyTemporalCluster
                                     (m_pParameters->m_bAliveClustersOnly,
                                      m_pData->m_nTimeIntervals,
                                      m_pData->m_nIntervalCut);

  MaxCluster->SetLogLikelihood(m_pData->m_pModel->GetLogLikelihoodForTotal());

  C_PT->SetRate(m_pParameters->m_nAreas);
  C_PT->InitTimeIntervalIndeces();

  while (C_PT->SetNextTimeInterval(m_pData->m_pPTCases,
                                   m_pData->m_pPTMeasure))
  {
    if (C_PT->RateIsOfInterest(m_pData->m_nTotalCases, m_pData->m_nTotalMeasure))
    {
      C_PT->m_nLogLikelihood = m_pData->m_pModel->CalcLogLikelihood
                                                  (C_PT->m_nCases, C_PT->m_nMeasure);
      //C_PT->SetLogLikelihood(m_pData->m_nTotalCases, m_pData->m_nTotalMeasure);
      if (C_PT->m_nLogLikelihood > MaxCluster->m_nLogLikelihood)
        *MaxCluster = *C_PT;
    }
  }

  delete C_PT;

  if (MaxCluster->ClusterDefined())
  {
    MaxCluster->SetRatio(m_pData->m_pModel->GetLogLikelihoodForTotal());
    MaxCluster->SetStartAndEndDates(m_pData->m_pIntervalStartTimes,
                                    m_pData->m_nTimeIntervals);
  }
  
  m_nClustersRetained++;

  return MaxCluster;
}

/*void C_ST_PT_Analysis::MakeData()
{
  CAnalysis::MakeData();
  m_pData->SetPurelyTemporalSimCases();
}
*/
double C_ST_PT_Analysis::MonteCarlo()
{
  CMeasureList* pMeasureList;

  CPurelyTemporalCluster C_PT(m_pParameters->m_bAliveClustersOnly,
                              m_pData->m_nTimeIntervals,
                              m_pData->m_nIntervalCut);
  CSpaceTimeCluster      C_ST(m_pParameters->m_bAliveClustersOnly,
                              m_pData->m_nTimeIntervals,
                              m_pData->m_nIntervalCut);

  C_ST.SetRate(m_pParameters->m_nAreas);

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

  for (tract_t i = 0; i<m_pData->m_nGridTracts; i++)
  {
    C_ST.Initialize(i);

    for (tract_t j=1; j<=m_pData->m_NeighborCounts[i]; j++)
    {
      C_ST.AddNeighbor(*m_pData, m_pData->m_pSimCases, j);
      C_ST.InitTimeIntervalIndeces();
      while (C_ST.SetNextTimeInterval())
        pMeasureList->AddMeasure(C_ST.m_nCases, C_ST.m_nMeasure);
    }
  }

  C_PT.Initialize(0);
  C_PT.SetRate(m_pParameters->m_nAreas);
  C_PT.InitTimeIntervalIndeces();
  while (C_PT.SetNextTimeInterval(m_pData->m_pPTSimCases,
                                  m_pData->m_pPTMeasure))
    pMeasureList->AddMeasure(C_PT.m_nCases, C_PT.m_nMeasure);

  nMaxLogLikelihood = pMeasureList->GetMaxLogLikelihood(*m_pData);

  delete pMeasureList;
  return (nMaxLogLikelihood - m_pData->m_pModel->GetLogLikelihoodForTotal());
}



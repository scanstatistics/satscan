// ST+PSanalysis.cpp

#include "ST+PSanalysis.h"
#include "STCluster.h"
#include "PSCluster.h"
#include "display.h"
#include "MeasureList.h"
#include "Model.h"

C_ST_PS_Analysis::C_ST_PS_Analysis(CParameters*  pParameters,
                                   CSaTScanData* pData)
                 :CSpaceTimeAnalysis(pParameters, pData)
{
}

C_ST_PS_Analysis::~C_ST_PS_Analysis()
{
}

CCluster* C_ST_PS_Analysis::GetTopCluster(tract_t nCenter)
{
  CCluster* MaxCluster  = new CCluster;
  CCluster* OrigCluster = MaxCluster;

  CPurelySpatialCluster* C_PS     = new CPurelySpatialCluster;
  CPurelySpatialCluster* C_PS_MAX = new CPurelySpatialCluster;

  CSpaceTimeCluster* C_ST     = new CSpaceTimeCluster
                                    (m_pParameters->m_bAliveClustersOnly,
                                     m_pData->m_nTimeIntervals,
                                     m_pData->m_nIntervalCut);
  CSpaceTimeCluster* C_ST_MAX = new CSpaceTimeCluster
                                    (m_pParameters->m_bAliveClustersOnly,
                                     m_pData->m_nTimeIntervals,
                                     m_pData->m_nIntervalCut);

  MaxCluster->SetLogLikelihood(m_pData->m_pModel->GetLogLikelihoodForTotal());

  C_PS->SetCenter(nCenter);
  C_PS->SetRate(m_pParameters->m_nAreas);

  C_ST->SetCenter(nCenter);
  C_ST->SetRate(m_pParameters->m_nAreas);

  for (tract_t i=1; i<=m_pData->m_NeighborCounts[nCenter]; i++)
  {
    C_PS->AddNeighbor(*m_pData, m_pData->m_pCases, i);
    if (C_PS->RateIsOfInterest(m_pData->m_nTotalCases, m_pData->m_nTotalMeasure))
    {
      C_PS->m_nLogLikelihood = m_pData->m_pModel->CalcLogLikelihood
                                                  (C_PS->m_nCases, C_PS->m_nMeasure);
      //C_PS->SetLogLikelihood(m_pData->m_nTotalCases, m_pData->m_nTotalMeasure);
      if (C_PS->m_nLogLikelihood > MaxCluster->m_nLogLikelihood)
      {
        *C_PS_MAX  = *C_PS;
        MaxCluster = C_PS_MAX;
      }
    }

    C_ST->AddNeighbor(*m_pData, m_pData->m_pCases, i);
    C_ST->InitTimeIntervalIndeces();
    while (C_ST->SetNextTimeInterval())
    {

      if (C_ST->RateIsOfInterest(m_pData->m_nTotalCases, m_pData->m_nTotalMeasure))
      {
        C_ST->m_nLogLikelihood = m_pData->m_pModel->CalcLogLikelihood
                                                    (C_ST->m_nCases, C_ST->m_nMeasure);
        //C_ST->SetLogLikelihood(m_pData->m_nTotalCases, m_pData->m_nTotalMeasure);
        if (C_ST->m_nLogLikelihood > MaxCluster->m_nLogLikelihood)
        {
          *C_ST_MAX  = *C_ST;
          MaxCluster = C_ST_MAX;
        }
      }

    }

  }

  delete C_PS;
  delete C_ST;

  if (MaxCluster == OrigCluster)
  {
    delete C_PS_MAX;
    delete C_ST_MAX;
  }
  else
  {
    delete OrigCluster;
    if (MaxCluster == C_PS_MAX)
      delete C_ST_MAX;
    else
      delete C_PS_MAX;
  }

  MaxCluster->SetRatioAndDates(*m_pData);
  return MaxCluster;
}

double C_ST_PS_Analysis::MonteCarlo()
{
  CMeasureList* pMeasureList;

  CPurelySpatialCluster C_PS;
  CSpaceTimeCluster     C_ST(m_pParameters->m_bAliveClustersOnly,
                             m_pData->m_nTimeIntervals,
                             m_pData->m_nIntervalCut);

  C_PS.SetRate(m_pParameters->m_nAreas);
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
    C_PS.Initialize(i);
    C_ST.Initialize(i);

    for (tract_t j=1; j<=m_pData->m_NeighborCounts[i]; j++)
    {
      C_PS.AddNeighbor(*m_pData, m_pData->m_pSimCases, j);
      pMeasureList->AddMeasure(C_PS.m_nCases, C_PS.m_nMeasure);

      C_ST.AddNeighbor(*m_pData, m_pData->m_pSimCases, j);
      C_ST.InitTimeIntervalIndeces();
      while (C_ST.SetNextTimeInterval())
        pMeasureList->AddMeasure(C_ST.m_nCases, C_ST.m_nMeasure);

    }
  }

  nMaxLogLikelihood = pMeasureList->GetMaxLogLikelihood(*m_pData);

  delete pMeasureList;
  return (nMaxLogLikelihood - m_pData->m_pModel->GetLogLikelihoodForTotal());
}



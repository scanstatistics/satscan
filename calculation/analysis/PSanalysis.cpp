// PSAnalysis.cpp

#include <time.h>
#include "PSAnalysis.h"
#include "PSCluster.h"
#include "MeasureList.h"
#include "LogLikelihood.h"
#include "model.h"

CPurelySpatialAnalysis::CPurelySpatialAnalysis(CParameters*  pParameters,
                                               CSaTScanData* pData)
                       :CAnalysis(pParameters, pData)
{
}

CPurelySpatialAnalysis::~CPurelySpatialAnalysis()
{
}

CCluster* CPurelySpatialAnalysis::GetTopCluster(tract_t nCenter)
{
  CPurelySpatialCluster* MaxCluster = new CPurelySpatialCluster;
  CPurelySpatialCluster* C          = new CPurelySpatialCluster;

  MaxCluster->SetLogLikelihood(m_pData->m_pModel->GetLogLikelihoodForTotal());

  C->SetCenter(nCenter);
  C->SetRate(m_pParameters->m_nAreas);

  for (int i=1; i<=m_pData->m_NeighborCounts[nCenter]; i++)
  {
    C->AddNeighbor(*m_pData, m_pData->m_pCases, i);
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

double CPurelySpatialAnalysis::MonteCarlo()
{
  CMeasureList* pMeasureList;
  CPurelySpatialCluster C;
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

  for (tract_t i = 0; i<m_pData->m_nGridTracts; i++)
  {
    C.Initialize(i);
    for (tract_t j=1; j<=m_pData->m_NeighborCounts[i]; j++)
    {
      C.AddNeighbor(*m_pData, m_pData->m_pSimCases, j);
      pMeasureList->AddMeasure(C.m_nCases, C.m_nMeasure);
    }
  }

  nMaxLogLikelihood = pMeasureList->GetMaxLogLikelihood(*m_pData);

  delete pMeasureList;
  return (nMaxLogLikelihood - m_pData->m_pModel->GetLogLikelihoodForTotal());
}



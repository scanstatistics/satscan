// STanalysis.cpp

#include "STanalysis.h"
#include "STCluster.h"
#include "display.h"
#include "MeasureList.h"
#include "Model.h"

CSpaceTimeAnalysis::CSpaceTimeAnalysis(CParameters*  pParameters,
                                       CSaTScanData* pData)
                   :CAnalysis(pParameters, pData)
{
}

CSpaceTimeAnalysis::~CSpaceTimeAnalysis()
{
}

CCluster* CSpaceTimeAnalysis::GetTopCluster(tract_t nCenter)
{
  CSpaceTimeCluster* MaxCluster=new CSpaceTimeCluster(m_pParameters->m_bAliveClustersOnly,
                                                      m_pData->m_nTimeIntervals,
                                                      m_pData->m_nIntervalCut);
  CSpaceTimeCluster* C         =new CSpaceTimeCluster(m_pParameters->m_bAliveClustersOnly,
                                                      m_pData->m_nTimeIntervals,
                                                      m_pData->m_nIntervalCut);

  MaxCluster->CCluster::SetLogLikelihood(m_pData->m_pModel->GetLogLikelihoodForTotal());

  C->SetCenter(nCenter);
  C->SetRate(m_pParameters->m_nAreas);

  for (tract_t i=1; i<=m_pData->m_NeighborCounts[nCenter]; i++)
  {
    C->AddNeighbor(*m_pData, m_pData->m_pCases, i);
    C->InitTimeIntervalIndeces();
    while (C->SetNextTimeInterval())
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
  }

  delete C;
  MaxCluster->SetRatioAndDates(*m_pData);
  return MaxCluster;
}

double CSpaceTimeAnalysis::MonteCarlo()
{
  CMeasureList* pMeasureList;

  CSpaceTimeCluster C(m_pParameters->m_bAliveClustersOnly,
                      m_pData->m_nTimeIntervals,
                      m_pData->m_nIntervalCut);;

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

//  #if DEBUGANALYSIS
//  pMeasureList->Display(m_pDebugFile);
//  #endif

  long lCounter = 0;
  for (tract_t i = 0; i<m_pData->m_nGridTracts; i++)
  {
    C.Initialize(i);

    for (tract_t j=1; j<=m_pData->m_NeighborCounts[i]; j++)
    {
//      printf("****** Tract = %i ******\n", j);

      C.AddNeighbor(*m_pData, m_pData->m_pSimCases, j);
      C.InitTimeIntervalIndeces();
      while (C.SetNextTimeInterval())
	  {
		  lCounter ++;
        pMeasureList->AddMeasure(C.m_nCases, C.m_nMeasure);
	  }
    }
  }


//  #if DEBUGANALYSIS
//  pMeasureList->Display(m_pDebugFile);
//  m_pData->DisplayCases(m_pDebugFile);
//  #endif

  nMaxLogLikelihood = pMeasureList->GetMaxLogLikelihood(*m_pData);

  delete pMeasureList;
  return (nMaxLogLikelihood - m_pData->m_pModel->GetLogLikelihoodForTotal());
}



#include "SaTScan.h"
#pragma hdrstop
#include "PurelyTemporalAnalysis.h"

CPurelyTemporalAnalysis::CPurelyTemporalAnalysis(CParameters*  pParameters,
                                                 CSaTScanData* pData,
                                                 BasePrint *pPrintDirection)
                        :CAnalysis(pParameters, pData, pPrintDirection)
{
}

CPurelyTemporalAnalysis::~CPurelyTemporalAnalysis()
{
}

//FAST FUNCTION - DO NOT NEED TO ADD CANCEL CHECK HERE.
bool CPurelyTemporalAnalysis::FindTopClusters()
{
   bool bOk = true;

   try
      {
#ifdef DEBUGANALYSIS
      m_pData->CSaTScanData::DisplayCases(m_pDebugFile);
      m_pData->CSaTScanData::DisplayMeasure(m_pDebugFile);
#endif

      gpPrintDirection->SatScanPrintf("Find top cluster.\n");
      m_pTopClusters[0] = GetTopCluster(0);
      if (m_pTopClusters[0]->ClusterDefined())
         m_nClustersRetained++;
      else
         {
         delete m_pTopClusters[0];
         m_pTopClusters[0] = NULL;
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("FindTopClusters()", "CPurelyTemporalAnalysis");
      throw;
      }
    return bOk;
}

CCluster* CPurelyTemporalAnalysis::GetTopCluster(tract_t nCenter)
{
   CPurelyTemporalCluster* MaxCluster = 0;
   CPurelyTemporalCluster* C = 0;
   bool  bAliveCluster;

   try
      {
      // if Prospective Space-Time then Alive Clusters Only.
      if (m_pParameters->m_nAnalysisType == PROSPECTIVESPACETIME)
         bAliveCluster = true;
      else
         bAliveCluster = m_pParameters->m_bAliveClustersOnly;

      gpPrintDirection->SatScanPrintf("Get Top P.T. Cluster.\n");
      MaxCluster = new CPurelyTemporalCluster
                       (bAliveCluster,
                        m_pData->m_nTimeIntervals,
                        m_pData->m_nIntervalCut,
                        gpPrintDirection);
      C          = new CPurelyTemporalCluster
                       (bAliveCluster,
                        m_pData->m_nTimeIntervals,
                        m_pData->m_nIntervalCut,
                        gpPrintDirection);
    
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
    
      delete C; C = 0;
    
      MaxCluster->SetRatioAndDates(*m_pData);
      }
   catch (SSException & x)
      {
      delete C; C = 0;
      delete MaxCluster;
      x.AddCallpath("GetTopCluster()", "CPurelyTemporalAnalysis");
      throw;
      }
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
   CMeasureList* pMeasureList = 0;
   double nMaxLogLikelihood;

   try
      {
      CPurelyTemporalCluster C(m_pParameters->m_bAliveClustersOnly,
                               m_pData->m_nTimeIntervals,
                               m_pData->m_nIntervalCut,
                               gpPrintDirection);
    
      C.SetRate(m_pParameters->m_nAreas);
    
      nMaxLogLikelihood = m_pData->m_pModel->GetLogLikelihoodForTotal();
    
      switch (m_pParameters->m_nAreas)
      {
        case HIGH      : pMeasureList = new CMinMeasureList(m_pData->m_nTotalCases, m_pData->m_nTotalMeasure, gpPrintDirection);
                         break;
        case LOW       : pMeasureList = new CMaxMeasureList(m_pData->m_nTotalCases, m_pData->m_nTotalMeasure, gpPrintDirection);
                         break;
        case HIGHANDLOW: pMeasureList = new CMinMaxMeasureList(m_pData->m_nTotalCases, m_pData->m_nTotalMeasure, gpPrintDirection);
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
      }
   catch (SSException & x)
      {
      delete pMeasureList;
      x.AddCallpath("MonteCarlo()", "CPurelyTemporalAnalysis");
      throw;
      }
  return (nMaxLogLikelihood - m_pData->m_pModel->GetLogLikelihoodForTotal());
}

//same as MonteCarlo()
double CPurelyTemporalAnalysis::MonteCarloProspective()
{
   CMeasureList* pMeasureList = 0;
   double nMaxLogLikelihood;

   try
      {
      CPurelyTemporalCluster C(m_pParameters->m_bAliveClustersOnly,
                               m_pData->m_nTimeIntervals,
                               m_pData->m_nIntervalCut,
                               gpPrintDirection);
    
      C.SetRate(m_pParameters->m_nAreas);
    
      nMaxLogLikelihood = m_pData->m_pModel->GetLogLikelihoodForTotal();
    
      switch (m_pParameters->m_nAreas)
      {
        case HIGH      : pMeasureList = new CMinMeasureList(m_pData->m_nTotalCases, m_pData->m_nTotalMeasure, gpPrintDirection);
                         break;
        case LOW       : pMeasureList = new CMaxMeasureList(m_pData->m_nTotalCases, m_pData->m_nTotalMeasure, gpPrintDirection);
                         break;
        case HIGHANDLOW: pMeasureList = new CMinMaxMeasureList(m_pData->m_nTotalCases, m_pData->m_nTotalMeasure, gpPrintDirection);
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
      }
   catch (SSException & x)
      {
      delete pMeasureList;
      x.AddCallpath("MonteCarlo()", "CPurelyTemporalAnalysis");
      throw;
      }
  return (nMaxLogLikelihood - m_pData->m_pModel->GetLogLikelihoodForTotal());
}

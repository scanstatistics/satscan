#include "SaTScan.h"
#pragma hdrstop
#include "SpaceTimeAnalysis.h"

CSpaceTimeAnalysis::CSpaceTimeAnalysis(CParameters*  pParameters,
                                       CSaTScanData* pData,
                                       BasePrint *pPrintDirection)
                   :CAnalysis(pParameters, pData, pPrintDirection)
{
}

CSpaceTimeAnalysis::~CSpaceTimeAnalysis()
{
}

CCluster* CSpaceTimeAnalysis::GetTopCluster(tract_t nCenter)
{
   CSpaceTimeCluster* MaxCluster = 0;
   CSpaceTimeCluster* C          = 0;
   bool  bAliveCluster;

   try
      {
      // if Prospective Space-Time then Alive Clusters Only.
      if (m_pParameters->m_nAnalysisType == PROSPECTIVESPACETIME)
         bAliveCluster = true;
      else
         bAliveCluster = m_pParameters->m_bAliveClustersOnly;

      MaxCluster=new CSpaceTimeCluster(bAliveCluster,
                                       m_pData->m_nTimeIntervals,
                                       m_pData->m_nIntervalCut,
                                       gpPrintDirection);
     /* C         =new CSpaceTimeCluster(bAliveCluster,
                                       m_pData->m_nTimeIntervals,
                                       m_pData->m_nIntervalCut,
                                       gpPrintDirection);  */

      MaxCluster->CCluster::SetLogLikelihood(m_pData->m_pModel->GetLogLikelihoodForTotal());
    
     // C->SetCenter(nCenter);
     // C->SetRate(m_pParameters->m_nAreas);

      for (int k = 0; k <= m_pParameters->m_lTotalNumEllipses; k++)   //circle is 0 offset... (always there)
         {
         C         =new CSpaceTimeCluster(bAliveCluster,
                                       m_pData->m_nTimeIntervals,
                                       m_pData->m_nIntervalCut,
                                       gpPrintDirection);
         C->SetCenter(nCenter);
         C->SetRate(m_pParameters->m_nAreas);
         C->SetEllipseOffset(k);

         for (tract_t i=1; i<=m_pData->m_NeighborCounts[k][nCenter]; i++)     // k use to be "0"
         {
           C->AddNeighbor(k, *m_pData, m_pData->m_pCases, i);                 // k use to be "0"
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
        delete C;  C = 0;
      }

      //delete C;  C = 0;
      MaxCluster->SetRatioAndDates(*m_pData);
      }
   catch (SSException & x)
      {
      delete C;
      delete MaxCluster;
      x.AddCallpath("GetTopCluster()", "CSpaceTimeAnalysis");
      throw;
      }
  return MaxCluster;
}

double CSpaceTimeAnalysis::MonteCarlo()
{
   CMeasureList* pMeasureList;
   double nMaxLogLikelihood;
   int k;
   tract_t j, i;
   
   try
      {
      CSpaceTimeCluster C(m_pParameters->m_bAliveClustersOnly,
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
    
       // #ifdef DEBUGANALYSIS
      //  pMeasureList->Display(m_pDebugFile);
      //  #endif

      for (k = 0; k <= m_pParameters->m_lTotalNumEllipses; k++)   //circle is 0 offset... (always there)
         {
         for (i = 0; i<m_pData->m_nGridTracts; i++)
           {
           C.Initialize(i);

           for (j=1; j<=m_pData->m_NeighborCounts[k][i]; j++)   // k use to be "0"
             {
             C.AddNeighbor(k, *m_pData, m_pData->m_pSimCases, j);    // k use to be "0"
             C.InitTimeIntervalIndeces();
             while (C.SetNextTimeInterval())
               pMeasureList->AddMeasure(C.m_nCases, C.m_nMeasure);
             }
          }
      }

     //   #ifdef DEBUGANALYSIS
     //   pMeasureList->Display(m_pDebugFile);
     //   m_pData->DisplayCases(m_pDebugFile);
     //   #endif

      nMaxLogLikelihood = pMeasureList->GetMaxLogLikelihood(*m_pData);

      delete pMeasureList;
      }
   catch (SSException & x)
      {
      x.AddCallpath("MonteCarlo()", "CSpaceTimeAnalysis");
      throw;
      }
  return (nMaxLogLikelihood - m_pData->m_pModel->GetLogLikelihoodForTotal());
}

double CSpaceTimeAnalysis::MonteCarloProspective()
{
   CMeasureList* pMeasureList;
   double nMaxLogLikelihood;
   long lTime;
   Julian jCurrentDate;
   int iThisStartInterval, n, m;
  
   try
      {
      //for prospective Space-Time, m_bAliveClustersOnly should be false..
      //m_bAliveClustersOnly is the first parameter into the CSpaceTimeCluster class
      CSpaceTimeCluster C(false,
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

#ifdef DEBUGPROSPECTIVETIME
      pMeasureList->Display(m_pDebugFile);
#endif

      for (int k = 0; k <= m_pParameters->m_lTotalNumEllipses; k++)   //circle is 0 offset... (always there)
         {
         for (tract_t i = 0; i<m_pData->m_nGridTracts; i++)
           {
           C.Initialize(i);

           for (tract_t j=1; j<=m_pData->m_NeighborCounts[k][i]; j++)   // k use to be "0"
             {
             C.AddNeighbor(k, *m_pData, m_pData->m_pSimCases, j);    // k use to be "0"
             //Need to keep track of the current date as you loop through intervals
             jCurrentDate = m_pData->m_nEndDate;
             // Loop from study end date back to Prospective start date -- loop by interval
             for (n = m_pData->m_nTimeIntervals; n >= m_pData->m_nProspectiveIntervalStart; n--)
                {
                //Need to re-compute duration by using current date (whatever date loop "n" is at)
                //and the Begin Study Date.  Must use time percent specified by user...
                iThisStartInterval = m_pData->ComputeNewCutoffInterval(m_pData->m_nStartDate,jCurrentDate);

                //Now compute a new Current Date by subtracting the interval duration
                jCurrentDate = DecrementDate(jCurrentDate, m_pParameters->m_nIntervalUnits, m_pParameters->m_nIntervalLength);

                C.InitTimeIntervalIndeces(iThisStartInterval, n);
                while (C.SetNextProspTimeInterval())
                   pMeasureList->AddMeasure(C.m_nCases, C.m_nMeasure);
                }
             }
           }
         }

#ifdef DEBUGPROSPECTIVETIME
      pMeasureList->Display(m_pDebugFile);
      m_pData->DisplayCases(m_pDebugFile);
#endif

      nMaxLogLikelihood = pMeasureList->GetMaxLogLikelihood(*m_pData);

      delete pMeasureList;
      }
   catch (SSException & x)
      {
      x.AddCallpath("MonteCarloProspective()", "CSpaceTimeAnalysis");
      throw;
      }
  return (nMaxLogLikelihood - m_pData->m_pModel->GetLogLikelihoodForTotal());
}



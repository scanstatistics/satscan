#pragma hdrstop
#include "SpaceTimeIncludePurelyTemporalAnalysis.h"

C_ST_PT_Analysis::C_ST_PT_Analysis(CParameters*  pParameters,
                                   CSaTScanData* pData, BasePrint *pPrintDirection)
                 :CSpaceTimeAnalysis(pParameters, pData, pPrintDirection)
{
}

C_ST_PT_Analysis::~C_ST_PT_Analysis()
{
}

void C_ST_PT_Analysis::SetMaxNumClusters()
{
   m_nMaxClusters = m_pData->m_nGridTracts+1;
}

bool C_ST_PT_Analysis::FindTopClusters()
{
   try
      {
      if (!CSpaceTimeAnalysis::FindTopClusters())
         return false;

      tract_t nLastClusterIndex = m_nClustersRetained;
      m_pTopClusters[nLastClusterIndex] = GetTopPTCluster();

      SortTopClusters();
      }
   catch (SSException & x)
      {
      x.AddCallpath("FindTopClusters()", "C_ST_PT_Analysis");
      throw;
      }
  return true;
}

CPurelyTemporalCluster* C_ST_PT_Analysis::GetTopPTCluster()
{
   CPurelyTemporalCluster* MaxCluster = 0;
   CPurelyTemporalCluster* C_PT       = 0;
   bool bAliveCluster;

   try
      {
      // if Prospective Space-Time then Alive Clusters Only.
      if (m_pParameters->m_nAnalysisType == PROSPECTIVESPACETIME)
         bAliveCluster = true;
      else
         bAliveCluster = m_pParameters->m_bAliveClustersOnly;

      MaxCluster = new CPurelyTemporalCluster
                      (bAliveCluster,
                       m_pData->m_nTimeIntervals,
                       m_pData->m_nIntervalCut,
                       gpPrintDirection);

      C_PT = new CPurelyTemporalCluster
                      (bAliveCluster,
                       m_pData->m_nTimeIntervals,
                       m_pData->m_nIntervalCut,
                       gpPrintDirection);

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
     
      delete C_PT;   C_PT = 0;
     
      if (MaxCluster->ClusterDefined())
      {
        MaxCluster->SetRatio(m_pData->m_pModel->GetLogLikelihoodForTotal());
        MaxCluster->SetStartAndEndDates(m_pData->m_pIntervalStartTimes,
                                        m_pData->m_nTimeIntervals);
      }
      
      m_nClustersRetained++;
      }
   catch (SSException & x)
      {
      delete C_PT;
      delete MaxCluster;
      x.AddCallpath("GetTopPTCluster()", "C_ST_PT_Analysis");
      throw;
      }
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
   CMeasureList* pMeasureList = 0;
   double nMaxLogLikelihood;

   try
      {
      CPurelyTemporalCluster C_PT(m_pParameters->m_bAliveClustersOnly,
                                  m_pData->m_nTimeIntervals,
                                  m_pData->m_nIntervalCut,
                                  gpPrintDirection);
      CSpaceTimeCluster      C_ST(m_pParameters->m_bAliveClustersOnly,
                                  m_pData->m_nTimeIntervals,
                                  m_pData->m_nIntervalCut,
                                  gpPrintDirection);
    
      C_ST.SetRate(m_pParameters->m_nAreas);
    
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
    
      for (int k = 0; k <= m_pParameters->m_lTotalNumEllipses; k++)   //circle is 0 offset... (always there)
         {
         for (tract_t i = 0; i<m_pData->m_nGridTracts; i++)
         {
           C_ST.Initialize(i);
    
           for (tract_t j=1; j<=m_pData->m_NeighborCounts[k][i]; j++)   // k use to be "0"
           {
             C_ST.AddNeighbor(k, *m_pData, m_pData->m_pSimCases, j);     // k use to be "0"
             C_ST.InitTimeIntervalIndeces();
             while (C_ST.SetNextTimeInterval())
               pMeasureList->AddMeasure(C_ST.m_nCases, C_ST.m_nMeasure);
           }
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
      }
   catch (SSException & x)
      {
      delete pMeasureList;
      x.AddCallpath("MonteCarlo()", "C_ST_PT_Analysis");
      throw;
      }
  return (nMaxLogLikelihood - m_pData->m_pModel->GetLogLikelihoodForTotal());
}

double C_ST_PT_Analysis::MonteCarloProspective()
{
   CMeasureList* pMeasureList = 0;
   double nMaxLogLikelihood;
   long lTime;
   Julian jCurrentDate;
   int iThisStartInterval, n, m;

   try
      {
      //for prospective Space-Time, m_bAliveClustersOnly should be false..
      //m_bAliveClustersOnly is the first parameter into the CSpaceTimeCluster class
      CPurelyTemporalCluster C_PT(false,
                                  m_pData->m_nTimeIntervals,
                                  m_pData->m_nIntervalCut,
                                  gpPrintDirection);
      CSpaceTimeCluster      C_ST(false,
                                  m_pData->m_nTimeIntervals,
                                  m_pData->m_nIntervalCut,
                                  gpPrintDirection);
    
      C_ST.SetRate(m_pParameters->m_nAreas);
    
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
    
      for (int k = 0; k <= m_pParameters->m_lTotalNumEllipses; k++)   //circle is 0 offset... (always there)
         {
         for (tract_t i = 0; i<m_pData->m_nGridTracts; i++)
           {
           C_ST.Initialize(i);

           for (tract_t j=1; j<=m_pData->m_NeighborCounts[k][i]; j++)   // k use to be "0"
             {
             C_ST.AddNeighbor(k, *m_pData, m_pData->m_pSimCases, j);    // k use to be "0"
             //Need to keep track of the current date as you loop through intervals
             jCurrentDate = m_pData->m_nEndDate;
             // Loop from study end date back to Prospective start date -- loop by interval
             for (n = m_pData->m_nTimeIntervals; n >= m_pData->m_nProspectiveIntervalStart; n--)
                {
                //Need to re-compute duration due to by using current date (whatever date loop "n" is at)
                //and the Begin Study Date
                iThisStartInterval = m_pData->ComputeNewCutoffInterval(m_pData->m_nStartDate,jCurrentDate);

                //Now compute a new Current Date by subtracting the interval duration
                jCurrentDate = DecrementDate(jCurrentDate, m_pParameters->m_nIntervalUnits, m_pParameters->m_nIntervalLength);

                C_ST.InitTimeIntervalIndeces(iThisStartInterval, n);
                while (C_ST.SetNextProspTimeInterval())
                   pMeasureList->AddMeasure(C_ST.m_nCases, C_ST.m_nMeasure);
                }
             }
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
      }
   catch (SSException & x)
      {
      delete pMeasureList;
      x.AddCallpath("MonteCarlo()", "C_ST_PT_Analysis");
      throw;
      }
  return (nMaxLogLikelihood - m_pData->m_pModel->GetLogLikelihoodForTotal());
}

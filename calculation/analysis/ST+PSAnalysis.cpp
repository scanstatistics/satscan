// ST+PSanalysis.cpp

#include "ST+PSanalysis.h"
#include "STCluster.h"
#include "PSCluster.h"
#include "display.h"
#include "MeasureList.h"
#include "Model.h"

C_ST_PS_Analysis::C_ST_PS_Analysis(CParameters*  pParameters,
                                   CSaTScanData* pData, BasePrint *pPrintDirection)
                 :CSpaceTimeAnalysis(pParameters, pData, pPrintDirection)
{
}

C_ST_PS_Analysis::~C_ST_PS_Analysis()
{
}

CCluster* C_ST_PS_Analysis::GetTopCluster(tract_t nCenter)
{
   int j;
   tract_t i;
   CCluster* MaxCluster            = 0;
   CPurelySpatialCluster* C_PS_MAX = 0;
   CSpaceTimeCluster* C_ST_MAX     = 0;
   CPurelySpatialCluster* C_PS     = 0;
   CSpaceTimeCluster* C_ST         = 0;
   CCluster* OrigCluster           = 0;
   bool      bAliveCluster;

   try
      {
      MaxCluster  = new CCluster(gpPrintDirection);
      OrigCluster = MaxCluster;

      // if Prospective Space-Time then Alive Clusters Only.
      if (m_pParameters->m_nAnalysisType == PROSPECTIVESPACETIME)
         bAliveCluster = true;
      else
         bAliveCluster = m_pParameters->m_bAliveClustersOnly;

      //  CPurelySpatialCluster* C_PS     = new CPurelySpatialCluster;
      C_PS_MAX = new CPurelySpatialCluster(gpPrintDirection);
    
      // CSpaceTimeCluster* C_ST     = new CSpaceTimeCluster
      //                                   (m_pParameters->m_bAliveClustersOnly,
      //                                    m_pData->m_nTimeIntervals,
      //                                    m_pData->m_nIntervalCut);
      C_ST_MAX = new CSpaceTimeCluster
                     (bAliveCluster,
                      m_pData->m_nTimeIntervals,
                      m_pData->m_nIntervalCut,
                      gpPrintDirection);
    
      MaxCluster->SetLogLikelihood(m_pData->m_pModel->GetLogLikelihoodForTotal());
    
      //C_PS->SetCenter(nCenter);
      //C_PS->SetRate(m_pParameters->m_nAreas);
    
      //C_ST->SetCenter(nCenter);
      //C_ST->SetRate(m_pParameters->m_nAreas);
    
      for (j = 0 ; j <= m_pParameters->m_lTotalNumEllipses; j++)
         {
         //set some counts back to zero
         /*C_PS->m_nCases = 0;                     // these initializations are new...
         C_PS->m_nMeasure = 0;
         C_PS->m_nTracts = 0;                    // DTG - POSSIBLY RESET ALL OF THE CLUSTER GLOBALS ???
         C_ST->m_nCases = 0;
         C_ST->m_nMeasure = 0;
         C_ST->m_nTracts = 0;
         C_ST->ResetCummulatives();*/
    
         C_PS     = new CPurelySpatialCluster(gpPrintDirection);
         C_ST     = new CSpaceTimeCluster
                        (bAliveCluster,
                         m_pData->m_nTimeIntervals,
                         m_pData->m_nIntervalCut,
                         gpPrintDirection);
         C_PS->SetCenter(nCenter);
         C_PS->SetRate(m_pParameters->m_nAreas);
    
         C_ST->SetCenter(nCenter);
         C_ST->SetRate(m_pParameters->m_nAreas);
    
    
         for (i=1; i<=m_pData->m_NeighborCounts[j][nCenter]; i++)                 // update this later
           {
           C_PS->AddNeighbor(j, *m_pData, m_pData->m_pCases, i);                          // update this later
           if (C_PS->RateIsOfInterest(m_pData->m_nTotalCases, m_pData->m_nTotalMeasure))
             {
             C_PS->m_nLogLikelihood = m_pData->m_pModel->CalcLogLikelihood
                                                 (C_PS->m_nCases, C_PS->m_nMeasure);
             //C_PS->SetLogLikelihood(m_pData->m_nTotalCases, m_pData->m_nTotalMeasure);
             if (C_PS->m_nLogLikelihood > MaxCluster->m_nLogLikelihood)
               {
               *C_PS_MAX  = *C_PS;
               MaxCluster = C_PS_MAX;
               // KEEP MORE INFORMATION HERE ??   LIKE WHICH ELLIPSOID WAS SELECTED, IF ONE WAS....
               MaxCluster->m_iEllipseOffset = j;
               }
             }
    
           C_ST->AddNeighbor(j, *m_pData, m_pData->m_pCases, i);                            // update this later
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
                 MaxCluster->m_iEllipseOffset = j;
                 }
               }
             }
           }
         delete C_PS; C_PS = 0;
         delete C_ST; C_ST = 0;
         }
    
      //delete C_PS;
      //delete C_ST;
    
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
      }
   catch (SSException & x)
      {
      // do not need to delete MaxCluster... is one of the objects below
      //
      delete C_PS; C_PS = 0;
      delete C_ST; C_ST = 0;
      delete C_PS_MAX;C_PS_MAX=0;
      delete C_ST_MAX;C_ST_MAX=0;
      delete OrigCluster;
      x.AddCallpath("GetTopCluster()", "C_ST_PS_Analysis");
      throw;
      }
  return MaxCluster;
}

double C_ST_PS_Analysis::MonteCarlo()
{
  CMeasureList* pMeasureList = 0;
  CPurelySpatialCluster C_PS(gpPrintDirection);
  double nMaxLogLikelihood;
  
  try
     {
     CSpaceTimeCluster     C_ST(m_pParameters->m_bAliveClustersOnly,
                                m_pData->m_nTimeIntervals,
                                m_pData->m_nIntervalCut,
                                gpPrintDirection);
    
     C_PS.SetRate(m_pParameters->m_nAreas);
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
          C_PS.Initialize(i);
          C_ST.Initialize(i);
    
          for (tract_t j=1; j<=m_pData->m_NeighborCounts[k][i]; j++)    //k use to be "0"
          {
            C_PS.AddNeighbor(k, *m_pData, m_pData->m_pSimCases, j);     //k use to be "0"
            pMeasureList->AddMeasure(C_PS.m_nCases, C_PS.m_nMeasure);
    
            C_ST.AddNeighbor(k, *m_pData, m_pData->m_pSimCases, j);      //k use to be "0"
            C_ST.InitTimeIntervalIndeces();
            while (C_ST.SetNextTimeInterval())
              pMeasureList->AddMeasure(C_ST.m_nCases, C_ST.m_nMeasure);
          }
    
       }
     }
    
     nMaxLogLikelihood = pMeasureList->GetMaxLogLikelihood(*m_pData);
    
     delete pMeasureList;
      }
   catch (SSException & x)
      {
      delete pMeasureList;
      x.AddCallpath("MonteCarlo()", "C_ST_PS_Analysis");
      throw;
      }
  return (nMaxLogLikelihood - m_pData->m_pModel->GetLogLikelihoodForTotal());
}


double C_ST_PS_Analysis::MonteCarloProspective()
{
  CMeasureList* pMeasureList = 0;
  CPurelySpatialCluster C_PS(gpPrintDirection);
  double nMaxLogLikelihood;
  long lTime;
  Julian jCurrentDate;
  int iThisStartInterval, n, m;

  try
     {
     //for prospective Space-Time, m_bAliveClustersOnly should be false..
     //m_bAliveClustersOnly is the first parameter into the CSpaceTimeCluster class
     CSpaceTimeCluster     C_ST(false,
                                m_pData->m_nTimeIntervals,
                                m_pData->m_nIntervalCut,
                                gpPrintDirection);
    
     C_PS.SetRate(m_pParameters->m_nAreas);
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
           C_PS.Initialize(i);
           C_ST.Initialize(i);

           for (tract_t j=1; j<=m_pData->m_NeighborCounts[k][i]; j++)   // k use to be "0"
             {
             C_PS.AddNeighbor(k, *m_pData, m_pData->m_pSimCases, j);     //k use to be "0"
             pMeasureList->AddMeasure(C_PS.m_nCases, C_PS.m_nMeasure);


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

     nMaxLogLikelihood = pMeasureList->GetMaxLogLikelihood(*m_pData);
    
     delete pMeasureList;
      }
   catch (SSException & x)
      {
      delete pMeasureList;
      x.AddCallpath("MonteCarloProspective()", "C_ST_PS_Analysis");
      throw;
      }
  return (nMaxLogLikelihood - m_pData->m_pModel->GetLogLikelihoodForTotal());
}



#include "SaTScan.h"
#pragma hdrstop
#include "PurelySpatialAnalysis.h"

CPurelySpatialAnalysis::CPurelySpatialAnalysis(CParameters*  pParameters,
                                               CSaTScanData* pData,
                                               BasePrint *pPrintDirection)
                       :CAnalysis(pParameters, pData, pPrintDirection)
{
}

CPurelySpatialAnalysis::~CPurelySpatialAnalysis()
{
}

CCluster* CPurelySpatialAnalysis::GetTopCluster(tract_t nCenter)
{
   int i, j;
   CPurelySpatialCluster* MaxCluster;
   CPurelySpatialCluster* C = 0;

   try
      {

      MaxCluster = new CPurelySpatialCluster(gpPrintDirection);

      MaxCluster->SetLogLikelihood(m_pData->m_pModel->GetLogLikelihoodForTotal());

      //***************************************************************************
      // Need to create a new cluster object for each circle or ellipse.
      // If you do not, then AddNeighbor creates invalid case counts as you tally
      // over multiple objects (i.e. the circle and ellipses.
      //***************************************************************************
      for (j = 0; j <= m_pParameters->m_lTotalNumEllipses; j++)   //circle is 0 offset... (always there)
         {
         C = new CPurelySpatialCluster(gpPrintDirection);
         C->SetCenter(nCenter);
         C->SetRate(m_pParameters->m_nAreas);
         C->SetEllipseOffset(j);                       // store the ellipse link in the cluster obj
    
         for (i=1; i<=m_pData->m_NeighborCounts[j][nCenter]; i++)
           {
           C->AddNeighbor(j, *m_pData, m_pData->m_pCases, i);
           if (C->RateIsOfInterest(m_pData->m_nTotalCases, m_pData->m_nTotalMeasure))
             {
             C->m_nLogLikelihood = m_pData->m_pModel->CalcLogLikelihood(C->m_nCases, C->m_nMeasure);
             //C->SetLogLikelihood(m_pData->m_nTotalCases, m_pData->m_nTotalMeasure);
             if (C->m_nLogLikelihood > MaxCluster->m_nLogLikelihood)
                {
                *MaxCluster = *C;
                // Is it the circle or one of the ellipses ??
    
                }
             }
           }
           delete C; C = 0;
         }
    
      MaxCluster->SetRatioAndDates(*m_pData);
      }
   catch (SSException & x)
      {
      delete C;
      x.AddCallpath("GetTopCluster()", "CPurelySpatialAnalysis");
      throw;
      }
   return MaxCluster;
}

double CPurelySpatialAnalysis::MonteCarlo()
{
   CMeasureList               * pMeasureList = 0;
   CPurelySpatialCluster        C(gpPrintDirection);
   double                       nMaxLogLikelihood;
   tract_t                      i, j;

   try
      {
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
    
      for (int k = 0; k <= m_pParameters->m_lTotalNumEllipses; k++)   //circle is 0 offset... (always there)
         {
         for (i=0; i<m_pData->m_nGridTracts; i++)
         {
           C.Initialize(i);
           for (j=1; j<=m_pData->m_NeighborCounts[k][i]; j++)   //k use to be "0"
           {
             C.AddNeighbor(k, *m_pData, m_pData->m_pSimCases, j);       //k use to be "0"
             pMeasureList->AddMeasure(C.m_nCases, C.m_nMeasure);
           }
        }
      }
    
      nMaxLogLikelihood = pMeasureList->GetMaxLogLikelihood(*m_pData);
    
      delete pMeasureList;
      }
   catch (SSException & x)
      {
      delete pMeasureList;
      x.AddCallpath("MonteCarlo()", "CPurelySpatialAnalysis");
      throw;
      }
  return (nMaxLogLikelihood - m_pData->m_pModel->GetLogLikelihoodForTotal());
}

//same as MonteCarlo()
double CPurelySpatialAnalysis::MonteCarloProspective()
{
   CMeasureList* pMeasureList = 0;
   CPurelySpatialCluster C(gpPrintDirection);
   double nMaxLogLikelihood;

   try
      {
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
    
      for (int k = 0; k <= m_pParameters->m_lTotalNumEllipses; k++)   //circle is 0 offset... (always there)
         {
         for (tract_t i = 0; i<m_pData->m_nGridTracts; i++)
         {
           C.Initialize(i);
           for (tract_t j=1; j<=m_pData->m_NeighborCounts[k][i]; j++)   //k use to be "0"
           {
             C.AddNeighbor(k, *m_pData, m_pData->m_pSimCases, j);       //k use to be "0"
             pMeasureList->AddMeasure(C.m_nCases, C.m_nMeasure);
           }
        }
      }
    
      nMaxLogLikelihood = pMeasureList->GetMaxLogLikelihood(*m_pData);
    
      delete pMeasureList;
      }
   catch (SSException & x)
      {
      delete pMeasureList;
      x.AddCallpath("MonteCarloProspective()", "CPurelySpatialAnalysis");
      throw;
      }
  return (nMaxLogLikelihood - m_pData->m_pModel->GetLogLikelihoodForTotal());
}


#include "SaTScan.h"
#pragma hdrstop
#include "PurelySpatialMonotoneAnalysis.h"

CPSMonotoneAnalysis::CPSMonotoneAnalysis(CParameters*  pParameters,
                                         CSaTScanData* pData,
                                         BasePrint *pPrintDirection)
                    :CPurelySpatialAnalysis(pParameters, pData, pPrintDirection)
{
}

CPSMonotoneAnalysis::~CPSMonotoneAnalysis()
{
}

CCluster* CPSMonotoneAnalysis::GetTopCluster(tract_t nCenter) {
  CPSMonotoneCluster          * MaxCluster = 0;
  CPSMonotoneCluster          * C_High = 0;
  CPSMonotoneCluster          * C_Low = 0;
  count_t                    ** ppCases(m_pData->GetCasesArray());

   try
     {
     if (m_pParameters->GetAreaScanRateType() == HIGHANDLOW)
     {
       C_High = new CPSMonotoneCluster(gpPrintDirection);
       C_High->SetCenter(nCenter);
       C_High->AllocateForMaxCircles(m_pData->GetNeighborCountArray()[0][nCenter]+1);
       C_High->SetRate(HIGH);
       C_High->DefineTopCluster(*m_pData, ppCases);
   
       C_Low = new CPSMonotoneCluster(gpPrintDirection);
       C_Low->SetCenter(nCenter);
       C_Low->AllocateForMaxCircles(m_pData->GetNeighborCountArray()[0][nCenter]+1);
       C_Low->SetRate(LOW);
       C_Low->DefineTopCluster(*m_pData, ppCases);
   
       //    if (C_High->m_nLogLikelihood >= C_Low->m_nLogLikelihood)
       if (C_High->m_nRatio >= C_Low->m_nRatio)
       {
         MaxCluster = C_High;
         delete C_Low;
       }
       else
       {
         MaxCluster = C_Low;
         delete C_High;
       }
   
     }
     else
     {
       MaxCluster = new CPSMonotoneCluster(gpPrintDirection);
       MaxCluster->SetCenter(nCenter);
       MaxCluster->AllocateForMaxCircles(m_pData->GetNeighborCountArray()[0][nCenter]+1);
       MaxCluster->SetRate(m_pParameters->GetAreaScanRateType());
       MaxCluster->DefineTopCluster(*m_pData, ppCases);
     }
   
     MaxCluster->SetStartAndEndDates(m_pData->GetTimeIntervalStartTimes(), m_pData->m_nTimeIntervals);
     }
   catch (ZdException & x)
      {
      delete C_High; C_High = 0;
      delete C_Low;  C_Low = 0;
      //MaxCluster could be C_High or C_Low or neither...
      //so set high and low to zero after deletion...  just in case.
      delete MaxCluster;
      x.AddCallpath("GetTopCluster()", "CPSMonotoneAnalysis");
      throw;
      }
  return MaxCluster;
}

double CPSMonotoneAnalysis::MonteCarlo() {
   CPSMonotoneCluster           MaxCluster(gpPrintDirection);
   CPSMonotoneCluster           C(gpPrintDirection);
   CPSMonotoneCluster           C_High(gpPrintDirection);
   CPSMonotoneCluster           C_Low(gpPrintDirection);
   count_t                   ** ppSimCases(m_pData->GetSimCasesArray());

   try
      {
      MaxCluster.Initialize(0);
      MaxCluster.m_nLogLikelihood = m_pData->GetProbabilityModel().GetLogLikelihoodForTotal();
    
      C.AllocateForMaxCircles(m_pData->m_nGridTracts+1);
      C_High.AllocateForMaxCircles(m_pData->m_nGridTracts+1);
      C_Low.AllocateForMaxCircles(m_pData->m_nGridTracts+1);
    
      for (int i = 0; i<m_pData->m_nGridTracts; i++)
        {
        if (m_pParameters->GetAreaScanRateType() == HIGHANDLOW)
          {
          C_High.Initialize(i);
          C_High.SetRate(HIGH);
          C_High.DefineTopCluster(*m_pData,  ppSimCases);
    
          C_Low.Initialize(i);
          C_Low.SetRate(LOW);
          C_Low.DefineTopCluster(*m_pData,  ppSimCases);

          //if (C_High.m_nLogLikelihood >= C_Low.m_nLogLikelihood &&
          //   C_High.m_nLogLikelihood > MaxCluster.m_nLogLikelihood)
          if (C_High.m_nRatio >= C_Low.m_nRatio &&
              C_High.m_nRatio > MaxCluster.m_nRatio)
            MaxCluster = C_High;
          //else if (C_Low.m_nLogLikelihood > MaxCluster.m_nLogLikelihood)
          else if (C_Low.m_nRatio > MaxCluster.m_nRatio)
            MaxCluster = C_Low;
    
          }
        else
          {
          C.Initialize(i);
          C.SetRate(m_pParameters->GetAreaScanRateType());
          C.DefineTopCluster(*m_pData, ppSimCases);
    
          //if (C.m_nLogLikelihood > MaxCluster.m_nLogLikelihood)
          if (C.m_nRatio > MaxCluster.m_nRatio)
            MaxCluster = C;
          }
        }
      }
   catch (ZdException & x)
      {
      x.AddCallpath("MonteCarlo()", "CPSMonotoneAnalysis");
      throw;
      }
   return (MaxCluster.GetRatio());
}

double CPSMonotoneAnalysis::MonteCarloProspective() {
   CPSMonotoneCluster           MaxCluster(gpPrintDirection);
   CPSMonotoneCluster           C(gpPrintDirection);
   CPSMonotoneCluster           C_High(gpPrintDirection);
   CPSMonotoneCluster           C_Low(gpPrintDirection);
   count_t                   ** ppSimCases(m_pData->GetSimCasesArray());

   try
      {
      MaxCluster.Initialize(0);
      MaxCluster.m_nLogLikelihood = m_pData->GetProbabilityModel().GetLogLikelihoodForTotal();
    
      C.AllocateForMaxCircles(m_pData->m_nGridTracts+1);
      C_High.AllocateForMaxCircles(m_pData->m_nGridTracts+1);
      C_Low.AllocateForMaxCircles(m_pData->m_nGridTracts+1);
    
      for (int i = 0; i<m_pData->m_nGridTracts; i++)
        {
        if (m_pParameters->GetAreaScanRateType() == HIGHANDLOW)
          {
          C_High.Initialize(i);
          C_High.SetRate(HIGH);
          C_High.DefineTopCluster(*m_pData,  ppSimCases);
    
          C_Low.Initialize(i);
          C_Low.SetRate(LOW);
          C_Low.DefineTopCluster(*m_pData,  ppSimCases);
    
          //if (C_High.m_nLogLikelihood >= C_Low.m_nLogLikelihood &&
          //   C_High.m_nLogLikelihood > MaxCluster.m_nLogLikelihood)
          if (C_High.m_nRatio >= C_Low.m_nRatio &&
              C_High.m_nRatio > MaxCluster.m_nRatio)
            MaxCluster = C_High;
          //else if (C_Low.m_nLogLikelihood > MaxCluster.m_nLogLikelihood)
          else if (C_Low.m_nRatio > MaxCluster.m_nRatio)
            MaxCluster = C_Low;
    
          }
        else
          {
          C.Initialize(i);
          C.SetRate(m_pParameters->GetAreaScanRateType());
          C.DefineTopCluster(*m_pData, ppSimCases);
    
          //if (C.m_nLogLikelihood > MaxCluster.m_nLogLikelihood)
          if (C.m_nRatio > MaxCluster.m_nRatio)
            MaxCluster = C;
          }
        }
      }
   catch (ZdException & x)
      {
      x.AddCallpath("MonteCarloProspective()", "CPSMonotoneAnalysis");
      throw;
      }
   return (MaxCluster.GetRatio());
}




#include "SaTScan.h"
#pragma hdrstop
#include "PurelySpatialMonotoneAnalysis.h"

CPSMonotoneAnalysis::CPSMonotoneAnalysis(CParameters*  pParameters, CSaTScanData* pData, BasePrint *pPrintDirection)
                    :CPurelySpatialAnalysis(pParameters, pData, pPrintDirection) {
  Init();
}

CPSMonotoneAnalysis::~CPSMonotoneAnalysis() {
  try {
    delete gpMaxCluster;
  }
  catch (...){}  
}

void CPSMonotoneAnalysis::CalculateTopCluster(tract_t tCenter, const DataStreamGateway & DataGateway, bool bSimulation) {
  CPSMonotoneCluster          * C_High = 0;
  CPSMonotoneCluster          * C_Low = 0;
  count_t                    ** ppCases(DataGateway.GetDataStreamInterface(0/*for now*/).GetCaseArray());

  try {
    if (m_pParameters->GetAreaScanRateType() == HIGHANDLOW) {
      C_High = new CPSMonotoneCluster(*m_pData, gpPrintDirection);
      C_High->SetCenter(tCenter);
      C_High->AllocateForMaxCircles(m_pData->GetNeighborCountArray()[0][tCenter]+1);
      C_High->SetRate(HIGH);
      C_High->DefineTopCluster(*m_pData, ppCases);

      C_Low = new CPSMonotoneCluster(*m_pData, gpPrintDirection);
      C_Low->SetCenter(tCenter);
      C_Low->AllocateForMaxCircles(m_pData->GetNeighborCountArray()[0][tCenter]+1);
      C_Low->SetRate(LOW);
      C_Low->DefineTopCluster(*m_pData, ppCases);
      //    if (C_High->m_nLogLikelihood >= C_Low->m_nLogLikelihood)
      if (C_High->m_nRatio >= C_Low->m_nRatio) {
         gpMaxCluster = C_High;
         delete C_Low; C_Low=0;
       }
       else {
         gpMaxCluster = C_Low;
         delete C_High; C_High=0;
       }
    }
    else {
      gpMaxCluster = new CPSMonotoneCluster(*m_pData, gpPrintDirection);
      gpMaxCluster->SetCenter(tCenter);
      gpMaxCluster->AllocateForMaxCircles(m_pData->GetNeighborCountArray()[0][tCenter]+1);
      gpMaxCluster->SetRate(m_pParameters->GetAreaScanRateType());
      gpMaxCluster->DefineTopCluster(*m_pData, ppCases);
    }

    gpMaxCluster->SetStartAndEndDates(m_pData->GetTimeIntervalStartTimes(), m_pData->m_nTimeIntervals);
  }
  catch (ZdException &x) {
    delete C_High; C_High=0;
    delete C_Low; C_Low=0;
    //MaxCluster could be C_High or C_Low or neither...
    //so set high and low to zero after deletion...  just in case.
    delete gpMaxCluster; gpMaxCluster=0;
    x.AddCallpath("CalculateTopCluster()","CPSMonotoneAnalysis");
    throw;
  }
}

CCluster & CPSMonotoneAnalysis::GetTopCalculatedCluster() {
  return *gpMaxCluster;
}

double CPSMonotoneAnalysis::MonteCarlo(const DataStreamInterface & Interface) {
   CPSMonotoneCluster           MaxCluster(*m_pData, gpPrintDirection);
   CPSMonotoneCluster           C(*m_pData, gpPrintDirection);
   CPSMonotoneCluster           C_High(*m_pData, gpPrintDirection);
   CPSMonotoneCluster           C_Low(*m_pData, gpPrintDirection);
   count_t                   ** ppSimCases(Interface.GetCaseArray());

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

double CPSMonotoneAnalysis::MonteCarloProspective(const DataStreamInterface & Interface) {
   CPSMonotoneCluster           MaxCluster(*m_pData, gpPrintDirection);
   CPSMonotoneCluster           C(*m_pData, gpPrintDirection);
   CPSMonotoneCluster           C_High(*m_pData, gpPrintDirection);
   CPSMonotoneCluster           C_Low(*m_pData, gpPrintDirection);
   count_t                   ** ppSimCases(Interface.GetCaseArray());

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

void CPSMonotoneAnalysis::SetTopClusters(const DataStreamGateway & DataGateway, bool bSimulation) {
  delete gpMaxCluster; gpMaxCluster=0;
}



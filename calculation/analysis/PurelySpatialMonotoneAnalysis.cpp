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

/** Allocates objects used during simulations, instead of repeated allocations
    for each simulation.
    NOTE: This analysis has not been optimized to 'pre' allocate objects used in
          simulation process. This function is only a shell.                     */
void CPSMonotoneAnalysis::AllocateSimulationObjects(const AbtractDataStreamGateway & DataGateway) {
}

/** Allocates objects used during calculation of most likely clusters, instead
    of repeated allocations for each simulation.
    NOTE: This analysis has not been optimized to 'pre' allocate objects used in
          process of finding most likely clusters. This function is only a shell. */
void CPSMonotoneAnalysis::AllocateTopClustersObjects(const AbtractDataStreamGateway & DataGateway) {
}

const CCluster & CPSMonotoneAnalysis::CalculateTopCluster(tract_t tCenter, const AbtractDataStreamGateway & DataGateway) {
  CPSMonotoneCluster          * C_High = 0;
  CPSMonotoneCluster          * C_Low = 0;
  count_t                    ** ppCases(DataGateway.GetDataStreamInterface(0/*for now*/).GetCaseArray());

  try {
    if (m_pParameters->GetAreaScanRateType() == HIGHANDLOW) {
      C_High = new CPSMonotoneCluster(gpClusterDataFactory, DataGateway, m_pParameters->GetAreaScanRateType(), gpPrintDirection);
      C_High->SetCenter(tCenter);
      C_High->AllocateForMaxCircles(m_pData->GetNeighborCountArray()[0][tCenter]+1);
      C_High->SetRate(HIGH);
      C_High->DefineTopCluster(*m_pData, ppCases);

      C_Low = new CPSMonotoneCluster(gpClusterDataFactory, DataGateway, m_pParameters->GetAreaScanRateType(), gpPrintDirection);
      C_Low->SetCenter(tCenter);
      C_Low->AllocateForMaxCircles(m_pData->GetNeighborCountArray()[0][tCenter]+1);
      C_Low->SetRate(LOW);
      C_Low->DefineTopCluster(*m_pData, ppCases);
      //    if (C_High->m_nLogLikelihood >= C_Low->m_nLogLikelihood)
      if (C_High->m_nRatio >= C_Low->m_nRatio) {
         delete gpMaxCluster;
         gpMaxCluster = C_High;
         delete C_Low; C_Low=0;
       }
       else {
         delete gpMaxCluster;
         gpMaxCluster = C_Low;
         delete C_High; C_High=0;
       }
    }
    else {
      delete gpMaxCluster; gpMaxCluster=0;
      gpMaxCluster = new CPSMonotoneCluster(gpClusterDataFactory, DataGateway, m_pParameters->GetAreaScanRateType(), gpPrintDirection);
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
  return *gpMaxCluster;
}

double CPSMonotoneAnalysis::MonteCarlo(const DataStreamInterface & Interface) {
   CPSMonotoneCluster           MaxCluster(gpClusterDataFactory, Interface, m_pParameters->GetAreaScanRateType(), gpPrintDirection);
   CPSMonotoneCluster           C(gpClusterDataFactory, Interface, m_pParameters->GetAreaScanRateType(), gpPrintDirection);
   CPSMonotoneCluster           C_High(gpClusterDataFactory, Interface, HIGH, gpPrintDirection);
   CPSMonotoneCluster           C_Low(gpClusterDataFactory, Interface, LOW, gpPrintDirection);
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

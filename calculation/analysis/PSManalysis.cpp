// PSMAnalysis.cpp

#include "PSManalysis.h"
#include "LogLikelihood.h"
#include "display.h"
#include "PSMCluster.h"
#include "Param.h"
#include "Model.h"

CPSMonotoneAnalysis::CPSMonotoneAnalysis(CParameters*  pParameters,
                                         CSaTScanData* pData)
                    :CPurelySpatialAnalysis(pParameters, pData)
{
}

CPSMonotoneAnalysis::~CPSMonotoneAnalysis()
{
}

CCluster* CPSMonotoneAnalysis::GetTopCluster(tract_t nCenter)
{
  CPSMonotoneCluster* MaxCluster;
  CPSMonotoneCluster* C_High;
  CPSMonotoneCluster* C_Low;

  if (m_pParameters->m_nAreas == HIGHANDLOW)
  {
    C_High = new CPSMonotoneCluster;
    C_High->SetCenter(nCenter);
    C_High->AllocateForMaxCircles(m_pData->m_NeighborCounts[nCenter]+1);
    C_High->SetRate(HIGH);
    C_High->DefineTopCluster(*m_pData, m_pData->m_pCases);

    C_Low = new CPSMonotoneCluster;
    C_Low->SetCenter(nCenter);
    C_Low->AllocateForMaxCircles(m_pData->m_NeighborCounts[nCenter]+1);
    C_Low->SetRate(LOW);
    C_Low->DefineTopCluster(*m_pData, m_pData->m_pCases);

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
    MaxCluster = new CPSMonotoneCluster;
    MaxCluster->SetCenter(nCenter);
    MaxCluster->AllocateForMaxCircles(m_pData->m_NeighborCounts[nCenter]+1);
    MaxCluster->SetRate(m_pParameters->m_nAreas);
    MaxCluster->DefineTopCluster(*m_pData, m_pData->m_pCases);
  }

  MaxCluster->SetStartAndEndDates(m_pData->m_pIntervalStartTimes,
                                  m_pData->m_nTimeIntervals);

  return MaxCluster;
}

double CPSMonotoneAnalysis::MonteCarlo()
{
  CPSMonotoneCluster MaxCluster, C, C_High, C_Low;

  MaxCluster.Initialize(0);
  MaxCluster.m_nLogLikelihood = m_pData->m_pModel->GetLogLikelihoodForTotal();

  C.AllocateForMaxCircles(m_pData->m_nGridTracts+1);
  C_High.AllocateForMaxCircles(m_pData->m_nGridTracts+1);
  C_Low.AllocateForMaxCircles(m_pData->m_nGridTracts+1);

  for (int i = 0; i<m_pData->m_nGridTracts; i++)
  {
    if (m_pParameters->m_nAreas == HIGHANDLOW)
    {
      C_High.Initialize(i);
      C_High.SetRate(HIGH);
      C_High.DefineTopCluster(*m_pData,  m_pData->m_pSimCases);

      C_Low.Initialize(i);
      C_Low.SetRate(LOW);
      C_Low.DefineTopCluster(*m_pData,  m_pData->m_pSimCases);

//      if (C_High.m_nLogLikelihood >= C_Low.m_nLogLikelihood &&
//          C_High.m_nLogLikelihood > MaxCluster.m_nLogLikelihood)
      if (C_High.m_nRatio >= C_Low.m_nRatio &&
          C_High.m_nRatio > MaxCluster.m_nRatio)
        MaxCluster = C_High;
//      else if (C_Low.m_nLogLikelihood > MaxCluster.m_nLogLikelihood)
      else if (C_Low.m_nRatio > MaxCluster.m_nRatio)
        MaxCluster = C_Low;

    }
    else
    {
      C.Initialize(i);
      C.SetRate(m_pParameters->m_nAreas);
      C.DefineTopCluster(*m_pData, m_pData->m_pSimCases);

//      if (C.m_nLogLikelihood > MaxCluster.m_nLogLikelihood)
      if (C.m_nRatio > MaxCluster.m_nRatio)
        MaxCluster = C;
    }
  }

  return (MaxCluster.GetRatio());
}



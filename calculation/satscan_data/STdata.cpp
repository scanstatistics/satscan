// dataPS.cpp

#include "STdata.h"

CSpaceTimeData::CSpaceTimeData(CParameters* pParameters)
               :CSaTScanData(pParameters)
{
}

CSpaceTimeData::~CSpaceTimeData()
{
}

void CSpaceTimeData::SetIntervalCut()
{
  CSaTScanData::SetIntervalCut();

  /* Avoids double calculations of the loglikelihood when IPS==1 and     */
  /* IntervalCut==nTimeIntervals. Increases speed in functions Cluster2() */
  /* and Montercarlo2().                                                 */
  if (m_pParameters->m_bIncludePurelySpatial)
    if (m_nTimeIntervals == m_nIntervalCut)
      m_nIntervalCut--;
}

void CSpaceTimeData::ReadDataFromFiles()
{
  CSaTScanData::ReadDataFromFiles();
  if (m_pParameters->m_bIncludePurelyTemporal)
    SetPurelyTemporalCases();
}

bool CSpaceTimeData::CalculateMeasure()
{
  bool bResult = CSaTScanData::CalculateMeasure();
  if (m_pParameters->m_bIncludePurelyTemporal)
    SetPurelyTemporalMeasures();
  return bResult;
}

void CSpaceTimeData::AllocSimCases()
{
  CSaTScanData::AllocSimCases();
  if (m_pParameters->m_bIncludePurelyTemporal)
    m_pPTSimCases = (count_t*)Smalloc(m_nTimeIntervals * sizeof(count_t));
}

void CSpaceTimeData::DeAllocSimCases()
{
  CSaTScanData::DeAllocSimCases();
  if (m_pParameters->m_bIncludePurelyTemporal)
    free(m_pPTSimCases);
}

void CSpaceTimeData::MakeData()
{
  CSaTScanData::MakeData();
  if (m_pParameters->m_bIncludePurelyTemporal)
    SetPurelyTemporalSimCases();
}




#pragma hdrstop
#include "SpaceTimeData.h"

CSpaceTimeData::CSpaceTimeData(CParameters* pParameters, BasePrint *pPrintDirection)
               :CSaTScanData(pParameters, pPrintDirection)
{
}

CSpaceTimeData::~CSpaceTimeData()
{
}

void CSpaceTimeData::SetIntervalCut()
{
   try
      {
      CSaTScanData::SetIntervalCut();

      /* Avoids double calculations of the loglikelihood when IPS==1 and     */
      /* IntervalCut==nTimeIntervals. Increases speed in functions Cluster2() */
      /* and Montercarlo2().                                                 */
      if (m_pParameters->m_bIncludePurelySpatial)
         if (m_nTimeIntervals == m_nIntervalCut)
            m_nIntervalCut--;
      }
   catch (SSException & x)
      {
      x.AddCallpath("SetIntervalCut()", "CSpaceTimeData");
      throw;
      }
}

void CSpaceTimeData::ReadDataFromFiles()
{
   try
      {
      CSaTScanData::ReadDataFromFiles();
      if (m_pParameters->m_bIncludePurelyTemporal)
         SetPurelyTemporalCases();
      }
   catch (SSException & x)
      {
      x.AddCallpath("ReadDataFromFiles()", "CSpaceTimeData");
      throw;
      }
}

bool CSpaceTimeData::CalculateMeasure()
{
  bool bResult;

  try
     {
     bResult = CSaTScanData::CalculateMeasure();
     if (m_pParameters->m_bIncludePurelyTemporal)
        SetPurelyTemporalMeasures();
      }
   catch (SSException & x)
      {
      x.AddCallpath("CalculateMeasure()", "CSpaceTimeData");
      throw;
      }
  return bResult;
}

void CSpaceTimeData::AllocSimCases()
{
   try
      {
      CSaTScanData::AllocSimCases();
      if (m_pParameters->m_bIncludePurelyTemporal)
         m_pPTSimCases = (count_t*)Smalloc(m_nTimeIntervals * sizeof(count_t), gpPrintDirection);
      }
   catch (SSException & x)
      {
      x.AddCallpath("AllocSimCases()", "CSpaceTimeData");
      throw;
      }
}

void CSpaceTimeData::DeAllocSimCases()
{
   try
      {
      CSaTScanData::DeAllocSimCases();
      if (m_pParameters->m_bIncludePurelyTemporal)
         free(m_pPTSimCases);
      }
   catch (SSException & x)
      {
      x.AddCallpath("DeAllocSimCases()", "CSpaceTimeData");
      throw;
      }
}

void CSpaceTimeData::MakeData()
{
   try
      {
      CSaTScanData::MakeData();
      if (m_pParameters->m_bIncludePurelyTemporal)
         SetPurelyTemporalSimCases();
      }
   catch (SSException & x)
      {
      x.AddCallpath("MakeData()", "CSpaceTimeData");
      throw;
      }
}




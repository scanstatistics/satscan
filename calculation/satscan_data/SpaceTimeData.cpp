#include "SaTScan.h"
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
      if (m_pParameters->GetIncludePurelySpatialClusters())
         if (m_nTimeIntervals == m_nIntervalCut)
            m_nIntervalCut--;
      }
   catch (ZdException & x)
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
      if (m_pParameters->GetIncludePurelyTemporalClusters())
         SetPurelyTemporalCases();
      }
   catch (ZdException & x)
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
     if (m_pParameters->GetIncludePurelyTemporalClusters())
        SetPurelyTemporalMeasures();
      }
   catch (ZdException & x)
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
      if (m_pParameters->GetIncludePurelyTemporalClusters())
         m_pPTSimCases = (count_t*)Smalloc(m_nTimeIntervals * sizeof(count_t), gpPrint);
      }
   catch (ZdException & x)
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
      if (m_pParameters->GetIncludePurelyTemporalClusters())
         free(m_pPTSimCases);
      }
   catch (ZdException & x)
      {
      x.AddCallpath("DeAllocSimCases()", "CSpaceTimeData");
      throw;
      }
}

void CSpaceTimeData::MakeData(int iSimulationNumber)
{
   try
      {
      CSaTScanData::MakeData(iSimulationNumber);
      if (m_pParameters->GetIncludePurelyTemporalClusters())
         SetPurelyTemporalSimCases();
      }
   catch (ZdException & x)
      {
      x.AddCallpath("MakeData()", "CSpaceTimeData");
      throw;
      }
}




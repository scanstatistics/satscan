#include "SaTScan.h"
#pragma hdrstop
#include "PurelyTemporalData.h"

CPurelyTemporalData::CPurelyTemporalData(CParameters* pParameters, BasePrint *pPrintDirection)
                    :CSaTScanData(pParameters, pPrintDirection)
{
   m_pPTCases    = 0;
   m_pPTSimCases = 0;
   m_pPTMeasure  = 0;
}

CPurelyTemporalData::~CPurelyTemporalData()
{
   if (m_pPTSimCases)
      DeAllocSimCases();
}

void CPurelyTemporalData::AllocSimCases()
{
   try
      {
      CSaTScanData::AllocSimCases();  // Use until MakePurelyTemporalData implemented
      m_pPTSimCases = (count_t*)Smalloc(m_nTimeIntervals * sizeof(count_t), gpPrint);
      }
   catch (ZdException & x)
      {
      x.AddCallpath("AllocSimCases()", "CPurelyTemporalData");
      throw;
      }
}

void CPurelyTemporalData::DeAllocSimCases()
{
   try
      {
      CSaTScanData::DeAllocSimCases();  // Use until MakePurelyTemporalData implemented
      free(m_pPTSimCases);
      m_pPTSimCases = 0;
      }
   catch (...)
      {
      }
}

void CPurelyTemporalData::ReadDataFromFiles()
{
   try
      {
      CSaTScanData::ReadDataFromFiles();
      SetPurelyTemporalCases();
      }
   catch (ZdException & x)
      {
      x.AddCallpath("ReadDataFromFiles()", "CPurelyTemporalData");
      throw;
      }
}

bool CPurelyTemporalData::CalculateMeasure()
{
   bool bResult;

   try
      {
      bResult = CSaTScanData::CalculateMeasure();
      SetPurelyTemporalMeasures();
      }
   catch (ZdException & x)
      {
      x.AddCallpath("CalculateMeasure()", "CPurelyTemporalData");
      throw;
      }
  return bResult;
}

void CPurelyTemporalData::DisplayCases(FILE* pFile)
{
   try
      {
      fprintf(pFile, "PT Case counts (m_pPTCases)   m_nTimeIntervals=%i\n\n", m_nTimeIntervals);
      for (int i = 0; i < m_nTimeIntervals; i++)
         fprintf(pFile, "PTCases [%i] = %i\n", i,m_pPTCases[i]);
      fprintf(pFile, "\n\n");
      }
   catch (ZdException & x)
      {
      x.AddCallpath("DisplayCases()", "CPurelyTemporalData");
      throw;
      }
}

void CPurelyTemporalData::DisplaySimCases(FILE* pFile)
{
   try
      {
      fprintf(pFile, "PT Simulated Case counts (m_pPTSimCases)\n\n");
      for (int i = 0; i < m_nTimeIntervals; i++)
         fprintf(pFile, "PTSimCases [%i] = %i\n", i,m_pPTSimCases[i]);
      fprintf(pFile, "\n");
      }
   catch (ZdException & x)
      {
      x.AddCallpath("DisplaySimCases()", "CPurelyTemporalData");
      throw;
      }
}

void CPurelyTemporalData::DisplayMeasure(FILE* pFile)
{
   try
      {
      fprintf(pFile, "PT Measures (m_pPTMeasure)   m_nTimeIntervals=%i\n\n", m_nTimeIntervals);
      for (int i = 0; i < m_nTimeIntervals; i++)
         fprintf(pFile, "PTMeasure [%i] = %f\n", i,m_pPTMeasure[i]);
      fprintf(pFile, "\n\n");
      }
   catch (ZdException & x)
      {
      x.AddCallpath("DisplayMeasure()", "CPurelyTemporalData");
      throw;
      }
}

void CPurelyTemporalData::MakeData(int iSimulationNumber)
{
   try
      {
      CSaTScanData::MakeData(iSimulationNumber);
      SetPurelyTemporalSimCases();
      }
   catch (ZdException & x)
      {
      x.AddCallpath("MakeData()", "CPurelyTemporalData");
      throw;
      }
}



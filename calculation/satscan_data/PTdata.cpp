// dataPT.cpp

#include "PTdata.h"

CPurelyTemporalData::CPurelyTemporalData(CParameters* pParameters)
                    :CSaTScanData(pParameters)
{
  m_pPTCases    = 0;
  m_pPTSimCases = 0;
  m_pPTMeasure  = 0;
}

CPurelyTemporalData::~CPurelyTemporalData()
{
}

void CPurelyTemporalData::AllocSimCases()
{
  CSaTScanData::AllocSimCases();  // Use until MakePurelyTemporalData implemented
  m_pPTSimCases = (count_t*)Smalloc(m_nTimeIntervals * sizeof(count_t));
}

void CPurelyTemporalData::DeAllocSimCases()
{
  CSaTScanData::DeAllocSimCases();  // Use until MakePurelyTemporalData implemented
  free(m_pPTSimCases);
}

void CPurelyTemporalData::ReadDataFromFiles()
{
  CSaTScanData::ReadDataFromFiles();
  SetPurelyTemporalCases();
}

bool CPurelyTemporalData::CalculateMeasure()
{
  bool bResult = CSaTScanData::CalculateMeasure();
  SetPurelyTemporalMeasures();
  return bResult;
}

void CPurelyTemporalData::DisplayCases(FILE* pFile)
{
  fprintf(pFile, "PT Case counts (m_pPTCases)   m_nTimeIntervals=%i\n\n", m_nTimeIntervals);

  for (int i = 0; i < m_nTimeIntervals; i++)
    fprintf(pFile, "PTCases [%i] = %i\n", i,m_pPTCases[i]);

  fprintf(pFile, "\n\n");
}

void CPurelyTemporalData::DisplaySimCases(FILE* pFile)
{
  fprintf(pFile, "PT Simulated Case counts (m_pPTSimCases)\n\n");

  for (int i = 0; i < m_nTimeIntervals; i++)
    fprintf(pFile, "PTSimCases [%i] = %i\n", i,m_pPTSimCases[i]);

  fprintf(pFile, "\n");
}

void CPurelyTemporalData::DisplayMeasure(FILE* pFile)
{
  fprintf(pFile, "PT Measures (m_pPTMeasure)   m_nTimeIntervals=%i\n\n", m_nTimeIntervals);

  for (int i = 0; i < m_nTimeIntervals; i++)
    fprintf(pFile, "PTMeasure [%i] = %f\n", i,m_pPTMeasure[i]);

  fprintf(pFile, "\n\n");
}

void CPurelyTemporalData::MakeData()
{
  CSaTScanData::MakeData();
  SetPurelyTemporalSimCases();
}



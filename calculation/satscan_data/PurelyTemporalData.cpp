#include "SaTScan.h"
#pragma hdrstop
#include "PurelyTemporalData.h"
#include "PoissonModel.h"
#include "BernoulliModel.h"
#include "SpaceTimePermutationModel.h"

/** constructor */
CPurelyTemporalData::CPurelyTemporalData(CParameters* pParameters, BasePrint *pPrintDirection)
                    :CSaTScanData(pParameters, pPrintDirection) {
  try {
    SetProbabilityModel();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CPurelyTemporalData");
    throw;
  }
}

/** destructor */
CPurelyTemporalData::~CPurelyTemporalData() {
  try {
    if (m_pPTSimCases) DeAllocSimCases();
  }
  catch(...){}  
}

void CPurelyTemporalData::AdjustNeighborCounts() {
  ZdGenerateException("AdjustNeighborCounts() not implemented for CPurelyTemporalData.","AdjustNeighborCounts()");
}

void CPurelyTemporalData::AllocSimCases() {
  try {
    CSaTScanData::AllocSimCases();  // Use until MakePurelyTemporalData implemented
    m_pPTSimCases = (count_t*)Smalloc(m_nTimeIntervals * sizeof(count_t), gpPrint);
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocSimCases()","CPurelyTemporalData");
    throw;
  }
}

bool CPurelyTemporalData::CalculateMeasure() {
  bool bResult;

  try {
    bResult = CSaTScanData::CalculateMeasure();
    SetPurelyTemporalMeasures();
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateMeasure()", "CPurelyTemporalData");
    throw;
  }
  return bResult;
}

void CPurelyTemporalData::DeAllocSimCases() {
  try {
    CSaTScanData::DeAllocSimCases();  // Use until MakePurelyTemporalData implemented
    free(m_pPTSimCases);
    m_pPTSimCases = 0;
  }
  catch (ZdException &x) {
    x.AddCallpath("DeAllocSimCases()","CPurelyTemporalData");
    throw;
  }
}

void CPurelyTemporalData::DisplayCases(FILE* pFile) {
  try {
    fprintf(pFile, "PT Case counts (m_pPTCases)   m_nTimeIntervals=%i\n\n", m_nTimeIntervals);
    for (int i = 0; i < m_nTimeIntervals; i++)
       fprintf(pFile, "PTCases [%i] = %i\n", i,m_pPTCases[i]);
    fprintf(pFile, "\n\n");
  }
  catch (ZdException &x) {
    x.AddCallpath("DisplayCases()","CPurelyTemporalData");
    throw;
  }
}

void CPurelyTemporalData::DisplayMeasure(FILE* pFile) {
  try {
    fprintf(pFile, "PT Measures (m_pPTMeasure)   m_nTimeIntervals=%i\n\n", m_nTimeIntervals);
    for (int i = 0; i < m_nTimeIntervals; i++)
       fprintf(pFile, "PTMeasure [%i] = %f\n", i,m_pPTMeasure[i]);
    fprintf(pFile, "\n\n");
  }
  catch (ZdException &x) {
    x.AddCallpath("DisplayMeasure()","CPurelyTemporalData");
    throw;
  }
}

void CPurelyTemporalData::DisplaySimCases(FILE* pFile) {
  try {
    fprintf(pFile, "PT Simulated Case counts (m_pPTSimCases)\n\n");
    for (int i = 0; i < m_nTimeIntervals; i++)
       fprintf(pFile, "PTSimCases [%i] = %i\n", i,m_pPTSimCases[i]);
    fprintf(pFile, "\n");
  }
  catch (ZdException &x) {
    x.AddCallpath("DisplaySimCases()","CPurelyTemporalData");
    throw;
  }
}

tract_t CPurelyTemporalData::GetNeighbor(int iEllipse, tract_t t, unsigned int nearness) const {
  ZdGenerateException("GetNeighbor() not implemented for CPurelyTemporalData.","GetNeighbor()");
  return 0;
}

void CPurelyTemporalData::MakeData(int iSimulationNumber) {
  try {
    CSaTScanData::MakeData(iSimulationNumber);
    SetPurelyTemporalSimCases();
  }
  catch (ZdException &x) {
    x.AddCallpath("MakeData()", "CPurelyTemporalData");
    throw;
  }
}

void CPurelyTemporalData::PrintNeighbors() {
  ZdGenerateException("PrintNeighbors() not implemented for CPurelyTemporalData.","PrintNeighbors()");
}

void CPurelyTemporalData::ReadDataFromFiles() {
  try {
    CSaTScanData::ReadDataFromFiles();
    SetPurelyTemporalCases();
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadDataFromFiles()","CPurelyTemporalData");
    throw;
  }
}

/** allocates probability model */
void CPurelyTemporalData::SetProbabilityModel() {
  try {
    switch (m_pParameters->GetProbabiltyModelType()) {
       case POISSON              : m_pModel = new CPoissonModel(*m_pParameters, *this, *gpPrint);   break;
       case BERNOULLI            : m_pModel = new CBernoulliModel(*m_pParameters, *this, *gpPrint); break;
       case SPACETIMEPERMUTATION : ZdException::Generate("Purely Temporal analysis not implemented for Space-Time Permutation model.\n",
                                                         "SetProbabilityModel()");
       default : ZdException::Generate("Unknown probability model type: '%d'.\n", "SetProbabilityModel()",
                                       m_pParameters->GetProbabiltyModelType());
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetProbabilityModel()","CPurelyTemporalData");
    throw;
  }
}



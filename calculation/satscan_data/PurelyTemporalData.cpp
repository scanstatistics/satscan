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
CPurelyTemporalData::~CPurelyTemporalData() {}

void CPurelyTemporalData::AdjustNeighborCounts() {
  ZdGenerateException("AdjustNeighborCounts() not implemented for CPurelyTemporalData.","AdjustNeighborCounts()");
}

void CPurelyTemporalData::AllocateSimulationStructures() {
  ProbabiltyModelType   eProbabiltyModelType(m_pParameters->GetProbabiltyModelType());
  AnalysisType          eAnalysisType(m_pParameters->GetAnalysisType());

  try {
    CSaTScanData::AllocateSimulationStructures();  // Use until MakePurelyTemporalData implemented
    
    //allocate simulation case arrays
    if (eProbabiltyModelType != 10/*normal*/ && eProbabiltyModelType != 11/*rank*/)
      gpDataStreams->AllocatePTSimCases();
    //allocate simulation measure arrays
    if (eProbabiltyModelType != 10/*normal*/ || eProbabiltyModelType != 11/*rank*/ || eProbabiltyModelType != 12/*survival*/)
      gpDataStreams->AllocateSimulationPTMeasure();
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateSimulationStructures()","CPurelyTemporalData");
    throw;
  }
}

bool CPurelyTemporalData::CalculateMeasure(DataStream & thisStream) {
  bool bResult;

  try {
    bResult = CSaTScanData::CalculateMeasure(thisStream);
    thisStream.SetPTMeasureArray();
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateMeasure()", "CPurelyTemporalData");
    throw;
  }
  return bResult;
}

void CPurelyTemporalData::DisplayCases(FILE* pFile) {
  try {
    fprintf(pFile, "PT Case counts (PTCases)   m_nTimeIntervals=%i\n\n", m_nTimeIntervals);
    for (int i = 0; i < m_nTimeIntervals; i++)
       fprintf(pFile, "PTCases [%i] = %i\n", i, gpDataStreams->GetStream(0/*for now*/).GetPTCasesArray()[i]);
    fprintf(pFile, "\n\n");
  }
  catch (ZdException &x) {
    x.AddCallpath("DisplayCases()","CPurelyTemporalData");
    throw;
  }
}

void CPurelyTemporalData::DisplayMeasure(FILE* pFile) {
  try {
    fprintf(pFile, "PT Measures (PTMeasure)   m_nTimeIntervals=%i\n\n", m_nTimeIntervals);
    for (int i = 0; i < m_nTimeIntervals; i++)
       fprintf(pFile, "PTMeasure [%i] = %f\n", i, gpDataStreams->GetStream(0/*for now*/).GetPTMeasureArray()[i]);
    fprintf(pFile, "\n\n");
  }
  catch (ZdException &x) {
    x.AddCallpath("DisplayMeasure()","CPurelyTemporalData");
    throw;
  }
}

void CPurelyTemporalData::DisplaySimCases(FILE* pFile) {
  try {
    fprintf(pFile, "PT Simulated Case counts (PTSimCases)\n\n");
    for (int i = 0; i < m_nTimeIntervals; i++)
       fprintf(pFile, "PTSimCases [%i] = %i\n", i, gpDataStreams->GetStream(0/*for now*/).GetPTSimCasesArray()[i]);
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

void CPurelyTemporalData::MakeData(int iSimulationNumber, DataStreamGateway & DataGateway) {
  try {
    CSaTScanData::MakeData(iSimulationNumber, DataGateway);
    gpDataStreams->SetPurelyTemporalSimCases();
  }
  catch (ZdException &x) {
    x.AddCallpath("MakeData()","CPurelyTemporalData");
    throw;
  }
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



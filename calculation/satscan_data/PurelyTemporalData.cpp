//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "PurelyTemporalData.h"
#include "PoissonModel.h"
#include "BernoulliModel.h"
#include "SpaceTimePermutationModel.h"
#include "NormalModel.h"
#include "SurvivalModel.h"
#include "RankModel.h"

/** constructor */
CPurelyTemporalData::CPurelyTemporalData(const CParameters* pParameters, BasePrint *pPrintDirection)
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

void CPurelyTemporalData::CalculateMeasure(RealDataStream & thisStream) {
  try {
    CSaTScanData::CalculateMeasure(thisStream);
    gpDataStreams->SetPurelyTemporalMeasureData(thisStream);
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateMeasure()","CPurelyTemporalData");
    throw;
  }
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
//  try {
//    fprintf(pFile, "PT Simulated Case counts (PTSimCases)\n\n");
//    for (int i = 0; i < m_nTimeIntervals; i++)
//       fprintf(pFile, "PTSimCases [%i] = %i\n", i, gpDataStreams->GetStream(0/*for now*/).GetPTSimCasesArray()[i]);
//    fprintf(pFile, "\n");
//  }
//  catch (ZdException &x) {
//    x.AddCallpath("DisplaySimCases()","CPurelyTemporalData");
//    throw;
//  }
}

tract_t CPurelyTemporalData::GetNeighbor(int iEllipse, tract_t t, unsigned int nearness) const {
  ZdGenerateException("GetNeighbor() not implemented for CPurelyTemporalData.","GetNeighbor()");
  return 0;
}

void CPurelyTemporalData::RandomizeData(SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber) {
  try {
    CSaTScanData::RandomizeData(SimDataContainer, iSimulationNumber);
    gpDataStreams->SetPurelyTemporalSimulationData(SimDataContainer);
  }
  catch (ZdException &x) {
    x.AddCallpath("RandomizeData()","CPurelyTemporalData");
    throw;
  }
}

/** Randomizes collection of simulation data in concert with passed collection of randomizers. */
void CPurelyTemporalData::RandomizeIsolatedData(RandomizerContainer_t& RandomizerContainer,
                                                SimulationDataContainer_t& SimDataContainer,
                                                unsigned int iSimulationNumber) const {
  try {
    CSaTScanData::RandomizeIsolatedData(RandomizerContainer, SimDataContainer, iSimulationNumber);
    gpDataStreams->SetPurelyTemporalSimulationData(SimDataContainer);
  }
  catch (ZdException &x) {
    x.AddCallpath("RandomizeIsolatedData()","CPurelyTemporalData");
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
       case NORMAL               : m_pModel = new CNormalModel(*m_pParameters, *this, *gpPrint); break;
       case SURVIVAL             : m_pModel = new CSurvivalModel(*m_pParameters, *this, *gpPrint); break;
       case RANK                 : m_pModel = new CRankModel(*m_pParameters, *this, *gpPrint); break;
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



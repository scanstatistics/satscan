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

/** class constructor */
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

/** class destructor */
CPurelyTemporalData::~CPurelyTemporalData() {}

/** Not implemented throws ZdException. */
void CPurelyTemporalData::AdjustNeighborCounts() {
  ZdGenerateException("AdjustNeighborCounts() not implemented for CPurelyTemporalData.","AdjustNeighborCounts()");
}

/** Calls base class CSaTScanData::CalculateMeasure(). Sets data stream object's
    temporal data structures. */
void CPurelyTemporalData::CalculateMeasure(RealDataStream& thisStream) {
  try {
    CSaTScanData::CalculateMeasure(thisStream);
    //Set temporal structures
    gpDataStreams->SetPurelyTemporalMeasureData(thisStream);
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateMeasure()","CPurelyTemporalData");
    throw;
  }
}

/** Debug utility function - prints case counts for all data streams. Caller is
    responsible for ensuring that passed file pointer points to valid, open file
    handle. */
void CPurelyTemporalData::DisplayCases(FILE* pFile) {
  unsigned int   i, j;

  fprintf(pFile, "PT Case counts (PTCases)   m_nTimeIntervals=%i\n\n", m_nTimeIntervals);
  for (j=0; j <  gpDataStreams->GetNumStreams(); ++j) {
     fprintf(pFile, "Data Stream %u:\n", j);
     for (i=0; i < (unsigned int)m_nTimeIntervals; ++i)
        fprintf(pFile, "PTCases [%u] = %i\n", i, gpDataStreams->GetStream(j).GetPTCasesArray()[i]);
     fprintf(pFile, "\n\n");
  }
}

/** Debug utility function - prints expected case counts for all data streams.
    Caller is responsible for ensuring that passed file pointer points to valid,
    open file handle. */
void CPurelyTemporalData::DisplayMeasure(FILE* pFile) {
  unsigned int   i, j;

  fprintf(pFile, "PT Measures (PTMeasure)   m_nTimeIntervals=%i\n\n", m_nTimeIntervals);
  for (j=0; j <  gpDataStreams->GetNumStreams(); ++j) {
     fprintf(pFile, "Data Stream %u:\n", j);
     for (i=0; i < (unsigned int)m_nTimeIntervals; ++i)
        fprintf(pFile, "PTMeasure [%u] = %lf\n", i, gpDataStreams->GetStream(j).GetPTMeasureArray()[i]);
     fprintf(pFile, "\n\n");
  }
}

/** Debug utility function - not implemented - needs updating. */
void CPurelyTemporalData::DisplaySimCases(FILE* pFile) {
//  unsigned int   i, j;
//
//  fprintf(pFile, "PT Simulated Case counts (PTSimCases)\n\n");
// for (j=0; j <  gpDataStreams->GetNumStreams(); ++j) {
//     fprintf(pFile, "Data Stream %u:\n", j);
//     for (i=0; i < m_nTimeIntervals; ++i)
//        fprintf(pFile, "PTSimCases [%u] = %i\n", i, gpDataStreams->GetStream(j).GetPTSimCasesArray()[i]);
//     fprintf(pFile, "\n\n");
//  }
}

/** Not implemented - throws ZdException. */
tract_t CPurelyTemporalData::GetNeighbor(int iEllipse, tract_t t, unsigned int nearness) const {
  ZdGenerateException("GetNeighbor() not implemented for CPurelyTemporalData.","GetNeighbor()");
  return 0;
}

/** Randomizes collection of simulation data in concert with passed collection of randomizers. */
void CPurelyTemporalData::RandomizeData(RandomizerContainer_t& RandomizerContainer,
                                        SimulationDataContainer_t& SimDataContainer,
                                        unsigned int iSimulationNumber) const {
  try {
    CSaTScanData::RandomizeData(RandomizerContainer, SimDataContainer, iSimulationNumber);
    gpDataStreams->SetPurelyTemporalSimulationData(SimDataContainer);
  }
  catch (ZdException &x) {
    x.AddCallpath("RandomizeData()","CPurelyTemporalData");
    throw;
  }
}

/** Calls base class CSaTScanData::ReadDataFromFiles(). Sets data stream objects'
    temporal data structures. */
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

/** Allocates probability model object. Throws ZdException if probability model
    type is space-time permutation. */
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



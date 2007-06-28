//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "PurelyTemporalData.h"
#include "PoissonModel.h"
#include "BernoulliModel.h"
#include "SpaceTimePermutationModel.h"
#include "NormalModel.h"
#include "ExponentialModel.h"
#include "RankModel.h"
#include "OrdinalModel.h"
#include "SSException.h"

/** class constructor */
CPurelyTemporalData::CPurelyTemporalData(const CParameters& Parameters, BasePrint& PrintDirection)
                    :CSaTScanData(Parameters, PrintDirection) {
  try {
    SetProbabilityModel();
  }
  catch (prg_exception &x) {
    x.addTrace("constructor()","CPurelyTemporalData");
    throw;
  }
}

/** class destructor */
CPurelyTemporalData::~CPurelyTemporalData() {}

/** Not implemented throws prg_error. */
void CPurelyTemporalData::AdjustNeighborCounts(ExecutionType geExecutingType) {
  throw prg_error("AdjustNeighborCounts() not implemented for CPurelyTemporalData.","AdjustNeighborCounts()");
}

/** Calls base class CSaTScanData::CalculateMeasure(). Sets dataset object's
    temporal data structures. */
void CPurelyTemporalData::CalculateMeasure(RealDataSet& DataSet) {
  try {
    CSaTScanData::CalculateMeasure(DataSet);
    //Set temporal structures
    if (gParameters.GetProbabilityModelType() != ORDINAL)
      gDataSets->SetPurelyTemporalMeasureData(DataSet);
  }
  catch (prg_exception &x) {
    x.addTrace("CalculateMeasure()","CPurelyTemporalData");
    throw;
  }
}

/** Debug utility function - prints case counts for all datasets. Caller is
    responsible for ensuring that passed file pointer points to valid, open file
    handle. */
void CPurelyTemporalData::DisplayCases(FILE* pFile) {
  unsigned int   i, j;

  fprintf(pFile, "PT Case counts (PTCases)   m_nTimeIntervals=%i\n\n", m_nTimeIntervals);
  for (j=0; j <  gDataSets->GetNumDataSets(); ++j) {
     fprintf(pFile, "Data Set %u:\n", j);
     for (i=0; i < (unsigned int)m_nTimeIntervals; ++i)
        fprintf(pFile, "PTCases [%u] = %i\n", i, gDataSets->GetDataSet(j).getCaseData_PT()[i]);
     fprintf(pFile, "\n\n");
  }
}

/** Debug utility function - prints expected case counts for all datasets.
    Caller is responsible for ensuring that passed file pointer points to valid,
    open file handle. */
void CPurelyTemporalData::DisplayMeasure(FILE* pFile) {
  unsigned int   i, j;

  fprintf(pFile, "PT Measures (PTMeasure)   m_nTimeIntervals=%i\n\n", m_nTimeIntervals);
  for (j=0; j <  gDataSets->GetNumDataSets(); ++j) {
     fprintf(pFile, "Data Set %u:\n", j);
     for (i=0; i < (unsigned int)m_nTimeIntervals; ++i)
        fprintf(pFile, "PTMeasure [%u] = %lf\n", i, gDataSets->GetDataSet(j).getMeasureData_PT()[i]);
     fprintf(pFile, "\n\n");
  }
}

/** Debug utility function - not implemented - needs updating. */
void CPurelyTemporalData::DisplaySimCases(FILE* pFile) {
//  unsigned int   i, j;
//
//  fprintf(pFile, "PT Simulated Case counts (PTSimCases)\n\n");
// for (j=0; j <  gDataSets->GetNumDataSets(); ++j) {
//     fprintf(pFile, "Data Set %u:\n", j);
//     for (i=0; i < m_nTimeIntervals; ++i)
//        fprintf(pFile, "PTSimCases [%u] = %i\n", i, gDataSets->GetDataSet(j).GetPTSimCasesArray()[i]);
//     fprintf(pFile, "\n\n");
//  }
}

/** Not implemented - throws prg_error. */
tract_t CPurelyTemporalData::GetNeighbor(int iEllipse, tract_t t, unsigned int nearness, double dMaxRadius) const {
  throw prg_error("GetNeighbor() not implemented for CPurelyTemporalData.","GetNeighbor()");
}

/** Randomizes collection of simulation data in concert with passed collection of randomizers. */
void CPurelyTemporalData::RandomizeData(RandomizerContainer_t& RandomizerContainer,
                                        SimulationDataContainer_t& SimDataContainer,
                                        unsigned int iSimulationNumber) const {
  try {
    gDataSets->RandomizeData(RandomizerContainer, SimDataContainer, iSimulationNumber);
  }
  catch (prg_exception& x) {
    x.addTrace("RandomizeData()","CPurelyTemporalData");
    throw;
  }
}

/** Calls base class CSaTScanData::ReadDataFromFiles(). Sets dataset objects'
    temporal data structures. */
void CPurelyTemporalData::ReadDataFromFiles() {
  try {
    CSaTScanData::ReadDataFromFiles();
    SetPurelyTemporalCases();
  }
  catch (prg_exception& x) {
    x.addTrace("ReadDataFromFiles()","CPurelyTemporalData");
    throw;
  }
}

/** Allocates probability model object. Throws prg_error if probability model
    type is space-time permutation. */
void CPurelyTemporalData::SetProbabilityModel() {
  switch (gParameters.GetProbabilityModelType()) {
     case POISSON              : m_pModel = new CPoissonModel(*this);   break;
     case BERNOULLI            : m_pModel = new CBernoulliModel(); break;
     case ORDINAL              : m_pModel = new OrdinalModel(); break;
     case EXPONENTIAL          : m_pModel = new ExponentialModel(); break;
     case WEIGHTEDNORMAL       :
     case NORMAL               : m_pModel = new CNormalModel(); break;
     case RANK                 : m_pModel = new CRankModel(); break;
     case SPACETIMEPERMUTATION : throw prg_error("Purely Temporal analysis not implemented for Space-Time Permutation model.\n",
                                                 "SetProbabilityModel()");
     default : throw prg_error("Unknown probability model type: '%d'.\n", "SetProbabilityModel()",
                               gParameters.GetProbabilityModelType());
  }
}



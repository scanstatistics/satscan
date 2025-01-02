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
#include "UniformTimeModel.h"
#include "BatchedModel.h"

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
    if (gParameters.GetProbabilityModelType() != ORDINAL && gParameters.GetProbabilityModelType() != CATEGORICAL)
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
void CPurelyTemporalData::DisplayCases(FILE* pFile) const {
  unsigned int   i, j;

  fprintf(pFile, "PT Case counts (PTCases)   # TimeIntervals=%i\n\n", GetNumTimeIntervals());
  for (j=0; j <  gDataSets->GetNumDataSets(); ++j) {
     fprintf(pFile, "Data Set %u:\n", j);
     for (i=0; i < (unsigned int)GetNumTimeIntervals(); ++i)
        fprintf(pFile, "PTCases [%u] = %li\n", i, gDataSets->GetDataSet(j).getCaseData_PT()[i]);
     fprintf(pFile, "\n\n");
  }
  fflush(pFile);
}

/** Debug utility function - prints expected case counts for all datasets.
    Caller is responsible for ensuring that passed file pointer points to valid,
    open file handle. */
void CPurelyTemporalData::DisplayMeasure(FILE* pFile) const {
  unsigned int   i, j;

  fprintf(pFile, "PT Measures (PTMeasure)   # TimeIntervals=%i\n\n", GetNumTimeIntervals());
  for (j=0; j <  gDataSets->GetNumDataSets(); ++j) {
     fprintf(pFile, "Data Set %u:\n", j);
     for (i=0; i < (unsigned int)GetNumTimeIntervals(); ++i)
        fprintf(pFile, "PTMeasure [%u] = %lf\n", i, gDataSets->GetDataSet(j).getMeasureData_PT()[i]);
     fprintf(pFile, "\n\n");
  }
  fflush(pFile);
}

/** Debug utility function - prints simulation case counts for all datasets. Caller is
    responsible for ensuring that passed file pointer points to valid, open file
    handle. */
void CPurelyTemporalData::DisplaySimCases(SimulationDataContainer_t& Container, FILE* pFile) const {
  fprintf(pFile, "PT Simulated Case counts (PTSimCases)\n\n");
  for (size_t j=0; j < Container.size(); ++j) {
     fprintf(pFile, "Data Set %u:\n", j);
     for (int i=0; i < GetNumTimeIntervals(); ++i)
        fprintf(pFile, "PTSimCases [%u] = %li\n", i, Container.at(j)->getCaseData_PT()[i]);
     fprintf(pFile, "\n\n");
  }
  fflush(pFile);
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

void CPurelyTemporalData::PostDataRead() {
	// Do not set purely temporal case data if performing power evaluation without analysis. 
	// In one situation, it is not needed and in the other, case data structures are not defined (no case file).
	CSaTScanData::PostDataRead();
	if (!(gParameters.getPerformPowerEvaluation() &&
		(gParameters.getPowerEvaluationMethod() == PE_ONLY_CASEFILE || gParameters.getPowerEvaluationMethod() == PE_ONLY_SPECIFIED_CASES))) {
		SetPurelyTemporalCases();
	}
}

/** Allocates probability model object. */
void CPurelyTemporalData::SetProbabilityModel() {
  switch (gParameters.GetProbabilityModelType()) {
     case POISSON     : m_pModel = new CPoissonModel(*this);   break;
     case BERNOULLI   : m_pModel = new CBernoulliModel(); break;
     case CATEGORICAL :
     case ORDINAL     : m_pModel = new OrdinalModel(); break;
     case EXPONENTIAL : m_pModel = new ExponentialModel(); break;
     case NORMAL      : m_pModel = new CNormalModel(); break;
     case RANK        : m_pModel = new CRankModel(); break;
     case UNIFORMTIME : m_pModel = new UniformTimeModel(*this); break;
     case BATCHED     : m_pModel = new BatchedModel(); break;
     default : throw prg_error("Not implemented for probability model type: '%d'.\n", "SetProbabilityModel()", gParameters.GetProbabilityModelType());
  }
}

//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "SpaceTimeData.h"
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
CSpaceTimeData::CSpaceTimeData(const CParameters& Parameters, BasePrint& PrintDirection)
               :CSaTScanData(Parameters, PrintDirection) {
  try {
    SetProbabilityModel();
  }
  catch (prg_exception& x) {
    x.addTrace("constructor()","CSpaceTimeData");
    throw;
  }
}

/** class destructor */
CSpaceTimeData::~CSpaceTimeData() {}

/** Calls base class CSaTScanData::CalculateMeasure(). If purely temporal
    clusters were requested, ensures that each datasets' corresponding
    temporal data structures are allocated and set. */
void CSpaceTimeData::CalculateMeasure(RealDataSet& DataSet) {
  try {
    CSaTScanData::CalculateMeasure(DataSet);
    if (gParameters.GetIncludePurelyTemporalClusters() && 
        gParameters.GetProbabilityModelType() != ORDINAL && gParameters.GetProbabilityModelType() != CATEGORICAL)
      gDataSets->SetPurelyTemporalMeasureData(DataSet);
  }
  catch (prg_exception& x) {
    x.addTrace("CalculateMeasure()","CSpaceTimeData");
    throw;
  }
}

/** Randomizes collection of simulation data in concert with passed collection of randomizers. */
void CSpaceTimeData::RandomizeData(RandomizerContainer_t& RandomizerContainer,
                                   SimulationDataContainer_t& SimDataContainer,
                                   unsigned int iSimulationNumber) const {
  try {
    CSaTScanData::RandomizeData(RandomizerContainer, SimDataContainer, iSimulationNumber);
    if (gParameters.GetIncludePurelyTemporalClusters())
      gDataSets->SetPurelyTemporalSimulationData(SimDataContainer);
  }
  catch (prg_exception& x) {
    x.addTrace("RandomizeData()","CSpaceTimeData");
    throw;
  }
}

void CSpaceTimeData::PostDataRead() {
    try {
        // Do not set purely temporal case data if performing power evaluation without analysis. 
        // In one situation, it is not needed and in the other, case data structures are not defined (no case file).
		CSaTScanData::PostDataRead();
        if (!(gParameters.getPerformPowerEvaluation() && 
             (gParameters.getPowerEvaluationMethod() == PE_ONLY_CASEFILE || gParameters.getPowerEvaluationMethod() == PE_ONLY_SPECIFIED_CASES))) {
            SetPurelyTemporalCases();
            if (gParameters.GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION) {
                for (size_t t = 0; t < gDataSets->GetNumDataSets(); ++t)
                    gDataSets->GetDataSet(t).setMeasureData_PT();
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("PostDataRead()","CSpaceTimeData");
        throw;
    }
}

/** Calls base class method CSaTScanData::SetIntervalCut() to calculate the
    maximum temporal cluster size in terms of time interval slices. Please see
    documentation cotained in function for further explain of derived functions
    other tasks. */
void CSpaceTimeData::SetIntervalCut() {
  try {
    CSaTScanData::SetIntervalCut();
    if (gParameters.GetIncludePurelySpatialClusters() && GetNumTimeIntervals() == m_nIntervalCut)
      //This code supposedly prevents calculating a purely temporal cluster twice, once
      //expliciatly by user's request; another when the maximum temporal window is equal
      //to the number of time interval slices. I'm not sure if the latter is possible.
      m_nIntervalCut--;
  }
  catch (prg_exception& x) {
    x.addTrace("SetIntervalCut()","CSpaceTimeData");
    throw;
  }
}

/** Allocates probability model object.  */
void CSpaceTimeData::SetProbabilityModel() {
  switch (gParameters.GetProbabilityModelType()) {
     case POISSON              : m_pModel = new CPoissonModel(*this);   break;
     case BERNOULLI            : m_pModel = new CBernoulliModel(); break;
     case CATEGORICAL          :
     case ORDINAL              : m_pModel = new OrdinalModel(); break;
     case EXPONENTIAL          : m_pModel = new ExponentialModel(); break;
     case NORMAL               : m_pModel = new CNormalModel(); break;
     case RANK                 : m_pModel = new CRankModel(); break;
     case SPACETIMEPERMUTATION : m_pModel = new CSpaceTimePermutationModel(); break;
     case UNIFORMTIME          : m_pModel = new UniformTimeModel(*this); break;
     case BATCHED              : m_pModel = new BatchedModel(); break;
     default : throw prg_error("Unknown probability model type: '%d'.\n", "SetProbabilityModel()", gParameters.GetProbabilityModelType());
  }
}



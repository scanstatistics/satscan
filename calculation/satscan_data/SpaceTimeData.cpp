//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "SpaceTimeData.h"
#include "PoissonModel.h"
#include "BernoulliModel.h"
#include "SpaceTimePermutationModel.h"
#include "NormalModel.h"
#include "SurvivalModel.h"
#include "RankModel.h"

/** class constructor */
CSpaceTimeData::CSpaceTimeData(const CParameters* pParameters, BasePrint *pPrintDirection)
               :CSaTScanData(pParameters, pPrintDirection) {
  try {
    SetProbabilityModel();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CSpaceTimeData");
    throw;
  }
}

/** class destructor */
CSpaceTimeData::~CSpaceTimeData() {}

/** Calls base class CSaTScanData::CalculateMeasure(). If purely temporal
    clusters were requested, ensures that each data streams' corresponding
    temporal data structures are allocated and set. */
void CSpaceTimeData::CalculateMeasure(RealDataStream& thisStream) {
  try {
    CSaTScanData::CalculateMeasure(thisStream);
    if (m_pParameters->GetIncludePurelyTemporalClusters())
      gpDataStreams->SetPurelyTemporalMeasureData(thisStream);
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateMeasure()","CSpaceTimeData");
    throw;
  }
}

/** Randomizes collection of simulation data in concert with passed collection of randomizers. */
void CSpaceTimeData::RandomizeData(RandomizerContainer_t& RandomizerContainer,
                                   SimulationDataContainer_t& SimDataContainer,
                                   unsigned int iSimulationNumber) const {
  try {
    CSaTScanData::RandomizeData(RandomizerContainer, SimDataContainer, iSimulationNumber);
    if (m_pParameters->GetIncludePurelyTemporalClusters())
      for (size_t t=0; t < SimDataContainer.size(); ++t)
        SimDataContainer[t]->SetPTCasesArray();
  }
  catch (ZdException &x) {
    x.AddCallpath("RandomizeData()","CSpaceTimeData");
    throw;
  }
}

/** Calls base class CSaTScanData::ReadDataFromFiles().If purely temporal
    clusters were requested, ensures that each data streams' corresponding
    data structures are allocated and set. */
void CSpaceTimeData::ReadDataFromFiles() {
  try {
    CSaTScanData::ReadDataFromFiles();
    if (m_pParameters->GetIncludePurelyTemporalClusters())
      SetPurelyTemporalCases();
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadDataFromFiles()","CSpaceTimeData");
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
    if (m_pParameters->GetIncludePurelySpatialClusters() && m_nTimeIntervals == m_nIntervalCut)
      //This code supposedly prevents calculating a purely temporal cluster twice, once
      //expliciatly by user's request; another when the maximum temporal window is equal
      //to the number of time interval slices. I'm not sure if the latter is possible.
      m_nIntervalCut--;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetIntervalCut()","CSpaceTimeData");
    throw;
  }
}

/** Allocates probability model object.  */
void CSpaceTimeData::SetProbabilityModel() {
  try {
    switch (m_pParameters->GetProbabiltyModelType()) {
       case POISSON              : m_pModel = new CPoissonModel(*m_pParameters, *this, *gpPrint);   break;
       case BERNOULLI            : m_pModel = new CBernoulliModel(*m_pParameters, *this, *gpPrint); break;
       case NORMAL               : m_pModel = new CNormalModel(*m_pParameters, *this, *gpPrint); break;
       case SURVIVAL             : m_pModel = new CSurvivalModel(*m_pParameters, *this, *gpPrint); break;
       case RANK                 : m_pModel = new CRankModel(*m_pParameters, *this, *gpPrint); break;
       case SPACETIMEPERMUTATION : m_pModel = new CSpaceTimePermutationModel(*m_pParameters, *this, *gpPrint); break;
       default : ZdException::Generate("Unknown probability model type: '%d'.\n",
                                       "SetProbabilityModel()", m_pParameters->GetProbabiltyModelType());
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetProbabilityModel()","CSpaceTimeData");
    throw;
  }
}



#include "SaTScan.h"
#pragma hdrstop
#include "SpaceTimeData.h"
#include "PoissonModel.h"
#include "BernoulliModel.h"
#include "SpaceTimePermutationModel.h"
#include "NormalModel.h"
#include "SurvivalModel.h"
#include "RankModel.h"

/** constructor */
CSpaceTimeData::CSpaceTimeData(CParameters* pParameters, BasePrint *pPrintDirection)
               :CSaTScanData(pParameters, pPrintDirection) {
  try {
    SetProbabilityModel();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CSpaceTimeData");
    throw;
  }
}

/** desctructor */
CSpaceTimeData::~CSpaceTimeData() {}

bool CSpaceTimeData::CalculateMeasure(DataStream & thisStream) {
  bool bResult;

  try {
    bResult = CSaTScanData::CalculateMeasure(thisStream);
    if (m_pParameters->GetIncludePurelyTemporalClusters())
      gpDataStreams->SetPurelyTemporalMeasureData(thisStream);
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateMeasure()","CSpaceTimeData");
    throw;
  }
  return bResult;
}

void CSpaceTimeData::RandomizeData(int iSimulationNumber) {
  try {
    CSaTScanData::RandomizeData(iSimulationNumber);
    if (m_pParameters->GetIncludePurelyTemporalClusters())
      gpDataStreams->SetPurelyTemporalSimulationData();
  }
  catch (ZdException &x) {
    x.AddCallpath("RandomizeData()","CSpaceTimeData");
    throw;
  }
}

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

void CSpaceTimeData::SetIntervalCut()
{
   try
      {
      CSaTScanData::SetIntervalCut();

      /* Avoids double calculations of the loglikelihood when IPS==1 and     */
      /* IntervalCut==nTimeIntervals. Increases speed in functions Cluster2() */
      /* and Montercarlo2().                                                 */
      if (m_pParameters->GetIncludePurelySpatialClusters())
         if (m_nTimeIntervals == m_nIntervalCut)
            m_nIntervalCut--;
      }
   catch (ZdException & x)
      {
      x.AddCallpath("SetIntervalCut()", "CSpaceTimeData");
      throw;
      }
}

/** allocates probability model */
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



//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "PurelySpatialData.h"
#include "PoissonModel.h"
#include "BernoulliModel.h"
#include "SpaceTimePermutationModel.h"
#include "NormalModel.h"
#include "SurvivalModel.h"
#include "RankModel.h"

/** class constructor */
CPurelySpatialData::CPurelySpatialData(const CParameters* pParameters, BasePrint *pPrintDirection)
                   :CSaTScanData(pParameters, pPrintDirection) {
  try {
    SetProbabilityModel();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CPurelySpatialData");
    throw;
  }
}

/** class destructor */
CPurelySpatialData::~CPurelySpatialData() {}

/** Calculates time interval start times for a purely spatial analysis; of which
    there is only one time interval - the study period. */
void CPurelySpatialData::SetIntervalStartTimes() {
  gvTimeIntervalStartTimes.clear();
  gvTimeIntervalStartTimes.push_back(m_nStartDate);
  gvTimeIntervalStartTimes.push_back(m_nEndDate+1);
  m_nTimeIntervals = 1;
}

/** Allocates probability model object. Throws ZdException if probability model
    type is space-time permutation. */
void CPurelySpatialData::SetProbabilityModel() {
  try {
    switch (m_pParameters->GetProbabiltyModelType()) {
       case POISSON              : m_pModel = new CPoissonModel(*m_pParameters, *this, *gpPrint);   break;
       case BERNOULLI            : m_pModel = new CBernoulliModel(*m_pParameters, *this, *gpPrint); break;
       case NORMAL               : m_pModel = new CNormalModel(*m_pParameters, *this, *gpPrint); break;
       case SURVIVAL             : m_pModel = new CSurvivalModel(*m_pParameters, *this, *gpPrint); break;
       case RANK                 : m_pModel = new CRankModel(*m_pParameters, *this, *gpPrint); break;
       case SPACETIMEPERMUTATION : ZdException::Generate("Purely Spatial analysis not implemented for Space-Time Permutation model.\n",
                                                         "SetProbabilityModel()");
       default : ZdException::Generate("Unknown probability model type: '%d'.\n", "SetProbabilityModel()",
                                       m_pParameters->GetProbabiltyModelType());
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetProbabilityModel()","CPurelySpatialData");
    throw;
  }
}

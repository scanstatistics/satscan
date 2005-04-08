//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "PurelySpatialData.h"
#include "PoissonModel.h"
#include "BernoulliModel.h"
#include "SpaceTimePermutationModel.h"
#include "NormalModel.h"
#include "ExponentialModel.h"
#include "RankModel.h"
#include "OrdinalModel.h"

/** class constructor */
CPurelySpatialData::CPurelySpatialData(const CParameters& Parameters, BasePrint& PrintDirection)
                   :CSaTScanData(Parameters, PrintDirection) {
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
    switch (gParameters.GetProbabilityModelType()) {
       case POISSON              : m_pModel = new CPoissonModel(*this);   break;
       case BERNOULLI            : m_pModel = new CBernoulliModel(); break;
       case ORDINAL              : m_pModel = new OrdinalModel(); break;
       case EXPONENTIAL          : m_pModel = new ExponentialModel(); break;
       case NORMAL               : m_pModel = new CNormalModel(); break;
       case RANK                 : m_pModel = new CRankModel(); break;
       case SPACETIMEPERMUTATION : ZdException::Generate("Purely Spatial analysis not implemented for Space-Time Permutation model.\n",
                                                         "SetProbabilityModel()");
       default : ZdException::Generate("Unknown probability model type: '%d'.\n", "SetProbabilityModel()",
                                       gParameters.GetProbabilityModelType());
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetProbabilityModel()","CPurelySpatialData");
    throw;
  }
}

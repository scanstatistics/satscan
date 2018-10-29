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
#include "SSException.h"
#include "HomogeneousPoissonModel.h"
#include "UniformTimeModel.h"

/** class constructor */
CPurelySpatialData::CPurelySpatialData(const CParameters& Parameters, BasePrint& PrintDirection)
                   :CSaTScanData(Parameters, PrintDirection) {
  try {
    SetProbabilityModel();
  }
  catch (prg_exception& x) {
    x.addTrace("constructor()","CPurelySpatialData");
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

/** Allocates probability model object. Throws prg_error if probability model
    type is space-time permutation. */
void CPurelySpatialData::SetProbabilityModel() {
  switch (gParameters.GetProbabilityModelType()) {
     case POISSON              : m_pModel = new CPoissonModel(*this);   break;
     case BERNOULLI            : m_pModel = new CBernoulliModel(); break;
     case CATEGORICAL          :
     case ORDINAL              : m_pModel = new OrdinalModel(); break;
     case EXPONENTIAL          : m_pModel = new ExponentialModel(); break;
     case NORMAL               : m_pModel = new CNormalModel(); break;
     case RANK                 : m_pModel = new CRankModel(); break;
     case UNIFORMTIME          : m_pModel = new UniformTimeModel(*this); break;
     case HOMOGENEOUSPOISSON   : m_pModel = new HomogenousPoissonModel(); break;
     case SPACETIMEPERMUTATION : throw prg_error("Purely Spatial analysis not implemented for Space-Time Permutation model.\n",
                                                 "SetProbabilityModel()");
     default : throw prg_error("Unknown probability model type: '%d'.\n", "SetProbabilityModel()",
                               gParameters.GetProbabilityModelType());
  }
}

#include "SaTScan.h"
#pragma hdrstop
#include "PurelySpatialData.h"
#include "PoissonModel.h"
#include "BernoulliModel.h"
#include "SpaceTimePermutationModel.h"

/** constructor */
CPurelySpatialData::CPurelySpatialData(CParameters* pParameters, BasePrint *pPrintDirection)
                   :CSaTScanData(pParameters, pPrintDirection) {
  try {
    SetProbabilityModel();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CPurelySpatialData");
    throw;
  }
}

/** destructor */
CPurelySpatialData::~CPurelySpatialData() {}

/** allocates probability model */
void CPurelySpatialData::SetProbabilityModel() {
  try {
    switch (m_pParameters->GetProbabiltyModelType()) {
       case POISSON              : m_pModel = new CPoissonModel(*m_pParameters, *this, *gpPrint);   break;
       case BERNOULLI            : m_pModel = new CBernoulliModel(*m_pParameters, *this, *gpPrint); break;
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

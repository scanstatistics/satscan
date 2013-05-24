//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "HomogeneousPoissonModel.h"
#include "SaTScanData.h"
#include "SSException.h"

/** constructor */
HomogenousPoissonModel::HomogenousPoissonModel() : CModel() {}

/** destructor */
HomogenousPoissonModel::~HomogenousPoissonModel() {}

/** No operation performed for this probability model. */
void HomogenousPoissonModel::CalculateMeasure(RealDataSet&, const CSaTScanData&) {/* no action here */}

/** Returns population as defined in CCluster object. Not implemeneted yet - throws exception. */
double HomogenousPoissonModel::GetPopulation(size_t tSetIndex, const CCluster& Cluster, const CSaTScanData&) const {
  throw prg_error("GetPopulation() not implementated.","HomogenousPoissonModel");
}
 

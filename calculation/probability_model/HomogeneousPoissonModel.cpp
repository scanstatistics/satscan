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

/** TODO : document */
void HomogenousPoissonModel::CalculateMeasure(RealDataSet&) {/* no action here */}

/** Returns population as defined in CCluster object. Not implemeneted yet - throws exception. */
double HomogenousPoissonModel::GetPopulation(size_t tSetIndex, const CCluster& Cluster, const CSaTScanData&) const {
  throw prg_error("GetPopulation() not implementated.","HomogenousPoissonModel");
}
 

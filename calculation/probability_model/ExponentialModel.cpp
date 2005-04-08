//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ExponentialModel.h"
#include "SaTScanData.h"

/** constructor */
ExponentialModel::ExponentialModel() : CModel() {}

/** destructor */
ExponentialModel::~ExponentialModel() {}

/** TODO: document */
void ExponentialModel::CalculateMeasure(RealDataSet&) {/* no action here */}

/** Returns population as defined in CCluster object. Not implemeneted yet - throws exception. */
double ExponentialModel::GetPopulation(size_t tSetIndex, const CCluster& Cluster, const CSaTScanData&) const {
  ZdGenerateException("GetPopulation() not implementated YET!!!.","ExponentialModel");
  return 0;
}


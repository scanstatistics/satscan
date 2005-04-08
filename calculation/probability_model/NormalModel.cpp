//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "NormalModel.h"

/** constructor */
CNormalModel::CNormalModel() : CModel() {}

/** destructor */
CNormalModel::~CNormalModel() {}

/** TODO : document */
void CNormalModel::CalculateMeasure(RealDataSet&) {/* no action here */}

/** Returns population as defined in CCluster object. Not implemeneted yet - throws exception. */
double CNormalModel::GetPopulation(size_t tSetIndex, const CCluster& Cluster, const CSaTScanData&) const {
  ZdGenerateException("GetPopulation() not implementated.","CNormalModel");
  return 0;
}


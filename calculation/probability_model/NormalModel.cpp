//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "NormalModel.h"
#include "SSException.h"

/** constructor */
CNormalModel::CNormalModel() : CModel() {}

/** destructor */
CNormalModel::~CNormalModel() {}

/** No operation performed for this probability model. */
void CNormalModel::CalculateMeasure(RealDataSet&, const CSaTScanData&) {/* no action here */}

/** Returns population as defined in CCluster object. Not implemeneted yet - throws exception. */
double CNormalModel::GetPopulation(size_t tSetIndex, const CCluster& Cluster, const CSaTScanData&) const {
  throw prg_error("GetPopulation() not implementated.","CNormalModel");
}

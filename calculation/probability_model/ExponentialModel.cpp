//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ExponentialModel.h"
#include "SaTScanData.h"

/** constructor */
ExponentialModel::ExponentialModel(const CParameters& Parameters, CSaTScanData& DataHub, BasePrint& PrintDirection)
                 :CModel(Parameters, DataHub, PrintDirection) {}

/** destructor */
ExponentialModel::~ExponentialModel() {}

void ExponentialModel::CalculateMeasure(RealDataSet&) {
  /* no action here */
}

/** Returns population as defined in CCluster object. Not implemeneted yet - throws exception. */
double ExponentialModel::GetPopulation(size_t tSetIndex, const CCluster& Cluster) const {
  ZdGenerateException("GetPopulation() not implementated YET!!!.","ExponentialModel");
  return 0;
}


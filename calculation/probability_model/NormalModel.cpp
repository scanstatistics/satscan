//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "NormalModel.h"

/** constructor */
CNormalModel::CNormalModel(const CParameters& Parameters, CSaTScanData& DataHub, BasePrint& PrintDirection)
             :CModel(Parameters, DataHub, PrintDirection) {}

/** destructor */
CNormalModel::~CNormalModel() {}

void CNormalModel::CalculateMeasure(RealDataSet&) {
  /* no action here */
}

/** Returns population as defined in CCluster object. Not implemeneted yet - throws exception. */
double CNormalModel::GetPopulation(size_t tSetIndex, const CCluster& Cluster) const {
  ZdGenerateException("GetPopulation() not implementated.","CNormalModel");
  return 0;
}


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

double ExponentialModel::GetPopulation(size_t, int, tract_t, tract_t, int, int) const {
  ZdGenerateException("GetPopulation() not implementated YET!!!.","ExponentialModel");
  return 0;
}


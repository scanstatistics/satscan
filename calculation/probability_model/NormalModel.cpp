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

double CNormalModel::GetPopulation(size_t, int, tract_t, tract_t, int, int) const {
  ZdGenerateException("GetPopulation() not implementated.","CNormalModel");
  return 0;
}


//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "OrdinalModel.h"

/** constructor */
OrdinalModel::OrdinalModel(const CParameters& Parameters, CSaTScanData& DataHub, BasePrint& PrintDirection)
             :CModel(Parameters, DataHub, PrintDirection) {}

/** destructor */
OrdinalModel::~OrdinalModel() {}

void OrdinalModel::CalculateMeasure(RealDataSet&) {
   /* no action here */
}

double OrdinalModel::GetPopulation(size_t, int, tract_t, tract_t, int, int) const {
  ZdGenerateException("GetPopulation() not implementated YET!","OrdinalModel");
  return 0;
}


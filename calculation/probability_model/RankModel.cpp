//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "RankModel.h"
#include "SaTScanData.h"

/** constructor */
CRankModel::CRankModel(const CParameters& Parameters, CSaTScanData& DataHub, BasePrint& PrintDirection)
           :CModel(Parameters, DataHub, PrintDirection) {}

/** destructor */
CRankModel::~CRankModel() {}

void CRankModel::CalculateMeasure(RealDataSet&) {
  /* no action here */
}

/** not implemented - throws exception */
double CRankModel::GetPopulation(size_t, int, tract_t, tract_t, int, int) const {
  ZdGenerateException("GetPopulation() not implementated.","CRankModel");
  return 0;
}
 

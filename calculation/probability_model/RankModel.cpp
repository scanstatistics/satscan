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

/** Returns population as defined in CCluster object. Not implemeneted yet - throws exception. */
double CRankModel::GetPopulation(size_t tSetIndex, const CCluster& Cluster) const {
  ZdGenerateException("GetPopulation() not implementated.","CRankModel");
  return 0;
}
 

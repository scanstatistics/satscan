//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "RankModel.h"
#include "SaTScanData.h"

/** constructor */
CRankModel::CRankModel() : CModel() {}

/** destructor */
CRankModel::~CRankModel() {}

/** TODO : document */
void CRankModel::CalculateMeasure(RealDataSet&) {/* no action here */}

/** Returns population as defined in CCluster object. Not implemeneted yet - throws exception. */
double CRankModel::GetPopulation(size_t tSetIndex, const CCluster& Cluster, const CSaTScanData&) const {
  ZdGenerateException("GetPopulation() not implementated.","CRankModel");
  return 0;
}
 

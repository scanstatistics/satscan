//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "RankModel.h"
#include "SaTScanData.h"
#include "SSException.h"

/** constructor */
CRankModel::CRankModel() : CModel() {}

/** destructor */
CRankModel::~CRankModel() {}

/** No operation performed for this probability model. */
void CRankModel::CalculateMeasure(RealDataSet&, const CSaTScanData&) {/* no action here */}

/** Returns population as defined in CCluster object. Not implemeneted yet - throws exception. */
double CRankModel::GetPopulation(size_t tSetIndex, const CCluster& Cluster, const CSaTScanData&) const {
  throw prg_error("GetPopulation() not implementated.","CRankModel");
}
 

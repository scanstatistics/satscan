//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ProbabilityModel.h"
#include "SaTScanData.h"

/** constructor */
CModel::CModel() {}

/** destructor */
CModel::~CModel() {}

/** Returns population as defined in CCluster object. Not implemented - throws exception. */
double CModel::GetPopulation(size_t, const CCluster&, const CSaTScanData&) const {
  ZdGenerateException("GetPopulation() not implementated.","CModel");
  return 0;
}


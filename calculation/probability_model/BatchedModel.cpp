//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "BatchedModel.h"
#include "SaTScanData.h"
#include "cluster.h"
#include "SSException.h"

/** Returns population (total number of traps) as defined in CCluster object. */
double BatchedModel::GetPopulation(size_t tSetIndex, const CCluster& Cluster, const CSaTScanData& DataHub) const {
    throw prg_error("GetPopulation() not implementated.", "BatchedModel");
}

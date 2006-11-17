//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "AbstractClusterDataFactory.h"
#include "SSException.h"

/** Not implemented - throws prg_error. */
AbtractSVTTClusterData * AbstractClusterDataFactory::GetNewSVTTClusterData(const DataSetInterface&) const {
  throw prg_error("GetNewSVTTClusterData(const DataSetInterface&) not implemented.","AbstractClusterDataFactory");
}

/** Not implemented - throws prg_error. */
AbtractSVTTClusterData * AbstractClusterDataFactory::GetNewSVTTClusterData(const AbstractDataSetGateway&) const {
  throw prg_error("GetNewSVTTClusterData(const AbstractDataSetGateway&) not implemented.","AbstractClusterDataFactory");
}


//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "AbstractClusterDataFactory.h"

/** Not implemented - throws ZdException. */
AbtractSVTTClusterData * AbstractClusterDataFactory::GetNewSVTTClusterData(const DataSetInterface&) const {
  ZdGenerateException("GetNewSVTTClusterData(const DataSetInterface&) not implemented.","AbstractClusterDataFactory");
  return 0;
}

/** Not implemented - throws ZdException. */
AbtractSVTTClusterData * AbstractClusterDataFactory::GetNewSVTTClusterData(const AbstractDataSetGateway&) const {
  ZdGenerateException("GetNewSVTTClusterData(const AbstractDataSetGateway&) not implemented.","AbstractClusterDataFactory");
  return 0;
}


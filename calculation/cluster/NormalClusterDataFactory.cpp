//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "NormalClusterDataFactory.h"
#include "NormalClusterData.h"

/** class constructor */
NormalClusterDataFactory::NormalClusterDataFactory() : AbstractClusterDataFactory() {}

/** class destructor */
NormalClusterDataFactory::~NormalClusterDataFactory() {}

/** Not implemented. Throws ZdException. */
AbstractSpatialClusterData * NormalClusterDataFactory::GetNewSpatialClusterData(const DataSetInterface&, int) const {
  ZdGenerateException("GetNewSpatialClusterData(const DataSetInterface&, int) not implemented.","NormalClusterDataFactory");
  return 0;
}

/** Returns newly created NormalSpatialData object as AbstractSpatialClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractSpatialClusterData * NormalClusterDataFactory::GetNewSpatialClusterData(const AbstractDataSetGateway& DataGateway, int iRate) const {
  return new NormalSpatialData(DataGateway, iRate);
}

/** Not implemented. Throws ZdException. */
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData&, const DataSetInterface&) const {
  ZdGenerateException("GetNewProspectiveSpatialClusterData(const DataSetInterface&, int) not implemented.","NormalClusterDataFactory");
  return 0;
//  return new NormalProspectiveSpatialData(Data, Interface); ???
}

/** Returns newly created NormalProspectiveSpatialData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway) const {
  return new NormalProspectiveSpatialData(Data, DataGateway);
}

/** Not implemented. Throws ZdException. */
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewTemporalClusterData(const DataSetInterface&) const {
  ZdGenerateException("GetNewTemporalClusterData(const DataSetInterface&) not implemented.","NormalClusterDataFactory");
  return 0;
}

/** Returns newly created NormalTemporalData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewTemporalClusterData(const AbstractDataSetGateway& DataGateway) const {
  return new NormalTemporalData(DataGateway);
}

/** Not implemented. Throws ZdException. */
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewSpaceTimeClusterData(const DataSetInterface&) const {
  ZdGenerateException("GetNewSpaceTimeClusterData(const DataSetInterface&) not implemented.","NormalClusterDataFactory");
  return 0;
}

/** Returns newly created NormalSpaceTimeData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewSpaceTimeClusterData(const AbstractDataSetGateway& DataGateway) const {
  return new NormalSpaceTimeData(DataGateway);
}


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
AbstractSpatialClusterData * NormalClusterDataFactory::GetNewSpatialClusterData(const DataStreamInterface&, int) const {
  ZdGenerateException("GetNewSpatialClusterData(const DataStreamInterface&, int) not implemented.","NormalClusterDataFactory");
  return 0;
}

/** Returns newly created NormalSpatialData object as AbstractSpatialClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractSpatialClusterData * NormalClusterDataFactory::GetNewSpatialClusterData(const AbtractDataStreamGateway& DataGateway, int iRate) const {
  return new NormalSpatialData(DataGateway, iRate);
}

/** Not implemented. Throws ZdException. */
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData&, const DataStreamInterface&) const {
  ZdGenerateException("GetNewProspectiveSpatialClusterData(const DataStreamInterface&, int) not implemented.","NormalClusterDataFactory");
  return 0;
//  return new NormalProspectiveSpatialData(Data, Interface); ???
}

/** Returns newly created NormalProspectiveSpatialData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData& Data, const AbtractDataStreamGateway& DataGateway) const {
  return new NormalProspectiveSpatialData(Data, DataGateway);
}

/** Not implemented. Throws ZdException. */
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewTemporalClusterData(const DataStreamInterface&) const {
  ZdGenerateException("GetNewTemporalClusterData(const DataStreamInterface&) not implemented.","NormalClusterDataFactory");
  return 0;
}

/** Returns newly created NormalTemporalData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewTemporalClusterData(const AbtractDataStreamGateway& DataGateway) const {
  return new NormalTemporalData(DataGateway);
}

/** Not implemented. Throws ZdException. */
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewSpaceTimeClusterData(const DataStreamInterface&) const {
  ZdGenerateException("GetNewSpaceTimeClusterData(const DataStreamInterface&) not implemented.","NormalClusterDataFactory");
  return 0;
}

/** Returns newly created NormalSpaceTimeData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewSpaceTimeClusterData(const AbtractDataStreamGateway& DataGateway) const {
  return new NormalSpaceTimeData(DataGateway);
}


//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "NormalClusterDataFactory.h"
#include "NormalClusterData.h"
#include "MultiSetNormalClusterData.h"

/** class constructor */
NormalClusterDataFactory::NormalClusterDataFactory() : AbstractClusterDataFactory() {}

/** class destructor */
NormalClusterDataFactory::~NormalClusterDataFactory() {}

/** Not implemented. Throws ZdException. */
AbstractSpatialClusterData * NormalClusterDataFactory::GetNewSpatialClusterData(const DataSetInterface& Interface, int iRate) const {
  return new NormalSpatialData(Interface, iRate);
}

/** Returns newly created NormalSpatialData object as AbstractSpatialClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractSpatialClusterData * NormalClusterDataFactory::GetNewSpatialClusterData(const AbstractDataSetGateway& DataGateway, int iRate) const {
  return new NormalSpatialData(DataGateway, iRate);
}

/** Not implemented. Throws ZdException. */
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData& DataHub, const DataSetInterface& Interface) const {
  return new NormalProspectiveSpatialData(DataHub, Interface);
}

/** Returns newly created NormalProspectiveSpatialData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway) const {
  return new NormalProspectiveSpatialData(Data, DataGateway);
}

/** Not implemented. Throws ZdException. */
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewTemporalClusterData(const DataSetInterface& Interface) const {
  return new NormalTemporalData(Interface);
}

/** Returns newly created NormalTemporalData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewTemporalClusterData(const AbstractDataSetGateway& DataGateway) const {
  return new NormalTemporalData(DataGateway);
}

/** Not implemented. Throws ZdException. */
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewSpaceTimeClusterData(const DataSetInterface& Interface) const {
  return new NormalSpaceTimeData(Interface);
}

/** Returns newly created NormalSpaceTimeData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewSpaceTimeClusterData(const AbstractDataSetGateway& DataGateway) const {
  return new NormalSpaceTimeData(DataGateway);
}

//***************** class MultiSetNormalClusterDataFactory *********************

/** class constructor */
MultiSetNormalClusterDataFactory::MultiSetNormalClusterDataFactory() : AbstractClusterDataFactory() {}

/** class destructor */
MultiSetNormalClusterDataFactory::~MultiSetNormalClusterDataFactory() {}

/** Not implemented. Throws ZdException. */
AbstractSpatialClusterData * MultiSetNormalClusterDataFactory::GetNewSpatialClusterData(const DataSetInterface&, int) const {
  ZdGenerateException("GetNewSpatialClusterData(const DataSetInterface&, int) not implemented.","MultiSetNormalClusterDataFactory");
  return 0;
}

/** Returns newly created MultiSetNormalSpatialData object as AbstractSpatialClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractSpatialClusterData * MultiSetNormalClusterDataFactory::GetNewSpatialClusterData(const AbstractDataSetGateway& DataGateway, int iRate) const {
  return new MultiSetNormalSpatialData(gClusterDataFactory, DataGateway, iRate);
}

/** Not implemented. Throws ZdException. */
AbstractTemporalClusterData * MultiSetNormalClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData&, const DataSetInterface&) const {
  ZdGenerateException("GetNewProspectiveSpatialClusterData(const CSaTScanData&, const DataSetInterface&) not implemented.","MultiSetNormalClusterDataFactory");
  return 0;
}

/** Returns newly created MultiSetNormalProspectiveSpatialData object as
    AbstractTemporalClusterData pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * MultiSetNormalClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway) const {
  return new MultiSetNormalProspectiveSpatialData(gClusterDataFactory, Data, DataGateway);
}

/** Not implemented. Throws ZdException. */
AbstractTemporalClusterData * MultiSetNormalClusterDataFactory::GetNewTemporalClusterData(const DataSetInterface&) const {
  ZdGenerateException("GetNewTemporalClusterData(const DataSetInterface&) not implemented.","MultiSetNormalClusterDataFactory");
  return 0;
}

/** Returns newly created MultiSetNormalTemporalData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * MultiSetNormalClusterDataFactory::GetNewTemporalClusterData(const AbstractDataSetGateway& DataGateway) const {
  return new MultiSetNormalTemporalData(gClusterDataFactory, DataGateway);
}

/** Not implemented. Throws ZdException. */
AbstractTemporalClusterData * MultiSetNormalClusterDataFactory::GetNewSpaceTimeClusterData(const DataSetInterface&) const {
  ZdGenerateException("GetNewSpaceTimeClusterData(const DataSetInterface&) not implemented.","MultiSetNormalClusterDataFactory");
  return 0;
}

/** Returns newly created MultiSetNormalSpaceTimeData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * MultiSetNormalClusterDataFactory::GetNewSpaceTimeClusterData(const AbstractDataSetGateway& DataGateway) const {
  return new MultiSetNormalSpaceTimeData(gClusterDataFactory, DataGateway);
}


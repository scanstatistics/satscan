//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "UniformTimeClusterDataFactory.h"
#include "UniformTimeClusterData.h"
#include "MultiSetUniformTimeClusterData.h"
#include "SSException.h"
#include "SaTScanData.h"

//***************** class UniformTimeClusterDataFactory *********************

/** class constructor */
UniformTimeClusterDataFactory::UniformTimeClusterDataFactory() : AbstractClusterDataFactory() {}

/** class destructor */
UniformTimeClusterDataFactory::~UniformTimeClusterDataFactory() {}

/** Not implemented. Throws prg_error. */
AbstractSpatialClusterData * UniformTimeClusterDataFactory::GetNewSpatialClusterData(const DataSetInterface& Interface) const {
  throw prg_error("GetNewSpatialClusterData(const DataSetInterface&) not implemented.","UniformTimeClusterDataFactory");
}

/** Returns newly created UniformSpatialData object as AbstractSpatialClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractSpatialClusterData * UniformTimeClusterDataFactory::GetNewSpatialClusterData(const AbstractDataSetGateway& DataGateway) const {
  throw prg_error("GetNewSpatialClusterData(const AbstractDataSetGateway&) not implemented.","UniformTimeClusterDataFactory");
}

/** Not implemented. Throws prg_error. */
AbstractTemporalClusterData * UniformTimeClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData& DataHub, const DataSetInterface& Interface) const {
  throw prg_error("GetNewProspectiveSpatialClusterData(const CSaTScanData&,  const DataSetInterface&) not implemented.","UniformTimeClusterDataFactory");
}

/** Returns newly created UniformTimeClusterDataFactory object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * UniformTimeClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway) const {
  throw prg_error("GetNewProspectiveSpatialClusterData(const CSaTScanData&,  const AbstractDataSetGateway&) not implemented.","UniformTimeClusterDataFactory");
}

AbstractTemporalClusterData * UniformTimeClusterDataFactory::GetNewTemporalClusterData(const DataSetInterface& Interface) const {
    return new UniformTimeTemporalData(Interface);
}

/** Returns newly created UniformTimeTemporalData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * UniformTimeClusterDataFactory::GetNewTemporalClusterData(const AbstractDataSetGateway& DataGateway) const {
  return new UniformTimeTemporalData(DataGateway);
}

AbstractTemporalClusterData * UniformTimeClusterDataFactory::GetNewSpaceTimeClusterData(const DataSetInterface& Interface) const {
  return new UniformTimeSpaceTimeData(Interface);
}

/** Returns newly created UniformTimeTemporalData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * UniformTimeClusterDataFactory::GetNewSpaceTimeClusterData(const AbstractDataSetGateway& DataGateway) const {
  return new UniformTimeSpaceTimeData(DataGateway);
}

//***************** class MultiSetUniformTimeClusterDataFactory *********************

/** class constructor */
MultiSetUniformTimeClusterDataFactory::MultiSetUniformTimeClusterDataFactory() : AbstractClusterDataFactory() {}

/** class destructor */
MultiSetUniformTimeClusterDataFactory::~MultiSetUniformTimeClusterDataFactory() {}

/** Not implemented. Throws prg_error. */
AbstractSpatialClusterData * MultiSetUniformTimeClusterDataFactory::GetNewSpatialClusterData(const DataSetInterface&) const {
  throw prg_error("GetNewSpatialClusterData(const DataSetInterface&) not implemented.","MultiSetUniformTimeClusterDataFactory");
}

/** Returns newly created MultiSetNormalSpatialData object as AbstractSpatialClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractSpatialClusterData * MultiSetUniformTimeClusterDataFactory::GetNewSpatialClusterData(const AbstractDataSetGateway& DataGateway) const {
  throw prg_error("GetNewSpatialClusterData(const AbstractDataSetGateway&) not implemented.","MultiSetUniformTimeClusterDataFactory");
}

/** Not implemented. Throws prg_error. */
AbstractTemporalClusterData * MultiSetUniformTimeClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData&, const DataSetInterface&) const {
  throw prg_error("GetNewSpatialClusterData(const CSaTScanData&, const DataSetInterface&) not implemented.","MultiSetUniformTimeClusterDataFactory");
}

/** Returns newly created MultiSetNormalProspectiveSpatialData object as
    AbstractTemporalClusterData pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * MultiSetUniformTimeClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway) const {
  throw prg_error("GetNewProspectiveSpatialClusterData(const CSaTScanData&, const AbstractDataSetGateway&) not implemented.","MultiSetUniformTimeClusterDataFactory");
}

/** Not implemented. Throws prg_error. */
AbstractTemporalClusterData * MultiSetUniformTimeClusterDataFactory::GetNewTemporalClusterData(const DataSetInterface&) const {
  throw prg_error("GetNewTemporalClusterData(const DataSetInterface&) not implemented.","MultiSetNormalClusterDataFactory");
}

/** Returns newly created MultiSetNormalTemporalData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * MultiSetUniformTimeClusterDataFactory::GetNewTemporalClusterData(const AbstractDataSetGateway& DataGateway) const {
  return new MultiSetUniformTimeTemporalData(gClusterDataFactory, DataGateway);
}

/** Not implemented. Throws prg_error. */
AbstractTemporalClusterData * MultiSetUniformTimeClusterDataFactory::GetNewSpaceTimeClusterData(const DataSetInterface&) const {
  throw prg_error("GetNewSpaceTimeClusterData(const DataSetInterface&) not implemented.","MultiSetNormalClusterDataFactory");
}

/** Returns newly created MultiSetNormalSpaceTimeData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * MultiSetUniformTimeClusterDataFactory::GetNewSpaceTimeClusterData(const AbstractDataSetGateway& DataGateway) const {
  return new MultiSetUniformTimeSpaceTimeData(gClusterDataFactory, DataGateway);
}


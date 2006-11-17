//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "NormalClusterDataFactory.h"
#include "NormalClusterData.h"
#include "MultiSetNormalClusterData.h"
#include "SSException.h"

/** class constructor */
NormalClusterDataFactory::NormalClusterDataFactory() : AbstractClusterDataFactory() {}

/** class destructor */
NormalClusterDataFactory::~NormalClusterDataFactory() {}

/** Not implemented. Throws prg_error. */
AbstractSpatialClusterData * NormalClusterDataFactory::GetNewSpatialClusterData(const DataSetInterface& Interface) const {
  return new NormalSpatialData(Interface);
}

/** Returns newly created NormalSpatialData object as AbstractSpatialClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractSpatialClusterData * NormalClusterDataFactory::GetNewSpatialClusterData(const AbstractDataSetGateway& DataGateway) const {
  return new NormalSpatialData(DataGateway);
}

/** Not implemented. Throws prg_error. */
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData& DataHub, const DataSetInterface& Interface) const {
  return new NormalProspectiveSpatialData(DataHub, Interface);
}

/** Returns newly created NormalProspectiveSpatialData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway) const {
  return new NormalProspectiveSpatialData(Data, DataGateway);
}

/** Not implemented. Throws prg_error. */
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewTemporalClusterData(const DataSetInterface& Interface) const {
  return new NormalTemporalData(Interface);
}

/** Returns newly created NormalTemporalData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewTemporalClusterData(const AbstractDataSetGateway& DataGateway) const {
  return new NormalTemporalData(DataGateway);
}

/** Not implemented. Throws prg_error. */
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

/** Not implemented. Throws prg_error. */
AbstractSpatialClusterData * MultiSetNormalClusterDataFactory::GetNewSpatialClusterData(const DataSetInterface&) const {
  throw prg_error("GetNewSpatialClusterData(const DataSetInterface&) not implemented.","MultiSetNormalClusterDataFactory");
}

/** Returns newly created MultiSetNormalSpatialData object as AbstractSpatialClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractSpatialClusterData * MultiSetNormalClusterDataFactory::GetNewSpatialClusterData(const AbstractDataSetGateway& DataGateway) const {
  return new MultiSetNormalSpatialData(gClusterDataFactory, DataGateway);
}

/** Not implemented. Throws prg_error. */
AbstractTemporalClusterData * MultiSetNormalClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData&, const DataSetInterface&) const {
  throw prg_error("GetNewProspectiveSpatialClusterData(const CSaTScanData&, const DataSetInterface&) not implemented.","MultiSetNormalClusterDataFactory");
}

/** Returns newly created MultiSetNormalProspectiveSpatialData object as
    AbstractTemporalClusterData pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * MultiSetNormalClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway) const {
  return new MultiSetNormalProspectiveSpatialData(gClusterDataFactory, Data, DataGateway);
}

/** Not implemented. Throws prg_error. */
AbstractTemporalClusterData * MultiSetNormalClusterDataFactory::GetNewTemporalClusterData(const DataSetInterface&) const {
  throw prg_error("GetNewTemporalClusterData(const DataSetInterface&) not implemented.","MultiSetNormalClusterDataFactory");
}

/** Returns newly created MultiSetNormalTemporalData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * MultiSetNormalClusterDataFactory::GetNewTemporalClusterData(const AbstractDataSetGateway& DataGateway) const {
  return new MultiSetNormalTemporalData(gClusterDataFactory, DataGateway);
}

/** Not implemented. Throws prg_error. */
AbstractTemporalClusterData * MultiSetNormalClusterDataFactory::GetNewSpaceTimeClusterData(const DataSetInterface&) const {
  throw prg_error("GetNewSpaceTimeClusterData(const DataSetInterface&) not implemented.","MultiSetNormalClusterDataFactory");
}

/** Returns newly created MultiSetNormalSpaceTimeData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * MultiSetNormalClusterDataFactory::GetNewSpaceTimeClusterData(const AbstractDataSetGateway& DataGateway) const {
  return new MultiSetNormalSpaceTimeData(gClusterDataFactory, DataGateway);
}


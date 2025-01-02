//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "BatchedClusterDataFactory.h"
#include "BatchedClusterData.h"
#include "MultiSetBatchedClusterData.h"
#include "SSException.h"
#include "SaTScanData.h"

//***************** class BatchedClusterDataFactory ****************************

/** Returns newly created BatchedSpatialData object as AbstractSpatialClusterData pointer. */
AbstractSpatialClusterData* BatchedClusterDataFactory::GetNewSpatialClusterData(const DataSetInterface& Interface) const {
    return new BatchedSpatialData(Interface);
}

/** Returns newly created BatchedSpatialData object as AbstractSpatialClusterData pointer. */
AbstractSpatialClusterData* BatchedClusterDataFactory::GetNewSpatialClusterData(const AbstractDataSetGateway& DataGateway) const {
    return new BatchedSpatialData(DataGateway);
}

/** Returns newly created BatchedProspectiveSpatialData object as AbstractTemporalClusterData pointer. */
AbstractTemporalClusterData* BatchedClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData& DataHub, const DataSetInterface& Interface) const {
    return new BatchedProspectiveSpatialData(DataHub, Interface);
}

/** Returns newly created BatchedProspectiveSpatialData object as AbstractTemporalClusterData pointer. */
AbstractTemporalClusterData* BatchedClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway) const {
    return new BatchedProspectiveSpatialData(Data, DataGateway);
}

/** Returns newly created BatchedTemporalData object as AbstractTemporalClusterData pointer. */
AbstractTemporalClusterData* BatchedClusterDataFactory::GetNewTemporalClusterData(const DataSetInterface& Interface) const {
    return new BatchedTemporalData(Interface);
}

/** Returns newly created BatchedTemporalData object as AbstractTemporalClusterData pointer. */
AbstractTemporalClusterData* BatchedClusterDataFactory::GetNewTemporalClusterData(const AbstractDataSetGateway& DataGateway) const {
    return new BatchedTemporalData(DataGateway);
}

/** Returns newly created BatchedSpaceTimeData object as AbstractTemporalClusterData pointer. */
AbstractTemporalClusterData* BatchedClusterDataFactory::GetNewSpaceTimeClusterData(const DataSetInterface& Interface) const {
    return new BatchedSpaceTimeData(Interface);
}

/** Returns newly created BatchedSpaceTimeData object as AbstractTemporalClusterData pointer. */
AbstractTemporalClusterData* BatchedClusterDataFactory::GetNewSpaceTimeClusterData(const AbstractDataSetGateway& DataGateway) const {
    return new BatchedSpaceTimeData(DataGateway);
}

//***************** class MultiSetBatchedClusterDataFactory *********************


/** Not implemented, throws prg_error. */
AbstractSpatialClusterData* MultiSetBatchedClusterDataFactory::GetNewSpatialClusterData(const DataSetInterface&) const {
    throw prg_error("GetNewSpatialClusterData(const DataSetInterface&) not implemented.", "MultiSetBatchedClusterDataFactory");
}

/** Returns newly created MultiSetBatchedSpatialData object as AbstractSpatialClusterData pointer. */
AbstractSpatialClusterData* MultiSetBatchedClusterDataFactory::GetNewSpatialClusterData(const AbstractDataSetGateway& DataGateway) const {
    return new MultiSetBatchedSpatialData(gClusterDataFactory, DataGateway);
}

/** Not implemented, throws prg_error. */
AbstractTemporalClusterData* MultiSetBatchedClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData&, const DataSetInterface&) const {
    throw prg_error("GetNewProspectiveSpatialClusterData(const CSaTScanData&, const DataSetInterface&) not implemented.", "MultiSetBatchedClusterDataFactory");
}

/** Returns newly created MultiSetBatchedProspectiveSpatialData object as AbstractTemporalClusterData pointer. */
AbstractTemporalClusterData* MultiSetBatchedClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway) const {
    return new MultiSetBatchedProspectiveSpatialData(gClusterDataFactory, Data, DataGateway);
}

/** Not implemented, throws prg_error. */
AbstractTemporalClusterData* MultiSetBatchedClusterDataFactory::GetNewTemporalClusterData(const DataSetInterface&) const {
    throw prg_error("GetNewTemporalClusterData(const DataSetInterface&) not implemented.", "MultiSetBatchedClusterDataFactory");
}

/** Returns newly created MultiSetBatchedTemporalData object as AbstractTemporalClusterData pointer. */
AbstractTemporalClusterData* MultiSetBatchedClusterDataFactory::GetNewTemporalClusterData(const AbstractDataSetGateway& DataGateway) const {
    return new MultiSetBatchedTemporalData(gClusterDataFactory, DataGateway);
}

/** Not implemented, throws prg_error. */
AbstractTemporalClusterData* MultiSetBatchedClusterDataFactory::GetNewSpaceTimeClusterData(const DataSetInterface&) const {
    throw prg_error("GetNewSpaceTimeClusterData(const DataSetInterface&) not implemented.", "MultiSetBatchedClusterDataFactory");
}

/** Returns newly created MultiSetBatchedSpaceTimeData object as AbstractTemporalClusterData pointer. */
AbstractTemporalClusterData* MultiSetBatchedClusterDataFactory::GetNewSpaceTimeClusterData(const AbstractDataSetGateway& DataGateway) const {
    return new MultiSetBatchedSpaceTimeData(gClusterDataFactory, DataGateway);
}

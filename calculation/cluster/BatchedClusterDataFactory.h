//******************************************************************************
#ifndef __BatchedClusterDataFactory_H
#define __BatchedClusterDataFactory_H
//******************************************************************************
#include "AbstractClusterDataFactory.h"

/** Cluster data factory which implements the interface for getting cluster data with the Batched probability model.*/
class BatchedClusterDataFactory : public AbstractClusterDataFactory {

public:
    BatchedClusterDataFactory(const CSaTScanData& Data) : AbstractClusterDataFactory() {}
    virtual ~BatchedClusterDataFactory() {}

    //spatial cluster data
    virtual AbstractSpatialClusterData* GetNewSpatialClusterData(const DataSetInterface& Interface) const;
    virtual AbstractSpatialClusterData* GetNewSpatialClusterData(const AbstractDataSetGateway& DataGateway) const;

    //prospective spatial cluster data
    virtual AbstractTemporalClusterData* GetNewProspectiveSpatialClusterData(const CSaTScanData& Data, const DataSetInterface& Interface) const;
    virtual AbstractTemporalClusterData* GetNewProspectiveSpatialClusterData(const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway) const;

    //temporal cluster data
    virtual AbstractTemporalClusterData* GetNewTemporalClusterData(const DataSetInterface& Interface) const;
    virtual AbstractTemporalClusterData* GetNewTemporalClusterData(const AbstractDataSetGateway& DataGateway) const;

    //space-time cluster data
    virtual AbstractTemporalClusterData* GetNewSpaceTimeClusterData(const DataSetInterface& Interface) const;
    virtual AbstractTemporalClusterData* GetNewSpaceTimeClusterData(const AbstractDataSetGateway& DataGateway) const;
};

/** Cluster data factory which implements the interface for getting cluster data with the Batched probability model
    when using multiple data sets. 
*/
class MultiSetBatchedClusterDataFactory : public AbstractClusterDataFactory {

private:
    BatchedClusterDataFactory gClusterDataFactory;

public:
    MultiSetBatchedClusterDataFactory(const CSaTScanData& Data) : gClusterDataFactory(Data), AbstractClusterDataFactory() {}
    virtual ~MultiSetBatchedClusterDataFactory() {}

    //spatial cluster data
    virtual AbstractSpatialClusterData* GetNewSpatialClusterData(const DataSetInterface& Interface) const;
    virtual AbstractSpatialClusterData* GetNewSpatialClusterData(const AbstractDataSetGateway& DataGateway) const;

    //prospective spatial cluster data
    virtual AbstractTemporalClusterData* GetNewProspectiveSpatialClusterData(const CSaTScanData& Data, const DataSetInterface& Interface) const;
    virtual AbstractTemporalClusterData* GetNewProspectiveSpatialClusterData(const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway) const;

    //temporal cluster data
    virtual AbstractTemporalClusterData* GetNewTemporalClusterData(const DataSetInterface& Interface) const;
    virtual AbstractTemporalClusterData* GetNewTemporalClusterData(const AbstractDataSetGateway& DataGateway) const;

    //space-time cluster data
    virtual AbstractTemporalClusterData* GetNewSpaceTimeClusterData(const DataSetInterface& Interface) const;
    virtual AbstractTemporalClusterData* GetNewSpaceTimeClusterData(const AbstractDataSetGateway& DataGateway) const;
};
//******************************************************************************
#endif

//******************************************************************************
#ifndef __AbstractClusterDataFactory_H
#define __AbstractClusterDataFactory_H
//******************************************************************************
#include "AbstractClusterData.h"

/** Class that defines base interface for cluster factories from which cluster
    objects will request new cluster data objects. */
class AbstractClusterDataFactory {
 public:
   AbstractClusterDataFactory() {}
   virtual ~AbstractClusterDataFactory() {}

   //spatial cluster data
   virtual AbstractSpatialClusterData  * GetNewSpatialClusterData(const DataStreamInterface& Interface, int iRate) const = 0;
   virtual AbstractSpatialClusterData  * GetNewSpatialClusterData(const AbtractDataStreamGateway& DataGateway, int iRate) const = 0;

   //prospective spatial cluster data
   virtual AbstractTemporalClusterData * GetNewProspectiveSpatialClusterData(const CSaTScanData& Data, const DataStreamInterface & Interface) const = 0;
   virtual AbstractTemporalClusterData * GetNewProspectiveSpatialClusterData(const CSaTScanData & Data, const AbtractDataStreamGateway & DataGateway) const = 0;

   //temporal cluster data
   virtual AbstractTemporalClusterData * GetNewTemporalClusterData(const DataStreamInterface & Interface) const = 0;
   virtual AbstractTemporalClusterData * GetNewTemporalClusterData(const AbtractDataStreamGateway & DataGateway) const = 0;

   //space-time cluster data
   virtual AbstractTemporalClusterData * GetNewSpaceTimeClusterData(const DataStreamInterface & Interface) const = 0;
   virtual AbstractTemporalClusterData * GetNewSpaceTimeClusterData(const AbtractDataStreamGateway & DataGateway) const = 0;
};
//******************************************************************************
#endif


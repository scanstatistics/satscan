//******************************************************************************
#ifndef __NormalClusterDataFactory_H
#define __NormalClusterDataFactory_H
//******************************************************************************
#include "AbstractClusterDataFactory.h"

/** Cluster factory class that implements the interface getting cluster data for
    the Normal probability model. This model type requires special behavior due
    to the second measure variable.
    Note: All of the public functions to get new cluster data objects with
          'const DataStreamInterface&' parameter are not implemented and will
          throw an ZdException. This probability model type is required to
          perform simulations through same process as that of real data and
          thus only the functions with 'const AbtractDataStreamGateway&' are of
          us at this time.                                                    */
class NormalClusterDataFactory : public AbstractClusterDataFactory {
 public:
   NormalClusterDataFactory();
   virtual ~NormalClusterDataFactory();

   //spatial cluster data
   virtual AbstractSpatialClusterData  * GetNewSpatialClusterData(const DataStreamInterface& Interface, int iRate) const;
   virtual AbstractSpatialClusterData  * GetNewSpatialClusterData(const AbtractDataStreamGateway& DataGateway, int iRate) const;

   //prospective spatial cluster data
   virtual AbstractTemporalClusterData * GetNewProspectiveSpatialClusterData(const CSaTScanData& Data, const DataStreamInterface& Interface) const;
   virtual AbstractTemporalClusterData * GetNewProspectiveSpatialClusterData(const CSaTScanData& Data, const AbtractDataStreamGateway& DataGateway) const;

   //temporal cluster data
   virtual AbstractTemporalClusterData * GetNewTemporalClusterData(const DataStreamInterface& Interface) const;
   virtual AbstractTemporalClusterData * GetNewTemporalClusterData(const AbtractDataStreamGateway& DataGateway) const;

   //space-time cluster data
   virtual AbstractTemporalClusterData * GetNewSpaceTimeClusterData(const DataStreamInterface& Interface) const;
   virtual AbstractTemporalClusterData * GetNewSpaceTimeClusterData(const AbtractDataStreamGateway& DataGateway) const;
};
//******************************************************************************
#endif

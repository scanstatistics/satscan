//******************************************************************************
#ifndef __CategoricalClusterDataFactory_H
#define __CategoricalClusterDataFactory_H
//******************************************************************************
#include "AbstractClusterDataFactory.h"

/** Cluster factory class that implements interface for getting cluster
    objects which partition data by category:
    CategoricalSpatialData, CategoricalTemporalData,
    CategoricalProspectiveSpatialData, and  CategoricalSpaceTimeData.  */
class CategoricalClusterDataFactory : public AbstractClusterDataFactory {
 public:
   CategoricalClusterDataFactory();
   virtual ~CategoricalClusterDataFactory();

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

/** luster factory class that implements interface for getting cluster
    objects in multiple data sets, where set data is partitioned by category:
    MultiSetCategoricalSpatialData, MultiSetCategoricalTemporalData,
    MultiSetCategoricalProspectiveSpatialData, and  MultiSetCategoricalSpaceTimeData.
    Note: All of the public functions to get new cluster data objects with
          'const DataStreamInterface&' parameter are not implemented and will
          throw an ZdException. Having multiple data streams imposes requirement
          to perform simulations through same process as that of real data and
          thus only the functions with 'const AbtractDataStreamGateway&' are of
          us at this time.  */
class MultiSetsCategoricalClusterDataFactory : public AbstractClusterDataFactory {
 private:
   CategoricalClusterDataFactory        gClusterDataFactory;   /** cluster factory object */

   void                                 Setup(const CParameters & Parameters);

 public:
   MultiSetsCategoricalClusterDataFactory(const CParameters& Parameters);
   virtual ~MultiSetsCategoricalClusterDataFactory();

   //spatial cluster data
   virtual AbstractSpatialClusterData * GetNewSpatialClusterData(const DataStreamInterface& Interface, int iRate) const;
   virtual AbstractSpatialClusterData * GetNewSpatialClusterData(const AbtractDataStreamGateway& DataGateway, int iRate) const;

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

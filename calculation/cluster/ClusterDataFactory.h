//******************************************************************************
#ifndef __ClusterDataFactory_H
#define __ClusterDataFactory_H
//******************************************************************************
#include "AbstractClusterDataFactory.h"

/** Cluster factory class that implements the base class interface for cluster
    objects: SpatialData, TemporalData, ProspectiveSpatialData, and
    SpaceTimeData (currently).  */
class ClusterDataFactory : public AbstractClusterDataFactory {
 public:
   ClusterDataFactory();
   virtual ~ClusterDataFactory();

   //spatial cluster data
   virtual AbstractSpatialClusterData  * GetNewSpatialClusterData(const DataSetInterface& Interface) const;
   virtual AbstractSpatialClusterData  * GetNewSpatialClusterData(const AbstractDataSetGateway& DataGateway) const;

   //svtt cluster data
   virtual AbtractSVTTClusterData      * GetNewSVTTClusterData(const DataSetInterface& Interface) const;
   virtual AbtractSVTTClusterData      * GetNewSVTTClusterData(const AbstractDataSetGateway& DataGateway) const;

   //prospective spatial cluster data
   virtual AbstractTemporalClusterData * GetNewProspectiveSpatialClusterData(const CSaTScanData& Data, const DataSetInterface& Interface) const;
   virtual AbstractTemporalClusterData * GetNewProspectiveSpatialClusterData(const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway) const;

   //temporal cluster data
   virtual AbstractTemporalClusterData * GetNewTemporalClusterData(const DataSetInterface& Interface) const;
   virtual AbstractTemporalClusterData * GetNewTemporalClusterData(const AbstractDataSetGateway& DataGateway) const;

   //space-time cluster data
   virtual AbstractTemporalClusterData * GetNewSpaceTimeClusterData(const DataSetInterface& Interface) const;
   virtual AbstractTemporalClusterData * GetNewSpaceTimeClusterData(const AbstractDataSetGateway& DataGateway) const;
};

/** Cluster factory class that implements the base class interface for data sets
    which contain multiple data sets.
    Note: This class does not provide functionality for the Normal probability
          model. A new multiple dataset class will likely be needed when that
          code modification is requested.
    Note: All of the public functions to get new cluster data objects with
          'const DataSetInterface&' parameter are not implemented and will
          throw an prg_error. Having multiple datasets imposes requirement
          to perform simulations through same process as that of real data and
          thus only the functions with 'const AbstractDataSetGateway&' are of
          us at this time.  */
class MultiSetClusterDataFactory : public AbstractClusterDataFactory {
 private:
   ClusterDataFactory                   gClusterDataFactory;   /** cluster factory object */

   void                                 Setup(const CParameters& Parameters);

 public:
   MultiSetClusterDataFactory(const CParameters& Parameters);
   virtual ~MultiSetClusterDataFactory();

   //spatial cluster data
   virtual AbstractSpatialClusterData * GetNewSpatialClusterData(const DataSetInterface& Interface) const;
   virtual AbstractSpatialClusterData * GetNewSpatialClusterData(const AbstractDataSetGateway& DataGateway) const;

   //svtt cluster data
   virtual AbtractSVTTClusterData      * GetNewSVTTClusterData(const DataSetInterface& Interface) const;
   virtual AbtractSVTTClusterData      * GetNewSVTTClusterData(const AbstractDataSetGateway& DataGateway) const;

   //prospective spatial cluster data
   virtual AbstractTemporalClusterData * GetNewProspectiveSpatialClusterData(const CSaTScanData& Data, const DataSetInterface& Interface) const;
   virtual AbstractTemporalClusterData * GetNewProspectiveSpatialClusterData(const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway) const;

   //temporal cluster data
   virtual AbstractTemporalClusterData * GetNewTemporalClusterData(const DataSetInterface& Interface) const;
   virtual AbstractTemporalClusterData * GetNewTemporalClusterData(const AbstractDataSetGateway& DataGateway) const;

   //space-time cluster data
   virtual AbstractTemporalClusterData * GetNewSpaceTimeClusterData(const DataSetInterface& Interface) const;
   virtual AbstractTemporalClusterData * GetNewSpaceTimeClusterData(const AbstractDataSetGateway& DataGateway) const;
};
//******************************************************************************
#endif

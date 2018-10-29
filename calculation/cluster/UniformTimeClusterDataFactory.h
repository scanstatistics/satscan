//******************************************************************************
#ifndef __UniformTimeClusterDataFactory_H
#define __UniformTimeClusterDataFactory_H
//******************************************************************************
#include "AbstractClusterDataFactory.h"

/** Cluster factory class that implements the interface getting cluster data for
    the uniform time probability model. This model type requires special behavior due
    to the second measure variable.
    Note: All of the public functions to get new cluster data objects with
          'const DataSetInterface&' parameter are not implemented and will
          throw an prg_error. This probability model type is required to
          perform simulations through same process as that of real data and
          thus only the functions with 'const AbstractDataSetGateway&' are of
          us at this time.                                                    */
class UniformTimeClusterDataFactory : public AbstractClusterDataFactory {

 public:
   UniformTimeClusterDataFactory();
   virtual ~UniformTimeClusterDataFactory();

   //spatial cluster data
   virtual AbstractSpatialClusterData  * GetNewSpatialClusterData(const DataSetInterface& Interface) const;
   virtual AbstractSpatialClusterData  * GetNewSpatialClusterData(const AbstractDataSetGateway& DataGateway) const;

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
    Note: This class does not provide functionality for the uniforn time probability
          model. A new multiple dataset class will likely be needed when that
          code modification is requested.
    Note: All of the public functions to get new cluster data objects with
          'const DataSetInterface&' parameter are not implemented and will
          throw an prg_error. Having multiple datasets imposes requirement
          to perform simulations through same process as that of real data and
          thus only the functions with 'const AbstractDataSetGateway&' are of
          us at this time.  */
class MultiSetUniformTimeClusterDataFactory : public AbstractClusterDataFactory {
 private:
   UniformTimeClusterDataFactory             gClusterDataFactory;   /** cluster factory object */

 public:
   MultiSetUniformTimeClusterDataFactory();
   virtual ~MultiSetUniformTimeClusterDataFactory();

   //spatial cluster data
   virtual AbstractSpatialClusterData * GetNewSpatialClusterData(const DataSetInterface& Interface) const;
   virtual AbstractSpatialClusterData * GetNewSpatialClusterData(const AbstractDataSetGateway& DataGateway) const;

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

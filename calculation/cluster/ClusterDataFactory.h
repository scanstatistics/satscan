//---------------------------------------------------------------------------
#ifndef ClusterDataFactoryH
#define ClusterDataFactoryH
//---------------------------------------------------------------------------
#include "ClusterData.h"

/** Class that defines base interface for cluster factories from which cluster
    objects will request new cluster data objects. */
class AbstractClusterDataFactory {
 public:
   AbstractClusterDataFactory() {}
   virtual ~AbstractClusterDataFactory() {}

   //spatial cluster data
   virtual AbstractSpatialClusterData  * GetNewSpatialClusterData(const DataStreamInterface & Interface, int iRate) const = 0;
   virtual AbstractSpatialClusterData  * GetNewSpatialClusterData(const AbtractDataStreamGateway & DataGateway, int iRate) const = 0;

   //prospective spatial cluster data
   virtual AbstractTemporalClusterData * GetNewProspectiveSpatialClusterData(const CSaTScanData & Data, const DataStreamInterface & Interface) const = 0;
   virtual AbstractTemporalClusterData * GetNewProspectiveSpatialClusterData(const CSaTScanData & Data, const AbtractDataStreamGateway & DataGateway) const = 0;

   //temporal cluster data
   virtual AbstractTemporalClusterData * GetNewTemporalClusterData(const DataStreamInterface & Interface) const = 0;
   virtual AbstractTemporalClusterData * GetNewTemporalClusterData(const AbtractDataStreamGateway & DataGateway) const = 0;

   //space-time cluster data
   virtual AbstractTemporalClusterData * GetNewSpaceTimeClusterData(const DataStreamInterface & Interface) const = 0;
   virtual AbstractTemporalClusterData * GetNewSpaceTimeClusterData(const AbtractDataStreamGateway & DataGateway) const = 0;
};

/** Cluster factory class that implements the base class interface for cluster
    objects: SpatialData, TemporalData, ProspectiveSpatialData, and
    SpaceTimeData (currently).  */
class ClusterDataFactory : public AbstractClusterDataFactory {
 public:
   ClusterDataFactory();
   virtual ~ClusterDataFactory();

   //spatial cluster data
   virtual AbstractSpatialClusterData  * GetNewSpatialClusterData(const DataStreamInterface & Interface, int iRate) const;
   virtual AbstractSpatialClusterData  * GetNewSpatialClusterData(const AbtractDataStreamGateway & DataGateway, int iRate) const;
   virtual SpatialData                 * GetNewSpatialClusterDataAsSpatialData(const DataStreamInterface & Interface, int iRate) const;
   virtual SpatialData                 * GetNewSpatialClusterDataAsSpatialData(const AbtractDataStreamGateway & DataGateway, int iRate) const;

   //prospective spatial cluster data
   virtual AbstractTemporalClusterData * GetNewProspectiveSpatialClusterData(const CSaTScanData & Data, const DataStreamInterface & Interface) const;
   virtual AbstractTemporalClusterData * GetNewProspectiveSpatialClusterData(const CSaTScanData & Data, const AbtractDataStreamGateway & DataGateway) const;
   virtual TemporalData                * GetNewProspectiveSpatialClusterDataAsTemporalData(const CSaTScanData & Data, const DataStreamInterface & Interface) const;
   virtual TemporalData                * GetNewProspectiveSpatialClusterDataAsTemporalData(const CSaTScanData & Data, const AbtractDataStreamGateway & DataGateway) const;

   //temporal cluster data
   virtual AbstractTemporalClusterData * GetNewTemporalClusterData(const DataStreamInterface & Interface) const;
   virtual AbstractTemporalClusterData * GetNewTemporalClusterData(const AbtractDataStreamGateway & DataGateway) const;
   virtual TemporalData                * GetNewTemporalClusterDataAsTemporalData(const DataStreamInterface & Interface) const;
   virtual TemporalData                * GetNewTemporalClusterDataAsTemporalData(const AbtractDataStreamGateway & DataGateway) const;

   //space-time cluster data
   virtual AbstractTemporalClusterData * GetNewSpaceTimeClusterData(const DataStreamInterface & Interface) const;
   virtual AbstractTemporalClusterData * GetNewSpaceTimeClusterData(const AbtractDataStreamGateway & DataGateway) const;
   virtual TemporalData                * GetNewSpaceTimeClusterDataAsTemporalData(const DataStreamInterface & Interface) const;
   virtual TemporalData                * GetNewSpaceTimeClusterDataAsTemporalData(const AbtractDataStreamGateway & DataGateway) const;
};

/** Cluster factory class that implements the base class interface for the
    Normal probability model. This model type requires special behavior due to
    the second measure variable.
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
   virtual AbstractSpatialClusterData  * GetNewSpatialClusterData(const DataStreamInterface & Interface, int iRate) const;
   virtual AbstractSpatialClusterData  * GetNewSpatialClusterData(const AbtractDataStreamGateway & DataGateway, int iRate) const;

   //prospective spatial cluster data
   virtual AbstractTemporalClusterData * GetNewProspectiveSpatialClusterData(const CSaTScanData & Data, const DataStreamInterface & Interface) const;
   virtual AbstractTemporalClusterData * GetNewProspectiveSpatialClusterData(const CSaTScanData & Data, const AbtractDataStreamGateway & DataGateway) const;

   //temporal cluster data
   virtual AbstractTemporalClusterData * GetNewTemporalClusterData(const DataStreamInterface & Interface) const;
   virtual AbstractTemporalClusterData * GetNewTemporalClusterData(const AbtractDataStreamGateway & DataGateway) const;

   //space-time cluster data
   virtual AbstractTemporalClusterData * GetNewSpaceTimeClusterData(const DataStreamInterface & Interface) const;
   virtual AbstractTemporalClusterData * GetNewSpaceTimeClusterData(const AbtractDataStreamGateway & DataGateway) const;
};

/** Cluster factory class that implements the base class interface for data sets
    which contain multiple data streams.
    Note: This class does not provide functionality for the Normal probability
          model. A new multiple stream class will likely be needed when that
          code modification is requested.
    Note: All of the public functions to get new cluster data objects with
          'const DataStreamInterface&' parameter are not implemented and will
          throw an ZdException. Having multiple data streams imposes requirement
          to perform simulations through same process as that of real data and
          thus only the functions with 'const AbtractDataStreamGateway&' are of
          us at this time.  */
class MultipleStreamsClusterDataFactory : public AbstractClusterDataFactory {
 private:
   ClusterDataFactory                   gStreamClusterFactory;   /** cluster factory object */

   void                                 Setup(const CParameters & Parameters);

 public:
   MultipleStreamsClusterDataFactory(const CParameters& Parameters);
   virtual ~MultipleStreamsClusterDataFactory();

   //spatial cluster data
   virtual AbstractSpatialClusterData * GetNewSpatialClusterData(const DataStreamInterface & Interface, int iRate) const;
   virtual AbstractSpatialClusterData * GetNewSpatialClusterData(const AbtractDataStreamGateway & DataGateway, int iRate) const;

   //prospective spatial cluster data
   virtual AbstractTemporalClusterData * GetNewProspectiveSpatialClusterData(const CSaTScanData & Data, const DataStreamInterface & Interface) const;
   virtual AbstractTemporalClusterData * GetNewProspectiveSpatialClusterData(const CSaTScanData & Data, const AbtractDataStreamGateway & DataGateway) const;

   //temporal cluster data
   virtual AbstractTemporalClusterData * GetNewTemporalClusterData(const DataStreamInterface & Interface) const;
   virtual AbstractTemporalClusterData * GetNewTemporalClusterData(const AbtractDataStreamGateway & DataGateway) const;

   //space-time cluster data
   virtual AbstractTemporalClusterData * GetNewSpaceTimeClusterData(const DataStreamInterface & Interface) const;
   virtual AbstractTemporalClusterData * GetNewSpaceTimeClusterData(const AbtractDataStreamGateway & DataGateway) const;
};
//---------------------------------------------------------------------------
#endif

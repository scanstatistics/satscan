//---------------------------------------------------------------------------
#ifndef ClusterDataFactoryH
#define ClusterDataFactoryH
//---------------------------------------------------------------------------
#include "ClusterData.h"

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

/**  */
class ClusterDataFactory : public AbstractClusterDataFactory {
 public:
   ClusterDataFactory();
   virtual ~ClusterDataFactory();

   //spatial cluster data
   virtual AbstractSpatialClusterData  * GetNewSpatialClusterData(const DataStreamInterface & Interface, int iRate) const;
   virtual AbstractSpatialClusterData  * GetNewSpatialClusterData(const AbtractDataStreamGateway & DataGateway, int iRate) const;

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

/** */
class NormalClusterDataFactory : public ClusterDataFactory {
 public:
   NormalClusterDataFactory();
   virtual ~NormalClusterDataFactory();

   //spatial cluster data
   virtual AbstractSpatialClusterData  * GetNewSpatialClusterData(const DataStreamInterface & Interface, int iRate) const;
   virtual AbstractSpatialClusterData  * GetNewSpatialClusterData(const AbtractDataStreamGateway & DataGateway, int iRate) const;

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

/** */
class MultipleStreamsClusterDataFactory : public AbstractClusterDataFactory {
 private:
   ClusterDataFactory                 * gpStreamClusterFactory;

   void                                 Init() {gpStreamClusterFactory=0;}
   void                                 Setup(const CParameters & Parameters);

 public:
   MultipleStreamsClusterDataFactory(const CParameters & Parameters);
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

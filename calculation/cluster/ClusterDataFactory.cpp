//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "ClusterDataFactory.h"
#include "ClusterData.h"
#include "NormalClusterData.h"
#include "MultipleStreamClusterData.h"

/** class constructor*/
ClusterDataFactory::ClusterDataFactory() : AbstractClusterDataFactory() {}

/** class destructor */
ClusterDataFactory::~ClusterDataFactory() {}

/** Returns newly created SpatialData object as AbstractSpatialClusterData
    pointer. Caller is responsible for object destruction. */
AbstractSpatialClusterData * ClusterDataFactory::GetNewSpatialClusterData(const DataStreamInterface& Interface, int iRate) const {
  return new SpatialData(Interface, iRate);
}

/** Returns newly created SpatialData object as AbstractSpatialClusterData
    pointer. Caller is responsible for object destruction. */
AbstractSpatialClusterData * ClusterDataFactory::GetNewSpatialClusterData(const AbtractDataStreamGateway& DataGateway, int iRate) const {
  return new SpatialData(DataGateway, iRate);
}

/** Returns newly created SpatialData object. Caller is responsible for object
    destruction. */
SpatialData * ClusterDataFactory::GetNewSpatialClusterDataAsSpatialData(const DataStreamInterface & Interface, int iRate) const {
  return new SpatialData(Interface, iRate);
}

/** Returns newly created SpatialData object. Caller is responsible for object
    destruction. */
SpatialData * ClusterDataFactory::GetNewSpatialClusterDataAsSpatialData(const AbtractDataStreamGateway & DataGateway, int iRate) const {
  return new SpatialData(DataGateway, iRate);
}

/** Returns newly created ProspectiveSpatialData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction. */
AbstractTemporalClusterData * ClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData & Data, const DataStreamInterface & Interface) const {
  return new ProspectiveSpatialData(Data, Interface);
}

/** Returns newly created ProspectiveSpatialData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction. */
AbstractTemporalClusterData * ClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData & Data, const AbtractDataStreamGateway& DataGateway) const {
  return new ProspectiveSpatialData(Data, DataGateway);
}

/** Returns newly created ProspectiveSpatialData object as TemporalData pointer.
    Caller is responsible for object destruction.*/
TemporalData * ClusterDataFactory::GetNewProspectiveSpatialClusterDataAsTemporalData(const CSaTScanData & Data, const DataStreamInterface & Interface) const {
  return new ProspectiveSpatialData(Data, Interface);
}

/** Returns newly created ProspectiveSpatialData object as TemporalData pointer.
    Caller is responsible for object destruction.*/
TemporalData * ClusterDataFactory::GetNewProspectiveSpatialClusterDataAsTemporalData(const CSaTScanData & Data, const AbtractDataStreamGateway& DataGateway) const {
  return new ProspectiveSpatialData(Data, DataGateway);
}

/** Returns newly created TemporalData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * ClusterDataFactory::GetNewTemporalClusterData(const DataStreamInterface & Interface) const {
  return new TemporalData(Interface);
}

/** Returns newly created TemporalData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * ClusterDataFactory::GetNewTemporalClusterData(const AbtractDataStreamGateway & DataGateway) const {
  return new TemporalData(DataGateway);
}

/** Returns newly created TemporalData object. Caller is responsible for object
    destruction.*/
TemporalData * ClusterDataFactory::GetNewTemporalClusterDataAsTemporalData(const DataStreamInterface & Interface) const {
  return new TemporalData(Interface);
}

/** Returns newly created TemporalData object. Caller is responsible for object
    destruction.*/
TemporalData * ClusterDataFactory::GetNewTemporalClusterDataAsTemporalData(const AbtractDataStreamGateway & DataGateway) const {
  return new TemporalData(DataGateway);
}

/** Returns newly created SpaceTimeData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * ClusterDataFactory::GetNewSpaceTimeClusterData(const DataStreamInterface & Interface) const {
  return new SpaceTimeData(Interface);
}

/** Returns newly created SpaceTimeData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * ClusterDataFactory::GetNewSpaceTimeClusterData(const AbtractDataStreamGateway& DataGateway) const {
  return new SpaceTimeData(DataGateway);
}

/** Returns newly created SpaceTimeData object as TemporalData pointer. Caller
    is responsible for object destruction.*/
TemporalData * ClusterDataFactory::GetNewSpaceTimeClusterDataAsTemporalData(const DataStreamInterface & Interface) const {
  return new SpaceTimeData(Interface);
}

/** Returns newly created SpaceTimeData object as TemporalData pointer. Caller
    is responsible for object destruction.*/
TemporalData * ClusterDataFactory::GetNewSpaceTimeClusterDataAsTemporalData(const AbtractDataStreamGateway& DataGateway) const {
  return new SpaceTimeData(DataGateway);
}
 

/** class constructor */
NormalClusterDataFactory::NormalClusterDataFactory() : AbstractClusterDataFactory() {}

/** class destructor */
NormalClusterDataFactory::~NormalClusterDataFactory() {}

/** Not implemented. Throws ZdException. */
AbstractSpatialClusterData * NormalClusterDataFactory::GetNewSpatialClusterData(const DataStreamInterface&, int) const {
  ZdGenerateException("GetNewSpatialClusterData(const DataStreamInterface&, int) not implemented.","NormalClusterDataFactory");
  return 0;
}

/** Returns newly created NormalSpatialData object as AbstractSpatialClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractSpatialClusterData * NormalClusterDataFactory::GetNewSpatialClusterData(const AbtractDataStreamGateway & DataGateway, int iRate) const {
  return new NormalSpatialData(DataGateway, iRate);
}

/** Not implemented. Throws ZdException. */
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData&, const DataStreamInterface&) const {
  ZdGenerateException("GetNewProspectiveSpatialClusterData(const DataStreamInterface&, int) not implemented.","NormalClusterDataFactory");
  return 0;
//  return new NormalProspectiveSpatialData(Data, Interface); ???
}

/** Returns newly created NormalProspectiveSpatialData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData & Data, const AbtractDataStreamGateway& DataGateway) const {
  return new NormalProspectiveSpatialData(Data, DataGateway);
}

/** Not implemented. Throws ZdException. */
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewTemporalClusterData(const DataStreamInterface&) const {
  ZdGenerateException("GetNewTemporalClusterData(const DataStreamInterface&) not implemented.","NormalClusterDataFactory");
  return 0;
}

/** Returns newly created NormalTemporalData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewTemporalClusterData(const AbtractDataStreamGateway & DataGateway) const {
  return new NormalTemporalData(DataGateway);
}

/** Not implemented. Throws ZdException. */
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewSpaceTimeClusterData(const DataStreamInterface&) const {
  ZdGenerateException("GetNewSpaceTimeClusterData(const DataStreamInterface&) not implemented.","NormalClusterDataFactory");
  return 0;
}

/** Returns newly created NormalSpaceTimeData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewSpaceTimeClusterData(const AbtractDataStreamGateway&DataGateway) const {
  return new NormalSpaceTimeData(DataGateway);
}


/** class constructor */
MultipleStreamsClusterDataFactory::MultipleStreamsClusterDataFactory(const CParameters & Parameters) : AbstractClusterDataFactory() {
  try {
    Setup(Parameters);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","MultipleStreamsClusterDataFactory");
    throw;
  }
}

/** class destructor */
MultipleStreamsClusterDataFactory::~MultipleStreamsClusterDataFactory() {}

/** Not implemented. Throws ZdException. */
AbstractSpatialClusterData * MultipleStreamsClusterDataFactory::GetNewSpatialClusterData(const DataStreamInterface&, int) const {
  ZdGenerateException("GetNewSpatialClusterData(const DataStreamInterface&, int) not implemented.","MultipleStreamsClusterDataFactory");
  return 0;
}

/** Returns newly created MultipleStreamSpatialData object as AbstractSpatialClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractSpatialClusterData * MultipleStreamsClusterDataFactory::GetNewSpatialClusterData(const AbtractDataStreamGateway & DataGateway, int iRate) const {
  return new MultipleStreamSpatialData(gStreamClusterFactory, DataGateway, iRate);
}

/** Not implemented. Throws ZdException. */
AbstractTemporalClusterData * MultipleStreamsClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData&, const DataStreamInterface&) const {
  ZdGenerateException("GetNewProspectiveSpatialClusterData(const CSaTScanData&, const DataStreamInterface&) not implemented.","MultipleStreamsClusterDataFactory");
  return 0;
}

/** Returns newly created MultipleStreamProspectiveSpatialData object as
    AbstractTemporalClusterData pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * MultipleStreamsClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData & Data, const AbtractDataStreamGateway& DataGateway) const {
  return new MultipleStreamProspectiveSpatialData(gStreamClusterFactory, Data, DataGateway);
}

/** Not implemented. Throws ZdException. */
AbstractTemporalClusterData * MultipleStreamsClusterDataFactory::GetNewTemporalClusterData(const DataStreamInterface&) const {
  ZdGenerateException("GetNewTemporalClusterData(const DataStreamInterface&) not implemented.","MultipleStreamsClusterDataFactory");
  return 0;
}

/** Returns newly created MultipleStreamTemporalData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * MultipleStreamsClusterDataFactory::GetNewTemporalClusterData(const AbtractDataStreamGateway & DataGateway) const {
  return new MultipleStreamTemporalData(gStreamClusterFactory, DataGateway);
}

/** Not implemented. Throws ZdException. */
AbstractTemporalClusterData * MultipleStreamsClusterDataFactory::GetNewSpaceTimeClusterData(const DataStreamInterface&) const {
  ZdGenerateException("GetNewSpaceTimeClusterData(const DataStreamInterface&) not implemented.","MultipleStreamsClusterDataFactory");
  return 0;
}

/** Returns newly created MultipleStreamSpaceTimeData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * MultipleStreamsClusterDataFactory::GetNewSpaceTimeClusterData(const AbtractDataStreamGateway& DataGateway) const {
  return new MultipleStreamSpaceTimeData(gStreamClusterFactory, DataGateway);
}

/** Internal class setup function. */
void MultipleStreamsClusterDataFactory::Setup(const CParameters& Parameters) {
  try {
    if (Parameters.GetProbabiltyModelType() == NORMAL)
      ZdGenerateException("This class does not implement a multiple stream factory\n"
                          "interface for the Normal probability model.\n","Setup()");
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","MultipleStreamsClusterDataFactory");
    throw;
  }
}

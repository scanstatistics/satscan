//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "ClusterDataFactory.h"
#include "ClusterData.h"
#include "NormalClusterData.h"
#include "MultipleStreamClusterData.h"

/** constructor*/
ClusterDataFactory::ClusterDataFactory() : AbstractClusterDataFactory() {}

/** destructor */
ClusterDataFactory::~ClusterDataFactory() {}

/** returns newly created SpatialData */
AbstractSpatialClusterData * ClusterDataFactory::GetNewSpatialClusterData(const DataStreamInterface & Interface, int iRate) const {
  return new SpatialData(Interface, iRate);
}

/** returns newly created SpatialData */
AbstractSpatialClusterData * ClusterDataFactory::GetNewSpatialClusterData(const AbtractDataStreamGateway & DataGateway, int iRate) const {
  return new SpatialData(DataGateway, iRate);
}

/** returns newly created ProspectiveSpatialData */
AbstractTemporalClusterData * ClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData & Data, const DataStreamInterface & Interface) const {
  return new ProspectiveSpatialData(Data, Interface);
}

/** returns newly created ProspectiveSpatialData */
AbstractTemporalClusterData * ClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData & Data, const AbtractDataStreamGateway& DataGateway) const {
  return new ProspectiveSpatialData(Data, DataGateway);
}

/** returns newly created ProspectiveSpatialData */
TemporalData * ClusterDataFactory::GetNewProspectiveSpatialClusterDataAsTemporalData(const CSaTScanData & Data, const DataStreamInterface & Interface) const {
  return new ProspectiveSpatialData(Data, Interface);
}

/** returns newly created ProspectiveSpatialData */
TemporalData * ClusterDataFactory::GetNewProspectiveSpatialClusterDataAsTemporalData(const CSaTScanData & Data, const AbtractDataStreamGateway& DataGateway) const {
  return new ProspectiveSpatialData(Data, DataGateway);
}

/** returns newly created TemporalData */
AbstractTemporalClusterData * ClusterDataFactory::GetNewTemporalClusterData(const DataStreamInterface & Interface) const {
  return new TemporalData(Interface);
}

/** returns newly created TemporalData */
AbstractTemporalClusterData * ClusterDataFactory::GetNewTemporalClusterData(const AbtractDataStreamGateway & DataGateway) const {
  return new TemporalData(DataGateway);
}

/** returns newly created TemporalData */
TemporalData * ClusterDataFactory::GetNewTemporalClusterDataAsTemporalData(const DataStreamInterface & Interface) const {
  return new TemporalData(Interface);
}

/** returns newly created TemporalData */
TemporalData * ClusterDataFactory::GetNewTemporalClusterDataAsTemporalData(const AbtractDataStreamGateway & DataGateway) const {
  return new TemporalData(DataGateway);
}

/** returns newly created TemporalData */
AbstractTemporalClusterData * ClusterDataFactory::GetNewSpaceTimeClusterData(const DataStreamInterface & Interface) const {
  return new SpaceTimeData(Interface);
}

/** returns newly created TemporalData */
AbstractTemporalClusterData * ClusterDataFactory::GetNewSpaceTimeClusterData(const AbtractDataStreamGateway& DataGateway) const {
  return new SpaceTimeData(DataGateway);
}

/** returns newly created TemporalData */
TemporalData * ClusterDataFactory::GetNewSpaceTimeClusterDataAsTemporalData(const DataStreamInterface & Interface) const {
  return new SpaceTimeData(Interface);
}

/** returns newly created TemporalData */
TemporalData * ClusterDataFactory::GetNewSpaceTimeClusterDataAsTemporalData(const AbtractDataStreamGateway& DataGateway) const {
  return new SpaceTimeData(DataGateway);
}
 



/** constructor */
NormalClusterDataFactory::NormalClusterDataFactory() : ClusterDataFactory() {}

/** destructor */
NormalClusterDataFactory::~NormalClusterDataFactory() {}

/** returns newly created SpatialData */
AbstractSpatialClusterData * NormalClusterDataFactory::GetNewSpatialClusterData(const DataStreamInterface & Interface, int iRate) const {
  ZdGenerateException("GetNewSpatialClusterData(const DataStreamInterface&, int) not implemented.","NormalClusterDataFactory");
  return 0;
}

/** returns newly created SpatialData */
AbstractSpatialClusterData * NormalClusterDataFactory::GetNewSpatialClusterData(const AbtractDataStreamGateway & DataGateway, int iRate) const {
  return new NormalSpatialData(DataGateway, iRate);
}

/** returns newly created NormalProspectiveSpatialData */
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData & Data, const DataStreamInterface & Interface) const {
  return new NormalProspectiveSpatialData(Data, Interface);
}

/** returns newly created NormalProspectiveSpatialData */
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData & Data, const AbtractDataStreamGateway& DataGateway) const {
  return new NormalProspectiveSpatialData(Data, DataGateway);
}

/** returns newly created NormalProspectiveSpatialData */
TemporalData * NormalClusterDataFactory::GetNewProspectiveSpatialClusterDataAsTemporalData(const CSaTScanData & Data, const DataStreamInterface & Interface) const {
  return new NormalProspectiveSpatialData(Data, Interface);
}

/** returns newly created NormalProspectiveSpatialData */
TemporalData * NormalClusterDataFactory::GetNewProspectiveSpatialClusterDataAsTemporalData(const CSaTScanData & Data, const AbtractDataStreamGateway& DataGateway) const {
  return new NormalProspectiveSpatialData(Data, DataGateway);
}

/** not implemented - throw exception */
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewTemporalClusterData(const DataStreamInterface & Interface) const {
  ZdGenerateException("GetNewTemporalClusterData(const DataStreamInterface&) not implemented.","NormalClusterDataFactory");
  return 0;
}

/** returns newly created NormalTemporalData */
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewTemporalClusterData(const AbtractDataStreamGateway & DataGateway) const {
  return new NormalTemporalData(DataGateway);
}

/** not implemented - throw exception */
TemporalData * NormalClusterDataFactory::GetNewTemporalClusterDataAsTemporalData(const DataStreamInterface & Interface) const {
  ZdGenerateException("GetNewTemporalClusterData(const DataStreamInterface&) not implemented.","NormalClusterDataFactory");
  return 0;
}

/** returns newly created NormalTemporalData */
TemporalData * NormalClusterDataFactory::GetNewTemporalClusterDataAsTemporalData(const AbtractDataStreamGateway & DataGateway) const {
  return new NormalTemporalData(DataGateway);
}

/** not implemented - throw exception */
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewSpaceTimeClusterData(const DataStreamInterface&) const {
  ZdGenerateException("GetNewSpaceTimeClusterData(const DataStreamInterface&) not implemented.","NormalClusterDataFactory");
  return 0;
}

/** returns newly created NormalSpaceTimeData */
AbstractTemporalClusterData * NormalClusterDataFactory::GetNewSpaceTimeClusterData(const AbtractDataStreamGateway&DataGateway) const {
  return new NormalSpaceTimeData(DataGateway);
}

/** not implemented - throw exception */
TemporalData * NormalClusterDataFactory::GetNewSpaceTimeClusterDataAsTemporalData(const DataStreamInterface&) const {
  ZdGenerateException("GetNewSpaceTimeClusterDataAsTemporalData(const DataStreamInterface&) not implemented.","NormalClusterDataFactory");
  return 0;
}

/** returns newly created NormalSpaceTimeData */
TemporalData * NormalClusterDataFactory::GetNewSpaceTimeClusterDataAsTemporalData(const AbtractDataStreamGateway&DataGateway) const {
  return new NormalSpaceTimeData(DataGateway);
}


/** constructor */
MultipleStreamsClusterDataFactory::MultipleStreamsClusterDataFactory(const CParameters & Parameters) : AbstractClusterDataFactory() {
  try {
    Init();
    Setup(Parameters);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","MultipleStreamsClusterDataFactory");
    throw;
  }
}

/** destructor */
MultipleStreamsClusterDataFactory::~MultipleStreamsClusterDataFactory() {
  try {
    delete gpStreamClusterFactory;
  }
  catch (...){}  
}

/** no implemented - throw exception */
AbstractSpatialClusterData * MultipleStreamsClusterDataFactory::GetNewSpatialClusterData(const DataStreamInterface & Interface, int iRate) const {
  ZdGenerateException("GetNewSpatialClusterData(const DataStreamInterface&, int) not implemented.","MultipleStreamsClusterDataFactory");
  return 0;
}

/** returns newly created MultipleStreamSpatialData */
AbstractSpatialClusterData * MultipleStreamsClusterDataFactory::GetNewSpatialClusterData(const AbtractDataStreamGateway & DataGateway, int iRate) const {
  return new MultipleStreamSpatialData(*gpStreamClusterFactory, DataGateway, iRate);
}

/** no implemented - throw exception */
AbstractTemporalClusterData * MultipleStreamsClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData & Data, const DataStreamInterface & Interface) const {
  ZdGenerateException("GetNewProspectiveSpatialClusterData(const CSaTScanData&) not implemented.","MultipleStreamsClusterDataFactory");
  return 0;
}

/** returns newly created MultipleStreamProspectiveSpatialData */
AbstractTemporalClusterData * MultipleStreamsClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData & Data, const AbtractDataStreamGateway& DataGateway) const {
  return new MultipleStreamProspectiveSpatialData(*gpStreamClusterFactory, Data, DataGateway);
}

/** no implemented - throw exception */
AbstractTemporalClusterData * MultipleStreamsClusterDataFactory::GetNewTemporalClusterData(const DataStreamInterface & Interface) const {
  ZdGenerateException("GetNewTemporalClusterData(const DataStreamInterface&) not implemented.","MultipleStreamsClusterDataFactory");
  return 0;
}

/** returns newly created MultipleStreamTemporalData */
AbstractTemporalClusterData * MultipleStreamsClusterDataFactory::GetNewTemporalClusterData(const AbtractDataStreamGateway & DataGateway) const {
  return new MultipleStreamTemporalData(*gpStreamClusterFactory, DataGateway);
}

/** no implemented - throw exception */
AbstractTemporalClusterData * MultipleStreamsClusterDataFactory::GetNewSpaceTimeClusterData(const DataStreamInterface&) const {
  ZdGenerateException("GetNewSpaceTimeClusterData(unsigned int) not implemented.","MultipleStreamsClusterDataFactory");
  return 0;
}

/** returns newly created MultipleStreamSpaceTimeData */
AbstractTemporalClusterData * MultipleStreamsClusterDataFactory::GetNewSpaceTimeClusterData(const AbtractDataStreamGateway& DataGateway) const {
  return new MultipleStreamSpaceTimeData(*gpStreamClusterFactory ,DataGateway);
}

void MultipleStreamsClusterDataFactory::Setup(const CParameters & Parameters) {
  try {
    if (Parameters.GetProbabiltyModelType() == NORMAL)
      gpStreamClusterFactory = new NormalClusterDataFactory();
    else
      gpStreamClusterFactory = new ClusterDataFactory();
  }
  catch (ZdException &x) {
    delete gpStreamClusterFactory;
    x.AddCallpath("Setup()","MultipleStreamsClusterDataFactory");
    throw;
  }
}

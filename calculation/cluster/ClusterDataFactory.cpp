//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ClusterDataFactory.h"
#include "ClusterData.h"
#include "MultipleStreamClusterData.h"
#include "CategoricalClusterData.h"

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

/** Returns newly created ProspectiveSpatialData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction. */
AbstractTemporalClusterData * ClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData& Data, const DataStreamInterface& Interface) const {
  return new ProspectiveSpatialData(Data, Interface);
}

/** Returns newly created ProspectiveSpatialData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction. */
AbstractTemporalClusterData * ClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData& Data, const AbtractDataStreamGateway& DataGateway) const {
  return new ProspectiveSpatialData(Data, DataGateway);
}

/** Returns newly created TemporalData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * ClusterDataFactory::GetNewTemporalClusterData(const DataStreamInterface& Interface) const {
  return new TemporalData(Interface);
}

/** Returns newly created TemporalData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * ClusterDataFactory::GetNewTemporalClusterData(const AbtractDataStreamGateway& DataGateway) const {
  return new TemporalData(DataGateway);
}

/** Returns newly created SpaceTimeData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * ClusterDataFactory::GetNewSpaceTimeClusterData(const DataStreamInterface& Interface) const {
  return new SpaceTimeData(Interface);
}

/** Returns newly created SpaceTimeData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * ClusterDataFactory::GetNewSpaceTimeClusterData(const AbtractDataStreamGateway& DataGateway) const {
  return new SpaceTimeData(DataGateway);
}

//******************************************************************************

/** class constructor */
MultiSetClusterDataFactory::MultiSetClusterDataFactory(const CParameters& Parameters) : AbstractClusterDataFactory() {
  try {
    Setup(Parameters);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","MultiSetClusterDataFactory");
    throw;
  }
}

/** class destructor */
MultiSetClusterDataFactory::~MultiSetClusterDataFactory() {}

/** Not implemented. Throws ZdException. */
AbstractSpatialClusterData * MultiSetClusterDataFactory::GetNewSpatialClusterData(const DataStreamInterface&, int) const {
  ZdGenerateException("GetNewSpatialClusterData(const DataStreamInterface&, int) not implemented.","MultiSetClusterDataFactory");
  return 0;
}

/** Returns newly created MultiSetSpatialData object as AbstractSpatialClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractSpatialClusterData * MultiSetClusterDataFactory::GetNewSpatialClusterData(const AbtractDataStreamGateway& DataGateway, int iRate) const {
  return new MultiSetSpatialData(gClusterDataFactory, DataGateway, iRate);
}

/** Not implemented. Throws ZdException. */
AbstractTemporalClusterData * MultiSetClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData&, const DataStreamInterface&) const {
  ZdGenerateException("GetNewProspectiveSpatialClusterData(const CSaTScanData&, const DataStreamInterface&) not implemented.","MultiSetClusterDataFactory");
  return 0;
}

/** Returns newly created MultipleStreamProspectiveSpatialData object as
    AbstractTemporalClusterData pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * MultiSetClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData& Data, const AbtractDataStreamGateway& DataGateway) const {
  return new MultiSetProspectiveSpatialData(gClusterDataFactory, Data, DataGateway);
}

/** Not implemented. Throws ZdException. */
AbstractTemporalClusterData * MultiSetClusterDataFactory::GetNewTemporalClusterData(const DataStreamInterface&) const {
  ZdGenerateException("GetNewTemporalClusterData(const DataStreamInterface&) not implemented.","MultiSetClusterDataFactory");
  return 0;
}

/** Returns newly created MultipleStreamTemporalData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * MultiSetClusterDataFactory::GetNewTemporalClusterData(const AbtractDataStreamGateway& DataGateway) const {
  return new MultiSetTemporalData(gClusterDataFactory, DataGateway);
}

/** Not implemented. Throws ZdException. */
AbstractTemporalClusterData * MultiSetClusterDataFactory::GetNewSpaceTimeClusterData(const DataStreamInterface&) const {
  ZdGenerateException("GetNewSpaceTimeClusterData(const DataStreamInterface&) not implemented.","MultiSetClusterDataFactory");
  return 0;
}

/** Returns newly created MultipleStreamSpaceTimeData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * MultiSetClusterDataFactory::GetNewSpaceTimeClusterData(const AbtractDataStreamGateway& DataGateway) const {
  return new MultiSetSpaceTimeData(gClusterDataFactory, DataGateway);
}

/** Internal class setup function. */
void MultiSetClusterDataFactory::Setup(const CParameters& Parameters) {
  try {
    if (Parameters.GetProbabilityModelType() == NORMAL)
      ZdGenerateException("This class does not implement a multiple stream factory\n"
                          "interface for the Normal probability model.\n","Setup()");
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","MultiSetClusterDataFactory");
    throw;
  }
}


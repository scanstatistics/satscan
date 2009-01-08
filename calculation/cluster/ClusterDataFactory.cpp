//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ClusterDataFactory.h"
#include "ClusterData.h"
#include "MultiSetClusterData.h"
#include "CategoricalClusterData.h"
#include "SVTTCluster.h"
#include "SSException.h"

/** class constructor*/
ClusterDataFactory::ClusterDataFactory() : AbstractClusterDataFactory() {}

/** class destructor */
ClusterDataFactory::~ClusterDataFactory() {}

/** Returns newly created SpatialData object as AbstractSpatialClusterData
    pointer. Caller is responsible for object destruction. */
AbstractSpatialClusterData * ClusterDataFactory::GetNewSpatialClusterData(const DataSetInterface& Interface) const {
  return new SpatialData(Interface);
}

/** Returns newly created SpatialData object as AbstractSpatialClusterData
    pointer. Caller is responsible for object destruction. */
AbstractSpatialClusterData * ClusterDataFactory::GetNewSpatialClusterData(const AbstractDataSetGateway& DataGateway) const {
  return new SpatialData(DataGateway);
}

/** Returns newly created SVTTClusterData object as AbtractSVTTClusterData
    pointer. Caller is responsible for object destruction. */
AbtractSVTTClusterData * ClusterDataFactory::GetNewSVTTClusterData(const DataSetInterface& Interface) const {
  return new SVTTClusterData(Interface);
}

/** Returns newly created SVTTClusterData object as AbtractSVTTClusterData
    pointer. Caller is responsible for object destruction. */
AbtractSVTTClusterData * ClusterDataFactory::GetNewSVTTClusterData(const AbstractDataSetGateway& DataGateway) const {
  return new SVTTClusterData(DataGateway);
}

/** Returns newly created ProspectiveSpatialData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction. */
AbstractTemporalClusterData * ClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData& Data, const DataSetInterface& Interface) const {
  return new ProspectiveSpatialData(Data, Interface);
}

/** Returns newly created ProspectiveSpatialData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction. */
AbstractTemporalClusterData * ClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway) const {
  return new ProspectiveSpatialData(Data, DataGateway);
}

/** Returns newly created TemporalData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * ClusterDataFactory::GetNewTemporalClusterData(const DataSetInterface& Interface) const {
  return new TemporalData(Interface);
}

/** Returns newly created TemporalData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * ClusterDataFactory::GetNewTemporalClusterData(const AbstractDataSetGateway& DataGateway) const {
  return new TemporalData(DataGateway);
}

/** Returns newly created SpaceTimeData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * ClusterDataFactory::GetNewSpaceTimeClusterData(const DataSetInterface& Interface) const {
  return new SpaceTimeData(Interface);
}

/** Returns newly created SpaceTimeData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * ClusterDataFactory::GetNewSpaceTimeClusterData(const AbstractDataSetGateway& DataGateway) const {
  return new SpaceTimeData(DataGateway);
}

//******************************************************************************

/** class constructor */
MultiSetClusterDataFactory::MultiSetClusterDataFactory(const CParameters& Parameters) : AbstractClusterDataFactory() {
  try {
    Setup(Parameters);
  }
  catch (prg_exception& x) {
    x.addTrace("constructor()","MultiSetClusterDataFactory");
    throw;
  }
}

/** class destructor */
MultiSetClusterDataFactory::~MultiSetClusterDataFactory() {}

/** Not implemented. Throws prg_error. */
AbstractSpatialClusterData * MultiSetClusterDataFactory::GetNewSpatialClusterData(const DataSetInterface&) const {
  throw prg_error("GetNewSpatialClusterData(const DataSetInterface&) not implemented.","MultiSetClusterDataFactory");
}

/** Returns newly created MultiSetSpatialData object as AbstractSpatialClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractSpatialClusterData * MultiSetClusterDataFactory::GetNewSpatialClusterData(const AbstractDataSetGateway& DataGateway) const {
  return new MultiSetSpatialData(gClusterDataFactory, DataGateway);
}

/** Returns newly created SVTTClusterData object as AbtractSVTTClusterData
    pointer. Caller is responsible for object destruction. */
AbtractSVTTClusterData * MultiSetClusterDataFactory::GetNewSVTTClusterData(const DataSetInterface& Interface) const {
  throw prg_error("GetNewSVTTClusterData(const DataSetInterface&) not implemented.","MultiSetClusterDataFactory");
}

/** Returns newly created SVTTClusterData object as AbtractSVTTClusterData
    pointer. Caller is responsible for object destruction. */
AbtractSVTTClusterData * MultiSetClusterDataFactory::GetNewSVTTClusterData(const AbstractDataSetGateway& DataGateway) const {
  return new MultiSetSVTTClusterData(DataGateway);
}

/** Not implemented. Throws prg_error. */
AbstractTemporalClusterData * MultiSetClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData&, const DataSetInterface&) const {
  throw prg_error("GetNewProspectiveSpatialClusterData(const CSaTScanData&, const DataSetInterface&) not implemented.","MultiSetClusterDataFactory");
}

/** Returns newly created MultiSetProspectiveSpatialData object as
    AbstractTemporalClusterData pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * MultiSetClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway) const {
  return new MultiSetProspectiveSpatialData(gClusterDataFactory, Data, DataGateway);
}

/** Not implemented. Throws prg_error. */
AbstractTemporalClusterData * MultiSetClusterDataFactory::GetNewTemporalClusterData(const DataSetInterface&) const {
  throw prg_error("GetNewTemporalClusterData(const DataSetInterface&) not implemented.","MultiSetClusterDataFactory");
}

/** Returns newly created MultiSetTemporalData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * MultiSetClusterDataFactory::GetNewTemporalClusterData(const AbstractDataSetGateway& DataGateway) const {
  return new MultiSetTemporalData(gClusterDataFactory, DataGateway);
}

/** Not implemented. Throws prg_error. */
AbstractTemporalClusterData * MultiSetClusterDataFactory::GetNewSpaceTimeClusterData(const DataSetInterface&) const {
  throw prg_error("GetNewSpaceTimeClusterData(const DataSetInterface&) not implemented.","MultiSetClusterDataFactory");
}

/** Returns newly created MultiSetSpaceTimeData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * MultiSetClusterDataFactory::GetNewSpaceTimeClusterData(const AbstractDataSetGateway& DataGateway) const {
  return new MultiSetSpaceTimeData(gClusterDataFactory, DataGateway);
}

/** Internal class setup function. */
void MultiSetClusterDataFactory::Setup(const CParameters& Parameters) {
  try {
    if (Parameters.GetProbabilityModelType() == NORMAL)
      throw prg_error("This class does not implement a multiple dataset factory\n"
                          "interface for the Normal probability model.\n","Setup()");
  }
  catch (prg_exception& x) {
    x.addTrace("Setup()","MultiSetClusterDataFactory");
    throw;
  }
}


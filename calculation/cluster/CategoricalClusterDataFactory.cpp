//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "CategoricalClusterDataFactory.h"
#include "MultiSetCategoricalClusterData.h"
#include "SSException.h"

/** class constructor*/
CategoricalClusterDataFactory::CategoricalClusterDataFactory() : AbstractClusterDataFactory() {}

/** class destructor */
CategoricalClusterDataFactory::~CategoricalClusterDataFactory() {}

/** Returns newly created CategoricalSpatialData object as AbstractSpatialClusterData
    pointer. Caller is responsible for object destruction. */
AbstractSpatialClusterData * CategoricalClusterDataFactory::GetNewSpatialClusterData(const DataSetInterface& Interface) const {
  return new CategoricalSpatialData(Interface);
}

/** Returns newly created CategoricalSpatialData object as AbstractSpatialClusterData
    pointer. Caller is responsible for object destruction. */
AbstractSpatialClusterData * CategoricalClusterDataFactory::GetNewSpatialClusterData(const AbstractDataSetGateway& DataGateway) const {
  return new CategoricalSpatialData(DataGateway);
}

/** Returns newly created CategoricalProspectiveSpatialData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction. */
AbstractTemporalClusterData * CategoricalClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData& Data, const DataSetInterface & Interface) const {
  return new CategoricalProspectiveSpatialData(Data, Interface);
}

/** Returns newly created CategoricalProspectiveSpatialData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction. */
AbstractTemporalClusterData * CategoricalClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway) const {
  return new CategoricalProspectiveSpatialData(Data, DataGateway);
}

/** Returns newly created CategoricalTemporalData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * CategoricalClusterDataFactory::GetNewTemporalClusterData(const DataSetInterface& Interface) const {
  return new CategoricalTemporalData(Interface);
}

/** Returns newly created CategoricalTemporalData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * CategoricalClusterDataFactory::GetNewTemporalClusterData(const AbstractDataSetGateway& DataGateway) const {
  return new CategoricalTemporalData(DataGateway);
}

/** Returns newly created CategoricalSpaceTimeData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * CategoricalClusterDataFactory::GetNewSpaceTimeClusterData(const DataSetInterface& Interface) const {
  return new CategoricalSpaceTimeData(Interface);
}

/** Returns newly created CategoricalSpaceTimeData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * CategoricalClusterDataFactory::GetNewSpaceTimeClusterData(const AbstractDataSetGateway& DataGateway) const {
  return new CategoricalSpaceTimeData(DataGateway);
}

//******************************************************************************

/** class constructor */
MultiSetsCategoricalClusterDataFactory::MultiSetsCategoricalClusterDataFactory(const CParameters & Parameters) : AbstractClusterDataFactory() {
  try {
    Setup(Parameters);
  }
  catch (prg_exception& x) {
    x.addTrace("constructor()","MultiSetsCategoricalClusterDataFactory");
    throw;
  }
}

/** class destructor */
MultiSetsCategoricalClusterDataFactory::~MultiSetsCategoricalClusterDataFactory() {}

/** Not implemented. Throws prg_error. */
AbstractSpatialClusterData * MultiSetsCategoricalClusterDataFactory::GetNewSpatialClusterData(const DataSetInterface&) const {
  throw prg_error("GetNewSpatialClusterData(const DataSetInterface&) not implemented.","MultiSetsCategoricalClusterDataFactory");
}

/** Returns newly created MultiSetCategoricalSpatialData object as AbstractSpatialClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractSpatialClusterData * MultiSetsCategoricalClusterDataFactory::GetNewSpatialClusterData(const AbstractDataSetGateway& DataGateway) const {
  return new MultiSetCategoricalSpatialData(gClusterDataFactory, DataGateway);
}

/** Not implemented. Throws prg_error. */
AbstractTemporalClusterData * MultiSetsCategoricalClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData&, const DataSetInterface&) const {
  throw prg_error("GetNewProspectiveSpatialClusterData(const CSaTScanData&, const DataSetInterface&) not implemented.","MultiSetsCategoricalClusterDataFactory");
}

/** Returns newly created MultiSetCategoricalProspectiveSpatialData object as
    AbstractTemporalClusterData pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * MultiSetsCategoricalClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway) const {
  return new MultiSetCategoricalProspectiveSpatialData(gClusterDataFactory, Data, DataGateway);
}

/** Not implemented. Throws prg_error. */
AbstractTemporalClusterData * MultiSetsCategoricalClusterDataFactory::GetNewTemporalClusterData(const DataSetInterface&) const {
  throw prg_error("GetNewTemporalClusterData(const DataSetInterface&) not implemented.","MultiSetsCategoricalClusterDataFactory");
}

/** Returns newly created MultiSetTemporalData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * MultiSetsCategoricalClusterDataFactory::GetNewTemporalClusterData(const AbstractDataSetGateway& DataGateway) const {
  return new MultiSetCategoricalTemporalData(gClusterDataFactory, DataGateway);
}

/** Not implemented - throws prg_error. */
AbstractTemporalClusterData * MultiSetsCategoricalClusterDataFactory::GetNewSpaceTimeClusterData(const DataSetInterface&) const {
  throw prg_error("GetNewSpaceTimeClusterData(const DataSetInterface&) not implemented.","MultiSetsCategoricalClusterDataFactory");
}

/** Returns newly created MultiSetCategoricalSpaceTimeData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * MultiSetsCategoricalClusterDataFactory::GetNewSpaceTimeClusterData(const AbstractDataSetGateway& DataGateway) const {
  return new MultiSetCategoricalSpaceTimeData(gClusterDataFactory, DataGateway);
}

/** Internal class setup function. */
void MultiSetsCategoricalClusterDataFactory::Setup(const CParameters& Parameters) {
  try {
    if (!(Parameters.GetProbabilityModelType() == ORDINAL || Parameters.GetProbabilityModelType() == CATEGORICAL))
      throw prg_error("This class does implements a multiple dataset factory\n"
                      "interface for the Ordinal and Categorical probability models only.\n","Setup()");
  }
  catch (prg_exception& x) {
    x.addTrace("Setup(const CParameters&)","MultiSetsCategoricalClusterDataFactory");
    throw;
  }
}

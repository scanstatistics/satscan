//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "CategoricalClusterDataFactory.h"
#include "MultiSetCategoricalClusterData.h"

/** class constructor*/
CategoricalClusterDataFactory::CategoricalClusterDataFactory() : AbstractClusterDataFactory() {}

/** class destructor */
CategoricalClusterDataFactory::~CategoricalClusterDataFactory() {}

/** Returns newly created CategoricalSpatialData object as AbstractSpatialClusterData
    pointer. Caller is responsible for object destruction. */
AbstractSpatialClusterData * CategoricalClusterDataFactory::GetNewSpatialClusterData(const DataStreamInterface& Interface, int iRate) const {
  return new CategoricalSpatialData(Interface);
}

/** Returns newly created CategoricalSpatialData object as AbstractSpatialClusterData
    pointer. Caller is responsible for object destruction. */
AbstractSpatialClusterData * CategoricalClusterDataFactory::GetNewSpatialClusterData(const AbtractDataStreamGateway& DataGateway, int iRate) const {
  return new CategoricalSpatialData(DataGateway);
}

/** Returns newly created CategoricalProspectiveSpatialData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction. */
AbstractTemporalClusterData * CategoricalClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData& Data, const DataStreamInterface & Interface) const {
  return new CategoricalProspectiveSpatialData(Data, Interface);
}

/** Returns newly created CategoricalProspectiveSpatialData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction. */
AbstractTemporalClusterData * CategoricalClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData& Data, const AbtractDataStreamGateway& DataGateway) const {
  return new CategoricalProspectiveSpatialData(Data, DataGateway);
}

/** Returns newly created CategoricalTemporalData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * CategoricalClusterDataFactory::GetNewTemporalClusterData(const DataStreamInterface& Interface) const {
  return new CategoricalTemporalData(Interface);
}

/** Returns newly created CategoricalTemporalData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * CategoricalClusterDataFactory::GetNewTemporalClusterData(const AbtractDataStreamGateway& DataGateway) const {
  return new CategoricalTemporalData(DataGateway);
}

/** Returns newly created CategoricalSpaceTimeData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * CategoricalClusterDataFactory::GetNewSpaceTimeClusterData(const DataStreamInterface& Interface) const {
  return new CategoricalSpaceTimeData(Interface);
}

/** Returns newly created CategoricalSpaceTimeData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * CategoricalClusterDataFactory::GetNewSpaceTimeClusterData(const AbtractDataStreamGateway& DataGateway) const {
  return new CategoricalSpaceTimeData(DataGateway);
}

//******************************************************************************

/** class constructor */
MultiSetsCategoricalClusterDataFactory::MultiSetsCategoricalClusterDataFactory(const CParameters & Parameters) : AbstractClusterDataFactory() {
  try {
    Setup(Parameters);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","MultiSetsCategoricalClusterDataFactory");
    throw;
  }
}

/** class destructor */
MultiSetsCategoricalClusterDataFactory::~MultiSetsCategoricalClusterDataFactory() {}

/** Not implemented. Throws ZdException. */
AbstractSpatialClusterData * MultiSetsCategoricalClusterDataFactory::GetNewSpatialClusterData(const DataStreamInterface&, int) const {
  ZdGenerateException("GetNewSpatialClusterData(const DataStreamInterface&, int) not implemented.","MultiSetsCategoricalClusterDataFactory");
  return 0;
}

/** Returns newly created MultiSetCategoricalSpatialData object as AbstractSpatialClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractSpatialClusterData * MultiSetsCategoricalClusterDataFactory::GetNewSpatialClusterData(const AbtractDataStreamGateway& DataGateway, int iRate) const {
  return new MultiSetCategoricalSpatialData(gClusterDataFactory, DataGateway, iRate);
}

/** Not implemented. Throws ZdException. */
AbstractTemporalClusterData * MultiSetsCategoricalClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData&, const DataStreamInterface&) const {
  ZdGenerateException("GetNewProspectiveSpatialClusterData(const CSaTScanData&, const DataStreamInterface&) not implemented.","MultiSetsCategoricalClusterDataFactory");
  return 0;
}

/** Returns newly created MultiSetCategoricalProspectiveSpatialData object as
    AbstractTemporalClusterData pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * MultiSetsCategoricalClusterDataFactory::GetNewProspectiveSpatialClusterData(const CSaTScanData& Data, const AbtractDataStreamGateway& DataGateway) const {
  return new MultiSetCategoricalProspectiveSpatialData(gClusterDataFactory, Data, DataGateway);
}

/** Not implemented. Throws ZdException. */
AbstractTemporalClusterData * MultiSetsCategoricalClusterDataFactory::GetNewTemporalClusterData(const DataStreamInterface&) const {
  ZdGenerateException("GetNewTemporalClusterData(const DataStreamInterface&) not implemented.","MultiSetsCategoricalClusterDataFactory");
  return 0;
}

/** Returns newly created MultipleStreamTemporalData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * MultiSetsCategoricalClusterDataFactory::GetNewTemporalClusterData(const AbtractDataStreamGateway& DataGateway) const {
  return new MultiSetCategoricalTemporalData(gClusterDataFactory, DataGateway);
}

/** Not implemented - throws ZdException. */
AbstractTemporalClusterData * MultiSetsCategoricalClusterDataFactory::GetNewSpaceTimeClusterData(const DataStreamInterface&) const {
  ZdGenerateException("GetNewSpaceTimeClusterData(const DataStreamInterface&) not implemented.","MultiSetsCategoricalClusterDataFactory");
  return 0;
}

/** Returns newly created MultiSetCategoricalSpaceTimeData object as AbstractTemporalClusterData
    pointer. Caller is responsible for object destruction.*/
AbstractTemporalClusterData * MultiSetsCategoricalClusterDataFactory::GetNewSpaceTimeClusterData(const AbtractDataStreamGateway& DataGateway) const {
  return new MultiSetCategoricalSpaceTimeData(gClusterDataFactory, DataGateway);
}

/** Internal class setup function. */
void MultiSetsCategoricalClusterDataFactory::Setup(const CParameters& Parameters) {
  try {
    if (Parameters.GetProbabilityModelType() != ORDINAL)
      ZdGenerateException("This class does implements a multiple stream factory\n"
                          "interface for the Ordinal probability model only.\n","Setup()");
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup(const CParameters&)","MultiSetsCategoricalClusterDataFactory");
    throw;
  }
}

//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
#include "AbstractClusterData.h"
//---------------------------------------------------------------------------

//void AbstractClusterData::AddNeighborData(tract_t tNeighbor, const AbtractDataStreamGateway & DataGateway) {
//  ZdGenerateException("AddNeighborData(tract_t, const DataStreamInterface&, size_t) not implemented.","AbstractClusterData");
//}

double AbstractClusterData::CalculateLoglikelihoodRatio(CModel & Model) {
  ZdGenerateException("CalculateLoglikelihoodRatio() not implemented.","AbstractClusterData");
  return 0;
}

/** constructor */
AbstractClusterData::AbstractClusterData() {}

/** destructor */
AbstractClusterData::~AbstractClusterData() {}

/** constructor */
AbstractSpatialClusterData::AbstractSpatialClusterData(int iRate) : AbstractClusterData() {
  switch (iRate) {
    case LOW        : gfRateOfInterest = LowRate;       break;
    case HIGHANDLOW : gfRateOfInterest = HighOrLowRate; break;
    case HIGH       :
    default         : gfRateOfInterest = HighRate;
  };
}

/** destructor */
AbstractSpatialClusterData::~AbstractSpatialClusterData() {}

/** constructor */
AbstractTemporalClusterData::AbstractTemporalClusterData() : AbstractClusterData() {}

/** destructor */
AbstractTemporalClusterData::~AbstractTemporalClusterData() {}


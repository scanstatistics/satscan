//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "AbstractClusterData.h"

/** class constructor */
AbstractClusterData::AbstractClusterData() {}

/** class destructor */
AbstractClusterData::~AbstractClusterData() {}

/** Not implemented - throws ZdException. */
double AbstractClusterData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator) {
  ZdGenerateException("CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator&) not implemented.","AbstractClusterData");
  return 0;
}

//******************************************************************************

/** class constructor */
AbstractSpatialClusterData::AbstractSpatialClusterData(int iRate) : AbstractClusterData() {
  switch (iRate) {
    case LOW        : gfRateOfInterest = LowRate;       break;
    case HIGHANDLOW : gfRateOfInterest = HighOrLowRate; break;
    case HIGH       :
    default         : gfRateOfInterest = HighRate;
  };
}

/** class destructor */
AbstractSpatialClusterData::~AbstractSpatialClusterData() {}

//******************************************************************************

/** class constructor */
AbstractTemporalClusterData::AbstractTemporalClusterData() : AbstractClusterData() {}

/** class destructor */
AbstractTemporalClusterData::~AbstractTemporalClusterData() {}


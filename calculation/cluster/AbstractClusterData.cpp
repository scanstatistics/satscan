//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
#include "AbstractClusterData.h"
//---------------------------------------------------------------------------

/** constructor */
AbstractClusterData::AbstractClusterData() {}

/** destructor */
AbstractClusterData::~AbstractClusterData() {}

/** Calculates the log likelihood ratio of accumulated data through
    AbstractLikelihoodCalculator object. Not implementated in base class, throws
    ZdException. */
double AbstractClusterData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator & Calculator) {
  ZdGenerateException("CalculateLoglikelihoodRatio() not implemented.","AbstractClusterData");
  return 0;
}

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

/** class constructor */
AbstractTemporalClusterData::AbstractTemporalClusterData() : AbstractClusterData() {}

/** class destructor */
AbstractTemporalClusterData::~AbstractTemporalClusterData() {}


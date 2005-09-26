//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "AbstractClusterData.h"
#include "LoglikelihoodRatioUnifier.h"

/** class constructor */
AbstractClusterData::AbstractClusterData() {}

/** class destructor */
AbstractClusterData::~AbstractClusterData() {}

/** Not implemented - throws ZdException. */
double AbstractClusterData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator) {
  ZdGenerateException("CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator&) not implemented.","AbstractClusterData");
  return 0;
}

/** Fills passed vector with indexes of data sets that contributed to calculated loglikelihood ratio.
    -- This base class implementation handles all cluster data objects for which there is only one
       data set analyzed. */
void AbstractClusterData::GetDataSetIndexesComprisedInRatio(double, AbstractLikelihoodCalculator&, std::vector<unsigned int>& vDataSetIndexes) const {
  vDataSetIndexes.clear();
  vDataSetIndexes.push_back(0);
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

/** Reassociates internal data with passed DataSetInterface pointers.
    Not implemented - throws exception */
void AbstractTemporalClusterData::Reassociate(const DataSetInterface& Interface) {
  ZdGenerateException("Reassociate(const DataSetInterface&) not implemented.","AbstractTemporalClusterData");
}

/** Reassociates internal data with passed DataSetInterface pointers of DataGateway.
    Not implemented - throws exception */
void AbstractTemporalClusterData::Reassociate(const AbstractDataSetGateway& DataGateway) {
  ZdGenerateException("Reassociate(const AbstractDataSetGateway&) not implemented.","AbstractTemporalClusterData");
}


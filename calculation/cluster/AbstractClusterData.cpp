//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "AbstractClusterData.h"
#include "LoglikelihoodRatioUnifier.h"
#include "SSException.h"

/** class constructor */
AbstractClusterData::AbstractClusterData() {}

/** class destructor */
AbstractClusterData::~AbstractClusterData() {}

/** Not implemented - throws prg_error. */
double AbstractClusterData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator) {
  throw prg_error("CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator&) not implemented.","AbstractClusterData");
}

/** Not implemented - throws prg_error. */
count_t AbstractClusterData::GetCategoryCaseCount(unsigned int iCategoryIndex, unsigned int tSetIndex) const {
  throw prg_error("GetCategoryCaseCount(unsigned int,unsigned int) not implemented.","AbstractClusterData");
}

const AbstractLoglikelihoodRatioUnifier & AbstractClusterData::getRatioUnified(AbstractLikelihoodCalculator& Calculator) const {
  throw prg_error("getRatioUnified(AbstractLikelihoodCalculator&) not implemented.","AbstractClusterData");
}

//******************************************************************************

/** class constructor */
AbstractSpatialClusterData::AbstractSpatialClusterData() : AbstractClusterData() {}

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
  throw prg_error("Reassociate(const DataSetInterface&) not implemented.","AbstractTemporalClusterData");
}

/** Reassociates internal data with passed DataSetInterface pointers of DataGateway.
    Not implemented - throws exception */
void AbstractTemporalClusterData::Reassociate(const AbstractDataSetGateway& DataGateway) {
  throw prg_error("Reassociate(const AbstractDataSetGateway&) not implemented.","AbstractTemporalClusterData");
}


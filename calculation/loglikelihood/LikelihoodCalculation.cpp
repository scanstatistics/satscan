//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "LikelihoodCalculation.h"
#include "SaTScanData.h"
#include "LoglikelihoodRatioUnifier.h"
#include "SSException.h"

/** class constructor */
AbstractLikelihoodCalculator::AbstractLikelihoodCalculator(const CSaTScanData& DataHub)
                             :gDataHub(DataHub), gpUnifier(0), gtMinLowRateCases(0), gtMinHighRateCases(2),
                              gpRateOfInterest(0), gpRateOfInterestNormal(0) {
  try {
    //store data set totals for later calculation
    for (size_t t=0; t < gDataHub.GetDataSetHandler().GetNumDataSets(); ++t) {
       gvDataSetTotals.push_back(std::make_pair(gDataHub.GetDataSetHandler().GetDataSet(t).getTotalCases(),
                                                gDataHub.GetDataSetHandler().GetDataSet(t).getTotalMeasure()));
       gvDataSetMeasureAuxTotals.push_back(DataHub.GetDataSetHandler().GetDataSet(t).getTotalMeasureAux());
    }
    if (gDataHub.GetParameters().GetProbabilityModelType() == NORMAL) {
      switch (gDataHub.GetParameters().GetExecuteScanRateType()) {
        case LOW        : gpRateOfInterestNormal = &AbstractLikelihoodCalculator::LowRateNormal; break;
        case HIGHANDLOW : gpRateOfInterestNormal = &AbstractLikelihoodCalculator::HighOrLowRateNormal; break;
        case HIGH       :
        default         : gpRateOfInterestNormal = &AbstractLikelihoodCalculator::HighRateNormal;
      };
    }
    else if (gDataHub.GetParameters().GetProbabilityModelType() == WEIGHTEDNORMAL) {
      switch (gDataHub.GetParameters().GetExecuteScanRateType()) {
        case LOW        : gpRateOfInterestNormal = &AbstractLikelihoodCalculator::LowRateWeightedNormal; break;
        case HIGHANDLOW : gpRateOfInterestNormal = &AbstractLikelihoodCalculator::HighOrLowRateWeightedNormal; break;
        case HIGH       :
        default         : gpRateOfInterestNormal = &AbstractLikelihoodCalculator::HighRateWeightedNormal;
      };
    }
    else {
      switch (gDataHub.GetParameters().GetExecuteScanRateType()) {
        case LOW        : gpRateOfInterest = &AbstractLikelihoodCalculator::LowRate; break;
        case HIGHANDLOW : gpRateOfInterest = &AbstractLikelihoodCalculator::HighOrLowRate; break;
        case HIGH       :
        default         : gpRateOfInterest = &AbstractLikelihoodCalculator::HighRate;
      };
    }
    if (gDataHub.GetParameters().GetNumDataSets() > 1) {
      switch (gDataHub.GetParameters().GetMultipleDataSetPurposeType()) {
        case MULTIVARIATE :
          gpUnifier = new MultivariateUnifier(gDataHub.GetParameters().GetExecuteScanRateType(), gDataHub.GetParameters().GetProbabilityModelType()); break;
        case ADJUSTMENT :
          gpUnifier = new AdjustmentUnifier(gDataHub.GetParameters().GetExecuteScanRateType()); break;
        default :
          throw prg_error("Unknown purpose for multiple data sets '%d'.","constructor()",
                              gDataHub.GetParameters().GetMultipleDataSetPurposeType());
      }
    }
    switch (gDataHub.GetParameters().GetProbabilityModelType()) {
      case POISSON              :
      case BERNOULLI            :
      case SPACETIMEPERMUTATION :
      case CATEGORICAL          :
      case ORDINAL              :
      case RANK                 :
      case HOMOGENEOUSPOISSON   :
      case EXPONENTIAL          : gtMinLowRateCases = 0; gtMinHighRateCases = 2; break;
      case WEIGHTEDNORMAL       :
      case NORMAL               : gtMinLowRateCases = 2; gtMinHighRateCases = 2; break;
      default : throw prg_error("Unknown data model type '%d'.","constructor()", gDataHub.GetParameters().GetProbabilityModelType());
    };
  }
  catch (prg_exception& x) {
    delete gpUnifier;
    x.addTrace("constructor()","AbstractLikelihoodCalculator");
    throw;
  }
}

/** class destructor */
AbstractLikelihoodCalculator::~AbstractLikelihoodCalculator() {
  try {delete gpUnifier;}catch(...){}
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalcLogLikelihood(count_t, measure_t) const {
  throw prg_error("CalcLogLikelihood(count_t,measure_t) not implementated.","AbstractLikelihoodCalculator");
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalcLogLikelihoodRatio(count_t, measure_t,size_t) const {
  throw prg_error("CalcLogLikelihoodRatio(count_t,measure_t.size_t) not implementated.","AbstractLikelihoodCalculator");
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalcLogLikelihoodRatioOrdinal(const std::vector<count_t>& vOrdinalCases, size_t tSetIndex) const {
  throw prg_error("CalcLogLikelihoodRatioOrdinal(const std::vector<count_t>&,const std::vector<count_t>&) not implementated.","AbstractLikelihoodCalculator");
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalcLogLikelihoodRatioNormal(count_t tCases, measure_t tMeasure, measure_t tMeasure2, size_t tSetIndex) const {
  throw prg_error("CalcLogLikelihoodRatioNormal(count_t,measure_t,measure_t,count_t,measure_t,measure_t) not implementated.","AbstractLikelihoodCalculator");
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalcMonotoneLogLikelihood(tract_t, const std::vector<count_t>&, const std::vector<measure_t>&) const {
  throw prg_error("CalcMonotoneLogLikelihood(tract_t, const std::vector<count_t>&, const std::vector<measure_t>&) not implementated.","AbstractLikelihoodCalculator");
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalcSVTTLogLikelihood(size_t, SVTTClusterData&, const CTimeTrend&) const {
  throw prg_error("CalcSVTTLogLikelihood(size_t, CSVTTCluster*, const CTimeTrend&) not implementated.","AbstractLikelihoodCalculator");
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalculateFullStatistic(double dMaximizingValue, size_t tDataSetIndex) const {
  throw prg_error("CalculateFullStatistic(double_t,size_t) not implementated.","AbstractLikelihoodCalculator");
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalculateMaximizingValue(count_t n, measure_t u, size_t tDataSetIndex) const {
  throw prg_error("CalculateMaximizingValue(count_t,measure_t,size_t) not implementated.","AbstractLikelihoodCalculator");
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalculateMaximizingValueNormal(count_t n, measure_t u, measure_t u2, size_t tDataSetIndex) const {
  throw prg_error("CalculateMaximizingValueNormal(count_t,measure_t,measure_t,size_t) not implementated.","AbstractLikelihoodCalculator");
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalculateMaximizingValueOrdinal(const std::vector<count_t>& vOrdinalCases, size_t tSetIndex) const {
  throw prg_error("CalculateMaximizingValueOrdinal(const std::vector<count_t>&,size_t) not implementated.","AbstractLikelihoodCalculator");
}

/** returns log likelihood for total - not implemented - throws exception. */
double AbstractLikelihoodCalculator::GetLogLikelihoodForTotal(size_t) const {
  throw prg_error("GetLogLikelihoodForTotal(size_t) not implementated.","AbstractLikelihoodCalculator");
}

/** Returns reference to AbstractLoglikelihoodRatioUnifier object. Throw exception
    if object not allocated. */
AbstractLoglikelihoodRatioUnifier & AbstractLikelihoodCalculator::GetUnifier() const {
  if (!gpUnifier)
    throw prg_error("Log likelihood unifier not allocated.","GetUnifier()");
  return *gpUnifier;
}


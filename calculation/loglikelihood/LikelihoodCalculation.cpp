//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "LikelihoodCalculation.h"
#include "SaTScanData.h"
#include "LoglikelihoodRatioUnifier.h"

/** class constructor */
AbstractLikelihoodCalculator::AbstractLikelihoodCalculator(const CSaTScanData& DataHub)
                             :gDataHub(DataHub), gpUnifier(0) {
  Setup();
}

/** class destructor */
AbstractLikelihoodCalculator::~AbstractLikelihoodCalculator() {}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalcLogLikelihood(count_t, measure_t) const {
  ZdGenerateException("CalcLogLikelihood(count_t,measure_t) not implementated.","AbstractLikelihoodCalculator");
  return 0;
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalcLogLikelihoodRatio(count_t, measure_t,size_t) const {
  ZdGenerateException("CalcLogLikelihoodRatio(count_t,measure_t.size_t) not implementated.","AbstractLikelihoodCalculator");
  return 0;
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalcLogLikelihoodRatioOrdinal(const std::vector<count_t>& vOrdinalCases, size_t tSetIndex) const {
  ZdGenerateException("CalcLogLikelihoodRatioOrdinal(const std::vector<count_t>&,const std::vector<count_t>&) not implementated.","AbstractLikelihoodCalculator");
  return 0;
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalcLogLikelihoodRatioNormal(count_t tCases, measure_t tMeasure, measure_t tMeasure2, size_t tSetIndex) const {
  ZdGenerateException("CalcLogLikelihoodRatioNormal(count_t,measure_t,measure_t,count_t,measure_t,measure_t) not implementated.","AbstractLikelihoodCalculator");
  return 0;
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalcMonotoneLogLikelihood(const CPSMonotoneCluster&) const {
  ZdGenerateException("CalcMonotoneLogLikelihood(const CPSMonotoneCluster&) not implementated.","AbstractLikelihoodCalculator");
  return 0;
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalcSVTTLogLikelihood(size_t, CSVTTCluster*, const CTimeTrend&) const {
  ZdGenerateException("CalcSVTTLogLikelihood(size_t, CSVTTCluster*, const CTimeTrend&) not implementated.","AbstractLikelihoodCalculator");
  return 0;
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalculateFullStatistic(double dMaximizingValue, size_t tDataSetIndex) const {
  ZdGenerateException("CalculateFullStatistic(double_t,size_t) not implementated.","AbstractLikelihoodCalculator");
  return 0;
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalculateMaximizingValue(count_t n, measure_t u, size_t tDataSetIndex) const {
  ZdGenerateException("CalculateMaximizingValue(count_t,measure_t,size_t) not implementated.","AbstractLikelihoodCalculator");
  return 0;
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalculateMaximizingValueNormal(count_t n, measure_t u, measure_t u2, size_t tDataSetIndex) const {
  ZdGenerateException("CalculateMaximizingValueNormal(count_t,measure_t,measure_t,size_t) not implementated.","AbstractLikelihoodCalculator");
  return 0;
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalculateMaximizingValueOrdinal(const std::vector<count_t>& vOrdinalCases, size_t tSetIndex) const {
  ZdGenerateException("CalculateMaximizingValueOrdinal(const std::vector<count_t>&,size_t) not implementated.","AbstractLikelihoodCalculator");
  return 0;
}

/** returns log likelihood for total - not implemented - throws exception. */
double AbstractLikelihoodCalculator::GetLogLikelihoodForTotal(size_t) const {
  ZdGenerateException("GetLogLikelihoodForTotal(size_t) not implementated.","AbstractLikelihoodCalculator");
  return 0;
}

/** Returns reference to AbstractLoglikelihoodRatioUnifier object. Throw exception
    if object not allocated. */
AbstractLoglikelihoodRatioUnifier & AbstractLikelihoodCalculator::GetUnifier() const {
  if (!gpUnifier)
    ZdGenerateException("Log likelihood unifier not allocated.","GetUnifier()");
  return *gpUnifier;
}

/** Internal class setup */
void AbstractLikelihoodCalculator::Setup() {
  try {
    //store data set totals for later calculation
    for (size_t t=0; t < gDataHub.GetDataSetHandler().GetNumDataSets(); ++t)
       gvDataSetTotals.push_back(std::make_pair(gDataHub.GetDataSetHandler().GetDataSet(t).getTotalCases(),
                                                gDataHub.GetDataSetHandler().GetDataSet(t).getTotalMeasure()));
                                                
    switch (gDataHub.GetParameters().GetExecuteScanRateType()) {
      case LOW        : gpRateOfInterest = &AbstractLikelihoodCalculator::LowRate; break;
      case HIGHANDLOW : gpRateOfInterest = &AbstractLikelihoodCalculator::HighOrLowRate; break;
      case HIGH       :
      default         : gpRateOfInterest = &AbstractLikelihoodCalculator::HighRate;
    };

    if (gDataHub.GetParameters().GetNumDataSets() > 1) {
      switch (gDataHub.GetParameters().GetMultipleDataSetPurposeType()) {
        case MULTIVARIATE :
          gpUnifier = new MultivariateUnifier(gDataHub.GetParameters().GetExecuteScanRateType()); break;
        case ADJUSTMENT :
          gpUnifier = new AdjustmentUnifier(gDataHub.GetParameters().GetExecuteScanRateType()); break;
        default :
          ZdGenerateException("Unknown purpose for multiple data sets '%d'.","GetUnifier",
                              gDataHub.GetParameters().GetMultipleDataSetPurposeType());
      }
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","AbstractLikelihoodCalculator");
    throw;
  }
}


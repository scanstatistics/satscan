//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "LikelihoodCalculation.h"
#include "SaTScanData.h"
#include "LoglikelihoodRatioUnifier.h"

/** class constructor */
AbstractLikelihoodCalculator::AbstractLikelihoodCalculator(const CSaTScanData& Data)
                             :gData(Data),
                              gtTotalCasesInDataSet(Data.GetTotalCases()),
                              gtTotalMeasureInDataSet(Data.GetTotalMeasure()) {
  Init();
  Setup();
}

/** class destructor */
AbstractLikelihoodCalculator::~AbstractLikelihoodCalculator() {
  try {
  }
  catch (...){}
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalcLogLikelihoodRatioOrdinal(const std::vector<count_t>& vOrdinalCases,
                                                                   const std::vector<count_t>& vOrdinalTotalCases) const {
  ZdGenerateException("CalcLogLikelihoodRatioOrdinal(const std::vector<count_t>&,const std::vector<count_t>&) not implementated.","AbstractLikelihoodCalculator");
  return 0;
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalcLogLikelihoodRatioNormal(count_t tCases,
                                                                  measure_t tMeasure,
                                                                  measure_t tMeasure2,
                                                                  count_t tTotalCases,
                                                                  measure_t tTotalMeasure) const {
  ZdGenerateException("CalcLogLikelihoodRatioNormal(count_t,measure_t,measure_t,count_t,measure_t) not implementated.","AbstractLikelihoodCalculator");
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
    if (gData.GetParameters().GetNumDataStreams() > 1) {
      switch (gData.GetParameters().GetMultipleDataStreamPurposeType()) {
        case MULTIVARIATE :
          gpUnifier = new MultivariateUnifier(gData.GetParameters().GetAreaScanRateType()); break;
        case ADJUSTMENT :
          gpUnifier = new AdjustmentUnifier(gData.GetParameters().GetAreaScanRateType()); break;
        default :
          ZdGenerateException("Unknown purpose for multiple data sets '%d'.","GetUnifier",
                              gData.GetParameters().GetMultipleDataStreamPurposeType());
      }
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","AbstractLikelihoodCalculator");
    throw;
  }
}

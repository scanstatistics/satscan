//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "LikelihoodCalculation.h"
#include "SaTScanData.h"

/** class constructor */
MultivariateUnifier::MultivariateUnifier(AreaRateType eScanningArea)
                    :gdHighRateRatios(0), gdLowRateRatios(0),
                     gbScanLowRates(eScanningArea == LOW || eScanningArea == HIGHANDLOW),
                     gbScanHighRates(eScanningArea == HIGH || eScanningArea == HIGHANDLOW) {}

/** Calculates loglikelihood ratio given parameter data; accumulating like high and low
    rate separately. */
void MultivariateUnifier::AdjoinRatio(AbstractLikelihoodCalculator& Calculator,
                                      count_t tCases,
                                      measure_t tMeasure,
                                      count_t tTotalCases,
                                      measure_t tTotalMeasure) {

  if (gbScanLowRates && LowRate(tCases, tMeasure, tTotalCases, tTotalMeasure))
    gdLowRateRatios += Calculator.CalcLogLikelihoodRatio(tCases, tMeasure, tTotalCases, tTotalMeasure);
  if (gbScanHighRates && MultipleSetsHighRate(tCases, tMeasure, tTotalCases, tTotalMeasure))
    gdHighRateRatios += Calculator.CalcLogLikelihoodRatio(tCases, tMeasure, tTotalCases, tTotalMeasure);
}

/** Returns the largest calculated loglikelihood ratio by comparing summed ratios
    that were for high rates to those that were for low rates. */
double MultivariateUnifier::GetLoglikelihoodRatio() const {
  return std::max(gdHighRateRatios, gdLowRateRatios);
}

/** Resets internal class members for another iteration of computing unified
    log likelihood ratios.*/
void MultivariateUnifier::Reset() {
  gdHighRateRatios = gdLowRateRatios = 0;
}

/** class constructor */
AdjustmentUnifier::AdjustmentUnifier(AreaRateType eScanningArea)
                  :geScanningArea(eScanningArea) {}

/** Calculates loglikelihood ratio given parameter data. The calculated ratio
    might be adjusted through multiplying by positive or negative one; based
    upon comparing observed to expected cases. */
void AdjustmentUnifier::AdjoinRatio(AbstractLikelihoodCalculator& Calculator,
                                    count_t tCases,
                                    measure_t tMeasure,
                                    count_t tTotalCases,
                                    measure_t tTotalMeasure) {

  if (MultipleSetsHighRate(tCases, tMeasure, tTotalCases, tTotalMeasure))
    gdRatio += Calculator.CalcLogLikelihoodRatio(tCases, tMeasure, tTotalCases, tTotalMeasure);
  else
    gdRatio += -1 * Calculator.CalcLogLikelihoodRatio(tCases, tMeasure, tTotalCases, tTotalMeasure);
}

/** Returns calculated loglikelihood ratio that is the sum of adjoined values.
    Based upon scanning rate, returned value is adjusted such that if rate is:
    high         ; no adjustment occurs
    low          ; ratio * -1 is returned
    high and low ; absolute value of ratio is returned */
double AdjustmentUnifier::GetLoglikelihoodRatio() const {
  switch (geScanningArea) {
    case HIGHANDLOW : return std::fabs(gdRatio);
    case LOW        : return gdRatio * -1;
    default         : return gdRatio;
  };
}

/** Resets internal class members for another iteration of computing unified
    log likelihood ratios.*/
void AdjustmentUnifier::Reset() {
  gdRatio = 0;
}

/** constructor */                           
AbstractLikelihoodCalculator::AbstractLikelihoodCalculator(const CSaTScanData& Data)
                             :gData(Data),
                              gtTotalCasesInDataSet(Data.GetTotalCases()),
                              gtTotalMeasureInDataSet(Data.GetTotalMeasure()) {
  Init();
  Setup();
}

/** destructor */
AbstractLikelihoodCalculator::~AbstractLikelihoodCalculator() {
  try {
  }
  catch (...){}
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalcLogLikelihoodRatioEx(count_t tCases,
                                                              measure_t tMeasure,
                                                              measure_t tMeasure2,
                                                              count_t tTotalCases,
                                                              measure_t tTotalMeasure) const {
  ZdGenerateException("CalcLogLikelihoodRatioEx() not implementated.","AbstractLikelihoodCalculator");
  return 0;
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster) const {
  ZdGenerateException("CalcMonotoneLogLikelihood() not implementated.","AbstractLikelihoodCalculator");
  return 0;
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalcSVTTLogLikelihood(size_t tStream, CSVTTCluster* Cluster, const CTimeTrend& GlobalTimeTrend) const {
  ZdGenerateException("CalcSVTTLogLikelihood() not implementated.","AbstractLikelihoodCalculator");
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
          ZdGenerateException("Unknown purpose for multiple data streams '%d'.","GetUnifier",
                              gData.GetParameters().GetMultipleDataStreamPurposeType());
      }
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","AbstractLikelihoodCalculator");
    throw;
  }
}

//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "LoglikelihoodRatioUnifier.h"
#include "LikelihoodCalculation.h"
#include "OrdinalLikelihoodCalculation.h"

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

/** Calculates loglikelihood ratio given ordinal data; accumulating like high
    and low rate separately. */
void MultivariateUnifier::AdjoinRatio(AbstractLikelihoodCalculator& Calculator, const std::vector<count_t>& vOrdinalCases, size_t tSetIndex) {
  if (gbScanLowRates)
    gdLowRateRatios += ((OrdinalLikelihoodCalculator&)Calculator).CalcLogLikelihoodRatioOrdinalLowRate(vOrdinalCases, tSetIndex);
  if (gbScanHighRates)
    gdHighRateRatios += ((OrdinalLikelihoodCalculator&)Calculator).CalcLogLikelihoodRatioOrdinalHighRate(vOrdinalCases, tSetIndex);
}

/** Calculates loglikelihood ratio given parameter data; accumulating high and low
    rates separately; storing is passed pair object. */
void MultivariateUnifier::GetHighLowRatio(AbstractLikelihoodCalculator& Calculator,
                                          count_t tCases,
                                          measure_t tMeasure,
                                          count_t tTotalCases,
                                          measure_t tTotalMeasure,
                                          std::pair<double, double>& prHighLowRatios) {

  prHighLowRatios.second = 0;
  prHighLowRatios.first = 0;
  if (gbScanLowRates && LowRate(tCases, tMeasure, tTotalCases, tTotalMeasure))
    prHighLowRatios.second = Calculator.CalcLogLikelihoodRatio(tCases, tMeasure, tTotalCases, tTotalMeasure);
  if (gbScanHighRates && MultipleSetsHighRate(tCases, tMeasure, tTotalCases, tTotalMeasure))
    prHighLowRatios.first = Calculator.CalcLogLikelihoodRatio(tCases, tMeasure, tTotalCases, tTotalMeasure);
}

/** Calculates loglikelihood ratio given parameter data; accumulating high and low
    rates separately; storing is passed pair object. */
void MultivariateUnifier::GetHighLowRatioOrdinal(AbstractLikelihoodCalculator& Calculator,
                                                 const std::vector<count_t>& vOrdinalCases,
                                                 size_t tSetIndex,
                                                 std::pair<double, double>& prHighLowRatios) {

  prHighLowRatios.second = 0;
  prHighLowRatios.first = 0;
  if (gbScanLowRates)
    prHighLowRatios.second = ((OrdinalLikelihoodCalculator&)Calculator).CalcLogLikelihoodRatioOrdinalLowRate(vOrdinalCases, tSetIndex);
  if (gbScanHighRates)
    prHighLowRatios.first = ((OrdinalLikelihoodCalculator&)Calculator).CalcLogLikelihoodRatioOrdinalHighRate(vOrdinalCases, tSetIndex);
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

//******************************************************************************

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

void AdjustmentUnifier::AdjoinRatio(AbstractLikelihoodCalculator& Calculator, const std::vector<count_t>& vOrdinalCases, size_t tSetIndex) {
  ZdGenerateException("AdjoinRatio() not implementated yet for Adjustment option!","AdjustmentUnifier");
  //gdRatio += Calculator.CalcLogLikelihoodRatioOrdinal(vOrdinalCases, tSetIndex);
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


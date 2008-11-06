//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "LoglikelihoodRatioUnifier.h"
#include "LikelihoodCalculation.h"
#include "OrdinalLikelihoodCalculation.h"
#include "SSException.h"

/** class constructor */
MultivariateUnifier::MultivariateUnifier(AreaRateType eScanningArea, ProbabilityModelType eProbabilityModelType)
                    :gdHighRateRatios(0), gdLowRateRatios(0), geProbabilityModelType(eProbabilityModelType),
                     gbScanLowRates(eScanningArea == LOW || eScanningArea == HIGHANDLOW),
                     gbScanHighRates(eScanningArea == HIGH || eScanningArea == HIGHANDLOW) {}

/** Calculates loglikelihood ratio given parameter data; accumulating like high and low rate separately. */
void MultivariateUnifier::AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, size_t tSetIndex) {
  if (gbScanLowRates && Calculator.LowRate(tCases, tMeasure, tSetIndex))
    gdLowRateRatios += Calculator.CalcLogLikelihoodRatio(tCases, tMeasure, tSetIndex);
  if (gbScanHighRates && Calculator.MultipleSetsHighRate(tCases, tMeasure, tSetIndex))
    gdHighRateRatios += Calculator.CalcLogLikelihoodRatio(tCases, tMeasure, tSetIndex);
}

/** Calculates loglikelihood ratio given parameter data; accumulating like high and low rate separately. */
void MultivariateUnifier::AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, measure_t tMeasureAux, size_t tSetIndex) {
  if (gbScanLowRates && (geProbabilityModelType == WEIGHTEDNORMAL ? Calculator.LowRateWeightedNormal(tCases, tMeasure, tMeasureAux, tSetIndex) : Calculator.LowRate(tCases, tMeasure, tSetIndex)))
    gdLowRateRatios += Calculator.CalcLogLikelihoodRatioNormal(tCases, tMeasure, tMeasureAux, tSetIndex);
  if (gbScanHighRates && (geProbabilityModelType == WEIGHTEDNORMAL ? Calculator.MultipleSetsHighRateWeightedNormal(tCases, tMeasure, tMeasureAux, tSetIndex) : Calculator.MultipleSetsHighRate(tCases, tMeasure, tSetIndex)))
    gdHighRateRatios += Calculator.CalcLogLikelihoodRatioNormal(tCases, tMeasure, tMeasureAux, tSetIndex);
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
                                          size_t tSetIndex,
                                          std::pair<double, double>& prHighLowRatios) {

  prHighLowRatios.second = 0;
  prHighLowRatios.first = 0;
  if (gbScanLowRates && Calculator.LowRate(tCases, tMeasure, tSetIndex))
    prHighLowRatios.second = Calculator.CalcLogLikelihoodRatio(tCases, tMeasure, tSetIndex);
  if (gbScanHighRates && Calculator.MultipleSetsHighRate(tCases, tMeasure, tSetIndex))
    prHighLowRatios.first = Calculator.CalcLogLikelihoodRatio(tCases, tMeasure, tSetIndex);
}

/** Calculates loglikelihood ratio given parameter data; accumulating high and low
    rates separately; storing is passed pair object. */
void MultivariateUnifier::GetHighLowRatio(AbstractLikelihoodCalculator& Calculator,
                                          count_t tCases,
                                          measure_t tMeasure,
                                          measure_t tMeasureAux,
                                          size_t tSetIndex,
                                          std::pair<double, double>& prHighLowRatios) {
  prHighLowRatios.second = 0;
  prHighLowRatios.first = 0;
  if (gbScanLowRates && (geProbabilityModelType == WEIGHTEDNORMAL ? Calculator.LowRateWeightedNormal(tCases, tMeasure, tMeasureAux, tSetIndex) : Calculator.LowRate(tCases, tMeasure, tSetIndex)))
    prHighLowRatios.second = Calculator.CalcLogLikelihoodRatioNormal(tCases, tMeasure, tMeasureAux, tSetIndex);
  if (gbScanHighRates && (geProbabilityModelType == WEIGHTEDNORMAL ? Calculator.MultipleSetsHighRateWeightedNormal(tCases, tMeasure, tMeasureAux, tSetIndex) : Calculator.MultipleSetsHighRate(tCases, tMeasure, tSetIndex)))
    prHighLowRatios.first = Calculator.CalcLogLikelihoodRatioNormal(tCases, tMeasure, tMeasureAux, tSetIndex);
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
                                    size_t tSetIndex) {
  if (Calculator.MultipleSetsHighRate(tCases, tMeasure, tSetIndex))
    gdRatio += Calculator.CalcLogLikelihoodRatio(tCases, tMeasure, tSetIndex);
  else if (Calculator.LowRate(tCases, tMeasure, tSetIndex))
    gdRatio += -1 * Calculator.CalcLogLikelihoodRatio(tCases, tMeasure, tSetIndex);
}

void AdjustmentUnifier::AdjoinRatio(AbstractLikelihoodCalculator& Calculator,
                                    count_t tCases,
                                    measure_t tMeasure,
                                    measure_t tMeasureAux,
                                    size_t tSetIndex) {
  if (Calculator.MultipleSetsHighRate(tCases, tMeasure, tSetIndex))
    gdRatio += Calculator.CalcLogLikelihoodRatioNormal(tCases, tMeasure, tMeasureAux, tSetIndex);
  else if (Calculator.LowRate(tCases, tMeasure, tSetIndex))
    gdRatio += -1 * Calculator.CalcLogLikelihoodRatioNormal(tCases, tMeasure, tMeasureAux, tSetIndex);
}

void AdjustmentUnifier::AdjoinRatio(AbstractLikelihoodCalculator& Calculator, const std::vector<count_t>& vOrdinalCases, size_t tSetIndex) {
  throw prg_error("AdjoinRatio() not implementated yet for Adjustment option!","AdjustmentUnifier");
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


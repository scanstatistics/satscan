//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "LoglikelihoodRatioUnifier.h"
#include "LikelihoodCalculation.h"
#include "OrdinalLikelihoodCalculation.h"
#include "SaTScanData.h"

/////////////////////////////// MultivariateUnifier /////////////////////////////////////////////////

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
void MultivariateUnifier::AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) {
    if (gbScanLowRates && Calculator.LowRateUniformTime(tCases, tMeasure, casesInPeriod, measureInPeriod, tSetIndex))
        gdLowRateRatios += Calculator.CalcLogLikelihoodRatioUniformTime(tCases, tMeasure, casesInPeriod, measureInPeriod, tSetIndex);
    if (gbScanHighRates && Calculator.MultipleSetsHighRateUniformTime(tCases, tMeasure, casesInPeriod, measureInPeriod, tSetIndex))
        gdHighRateRatios += Calculator.CalcLogLikelihoodRatioUniformTime(tCases, tMeasure, casesInPeriod, measureInPeriod, tSetIndex);
}

/** Calculates loglikelihood ratio given parameter data; accumulating like high and low rate separately. */
void MultivariateUnifier::AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, measure_t tMeasureAux, size_t tSetIndex) {
  bool bWeightedNormal = geProbabilityModelType == NORMAL && Calculator.GetDataHub().GetParameters().getIsWeightedNormal();  
  if (gbScanLowRates && (bWeightedNormal ? Calculator.LowRateWeightedNormal(tCases, tMeasure, tMeasureAux, tSetIndex) : Calculator.LowRate(tCases, tMeasure, tSetIndex)))
    gdLowRateRatios += Calculator.CalcLogLikelihoodRatioNormal(tCases, tMeasure, tMeasureAux, tSetIndex);
  if (gbScanHighRates && (bWeightedNormal ? Calculator.MultipleSetsHighRateWeightedNormal(tCases, tMeasure, tMeasureAux, tSetIndex) : Calculator.MultipleSetsHighRate(tCases, tMeasure, tSetIndex)))
    gdHighRateRatios += Calculator.CalcLogLikelihoodRatioNormal(tCases, tMeasure, tMeasureAux, tSetIndex);
}

/** Calculates loglikelihood ratio given ordinal data; accumulating like high and low rate separately. */
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
                                          count_t casesInPeriod,
                                          measure_t measureInPeriod,
                                          size_t tSetIndex,
                                          std::pair<double, double>& prHighLowRatios) {

    prHighLowRatios.second = 0;
    prHighLowRatios.first = 0;
    if (gbScanLowRates && Calculator.LowRateUniformTime(tCases, tMeasure, casesInPeriod, measureInPeriod, tSetIndex))
        prHighLowRatios.second = Calculator.CalcLogLikelihoodRatioUniformTime(tCases, tMeasure, casesInPeriod, measureInPeriod, tSetIndex);
    if (gbScanHighRates && Calculator.MultipleSetsHighRateUniformTime(tCases, tMeasure, casesInPeriod, measureInPeriod, tSetIndex))
        prHighLowRatios.first = Calculator.CalcLogLikelihoodRatioUniformTime(tCases, tMeasure, casesInPeriod, measureInPeriod, tSetIndex);
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
  bool bWeightedNormal = geProbabilityModelType == NORMAL && Calculator.GetDataHub().GetParameters().getIsWeightedNormal();
  if (gbScanLowRates && (bWeightedNormal ? Calculator.LowRateWeightedNormal(tCases, tMeasure, tMeasureAux, tSetIndex) : Calculator.LowRate(tCases, tMeasure, tSetIndex)))
    prHighLowRatios.second = Calculator.CalcLogLikelihoodRatioNormal(tCases, tMeasure, tMeasureAux, tSetIndex);
  if (gbScanHighRates && (bWeightedNormal ? Calculator.MultipleSetsHighRateWeightedNormal(tCases, tMeasure, tMeasureAux, tSetIndex) : Calculator.MultipleSetsHighRate(tCases, tMeasure, tSetIndex)))
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

////////////////////////////////////////////// MultivariateUnifierRiskThreshold /////////////////////////////

/** Calculates loglikelihood ratio given parameter data; accumulating like high and low rate separately. */
void MultivariateUnifierRiskThreshold::AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, size_t tSetIndex) {
    bool set = false;
    if (gbScanLowRates && Calculator.LowRate(tCases, tMeasure, tSetIndex)) {
        gdLowRateRatios += Calculator.CalcLogLikelihoodRatio(tCases, tMeasure, tSetIndex);
        set = true;
    }
    if (gbScanHighRates && Calculator.MultipleSetsHighRate(tCases, tMeasure, tSetIndex)) {
        gdHighRateRatios += Calculator.CalcLogLikelihoodRatio(tCases, tMeasure, tSetIndex);
        set = true;
    }
    if (set) {
        _risk_threshold._sum_observed += tCases;
        _risk_threshold._sum_expected += tMeasure * Calculator.gvDataSetTotals[tSetIndex].first / Calculator.gvDataSetTotals[tSetIndex].second;
        _risk_threshold._sum_case_totals += Calculator.gvDataSetTotals[tSetIndex].first;
        _risk_threshold._sum_expected_totals += Calculator.gvDataSetTotals[tSetIndex].second;
        ++_risk_threshold._num_in_sum;
    }
}

///////////////////////////////////// AdjustmentUnifier /////////////////////////////////////////

/** class constructor */
AdjustmentUnifier::AdjustmentUnifier(AreaRateType eScanningArea) : geScanningArea(eScanningArea) {}

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

void AdjustmentUnifier::AdjoinRatio(AbstractLikelihoodCalculator& Calculator, 
                                    count_t tCases, 
                                    measure_t tMeasure, 
                                    count_t casesInPeriod, 
                                    measure_t measureInPeriod, 
                                    size_t tSetIndex) {
    if (Calculator.MultipleSetsHighRateUniformTime(tCases, tMeasure, casesInPeriod, measureInPeriod, tSetIndex))
        gdRatio += Calculator.CalcLogLikelihoodRatioUniformTime(tCases, tMeasure, casesInPeriod, measureInPeriod, tSetIndex);
    else if (Calculator.LowRateUniformTime(tCases, tMeasure, casesInPeriod, measureInPeriod, tSetIndex))
        gdRatio += -1 * Calculator.CalcLogLikelihoodRatioUniformTime(tCases, tMeasure, casesInPeriod, measureInPeriod, tSetIndex);
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

///////////////////////////////////////// AdjustmentUnifierRiskThreshold /////////////////////////

/** Calculates loglikelihood ratio given parameter data. The calculated ratio
might be adjusted through multiplying by positive or negative one; based
upon comparing observed to expected cases. */
void AdjustmentUnifierRiskThreshold::AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, size_t tSetIndex) {
    if (Calculator.MultipleSetsHighRate(tCases, tMeasure, tSetIndex)) {
        gdRatio += Calculator.CalcLogLikelihoodRatio(tCases, tMeasure, tSetIndex);
        _risk_threshold._sum_observed += tCases;
        _risk_threshold._sum_expected += tMeasure * Calculator.gvDataSetTotals[tSetIndex].first / Calculator.gvDataSetTotals[tSetIndex].second;
        _risk_threshold._sum_case_totals += Calculator.gvDataSetTotals[tSetIndex].first;
        _risk_threshold._sum_expected_totals += Calculator.gvDataSetTotals[tSetIndex].second;
        ++_risk_threshold._num_in_sum;
    }
    else if (Calculator.LowRate(tCases, tMeasure, tSetIndex)) {
        gdRatio += -1 * Calculator.CalcLogLikelihoodRatio(tCases, tMeasure, tSetIndex);
        _risk_threshold._sum_observed += tCases;
        _risk_threshold._sum_expected += tMeasure * Calculator.gvDataSetTotals[tSetIndex].first / Calculator.gvDataSetTotals[tSetIndex].second;
        _risk_threshold._sum_case_totals += Calculator.gvDataSetTotals[tSetIndex].first;
        _risk_threshold._sum_expected_totals += Calculator.gvDataSetTotals[tSetIndex].second;
        ++_risk_threshold._num_in_sum;
    }
}

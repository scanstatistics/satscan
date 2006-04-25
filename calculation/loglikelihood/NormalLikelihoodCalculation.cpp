//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "NormalLikelihoodCalculation.h"

/** constructor */
NormalLikelihoodCalculator::NormalLikelihoodCalculator(const CSaTScanData& Data)
                           :AbstractLikelihoodCalculator(Data) {}

/** destructor */
NormalLikelihoodCalculator::~NormalLikelihoodCalculator() {}

/** calculates the log likelihood given the number of observed and expected cases  - not implemented - throws exception.*/
double NormalLikelihoodCalculator::CalcLogLikelihood(count_t n, measure_t u) const {
  ZdGenerateException("CalcLogLikelihood() not implementated.","NormalLikelihoodCalculator");
  return 0;
}

/** calculates the Poisson log likelihood ratio given the number of observed and expected cases  - not implemented - throws exception.*/
double NormalLikelihoodCalculator::CalcLogLikelihoodRatio(count_t n, measure_t u, count_t N, measure_t U) const {
  ZdGenerateException("CalcLogLikelihoodRatio() not implementated.","NormalLikelihoodCalculator");
  return 0;
}

/** calculates the Poisson log likelihood ratio given the number of observed and expected cases, and second measure  - not implemented - throws exception. */
double NormalLikelihoodCalculator::CalcLogLikelihoodRatioNormal(count_t tCases, measure_t tMeasure, measure_t tMeasure2, count_t tTotalCases, measure_t tTotalMeasure, measure_t tTotalMeasureSq) const {
  if (!(tTotalCases - tCases)) return 0; //when the cluster contains all the cases in set, ratio is zero
  assert(tCases > 0);
  double dEstimatedMeanInside = tMeasure/tCases;
  double dEstimatedMeanOutside = (tTotalMeasure - tMeasure)/(tTotalCases - tCases);
  double dEstimatedVariance = (1.0/tTotalCases)
                              * (tMeasure2 - 2.0 * tMeasure * dEstimatedMeanInside + tCases * std::pow(dEstimatedMeanInside, 2) +
                                 (tTotalMeasureSq - tMeasure2) - 2.0 * (tTotalMeasure - tMeasure) * dEstimatedMeanOutside +
                                 (tTotalCases - tCases) * std::pow(dEstimatedMeanOutside, 2) );


  return -1 * tTotalCases * std::log(std::sqrt(dEstimatedVariance)) + tTotalCases * std::log(std::sqrt(tTotalMeasureSq/tTotalCases - std::pow(tTotalMeasure/tTotalCases, 2)));
}

/** returns log likelihood for total - not implemented - throws exception. */
double NormalLikelihoodCalculator::GetLogLikelihoodForTotal() const {
  ZdGenerateException("GetLogLikelihoodForTotal() not implementated.","NormalLikelihoodCalculator");
  return 0;
}

/** Returns log likelihood ratio given passed log likelihood - not implemented - throws exception.  */
double NormalLikelihoodCalculator::GetLogLikelihoodRatio(double dLogLikelihood) const {
  ZdGenerateException("GetLogLikelihoodRatio() not implementated.","NormalLikelihoodCalculator");
  return 0;
}


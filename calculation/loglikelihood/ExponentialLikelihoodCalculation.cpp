//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "ExponentialLikelihoodCalculation.h"

/** constructor */
ExponentialLikelihoodCalculator::ExponentialLikelihoodCalculator(count_t tTotalCases, measure_t tTotalMeasure)
                                :AbstractLikelihoodCalculator(tTotalCases, tTotalMeasure) {}

/** destructor */
ExponentialLikelihoodCalculator::~ExponentialLikelihoodCalculator() {}

/** calculates the log likelihood given the number of observed and expected cases */
double ExponentialLikelihoodCalculator::CalcLogLikelihood(count_t n, measure_t u) {
  ZdGenerateException("CalcLogLikelihood() not implementated.","ExponentialLikelihoodCalculator");
  return 0;
}

/** calculates the Poisson log likelihood ratio given the number of observed and expected cases */
double ExponentialLikelihoodCalculator::CalcLogLikelihoodRatio(count_t n, measure_t u, count_t N, measure_t U) {
  ZdGenerateException("CalcLogLikelihoodRatio() not implementated.","ExponentialLikelihoodCalculator");
  return 0;
}

/** returns log likelihood for total */
double ExponentialLikelihoodCalculator::GetLogLikelihoodForTotal() const {
  ZdGenerateException("GetLogLikelihoodForTotal() not implementated.","ExponentialLikelihoodCalculator");
  return 0;
}


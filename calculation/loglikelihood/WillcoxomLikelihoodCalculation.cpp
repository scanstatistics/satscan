//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "WillcoxomLikelihoodCalculation.h"

/** constructor */
WillcoxomLikelihoodCalculator::WillcoxomLikelihoodCalculator(count_t tTotalCases, measure_t tTotalMeasure)
                              :AbstractLikelihoodCalculator(tTotalCases, tTotalMeasure) {}

/** destructor */
WillcoxomLikelihoodCalculator::~WillcoxomLikelihoodCalculator() {}

/** calculates the log likelihood given the number of observed and expected cases */
double WillcoxomLikelihoodCalculator::CalcLogLikelihood(count_t n, measure_t u) {
  ZdGenerateException("CalcLogLikelihood() not implementated.","WillcoxomLikelihoodCalculator");
  return 0;
}

/** calculates the Poisson log likelihood ratio given the number of observed and expected cases */
double WillcoxomLikelihoodCalculator::CalcLogLikelihoodRatio(count_t n, measure_t u, count_t N, measure_t U) {
  ZdGenerateException("CalcLogLikelihoodRatio() not implementated.","WillcoxomLikelihoodCalculator");
  return 0;
}

/** returns log likelihood for total */
double WillcoxomLikelihoodCalculator::GetLogLikelihoodForTotal() const {
  ZdGenerateException("GetLogLikelihoodForTotal() not implementated.","WillcoxomLikelihoodCalculator");
  return 0;
}
 

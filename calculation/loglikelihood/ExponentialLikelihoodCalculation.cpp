//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "ExponentialLikelihoodCalculation.h"

/** constructor */
ExponentialLikelihoodCalculator::ExponentialLikelihoodCalculator(const CSaTScanData& Data)
                                :AbstractLikelihoodCalculator(Data) {}

/** destructor */
ExponentialLikelihoodCalculator::~ExponentialLikelihoodCalculator() {}

/** calculates the log likelihood given the number of observed and expected cases */
double ExponentialLikelihoodCalculator::CalcLogLikelihood(count_t n, measure_t u) const {
  ZdGenerateException("CalcLogLikelihood() not implementated.","ExponentialLikelihoodCalculator");
  return 0;
}

/** calculates the Poisson log likelihood ratio given the number of observed and expected cases */
double ExponentialLikelihoodCalculator::CalcLogLikelihoodRatio(count_t n, measure_t u, count_t N, measure_t U) const {
  ZdGenerateException("CalcLogLikelihoodRatio() not implementated.","ExponentialLikelihoodCalculator");
  return 0;
}

/** returns log likelihood for total */
double ExponentialLikelihoodCalculator::GetLogLikelihoodForTotal() const {
  ZdGenerateException("GetLogLikelihoodForTotal() not implementated.","ExponentialLikelihoodCalculator");
  return 0;
}


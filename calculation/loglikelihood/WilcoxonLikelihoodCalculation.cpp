//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "WilcoxonLikelihoodCalculation.h"

/** constructor */
WilcoxonLikelihoodCalculator::WilcoxonLikelihoodCalculator(count_t tTotalCases, measure_t tTotalMeasure)
                              :AbstractLikelihoodCalculator(tTotalCases, tTotalMeasure) {}

/** destructor */
WilcoxonLikelihoodCalculator::~WilcoxonLikelihoodCalculator() {}

/** calculates the log likelihood given the number of observed and expected cases */
double WilcoxonLikelihoodCalculator::CalcLogLikelihood(count_t n, measure_t u) {
  ZdGenerateException("CalcLogLikelihood() not implementated.","WilcoxonLikelihoodCalculator");
  return 0;
}

/** calculates the Poisson log likelihood ratio given the number of observed and expected cases */
double WilcoxonLikelihoodCalculator::CalcLogLikelihoodRatio(count_t n, measure_t u, count_t N, measure_t U) {
  ZdGenerateException("CalcLogLikelihoodRatio() not implementated.","WilcoxonLikelihoodCalculator");
  return 0;
}

/** returns log likelihood for total */
double WilcoxonLikelihoodCalculator::GetLogLikelihoodForTotal() const {
  ZdGenerateException("GetLogLikelihoodForTotal() not implementated.","WilcoxonLikelihoodCalculator");
  return 0;
}
 

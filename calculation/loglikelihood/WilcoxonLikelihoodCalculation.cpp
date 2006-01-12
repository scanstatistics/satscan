//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "WilcoxonLikelihoodCalculation.h"

/** constructor */
WilcoxonLikelihoodCalculator::WilcoxonLikelihoodCalculator(const CSaTScanData& Data)
                              :AbstractLikelihoodCalculator(Data) {}

/** destructor */
WilcoxonLikelihoodCalculator::~WilcoxonLikelihoodCalculator() {}

/** calculates the log likelihood given the number of observed and expected cases - not implemented - throws exception.*/
double WilcoxonLikelihoodCalculator::CalcLogLikelihood(count_t n, measure_t u) const {
  ZdGenerateException("CalcLogLikelihood() not implementated.","WilcoxonLikelihoodCalculator");
  return 0;
}

/** calculates the Poisson log likelihood ratio given the number of observed and expected cases - not implemented - throws exception. */
double WilcoxonLikelihoodCalculator::CalcLogLikelihoodRatio(count_t n, measure_t u, count_t N, measure_t U) const {
  ZdGenerateException("CalcLogLikelihoodRatio() not implementated.","WilcoxonLikelihoodCalculator");
  return 0;
}

/** returns log likelihood for total - not implemented - throws exception. */
double WilcoxonLikelihoodCalculator::GetLogLikelihoodForTotal() const {
  ZdGenerateException("GetLogLikelihoodForTotal() not implementated.","WilcoxonLikelihoodCalculator");
  return 0;
}

/** Returns log likelihood ratio given passed log likelihood - not implemented - throws exception.  */
double WilcoxonLikelihoodCalculator::GetLogLikelihoodRatio(double dLogLikelihood) const {
  ZdGenerateException("GetLogLikelihoodRatio() not implementated.","WilcoxonLikelihoodCalculator");
  return 0;
}


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

/** calculates the log likelihood given the number of observed and expected cases */
double NormalLikelihoodCalculator::CalcLogLikelihood(count_t n, measure_t u) const {
  ZdGenerateException("CalcLogLikelihood() not implementated.","NormalLikelihoodCalculator");
  return 0;
}

/** calculates the Poisson log likelihood ratio given the number of observed and expected cases */
double NormalLikelihoodCalculator::CalcLogLikelihoodRatio(count_t n, measure_t u, count_t N, measure_t U) const {
  ZdGenerateException("CalcLogLikelihoodRatio() not implementated.","NormalLikelihoodCalculator");
  return 0;
}

/** calculates the Poisson log likelihood ratio given the number of observed and expected cases, and second measure */
double NormalLikelihoodCalculator::CalcLogLikelihoodRatioEx(count_t tCases, measure_t tMeasure, measure_t tMeasure2, count_t tTotalCases, measure_t tTotalMeasure) const {
  ZdGenerateException("CalcLogLikelihoodRatioEx() not implementated.","NormalLikelihoodCalculator");
  return 0;
}

/** returns log likelihood for total */
double NormalLikelihoodCalculator::GetLogLikelihoodForTotal() const {
  ZdGenerateException("GetLogLikelihoodForTotal() not implementated.","NormalLikelihoodCalculator");
  return 0;
}
 

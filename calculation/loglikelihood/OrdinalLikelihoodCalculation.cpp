//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "OrdinalLikelihoodCalculation.h"

/** constructor */
OrdinalLikelihoodCalculator::OrdinalLikelihoodCalculator(const CSaTScanData& Data)
                            :AbstractLikelihoodCalculator(Data) {srand(1234567);}

/** destructor */
OrdinalLikelihoodCalculator::~OrdinalLikelihoodCalculator() {}

/** calculates the log likelihood given the number of observed and expected cases */
double OrdinalLikelihoodCalculator::CalcLogLikelihood(count_t n, measure_t u) const {
  ZdGenerateException("CalcLogLikelihood() not implementated.","OrdinalLikelihoodCalculator");
  return 0;
}

/** calculates the Poisson log likelihood ratio given the number of observed and expected cases */
double OrdinalLikelihoodCalculator::CalcLogLikelihoodRatio(count_t n, measure_t u, count_t N, measure_t U) const {
  ZdGenerateException("CalcLogLikelihoodRatio() not implementated.","OrdinalLikelihoodCalculator");
  return 0;
}

double OrdinalLikelihoodCalculator::CalcLogLikelihoodRatioOrdinal(const std::vector<count_t>& vOrdinalCases, const std::vector<count_t>& vOrdinalTotalCases) const {

  //$$ This method might have to call a function pointer which is determined by scanning rate. This method will
  //   always be called, with outside method calculating whether acculation is significant; as other models do.
  //   Martin indicated that: for high rates, the order is inorder sequencial (cases in category 1 - category 2 - category 3)
  //                          for low rates, the order is reverse order sequencial (cases in category 3 - category 2 - category 1)
  //                          for both rates, take the maximum of high and low rates
  return (rand() % 100) * .025;
}

/** returns log likelihood for total */
double OrdinalLikelihoodCalculator::GetLogLikelihoodForTotal() const {
  ZdGenerateException("GetLogLikelihoodForTotal() not implementated.","OrdinalLikelihoodCalculator");
  return 0;
}



//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "BernoulliLikelihoodCalculation.h"
#include "PurelySpatialMonotoneCluster.h"

/** constructor */
BernoulliLikelihoodCalculator::BernoulliLikelihoodCalculator(const CSaTScanData& Data)
                              :AbstractLikelihoodCalculator(Data) {}

/** destructor */
BernoulliLikelihoodCalculator::~BernoulliLikelihoodCalculator() {}

/** calculates the Bernoulli log likelihood given the number of observed and expected cases */
double BernoulliLikelihoodCalculator::CalcLogLikelihood(count_t n, measure_t u) const {
  count_t   N = gtTotalCasesInDataSet;
  measure_t U = gtTotalMeasureInDataSet;

  double    nLL_A = 0.0;
  double    nLL_B = 0.0;
  double    nLL_C = 0.0;
  double    nLL_D = 0.0;

  if (n != 0)
    nLL_A = n*log(n/u);
  if (n != u)
    nLL_B = (u-n)*log(1-(n/u));
  if (N-n != 0)
    nLL_C = (N-n)*log((N-n)/(U-u));
   if (N-n != U-u)
    nLL_D = ((U-u)-(N-n))*log(1-((N-n)/(U-u)));

  return nLL_A + nLL_B + nLL_C + nLL_D;
}

/** calculates the Bernoulli log likelihood ratio given the number of observed and expected cases */
double BernoulliLikelihoodCalculator::CalcLogLikelihoodRatio(count_t n, measure_t u, count_t N, measure_t U) const {
  double    dLogLikelihood;
  double    nLL_A = 0.0;
  double    nLL_B = 0.0;
  double    nLL_C = 0.0;
  double    nLL_D = 0.0;

  // calculate the loglikelihood
  if (n != 0)
    nLL_A = n*log(n/u);
  if (n != u)
    nLL_B = (u-n)*log(1-(n/u));
  if (N-n != 0)
    nLL_C = (N-n)*log((N-n)/(U-u));
   if (N-n != U-u)
    nLL_D = ((U-u)-(N-n))*log(1-((N-n)/(U-u)));

  dLogLikelihood = nLL_A + nLL_B + nLL_C + nLL_D;

  // return the logliklihood ratio (loglikelihood - loglikelihood for total)
  return dLogLikelihood - (N*log(N/U) + (U-N)*log((U-N)/U));
}

/** returns log likelihood for total */
double BernoulliLikelihoodCalculator::GetLogLikelihoodForTotal() const {
  count_t   N = gtTotalCasesInDataSet;
  measure_t U = gtTotalMeasureInDataSet;
  
  return (N*log(N/U) + (U-N)*log((U-N)/U));
}

/** calculates loglikelihood ratio for purely spatial monotone analysis given passed cluster */
double BernoulliLikelihoodCalculator::CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster) const {
  double    nLogLikelihood = 0;
  count_t   n;
  measure_t u;

  for (int i=0; i < PSMCluster.m_nSteps; ++i) {
     n = PSMCluster.m_pCasesList[i];
     u = PSMCluster.m_pMeasureList[i];
     if (n != 0  && n != u)
       nLogLikelihood += n*log(n/u) + (u-n)*log(1-(n/u));
     else if (n == 0)
       nLogLikelihood += (u-n)*log(1-(n/u));
     else if (n == u)
       nLogLikelihood += n*log(n/u);
  }

  return nLogLikelihood;
}


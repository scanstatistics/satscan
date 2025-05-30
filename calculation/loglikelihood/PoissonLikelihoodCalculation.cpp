//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "PoissonLikelihoodCalculation.h"
#include "PurelySpatialMonotoneCluster.h"
#include "SVTTCluster.h"

/** constructor */
PoissonLikelihoodCalculator::PoissonLikelihoodCalculator(const CSaTScanData& DataHub)
                            :AbstractLikelihoodCalculator(DataHub), gParameters(DataHub.GetParameters()) {
  //store data set loglikelihoods under null
  for (size_t t=0; t < DataHub.GetDataSetHandler().GetNumDataSets(); ++t) {
    count_t   N = DataHub.GetDataSetHandler().GetDataSet(t).getTotalCases();
    measure_t U = DataHub.GetDataSetHandler().GetDataSet(t).getTotalMeasure();
    gvDataSetLogLikelihoodUnderNull.push_back((N*log(N/U)));
  }
  _time_stratified = DataHub.GetParameters().GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION;
}

/** destructor */
PoissonLikelihoodCalculator::~PoissonLikelihoodCalculator() {}

/** calculates the Poisson log likelihood given the number of observed and expected cases
    - the total cases and expected cases used are that of first data set */
double PoissonLikelihoodCalculator::CalcLogLikelihood(count_t n, measure_t u) const {
  count_t   N = gvDataSetTotals[0].first;
  measure_t U = gvDataSetTotals[0].second;

   if (n != N && n != 0)
     return n*log(n/u) + (N-n)*log((N-n)/(U-u));
   else if (n == 0)
     return (N-n) * log((N-n)/(U-u));
   else
     return n*log(n/u);
}

/** Calculates the Poisson log likelihood ratio given the number of observed and expected cases in time interval. */
double PoissonLikelihoodCalculator::CalcLogLikelihoodTimeStratified(count_t n, measure_t u, count_t N, measure_t U) const {
    double dLogLikelihood;
    if (n != N && n != 0)
        dLogLikelihood = n*log(n / u) + (N - n)*log((N - n) / (U - u));
    else if (n == 0)
        dLogLikelihood = (N - n) * log((N - n) / (U - u));
    else
        dLogLikelihood = n*log(n / u) - N*log(N / U);
    // return the logliklihood ratio (loglikelihood - loglikelihood for total in window)
    return dLogLikelihood - N*log(N / U);
}

/** calculates the Poisson log likelihood ratio given the number of observed, expected cases and data set index. */
double PoissonLikelihoodCalculator::CalcLogLikelihoodRatio(count_t n, measure_t u, size_t tSetIndex) const {
  count_t   N = gvDataSetTotals[tSetIndex].first;
  measure_t U = gvDataSetTotals[tSetIndex].second;
  double    dLogLikelihood;

  // calculate the loglikelihood
   if (n != N && n != 0)
     dLogLikelihood = n*log(n/u) + (N-n)*log((N-n)/(U-u));
   else if (n == 0)
     dLogLikelihood = (N-n) * log((N-n)/(U-u));
   else
     dLogLikelihood = n*log(n/u);

  // return the logliklihood ratio (loglikelihood - loglikelihood for total)
  return dLogLikelihood - (gvDataSetLogLikelihoodUnderNull[tSetIndex]);
}

/** needs documentation */
double PoissonLikelihoodCalculator::CalcMonotoneLogLikelihood(tract_t tSteps, const std::vector<count_t>& vCasesList, const std::vector<measure_t>& vMeasureList) const {
  double nLogLikelihood=0;

  for (int i=0; i < tSteps; i++) {
     if (vCasesList.at(i) != 0)
       nLogLikelihood += vCasesList.at(i) * log(vCasesList.at(i)/vMeasureList.at(i));
  }
  return nLogLikelihood;
}

/** Calculates the full loglikelihood ratio/test statistic given passed maximizing value and
    data set index. For the Poisson calculator, the maximizing value would be the loglikelihood in
    a particular clustering. If maximizing value equals negative double max value, zero is returned
    as this indicates that no significant maximizing value was calculated. */
double PoissonLikelihoodCalculator::CalculateFullStatistic(double dMaximizingValue, size_t tSetIndex) const {
  if (dMaximizingValue == -std::numeric_limits<double>::max()) return 0.0;
  return dMaximizingValue - (_time_stratified ? 0.0 : gvDataSetLogLikelihoodUnderNull[tSetIndex]);
}

/** Calculates the maximizing value given observed cases, expected cases and data set index.
    For the Poisson calculator, the maximizing value is the loglikelihood. */
double PoissonLikelihoodCalculator::CalculateMaximizingValue(count_t n, measure_t u, size_t tDataSetIndex) const {
  count_t   N = gvDataSetTotals[tDataSetIndex].first;
  measure_t U = gvDataSetTotals[tDataSetIndex].second;

  if (n != N && n != 0)
    return n*log(n/u) + (N-n)*log((N-n)/(U-u));
  else if (n == 0)
    return (N-n) * log((N-n)/(U-u));
  else
    return n*log(n/u);
}

/** returns log likelihood for total data set at index */
double PoissonLikelihoodCalculator::GetLogLikelihoodForTotal(size_t tSetIndex) const {
  return gvDataSetLogLikelihoodUnderNull[tSetIndex];
}


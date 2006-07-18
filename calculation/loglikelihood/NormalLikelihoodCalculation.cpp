//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "NormalLikelihoodCalculation.h"
#include "SaTScanData.h"

/** constructor */
NormalLikelihoodCalculator::NormalLikelihoodCalculator(const CSaTScanData& DataHub)
                           :AbstractLikelihoodCalculator(DataHub) {
  //pre-calculate each data sets loglikelihood under the null
  for (size_t t=0; t < DataHub.GetDataSetHandler().GetNumDataSets(); ++t)
     gvDataSetMeasureSqTotals.push_back(DataHub.GetDataSetHandler().GetDataSet(t).GetTotalMeasureSq());
}

/** destructor */
NormalLikelihoodCalculator::~NormalLikelihoodCalculator() {}

/** Calculates the full loglikelihood ratio/test statistic given passed maximizing value and
    data set index. For the Normal calculator, the maximizing value would be the negative variance in
    a particular clustering. If maximizing value equals negative double max value, zero is returned
    as this indicates that no significant maximizing value was calculated. */
double NormalLikelihoodCalculator::CalculateFullStatistic(double dMaximizingValue, size_t tSetIndex) const {
  if (dMaximizingValue == -std::numeric_limits<double>::max()) return 0.0;
  count_t   N = gvDataSetTotals[tSetIndex].first;
  measure_t U = gvDataSetTotals[tSetIndex].second;
  measure_t U2 = gvDataSetMeasureSqTotals[tSetIndex];
  return -1 * N * std::log(std::sqrt((-1 * dMaximizingValue))) + N * std::log(std::sqrt(U2/N - std::pow(U/N, 2)));
}

/** Calculates the maximizing value given observed cases, expected cases, expected cases squared and data set index.
    For the Normal calculator, the maximizing value is the negative variance. */
double NormalLikelihoodCalculator::CalculateMaximizingValueNormal(count_t n, measure_t u, measure_t u2, size_t tSetIndex) const {
  count_t   N = gvDataSetTotals[tSetIndex].first;
  measure_t U = gvDataSetTotals[tSetIndex].second;
  measure_t U2 = gvDataSetMeasureSqTotals[tSetIndex];
  if (!(N - n)/*when the cluster contains all the cases in set*/ || n == 0) return -std::numeric_limits<double>::max();
  double dEstimatedMeanInside = u/n;
  double dEstimatedMeanOutside = (U - u)/(N - n);
  double dEstimatedVariance = (1.0/N) * (u2 - 2.0 * u * dEstimatedMeanInside + n * std::pow(dEstimatedMeanInside, 2) +
                              (U2 - u2) - 2.0 * (U - u) * dEstimatedMeanOutside + (N - n) * std::pow(dEstimatedMeanOutside, 2));
  return (dEstimatedVariance ? -1 * dEstimatedVariance : -std::numeric_limits<double>::max());
}

/** calculates the Normal log likelihood ratio given the number of observed, expected cases, and expected cases squared */
double NormalLikelihoodCalculator::CalcLogLikelihoodRatioNormal(count_t n, measure_t u, measure_t u2, size_t tSetIndex) const {
  count_t   N = gvDataSetTotals[tSetIndex].first;
  measure_t U = gvDataSetTotals[tSetIndex].second;
  measure_t U2 = gvDataSetMeasureSqTotals[tSetIndex];
  if (!(N - n)/*when the cluster contains all the cases in set, ratio is zero*/ || n == 0) return 0;
  double dEstimatedMeanInside = u/n;
  double dEstimatedMeanOutside = (U - u)/(N - n);
  double dEstimatedVariance = (1.0/N) * (u2 - 2.0 * u * dEstimatedMeanInside + n * std::pow(dEstimatedMeanInside, 2) +
                              (U2 - u2) - 2.0 * (U - u) * dEstimatedMeanOutside + (N - n) * std::pow(dEstimatedMeanOutside, 2));
  if (dEstimatedVariance > 0)
    return -1 * N * std::log(std::sqrt(dEstimatedVariance)) + N * std::log(std::sqrt(U2/N - std::pow(U/N, 2)));
  else
    return N * std::log(std::sqrt(U2/N - std::pow(U/N, 2)));
}


//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "BernoulliLikelihoodCalculation.h"
#include "PurelySpatialMonotoneCluster.h"

/** constructor */
BernoulliLikelihoodCalculator::BernoulliLikelihoodCalculator(const CSaTScanData& DataHub) : 
    AbstractLikelihoodCalculator(DataHub), _pt_counts_nc(0), _pt_measure_nc(0), _pp_counts(0), _pp_measure(0) {
    if (DataHub.GetParameters().GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION) {
        // With the time stratified adjustment, we need specialized behavior when calculating the full statistic.
        // We're already maximizing on the full statistic, so the call to CalculateFullStatistic is specialized.
        _is_time_stratified = DataHub.GetParameters().GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION;
        // store loglikelihoods under null per time interval ** assuming only one data set at this point **
        _pt_counts_nc = DataHub.GetDataSetHandler().GetDataSet().getCaseData_PT_NC();
        _pt_measure_nc = DataHub.GetDataSetHandler().GetDataSet().getMeasureData_PT_NC();
        count_t N;
        measure_t U;
        for (size_t t = 0; t < DataHub.GetNumTimeIntervals(); ++t) {
            N = _pt_counts_nc[t];
            U = _pt_measure_nc[t];
            gvDataSetLogLikelihoodUnderNull.push_back((N * log(N / U) + (U - N) * log((U - N) / U)));
        }
    } else if (DataHub.GetParameters().GetSpatialAdjustmentType() != SPATIAL_STRATIFIED_RANDOMIZATION) {
        // With the spatial stratified adjustment, we need specialized behavior when calculating the full statistic.
        // We're already maximizing on the full statistic, so the call to CalculateFullStatistic is specialized.
        _is_time_stratified = DataHub.GetParameters().GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION;
        // store loglikelihoods under null per tract ** assuming only one data set at this point **
        _pp_counts = DataHub.GetDataSetHandler().GetDataSet().getCaseData().GetArray();
        _pp_measure = DataHub.GetDataSetHandler().GetDataSet().getMeasureData().GetArray();
        count_t N;
        measure_t U;
        for (size_t t = 0; t < DataHub.GetNumTracts(); ++t) {
            N = _pp_counts[0][t];
            U = _pp_measure[0][t];
            gvDataSetLogLikelihoodUnderNull.push_back((N * log(N / U) + (U - N) * log((U - N) / U)));
        }
    } else {
        // store loglikelihoods under null per data stream
        for (size_t t = 0; t < DataHub.GetDataSetHandler().GetNumDataSets(); ++t) {
            count_t   N = DataHub.GetDataSetHandler().GetDataSet(t).getTotalCases();
            measure_t U = DataHub.GetDataSetHandler().GetDataSet(t).getTotalMeasure();
            gvDataSetLogLikelihoodUnderNull.push_back((N*log(N / U) + (U - N)*log((U - N) / U)));
        }
    }
}

/** destructor */
BernoulliLikelihoodCalculator::~BernoulliLikelihoodCalculator() {}

/** calculates the Bernoulli log likelihood given the number of observed and expected cases
    - the total cases and expected cases used are that of first data set */
double BernoulliLikelihoodCalculator::CalcLogLikelihood(count_t n, measure_t u) const {
  count_t   N = gvDataSetTotals[0].first;
  measure_t U = gvDataSetTotals[0].second;

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

/** calculates the Bernoulli log likelihood ratio given the number of observed, expected cases and data set index*/
double BernoulliLikelihoodCalculator::CalcLogLikelihoodRatio(count_t n, measure_t u, size_t tSetIndex) const {
  count_t   N = gvDataSetTotals[tSetIndex].first;
  measure_t U = gvDataSetTotals[tSetIndex].second;
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
  return dLogLikelihood - (gvDataSetLogLikelihoodUnderNull[tSetIndex]);
}

/** Calculates the Bernoulli log likelihood ratio given the number of observed and expected cases in time interval. */
double BernoulliLikelihoodCalculator::CalcLogLikelihoodBernoulliTimeStratified(count_t n, measure_t u, int interval) const {
    double    nLL_A = 0.0;
    double    nLL_B = 0.0;
    double    nLL_C = 0.0;
    double    nLL_D = 0.0;

    // calculate the loglikelihood
    count_t N = _pt_counts_nc[interval]; // total cases for time interval
    measure_t U = _pt_measure_nc[interval];  // total measure for time interval
    if (n != 0)
        nLL_A = n*log(n / u);
    if (n != u)
        nLL_B = (u - n)*log(1 - (n / u));
    if (N - n != 0)
        nLL_C = (N - n)*log((N - n) / (U - u));
    if (N - n != U - u)
        nLL_D = ((U - u) - (N - n))*log(1 - ((N - n) / (U - u)));

    double dLogLikelihood = nLL_A + nLL_B + nLL_C + nLL_D;

    // return the logliklihood ratio (loglikelihood - loglikelihood for total)
    return dLogLikelihood - gvDataSetLogLikelihoodUnderNull[interval];
}

/** Calculates the Bernoulli log likelihood ratio given the number of observed and expected cases for tract. */
double BernoulliLikelihoodCalculator::CalcLogLikelihoodBernoulliSpatialStratified(count_t n, measure_t u, tract_t tract) const {
    double    nLL_A = 0.0;
    double    nLL_B = 0.0;
    double    nLL_C = 0.0;
    double    nLL_D = 0.0;

    // calculate the loglikelihood
    count_t N = _pp_counts[0][tract]; // total cases for tract
    measure_t U = _pp_measure[0][tract]; // total measure for tract
    if (n != 0)
        nLL_A = n*log(n / u);
    if (n != u)
        nLL_B = (u - n)*log(1 - (n / u));
    if (N - n != 0)
        nLL_C = (N - n)*log((N - n) / (U - u));
    if (N - n != U - u)
        nLL_D = ((U - u) - (N - n))*log(1 - ((N - n) / (U - u)));

    double dLogLikelihood = nLL_A + nLL_B + nLL_C + nLL_D;

    // return the logliklihood ratio (loglikelihood - loglikelihood for total)
    return dLogLikelihood - gvDataSetLogLikelihoodUnderNull[tract];
}

/** calculates loglikelihood ratio for purely spatial monotone analysis given passed cluster */
double BernoulliLikelihoodCalculator::CalcMonotoneLogLikelihood(tract_t tSteps, const std::vector<count_t>& vCasesList, const std::vector<measure_t>& vMeasureList) const {
  double    nLogLikelihood = 0;
  count_t   n;
  measure_t u;

  for (int i=0; i < tSteps; ++i) {
     n = vCasesList.at(i);
     u = vMeasureList.at(i);
     if (n != 0  && n != u)
       nLogLikelihood += n*log(n/u) + (u-n)*log(1-(n/u));
     else if (n == 0)
       nLogLikelihood += (u-n)*log(1-(n/u));
     else if (n == u)
       nLogLikelihood += n*log(n/u);
  }

  return nLogLikelihood;
}

/** Calculates the full loglikelihood ratio/test statistic given passed maximizing value and
    data set index. For the Bernoulli calculator, the maximizing value would be the loglikelihood in
    a particular clustering. If maximizing value equals negative double max value, zero is returned
    as this indicates that no significant maximizing value was calculated. */
double BernoulliLikelihoodCalculator::CalculateFullStatistic(double dMaximizingValue, size_t tSetIndex) const {
  if (dMaximizingValue == -std::numeric_limits<double>::max()) return 0.0;
  return dMaximizingValue - (_is_time_stratified ? 0.0 : gvDataSetLogLikelihoodUnderNull[tSetIndex]);
}

/** Calculates the maximizing value given observed cases, expected cases and data set index.
    For the Bernoulli calculator, the maximizing value is the loglikelihood. */
double BernoulliLikelihoodCalculator::CalculateMaximizingValue(count_t n, measure_t u, size_t tDataSetIndex) const {
  count_t   N = gvDataSetTotals[tDataSetIndex].first;
  measure_t U = gvDataSetTotals[tDataSetIndex].second;
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

/** returns log likelihood for total data set at index */
double BernoulliLikelihoodCalculator::GetLogLikelihoodForTotal(size_t tSetIndex) const {
  return gvDataSetLogLikelihoodUnderNull[tSetIndex];
}


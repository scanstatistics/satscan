//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "WeightedNormalLikelihoodCalculation.h"
#include "SaTScanData.h"
#include "WeightedNormalRandomizer.h"

/** constructor */
WeightedNormalLikelihoodCalculator::WeightedNormalLikelihoodCalculator(const CSaTScanData& DataHub)
                           :AbstractLikelihoodCalculator(DataHub) {

   try {
     const AbstractWeightedNormalRandomizer *pRandomizer;
     for (size_t t=0; t < DataHub.GetDataSetHandler().GetNumDataSets(); ++t) {
       if ((pRandomizer = dynamic_cast<const AbstractWeightedNormalRandomizer*>(DataHub.GetDataSetHandler().GetRandomizer(t))) == 0)
          throw prg_error("Randomizer could not be dynamically casted to AbstractWeightedNormalRandomizer type.\n", "constructor()");
       gvDataSetConstants.push_back(std::make_pair(pRandomizer->getFirstRatioConstant(), pRandomizer->getSecondRatioConstant()));
     }
   }
  catch (prg_exception& x) {
    x.addTrace("constructor()","WeightedNormalLikelihoodCalculator");
    throw;
  }
}

/** destructor */
WeightedNormalLikelihoodCalculator::~WeightedNormalLikelihoodCalculator() {}

/** Calculates the full loglikelihood ratio/test statistic given passed maximizing value and
    data set index. For the Normal calculator, the maximizing value would be the negative variance in
    a particular clustering. If maximizing value equals negative double max value, zero is returned
    as this indicates that no significant maximizing value was calculated. */
double WeightedNormalLikelihoodCalculator::CalculateFullStatistic(double dMaximizingValue, size_t tSetIndex) const {
  //return -0.5 * gvDataSetTotals[tSetIndex].first * log(gvDataSetConstants[tSetIndex].first - dMaximizingValue) + gvDataSetConstants[tSetIndex].second;
  return 0.5 * gvDataSetTotals[tSetIndex].first * log(gvDataSetConstants[tSetIndex].first - dMaximizingValue) + gvDataSetConstants[tSetIndex].second;
}

/** Calculates the maximizing value given observed cases, expected cases, expected cases squared and data set index.
    For the Normal calculator, the maximizing value is the negative variance. */
double WeightedNormalLikelihoodCalculator::CalculateMaximizingValueNormal(count_t n, measure_t u, measure_t u2, size_t tSetIndex) const {
  count_t   N = gvDataSetTotals[tSetIndex].first;
  measure_t U = gvDataSetTotals[tSetIndex].second;
  measure_t U2 = gvDataSetMeasureAuxTotals[tSetIndex];

  if (!(N - n)/*when the cluster contains all the cases in set*/ || n == 0) return -std::numeric_limits<double>::max();

  return std::pow(u, 2)/u2 + std::pow(U-u, 2)/(U2-u2);
}

/** calculates the Normal log likelihood ratio given the number of observed, expected cases, and expected cases squared */
double WeightedNormalLikelihoodCalculator::CalcLogLikelihoodRatioNormal(count_t n, measure_t u, measure_t u2, size_t tSetIndex) const {
  count_t   N = gvDataSetTotals[tSetIndex].first;
  measure_t U = gvDataSetTotals[tSetIndex].second;
  measure_t U2 = gvDataSetMeasureAuxTotals[tSetIndex];

  if (!(N - n)/*when the cluster contains all the cases in set*/ || n == 0) return -std::numeric_limits<double>::max();
  return -0.5 * N * log(gvDataSetConstants[tSetIndex].first - (std::pow(u, 2)/u2 + std::pow(U-u, 2)/(U2-u2))) + gvDataSetConstants[tSetIndex].second;
}


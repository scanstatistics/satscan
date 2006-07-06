//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "OrdinalLikelihoodCalculation.h"
#include "SaTScanData.h"

/////////////////////////////////////////
/////// OrdinalCombinedCategory /////////
/////////////////////////////////////////

/** constructor */
OrdinalCombinedCategory::OrdinalCombinedCategory(int iInitialCategoryIndex) {
  Combine(iInitialCategoryIndex);
}

/** destructor */
OrdinalCombinedCategory::~OrdinalCombinedCategory() {}

/** Adds category as combined with existing categores. */
void OrdinalCombinedCategory::Combine(int iCategoryIndex) {
  if (std::find(gvCatorgiesIndexes.begin(), gvCatorgiesIndexes.end(), iCategoryIndex) == gvCatorgiesIndexes.end())
    gvCatorgiesIndexes.push_back(iCategoryIndex);
}

/** Returns index of combined category at index. */
int OrdinalCombinedCategory::GetCategoryIndex(size_t tCatgoryPositionIndex) const {
  try {
    if (!gvCatorgiesIndexes.size() || tCatgoryPositionIndex > gvCatorgiesIndexes.size() - 1)
      ZdGenerateException("Index %u out of range [size=%u].","GetCategoryIndex()", tCatgoryPositionIndex, gvCatorgiesIndexes.size());
  }
  catch (ZdException &x) {
    x.AddCallpath("GetCategoryIndex()","OrdinalCombinedCategory");
    throw;
  }
  return gvCatorgiesIndexes[tCatgoryPositionIndex];
}

/////////////////////////////////////////
/////// OrdinalLikelihoodCalculator /////
/////////////////////////////////////////

/** constructor */
OrdinalLikelihoodCalculator::OrdinalLikelihoodCalculator(const CSaTScanData& DataHub)
                            :AbstractLikelihoodCalculator(DataHub),
                             gbScanLowRates(DataHub.GetParameters().GetExecuteScanRateType() == LOW ||
                                            DataHub.GetParameters().GetExecuteScanRateType() == HIGHANDLOW),
                             gbScanHighRates(DataHub.GetParameters().GetExecuteScanRateType() == HIGH ||
                                             DataHub.GetParameters().GetExecuteScanRateType() == HIGHANDLOW) {
  double        LL0;
  size_t        tMaxNumCategories=0;

  // assemble total number of cases in each ordinal category for each data set and
  // calculate loglikelihood under null for each data set
  for (size_t t=0; t < DataHub.GetDataSetHandler().GetNumDataSets(); ++t) {
    const PopulationData& Population = DataHub.GetDataSetHandler().GetDataSet(t).GetPopulationData();
    gvDataSetTotalCasesPerCategory.push_back(std::vector<double>());
    LL0 = 0;
    for (size_t i=0; i < Population.GetNumOrdinalCategories(); ++i) {
       count_t tCategoryCases = Population.GetNumOrdinalCategoryCases(i);
       // add number of category cases to accumulation
       gvDataSetTotalCasesPerCategory.back().push_back(tCategoryCases);
       // add this categories contribution to loglikelihood under null to LL0
       if (tCategoryCases)
         LL0 += (double)tCategoryCases * log((double)tCategoryCases/(double)DataHub.GetDataSetHandler().GetDataSet(t).GetTotalCases());
    }
    // set loglikelihood under null for this data set
    gvDataSetLogLikelihoodUnderNull.push_back(LL0);
    tMaxNumCategories = std::max(tMaxNumCategories, Population.GetNumOrdinalCategories()); 
  }
  // resize P and Q vectors to maximum number of categories, considering all data sets - so we don't need to resize during calculation
  gvP.resize(tMaxNumCategories, 0);
  gvQ.resize(tMaxNumCategories, 0);
}

/** destructor */
OrdinalLikelihoodCalculator::~OrdinalLikelihoodCalculator() {}

/** Calculates loglikelihood ratio for ordinal data of data set 'tSetIndex', defined in vOrdinalCases.
    When scannnig for high and low rates, returns the maximum of low and high ratios. Returns zero if
    llr can not be calculated. */
double OrdinalLikelihoodCalculator::CalcLogLikelihoodRatioOrdinal(const std::vector<count_t>& vOrdinalCases, size_t tSetIndex) const {
  double lowLLR=0, highLLR=0;
  bool   bLoglikelihoodRatioCalculated=false;

  if (gbScanHighRates && CalculateProbabilitiesInsideAndOutsideOfCluster(vOrdinalCases,
                                                                         gvDataSetTotalCasesPerCategory[tSetIndex],
                                                                         &OrdinalLikelihoodCalculator::CompareRatioForHighScanningArea)) {
    highLLR = CalculateLogLikelihood(vOrdinalCases, tSetIndex) - gvDataSetLogLikelihoodUnderNull[tSetIndex];
    bLoglikelihoodRatioCalculated = true;
  }
  if (gbScanLowRates && CalculateProbabilitiesInsideAndOutsideOfCluster(vOrdinalCases,
                                                                        gvDataSetTotalCasesPerCategory[tSetIndex],
                                                                        &OrdinalLikelihoodCalculator::CompareRatioForLowScanningArea)) {
    lowLLR = CalculateLogLikelihood(vOrdinalCases, tSetIndex) - gvDataSetLogLikelihoodUnderNull[tSetIndex];
    highLLR = (bLoglikelihoodRatioCalculated ? std::max(lowLLR, highLLR) : lowLLR);
    bLoglikelihoodRatioCalculated = true;
  }

  // return calculated loglikelihood ratio
  return (bLoglikelihoodRatioCalculated ? highLLR : 0);
}

/** Calculates loglikelihood ratio for ordinal data of data set 'tSetIndex', defined in vOrdinalCases;
    scannnig for high rates. Returns zero if llr can not be calculated. */
double OrdinalLikelihoodCalculator::CalcLogLikelihoodRatioOrdinalHighRate(const std::vector<count_t>& vOrdinalCases, size_t tSetIndex) const {
  if (CalculateProbabilitiesInsideAndOutsideOfCluster(vOrdinalCases,
                                                      gvDataSetTotalCasesPerCategory[tSetIndex],
                                                      &OrdinalLikelihoodCalculator::CompareRatioForHighScanningArea))
    // return calculated loglikelihood ratio
    return CalculateLogLikelihood(vOrdinalCases, tSetIndex) - gvDataSetLogLikelihoodUnderNull[tSetIndex];

  // return zero - default if not calculated
  return 0;  
}

/** Calculates loglikelihood ratio for ordinal data of data set 'tSetIndex', defined in vOrdinalCases;
    scannnig for low rates. Returns zero if llr can not be calculated. */
double OrdinalLikelihoodCalculator::CalcLogLikelihoodRatioOrdinalLowRate(const std::vector<count_t>& vOrdinalCases, size_t tSetIndex) const {
  if (CalculateProbabilitiesInsideAndOutsideOfCluster(vOrdinalCases,
                                                      gvDataSetTotalCasesPerCategory[tSetIndex],
                                                      &OrdinalLikelihoodCalculator::CompareRatioForLowScanningArea))
    // return calculated loglikelihood ratio
    return CalculateLogLikelihood(vOrdinalCases, tSetIndex) - gvDataSetLogLikelihoodUnderNull[tSetIndex];
    
  // return zero - default if not calculated
  return 0;  
}

/** Creates a collection of OrdinalCombinedCategory objects, which will be representative
    of how the categories were possibly combined; dictated by calculated probabilities
    inside and outside of cluster. */
void OrdinalLikelihoodCalculator::CalculateCombinedCategories(const std::vector<count_t>& vOrdinalCases, std::vector<OrdinalCombinedCategory>& vOrdinalCategories) const {
  vOrdinalCategories.clear();
  for (size_t k=0; k < vOrdinalCases.size(); ++k) {
     vOrdinalCategories.push_back(OrdinalCombinedCategory(k));
     size_t i = k;
     while (++i < vOrdinalCases.size() &&
         /* both ratios are infinity */   ((!gvQ[k] && !gvQ[i]) ||
         /* ratios are equal */            (gvQ[k] && gvQ[i] && std::fabs(gvP[k]/gvQ[k] - gvP[i]/gvQ[i]) < .0000001))) {
          vOrdinalCategories.back().Combine(i);
          k = i;
     }
  }
}

/** Calculates the full loglikelihood ratio/test statistic given passed maximizing value and
    data set index. For the Ordinal calculator, the maximizing value would be the loglikelihood in
    a particular clustering. If maximizing value equals negative double max value, zero is returned
    as this indicates that no significant maximizing value was calculated. */
double OrdinalLikelihoodCalculator::CalculateFullStatistic(double dMaximizingValue, size_t tSetIndex) const {
  if (dMaximizingValue == -std::numeric_limits<double>::max()) return 0.0;
  return dMaximizingValue - gvDataSetLogLikelihoodUnderNull[tSetIndex];
}

/** Calculates the maximizing value given observed cases, expected cases and data set index.
    For the Ordinal calculator, the maximizing value is the loglikelihood. */
double OrdinalLikelihoodCalculator::CalculateMaximizingValueOrdinal(const std::vector<count_t>& vOrdinalCases, size_t tSetIndex) const {
  double lowMaximizingValue=0, highMaximizingValue=0;
  bool   bMaximizingValue=false;

  if (gbScanHighRates && CalculateProbabilitiesInsideAndOutsideOfCluster(vOrdinalCases,
                                                                         gvDataSetTotalCasesPerCategory[tSetIndex],
                                                                         &OrdinalLikelihoodCalculator::CompareRatioForHighScanningArea)) {
    highMaximizingValue = CalculateLogLikelihood(vOrdinalCases, tSetIndex);
    bMaximizingValue = true;
  }
  if (gbScanLowRates && CalculateProbabilitiesInsideAndOutsideOfCluster(vOrdinalCases,
                                                                        gvDataSetTotalCasesPerCategory[tSetIndex],
                                                                        &OrdinalLikelihoodCalculator::CompareRatioForLowScanningArea)) {
    lowMaximizingValue = CalculateLogLikelihood(vOrdinalCases, tSetIndex);
    highMaximizingValue = (bMaximizingValue ? std::max(lowMaximizingValue, highMaximizingValue) : lowMaximizingValue);
    bMaximizingValue = true;
  }

  // return calculated loglikelihood ratio
  return (bMaximizingValue ? highMaximizingValue : -std::numeric_limits<double>::max());
}

/** Given previously calculated probabilites inside and outside of cluster, calculates loglikelihood. */
double OrdinalLikelihoodCalculator::CalculateLogLikelihood(const std::vector<count_t>& vOrdinalCases, size_t tSetIndex) const {
  double        dLogLikelihood=0;

  for (size_t k=0; k < vOrdinalCases.size(); ++k) {
    //Formula is: vOrdinalCases[k] * log(gvP[k]) + (vOrdinalTotalCases[k] - vOrdinalCases[k]) * log(gvQ[k]),
    //but gvP[k] or gvQ[k] could be zero. So add to loglikelihood in parts, after check for zero.
    if (gvP[k])
      dLogLikelihood += vOrdinalCases[k] * log(gvP[k]);
    if (gvQ[k])
      dLogLikelihood += (gvDataSetTotalCasesPerCategory[tSetIndex][k] - vOrdinalCases[k]) * log(gvQ[k]);
  }
  return dLogLikelihood;
}

/** Given a collection of ordinal category cases known to produce a significant
    log likelihood ratio, re-calculates the log likelihood ratio to determine
    which, if any, categories where combined into one category during process
    of calculating probabilities inside and outside clustering. */
void OrdinalLikelihoodCalculator::CalculateOrdinalCombinedCategories(const std::vector<count_t>& vOrdinalCases,
                                                                     std::vector<OrdinalCombinedCategory>& vOrdinalCategories,
                                                                     size_t tSetIndex) const {
  double        dHighRateLoglikelihoodRatio;
  bool          bEvaluatedHighRates=false;

  vOrdinalCategories.clear();

  //Since we don't know whether the log likelihood ratio for this collection of ordinal cases
  //was produced by scanning for high or low rates (when scanning for both), re-calculate both
  //if necessary and keep combined category information for whichever ratio was greater; keeping
  //that for high should they be equal.
  if (gbScanHighRates && CalculateProbabilitiesInsideAndOutsideOfCluster(vOrdinalCases, gvDataSetTotalCasesPerCategory[tSetIndex], &OrdinalLikelihoodCalculator::CompareRatioForHighScanningArea)) {
    //Calculate log likelihood for high rates - we might need this to compare against that for low rates.
    dHighRateLoglikelihoodRatio = CalculateLogLikelihood(vOrdinalCases, tSetIndex) - gvDataSetLogLikelihoodUnderNull[tSetIndex];
    bEvaluatedHighRates = true;
    CalculateCombinedCategories(vOrdinalCases, vOrdinalCategories);
  }
  if (gbScanLowRates && CalculateProbabilitiesInsideAndOutsideOfCluster(vOrdinalCases, gvDataSetTotalCasesPerCategory[tSetIndex], &OrdinalLikelihoodCalculator::CompareRatioForLowScanningArea))
    if (!bEvaluatedHighRates ||
        CalculateLogLikelihood(vOrdinalCases, tSetIndex) - gvDataSetLogLikelihoodUnderNull[tSetIndex] > dHighRateLoglikelihoodRatio)
      CalculateCombinedCategories(vOrdinalCases, vOrdinalCategories);
}

/** Calculates probabilities inside and outside of clustering.
    gvP[k] is the probability that a case inside the window belongs to category k
    gvQ[k] is the probability that a case outside the window belongs to category k */
bool OrdinalLikelihoodCalculator::CalculateProbabilitiesInsideAndOutsideOfCluster(const std::vector<count_t>& vOrdinalCases, const std::vector<double>& vOrdinalTotalCases, COMPARE_RATIOS_METHOD pCompareMethod) const {
  double       W=0; // total cases inside window
  double       U=0; // total cases outside window
  double       dP, dQ;
  double       dTotalCasesInCategoryRange;
  int          i;

  // calculate total number of cases inside window
  for (size_t k=0; k < vOrdinalCases.size(); ++k)
     W += vOrdinalCases[k];
  if (!W) // return false if no cases to evaluate
     return false;
  // calculate total number of cases outside window
  for (size_t k=0; k < vOrdinalCases.size(); ++k)
     U += vOrdinalTotalCases[k] - vOrdinalCases[k];
  if (!U) // return false if no cases to evaluate
     return false;
  // calculate P and Q for each category
  for (size_t k=0; k < vOrdinalCases.size(); ++k) {
     gvP[k] = (double)vOrdinalCases[k]/W;
     gvQ[k] = ((double)(vOrdinalTotalCases[k] - vOrdinalCases[k]))/U;
     i = k;
     while (--i >= 0 && (this->*pCompareMethod)(gvP[k], gvQ[k], gvP[i], gvQ[i]) ) {
          dTotalCasesInCategoryRange=0;
          for (size_t j=i; j <= k; ++j)
            dTotalCasesInCategoryRange += vOrdinalTotalCases[j];
          dP=0;
          for (size_t j=i; j <= k; ++j)
            dP += vOrdinalCases[j];
          dP /= W;
          dQ=0;
          for (size_t j=i; j <= k; ++j)
            dQ += vOrdinalTotalCases[j] - vOrdinalCases[j];
          dQ /= U;
          if (dTotalCasesInCategoryRange)
            //when a iterative scan is performed, there is a possibility that
            //the number of cases in a ordinal category have become zero
            for (size_t j=i; j <= k; ++j) {
               gvP[j] = dP * (vOrdinalTotalCases[j]/dTotalCasesInCategoryRange);
               gvQ[j] = dQ * (vOrdinalTotalCases[j]/dTotalCasesInCategoryRange);
            }
          else
            for (size_t j=i; j <= k; ++j)
               gvP[j] = gvQ[j] = 0;
     }
  }

// -- bug test --
//  double Sp=0;
//  for (size_t t=0; t < vOrdinalCases.size(); ++t)
//     Sp += gvP[t];
//  if (abs(1 - Sp) > .0001)
//    ZdGenerateException("%lf difference with Ps.", "CalculateProbabilitiesInsideAndOutsideOfCluster()", 1 - Sp);
// double Sq=0;
//  for (size_t t=0; t < vOrdinalCases.size(); ++t)
//     Sq += gvQ[t];
//  if (abs(1 - Sq) > .0001)
//    ZdGenerateException("%lf difference with Qs.", "CalculateProbabilitiesInsideAndOutsideOfCluster()", 1 - Sq);
// -- bug test --

  return true;
}

/** Returns whether ratio of dPk/dQk <= dPi/dQi. */
bool OrdinalLikelihoodCalculator::CompareRatioForHighScanningArea(double dPk, double dQk, double dPi,double dQi) const {
  return (!dQk && !dQi) || // both dPk/dQk and dPi/dQi equal infinity
         (dQk && !dQi)  || // dPi/dQi equals infinity and is greater
         (dQk && dQi && dPk/dQk <= dPi/dQi);
}

/** Returns whether ratio of dPk/dQk >= dPi/dQi. */
bool OrdinalLikelihoodCalculator::CompareRatioForLowScanningArea(double dPk, double dQk, double dPi, double dQi) const {
  return (!dQk && !dQi) || // both dPk/dQk and dPi/dQi equal infinity
         (!dQk && dQi)  || // dPk/dQk equals infinity and is greater
         (dQk && dQi && dPk/dQk >= dPi/dQi);
}


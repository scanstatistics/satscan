//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "OrdinalLikelihoodCalculation.h"
#include "SaTScanData.h"

/** constructor */
OrdinalLikelihoodCalculator::OrdinalLikelihoodCalculator(const CSaTScanData& DataHub)
                            :AbstractLikelihoodCalculator(DataHub),
                             gbScanLowRates(DataHub.GetParameters().GetAreaScanRateType() == LOW ||
                                            DataHub.GetParameters().GetAreaScanRateType() == HIGHANDLOW),
                             gbScanHighRates(DataHub.GetParameters().GetAreaScanRateType() == HIGH ||
                                             DataHub.GetParameters().GetAreaScanRateType() == HIGHANDLOW) {
  double        LL0;
  size_t        tMaxNumCategories=0;

  // assemble total number of cases in each ordinal category for each data set and
  // calculate loglikelihood under null for each data set
  for (size_t t=0; t < DataHub.GetDataSetHandler().GetNumDataSets(); ++t) {
    const PopulationData& Population = DataHub.GetDataSetHandler().GetDataSet(t).GetPopulationData();
    gvDataSetTotalCasesPerCategory.push_back(std::vector<double>());
    LL0 = 0;
    for (size_t i=0; i < Population.GetNumOrdinalCategories(); ++i) {
       // add number of category cases to accumulation
       gvDataSetTotalCasesPerCategory.back().push_back(Population.GetNumOrdinalCategoryCases(i));
       // add this categories contribution to loglikelihood under null to LL0
       LL0 += (double)Population.GetNumOrdinalCategoryCases(i) *
              log((double)Population.GetNumOrdinalCategoryCases(i)/(double)DataHub.GetDataSetHandler().GetDataSet(t).GetTotalCases());
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

/** calculates the log likelihood given the number of observed and expected cases
    - not implemented - throws exception */
double OrdinalLikelihoodCalculator::CalcLogLikelihood(count_t n, measure_t u) const {
  ZdGenerateException("CalcLogLikelihood() not implementated.","OrdinalLikelihoodCalculator");
  return 0;
}

/** calculates the Poisson log likelihood ratio given the number of observed and expected cases 
    - not implemented - throws exception */
double OrdinalLikelihoodCalculator::CalcLogLikelihoodRatio(count_t n, measure_t u, count_t N, measure_t U) const {
  ZdGenerateException("CalcLogLikelihoodRatio() not implementated.","OrdinalLikelihoodCalculator");
  return 0;
}

double OrdinalLikelihoodCalculator::CalcLogLikelihoodRatioOrdinal(const std::vector<count_t>& vOrdinalCases, size_t tSetIndex) const {
  double lowLL=0, LL=0, dRatio=0;
  bool   bLoglikelihoodCalculated=false;

  if (gbScanHighRates && CalculatePandQs(vOrdinalCases, gvDataSetTotalCasesPerCategory[tSetIndex], &OrdinalLikelihoodCalculator::CompareRatioForHighScanningArea)) {
    // calculate loglikelihood
    for (size_t k=0; k < vOrdinalCases.size(); ++k) {
      // Formula is: LL += vOrdinalCases[k] * log(gvP[k]) + (vOrdinalTotalCases[k] - vOrdinalCases[k]) * log(gvQ[k]),
      // but gvP[k] or gvQ[k] could be zero. So add to loglikelihood in parts, after check for zero.
      if (gvP[k])
        LL += vOrdinalCases[k] * log(gvP[k]);
      if (gvQ[k])
        LL += (gvDataSetTotalCasesPerCategory[tSetIndex][k] - vOrdinalCases[k]) * log(gvQ[k]);
    }
    bLoglikelihoodCalculated = true;
  }
  if (gbScanLowRates && CalculatePandQs(vOrdinalCases, gvDataSetTotalCasesPerCategory[tSetIndex], &OrdinalLikelihoodCalculator::CompareRatioForLowScanningArea)) {
    // calculate loglikelihood
    for (size_t k=0; k < vOrdinalCases.size(); ++k) {
      // Formula is: LL += vOrdinalCases[k] * log(gvP[k]) + (vOrdinalTotalCases[k] - vOrdinalCases[k]) * log(gvQ[k]),
      // but gvP[k] or gvQ[k] could be zero. So add to loglikelihood in parts, after check for zero.
      if (gvP[k])
        lowLL += vOrdinalCases[k] * log(gvP[k]);
      if (gvQ[k])
        lowLL += (gvDataSetTotalCasesPerCategory[tSetIndex][k] - vOrdinalCases[k]) * log(gvQ[k]);
    }
    LL = (bLoglikelihoodCalculated ? std::max(lowLL, LL) : lowLL);
    bLoglikelihoodCalculated = true;
  }

  // return calculated loglikelihood ratio
  return (bLoglikelihoodCalculated ? LL - gvDataSetLogLikelihoodUnderNull[tSetIndex] : 0);
}

/** returns log likelihood for total  - not implemented - throws exception */
double OrdinalLikelihoodCalculator::GetLogLikelihoodForTotal() const {
  ZdGenerateException("GetLogLikelihoodForTotal() not implementated.","OrdinalLikelihoodCalculator");
  return 0;
}

/** Calculates Ps and Qs. Returns indication of whether values could be calculated. */
bool OrdinalLikelihoodCalculator::CalculatePandQs(const std::vector<count_t>& vOrdinalCases, const std::vector<double>& vOrdinalTotalCases, COMPARE_RATIOS_METHOD pCompareMethod) const {
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
     while (--i >= 0 && gvQ[k] && gvQ[i] && (this->*pCompareMethod)(gvP[k]/gvQ[k], gvP[i]/gvQ[i]) ) {
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
          for (size_t j=i; j <= k; ++j) {
             gvP[j] = dP * (vOrdinalTotalCases[j]/dTotalCasesInCategoryRange);
             gvQ[j] = dQ * (vOrdinalTotalCases[j]/dTotalCasesInCategoryRange);
          }
     }
  }

//  double Sp=0;
//  for (size_t t=0; t < vOrdinalCases.size(); ++t)
//     Sp += gvP[t];
//  if (abs(1 - Sp) > .0001)
//    ZdGenerateException("%lf difference with Ps.", "CalculatePandQs()", 1 - Sp);
// double Sq=0;
//  for (size_t t=0; t < vOrdinalCases.size(); ++t)
//     Sq += gvQ[t];
//  if (abs(1 - Sq) > .0001)
//    ZdGenerateException("%lf difference with Qs.", "CalculatePandQs()", 1 - Sq);

  return true;  
}


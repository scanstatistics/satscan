//******************************************************************************
#ifndef __LikelihoodCalculation_H
#define __LikelihoodCalculation_H
//******************************************************************************
#include "SaTScan.h"

class SpatialMonotoneData; /** forward class declaration */
class CSaTScanData;       /** forward class declaration */
class SVTTClusterData;    /** forward class declaration */
class CTimeTrend;         /** forward class declaration */
class AbstractLoglikelihoodRatioUnifier; /** forward class declaration */

/** Abstract interface for which to calculate log likelihoods and log likelihood ratios. */
class AbstractLikelihoodCalculator {
  public:
    typedef bool (AbstractLikelihoodCalculator::*SCANRATE_FUNCPTR) (count_t,measure_t,size_t) const;
    typedef bool (AbstractLikelihoodCalculator::*SCANRATENORMAL_FUNCPTR) (count_t,measure_t,measure_t,size_t) const;

  protected:
    const CSaTScanData                & gDataHub;                      /** const reference to data hub */
    AbstractLoglikelihoodRatioUnifier * gpUnifier;                     /** log likelihood ratio unifier for multiple data sets */
    std::vector<std::pair<count_t,measure_t> > gvDataSetTotals;
    std::vector<measure_t>              gvDataSetMeasureAuxTotals;
    count_t                             gtMinLowRateCases;
    count_t                             gtMinHighRateCases;

  public:
    AbstractLikelihoodCalculator(const CSaTScanData& DataHub);
    virtual ~AbstractLikelihoodCalculator();

    SCANRATE_FUNCPTR                    gpRateOfInterest;
    SCANRATENORMAL_FUNCPTR              gpRateOfInterestNormal;

    virtual double                      CalcLogLikelihood(count_t n, measure_t u) const;
    virtual double                      CalcLogLikelihoodRatio(count_t tCases, measure_t tMeasure, size_t tSetIndex=0) const;
    virtual double                      CalcLogLikelihoodRatioOrdinal(const std::vector<count_t>& vOrdinalCases, size_t tSetIndex=0) const;
    virtual double                      CalcLogLikelihoodRatioNormal(count_t tCases, measure_t tMeasure, measure_t tMeasure2, size_t tSetIndex=0) const;
    virtual double                      CalcMonotoneLogLikelihood(tract_t tSteps, const std::vector<count_t>& vCasesList, const std::vector<measure_t>& vMeasureList) const;
    virtual double                      CalcSVTTLogLikelihood(size_t tSetIndex, SVTTClusterData& ClusterData, const CTimeTrend& GlobalTimeTrend) const;
    virtual double                      CalculateFullStatistic(double dMaximizingValue, size_t tDataSetIndex=0) const;
    virtual double                      CalculateMaximizingValue(count_t n, measure_t u, size_t tDataSetIndex=0) const;
    virtual double                      CalculateMaximizingValueNormal(count_t n, measure_t u, measure_t u2, size_t tDataSetIndex=0) const;
    virtual double                      CalculateMaximizingValueOrdinal(const std::vector<count_t>& vOrdinalCases, size_t tSetIndex=0) const;
    const CSaTScanData                & GetDataHub() const {return gDataHub;}
    virtual double                      GetLogLikelihoodForTotal(size_t tSetIndex=0) const;
    AbstractLoglikelihoodRatioUnifier & GetUnifier() const;

    inline bool                         HighOrLowRate(count_t nCases, measure_t nMeasure, size_t tSetIndex=0) const;
    inline bool                         HighRate(count_t nCases, measure_t nMeasure, size_t tSetIndex=0) const;
    inline bool                         LowRate(count_t nCases, measure_t nMeasure, size_t tSetIndex=0) const;
    inline bool                         MultipleSetsHighRate(count_t nCases, measure_t nMeasure, size_t tSetIndex) const;

    inline bool                         HighOrLowRateNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux, size_t tSetIndex=0) const;
    inline bool                         HighRateNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux, size_t tSetIndex=0) const;
    inline bool                         LowRateNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux, size_t tSetIndex=0) const;
    inline bool                         MultipleSetsHighRateNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux, size_t tSetIndex) const;

    inline bool                         HighOrLowRateWeightedNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux, size_t tSetIndex=0) const;
    inline bool                         HighRateWeightedNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux, size_t tSetIndex=0) const;
    inline bool                         LowRateWeightedNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux, size_t tSetIndex=0) const;
    inline bool                         MultipleSetsHighRateWeightedNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux, size_t tSetIndex) const;
};

/** Indicates whether an area has lower than expected cases for a clustering
    within a single dataset. */
inline bool AbstractLikelihoodCalculator::LowRate(count_t nCases, measure_t nMeasure, size_t tSetIndex) const {
   if (nMeasure == 0 || nCases < gtMinLowRateCases) return false;
   return (nCases*gvDataSetTotals[tSetIndex].second < nMeasure*gvDataSetTotals[tSetIndex].first);
}

/** Indicates whether an area has high than expected cases for a clustering
    within a single dataset. Clusterings with less than two cases are not
    considered for high rates. Note this function should not be used for scannning
    for high rates with an analysis with multiple datasets; use MultipleSetsHighRate() */
inline bool AbstractLikelihoodCalculator::HighRate(count_t nCases, measure_t nMeasure, size_t tSetIndex) const {
   if (nMeasure == 0 || nCases < gtMinHighRateCases) return false;
   return (nCases*gvDataSetTotals[tSetIndex].second  > nMeasure*gvDataSetTotals[tSetIndex].first);
}
/** Indicates whether an area has lower than expected cases for a clustering
    within a single dataset. */
inline bool AbstractLikelihoodCalculator::HighOrLowRate(count_t nCases, measure_t nMeasure, size_t tSetIndex) const {
   if (nMeasure == 0) return false;
   //check for high rate
   if (nCases >= gtMinHighRateCases && nCases*gvDataSetTotals[tSetIndex].second > nMeasure*gvDataSetTotals[tSetIndex].first) return true;
   //check for low rate
   else if (nCases >= gtMinLowRateCases && nCases*gvDataSetTotals[tSetIndex].second < nMeasure*gvDataSetTotals[tSetIndex].first) return true;
   else return false;
}
/** For multiple sets, the criteria that a high rate must have more than one case
    is not currently implemented. */
inline bool AbstractLikelihoodCalculator::MultipleSetsHighRate(count_t nCases, measure_t nMeasure, size_t tSetIndex) const {
   if (nMeasure == 0) return false;
   return (nCases*gvDataSetTotals[tSetIndex].second  > nMeasure*gvDataSetTotals[tSetIndex].first);
}

/** Indicates whether an area has lower than expected cases for a clustering
    within a single dataset. */
inline bool AbstractLikelihoodCalculator::LowRateNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux, size_t tSetIndex) const {
   return LowRate(nCases, nMeasure, tSetIndex);
}

/** Indicates whether an area has high than expected cases for a clustering
    within a single dataset. Clusterings with less than two cases are not
    considered for high rates. Note this function should not be used for scannning
    for high rates with an analysis with multiple datasets; use MultipleSetsHighRate() */
inline bool AbstractLikelihoodCalculator::HighRateNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux,size_t tSetIndex) const {
   return HighRate(nCases, nMeasure, tSetIndex);
}
/** Indicates whether an area has lower than expected cases for a clustering
    within a single dataset. */
inline bool AbstractLikelihoodCalculator::HighOrLowRateNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux,size_t tSetIndex) const {
   return HighOrLowRate(nCases, nMeasure, tSetIndex);
}
/** For multiple sets, the criteria that a high rate must have more than one case
    is not currently implemented. */
inline bool AbstractLikelihoodCalculator::MultipleSetsHighRateNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux,size_t tSetIndex) const {
   return MultipleSetsHighRate(nCases, nMeasure, tSetIndex);
}

/** Indicates whether an area has lower than expected cases for a clustering
    within a single dataset. */
inline bool AbstractLikelihoodCalculator::LowRateWeightedNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux, size_t tSetIndex) const {
   if (nMeasure == 0 || nCases < gtMinLowRateCases) return false;
   return nMeasure/nMeasureAux < gvDataSetTotals[tSetIndex].second/gvDataSetMeasureAuxTotals[tSetIndex];
}

/** Indicates whether an area has high than expected cases for a clustering
    within a single dataset. Clusterings with less than two cases are not
    considered for high rates. Note this function should not be used for scannning
    for high rates with an analysis with multiple datasets; use MultipleSetsHighRate() */
inline bool AbstractLikelihoodCalculator::HighRateWeightedNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux,size_t tSetIndex) const {
   if (nMeasure == 0 || nCases < gtMinHighRateCases) return false;
   return nMeasure/nMeasureAux > gvDataSetTotals[tSetIndex].second/gvDataSetMeasureAuxTotals[tSetIndex];
}
/** Indicates whether an area has lower than expected cases for a clustering
    within a single dataset. */
inline bool AbstractLikelihoodCalculator::HighOrLowRateWeightedNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux,size_t tSetIndex) const {
   if (nMeasure == 0) return false;
   //check for high rate
   if (nCases >= gtMinHighRateCases && nMeasure/nMeasureAux > gvDataSetTotals[tSetIndex].second/gvDataSetMeasureAuxTotals[tSetIndex]) return true;
   //check for low rate
   else if (nCases >= gtMinLowRateCases && nMeasure/nMeasureAux < gvDataSetTotals[tSetIndex].second/gvDataSetMeasureAuxTotals[tSetIndex]) return true;
   else return false;
}
/** For multiple sets, the criteria that a high rate must have more than one case
    is not currently implemented. */
inline bool AbstractLikelihoodCalculator::MultipleSetsHighRateWeightedNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux,size_t tSetIndex) const {
   if (nMeasure == 0) return false;
   return (nMeasure/nMeasureAux > gvDataSetTotals[tSetIndex].second/gvDataSetMeasureAuxTotals[tSetIndex]);
}
//******************************************************************************
#endif


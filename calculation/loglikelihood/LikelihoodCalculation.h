//******************************************************************************
#ifndef __LikelihoodCalculation_H
#define __LikelihoodCalculation_H
//******************************************************************************
#include "SaTScan.h"
#include "boost/shared_ptr.hpp"

class SpatialMonotoneData; /** forward class declaration */
class CSaTScanData;       /** forward class declaration */
class SVTTClusterData;    /** forward class declaration */
class AbstractTimeTrend;         /** forward class declaration */
class AbstractLoglikelihoodRatioUnifier; /** forward class declaration */
class Matrix; /** forward class declaration */

/** Abstract interface for which to calculate log likelihoods and log likelihood ratios. */
class AbstractLikelihoodCalculator {
  public:
    typedef bool (AbstractLikelihoodCalculator::*SCANRATE_FUNCPTR) (count_t,measure_t,size_t) const;
    typedef bool (AbstractLikelihoodCalculator::*SCANRATENORMAL_FUNCPTR) (count_t,measure_t,measure_t,size_t) const;
    typedef bool (AbstractLikelihoodCalculator::*SCANRATEUNIFORMTIME_FUNCPTR) (count_t, measure_t, count_t, measure_t, size_t) const;
    typedef double (AbstractLikelihoodCalculator::*RISK_FUNCPTR) (count_t, measure_t, size_t) const;
    typedef double (AbstractLikelihoodCalculator::*RISKUNIFORMTIME_FUNCPTR) (count_t, measure_t, count_t, measure_t, size_t) const;

  protected:
    const CSaTScanData                & gDataHub; /** const reference to data hub */
    boost::shared_ptr<AbstractLoglikelihoodRatioUnifier> _unifier; /** log likelihood ratio unifier for multiple data sets */
    std::vector<std::pair<count_t,measure_t> > gvDataSetTotals;
    std::vector<measure_t>              gvDataSetMeasureAuxTotals;
    count_t                             _min_low_rate_cases;
    count_t                             _min_high_rate_cases;

    double                              _low_risk_threshold;
    double                              _high_risk_threshold;
    double                              _measure_adjustment;
    std::vector<double>                 _average_rank_dataset;

  public:
    AbstractLikelihoodCalculator(const CSaTScanData& DataHub);
    virtual ~AbstractLikelihoodCalculator() {}

    SCANRATE_FUNCPTR                    gpRateOfInterest;
    SCANRATENORMAL_FUNCPTR              gpRateOfInterestNormal;
    SCANRATEUNIFORMTIME_FUNCPTR         gpRateOfInterestUniformTime;
    RISK_FUNCPTR                        _risk_function;
    RISKUNIFORMTIME_FUNCPTR             _risk_function_uniformtime;

    virtual double                      CalcLogLikelihood(count_t n, measure_t u) const;
    virtual double                      CalcLogLikelihoodRatio(count_t tCases, measure_t tMeasure, size_t tSetIndex=0) const;
    virtual double                      CalcLogLikelihoodRatioOrdinal(const std::vector<count_t>& vOrdinalCases, size_t tSetIndex=0) const;
    virtual double                      CalcLogLikelihoodRatioNormal(count_t tCases, measure_t tMeasure, measure_t tMeasure2, size_t tSetIndex=0) const;
    virtual double                      CalcLogLikelihoodRatioNormal(Matrix& xg, Matrix& tobeinversed, Matrix& xgsigmaw, size_t tDataSetIndex=0) const;
    virtual double                      CalcLogLikelihoodRatioUniformTime(count_t tCases, measure_t tMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex = 0) const;
    virtual double                      CalcMonotoneLogLikelihood(tract_t tSteps, const std::vector<count_t>& vCasesList, const std::vector<measure_t>& vMeasureList) const;
    virtual double                      CalcSVTTLogLikelihood(size_t tSetIndex, SVTTClusterData& ClusterData, const AbstractTimeTrend& GlobalTimeTrend) const;
    virtual double                      CalcLogLikelihoodUniformTime(count_t cases, measure_t measure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex=0) const;
    virtual double                      CalculateFullStatistic(double dMaximizingValue, size_t tDataSetIndex=0) const;
    virtual double                      CalculateMaximizingValue(count_t n, measure_t u, size_t tDataSetIndex=0) const;
    virtual double                      CalculateMaximizingValueNormal(count_t n, measure_t u, measure_t u2, size_t tDataSetIndex=0) const;
    virtual double                      CalculateMaximizingValueNormal(Matrix& xg, Matrix& tobeinversed, Matrix& xgsigmaw, size_t tDataSetIndex=0) const;
    virtual double                      CalculateMaximizingValueOrdinal(const std::vector<count_t>& vOrdinalCases, size_t tSetIndex=0) const;
    virtual double                      CalculateMaximizingValueUniformTime(count_t cases, measure_t measure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex=0) const;
    const CSaTScanData                & GetDataHub() const {return gDataHub;}
    virtual double                      GetLogLikelihoodForTotal(size_t tSetIndex=0) const;
    AbstractLoglikelihoodRatioUnifier & GetUnifier() const;

    inline bool                         HighOrLowRate(count_t nCases, measure_t nMeasure, size_t tSetIndex=0) const;
    inline bool                         HighRate(count_t nCases, measure_t nMeasure, size_t tSetIndex=0) const;
    inline bool                         LowRate(count_t nCases, measure_t nMeasure, size_t tSetIndex=0) const;
    inline bool                         MultipleSetsHighRate(count_t nCases, measure_t nMeasure, size_t tSetIndex) const;

    inline double                       getObservedDividedExpected(count_t nCases, measure_t nMeasure, size_t tSetIndex = 0) const;
    inline double                       getRelativeRisk(count_t nCases, measure_t nMeasure, size_t tSetIndex=0) const;
    inline bool                         HighRisk(count_t nCases, measure_t nMeasure, size_t tSetIndex=0) const;
    inline bool                         LowRisk(count_t nCases, measure_t nMeasure, size_t tSetIndex=0) const;
    inline bool                         HighRateOrLowRisk(count_t nCases, measure_t nMeasure, size_t tSetIndex=0) const;
    inline bool                         HighRiskOrLowRate(count_t nCases, measure_t nMeasure, size_t tSetIndex = 0) const;
    inline bool                         HighRiskOrLowRisk(count_t nCases, measure_t nMeasure, size_t tSetIndex = 0) const;

    inline bool                         HighOrLowRateNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux, size_t tSetIndex=0) const;
    inline bool                         HighRateNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux, size_t tSetIndex=0) const;
    inline bool                         LowRateNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux, size_t tSetIndex=0) const;
    inline bool                         MultipleSetsHighRateNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux, size_t tSetIndex) const;

    inline bool                         HighOrLowRateWeightedNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux, size_t tSetIndex=0) const;
    inline bool                         HighRateWeightedNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux, size_t tSetIndex=0) const;
    inline bool                         LowRateWeightedNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux, size_t tSetIndex=0) const;
    inline bool                         MultipleSetsHighRateWeightedNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux, size_t tSetIndex) const;

    inline bool                         AllRatesWeightedNormalCovariates(count_t nCases, measure_t nMeasure, measure_t nMeasureAux, size_t tSetIndex=0) const;

    inline bool                         HighOrLowRateRank(count_t nCases, measure_t nMeasure, size_t tSetIndex = 0) const;
    inline bool                         HighRateRank(count_t nCases, measure_t nMeasure, size_t tSetIndex = 0) const;
    inline bool                         LowRateRank(count_t nCases, measure_t nMeasure, size_t tSetIndex = 0) const;
    inline bool                         MultipleSetsHighRateRank(count_t nCases, measure_t nMeasure, size_t tSetIndex) const;

    inline bool                         HighOrLowRateUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex = 0) const;
    inline bool                         HighRateUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex = 0) const;
    inline bool                         LowRateUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex = 0) const;
    inline bool                         MultipleSetsHighRateUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) const;

    inline double                       getRelativeRiskUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex = 0) const;
    inline bool                         HighRiskUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex = 0) const;
    inline bool                         LowRiskUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex = 0) const;
    inline bool                         HighRateOrLowRiskUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex = 0) const;
    inline bool                         HighRiskOrLowRateUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex = 0) const;
    inline bool                         HighRiskOrLowRiskUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex = 0) const;

};

/* Returns the relative risk (same as ODE) -- this function is used as a risk function when restricting clusters by risk level for uniform-time model. */
inline double AbstractLikelihoodCalculator::getRelativeRiskUniformTime(count_t cases, measure_t measure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) const {
    double measure_diff = measureInPeriod - measure;
    if (measure_diff) {
        double M = 2.0;
        double u = 0.0;
        if (cases < casesInPeriod)
            u = measure * static_cast<double>(casesInPeriod - cases) / (measure_diff);
        else if (cases == casesInPeriod)
            u = measure / (M * (measure_diff));
        return static_cast<double>(cases) / u;
    } return 0.0;
}

/* Returns whether potential cluster is high rate while exceeding high risk level minimum. */
inline bool AbstractLikelihoodCalculator::HighRiskUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) const {
    return HighRateUniformTime(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex) && (this->*_risk_function_uniformtime)(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex) >= _high_risk_threshold;
}

/* Returns whether potential cluster is low rate while not exceeding low risk level maximum. */
inline bool AbstractLikelihoodCalculator::LowRiskUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) const {
    return LowRateUniformTime(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex) && (this->*_risk_function_uniformtime)(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex) <= _low_risk_threshold;
}

/* Returns whether potential cluster is high rate or low rate while not exceeding low risk level maximum. */
inline bool AbstractLikelihoodCalculator::HighRateOrLowRiskUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) const {
    if (HighRateUniformTime(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex))
        return true;
    if (LowRateUniformTime(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex) && (this->*_risk_function_uniformtime)(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex) <= _low_risk_threshold)
        return true;
    return false;
}

/* Returns whether potential cluster is low rate or high rate while exceeding high risk level minimum. */
inline bool AbstractLikelihoodCalculator::HighRiskOrLowRateUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) const {
    if (LowRateUniformTime(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex))
        return true;
    if (HighRateUniformTime(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex) && (this->*_risk_function_uniformtime)(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex) >= _high_risk_threshold)
        return true;
    return false;
}

/* Returns whether potential cluster is low rate while not exceeding low risk level maximum or high rate while exceeding high risk level minimum. */
inline bool AbstractLikelihoodCalculator::HighRiskOrLowRiskUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) const {
    if (HighOrLowRateUniformTime(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex)) {
        double risk = (this->*_risk_function_uniformtime)(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex);
        return (risk <= _low_risk_threshold || risk >= _high_risk_threshold);
    }
    return false;
}

/** Indicates whether an area has lower than expected cases for a clustering within a single dataset. */
inline bool AbstractLikelihoodCalculator::LowRateUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) const {
    if (nCases < _min_low_rate_cases || casesInPeriod == 0 || nMeasure == 0.0) return false;
    return static_cast<double>(nCases)/ static_cast<double>(casesInPeriod) < nMeasure / measureInPeriod;
}

/** Indicates whether an area has higher than expected cases for a clustering within a single dataset. */
inline bool AbstractLikelihoodCalculator::HighRateUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) const {
    if (nCases < _min_high_rate_cases || casesInPeriod == 0 || nMeasure == 0.0) return false;
    return static_cast<double>(nCases) / static_cast<double>(casesInPeriod) > nMeasure / measureInPeriod;
}

/** Indicates whether an area has higher or lower than expected cases for a clustering within a single dataset. */
inline bool AbstractLikelihoodCalculator::HighOrLowRateUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) const {
    return LowRateUniformTime(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex) || 
           HighRateUniformTime(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex);
}

/** For multiple sets, the criteria that a high rate must have more than one case is not currently implemented. */
inline bool AbstractLikelihoodCalculator::MultipleSetsHighRateUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) const {
    if (LowRateUniformTime(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex))
        return true;
    if (HighRateUniformTime(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex))
        return true;
    return false;
}

/** Indicates whether an area has lower average rank compared to average rank in data set -- lower cluster. */
inline bool AbstractLikelihoodCalculator::LowRateRank(count_t nCases, measure_t nMeasure, size_t tSetIndex) const {
    if (nCases == 0 || nMeasure == 0.0) return false;
    return _average_rank_dataset[tSetIndex] > (nMeasure + 1.0) / nCases;

    /* TODO - Do we incorporate a minimum number of cases here?  -- yes */
}

/** Indicates whether an area has higher average rank compared to average rank in data set -- high cluster. */
inline bool AbstractLikelihoodCalculator::HighRateRank(count_t nCases, measure_t nMeasure, size_t tSetIndex) const {
    if (nCases == 0 || nMeasure == 0.0) return false;
    return _average_rank_dataset[tSetIndex] < (nMeasure + 1.0) / nCases;

    /* TODO - Do we incorporate a minimum number of cases here? -- yes */
}

/** Indicates whether an area has lower than expected cases for a clustering within a single dataset. */
inline bool AbstractLikelihoodCalculator::HighOrLowRateRank(count_t nCases, measure_t nMeasure, size_t tSetIndex) const {
    if (nCases == 0 || nMeasure == 0.0) return false;
    return true;
    /* TODO - Do we incorporate a minimum number of cases here?  -- yes */
}
/** For multiple sets, the criteria that a high rate must have more than one case is not currently implemented. */
inline bool AbstractLikelihoodCalculator::MultipleSetsHighRateRank(count_t nCases, measure_t nMeasure, size_t tSetIndex) const {
    if (nCases == 0 || nMeasure == 0.0) return false;
    return _average_rank_dataset[tSetIndex] > (nMeasure + 1.0) / nCases;

    /* TODO -Is this the correct function? Do we even need it? */
    /* TODO - Do we incorporate a minimum number of cases here? */
}

/* Returns the observed / expected -- this function is used as a risk function when restricting clusters by risk level. */
inline double AbstractLikelihoodCalculator::getObservedDividedExpected(count_t nCases, measure_t nMeasure, size_t tSetIndex) const {
    nMeasure *= _measure_adjustment; // apply measure adjustment --  applicable only to Bernoulli
    return (nMeasure ? static_cast<double>(nCases) / nMeasure : 0.0);
}

/* Returns the relative risk -- this function is used as a risk function when restricting clusters by risk level. */
inline double AbstractLikelihoodCalculator::getRelativeRisk(count_t nCases, measure_t nMeasure, size_t tSetIndex) const {
    double total_cases = gvDataSetTotals[tSetIndex].first;
    if (total_cases == nCases) return std::numeric_limits<double>::max(); // could use std::numeric_limits<double>::infinity()    
    nMeasure *= _measure_adjustment; // apply measure adjustment --  applicable only to Bernoulli
    if (nMeasure && total_cases - nMeasure && ((total_cases - nMeasure) / (total_cases - nMeasure)))
        return (nCases / nMeasure) / ((total_cases - nCases) / (total_cases - nMeasure));
    return 0.0;
}

/* Returns whether potential cluster is high rate while exceeding high risk level minimum. */
inline bool AbstractLikelihoodCalculator::HighRisk(count_t nCases, measure_t nMeasure, size_t tSetIndex) const {
    return HighRate(nCases, nMeasure, tSetIndex) && (this->*_risk_function)(nCases, nMeasure, tSetIndex) >= _high_risk_threshold;
}

/* Returns whether potential cluster is low rate while not exceeding low risk level maximum. */
inline bool AbstractLikelihoodCalculator::LowRisk(count_t nCases, measure_t nMeasure, size_t tSetIndex) const {
    return LowRate(nCases, nMeasure, tSetIndex) && (this->*_risk_function)(nCases, nMeasure, tSetIndex) <= _low_risk_threshold;
}

/* Returns whether potential cluster is high rate or low rate while not exceeding low risk level maximum. */
inline bool AbstractLikelihoodCalculator::HighRateOrLowRisk(count_t nCases, measure_t nMeasure, size_t tSetIndex) const {
    if (HighRate(nCases, nMeasure, tSetIndex))
        return true;
    if (LowRate(nCases, nMeasure, tSetIndex) && (this->*_risk_function)(nCases, nMeasure, tSetIndex) <= _low_risk_threshold)
        return true;
    return false;
}

/* Returns whether potential cluster is low rate or high rate while exceeding high risk level minimum. */
inline bool AbstractLikelihoodCalculator::HighRiskOrLowRate(count_t nCases, measure_t nMeasure, size_t tSetIndex) const {
    if (LowRate(nCases, nMeasure, tSetIndex))
        return true;
    if (HighRate(nCases, nMeasure, tSetIndex) && (this->*_risk_function)(nCases, nMeasure, tSetIndex) >= _high_risk_threshold)
        return true;
    return false;
}

/* Returns whether potential cluster is low rate while not exceeding low risk level maximum or high rate while exceeding high risk level minimum. */
inline bool AbstractLikelihoodCalculator::HighRiskOrLowRisk(count_t nCases, measure_t nMeasure, size_t tSetIndex) const {
    if (HighOrLowRate(nCases, nMeasure, tSetIndex)) {
        double risk = (this->*_risk_function)(nCases, nMeasure, tSetIndex);
        return (risk <= _low_risk_threshold || risk >= _high_risk_threshold);
    }
    return false;
}

/** Indicates whether an area has lower than expected cases for a clustering within a single dataset. */
inline bool AbstractLikelihoodCalculator::LowRate(count_t nCases, measure_t nMeasure, size_t tSetIndex) const {
   if (nMeasure == 0 || nCases < _min_low_rate_cases) return false;
   return (nCases*gvDataSetTotals[tSetIndex].second < nMeasure*gvDataSetTotals[tSetIndex].first);
}

/** Indicates whether an area has high than expected cases for a clustering within a single dataset. Clusterings with less
    than two cases are not considered for high rates. Note this function should not be used for scannning for high rates with
    an analysis with multiple datasets; use MultipleSetsHighRate() */
inline bool AbstractLikelihoodCalculator::HighRate(count_t nCases, measure_t nMeasure, size_t tSetIndex) const {
   if (nMeasure == 0 || nCases < _min_high_rate_cases) return false;
   return (nCases*gvDataSetTotals[tSetIndex].second  > nMeasure*gvDataSetTotals[tSetIndex].first);
}

/** Indicates whether an area has lower than expected cases for a clustering within a single dataset. */
inline bool AbstractLikelihoodCalculator::HighOrLowRate(count_t nCases, measure_t nMeasure, size_t tSetIndex) const {
   if (nMeasure == 0) return false;
   //check for high rate
   if (nCases >= _min_high_rate_cases && nCases*gvDataSetTotals[tSetIndex].second > nMeasure*gvDataSetTotals[tSetIndex].first) return true;
   //check for low rate
   else if (nCases >= _min_low_rate_cases && nCases*gvDataSetTotals[tSetIndex].second < nMeasure*gvDataSetTotals[tSetIndex].first) return true;
   else return false;
}
/** For multiple sets, the criteria that a high rate must have more than one case is not currently implemented. */
inline bool AbstractLikelihoodCalculator::MultipleSetsHighRate(count_t nCases, measure_t nMeasure, size_t tSetIndex) const {
   if (nMeasure == 0) return false;
   return (nCases*gvDataSetTotals[tSetIndex].second  > nMeasure*gvDataSetTotals[tSetIndex].first);
}

/** Indicates whether an area has lower than expected cases for a clustering within a single dataset. */
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
/** For multiple sets, the criteria that a high rate must have more than one case is not currently implemented. */
inline bool AbstractLikelihoodCalculator::MultipleSetsHighRateNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux,size_t tSetIndex) const {
   return MultipleSetsHighRate(nCases, nMeasure, tSetIndex);
}

/** Indicates whether an area has lower than expected cases for a clustering within a single dataset. */
inline bool AbstractLikelihoodCalculator::LowRateWeightedNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux, size_t tSetIndex) const {
   if (nMeasure == 0 || nCases < _min_low_rate_cases) return false;
   return nMeasure/nMeasureAux < gvDataSetTotals[tSetIndex].second/gvDataSetMeasureAuxTotals[tSetIndex];
}

/** Indicates whether an area has high than expected cases for a clustering
    within a single dataset. Clusterings with less than two cases are not
    considered for high rates. Note this function should not be used for scannning
    for high rates with an analysis with multiple datasets; use MultipleSetsHighRate() */
inline bool AbstractLikelihoodCalculator::HighRateWeightedNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux,size_t tSetIndex) const {
   if (nMeasure == 0 || nCases < _min_high_rate_cases) return false;
   return nMeasure/nMeasureAux > gvDataSetTotals[tSetIndex].second/gvDataSetMeasureAuxTotals[tSetIndex];
}
/** Indicates whether an area has lower than expected cases for a clustering within a single dataset. */
inline bool AbstractLikelihoodCalculator::HighOrLowRateWeightedNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux,size_t tSetIndex) const {
   if (nMeasure == 0) return false;
   //check for high rate
   if (nCases >= _min_high_rate_cases && nMeasure/nMeasureAux > gvDataSetTotals[tSetIndex].second/gvDataSetMeasureAuxTotals[tSetIndex]) return true;
   //check for low rate
   else if (nCases >= _min_low_rate_cases && nMeasure/nMeasureAux < gvDataSetTotals[tSetIndex].second/gvDataSetMeasureAuxTotals[tSetIndex]) return true;
   else return false;
}
/** For multiple sets, the criteria that a high rate must have more than one case is not currently implemented. */
inline bool AbstractLikelihoodCalculator::MultipleSetsHighRateWeightedNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux,size_t tSetIndex) const {
   if (nMeasure == 0) return false;
   return (nMeasure/nMeasureAux > gvDataSetTotals[tSetIndex].second/gvDataSetMeasureAuxTotals[tSetIndex]);
}

/** Indicates whether an area has enough cases for a clustering within a single dataset for normal model with weights and covariates. */
inline bool AbstractLikelihoodCalculator::AllRatesWeightedNormalCovariates(count_t nCases, measure_t nMeasure, measure_t nMeasureAux, size_t tSetIndex) const {
   return nCases >= _min_high_rate_cases;
}
//******************************************************************************
#endif


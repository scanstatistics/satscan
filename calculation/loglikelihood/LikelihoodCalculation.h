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
    typedef bool (AbstractLikelihoodCalculator::*SCANRATE_FUNCPTR) (count_t,measure_t) const;
    typedef bool (AbstractLikelihoodCalculator::*SCANRATETIMESTRATIFIED_FUNCPTR) (count_t, measure_t, count_t, measure_t) const;
    typedef bool (AbstractLikelihoodCalculator::* SCANRATEBATCHED_FUNCPTR) (count_t, measure_t) const;
    typedef bool (AbstractLikelihoodCalculator::*SCANRATENORMAL_FUNCPTR) (count_t,measure_t,measure_t) const;
    typedef bool (AbstractLikelihoodCalculator::*SCANRATEUNIFORMTIME_FUNCPTR) (count_t, measure_t, count_t, measure_t, size_t) const;
    typedef bool (AbstractLikelihoodCalculator::*SCANRATEMULTISET_FUNCPTR) (const AbstractLoglikelihoodRatioUnifier&, bool) const;
    typedef double (AbstractLikelihoodCalculator::*RISK_FUNCPTR) (count_t, measure_t) const;
    typedef double (AbstractLikelihoodCalculator::*RISK_MULTISET_FUNCPTR) (count_t, measure_t, count_t, measure_t) const;

  protected:
    const CSaTScanData                & gDataHub; /** const reference to data hub */
    boost::shared_ptr<AbstractLoglikelihoodRatioUnifier> _unifier; /** log likelihood ratio unifier for multiple data sets */
    count_t                             _min_low_rate_cases;
    count_t                             _min_high_rate_cases;

    double                              _low_risk_threshold;
    double                              _high_risk_threshold;
    double                              _measure_adjustment;
    std::vector<double>                 _average_rank_dataset;

  public:
    AbstractLikelihoodCalculator(const CSaTScanData& DataHub);
    virtual ~AbstractLikelihoodCalculator() {}

    std::vector<std::pair<count_t, measure_t> > gvDataSetTotals;
    std::vector<measure_t>              gvDataSetMeasureAuxTotals;
    std::vector<measure_t>              gvDataSetMeasureAux2Totals;

    SCANRATE_FUNCPTR                    gpRateOfInterest;
    SCANRATETIMESTRATIFIED_FUNCPTR      gpRateOfInterestTimeStratified;
    SCANRATEBATCHED_FUNCPTR             gpRateOfInterestBatched;
    SCANRATENORMAL_FUNCPTR              gpRateOfInterestNormal;
    SCANRATEUNIFORMTIME_FUNCPTR         gpRateOfInterestUniformTime;
    SCANRATEMULTISET_FUNCPTR            _rate_of_interest_multiset;
    RISK_FUNCPTR                        _risk_function;
    RISK_MULTISET_FUNCPTR               _risk_multiset_function;

    virtual double                      CalcLogLikelihood(count_t n, measure_t u) const;
    virtual double                      CalcLogLikelihoodRatio(count_t tCases, measure_t tMeasure, size_t tSetIndex=0) const;
    virtual double                      CalcLogLikelihoodRatioOrdinal(const std::vector<count_t>& vOrdinalCases, size_t tSetIndex=0) const;
    virtual double                      CalcLogLikelihoodRatioNormal(count_t tCases, measure_t tMeasure, measure_t tMeasure2, size_t tSetIndex = 0) const;
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
    virtual double                      CalcLogLikelihoodTimeStratified(count_t n, measure_t u, count_t N, measure_t U) const;
    virtual double                      CalcLogLikelihoodBernoulliSpatialStratified(count_t n, measure_t u, tract_t tract, size_t setIdx = 0) const;

    const CSaTScanData                & GetDataHub() const {return gDataHub;}
    virtual double                      GetLogLikelihoodForTotal(size_t tSetIndex=0) const;
    AbstractLoglikelihoodRatioUnifier & GetUnifier() const;

    /* Cluster evaluation functions for high, low or simultaneous high or low rate scanning (single data set).*/
    inline bool                         HighRate(count_t nCases, measure_t nMeasure) const;
    inline bool                         LowRate(count_t nCases, measure_t nMeasure) const;
    inline bool                         HighOrLowRate(count_t nCases, measure_t nMeasure) const;

    /* Cluster evaluation functions for high and low rate scanning in the context of a single data stream when analysis has multiple data streams. */
    inline bool                         HighRateDataStream(count_t nCases, measure_t nMeasure, size_t tSetIndex) const;
    inline bool                         LowRateDataStream(count_t nCases, measure_t nMeasure, size_t tSetIndex) const;

    /* Functions to calculate either relative risk or observed/expected when determining if a cluster meets relative risk threshold. */
    inline double                       getObservedDividedExpected(count_t nCases, measure_t nMeasure) const;
    inline double                       getRelativeRisk(count_t nCases, measure_t nMeasure) const;
    /* Multiple data stream versions. */
    inline double                       getObservedDividedExpectedMultiset(count_t sumObserved, measure_t sumExpected, count_t sumTotalCases, measure_t sumExpectedTotal) const;
    inline double                       getRelativeRiskMultiset(count_t sumObserved, measure_t sumExpected, count_t sumTotalCases, measure_t sumExpectedTotal) const;

    /* Cluster evaluation functions for high, low or simultaneous high and low relative risk scanning (single data set).*/
    inline bool                         HighRisk(count_t nCases, measure_t nMeasure) const;
    inline bool                         LowRisk(count_t nCases, measure_t nMeasure) const;
    inline bool                         HighRiskOrLowRisk(count_t nCases, measure_t nMeasure) const;
    /* Cluster evaluation function for high rate and low relative risk scanning (single data set).*/
    inline bool                         HighRateOrLowRisk(count_t nCases, measure_t nMeasure) const;
    /* Cluster evaluation function for high relative risk and low rate scanning (single data set).*/
    inline bool                         HighRiskOrLowRate(count_t nCases, measure_t nMeasure) const;

    /* Cluster evaluation functions that are specific to multiple data sets using any purpose type and scanning either high or low rates/risks.  */
    inline bool                         HighRateMultiset(const AbstractLoglikelihoodRatioUnifier& unifier, bool nonparametric) const;
    bool                                HighRiskMultiset(const AbstractLoglikelihoodRatioUnifier& unifier, bool nonparametric) const;
    inline bool                         LowRateMultiset(const AbstractLoglikelihoodRatioUnifier& unifier, bool nonparametric) const;
    bool                                LowRiskMultiset(const AbstractLoglikelihoodRatioUnifier& unifier, bool nonparametric) const;

    /* Cluster evaluation functions that are specific to multiple data sets using Adjustment purpose and simultaneous high or low rate scanning.  */
    bool                                AdjustmentHighRiskOrLowRiskMultiset(const AbstractLoglikelihoodRatioUnifier& unifier, bool nonparametric) const;
    bool                                AdjustmentHighRiskOrLowRateMultiset(const AbstractLoglikelihoodRatioUnifier& unifier, bool nonparametric) const;
    bool                                AdjustmentHighRateOrLowRiskMultiset(const AbstractLoglikelihoodRatioUnifier& unifier, bool nonparametric) const;
    bool                                AdjustmentHighOrLowRateMultiset(const AbstractLoglikelihoodRatioUnifier& unifier, bool nonparametric) const;

    /* Cluster evaluation functions that are specific to multiple data sets using Multivariate purpose and simultaneous high or low rate scanning.  */
    bool                                MultivariateHighRiskOrLowRiskMultiset(const AbstractLoglikelihoodRatioUnifier& unifier, bool nonparametric) const;
    bool                                MultivariateHighRiskOrLowRateMultiset(const AbstractLoglikelihoodRatioUnifier& unifier, bool nonparametric) const;
    bool                                MultivariateHighRateOrLowRiskMultiset(const AbstractLoglikelihoodRatioUnifier& unifier, bool nonparametric) const;
    bool                                MultivariateHighOrLowRateMultiset(const AbstractLoglikelihoodRatioUnifier& unifier, bool nonparametric) const;

    /* Cluster evaluation functions specific to the Batched model.*/
    inline bool                         HighRateBatchedExpected(count_t positiveBatches, measure_t expectedPositive) const;
    inline bool                         HighRiskBatchedExpected(count_t positiveBatches, measure_t expectedPositive) const;
    inline bool                         LowRateBatchedExpected(count_t positiveBatches, measure_t expectedPositive) const;
    inline bool                         LowRiskBatchedExpected(count_t positiveBatches, measure_t expectedPositive) const;
    inline bool                         HighOrLowRateBatchedExpected(count_t positiveBatches, measure_t expectedPositive) const;
    inline bool                         HighRiskOrLowRateBatchedExpected(count_t positiveBatches, measure_t expectedPositive) const;
    inline bool                         HighRateOrLowRiskBatchedExpected(count_t positiveBatches, measure_t expectedPositive) const;
    inline bool                         HighRiskOrLowRiskBatchedExpected(count_t positiveBatches, measure_t expectedPositive) const;

    /* Cluster evaluation functions specific to the Normal model.*/
    inline bool                         HighRateNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux) const;
    inline bool                         LowRateNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux) const;
    inline bool                         HighOrLowRateNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux) const;
    inline bool                         HighRateNormalDataStream(count_t nCases, measure_t nMeasure, measure_t nMeasureAux, size_t tSetIndex) const;

    /* Cluster evaluation functions specific to the weighted Normal model.*/
    inline bool                         HighRateWeightedNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux) const;
    inline bool                         LowRateWeightedNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux) const;
    inline bool                         HighOrLowRateWeightedNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux) const;
    inline bool                         AllRatesWeightedNormalCovariates(count_t nCases, measure_t nMeasure, measure_t nMeasureAux) const;
    inline bool                         MultipleSetsLowRateWeightedNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux, size_t tSetIndex) const;
    inline bool                         MultipleSetsHighRateWeightedNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux, size_t tSetIndex) const;

    /* Cluster evaluation functions specific to the Rank model.*/
    inline bool                         HighRateRank(count_t nCases, measure_t nMeasure) const;
    inline bool                         LowRateRank(count_t nCases, measure_t nMeasure) const;
    inline bool                         HighOrLowRateRank(count_t nCases, measure_t nMeasure) const;
    inline bool                         MultipleSetsHighRateRank(count_t nCases, measure_t nMeasure, size_t tSetIndex) const;

    /* Cluster evaluation functions specific to the Uniform Time model.*/
    inline bool                         HighRateUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex = 0) const;
    inline bool                         LowRateUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex = 0) const;
    inline bool                         HighOrLowRateUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex = 0) const;
    inline bool                         HighRiskUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex = 0) const;
    inline bool                         LowRiskUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex = 0) const;
    inline bool                         HighRiskOrLowRateUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex = 0) const;
    inline bool                         HighRiskOrLowRiskUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex = 0) const;
    inline bool                         HighRateOrLowRiskUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex = 0) const;
    inline bool                         MultipleSetsLowRateUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) const;
    inline bool                         MultipleSetsHighRateUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) const;
    inline double                       getRelativeRiskUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex = 0) const;

    /* Cluster evaluation functions specific to the time stratified temporal adjustment (Poisson or Bernoulli space-time).*/
    inline bool                         HighRateTimeStratified(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod) const;
    inline bool                         LowRateTimeStratified(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod) const;
    inline bool                         HighOrLowRateTimeStratified(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod) const;
    inline bool                         HighRiskTimeStratified(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod) const;
    inline bool                         LowRiskTimeStratified(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod) const;
    inline bool                         HighRiskOrLowRateTimeStratified(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod) const;
    inline bool                         HighRiskOrLowRiskTimeStratified(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod) const;
    inline bool                         HighRateOrLowRiskTimeStratified(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod) const;
    inline double                       getRelativeRiskTimeStratified(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod) const;
};

/* Returns whether potential cluster is high rate while exceeding high risk level minimum. */
inline bool AbstractLikelihoodCalculator::HighRiskTimeStratified(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod) const {
    return HighRateTimeStratified(nCases, nMeasure, casesInPeriod, measureInPeriod) && getRelativeRiskTimeStratified(nCases, nMeasure, casesInPeriod, measureInPeriod) >= _high_risk_threshold;
}

/* Returns whether potential cluster is low rate while not exceeding low risk level maximum. */
inline bool AbstractLikelihoodCalculator::LowRiskTimeStratified(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod) const {
    return LowRateUniformTime(nCases, nMeasure, casesInPeriod, measureInPeriod) && getRelativeRiskTimeStratified(nCases, nMeasure, casesInPeriod, measureInPeriod) <= _low_risk_threshold;
}

/* Returns whether potential cluster is high rate or low rate while not exceeding low risk level maximum. */
inline bool AbstractLikelihoodCalculator::HighRateOrLowRiskTimeStratified(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod) const {
    if (HighRateTimeStratified(nCases, nMeasure, casesInPeriod, measureInPeriod))
        return true;
    if (LowRateTimeStratified(nCases, nMeasure, casesInPeriod, measureInPeriod) && getRelativeRiskTimeStratified(nCases, nMeasure, casesInPeriod, measureInPeriod) <= _low_risk_threshold)
        return true;
    return false;
}

/* Returns whether potential cluster is low rate or high rate while exceeding high risk level minimum. */
inline bool AbstractLikelihoodCalculator::HighRiskOrLowRateTimeStratified(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod) const {
    if (HighRateTimeStratified(nCases, nMeasure, casesInPeriod, measureInPeriod))
        return getRelativeRiskTimeStratified(nCases, nMeasure, casesInPeriod, measureInPeriod) >= _high_risk_threshold;
    if (LowRateTimeStratified(nCases, nMeasure, casesInPeriod, measureInPeriod))
        return true;
    return false;
}

/* Returns whether potential cluster is low rate while not exceeding low risk level maximum or high rate while exceeding high risk level minimum. */
inline bool AbstractLikelihoodCalculator::HighRiskOrLowRiskTimeStratified(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod) const {
    if (HighRateUniformTime(nCases, nMeasure, casesInPeriod, measureInPeriod))
        return getRelativeRiskTimeStratified(nCases, nMeasure, casesInPeriod, measureInPeriod) >= _high_risk_threshold;
    if (LowRateUniformTime(nCases, nMeasure, casesInPeriod, measureInPeriod))
        return getRelativeRiskTimeStratified(nCases, nMeasure, casesInPeriod, measureInPeriod) <= _low_risk_threshold;
    return false;
}

/** Indicates whether an area has lower than expected cases for a clustering within a single dataset. */
inline bool AbstractLikelihoodCalculator::LowRateTimeStratified(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod) const {
    if (nMeasure == 0.0 || nCases < _min_low_rate_cases) return false;
    return nCases * measureInPeriod < nMeasure * casesInPeriod;
}

/** Indicates whether an area has higher than expected cases for a clustering within a single dataset. */
inline bool AbstractLikelihoodCalculator::HighRateTimeStratified(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod) const {
    if (nMeasure == 0.0 || nCases < _min_high_rate_cases) return false;
    return nCases * measureInPeriod > nMeasure * casesInPeriod;
}

/** Indicates whether an area has higher or lower than expected cases for a clustering within a single dataset. */
inline bool AbstractLikelihoodCalculator::HighOrLowRateTimeStratified(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod) const {
    return LowRateTimeStratified(nCases, nMeasure, casesInPeriod, measureInPeriod) || HighRateTimeStratified(nCases, nMeasure, casesInPeriod, measureInPeriod);
}

/* Returns the relative risk -- this function is used as a risk function when restricting clusters by risk level for time stratified ajustment. */
inline double AbstractLikelihoodCalculator::getRelativeRiskTimeStratified(count_t cases, measure_t measure, count_t casesInPeriod, measure_t measureInPeriod) const {
    if (casesInPeriod - measure && ((casesInPeriod - measure) / (measureInPeriod - measure))) {
        return (cases / measure) / ((casesInPeriod - cases) / (measureInPeriod - measure));
    } return 0.0;
}


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
    return HighRateUniformTime(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex) && getRelativeRiskUniformTime(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex) >= _high_risk_threshold;
}

/* Returns whether potential cluster is low rate while not exceeding low risk level maximum. */
inline bool AbstractLikelihoodCalculator::LowRiskUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) const {
    return LowRateUniformTime(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex) && getRelativeRiskUniformTime(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex) <= _low_risk_threshold;
}

/* Returns whether potential cluster is high rate or low rate while not exceeding low risk level maximum. */
inline bool AbstractLikelihoodCalculator::HighRateOrLowRiskUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) const {
    if (HighRateUniformTime(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex))
        return true;
    if (LowRateUniformTime(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex) && getRelativeRiskUniformTime(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex) <= _low_risk_threshold)
        return true;
    return false;
}

/* Returns whether potential cluster is low rate or high rate while exceeding high risk level minimum. */
inline bool AbstractLikelihoodCalculator::HighRiskOrLowRateUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) const {
    if (HighRateUniformTime(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex))
        return getRelativeRiskUniformTime(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex) >= _high_risk_threshold;
    if (LowRateUniformTime(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex))
        return true;
    return false;
}

/* Returns whether potential cluster is low rate while not exceeding low risk level maximum or high rate while exceeding high risk level minimum. */
inline bool AbstractLikelihoodCalculator::HighRiskOrLowRiskUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) const {
    if (HighRateUniformTime(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex))
        return getRelativeRiskUniformTime(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex) >= _high_risk_threshold;
    if (LowRateUniformTime(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex))
        return getRelativeRiskUniformTime(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex) <= _low_risk_threshold;
    return false;
}

/** Indicates whether an area has lower than expected cases for a clustering within a single dataset. */
inline bool AbstractLikelihoodCalculator::LowRateUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) const {
    if (nMeasure == 0.0 || nCases < _min_low_rate_cases) return false;
    return nCases * measureInPeriod < nMeasure * casesInPeriod;
}

/** Indicates whether an area has lower than expected cases for a clustering within a single dataset. */
inline bool AbstractLikelihoodCalculator::MultipleSetsLowRateUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) const {
    return nCases * measureInPeriod < nMeasure * casesInPeriod;
}

/** Indicates whether an area has higher than expected cases for a clustering within a single dataset. */
inline bool AbstractLikelihoodCalculator::HighRateUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) const {
    if (nMeasure == 0.0 || nCases < _min_high_rate_cases) return false;
    return nCases * measureInPeriod > nMeasure * casesInPeriod;
}

/** Indicates whether an area has higher or lower than expected cases for a clustering within a single dataset. */
inline bool AbstractLikelihoodCalculator::HighOrLowRateUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) const {
    return LowRateUniformTime(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex) || HighRateUniformTime(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex);
}

/** For multiple sets the criteria that a high rate must have a certain minumum is across data sets. */
inline bool AbstractLikelihoodCalculator::MultipleSetsHighRateUniformTime(count_t nCases, measure_t nMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) const {
    if (LowRateUniformTime(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex))
        return true;
    if (HighRateUniformTime(nCases, nMeasure, casesInPeriod, measureInPeriod, tSetIndex))
        return true;
    return false;
}

/** Indicates whether an area has lower average rank compared to average rank in data set -- lower cluster. */
inline bool AbstractLikelihoodCalculator::LowRateRank(count_t nCases, measure_t nMeasure) const {
    if (nCases == 0 || nMeasure == 0.0) return false;
    return _average_rank_dataset.front() > (nMeasure + 1.0) / nCases;

    /* TODO - Do we incorporate a minimum number of cases here?  -- yes */
}

/** Indicates whether an area has higher average rank compared to average rank in data set -- high cluster. */
inline bool AbstractLikelihoodCalculator::HighRateRank(count_t nCases, measure_t nMeasure) const {
    if (nCases == 0 || nMeasure == 0.0) return false;
    return _average_rank_dataset.front() < (nMeasure + 1.0) / nCases;

    /* TODO - Do we incorporate a minimum number of cases here? -- yes */
}

/** Indicates whether an area has lower than expected cases for a clustering within a single dataset. */
inline bool AbstractLikelihoodCalculator::HighOrLowRateRank(count_t nCases, measure_t nMeasure) const {
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

/* Returns the observed / expected -- this function is used as a risk function when restricting clusters by risk level . 
   This function is currently used for either space-time permutation or exponential probability models. */
inline double AbstractLikelihoodCalculator::getObservedDividedExpected(count_t nCases, measure_t nMeasure) const {
    return (nMeasure ? static_cast<double>(nCases) / nMeasure : 0.0);
}

/* Returns the relative risk -- this function is used as a risk function when restricting clusters by risk level. */
inline double AbstractLikelihoodCalculator::getRelativeRisk(count_t nCases, measure_t nMeasure) const {
    double total_cases = gvDataSetTotals.front().first;
    if (total_cases == nCases) return std::numeric_limits<double>::max(); // could use std::numeric_limits<double>::infinity()    
    nMeasure *= _measure_adjustment; // apply measure adjustment --  applicable only to Bernoulli
    if (nMeasure && total_cases - nMeasure && ((total_cases - nMeasure) / (total_cases - nMeasure)))
        return (nCases / nMeasure) / ((total_cases - nCases) / (total_cases - nMeasure));
    return 0.0;
}

/* Returns the observed / expected -- this function is used as a risk function when restricting clusters by risk level.
   This function is currently used for either space-time permutation or exponential probability models. 
   Note also that the implementation is not the average of ratios but instead the ratio of averages, so summation is suitable here. */
inline double AbstractLikelihoodCalculator::getObservedDividedExpectedMultiset(count_t sumObserved, measure_t sumExpected, count_t sumTotalCases, measure_t sumTotalExpected) const {
    return sumExpected ? static_cast<double>(sumObserved) / sumExpected : 0.0;
}

/* Returns the relative risk -- this function is used as a risk function when restricting clusters by risk level. 
   Note also that the implementation is not the average of ratios but instead the ratio of averages, so summation is suitable here. */
inline double AbstractLikelihoodCalculator::getRelativeRiskMultiset(count_t sumObserved, measure_t sumExpected, count_t sumTotalCases, measure_t sumTotalExpected) const {
    // It's possible that none of the data sets were signigicant in evaluating cluster - which means that the total is zero.
    if (sumTotalCases == 0) return 0.0; 
    /* If the observed equals the total cases, then relative risk goes to infinity. */
    if (sumObserved == sumTotalCases) return std::numeric_limits<double>::max();
    if (sumExpected && sumTotalCases - sumExpected && ((sumTotalCases - sumExpected) / (sumTotalExpected - sumExpected)))
        return (sumObserved / sumExpected) / ((sumTotalCases - sumObserved) / (sumTotalExpected - sumExpected));
    return 0.0;
}

/* Returns whether potential cluster is high rate while exceeding high risk level minimum. */
inline bool AbstractLikelihoodCalculator::HighRisk(count_t nCases, measure_t nMeasure) const {
    return HighRate(nCases, nMeasure) && (this->*_risk_function)(nCases, nMeasure) >= _high_risk_threshold;
}

/* Returns whether potential cluster is low rate while not exceeding low risk level maximum. */
inline bool AbstractLikelihoodCalculator::LowRisk(count_t nCases, measure_t nMeasure) const {
    return LowRate(nCases, nMeasure) && (this->*_risk_function)(nCases, nMeasure) <= _low_risk_threshold;
}

/* Returns whether potential cluster is high rate or low rate while not exceeding low risk level maximum. */
inline bool AbstractLikelihoodCalculator::HighRateOrLowRisk(count_t nCases, measure_t nMeasure) const {
    if (HighRate(nCases, nMeasure))
        return true;
    if (LowRate(nCases, nMeasure) && (this->*_risk_function)(nCases, nMeasure) <= _low_risk_threshold)
        return true;
    return false;
}

/* Returns whether potential cluster is low rate or high rate while exceeding high risk level minimum. */
inline bool AbstractLikelihoodCalculator::HighRiskOrLowRate(count_t nCases, measure_t nMeasure) const {
    if (LowRate(nCases, nMeasure))
        return true;
    if (HighRate(nCases, nMeasure) && (this->*_risk_function)(nCases, nMeasure) >= _high_risk_threshold)
        return true;
    return false;
}

/* Returns whether potential cluster is low rate while not exceeding low risk level maximum or high rate while exceeding high risk level minimum. */
inline bool AbstractLikelihoodCalculator::HighRiskOrLowRisk(count_t nCases, measure_t nMeasure) const {
    double risk = (this->*_risk_function)(nCases, nMeasure);
    return (HighRate(nCases, nMeasure) && risk >= _high_risk_threshold) || (LowRate(nCases, nMeasure) && risk <= _low_risk_threshold);
}

/** Indicates whether an area has lower than expected cases for a clustering within a single dataset. */
inline bool AbstractLikelihoodCalculator::LowRate(count_t nCases, measure_t nMeasure) const {
   if (nMeasure == 0.0 || nCases < _min_low_rate_cases) return false;
   return nCases * gvDataSetTotals.front().second < nMeasure * gvDataSetTotals.front().first;
}

/** Indicates whether an area has high than expected cases for a clustering within a single dataset. Clusterings with less
    than two cases are not considered for high rates. Note this function should not be used for scannning for high rates with
    an analysis with multiple datasets; use HighRateDataStream() */
inline bool AbstractLikelihoodCalculator::HighRate(count_t nCases, measure_t nMeasure) const {
    // First check whether this cluster contains the minimum number of cases for a high rate cluster.
   if (nMeasure == 0.0 || nCases < _min_high_rate_cases) return false;
   // Now check whether this is a high rate relative to data set. 
   return nCases * gvDataSetTotals.front().second  > nMeasure * gvDataSetTotals.front().first;
}

/** Indicates whether an area has lower than expected cases for a clustering within a single dataset. */
inline bool AbstractLikelihoodCalculator::HighOrLowRate(count_t nCases, measure_t nMeasure) const {
   if (nMeasure == 0.0) return false;
   //check for high rate
   if (nCases >= _min_high_rate_cases && nCases * gvDataSetTotals.front().second > nMeasure * gvDataSetTotals.front().first)
       return true;
   //check for low rate
   else if (nCases >= _min_low_rate_cases && nCases * gvDataSetTotals.front().second < nMeasure * gvDataSetTotals.front().first)
       return true;
   else return false;
}

/** For multiple sets the criteria that a low rate must have a certain minimum is implemented across all data sets. */
inline bool AbstractLikelihoodCalculator::LowRateDataStream(count_t nCases, measure_t nMeasure, size_t tSetIndex) const {
    return nCases * gvDataSetTotals[tSetIndex].second < nMeasure * gvDataSetTotals[tSetIndex].first;
}

/** For multiple sets the criteria that a high rate must have a certain minimum is implemented across all data sets. */
inline bool AbstractLikelihoodCalculator::HighRateDataStream(count_t nCases, measure_t nMeasure, size_t tSetIndex) const {
   return nCases * gvDataSetTotals[tSetIndex].second  > nMeasure * gvDataSetTotals[tSetIndex].first;
}

/** Returns whether an area has the minimum number of cases for a low rate clustering within a single dataset. */
inline bool AbstractLikelihoodCalculator::LowRateBatchedExpected(count_t positiveBatches, measure_t expectedPositive) const {
    // First check whether this cluster contains the minimum number of cases for a high rate cluster.
    if (expectedPositive == 0.0 || positiveBatches < _min_high_rate_cases) return false;
    // Now check whether this is a high rate relative to data set. 
    return positiveBatches < expectedPositive;
}

/** Returns whether an area has the minimum number of cases for a low rate clustering within a single dataset. */
inline bool AbstractLikelihoodCalculator::LowRiskBatchedExpected(count_t positiveBatches, measure_t expectedPositive) const {
    return LowRateBatchedExpected(positiveBatches, expectedPositive) && (this->*_risk_function)(positiveBatches, expectedPositive) <= _low_risk_threshold;
}

/** Returns whether an area has the minimum number of cases for a high rate clustering within a single dataset. */
inline bool AbstractLikelihoodCalculator::HighRateBatchedExpected(count_t positiveBatches, measure_t expectedPositive) const {
    // First check whether this cluster contains the minimum number of cases for a high rate cluster.
    if (expectedPositive == 0.0 || positiveBatches < _min_high_rate_cases) return false;
    // Now check whether this is a high rate relative to data set. 
    return positiveBatches > expectedPositive;
}

/** Returns whether an area has the minimum number of cases for a high rate clustering within a single dataset. */
inline bool AbstractLikelihoodCalculator::HighRiskBatchedExpected(count_t positiveBatches, measure_t expectedPositive) const {
    return HighRateBatchedExpected(positiveBatches, expectedPositive) && (this->*_risk_function)(positiveBatches, expectedPositive) >= _high_risk_threshold;
}

/** Indicates whether an area has expected cases for a clustering within a single dataset. */
inline bool AbstractLikelihoodCalculator::HighOrLowRateBatchedExpected(count_t positiveBatches, measure_t expectedPositive) const {
    if (expectedPositive == 0.0) return false;
    //check for high rate
    if (positiveBatches >= _min_high_rate_cases && positiveBatches > expectedPositive)
        return true;
    //check for low rate
    else if (positiveBatches >= _min_low_rate_cases && positiveBatches < expectedPositive)
        return true;
    else return false;
}

/** Indicates whether an area has expected cases for a clustering within a single dataset. */
inline bool AbstractLikelihoodCalculator::HighRiskOrLowRateBatchedExpected(count_t positiveBatches, measure_t expectedPositive) const {
    //check for high rate and risk
    if (HighRateBatchedExpected(positiveBatches, expectedPositive) && (this->*_risk_function)(positiveBatches, expectedPositive) >= _high_risk_threshold)
        return true;
    //check for low rate
    else if (LowRateBatchedExpected(positiveBatches, expectedPositive))
        return true;
    else return false;
}

/** Indicates whether an area has expected cases for a clustering within a single dataset. */
inline bool AbstractLikelihoodCalculator::HighRateOrLowRiskBatchedExpected(count_t positiveBatches, measure_t expectedPositive) const {
    //check for high rate
    if (HighRateBatchedExpected(positiveBatches, expectedPositive))
        return true;
    //check for low rate and risk
    else if (LowRateBatchedExpected(positiveBatches, expectedPositive) && (this->*_risk_function)(positiveBatches, expectedPositive) <= _low_risk_threshold)
        return true;
    else return false;
}

/** Indicates whether an area has expected cases for a clustering within a single dataset. */
inline bool AbstractLikelihoodCalculator::HighRiskOrLowRiskBatchedExpected(count_t positiveBatches, measure_t expectedPositive) const {
    //check for high rate
    if (HighRateBatchedExpected(positiveBatches, expectedPositive) && (this->*_risk_function)(positiveBatches, expectedPositive) >= _high_risk_threshold)
        return true;
    //check for low rate and risk
    else if (LowRateBatchedExpected(positiveBatches, expectedPositive) && (this->*_risk_function)(positiveBatches, expectedPositive) <= _low_risk_threshold)
        return true;
    else return false;
}

/** Indicates whether an area has lower than expected cases for a clustering within a single dataset. */
inline bool AbstractLikelihoodCalculator::LowRateNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux) const {
   return LowRate(nCases, nMeasure);
}

/** Indicates whether an area has high than expected cases for a clustering
    within a single dataset. Clusterings with less than two cases are not
    considered for high rates. Note this function should not be used for scannning
    for high rates with an analysis with multiple datasets; use MultipleSetsHighRate() */
inline bool AbstractLikelihoodCalculator::HighRateNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux) const {
   return HighRate(nCases, nMeasure);
}
/** Indicates whether an area has lower than expected cases for a clustering
    within a single dataset. */
inline bool AbstractLikelihoodCalculator::HighOrLowRateNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux) const {
   return HighOrLowRate(nCases, nMeasure);
}

/** For multiple sets, the criteria that a high rate must have more than one case is not currently implemented. */
inline bool AbstractLikelihoodCalculator::HighRateNormalDataStream(count_t nCases, measure_t nMeasure, measure_t nMeasureAux,size_t tSetIndex) const {
   return HighRateDataStream(nCases, nMeasure, tSetIndex);
}

/** Indicates whether an area has lower than expected cases for a clustering within a single dataset. */
inline bool AbstractLikelihoodCalculator::LowRateWeightedNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux) const {
   if (nMeasure == 0 || nCases < _min_low_rate_cases) return false;
   return nMeasure/nMeasureAux < gvDataSetTotals.front().second/gvDataSetMeasureAuxTotals.front();
}

/** Indicates whether an area has high than expected cases for a clustering
    within a single dataset. Clusterings with less than two cases are not
    considered for high rates. Note this function should not be used for scannning
    for high rates with an analysis with multiple datasets; use MultipleSetsHighRate() */
inline bool AbstractLikelihoodCalculator::HighRateWeightedNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux) const {
   if (nMeasure == 0 || nCases < _min_high_rate_cases) return false;
   return nMeasure/nMeasureAux > gvDataSetTotals.front().second/gvDataSetMeasureAuxTotals.front();
}
/** Indicates whether an area has lower than expected cases for a clustering within a single dataset. */
inline bool AbstractLikelihoodCalculator::HighOrLowRateWeightedNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux) const {
   if (nMeasure == 0) return false;
   //check for high rate
   if (nCases >= _min_high_rate_cases && nMeasure/nMeasureAux > gvDataSetTotals.front().second/gvDataSetMeasureAuxTotals.front()) return true;
   //check for low rate
   else if (nCases >= _min_low_rate_cases && nMeasure/nMeasureAux < gvDataSetTotals.front().second/gvDataSetMeasureAuxTotals.front()) return true;
   else return false;
}

/** For multiple sets, the criteria that a high rate must have more than one case is not currently implemented. */
inline bool AbstractLikelihoodCalculator::MultipleSetsLowRateWeightedNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux, size_t tSetIndex) const {
    return nMeasure / nMeasureAux < gvDataSetTotals[tSetIndex].second / gvDataSetMeasureAuxTotals[tSetIndex];
}

/** For multiple sets, the criteria that a high rate must have more than one case is not currently implemented. */
inline bool AbstractLikelihoodCalculator::MultipleSetsHighRateWeightedNormal(count_t nCases, measure_t nMeasure, measure_t nMeasureAux,size_t tSetIndex) const {
   return (nMeasure/nMeasureAux > gvDataSetTotals[tSetIndex].second/gvDataSetMeasureAuxTotals[tSetIndex]);
}

/** Indicates whether an area has enough cases for a clustering within a single dataset for normal model with weights and covariates. */
inline bool AbstractLikelihoodCalculator::AllRatesWeightedNormalCovariates(count_t nCases, measure_t nMeasure, measure_t nMeasureAux) const {
   return nCases >= _min_high_rate_cases;
}
//******************************************************************************
#endif


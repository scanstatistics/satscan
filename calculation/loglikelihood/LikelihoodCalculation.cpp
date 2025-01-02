//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "LikelihoodCalculation.h"
#include "SaTScanData.h"
#include "LoglikelihoodRatioUnifier.h"
#include "SSException.h"
#include "newmat.h"

/** class constructor */
AbstractLikelihoodCalculator::AbstractLikelihoodCalculator(const CSaTScanData& DataHub): gDataHub(DataHub), _measure_adjustment(1.0),
    _min_low_rate_cases(DataHub.GetParameters().getMinimumCasesLowRateClusters()), _low_risk_threshold(DataHub.GetParameters().getRiskThresholdLowClusters()),
    _min_high_rate_cases(DataHub.GetParameters().getMinimumCasesHighRateClusters()), _high_risk_threshold(DataHub.GetParameters().getRiskThresholdHighClusters()),
    gpRateOfInterest(0), gpRateOfInterestNormal(0), gpRateOfInterestUniformTime(0), _risk_function(0), _risk_multiset_function(0), _rate_of_interest_multiset(0),
    gpRateOfInterestBatched(0) {

    try {
        const CParameters& parameters = DataHub.GetParameters();
        //store data set totals for later calculation
        for (size_t t=0; t < DataHub.GetDataSetHandler().GetNumDataSets(); ++t) {
            gvDataSetTotals.push_back(std::make_pair(DataHub.GetDataSetHandler().GetDataSet(t).getTotalCases(), DataHub.GetDataSetHandler().GetDataSet(t).getTotalMeasure()));
            gvDataSetMeasureAuxTotals.push_back(DataHub.GetDataSetHandler().GetDataSet(t).getTotalMeasureAux());
            gvDataSetMeasureAux2Totals.push_back(DataHub.GetDataSetHandler().GetDataSet(t).getTotalMeasureAux2());
        }
        /* Assign the class function pointers that will determine if cluster passes scan area test. */
        if (parameters.GetProbabilityModelType() == BATCHED) {
            switch (parameters.GetExecuteScanRateType()) {
                case LOW: gpRateOfInterestBatched = &AbstractLikelihoodCalculator::LowRateBatched; break;
                case HIGHANDLOW: gpRateOfInterestBatched = &AbstractLikelihoodCalculator::HighOrLowRateBatched; break;
                case HIGH:
                default: gpRateOfInterestBatched = &AbstractLikelihoodCalculator::HighRateBatched;
            }
        } else if (parameters.GetProbabilityModelType() == NORMAL) {
            /* The normal model is specialized. */
            if (!parameters.getIsWeightedNormal()) {
                switch (parameters.GetExecuteScanRateType()) {
                    case LOW: gpRateOfInterestNormal = &AbstractLikelihoodCalculator::LowRateNormal; break;
                    case HIGHANDLOW: gpRateOfInterestNormal = &AbstractLikelihoodCalculator::HighOrLowRateNormal; break;
                    case HIGH:
                    default: gpRateOfInterestNormal = &AbstractLikelihoodCalculator::HighRateNormal;
                }
            } else if (parameters.getIsWeightedNormalCovariates()) {
                gpRateOfInterestNormal = &AbstractLikelihoodCalculator::AllRatesWeightedNormalCovariates;
                /* The AllRatesWeightedNormalCovariates method only uses one variable since we can't determine rate until calculating LLR.
                   -- see WeightedNormalCovariatesLikelihoodCalculator::CalculateMaximizingValueNormal */
                if (parameters.GetAreaScanRateType() == LOW) _min_high_rate_cases = _min_low_rate_cases;
                // This isn't technically correct since we can't determine rate at evaluation time -- so we're limiting by the greater value.
                if (parameters.GetAreaScanRateType() == HIGHANDLOW) _min_high_rate_cases = std::max(_min_low_rate_cases, _min_high_rate_cases);
            } else if (parameters.getIsWeightedNormal()) {
                switch (parameters.GetExecuteScanRateType()) {
                    case LOW: gpRateOfInterestNormal = &AbstractLikelihoodCalculator::LowRateWeightedNormal; break;
                    case HIGHANDLOW: gpRateOfInterestNormal = &AbstractLikelihoodCalculator::HighOrLowRateWeightedNormal; break;
                    case HIGH:
                    default: gpRateOfInterestNormal = &AbstractLikelihoodCalculator::HighRateWeightedNormal;
                }
            } else
                throw prg_error("Unable to assign scan area function pointer.", "constructor()");
        } else if (parameters.GetProbabilityModelType() == RANK) {
            /* The rank model is specialized. */
            switch (parameters.GetExecuteScanRateType()) {
                case LOW: gpRateOfInterest = &AbstractLikelihoodCalculator::LowRateRank; break;
                case HIGHANDLOW: gpRateOfInterest = &AbstractLikelihoodCalculator::HighOrLowRateRank; break;
                case HIGH:
                default: gpRateOfInterest = &AbstractLikelihoodCalculator::HighRateRank;
            }
            // Calculate the average rank per data set.
            for (size_t t=0; t < DataHub.GetDataSetHandler().GetNumDataSets(); ++t)
                _average_rank_dataset.push_back(static_cast<double>(DataHub.GetDataSetHandler().GetDataSet(t).getTotalCases() + 1) / 2.0);
            /* TODO -- What is the risk function for the rank model? TODO -- What about multiple data sets? */
        } else if (parameters.GetProbabilityModelType() == UNIFORMTIME) {
            /* The uniform time model is specialized. */
            switch (parameters.GetExecuteScanRateType()) {
                case LOW: 
                    gpRateOfInterestUniformTime = parameters.getRiskLimitLowClusters() ? &AbstractLikelihoodCalculator::LowRiskUniformTime : &AbstractLikelihoodCalculator::LowRateUniformTime;
                    break;
                case HIGHANDLOW:
                    if (parameters.getRiskLimitLowClusters() && parameters.getRiskLimitHighClusters())
                        gpRateOfInterestUniformTime = &AbstractLikelihoodCalculator::HighRiskOrLowRiskUniformTime;
                    else if (parameters.getRiskLimitLowClusters())
                        gpRateOfInterestUniformTime = &AbstractLikelihoodCalculator::HighRateOrLowRiskUniformTime;
                    else if (parameters.getRiskLimitHighClusters())
                        gpRateOfInterestUniformTime = &AbstractLikelihoodCalculator::HighRiskOrLowRateUniformTime;
                    else
                        gpRateOfInterestUniformTime = &AbstractLikelihoodCalculator::HighOrLowRateUniformTime;
                    break;
                case HIGH:
                default: gpRateOfInterestUniformTime = parameters.getRiskLimitHighClusters() ? &AbstractLikelihoodCalculator::HighRiskUniformTime : &AbstractLikelihoodCalculator::HighRateUniformTime;
            }
        } if (parameters.GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION) {
            /* Specialized for time stratification temporal adjustment. */
            switch (parameters.GetExecuteScanRateType()) {
                case LOW:
                    gpRateOfInterestTimeStratified = parameters.getRiskLimitLowClusters() ? &AbstractLikelihoodCalculator::LowRiskTimeStratified : &AbstractLikelihoodCalculator::LowRateTimeStratified;
                    if (parameters.getRiskLimitLowClusters())
                        _rate_of_interest_multiset = &AbstractLikelihoodCalculator::LowRiskMultiset;
                    else
                        _rate_of_interest_multiset = &AbstractLikelihoodCalculator::LowRateMultiset;
                    break;
                case HIGHANDLOW:
                    if (parameters.getRiskLimitLowClusters() && parameters.getRiskLimitHighClusters()) {
                        gpRateOfInterestTimeStratified = &AbstractLikelihoodCalculator::HighRiskOrLowRiskTimeStratified;
                        if (parameters.GetMultipleDataSetPurposeType() == ADJUSTMENT)
                            _rate_of_interest_multiset = &AbstractLikelihoodCalculator::AdjustmentHighRiskOrLowRiskMultiset;
                        else
                            _rate_of_interest_multiset = &AbstractLikelihoodCalculator::MultivariateHighRiskOrLowRiskMultiset;
                    }
                    else if (parameters.getRiskLimitLowClusters()) {
                        gpRateOfInterestTimeStratified = &AbstractLikelihoodCalculator::HighRateOrLowRiskTimeStratified;
                        if (parameters.GetMultipleDataSetPurposeType() == ADJUSTMENT)
                            _rate_of_interest_multiset = &AbstractLikelihoodCalculator::AdjustmentHighRateOrLowRiskMultiset;
                        else
                            _rate_of_interest_multiset = &AbstractLikelihoodCalculator::MultivariateHighRateOrLowRiskMultiset;
                    }
                    else if (parameters.getRiskLimitHighClusters()) {
                        gpRateOfInterestTimeStratified = &AbstractLikelihoodCalculator::HighRiskOrLowRateTimeStratified;
                        if (parameters.GetMultipleDataSetPurposeType() == ADJUSTMENT)
                            _rate_of_interest_multiset = &AbstractLikelihoodCalculator::AdjustmentHighRiskOrLowRateMultiset;
                        else
                            _rate_of_interest_multiset = &AbstractLikelihoodCalculator::MultivariateHighRiskOrLowRateMultiset;
                    }
                    else {
                        gpRateOfInterestTimeStratified = &AbstractLikelihoodCalculator::HighOrLowRateTimeStratified;
                        if (parameters.GetMultipleDataSetPurposeType() == ADJUSTMENT)
                            _rate_of_interest_multiset = &AbstractLikelihoodCalculator::AdjustmentHighOrLowRateMultiset;
                        else
                            _rate_of_interest_multiset = &AbstractLikelihoodCalculator::MultivariateHighOrLowRateMultiset;
                    } break;
                case HIGH:
                default: 
                    gpRateOfInterestTimeStratified = parameters.getRiskLimitHighClusters() ? &AbstractLikelihoodCalculator::HighRiskTimeStratified : &AbstractLikelihoodCalculator::HighRateTimeStratified;
                    if (parameters.getRiskLimitHighClusters())
                        _rate_of_interest_multiset = &AbstractLikelihoodCalculator::HighRiskMultiset;
                    else
                        _rate_of_interest_multiset = &AbstractLikelihoodCalculator::HighRateMultiset;
            }
            _risk_multiset_function = &AbstractLikelihoodCalculator::getRelativeRiskMultiset;
        } else {
            // Determine measure adjustment when restricting evaluated clusters by risk thresholds - this variable is only for singl data set.
            if ((parameters.getRiskLimitLowClusters() || parameters.getRiskLimitHighClusters()) && parameters.GetProbabilityModelType() == BERNOULLI)
                _measure_adjustment = gvDataSetTotals.front().first / gvDataSetTotals.front().second;
            /* The class function pointer for scan area is dependent on whether we're restricting cluster by rate only or both rate and risk level. */
            switch (parameters.GetExecuteScanRateType()) {
                case LOW:
                    gpRateOfInterest = parameters.getRiskLimitLowClusters() ? &AbstractLikelihoodCalculator::LowRisk : &AbstractLikelihoodCalculator::LowRate;
                    if (parameters.getRiskLimitLowClusters())
                        _rate_of_interest_multiset = &AbstractLikelihoodCalculator::LowRiskMultiset;
                    else
                        _rate_of_interest_multiset = &AbstractLikelihoodCalculator::LowRateMultiset;
                    break;
                case HIGHANDLOW: 
                    if (parameters.getRiskLimitLowClusters() && parameters.getRiskLimitHighClusters()) {
                        gpRateOfInterest = &AbstractLikelihoodCalculator::HighRiskOrLowRisk;
                        if (parameters.GetMultipleDataSetPurposeType() == ADJUSTMENT)
                            _rate_of_interest_multiset = &AbstractLikelihoodCalculator::AdjustmentHighRiskOrLowRiskMultiset;
                        else
                            _rate_of_interest_multiset = &AbstractLikelihoodCalculator::MultivariateHighRiskOrLowRiskMultiset;
                    } else if (parameters.getRiskLimitLowClusters()) {
                        gpRateOfInterest = &AbstractLikelihoodCalculator::HighRateOrLowRisk;
                        if (parameters.GetMultipleDataSetPurposeType() == ADJUSTMENT)
                            _rate_of_interest_multiset = &AbstractLikelihoodCalculator::AdjustmentHighRateOrLowRiskMultiset;
                        else
                            _rate_of_interest_multiset = &AbstractLikelihoodCalculator::MultivariateHighRateOrLowRiskMultiset;
                    } else if (parameters.getRiskLimitHighClusters()) {
                        gpRateOfInterest = &AbstractLikelihoodCalculator::HighRiskOrLowRate;
                        if (parameters.GetMultipleDataSetPurposeType() == ADJUSTMENT)
                            _rate_of_interest_multiset = &AbstractLikelihoodCalculator::AdjustmentHighRiskOrLowRateMultiset;
                        else
                            _rate_of_interest_multiset = &AbstractLikelihoodCalculator::MultivariateHighRiskOrLowRateMultiset;
                    } else {
                        gpRateOfInterest = &AbstractLikelihoodCalculator::HighOrLowRate;
                        if (parameters.GetMultipleDataSetPurposeType() == ADJUSTMENT)
                            _rate_of_interest_multiset = &AbstractLikelihoodCalculator::AdjustmentHighOrLowRateMultiset;
                        else
                            _rate_of_interest_multiset = &AbstractLikelihoodCalculator::MultivariateHighOrLowRateMultiset;
                    } break;
                case HIGH:
                    gpRateOfInterest = parameters.getRiskLimitHighClusters() ? &AbstractLikelihoodCalculator::HighRisk : &AbstractLikelihoodCalculator::HighRate;
                    if (parameters.getRiskLimitHighClusters())
                        _rate_of_interest_multiset = &AbstractLikelihoodCalculator::HighRiskMultiset;
                    else 
                        _rate_of_interest_multiset = &AbstractLikelihoodCalculator::HighRateMultiset;
                    break;
                default: throw prg_error("Unknown area scan type '%d'.", "constructor()", parameters.GetExecuteScanRateType());
            }
            /* The relative risk function is dependent on the probability model since the space-time permutation and exponential models do not report relative risk. */
            if (parameters.GetProbabilityModelType() == SPACETIMEPERMUTATION || parameters.GetProbabilityModelType() == EXPONENTIAL) {
                _risk_function = &AbstractLikelihoodCalculator::getObservedDividedExpected;
                _risk_multiset_function = &AbstractLikelihoodCalculator::getObservedDividedExpectedMultiset;
            } else {
                _risk_function = &AbstractLikelihoodCalculator::getRelativeRisk;
                _risk_multiset_function = &AbstractLikelihoodCalculator::getRelativeRiskMultiset;
            }
        }
        /* Determine unification class when analysis is using multiple data sets. */
        if (DataHub.GetNumDataSets() > 1) {
            switch (parameters.GetMultipleDataSetPurposeType()) {
                case MULTIVARIATE :
                    switch (parameters.GetExecuteScanRateType()) {
                        case HIGH: _unifier.reset(new MultivariateUnifierHighRate(DataHub.GetNumDataSets(), parameters.GetProbabilityModelType())); break;
                        case LOW: _unifier.reset(new MultivariateUnifierLowRate(DataHub.GetNumDataSets(), parameters.GetProbabilityModelType())); break;
                        case HIGHANDLOW: _unifier.reset(new MultivariateUnifierHighLowRate(DataHub.GetNumDataSets(), parameters.GetProbabilityModelType())); break;
                        default: throw prg_error("Unknown area scan type '%d'.", "constructor()", parameters.GetExecuteScanRateType());
                    } break;
                case ADJUSTMENT :
                    if (parameters.GetIsSpaceTimeAnalysis() && parameters.GetProbabilityModelType() == BATCHED && parameters.GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION)
                        _unifier.reset(new AdjustmentUnifierBatchModelTimeStratified(parameters.GetExecuteScanRateType()));
                    else if (parameters.getRiskLimitHighClusters() || parameters.getRiskLimitLowClusters())
                        _unifier.reset(new AdjustmentUnifierRiskThreshold(parameters.GetExecuteScanRateType(), parameters.GetProbabilityModelType()));
                    else
                        _unifier.reset(new AdjustmentUnifier(parameters.GetExecuteScanRateType(), parameters.GetProbabilityModelType()));
                    break;
                default : throw prg_error("Unknown purpose for multiple data sets '%d'.","constructor()", parameters.GetMultipleDataSetPurposeType());
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("constructor()","AbstractLikelihoodCalculator");
        throw;
    }
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalcLogLikelihood(count_t, measure_t) const {
    throw prg_error("CalcLogLikelihood(count_t,measure_t) not implementated.","AbstractLikelihoodCalculator");
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalcLogLikelihoodRatio(count_t, measure_t,size_t) const {
    throw prg_error("CalcLogLikelihoodRatio(count_t,measure_t.size_t) not implementated.","AbstractLikelihoodCalculator");
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalcLogLikelihoodRatioUniformTime(count_t tCases, measure_t tMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) const {
    throw prg_error("CalcLogLikelihoodRatioUniformTime(count_t,measure_t,count_t,measure_t,size_t) not implementated.", "AbstractLikelihoodCalculator");
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalcLogLikelihoodRatioOrdinal(const std::vector<count_t>& vOrdinalCases, size_t tSetIndex) const {
    throw prg_error("CalcLogLikelihoodRatioOrdinal(const std::vector<count_t>&,const std::vector<count_t>&) not implementated.","AbstractLikelihoodCalculator");
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalcLogLikelihoodRatioNormal(count_t tCases, measure_t tMeasure, measure_t tMeasure2, size_t tSetIndex) const {
    throw prg_error("CalcLogLikelihoodRatioNormal(count_t,measure_t,measure_t,count_t,measure_t,measure_t) not implementated.","AbstractLikelihoodCalculator");
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalcLogLikelihoodRatioNormal(Matrix& xg, Matrix& tobeinversed, Matrix& xgsigmaw, size_t tDataSetIndex) const {
    throw prg_error("CalcLogLikelihoodRatioNormal(Matrix&,Matrix&,Matrix&,size_t) not implementated.","AbstractLikelihoodCalculator");
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalculateMaximizingValueNormal(Matrix& xg, Matrix& tobeinversed, Matrix& xgsigmaw, size_t tDataSetIndex) const {
    throw prg_error("CalcLogLikelihoodRatioNormal(Matrix&,Matrix&,Matrix&,size_t) not implementated.","AbstractLikelihoodCalculator");
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalcMonotoneLogLikelihood(tract_t, const std::vector<count_t>&, const std::vector<measure_t>&) const {
    throw prg_error("CalcMonotoneLogLikelihood(tract_t, const std::vector<count_t>&, const std::vector<measure_t>&) not implementated.","AbstractLikelihoodCalculator");
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalcSVTTLogLikelihood(size_t, SVTTClusterData&, const AbstractTimeTrend&) const {
    throw prg_error("CalcSVTTLogLikelihood(size_t, CSVTTCluster*, const AbstractTimeTrend&) not implementated.","AbstractLikelihoodCalculator");
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalcLogLikelihoodUniformTime(count_t cases, measure_t measure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) const {
    throw prg_error("CalcLogLikelihoodUniformTime(count_t, measure_t, count_t, measure_t, size_t) not implementated.", "AbstractLikelihoodCalculator");
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalculateFullStatistic(double dMaximizingValue, size_t tDataSetIndex) const {
    throw prg_error("CalculateFullStatistic(double_t,size_t) not implementated.","AbstractLikelihoodCalculator");
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalculateMaximizingValue(count_t n, measure_t u, size_t tDataSetIndex) const {
    throw prg_error("CalculateMaximizingValue(count_t,measure_t,size_t) not implementated.","AbstractLikelihoodCalculator");
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalculateMaximizingValueNormal(count_t n, measure_t u, measure_t u2, size_t tDataSetIndex) const {
    throw prg_error("CalculateMaximizingValueNormal(count_t,measure_t,measure_t,size_t) not implementated.","AbstractLikelihoodCalculator");
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalculateMaximizingValueOrdinal(const std::vector<count_t>& vOrdinalCases, size_t tSetIndex) const {
    throw prg_error("CalculateMaximizingValueOrdinal(const std::vector<count_t>&,size_t) not implementated.","AbstractLikelihoodCalculator");
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalculateMaximizingValueUniformTime(count_t cases, measure_t measure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) const {
    throw prg_error("CalculateMaximizingValueUniformTime(count_t, measure_t, count_t, measure_t, size_t) not implementated.", "AbstractLikelihoodCalculator");
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalcLogLikelihoodTimeStratified(count_t n, measure_t u, count_t N, measure_t U) const {
    throw prg_error("CalcLogLikelihoodTimeStratified(count_t, measure_t, count_t, measure_t) not implementated.", "AbstractLikelihoodCalculator");
}

/** Throws exception. Not implemented in base class */
double AbstractLikelihoodCalculator::CalcLogLikelihoodBernoulliSpatialStratified(count_t n, measure_t u, tract_t tract, size_t setIdx) const {
    throw prg_error("CalcLogLikelihoodBernoulliSpatialStratified(count_t, measure_t, tract_t, size_t) not implementated.", "AbstractLikelihoodCalculator");
}

/** returns log likelihood for total - not implemented - throws exception. */
double AbstractLikelihoodCalculator::GetLogLikelihoodForTotal(size_t) const {
    throw prg_error("GetLogLikelihoodForTotal(size_t) not implementated.","AbstractLikelihoodCalculator");
}

/** Returns reference to AbstractLoglikelihoodRatioUnifier object. Throw exception if object not allocated. */
AbstractLoglikelihoodRatioUnifier & AbstractLikelihoodCalculator::GetUnifier() const {
    if (!_unifier.get()) throw prg_error("Log likelihood unifier not allocated.","GetUnifier()");
    return *_unifier.get();
}

/* Returns whether potential cluster exceeds the minimum cases for a high rate cluster when scanning mulitple data sets. */
bool AbstractLikelihoodCalculator::HighRateMultiset(const AbstractLoglikelihoodRatioUnifier& unifier, bool nonparametric) const {
    return unifier.getObservedCount() >= _min_high_rate_cases;
}

/* Returns whether potential cluster exceeds the minimum cases for a low rate cluster when scanning mulitple data sets. */
bool AbstractLikelihoodCalculator::LowRateMultiset(const AbstractLoglikelihoodRatioUnifier& unifier, bool nonparametric) const {
    return unifier.getObservedCount() >= _min_low_rate_cases;
}

/* Returns whether potential cluster exceeds the minimum cases for a high rate cluster and is exceeding risk threshold. */
bool AbstractLikelihoodCalculator::HighRiskMultiset(const AbstractLoglikelihoodRatioUnifier& unifier, bool nonparametric) const {
    return unifier.getObservedCount() >= _min_high_rate_cases &&
        (this->*_risk_multiset_function)(
            unifier.getObserved(), unifier.getExpected(), unifier.getCaseTotal(), nonparametric ? unifier.getTotalExpected() : unifier.getCaseTotal()
        ) >= _high_risk_threshold;
}

/* Returns whether potential cluster exceeds the minimum cases for a low rate cluster and is meeting risk threshold for low rate clusters. */
bool AbstractLikelihoodCalculator::LowRiskMultiset(const AbstractLoglikelihoodRatioUnifier& unifier, bool nonparametric) const {
    return unifier.getObservedCount() >= _min_low_rate_cases &&
        (this->*_risk_multiset_function)(
            unifier.getObserved(), unifier.getExpected(), unifier.getCaseTotal(), nonparametric ? unifier.getTotalExpected() : unifier.getCaseTotal()
        ) <= _low_risk_threshold;
}

/* Returns whether potential cluster meets one of these restrictions when multiple data set purpose is Adjustment:
   - high rate / meets minimum number of high rate cases threshold
   - low rate / meets minimum number of low rate cases threshold */
bool AbstractLikelihoodCalculator::AdjustmentHighOrLowRateMultiset(const AbstractLoglikelihoodRatioUnifier& unifier, bool nonparametric) const {
    if (unifier.GetRawLoglikelihoodRatio() >= 0.0) // high rate cluster
        return unifier.getObservedCount() >= _min_high_rate_cases;
    // low rate cluster
    return unifier.getObservedCount() >= _min_low_rate_cases;
}

/* Returns whether potential cluster meets one of these restrictions when multiple data set purpose is Adjustment:
- high rate / meets minimum number of high rate cases threshold
- low rate / meets minimum number of low rate cases threshold / meets minimum low rate relative risk threshold */
bool AbstractLikelihoodCalculator::AdjustmentHighRateOrLowRiskMultiset(const AbstractLoglikelihoodRatioUnifier& unifier, bool nonparametric) const {
    if (unifier.GetRawLoglikelihoodRatio() >= 0.0) // high rate cluster
        return unifier.getObservedCount() >= _min_high_rate_cases;
    // low rate cluster
    return unifier.getObservedCount() >= _min_low_rate_cases &&
        (this->*_risk_multiset_function)(
            unifier.getObserved(), unifier.getExpected(), unifier.getCaseTotal(), nonparametric ? unifier.getTotalExpected() : unifier.getCaseTotal()
        ) <= _low_risk_threshold;
}

/* Returns whether potential cluster meets one of these restrictions when multiple data set purpose is Adjustment:
- high rate / meets minimum number of high rate cases threshold / meets minimum high rate relative risk threshold
- low rate / meets minimum number of low rate cases threshold */
bool AbstractLikelihoodCalculator::AdjustmentHighRiskOrLowRateMultiset(const AbstractLoglikelihoodRatioUnifier& unifier, bool nonparametric) const {
    if (unifier.GetRawLoglikelihoodRatio() >= 0.0) // high rate cluster
        return unifier.getObservedCount() >= _min_high_rate_cases &&
            (this->*_risk_multiset_function)(
                unifier.getObserved(), unifier.getExpected(), unifier.getCaseTotal(), nonparametric ? unifier.getTotalExpected() : unifier.getCaseTotal()
            ) >= _high_risk_threshold;
    // low rate cluster
    return unifier.getObservedCount() >= _min_low_rate_cases;
}

/* Returns whether potential cluster meets one of these restrictions when multiple data set purpose is Adjustment:
- high rate / meets minimum number of high rate cases threshold / meets minimum high rate relative risk threshold
- low rate / meets minimum number of low rate cases threshold / meets minimum low rate relative risk threshold */
bool AbstractLikelihoodCalculator::AdjustmentHighRiskOrLowRiskMultiset(const AbstractLoglikelihoodRatioUnifier& unifier, bool nonparametric) const {
    if (unifier.GetRawLoglikelihoodRatio() >= 0.0) // high rate cluster
        return unifier.getObservedCount() >= _min_high_rate_cases &&
        (this->*_risk_multiset_function)(
            unifier.getObserved(), unifier.getExpected(), unifier.getCaseTotal(), nonparametric ? unifier.getTotalExpected() : unifier.getCaseTotal()
        ) >= _high_risk_threshold;
    // low rate cluster
    return unifier.getObservedCount() >= _min_low_rate_cases &&
        (this->*_risk_multiset_function)(
            unifier.getObserved(), unifier.getExpected(), unifier.getCaseTotal(), nonparametric ? unifier.getTotalExpected() : unifier.getCaseTotal()
        ) <= _low_risk_threshold;
}

/* Returns whether potential cluster meets one of these restrictions when multiple data set purpose is Multivariate:
- high rate / meets minimum number of high rate cases threshold
- low rate / meets minimum number of low rate cases threshold */
bool AbstractLikelihoodCalculator::MultivariateHighOrLowRateMultiset(const AbstractLoglikelihoodRatioUnifier& unifier, bool nonparametric) const {
    AbstractLoglikelihoodRatioUnifier::AccumulationPair_t highPair = unifier.getHighRateAccumulationPair();
    AbstractLoglikelihoodRatioUnifier::AccumulationPair_t lowPair = unifier.getLowRateAccumulationPair();
    // See which accumulation is the greatest to determine whether this is a high or low rate cluster.
    if (lowPair.first > highPair.first)
        return lowPair.second->getObserved() >= _min_low_rate_cases;
    return highPair.second->getObserved() >= _min_high_rate_cases;
}

/* Returns whether potential cluster meets one of these restrictions when multiple data set purpose is Multivariate:
- high rate / meets minimum number of high rate cases threshold
- low rate / meets minimum number of low rate cases threshold / meets minimum low rate relative risk threshold */
bool AbstractLikelihoodCalculator::MultivariateHighRateOrLowRiskMultiset(const AbstractLoglikelihoodRatioUnifier& unifier, bool nonparametric) const {
    AbstractLoglikelihoodRatioUnifier::AccumulationPair_t highPair = unifier.getHighRateAccumulationPair();
    AbstractLoglikelihoodRatioUnifier::AccumulationPair_t lowPair = unifier.getLowRateAccumulationPair();
    // See which accumulation is the greatest to determine whether this is a high or low rate cluster.
    if (lowPair.first > highPair.first)
        return lowPair.second->getObserved() >= _min_low_rate_cases && 
          (this->*_risk_multiset_function)(
              lowPair.second->getObserved(), lowPair.second->getExpected(), lowPair.second->getCaseTotal(), nonparametric ? lowPair.second->getTotalExpected() : lowPair.second->getCaseTotal()
          ) <= _low_risk_threshold;
    return highPair.second->getObserved() >= _min_high_rate_cases;
}

/* Returns whether potential cluster meets one of these restrictions when multiple data set purpose is Multivariate:
- high rate / meets minimum number of high rate cases threshold / meets minimum high rate relative risk threshold
- low rate / meets minimum number of low rate cases threshold */
bool AbstractLikelihoodCalculator::MultivariateHighRiskOrLowRateMultiset(const AbstractLoglikelihoodRatioUnifier& unifier, bool nonparametric) const {
    AbstractLoglikelihoodRatioUnifier::AccumulationPair_t highPair = unifier.getHighRateAccumulationPair();
    AbstractLoglikelihoodRatioUnifier::AccumulationPair_t lowPair = unifier.getLowRateAccumulationPair();
    // See which accumulation is the greatest to determine whether this is a high or low rate cluster.
    if (lowPair.first > highPair.first)
        return lowPair.second->getObserved() >= _min_low_rate_cases;
    return highPair.second->getObserved() >= _min_high_rate_cases &&
        (this->*_risk_multiset_function)(
            highPair.second->getObserved(), highPair.second->getExpected(), highPair.second->getCaseTotal(), nonparametric ? highPair.second->getTotalExpected() : highPair.second->getCaseTotal()
        ) >= _high_risk_threshold;
}

/* Returns whether potential cluster meets one of these restrictions when multiple data set purpose is Multivariate:
- high rate / meets minimum number of high rate cases threshold / meets minimum high rate relative risk threshold
- low rate / meets minimum number of low rate cases threshold / meets minimum low rate relative risk threshold */
bool AbstractLikelihoodCalculator::MultivariateHighRiskOrLowRiskMultiset(const AbstractLoglikelihoodRatioUnifier& unifier, bool nonparametric) const {
    AbstractLoglikelihoodRatioUnifier::AccumulationPair_t highPair = unifier.getHighRateAccumulationPair();
    AbstractLoglikelihoodRatioUnifier::AccumulationPair_t lowPair = unifier.getLowRateAccumulationPair();
    // See which accumulation is the greatest to determine whether this is a high or low rate cluster.
    if (lowPair.first > highPair.first)
        return lowPair.second->getObserved() >= _min_low_rate_cases && 
          (this->*_risk_multiset_function)(
              lowPair.second->getObserved(), lowPair.second->getExpected(), lowPair.second->getCaseTotal(), nonparametric ? lowPair.second->getTotalExpected() : lowPair.second->getCaseTotal()
          ) <= _low_risk_threshold;
    return highPair.second->getObserved() >= _min_high_rate_cases &&
        (this->*_risk_multiset_function)(
            highPair.second->getObserved(), highPair.second->getExpected(), highPair.second->getCaseTotal(), nonparametric ? highPair.second->getTotalExpected() : highPair.second->getCaseTotal()
        ) >= _high_risk_threshold;
}

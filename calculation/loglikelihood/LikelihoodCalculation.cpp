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
AbstractLikelihoodCalculator::AbstractLikelihoodCalculator(const CSaTScanData& DataHub):
    gDataHub(DataHub), gpRateOfInterest(0), gpRateOfInterestNormal(0), gpRateOfInterestUniformTime(0), _low_risk_threshold(0.0), _high_risk_threshold(0.0), _measure_adjustment(1.0) {

    try {
        const CParameters& parameters = DataHub.GetParameters();

        //store data set totals for later calculation
        for (size_t t=0; t < DataHub.GetDataSetHandler().GetNumDataSets(); ++t) {
            gvDataSetTotals.push_back(std::make_pair(DataHub.GetDataSetHandler().GetDataSet(t).getTotalCases(), DataHub.GetDataSetHandler().GetDataSet(t).getTotalMeasure()));
            gvDataSetMeasureAuxTotals.push_back(DataHub.GetDataSetHandler().GetDataSet(t).getTotalMeasureAux());
        }
        _min_low_rate_cases = parameters.getMinimumCasesLowRateClusters();
        _min_high_rate_cases = parameters.getMinimumCasesHighRateClusters();
        /* Assign the class function pointer that will determine if cluster passes scan area test. */
        if (parameters.GetProbabilityModelType() == NORMAL) {
            /* The normal model is somewhat specialized. */
            if (!parameters.getIsWeightedNormal()) {
                switch (parameters.GetExecuteScanRateType()) {
                case LOW: gpRateOfInterestNormal = &AbstractLikelihoodCalculator::LowRateNormal; break;
                case HIGHANDLOW: gpRateOfInterestNormal = &AbstractLikelihoodCalculator::HighOrLowRateNormal; break;
                case HIGH:
                default: gpRateOfInterestNormal = &AbstractLikelihoodCalculator::HighRateNormal;
                }
            }
            else if (parameters.getIsWeightedNormalCovariates()) {
                gpRateOfInterestNormal = &AbstractLikelihoodCalculator::AllRatesWeightedNormalCovariates;
                /* The AllRatesWeightedNormalCovariates method only uses one variable since we can't determine rate until calculating LLR.
                   -- see WeightedNormalCovariatesLikelihoodCalculator::CalculateMaximizingValueNormal */
                if (parameters.GetAreaScanRateType() == LOW) _min_high_rate_cases = _min_low_rate_cases;
                // This isn't technically correct since we can't determine rate at evaluation time -- so we're limiting by the greater value.
                if (parameters.GetAreaScanRateType() == HIGHANDLOW) _min_high_rate_cases = std::max(_min_low_rate_cases, _min_high_rate_cases);
            }
            else if (parameters.getIsWeightedNormal()) {
                switch (parameters.GetExecuteScanRateType()) {
                case LOW: gpRateOfInterestNormal = &AbstractLikelihoodCalculator::LowRateWeightedNormal; break;
                case HIGHANDLOW: gpRateOfInterestNormal = &AbstractLikelihoodCalculator::HighOrLowRateWeightedNormal; break;
                case HIGH:
                default: gpRateOfInterestNormal = &AbstractLikelihoodCalculator::HighRateWeightedNormal;
                }
            }
            else
                throw prg_error("Unable to assign scan area function pointer.", "constructor()");
        } else if (parameters.GetProbabilityModelType() == RANK) {
            /* The rank model is somewhat specialized. */
            switch (parameters.GetExecuteScanRateType()) {
            case LOW: gpRateOfInterest = &AbstractLikelihoodCalculator::LowRateRank; break;
            case HIGHANDLOW: gpRateOfInterest = &AbstractLikelihoodCalculator::HighOrLowRateRank; break;
            case HIGH:
            default: gpRateOfInterest = &AbstractLikelihoodCalculator::HighRateRank;
            }
            // Calculate the average rank per data set.
            for (size_t t=0; t < DataHub.GetDataSetHandler().GetNumDataSets(); ++t)
                _average_rank_dataset.push_back(static_cast<double>(DataHub.GetDataSetHandler().GetDataSet(t).getTotalCases() + 1) / 2.0);

            /* TODO -- What is the risk function for the rank model? */
            /* TODO -- What about multiple data sets? */
        }
        else if (parameters.GetProbabilityModelType() == UNIFORMTIME) {
            /* The rank model is somewhat specialized. */
            switch (parameters.GetExecuteScanRateType()) {
            case LOW: gpRateOfInterestUniformTime = &AbstractLikelihoodCalculator::LowRateUniformTime; break;
            case HIGHANDLOW: gpRateOfInterestUniformTime = &AbstractLikelihoodCalculator::HighOrLowRateUniformTime; break;
            case HIGH:
            default: gpRateOfInterestUniformTime = &AbstractLikelihoodCalculator::HighRateUniformTime;
            }
        } else {
            // Determine measure adjustment when restricting evaluated clusters by risk thresholds.
            if ((parameters.getRiskLimitLowClusters() || parameters.getRiskLimitHighClusters()) && parameters.GetProbabilityModelType() == BERNOULLI)
                _measure_adjustment = gvDataSetTotals.front().first / gvDataSetTotals.front().second;
            _low_risk_threshold = parameters.getRiskThresholdLowClusters();
            _high_risk_threshold = parameters.getRiskThresholdHighClusters();
            /* The class function pointer for scan area is dependent on whether we're restricting cluster by rate only or both rate and risk level. */
            switch (parameters.GetExecuteScanRateType()) {
                case LOW:
                    gpRateOfInterest = parameters.getRiskLimitLowClusters() ? &AbstractLikelihoodCalculator::LowRisk : &AbstractLikelihoodCalculator::LowRate;
                    break;
                case HIGHANDLOW: 
                    if (parameters.getRiskLimitLowClusters() && parameters.getRiskLimitHighClusters()) {
                        gpRateOfInterest = &AbstractLikelihoodCalculator::HighRiskOrLowRisk;
                    } else if (parameters.getRiskLimitLowClusters()) {
                        gpRateOfInterest = &AbstractLikelihoodCalculator::HighRateOrLowRisk;
                    } else if (parameters.getRiskLimitHighClusters()) {
                        gpRateOfInterest = &AbstractLikelihoodCalculator::HighRiskOrLowRate;
                    } else {
                        gpRateOfInterest = &AbstractLikelihoodCalculator::HighOrLowRate;
                    }
                    break;
                case HIGH:
                    gpRateOfInterest = parameters.getRiskLimitHighClusters() ? &AbstractLikelihoodCalculator::HighRisk : &AbstractLikelihoodCalculator::HighRate;
                    break;
                default: throw prg_error("Unknown area scan type '%d'.", "constructor()", parameters.GetExecuteScanRateType());
            }
            /* The risk function is dependent on the probability model since the space-time permutation and exponential models do not report relative risk. */
            if (parameters.GetProbabilityModelType() == SPACETIMEPERMUTATION || parameters.GetProbabilityModelType() == EXPONENTIAL)
                _risk_function = &AbstractLikelihoodCalculator::getObservedDividedExpected;
            else
                _risk_function = &AbstractLikelihoodCalculator::getRelativeRisk;
        }
        /* Assign class function pointer which unifies log likelihoods of all data sets. */
        if (parameters.GetNumDataSets() > 1) {
            switch (parameters.GetMultipleDataSetPurposeType()) {
                case MULTIVARIATE :
                    _unifier.reset(new MultivariateUnifier(parameters.GetExecuteScanRateType(), parameters.GetProbabilityModelType())); break;
                case ADJUSTMENT :
                    _unifier.reset(new AdjustmentUnifier(parameters.GetExecuteScanRateType())); break;
                default :
                    throw prg_error("Unknown purpose for multiple data sets '%d'.","constructor()", parameters.GetMultipleDataSetPurposeType());
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

/** returns log likelihood for total - not implemented - throws exception. */
double AbstractLikelihoodCalculator::GetLogLikelihoodForTotal(size_t) const {
    throw prg_error("GetLogLikelihoodForTotal(size_t) not implementated.","AbstractLikelihoodCalculator");
}

/** Returns reference to AbstractLoglikelihoodRatioUnifier object. Throw exception
    if object not allocated. */
AbstractLoglikelihoodRatioUnifier & AbstractLikelihoodCalculator::GetUnifier() const {
    if (!_unifier.get())
        throw prg_error("Log likelihood unifier not allocated.","GetUnifier()");
    return *_unifier.get();
}


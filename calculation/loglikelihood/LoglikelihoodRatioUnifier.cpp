//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "LoglikelihoodRatioUnifier.h"
#include "LikelihoodCalculation.h"
#include "OrdinalLikelihoodCalculation.h"
#include "SaTScanData.h"


////////////////////////////////// MultivariateUnifierHighRate //////////////////////////////////////

/** Calculates loglikelihood ratio given cluster data; adding log likelihood ratio to accumulation.
    Also maintains data stream accumulation for risk threshold restriction. */
void MultivariateUnifierHighRate::AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, size_t tSetIndex) {
    // perform check only if expected cases.
    if (tMeasure > 0.0 && Calculator.HighRateDataStream(tCases, tMeasure, tSetIndex)) { 
        _llr += Calculator.CalcLogLikelihoodRatio(tCases, tMeasure, tSetIndex);
        _data_stream_accumulator._sum_observed += tCases;
        _data_stream_accumulator._sum_expected += tMeasure * (_probability_model == BERNOULLI ? Calculator.gvDataSetTotals[tSetIndex].first / Calculator.gvDataSetTotals[tSetIndex].second : 1.0);
        _data_stream_accumulator._sum_case_totals += Calculator.gvDataSetTotals[tSetIndex].first;
        _unified_sets.set(tSetIndex);
    }
}

/** Adds loglikelihood ratio to accumulation. Also maintains data stream accumulation for risk threshold restriction.
    This particular method is intended for the Bernoulli time stratified adjustment, where we're calculating the data sets
    loglikelihood ratio by intervals separately in process previous to this call. */
void MultivariateUnifierHighRate::AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, size_t tSetIndex, double llr) {
    if (llr > 0.0) { // The sign of the passed loglikelihood ratio indicates whether high or low.
        _llr += llr;
        _data_stream_accumulator._sum_observed += tCases;
        _data_stream_accumulator._sum_expected += tMeasure * (_probability_model == BERNOULLI ? Calculator.gvDataSetTotals[tSetIndex].first / Calculator.gvDataSetTotals[tSetIndex].second : 1.0);
        _data_stream_accumulator._sum_case_totals += Calculator.gvDataSetTotals[tSetIndex].first;
        _unified_sets.set(tSetIndex);
    }
}

/** Calculates loglikelihood ratio given cluster data; adding log likelihood ratio to accumulation.
    Also maintains data stream observed cases accumulation for minimum number of cases restriction. */
void MultivariateUnifierHighRate::AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) {
    // perform check only if expected cases and there are cases in period
    if (tMeasure > 0.0 && casesInPeriod >= 0 && Calculator.MultipleSetsHighRateUniformTime(tCases, tMeasure, casesInPeriod, measureInPeriod, tSetIndex)) {
        _llr += Calculator.CalcLogLikelihoodRatioUniformTime(tCases, tMeasure, casesInPeriod, measureInPeriod, tSetIndex);
        _data_stream_accumulator._sum_observed += tCases;
        _unified_sets.set(tSetIndex);
    }
}

/** Calculates loglikelihood ratio given cluster data; adding log likelihood ratio to accumulation.
    Also maintains data stream observed cases accumulation for minimum number of cases restriction. */
void MultivariateUnifierHighRate::AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, measure_t tMeasureAux, size_t tSetIndex) {
    // perform check only if expected cases.
    if (tMeasure > 0.0) { 
        bool bWeightedNormal = _probability_model == NORMAL && Calculator.GetDataHub().GetParameters().getIsWeightedNormal();
        if (bWeightedNormal ? Calculator.MultipleSetsHighRateWeightedNormal(tCases, tMeasure, tMeasureAux, tSetIndex) : Calculator.HighRateDataStream(tCases, tMeasure, tSetIndex)) {
            _llr += Calculator.CalcLogLikelihoodRatioNormal(tCases, tMeasure, tMeasureAux, tSetIndex);
            _data_stream_accumulator._sum_observed += tCases;
            _unified_sets.set(tSetIndex);
        }
    }
}

/** Calculates loglikelihood ratio given ordinal data; adding log likelihood ratio to accumulation. */
void MultivariateUnifierHighRate::AdjoinRatio(AbstractLikelihoodCalculator& Calculator, const std::vector<count_t>& vOrdinalCases, size_t tSetIndex) {
    _llr += ((OrdinalLikelihoodCalculator&)Calculator).CalcLogLikelihoodRatioOrdinalHighRate(vOrdinalCases, tSetIndex);
    _unified_sets.set(tSetIndex);
}

/////////////////////////////// MultivariateUnifierLowRate //////////////////////////////////////////

/** Calculates loglikelihood ratio given parameter data; adding log likelihood ratio to accumulation.
     Also maintains data stream accumulation for risk threshold restriction. */
void MultivariateUnifierLowRate::AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, size_t tSetIndex) {
    // perform check only if expected cases.
    if (tMeasure > 0.0 && Calculator.LowRateDataStream(tCases, tMeasure, tSetIndex)) {
        _llr += Calculator.CalcLogLikelihoodRatio(tCases, tMeasure, tSetIndex);
        _data_stream_accumulator._sum_observed += tCases;
        _data_stream_accumulator._sum_expected += tMeasure * (_probability_model == BERNOULLI ? Calculator.gvDataSetTotals[tSetIndex].first / Calculator.gvDataSetTotals[tSetIndex].second : 1.0);
        _data_stream_accumulator._sum_case_totals += Calculator.gvDataSetTotals[tSetIndex].first;
        _unified_sets.set(tSetIndex);
    }
}

/** Calculates loglikelihood ratio given cluster data; adding log likelihood ratio to accumulation.
    Also maintains data stream observed cases accumulation for minimum number of cases restriction. */
void MultivariateUnifierLowRate::AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) {
    // perform check only if expected cases and there are cases in period
    if (tMeasure > 0.0 && casesInPeriod >= 0 && Calculator.MultipleSetsLowRateUniformTime(tCases, tMeasure, casesInPeriod, measureInPeriod, tSetIndex)) {
        _llr += Calculator.CalcLogLikelihoodRatioUniformTime(tCases, tMeasure, casesInPeriod, measureInPeriod, tSetIndex);
        _data_stream_accumulator._sum_observed += tCases;
        _unified_sets.set(tSetIndex);
    }
}

/** Calculates loglikelihood ratio given cluster data; adding log likelihood ratio to accumulation. 
    Also maintains data stream observed cases accumulation for minimum number of cases restriction. */
void MultivariateUnifierLowRate::AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, measure_t tMeasureAux, size_t tSetIndex) {
    // perform check only if expected cases.
    if (tMeasure > 0.0) { 
        bool bWeightedNormal = _probability_model == NORMAL && Calculator.GetDataHub().GetParameters().getIsWeightedNormal();
        if (bWeightedNormal ? Calculator.MultipleSetsLowRateWeightedNormal(tCases, tMeasure, tMeasureAux, tSetIndex) : Calculator.LowRateDataStream(tCases, tMeasure, tSetIndex)) {
            _llr += Calculator.CalcLogLikelihoodRatioNormal(tCases, tMeasure, tMeasureAux, tSetIndex);
            _data_stream_accumulator._sum_observed += tCases;
            _unified_sets.set(tSetIndex);
        }
    }
}

/** Calculates loglikelihood ratio given ordinal data; adding log likelihood ratio to accumulation. */
void MultivariateUnifierLowRate::AdjoinRatio(AbstractLikelihoodCalculator& Calculator, const std::vector<count_t>& vOrdinalCases, size_t tSetIndex) {
    _llr += ((OrdinalLikelihoodCalculator&)Calculator).CalcLogLikelihoodRatioOrdinalLowRate(vOrdinalCases, tSetIndex);
    _unified_sets.set(tSetIndex);
}

/////////////////////////////// MultivariateUnifierHighLowRate //////////////////////////////////////

/** Calculates loglikelihood ratio given data data; accumulating like high and low rate separately. */
void MultivariateUnifierHighLowRate::AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, size_t tSetIndex) {
    _high_rate.AdjoinRatio(Calculator, tCases, tMeasure, tSetIndex);
    _low_rate.AdjoinRatio(Calculator, tCases, tMeasure, tSetIndex);
}

/** Calculates loglikelihood ratio given cluster data; accumulating like high and low rate separately. */
void MultivariateUnifierHighLowRate::AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) {
    _high_rate.AdjoinRatio(Calculator, tCases, tMeasure, casesInPeriod, measureInPeriod, tSetIndex);
    _low_rate.AdjoinRatio(Calculator, tCases, tMeasure, casesInPeriod, measureInPeriod, tSetIndex);
}

/** Calculates loglikelihood ratio given cluster data; accumulating like high and low rate separately. */
void MultivariateUnifierHighLowRate::AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, measure_t tMeasureAux, size_t tSetIndex) {
    _high_rate.AdjoinRatio(Calculator, tCases, tMeasure, tMeasureAux, tSetIndex);
    _low_rate.AdjoinRatio(Calculator, tCases, tMeasure, tMeasureAux, tSetIndex);
}

/** Calculates loglikelihood ratio given ordinal data; accumulating like high and low rate separately. */
void MultivariateUnifierHighLowRate::AdjoinRatio(AbstractLikelihoodCalculator& Calculator, const std::vector<count_t>& vOrdinalCases, size_t tSetIndex) {
    _high_rate.AdjoinRatio(Calculator, vOrdinalCases, tSetIndex);
    _low_rate.AdjoinRatio(Calculator, vOrdinalCases, tSetIndex);
}

///////////////////////////////////// AdjustmentUnifier /////////////////////////////////////////

/** Calculates loglikelihood ratio give current observed and expected of data stream and adds to accumulation.
    Also maintains data stream observed cases accumulation for minimum number of cases restriction.*/
void AdjustmentUnifier::AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, size_t tSetIndex) {
    if (Calculator.HighRateDataStream(tCases, tMeasure, tSetIndex)) {
        _llr += Calculator.CalcLogLikelihoodRatio(tCases, tMeasure, tSetIndex);
        _data_stream_accumulator._sum_observed += tCases;
    } else if (Calculator.LowRateDataStream(tCases, tMeasure, tSetIndex)) {
        _llr += -1 * Calculator.CalcLogLikelihoodRatio(tCases, tMeasure, tSetIndex);
        _data_stream_accumulator._sum_observed += tCases;
    }
}

/** Calculates loglikelihood ratio give current observed and expected of data stream and adds to accumulation.
Also maintains data stream observed cases accumulation for minimum number of cases restriction.*/
void AdjustmentUnifier::AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, size_t tSetIndex, double llr) {
    _llr += llr; // The sign of the passed loglikelihood ratio already indicates whether high or low.
    _data_stream_accumulator._sum_observed += tCases;
}

/** Calculates loglikelihood ratio give current observed and expected of data stream and adds to accumulation. 
    Also maintains data stream observed cases accumulation for minimum number of cases restriction.*/
void AdjustmentUnifier::AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, measure_t tMeasureAux, size_t tSetIndex) {
    if (Calculator.HighRateDataStream(tCases, tMeasure, tSetIndex)) {
        _llr += Calculator.CalcLogLikelihoodRatioNormal(tCases, tMeasure, tMeasureAux, tSetIndex);
        _data_stream_accumulator._sum_observed += tCases;
    } else if (Calculator.LowRateDataStream(tCases, tMeasure, tSetIndex)) {
        _llr += -1 * Calculator.CalcLogLikelihoodRatioNormal(tCases, tMeasure, tMeasureAux, tSetIndex);
        _data_stream_accumulator._sum_observed += tCases;
    }
}

void AdjustmentUnifier::AdjoinRatio(AbstractLikelihoodCalculator& Calculator, const std::vector<count_t>& vOrdinalCases, size_t tSetIndex) {
    throw prg_error("AdjoinRatio() not implementated yet for Adjustment option!","AdjustmentUnifier");
}

/** Calculates loglikelihood ratio give current observed and expected of data stream and adds to accumulation. */
void AdjustmentUnifier::AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) {
    if (Calculator.MultipleSetsHighRateUniformTime(tCases, tMeasure, casesInPeriod, measureInPeriod, tSetIndex))
        _llr += Calculator.CalcLogLikelihoodRatioUniformTime(tCases, tMeasure, casesInPeriod, measureInPeriod, tSetIndex);
    else if (Calculator.MultipleSetsLowRateUniformTime(tCases, tMeasure, casesInPeriod, measureInPeriod, tSetIndex))
        _llr += -1 * Calculator.CalcLogLikelihoodRatioUniformTime(tCases, tMeasure, casesInPeriod, measureInPeriod, tSetIndex);
}

/** Returns calculated loglikelihood ratio that is the sum of adjoined values.
    Based upon scanning rate, returned value is adjusted such that if rate is:
    high         ; no adjustment occurs
    low          ; ratio * -1 is returned
    high and low ; absolute value of ratio is returned */
double AdjustmentUnifier::GetLoglikelihoodRatio() const {
    switch (_scan_area) {
        case HIGHANDLOW : return std::fabs(_llr);
        case LOW        : return _llr * -1;
        default         : return _llr;
    };
}

///////////////////////////////////////// AdjustmentUnifierRiskThreshold /////////////////////////

/** Calculates loglikelihood ratio give current observed and expected of data stream and adds to accumulation. 
    Also maintains data stream accumulation for risk threshold restriction. */
void AdjustmentUnifierRiskThreshold::AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, size_t tSetIndex) {
    if (Calculator.HighRateDataStream(tCases, tMeasure, tSetIndex)) {
        _llr += Calculator.CalcLogLikelihoodRatio(tCases, tMeasure, tSetIndex);
        _data_stream_accumulator._sum_observed += tCases;
        _data_stream_accumulator._sum_expected += tMeasure * (_probability_model == BERNOULLI ? Calculator.gvDataSetTotals[tSetIndex].first / Calculator.gvDataSetTotals[tSetIndex].second : 1.0);
        _data_stream_accumulator._sum_case_totals += Calculator.gvDataSetTotals[tSetIndex].first;
    } else if (Calculator.LowRateDataStream(tCases, tMeasure, tSetIndex)) {
        _llr += -1 * Calculator.CalcLogLikelihoodRatio(tCases, tMeasure, tSetIndex);
        _data_stream_accumulator._sum_observed += tCases;
        _data_stream_accumulator._sum_expected += tMeasure * (_probability_model == BERNOULLI ? Calculator.gvDataSetTotals[tSetIndex].first / Calculator.gvDataSetTotals[tSetIndex].second : 1.0);
        _data_stream_accumulator._sum_case_totals += Calculator.gvDataSetTotals[tSetIndex].first;
    }
}

/** Adds loglikelihood ratio to accumulation. Also maintains data stream accumulation for risk threshold restriction.
This particular method is intended for the Bernoulli time stratified adjustment, where we're calculating the data sets
loglikelihood ratio by intervals separately in process previous to this call. */
void AdjustmentUnifierRiskThreshold::AdjoinRatio(AbstractLikelihoodCalculator& Calculator, count_t tCases, measure_t tMeasure, size_t tSetIndex, double llr) {
    _llr += llr; // The sign of the passed loglikelihood ratio already indicates whether high or low.
    _data_stream_accumulator._sum_observed += tCases;
    _data_stream_accumulator._sum_expected += tMeasure * (_probability_model == BERNOULLI ? Calculator.gvDataSetTotals[tSetIndex].first / Calculator.gvDataSetTotals[tSetIndex].second : 1.0);
    _data_stream_accumulator._sum_case_totals += Calculator.gvDataSetTotals[tSetIndex].first;
}

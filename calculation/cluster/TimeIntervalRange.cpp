//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "TimeIntervalRange.h"
#include "SaTScanData.h"
#include "MeasureList.h"
#include "cluster.h"
#include "ClusterData.h"
#include "NormalClusterData.h"
#include "BatchedClusterData.h"
#include "UniformTimeClusterData.h"
#include "MultiSetUniformTimeClusterData.h"
#include "MultiSetClusterData.h"
#include "CategoricalClusterData.h"
#include "MultiSetCategoricalClusterData.h"
#include "LoglikelihoodRatioUnifier.h"
#include "MultiSetNormalClusterData.h"
#include "MultiSetBatchedClusterData.h"
#include "SSException.h"
#include "ClosedLoopData.h"
#include "BatchedLikelihoodCalculation.h"

//********** TemporalDataEvaluator **********

/** constructor */
TemporalDataEvaluator::TemporalDataEvaluator(
    const CSaTScanData& DataHub, AbstractLikelihoodCalculator & Calculator, IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType
): CTimeIntervals(DataHub, Calculator, eIncludeClustersType) {
    if (eExecutionType == CENTRICALLY) {
        gpCalculationMethod = &AbstractLikelihoodCalculator::CalcLogLikelihoodRatio;
        gdDefaultMaximizingValue = 0;
    } else {
        gpCalculationMethod = &AbstractLikelihoodCalculator::CalculateMaximizingValue;
        gdDefaultMaximizingValue = -std::numeric_limits<double>::max();
    }
}

/** Iterates through defined temporal window for accumulated data of 'Running' cluster. Calculates loglikelihood ratio
    of clusters that have rates of which we are interested in and updates clusterset accordingly. */
void TemporalDataEvaluator::CompareClusterSet(CCluster& Running, CClusterSet& clusterSet) {
    TemporalData & Data = (TemporalData&)*(Running.GetClusterData());//GetClusterDataAsType<TemporalData>(*(Running.GetClusterData()));
    count_t * pCases = Data.gpCases;
    measure_t * pMeasure = Data.gpMeasure;
    AbstractLikelihoodCalculator::SCANRATE_FUNCPTR pRateCheck = gLikelihoodCalculator.gpRateOfInterest;

    int iWindowStart, iMinWindowStart;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd=ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iMinWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iWindowStart = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength());
        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
            Data.gtCases = pCases[iWindowStart] - pCases[iWindowEnd];
            Data.gtMeasure = pMeasure[iWindowStart] - pMeasure[iWindowEnd];
            if ((gLikelihoodCalculator.*pRateCheck)(Data.gtCases, Data.gtMeasure)) {
                Running.m_nRatio = gLikelihoodCalculator.CalcLogLikelihoodRatio(Data.gtCases, Data.gtMeasure);
                Running.m_nFirstInterval = iWindowStart;
                Running.m_nLastInterval = iWindowEnd;
                clusterSet.update(Running);
            }
        }
    }
}

/** Iterates through previously defined temporal window for accumulated data of
    AbstractTemporalClusterData object. For each evaluated window, calls method
    CMeasureList::AddMeasure(cases,measure). */
void TemporalDataEvaluator::CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList) {
    TemporalData & Data = (TemporalData&)ClusterData;//GetClusterDataAsType<TemporalData>(ClusterData);
    count_t * pCases = Data.gpCases;
    measure_t * pMeasure = Data.gpMeasure;

    int iWindowStart, iMaxStartWindow;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd=ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iMaxStartWindow = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength() + 1);
        for (; iWindowStart < iMaxStartWindow; ++iWindowStart)
            MeasureList.AddMeasure(pCases[iWindowStart] - pCases[iWindowEnd], pMeasure[iWindowStart] - pMeasure[iWindowEnd]);
    }
}

/** Iterates through defined temporal window for accumulated cluster data.
    Calculates greatest loglikelihood ratio among clusterings that have rates
    which we are interested in. Returns greatest loglikelihood ratio. */
double TemporalDataEvaluator::ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData) {
    TemporalData & Data = (TemporalData&)ClusterData;//GetClusterDataAsType<TemporalData>(ClusterData);
    count_t * pCases = Data.gpCases;
    measure_t * pMeasure = Data.gpMeasure;
    double dMaxValue(gdDefaultMaximizingValue);
    AbstractLikelihoodCalculator::SCANRATE_FUNCPTR pRateCheck = gLikelihoodCalculator.gpRateOfInterest;

    int iWindowStart, iMaxStartWindow;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd=ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iMaxStartWindow = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength() + 1);
        for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
            Data.gtCases = pCases[iWindowStart] - pCases[iWindowEnd];
            Data.gtMeasure = pMeasure[iWindowStart] - pMeasure[iWindowEnd];
            if ((gLikelihoodCalculator.*pRateCheck)(Data.gtCases, Data.gtMeasure))
                dMaxValue = std::max(dMaxValue, (gLikelihoodCalculator.*gpCalculationMethod)(Data.gtCases, Data.gtMeasure, 0));
        }
    }
    return dMaxValue;
}

//********** TimeStratifiedTemporalDataEvaluator *******

/** constructor */
TimeStratifiedTemporalDataEvaluator::TimeStratifiedTemporalDataEvaluator(
    const CSaTScanData& DataHub, AbstractLikelihoodCalculator & Calculator, IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType
): CTimeIntervals(DataHub, Calculator, eIncludeClustersType) {
    if (DataHub.GetParameters().GetTimeTrendAdjustmentType() != TEMPORAL_STRATIFIED_RANDOMIZATION)
        throw prg_error("TimeStratifiedTemporalDataEvaluator not implemented for selected time trend adjustment type.", "BernoulliTimeStratifiedTemporalDataEvaluator");
    if (!DataHub.GetParameters().GetIsSpaceTimeAnalysis())
        throw prg_error("TimeStratifiedTemporalDataEvaluator only implemented for space-time analyses.", "BernoulliTimeStratifiedTemporalDataEvaluator");
    // get pointers to cumulative case and measure data, we'll need these during scanning
    _pt_counts = DataHub.GetDataSetHandler().GetDataSet().getCaseData_PT();
    _pt_measure = DataHub.GetDataSetHandler().GetDataSet().getMeasureData_PT();
}

/** Iterates through defined temporal window for accumulated data of 'Running' cluster. Calculates loglikelihood ratio
of clusters that have rates of which we are interested in and updates clusterset accordingly. */
void TimeStratifiedTemporalDataEvaluator::CompareClusterSet(CCluster& Running, CClusterSet& clusterSet) {
    TemporalData & Data = (TemporalData&)*(Running.GetClusterData());//GetClusterDataAsType<TemporalData>(*(Running.GetClusterData()));
    count_t * pCases = Data.gpCases, totalWindowCases;
    measure_t * pMeasure = Data.gpMeasure, totalWindowMeasure;
    AbstractLikelihoodCalculator::SCANRATETIMESTRATIFIED_FUNCPTR pRateCheck = gLikelihoodCalculator.gpRateOfInterestTimeStratified;
    int iWindowStart, iMinWindowStart;

    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd = ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iMinWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iWindowStart = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength());
        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
            Data.gtCases = pCases[iWindowStart] - pCases[iWindowEnd];
            Data.gtMeasure = pMeasure[iWindowStart] - pMeasure[iWindowEnd];
            totalWindowCases = _pt_counts[iWindowStart] - _pt_counts[iWindowEnd];
            totalWindowMeasure = _pt_measure[iWindowStart] - _pt_measure[iWindowEnd];
            if ((gLikelihoodCalculator.*pRateCheck)(Data.gtCases, Data.gtMeasure, totalWindowCases, totalWindowMeasure)) {
                Running.m_nRatio = gLikelihoodCalculator.CalcLogLikelihoodTimeStratified(Data.gtCases, Data.gtMeasure, totalWindowCases, totalWindowMeasure);
                Running.m_nFirstInterval = iWindowStart;
                Running.m_nLastInterval = iWindowEnd;
                clusterSet.update(Running);
            }
        }
    }
}

/** No implemented for this class */
void TimeStratifiedTemporalDataEvaluator::CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList) {
    throw prg_error("CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) not implemented.", "TimeStratifiedTemporalDataEvaluator");
}

/** Iterates through defined temporal window for accumulated cluster data.
Calculates greatest loglikelihood ratio among clusterings that have rates
which we are interested in. Returns greatest loglikelihood ratio. */
double TimeStratifiedTemporalDataEvaluator::ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData) {
    TemporalData & Data = (TemporalData&)ClusterData;//GetClusterDataAsType<TemporalData>(ClusterData);
    count_t * pCases = Data.gpCases, totalWindowCases;
    measure_t * pMeasure = Data.gpMeasure, totalWindowMeasure;
    AbstractLikelihoodCalculator::SCANRATETIMESTRATIFIED_FUNCPTR pRateCheck = gLikelihoodCalculator.gpRateOfInterestTimeStratified;
    double dMaxValue(0.0);
    int iWindowStart, iMinWindowStart;

    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd = ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iMinWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iWindowStart = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength());
        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
            Data.gtCases = pCases[iWindowStart] - pCases[iWindowEnd];
            Data.gtMeasure = pMeasure[iWindowStart] - pMeasure[iWindowEnd];
            totalWindowCases = _pt_counts[iWindowStart] - _pt_counts[iWindowEnd];
            totalWindowMeasure = _pt_measure[iWindowStart] - _pt_measure[iWindowEnd];
            if ((gLikelihoodCalculator.*pRateCheck)(Data.gtCases, Data.gtMeasure, totalWindowCases, totalWindowMeasure)) {
                dMaxValue = std::max(dMaxValue, gLikelihoodCalculator.CalcLogLikelihoodTimeStratified(Data.gtCases, Data.gtMeasure, totalWindowCases, totalWindowMeasure));
            }
        }
    }
    return dMaxValue;
}

//********** TimeStratifiedBatchedTemporalDataEvaluator **********

/** constructor */
TimeStratifiedBatchedTemporalDataEvaluator::TimeStratifiedBatchedTemporalDataEvaluator(
    const CSaTScanData& DataHub, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType
) : CTimeIntervals(DataHub, Calculator, eIncludeClustersType) {
    // get pointers to cumulative case and measure data, we'll need these during scanning
}

/** Iterates through defined temporal window for accumulated data of 'Running' cluster. Calculates loglikelihood ratio
    of clusters that have rates of which we are interested in and updates clusterset accordingly. */
void TimeStratifiedBatchedTemporalDataEvaluator::CompareClusterSet(CCluster& Running, CClusterSet& clusterSet) {
    BatchedSpaceTimeData& Data = (BatchedSpaceTimeData&)*(Running.GetClusterData());//GetClusterDataAsType<BatchedTemporalData>(*(Running.GetClusterData()));
    AbstractLikelihoodCalculator::SCANRATEBATCHED_FUNCPTR pRateCheck = gLikelihoodCalculator.gpRateOfInterestBatched;
	BatchedLikelihoodCalculator& batchedCalc = (BatchedLikelihoodCalculator&)gLikelihoodCalculator;
    ProbabilitiesRange_t probabilities;

    batchedCalc.clearCache();
    double cumulative_llr;
    int iWindowStart, iMinStartWindow, iEvaluateStart;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd = ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iMinStartWindow = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iEvaluateStart = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength());
        iWindowStart = iWindowEnd - 1;
        cumulative_llr = 0.0;
        for (; iWindowStart >= iMinStartWindow; --iWindowStart) {
            // In order to correctly calculate the accumulated LLR by time interval, we always need to calculate here.
            batchedCalc.CalculateProbabilitiesByTimeInterval(
                probabilities,
                Data.gpCases[iWindowStart] - Data.gpCases[iWindowStart + 1],
                Data.gpMeasure[iWindowStart] - Data.gpMeasure[iWindowStart + 1],
                Data.gpMeasureAux2[iWindowStart] - Data.gpMeasureAux2[iWindowStart + 1],
                Data.gpMeasureAux[iWindowStart] - Data.gpMeasureAux[iWindowStart + 1],
                Data.gpPositiveBatches[iWindowStart] - Data.gpPositiveBatches[iWindowStart + 1],
                iWindowStart
            );
            double i_llr = batchedCalc.getLoglikelihoodRatioForInterval(*probabilities, iWindowStart);
            if (probabilities->_paoi._pinside < probabilities->_paoi._poutside)
                cumulative_llr += -1.0 * i_llr;
            else
                cumulative_llr += i_llr;
            // For a prospective scan where the minimum temporal cluster size is greater than one, there will be intervals between
            // the start and end intervals that are not evaluated but contribute to the cumulative LLR.
            if (iWindowStart > iEvaluateStart)
                continue;
            Data.gtCases = Data.gpCases[iWindowStart] - Data.gpCases[iWindowEnd];
            Data.gBatches = Data.gpBatches[iWindowStart] - Data.gpBatches[iWindowEnd];
            if (cumulative_llr > Running.m_nRatio && 
                (gLikelihoodCalculator.*pRateCheck)(Data.gtCases, batchedCalc.getExpectedForBatches(Data.gBatches))) {
                Data.gtMeasure = Data.gpMeasure[iWindowStart] - Data.gpMeasure[iWindowEnd];
                Data.gtMeasureAux = Data.gpMeasureAux[iWindowStart] - Data.gpMeasureAux[iWindowEnd];
                Data.gtMeasureAux2 = Data.gpMeasureAux2[iWindowStart] - Data.gpMeasureAux2[iWindowEnd];
                Data.gPositiveBatches = Data.gpPositiveBatches[iWindowStart] - Data.gpPositiveBatches[iWindowEnd];
                Running.m_nFirstInterval = iWindowStart;
                Running.m_nLastInterval = iWindowEnd;
                Running.m_nRatio = cumulative_llr;
                clusterSet.update(Running);
            }
        }
    }
}


/** Not implemented - throws prg_error */
void TimeStratifiedBatchedTemporalDataEvaluator::CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) {
    throw prg_error("CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) not implemented.", "TimeStratifiedBatchedTemporalDataEvaluator");
}

/** Iterates through defined temporal window for accumulated cluster data.
    Calculates greatest loglikelihood ratio among clusterings that have rates
    which we are interested in. Returns greatest loglikelihood ratio. */
double TimeStratifiedBatchedTemporalDataEvaluator::ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData) {
    BatchedSpaceTimeData& Data = (BatchedSpaceTimeData&)ClusterData;//GetClusterDataAsType<BatchedTemporalData>(ClusterData);
    double dMaxValue(0.0);
    AbstractLikelihoodCalculator::SCANRATEBATCHED_FUNCPTR pRateCheck = gLikelihoodCalculator.gpRateOfInterestBatched;
    BatchedLikelihoodCalculator& batchedCalc = (BatchedLikelihoodCalculator&)gLikelihoodCalculator;
    ProbabilitiesRange_t probabilities;

    batchedCalc.clearCache();
    double cumulative_llr;
    //iterate through windows
    int iWindowStart, iMinWindowStart, iEvaluateStart;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd = ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iMinWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iEvaluateStart = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength());
        iWindowStart = iWindowEnd - 1;
        cumulative_llr = 0.0;
        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
            // In order to correctly calculate the accumulated LLR by time interval, we always need to calculate here.
            batchedCalc.CalculateProbabilitiesForSimulationByTimeInterval(
                probabilities,
                Data.gpCases[iWindowStart] - Data.gpCases[iWindowStart + 1],
                Data.gpMeasure[iWindowStart] - Data.gpMeasure[iWindowStart + 1],
                Data.gpMeasureAux2[iWindowStart] - Data.gpMeasureAux2[iWindowStart + 1],
                Data.gpMeasureAux[iWindowStart] - Data.gpMeasureAux[iWindowStart + 1],
                Data.gpPositiveBatches[iWindowStart] - Data.gpPositiveBatches[iWindowStart + 1],
                iWindowStart
            );
            double i_llr = batchedCalc.getLoglikelihoodRatioForInterval(*probabilities, iWindowStart);
            if (probabilities->_paoi._pinside < probabilities->_paoi._poutside)
                cumulative_llr += -1.0 * i_llr;
            else
                cumulative_llr += i_llr;
            // For a prospective scan where the minimum temporal cluster size is greater than one, there will be intervals between
            // the start and end intervals that are not evaluated but contribute to the cumulative LLR.
            if (iWindowStart > iEvaluateStart)
                continue;
            if ((gLikelihoodCalculator.*pRateCheck)(
                Data.gpCases[iWindowStart] - Data.gpCases[iWindowEnd],
                batchedCalc.getExpectedForBatches(Data.gpBatches[iWindowStart] - Data.gpBatches[iWindowEnd]))
                ) {
                dMaxValue = std::max(dMaxValue, cumulative_llr);
            }
        }
    }
    return dMaxValue;
}

//********** TimeStratifiedBatchedTemporalDataEvaluatorEnhanced **********

/** constructor */
TimeStratifiedBatchedTemporalDataEvaluatorEnhanced::TimeStratifiedBatchedTemporalDataEvaluatorEnhanced(
    const CSaTScanData& DataHub, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType
) : CTimeIntervals(DataHub, Calculator, eIncludeClustersType) {}

/** Iterates through defined temporal window for accumulated data of 'Running' cluster. Calculates loglikelihood ratio
    of clusters that have rates of which we are interested in and updates clusterset accordingly. */
void TimeStratifiedBatchedTemporalDataEvaluatorEnhanced::CompareClusterSet(CCluster& Running, CClusterSet& clusterSet) {
    BatchedSpaceTimeData& Data = (BatchedSpaceTimeData&)*(Running.GetClusterData());//GetClusterDataAsType<BatchedTemporalData>(*(Running.GetClusterData()));
    AbstractLikelihoodCalculator::SCANRATEBATCHED_FUNCPTR pRateCheck = gLikelihoodCalculator.gpRateOfInterestBatched;
    BatchedLikelihoodCalculator& batchedCalc = (BatchedLikelihoodCalculator&)gLikelihoodCalculator;
    BatchedLikelihoodCalculator::ProbabilitiesContainer_t probabilities;

    batchedCalc.clearCache(); // clear cached probabilities from previous iteration
    double cumulative_llr, w_llr;
    int iWindowStart, iMinStartWindow;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd = ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iMinStartWindow = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iWindowStart = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength());
        for (; iWindowStart >= iMinStartWindow; --iWindowStart) {
            batchedCalc.CalculateProbabilitiesForWindow(
                Data, iWindowStart, iWindowEnd - 1, probabilities
            );
            cumulative_llr = 0.0;
            for (auto& probability: probabilities) {
                w_llr = batchedCalc.getLoglikelihoodRatioForRange(*probability);
                if (probability->_paoi._pinside < probability->_paoi._poutside)
                    cumulative_llr += -1.0 * w_llr;
                else
                    cumulative_llr += w_llr;
            }
            Data.gtCases = Data.gpCases[iWindowStart] - Data.gpCases[iWindowEnd];
            Data.gBatches = Data.gpBatches[iWindowStart] - Data.gpBatches[iWindowEnd];
            if (cumulative_llr > Running.m_nRatio && (gLikelihoodCalculator.*pRateCheck)(Data.gtCases, batchedCalc.getExpectedForBatches(Data.gBatches))) {
                Data.gtMeasure = Data.gpMeasure[iWindowStart] - Data.gpMeasure[iWindowEnd];
                Data.gtMeasureAux = Data.gpMeasureAux[iWindowStart] - Data.gpMeasureAux[iWindowEnd];
                Data.gtMeasureAux2 = Data.gpMeasureAux2[iWindowStart] - Data.gpMeasureAux2[iWindowEnd];
                Data.gPositiveBatches = Data.gpPositiveBatches[iWindowStart] - Data.gpPositiveBatches[iWindowEnd];
                Running.m_nFirstInterval = iWindowStart;
                Running.m_nLastInterval = iWindowEnd;
                Running.m_nRatio = cumulative_llr;
                clusterSet.update(Running);
            }
        }
    }
}


/** Not implemented - throws prg_error */
void TimeStratifiedBatchedTemporalDataEvaluatorEnhanced::CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) {
    throw prg_error("CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) not implemented.", "TimeStratifiedBatchedTemporalDataEvaluatorEnhanced");
}

/** Iterates through defined temporal window for accumulated cluster data.
    Calculates greatest loglikelihood ratio among clusterings that have rates
    which we are interested in. Returns greatest loglikelihood ratio. */
double TimeStratifiedBatchedTemporalDataEvaluatorEnhanced::ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData) {
    BatchedSpaceTimeData& Data = (BatchedSpaceTimeData&)ClusterData;//GetClusterDataAsType<BatchedTemporalData>(ClusterData);
    double dMaxValue(0.0);
    AbstractLikelihoodCalculator::SCANRATEBATCHED_FUNCPTR pRateCheck = gLikelihoodCalculator.gpRateOfInterestBatched;
    BatchedLikelihoodCalculator& batchedCalc = (BatchedLikelihoodCalculator&)gLikelihoodCalculator;
    BatchedLikelihoodCalculator::ProbabilitiesContainer_t probabilities;

    batchedCalc.clearCache(); // clear cached probabilities from previous iteration
    double cumulative_llr, w_llr;
    //iterate through windows
    int iWindowStart, iMinStartWindow;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd = ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iMinStartWindow = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iWindowStart = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength());
        for (; iWindowStart >= iMinStartWindow; --iWindowStart) {
            batchedCalc.CalculateProbabilitiesForWindowForSimulation(
                Data, iWindowStart, iWindowEnd - 1, probabilities
            );
            cumulative_llr = 0.0;
            for (auto& probability : probabilities) {
                w_llr = batchedCalc.getLoglikelihoodRatioForRange(*probability);
                if (probability->_paoi._pinside < probability->_paoi._poutside)
                    cumulative_llr += -1.0 * w_llr;
                else
                    cumulative_llr += w_llr;
            }
            if ((gLikelihoodCalculator.*pRateCheck)(
                Data.gpCases[iWindowStart] - Data.gpCases[iWindowEnd],
                batchedCalc.getExpectedForBatches(Data.gpBatches[iWindowStart] - Data.gpBatches[iWindowEnd]))) {
                dMaxValue = std::max(dMaxValue, cumulative_llr);
            }
        }
    }
    return dMaxValue;
}

//********** MultiSetTimeStratifiedTemporalDataEvaluator ****/

/** constructor */
MultiSetTimeStratifiedTemporalDataEvaluator::MultiSetTimeStratifiedTemporalDataEvaluator(
    const CSaTScanData& DataHub, AbstractLikelihoodCalculator & Calculator, IncludeClustersType eIncludeClustersType
): CTimeIntervals(DataHub, Calculator, eIncludeClustersType) {
    if (DataHub.GetParameters().GetTimeTrendAdjustmentType() != TEMPORAL_STRATIFIED_RANDOMIZATION)
        throw prg_error("MultiSetBernoulliTimeStratifiedTemporalDataEvaluator not implemented for selected time trend adjustment type.", "MultiSetBernoulliTimeStratifiedTemporalDataEvaluator");
    if (!DataHub.GetParameters().GetIsSpaceTimeAnalysis())
        throw prg_error("MultiSetBernoulliTimeStratifiedTemporalDataEvaluator only implemented for space-time analyses.", "MultiSetBernoulliTimeStratifiedTemporalDataEvaluator");
    // get pointers to non-cumulative case and measure data, we'll need these during scanning
    for (size_t d=0; d < DataHub.GetDataSetHandler().GetNumDataSets(); ++d) {
        _pt_counts.push_back(DataHub.GetDataSetHandler().GetDataSet(d).getCaseData_PT());
        _pt_measure.push_back(DataHub.GetDataSetHandler().GetDataSet(d).getMeasureData_PT());
    }
}

/** Iterates through defined temporal window for accumulated data of 'Running' cluster. Calculates loglikelihood ratio
of clusters that have rates of which we are interested in and updates clusterset accordingly. */
void MultiSetTimeStratifiedTemporalDataEvaluator::CompareClusterSet(CCluster& Running, CClusterSet& clusterSet) {
    AbstractMultiSetTemporalData & Data = (AbstractMultiSetTemporalData&)*(Running.GetClusterData());//GetClusterDataAsType<AbstractMultiSetTemporalData>(*(Running.GetClusterData()));
    AbstractLoglikelihoodRatioUnifier & Unifier = gLikelihoodCalculator.GetUnifier();
    AbstractLikelihoodCalculator::SCANRATEMULTISET_FUNCPTR pRateCheck = gLikelihoodCalculator._rate_of_interest_multiset;

    int iWindowStart, iMinWindowStart;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd = ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iMinWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iWindowStart = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength());
        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
            Unifier.Reset();
            for (size_t d = 0; d < Data.gvSetClusterData.size(); ++d) {
                TemporalData & Datum = *(Data.gvSetClusterData[d]);
                Datum.gtCases = Datum.gpCases[iWindowStart] - Datum.gpCases[iWindowEnd];
                Datum.gtMeasure = Datum.gpMeasure[iWindowStart] - Datum.gpMeasure[iWindowEnd];
                Unifier.AdjoinRatioNonparametric(
                    gLikelihoodCalculator, Datum.gtCases, Datum.gtMeasure, 
                    _pt_counts[d][iWindowStart] - _pt_counts[d][iWindowEnd], _pt_measure[d][iWindowStart] - _pt_measure[d][iWindowEnd], d
                );
            }
            if ((gLikelihoodCalculator.*pRateCheck)(Unifier, true)) {
                Running.m_nFirstInterval = iWindowStart;
                Running.m_nLastInterval = iWindowEnd;
                Running.m_nRatio = Unifier.GetLoglikelihoodRatio();
                Running._ratio_sets = Unifier.getUnifiedSets();
                clusterSet.update(Running);
            }
        }
    }
}

/** Not implemented - throws prg_error */
void MultiSetTimeStratifiedTemporalDataEvaluator::CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) {
    throw prg_error("CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) not implemented.", "MultiSetBernoulliTimeStratifiedTemporalDataEvaluator");
}

/** Iterates through defined temporal window for accumulated cluster data.
Calculates greatest loglikelihood ratio among clusterings that have rates
which we are interested in. Returns greatest loglikelihood ratio. */
double MultiSetTimeStratifiedTemporalDataEvaluator::ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData) {
    AbstractMultiSetTemporalData        & Data = (AbstractMultiSetTemporalData&)ClusterData;//GetClusterDataAsType<AbstractMultiSetTemporalData>(ClusterData);
    AbstractLoglikelihoodRatioUnifier   & Unifier = gLikelihoodCalculator.GetUnifier();
    double                                dRatio(0);
    AbstractLikelihoodCalculator::SCANRATEMULTISET_FUNCPTR pRateCheck = gLikelihoodCalculator._rate_of_interest_multiset;

    //iterate through windows
    int iWindowStart, iMinWindowStart;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd = ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iMinWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iWindowStart = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength());
        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
            Unifier.Reset();
            for (size_t d = 0; d < Data.gvSetClusterData.size(); ++d) {
                TemporalData & Datum = *(Data.gvSetClusterData[d]);
                Unifier.AdjoinRatioNonparametric(
                    gLikelihoodCalculator, Datum.gpCases[iWindowStart] - Datum.gpCases[iWindowEnd], Datum.gpMeasure[iWindowStart] - Datum.gpMeasure[iWindowEnd],
                    _pt_counts[d][iWindowStart] - _pt_counts[d][iWindowEnd], _pt_measure[d][iWindowStart] - _pt_measure[d][iWindowEnd], d
                );
            }
            if ((gLikelihoodCalculator.*pRateCheck)(Unifier, true))
                dRatio = std::max(dRatio, Unifier.GetLoglikelihoodRatio());
        }
    }
    return dRatio;
}

//********** BernoulliSpatialStratifiedTemporalDataEvaluator *******

/** constructor */
BernoulliSpatialStratifiedTemporalDataEvaluator::BernoulliSpatialStratifiedTemporalDataEvaluator(
    const CSaTScanData& DataHub, AbstractLikelihoodCalculator & Calculator, IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType
): CTimeIntervals(DataHub, Calculator, eIncludeClustersType) {
    if (DataHub.GetParameters().GetSpatialAdjustmentType() != SPATIAL_STRATIFIED_RANDOMIZATION)
        throw prg_error("BernoulliSpatialStratifiedTemporalDataEvaluator not implemented for selected spatial adjustment type.", "BernoulliSpatialStratifiedTemporalDataEvaluator");
    // get pointers to non-cumulative case and measure data, we'll need these during scanning
    _pp_counts = DataHub.GetDataSetHandler().GetDataSet().getCaseData().GetArray();
    _pp_measure = DataHub.GetDataSetHandler().GetDataSet().getMeasureData().GetArray();
}

/** Iterates through defined temporal window for accumulated data of 'Running' cluster. Calculates loglikelihood ratio
of clusters that have rates of which we are interested in and updates clusterset accordingly. */
void BernoulliSpatialStratifiedTemporalDataEvaluator::CompareClusterSet(CCluster& Running, CClusterSet& clusterSet) {
    TemporalData & Data = (TemporalData&)*(Running.GetClusterData());//GetClusterDataAsType<TemporalData>(*(Running.GetClusterData()));
    count_t * pCases = Data.gpCases;
    measure_t * pMeasure = Data.gpMeasure;
    AbstractLikelihoodCalculator::SCANRATE_FUNCPTR pRateCheck = gLikelihoodCalculator.gpRateOfInterest;
    int iWindowStart, iMinWindowStart;

    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd = ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iMinWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iWindowStart = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength());
        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
            Data.gtCases = pCases[iWindowStart] - pCases[iWindowEnd];
            Data.gtMeasure = pMeasure[iWindowStart] - pMeasure[iWindowEnd];
            // Perform rate checks on cluster as a whole first.
            if ((gLikelihoodCalculator.*pRateCheck)(Data.gtCases, Data.gtMeasure)) {


                throw prg_error("Not implemented yet.", "BeronulliSpatialStratifiedTemporalDataEvaluator::CompareClusterSet");

                /* Note:
                   We've decided to suspend this feature at this point. The process here is similar logically to the time stratified
                   (except by tract vs interval) but the algorithm is a major deviation from the current. We'll leave the spatially straified
                   code in place -- should we someday decide this feature is worth the effort.
                */

                Running.m_nRatio = gLikelihoodCalculator.CalcLogLikelihoodBernoulliSpatialStratified(Data.gtCases, Data.gtMeasure, 0/* ?? */);
                Running.m_nFirstInterval = iWindowStart;
                Running.m_nLastInterval = iWindowEnd;
                clusterSet.update(Running);
            }
        }
    }
}

/** No implemented for this class */
void BernoulliSpatialStratifiedTemporalDataEvaluator::CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList) {
    throw prg_error("CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) not implemented.", "BeronulliSpatialStratifiedTemporalDataEvaluator");
}

/** Iterates through defined temporal window for accumulated cluster data.
Calculates greatest loglikelihood ratio among clusterings that have rates
which we are interested in. Returns greatest loglikelihood ratio. */
double BernoulliSpatialStratifiedTemporalDataEvaluator::ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData) {
    TemporalData & Data = (TemporalData&)ClusterData;//GetClusterDataAsType<TemporalData>(ClusterData);
    count_t * pCases = Data.gpCases;
    measure_t * pMeasure = Data.gpMeasure;
    double dMaxValue(0.0);
    AbstractLikelihoodCalculator::SCANRATE_FUNCPTR pRateCheck = gLikelihoodCalculator.gpRateOfInterest;
    double cumulative_llr;
    int iWindowStart, iMinWindowStart;

    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd = ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iMinWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iWindowStart = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength());
        cumulative_llr = 0.0;
        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
            Data.gtCases = pCases[iWindowStart] - pCases[iWindowEnd];
            Data.gtMeasure = pMeasure[iWindowStart] - pMeasure[iWindowEnd];
            // Perform rate checks on cluster as a whole first.
            if ((gLikelihoodCalculator.*pRateCheck)(Data.gtCases, Data.gtMeasure)) {

                throw prg_error("Not implemented yet.", "BeronulliSpatialStratifiedTemporalDataEvaluator::ComputeMaximizingValue");

                dMaxValue = std::max(dMaxValue, gLikelihoodCalculator.CalcLogLikelihoodBernoulliSpatialStratified(Data.gtCases, Data.gtMeasure, 0 /* ?? */));
            }
        }
    }
    return dMaxValue;
}

//********** ClosedLoopTemporalDataEvaluator **********

/** constructor */
ClosedLoopTemporalDataEvaluator::ClosedLoopTemporalDataEvaluator(
    const CSaTScanData& DataHub, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType
): CTimeIntervals(DataHub, Calculator, eIncludeClustersType) {

    // re-define interval period to include extended period
    const ClosedLoopData * dhub = dynamic_cast<const ClosedLoopData*>(&gDataHub);
    if (!dhub) throw prg_error("Unable to dynamic cast CSaTScanData to ClosedLoopData.", "constructor()");
    switch (eIncludeClustersType) {
        case ALLCLUSTERS        : 
            _interval_range = IntervalRange_t(0, dhub->getExtendedPeriodStart() - 1, gDataHub.getMinTimeIntervalCut(), dhub->getNumExtendedTimeIntervals()); break;
        default : throw prg_error("Unknown cluster inclusion type: '%d'.", "Setup()", gDataHub.GetParameters().GetIncludeClustersType());
    };
    _init_interval_range = _interval_range; // store initial range settings
    _extended_period_start = dhub->getExtendedPeriodStart();

    if (eExecutionType == CENTRICALLY) {
        gpCalculationMethod = &AbstractLikelihoodCalculator::CalcLogLikelihoodRatio;
        gdDefaultMaximizingValue = 0;
    } else {
        gpCalculationMethod = &AbstractLikelihoodCalculator::CalculateMaximizingValue;
        gdDefaultMaximizingValue = -std::numeric_limits<double>::max();
    }
}

/** Iterates through defined temporal window for accumulated data of 'Running' cluster. Calculates loglikelihood ratio
    of clusters that have rates of which we are interested in and updates clusterset accordingly. */
void ClosedLoopTemporalDataEvaluator::CompareClusterSet(CCluster& Running, CClusterSet& clusterSet) {
    TemporalData & Data = (TemporalData&)*(Running.GetClusterData());//GetClusterDataAsType<TemporalData>(*(Running.GetClusterData()));
    count_t * pCases = Data.gpCases;
    measure_t * pMeasure = Data.gpMeasure;
    AbstractLikelihoodCalculator::SCANRATE_FUNCPTR pRateCheck = gLikelihoodCalculator.gpRateOfInterest;

    int iWindowStart, iMinWindowStart;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd=ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iMinWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iWindowStart = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength());
        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
            //printf("start = %d, end = %d\n", iWindowStart, iWindowEnd);
            Data.gtCases = pCases[iWindowStart] - pCases[std::min(iWindowEnd, _extended_period_start)];
            Data.gtCases += pCases[0] - pCases[std::max(0, iWindowEnd - _extended_period_start)];
            Data.gtMeasure = pMeasure[iWindowStart] - pMeasure[std::min(iWindowEnd, _extended_period_start)];
            Data.gtMeasure += pMeasure[0] - pMeasure[std::max(0, iWindowEnd - _extended_period_start)];
            if ((gLikelihoodCalculator.*pRateCheck)(Data.gtCases, Data.gtMeasure)) {
                Running.m_nRatio = gLikelihoodCalculator.CalcLogLikelihoodRatio(Data.gtCases, Data.gtMeasure);
                Running.m_nFirstInterval = iWindowStart;
                Running.m_nLastInterval = iWindowEnd;
                clusterSet.update(Running);
            }
        }
    }
}

/** Iterates through previously defined temporal window for accumulated data of
    AbstractTemporalClusterData object. For each evaluated window, calls method
    CMeasureList::AddMeasure(cases,measure). */
void ClosedLoopTemporalDataEvaluator::CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList) {
    TemporalData & Data = (TemporalData&)ClusterData;//GetClusterDataAsType<TemporalData>(ClusterData);
    count_t * pCases = Data.gpCases;
    measure_t * pMeasure = Data.gpMeasure;

    count_t cases;
    measure_t measure;
    int iWindowStart, iMaxStartWindow;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd=ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iMaxStartWindow = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength() + 1);
        for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
            //printf("start = %d, end = %d\n", iWindowStart, iWindowEnd);
            cases = pCases[iWindowStart] - pCases[std::min(iWindowEnd, _extended_period_start)];
            cases += pCases[0] - pCases[std::max(0, iWindowEnd - _extended_period_start)];
            measure = pMeasure[iWindowStart] - pMeasure[std::min(iWindowEnd, _extended_period_start)];
            measure += pMeasure[0] - pMeasure[std::max(0, iWindowEnd - _extended_period_start)];
            MeasureList.AddMeasure(cases, measure);
        }
    }
}

/** Iterates through defined temporal window for accumulated cluster data.
    Calculates greatest loglikelihood ratio among clusterings that have rates
    which we are interested in. Returns greatest loglikelihood ratio. */
double ClosedLoopTemporalDataEvaluator::ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData) {
    TemporalData & Data = (TemporalData&)ClusterData;//GetClusterDataAsType<TemporalData>(ClusterData);
    count_t * pCases = Data.gpCases;
    measure_t * pMeasure = Data.gpMeasure;
    double dMaxValue(gdDefaultMaximizingValue);
    AbstractLikelihoodCalculator::SCANRATE_FUNCPTR pRateCheck = gLikelihoodCalculator.gpRateOfInterest;

    int iWindowStart, iMaxStartWindow;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd=ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iMaxStartWindow = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength() + 1);
        for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
            //printf("start = %d, end = %d\n", window_interval->first, window_interval->second);
            Data.gtCases = pCases[iWindowStart] - pCases[std::min(iWindowEnd, _extended_period_start)];
            Data.gtCases += pCases[0] - pCases[std::max(0, iWindowEnd - _extended_period_start)];
            Data.gtMeasure = pMeasure[iWindowStart] - pMeasure[std::min(iWindowEnd, _extended_period_start)];
            Data.gtMeasure += pMeasure[0] - pMeasure[std::max(0, iWindowEnd - _extended_period_start)];
            if ((gLikelihoodCalculator.*pRateCheck)(Data.gtCases, Data.gtMeasure))
                dMaxValue = std::max(dMaxValue, (gLikelihoodCalculator.*gpCalculationMethod)(Data.gtCases, Data.gtMeasure, 0));
        }
    }
    return dMaxValue;
}

//********** MultiSetTemporalDataEvaluator **********

/** constructor */
MultiSetTemporalDataEvaluator::MultiSetTemporalDataEvaluator(
    const CSaTScanData& DataHub, AbstractLikelihoodCalculator & Calculator, IncludeClustersType eIncludeClustersType
): CTimeIntervals(DataHub, Calculator, eIncludeClustersType) {}

/** Iterates through defined temporal window for accumulated data of 'Running' cluster. Calculates loglikelihood ratio
    of clusters that have rates of which we are interested in and updates clusterset accordingly. */
void MultiSetTemporalDataEvaluator::CompareClusterSet(CCluster& Running, CClusterSet& clusterSet) {
    AbstractMultiSetTemporalData & Data = (AbstractMultiSetTemporalData&)*(Running.GetClusterData());//GetClusterDataAsType<AbstractMultiSetTemporalData>(*(Running.GetClusterData()));
    AbstractLoglikelihoodRatioUnifier & Unifier = gLikelihoodCalculator.GetUnifier();
    AbstractLikelihoodCalculator::SCANRATEMULTISET_FUNCPTR pRateCheck = gLikelihoodCalculator._rate_of_interest_multiset;

    int iWindowStart, iMinWindowStart;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd=ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iMinWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iWindowStart = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength());
        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
            Unifier.Reset();
            for (size_t t=0; t < Data.gvSetClusterData.size(); ++t) {
                TemporalData & Datum = *(Data.gvSetClusterData[t]);
                Datum.gtCases = Datum.gpCases[iWindowStart] - Datum.gpCases[iWindowEnd];
                Datum.gtMeasure = Datum.gpMeasure[iWindowStart] - Datum.gpMeasure[iWindowEnd];
                Unifier.AdjoinRatio(gLikelihoodCalculator, Datum.gtCases, Datum.gtMeasure, t);
            }
            if ((gLikelihoodCalculator.*pRateCheck)(Unifier, false)) {
                Running.m_nFirstInterval = iWindowStart;
                Running.m_nLastInterval = iWindowEnd;
                Running.m_nRatio = Unifier.GetLoglikelihoodRatio();
                Running._ratio_sets = Unifier.getUnifiedSets();
                clusterSet.update(Running);
            }
        }
    }
}

/** Not implemented - throws prg_error */
void MultiSetTemporalDataEvaluator::CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) {
    throw prg_error("CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) not implemented.","MultiSetTemporalDataEvaluator");
}

/** Iterates through defined temporal window for accumulated cluster data.
    Calculates greatest loglikelihood ratio among clusterings that have rates
    which we are interested in. Returns greatest loglikelihood ratio. */
double MultiSetTemporalDataEvaluator::ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData) {
    AbstractMultiSetTemporalData        & Data = (AbstractMultiSetTemporalData&)ClusterData;//GetClusterDataAsType<AbstractMultiSetTemporalData>(ClusterData);
    AbstractLoglikelihoodRatioUnifier   & Unifier = gLikelihoodCalculator.GetUnifier();
    double                                dRatio(0);
    AbstractLikelihoodCalculator::SCANRATEMULTISET_FUNCPTR pRateCheck = gLikelihoodCalculator._rate_of_interest_multiset;

    //iterate through windows
    int iWindowStart, iMaxStartWindow;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd=ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iMaxStartWindow = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength() + 1);
        for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
            Unifier.Reset();
            for (size_t t=0; t < Data.gvSetClusterData.size(); ++t) {
                TemporalData & Datum = *(Data.gvSetClusterData[t]);
                Datum.gtCases = Datum.gpCases[iWindowStart] - Datum.gpCases[iWindowEnd];
                Datum.gtMeasure = Datum.gpMeasure[iWindowStart] - Datum.gpMeasure[iWindowEnd];
                Unifier.AdjoinRatio(gLikelihoodCalculator, Datum.gtCases, Datum.gtMeasure, t);
            }
            if ((gLikelihoodCalculator.*pRateCheck)(Unifier, false))
                dRatio = std::max(dRatio, Unifier.GetLoglikelihoodRatio());
        }
    }
    return dRatio;
}

//********** MultiSetUniformTimeTemporalDataEvaluator **********

/** constructor */
MultiSetUniformTimeTemporalDataEvaluator::MultiSetUniformTimeTemporalDataEvaluator(
    const CSaTScanData& DataHub, AbstractLikelihoodCalculator & Calculator, IncludeClustersType eIncludeClustersType
):CTimeIntervals(DataHub, Calculator, eIncludeClustersType) {}

/** Iterates through defined temporal window for accumulated data of 'Running' cluster. Calculates loglikelihood ratio
of clusters that have rates of which we are interested in and updates clusterset accordingly. */
void MultiSetUniformTimeTemporalDataEvaluator::CompareClusterSet(CCluster& Running, CClusterSet& clusterSet) {
    AbstractMultiSetUniformTimeTemporalData& Data = (AbstractMultiSetUniformTimeTemporalData&)*(Running.GetClusterData());//GetClusterDataAsType<AbstractMultiSetTemporalData>(*(Running.GetClusterData()));
    AbstractLoglikelihoodRatioUnifier & Unifier = gLikelihoodCalculator.GetUnifier();
    count_t * pCases = 0;
    measure_t * pMeasure = 0;

    int iWindowStart, iMinWindowStart;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd = ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iMinWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iWindowStart = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength());
        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
            Unifier.Reset();
            for (size_t t = 0; t < Data.getNumSets(); ++t) {
                UniformTimeClusterDataInterface & Datum = Data.getUniformTimeClusterDataInterface(t);
                pCases = Datum.getCasesArray();
                pMeasure = Datum.getMeasureArray();
                Datum.setCases(pCases[iWindowStart] - pCases[iWindowEnd]);
                Datum.setMeasure(pMeasure[iWindowStart] - pMeasure[iWindowEnd]);
                Unifier.AdjoinRatio(gLikelihoodCalculator, Datum.getCases(), Datum.getMeasure(), Datum.gtCasesInPeriod, Datum.gtMeasureInPeriod, t);
            }
            Running.m_nFirstInterval = iWindowStart;
            Running.m_nLastInterval = iWindowEnd;
            Running.m_nRatio = Unifier.GetLoglikelihoodRatio();
            Running._ratio_sets = Unifier.getUnifiedSets();
            clusterSet.update(Running);
        }
    }
}

/** Not implemented - throws prg_error */
void MultiSetUniformTimeTemporalDataEvaluator::CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) {
    throw prg_error("CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) not implemented.", "MultiSetTemporalDataEvaluator");
}

/** Iterates through defined temporal window for accumulated cluster data.
Calculates greatest loglikelihood ratio among clusterings that have rates
which we are interested in. Returns greatest loglikelihood ratio. */
double MultiSetUniformTimeTemporalDataEvaluator::ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData) {
    AbstractMultiSetUniformTimeTemporalData& Data = (AbstractMultiSetUniformTimeTemporalData&)ClusterData;//GetClusterDataAsType<AbstractMultiSetTemporalData>(ClusterData);
    AbstractLoglikelihoodRatioUnifier   & Unifier = gLikelihoodCalculator.GetUnifier();
    double dRatio(0);
    count_t * pCases = 0;
    measure_t * pMeasure = 0;

    //iterate through windows
    int iWindowStart, iMaxStartWindow;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd = ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iMaxStartWindow = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength() + 1);
        for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
            Unifier.Reset();
            for (size_t t = 0; t < Data.getNumSets(); ++t) {
                UniformTimeClusterDataInterface & Datum = Data.getUniformTimeClusterDataInterface(t);
                pCases = Datum.getCasesArray();
                pMeasure = Datum.getMeasureArray();
                Datum.setCases(pCases[iWindowStart] - pCases[iWindowEnd]);
                Datum.setMeasure(pMeasure[iWindowStart] - pMeasure[iWindowEnd]);
                Unifier.AdjoinRatio(gLikelihoodCalculator, Datum.getCases(), Datum.getMeasure(), Datum.gtCasesInPeriod, Datum.gtMeasureInPeriod, t);
            }
            dRatio = std::max(dRatio, Unifier.GetLoglikelihoodRatio());
        }
    }
    return dRatio;
}

//********** ClosedLoopMultiSetTemporalDataEvaluator **********

/** constructor */
ClosedLoopMultiSetTemporalDataEvaluator::ClosedLoopMultiSetTemporalDataEvaluator(const CSaTScanData& DataHub,
                                                                                 AbstractLikelihoodCalculator & Calculator,
                                                                                 IncludeClustersType eIncludeClustersType)
                                        :CTimeIntervals(DataHub, Calculator, eIncludeClustersType) {

    // re-define interval period to include extended period
    const ClosedLoopData * dhub = dynamic_cast<const ClosedLoopData*>(&gDataHub);
    if (!dhub) throw prg_error("Unable to dynamic cast CSaTScanData to ClosedLoopData.", "constructor()");
    switch (eIncludeClustersType) {
        case ALLCLUSTERS        : 
            _interval_range = IntervalRange_t(0, dhub->getExtendedPeriodStart() - 1, gDataHub.getMinTimeIntervalCut(), dhub->getNumExtendedTimeIntervals()); break;
        default : throw prg_error("Unknown cluster inclusion type: '%d'.", "Setup()", gDataHub.GetParameters().GetIncludeClustersType());
    };
    _init_interval_range = _interval_range; // store initial range settings
    _extended_period_start = dhub->getExtendedPeriodStart();
}

/** Iterates through defined temporal window for accumulated data of 'Running' cluster. Calculates loglikelihood ratio
    of clusters that have rates of which we are interested in and updates clusterset accordingly. */
void ClosedLoopMultiSetTemporalDataEvaluator::CompareClusterSet(CCluster& Running, CClusterSet& clusterSet) {
    AbstractMultiSetTemporalData & Data = (AbstractMultiSetTemporalData&)*(Running.GetClusterData());//GetClusterDataAsType<AbstractMultiSetTemporalData>(*(Running.GetClusterData()));
    AbstractLoglikelihoodRatioUnifier & Unifier = gLikelihoodCalculator.GetUnifier();
    AbstractLikelihoodCalculator::SCANRATEMULTISET_FUNCPTR pRateCheck = gLikelihoodCalculator._rate_of_interest_multiset;

    int iWindowStart, iMinWindowStart;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd=ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iMinWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iWindowStart = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength());
        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
            Unifier.Reset();
            for (size_t t=0; t < Data.gvSetClusterData.size(); ++t) {
                TemporalData & Datum = *(Data.gvSetClusterData[t]);
                //printf("start = %d, end = %d\n", iWindowStart, iWindowEnd);
                Datum.gtCases = Datum.gpCases[iWindowStart] - Datum.gpCases[std::min(iWindowEnd, _extended_period_start)];
                Datum.gtCases += Datum.gpCases[0] - Datum.gpCases[std::max(0, iWindowEnd - _extended_period_start)];
                Datum.gtMeasure = Datum.gpMeasure[iWindowStart] - Datum.gpMeasure[std::min(iWindowEnd, _extended_period_start)];
                Datum.gtMeasure += Datum.gpMeasure[0] - Datum.gpMeasure[std::max(0, iWindowEnd - _extended_period_start)];
                Unifier.AdjoinRatio(gLikelihoodCalculator, Datum.gtCases, Datum.gtMeasure, t);
            }
            if ((gLikelihoodCalculator.*pRateCheck)(Unifier, false)) {
                Running.m_nFirstInterval = iWindowStart;
                Running.m_nLastInterval = iWindowEnd;
                Running.m_nRatio = Unifier.GetLoglikelihoodRatio();
                Running._ratio_sets = Unifier.getUnifiedSets();
                clusterSet.update(Running);
            }
        }
    }
}

/** Not implemented - throws prg_error */
void ClosedLoopMultiSetTemporalDataEvaluator::CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) {
    throw prg_error("CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) not implemented.","ClosedLoopMultiSetTemporalDataEvaluator");
}

/** Iterates through defined temporal window for accumulated cluster data.
    Calculates greatest loglikelihood ratio among clusterings that have rates
    which we are interested in. Returns greatest loglikelihood ratio. */
double ClosedLoopMultiSetTemporalDataEvaluator::ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData) {
    AbstractMultiSetTemporalData        & Data = (AbstractMultiSetTemporalData&)ClusterData;//GetClusterDataAsType<AbstractMultiSetTemporalData>(ClusterData);
    AbstractLoglikelihoodRatioUnifier   & Unifier = gLikelihoodCalculator.GetUnifier();
    AbstractLikelihoodCalculator::SCANRATEMULTISET_FUNCPTR pRateCheck = gLikelihoodCalculator._rate_of_interest_multiset;
    double                                dRatio(0);

    //iterate through windows
    int iWindowStart, iMaxStartWindow;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd=ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iMaxStartWindow = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength() + 1);
        for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
            Unifier.Reset();
            for (size_t t=0; t < Data.gvSetClusterData.size(); ++t) {
                TemporalData & Datum = *(Data.gvSetClusterData[t]);
                //printf("start = %d, end = %d\n", window_interval->first, window_interval->second);
                Datum.gtCases = Datum.gpCases[iWindowStart] - Datum.gpCases[std::min(iWindowEnd, _extended_period_start)];
                Datum.gtCases += Datum.gpCases[0] - Datum.gpCases[std::max(0, iWindowEnd - _extended_period_start)];
                Datum.gtMeasure = Datum.gpMeasure[iWindowStart] - Datum.gpMeasure[std::min(iWindowEnd, _extended_period_start)];
                Datum.gtMeasure += Datum.gpMeasure[0] - Datum.gpMeasure[std::max(0, iWindowEnd - _extended_period_start)];
                Unifier.AdjoinRatio(gLikelihoodCalculator, Datum.gtCases, Datum.gtMeasure, t);
            }
            if ((gLikelihoodCalculator.*pRateCheck)(Unifier, false))
                dRatio = std::max(dRatio, Unifier.GetLoglikelihoodRatio());
        }
    }
    return dRatio;
}

//********** NormalTemporalDataEvaluator **********

/** constructor */
NormalTemporalDataEvaluator::NormalTemporalDataEvaluator(
    const CSaTScanData& DataHub, AbstractLikelihoodCalculator & Calculator, IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType
): CTimeIntervals(DataHub, Calculator, eIncludeClustersType) {
    if (eExecutionType == CENTRICALLY) {
        gpCalculationMethod = &AbstractLikelihoodCalculator::CalcLogLikelihoodRatioNormal;
        gdDefaultMaximizingValue = 0;
    } else {
        gpCalculationMethod = &AbstractLikelihoodCalculator::CalculateMaximizingValueNormal;
        gdDefaultMaximizingValue = -std::numeric_limits<double>::max();
    }
}

/** Iterates through defined temporal window for accumulated data of 'Running' cluster. Calculates loglikelihood ratio
    of clusters that have rates of which we are interested in and updates clusterset accordingly. */
void NormalTemporalDataEvaluator::CompareClusterSet(CCluster& Running, CClusterSet& clusterSet) {
    NormalTemporalData & Data = (NormalTemporalData&)*(Running.GetClusterData());//GetClusterDataAsType<NormalTemporalData>(*(Running.GetClusterData()));
    AbstractLikelihoodCalculator::SCANRATENORMAL_FUNCPTR pRateCheck = gLikelihoodCalculator.gpRateOfInterestNormal;

    int iWindowStart, iMinStartWindow;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd=ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iMinStartWindow = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iWindowStart = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength());
        for (; iWindowStart >= iMinStartWindow; --iWindowStart) {
            Data.gtCases = Data.gpCases[iWindowStart] - Data.gpCases[iWindowEnd];
            Data.gtMeasure = Data.gpMeasure[iWindowStart] - Data.gpMeasure[iWindowEnd];
            Data.gtMeasureAux = Data.gpMeasureAux[iWindowStart] - Data.gpMeasureAux[iWindowEnd];
            if ((gLikelihoodCalculator.*pRateCheck)(Data.gtCases, Data.gtMeasure, Data.gtMeasureAux)) {
                Running.m_nFirstInterval = iWindowStart;
                Running.m_nLastInterval = iWindowEnd;
                Running.m_nRatio = gLikelihoodCalculator.CalcLogLikelihoodRatioNormal(Data.gtCases, Data.gtMeasure, Data.gtMeasureAux);
                clusterSet.update(Running);
            }
        }
    }
}

/** Not implemented - throws prg_error */
void NormalTemporalDataEvaluator::CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) {
    throw prg_error("CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) not implemented.","NormalTemporalDataEvaluator");
}

/** Iterates through defined temporal window for accumulated cluster data.
    Calculates greatest loglikelihood ratio among clusterings that have rates
    which we are interested in. Returns greatest loglikelihood ratio. */
double NormalTemporalDataEvaluator::ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData) {
    NormalTemporalData & Data = (NormalTemporalData&)ClusterData;//GetClusterDataAsType<NormalTemporalData>(ClusterData);
    double dMaxValue(gdDefaultMaximizingValue);
    AbstractLikelihoodCalculator::SCANRATENORMAL_FUNCPTR pRateCheck = gLikelihoodCalculator.gpRateOfInterestNormal;

    //iterate through windows
    int iWindowStart, iMaxStartWindow;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd=ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iMaxStartWindow = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength() + 1);
        for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
            Data.gtCases = Data.gpCases[iWindowStart] - Data.gpCases[iWindowEnd];
            Data.gtMeasure = Data.gpMeasure[iWindowStart] - Data.gpMeasure[iWindowEnd];
            Data.gtMeasureAux = Data.gpMeasureAux[iWindowStart] - Data.gpMeasureAux[iWindowEnd];
            if ((gLikelihoodCalculator.*pRateCheck)(Data.gtCases, Data.gtMeasure, Data.gtMeasureAux))
                dMaxValue = std::max(dMaxValue, (gLikelihoodCalculator.*gpCalculationMethod)(Data.gtCases, Data.gtMeasure, Data.gtMeasureAux, 0));
        }
    }
    return dMaxValue;
}

//********** UniformTimeTemporalDataEvaluator **********

/** constructor */
UniformTimeTemporalDataEvaluator::UniformTimeTemporalDataEvaluator(
    const CSaTScanData& DataHub, AbstractLikelihoodCalculator & Calculator, IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType
) :CTimeIntervals(DataHub, Calculator, eIncludeClustersType) {
    if (eExecutionType == CENTRICALLY) {
        gpCalculationMethod = &AbstractLikelihoodCalculator::CalcLogLikelihoodRatioUniformTime;
        gdDefaultMaximizingValue = 0;
    }
    else {
        gpCalculationMethod = &AbstractLikelihoodCalculator::CalculateMaximizingValueUniformTime;
        gdDefaultMaximizingValue = -std::numeric_limits<double>::max();
    }
}

/** Iterates through defined temporal window for accumulated data of 'Running' cluster. Calculates loglikelihood ratio
of clusters that have rates of which we are interested in and updates clusterset accordingly. */
void UniformTimeTemporalDataEvaluator::CompareClusterSet(CCluster& Running, CClusterSet& clusterSet) {
    UniformTimeClusterDataInterface& Data = GetClusterDataAsType<UniformTimeClusterDataInterface>(*(Running.GetClusterData()));

    count_t * pCases = Data.getCasesArray();
    measure_t * pMeasure = Data.getMeasureArray();
    AbstractLikelihoodCalculator::SCANRATEUNIFORMTIME_FUNCPTR pRateCheck = gLikelihoodCalculator.gpRateOfInterestUniformTime;

    int iWindowStart, iMinStartWindow;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd = ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iMinStartWindow = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iWindowStart = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength());
        for (; iWindowStart >= iMinStartWindow; --iWindowStart) {
            Data.setCases(pCases[iWindowStart] - pCases[iWindowEnd]);
            Data.setMeasure(pMeasure[iWindowStart] - pMeasure[iWindowEnd]);
            if ((gLikelihoodCalculator.*pRateCheck)(Data.getCases(), Data.getMeasure(), Data.gtCasesInPeriod, Data.gtMeasureInPeriod, 0)) {
                Running.m_nFirstInterval = iWindowStart;
                Running.m_nLastInterval = iWindowEnd;
                Running.m_nRatio = gLikelihoodCalculator.CalcLogLikelihoodRatioUniformTime(Data.getCases(), Data.getMeasure(), Data.gtCasesInPeriod, Data.gtMeasureInPeriod, 0);
                clusterSet.update(Running);
            }
        }
    }
}

void UniformTimeTemporalDataEvaluator::CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList) {
    UniformTimeClusterDataInterface& Data = GetClusterDataAsType<UniformTimeClusterDataInterface>(ClusterData);

    count_t cases, * pCases = Data.getCasesArray();
    measure_t measure, * pMeasure = Data.getMeasureArray();
    double M = 2.0;

    int iWindowStart, iMaxStartWindow;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd = ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iMaxStartWindow = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength() + 1);
        for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
            cases = pCases[iWindowStart] - pCases[iWindowEnd];
            measure = pMeasure[iWindowStart] - pMeasure[iWindowEnd];
            if (cases < Data.gtCasesInPeriod)
                MeasureList.AddMeasure(cases, measure * static_cast<double>(Data.gtCasesInPeriod - cases) / (Data.gtMeasureInPeriod - measure));
            else if (cases == Data.gtCasesInPeriod)
                MeasureList.AddMeasure(cases, measure / (M * (Data.gtMeasureInPeriod - measure)));
        }
    }
}

/** Iterates through defined temporal window for accumulated cluster data.
Calculates greatest loglikelihood ratio among clusterings that have rates
which we are interested in. Returns greatest loglikelihood ratio. */
double UniformTimeTemporalDataEvaluator::ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData) {
    UniformTimeClusterDataInterface& Data = GetClusterDataAsType<UniformTimeClusterDataInterface>(ClusterData);
    double dMaxValue(gdDefaultMaximizingValue);
    AbstractLikelihoodCalculator::SCANRATEUNIFORMTIME_FUNCPTR pRateCheck = gLikelihoodCalculator.gpRateOfInterestUniformTime;
    count_t * pCases = Data.getCasesArray();
    measure_t * pMeasure = Data.getMeasureArray();

    //iterate through windows
    int iWindowStart, iMaxStartWindow;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd = ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iMaxStartWindow = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength() + 1);
        for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
            Data.setCases(pCases[iWindowStart] - pCases[iWindowEnd]);
            Data.setMeasure(pMeasure[iWindowStart] - pMeasure[iWindowEnd]);
            if ((gLikelihoodCalculator.*pRateCheck)(Data.getCases(), Data.getMeasure(), Data.gtCasesInPeriod, Data.gtMeasureInPeriod, 0))
                dMaxValue = std::max(dMaxValue, (gLikelihoodCalculator.*gpCalculationMethod)(Data.getCases(), Data.getMeasure(), Data.gtCasesInPeriod, Data.gtMeasureInPeriod, 0));
        }
    }
    return dMaxValue;
}

//********** ClosedLoopNormalTemporalDataEvaluator **********

/** constructor */
ClosedLoopNormalTemporalDataEvaluator::ClosedLoopNormalTemporalDataEvaluator(
    const CSaTScanData& DataHub, AbstractLikelihoodCalculator & Calculator, IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType
): CTimeIntervals(DataHub, Calculator, eIncludeClustersType) {

    // re-define interval period to include extended period
    const ClosedLoopData * dhub = dynamic_cast<const ClosedLoopData*>(&gDataHub);
    if (!dhub) throw prg_error("Unable to dynamic cast CSaTScanData to ClosedLoopData.", "constructor()");
    switch (eIncludeClustersType) {
        case ALLCLUSTERS        : 
            _interval_range = IntervalRange_t(0, dhub->getExtendedPeriodStart() - 1, gDataHub.getMinTimeIntervalCut(), dhub->getNumExtendedTimeIntervals()); break;
        default : throw prg_error("Unknown cluster inclusion type: '%d'.", "Setup()", gDataHub.GetParameters().GetIncludeClustersType());
    };
    _init_interval_range = _interval_range; // store initial range settings
    _extended_period_start = dhub->getExtendedPeriodStart();

    if (eExecutionType == CENTRICALLY) {
        gpCalculationMethod = &AbstractLikelihoodCalculator::CalcLogLikelihoodRatioNormal;
        gdDefaultMaximizingValue = 0;
    } else {
        gpCalculationMethod = &AbstractLikelihoodCalculator::CalculateMaximizingValueNormal;
        gdDefaultMaximizingValue = -std::numeric_limits<double>::max();
    }
}

/** Iterates through defined temporal window for accumulated data of 'Running' cluster. Calculates loglikelihood ratio
    of clusters that have rates of which we are interested in and updates clusterset accordingly. */
void ClosedLoopNormalTemporalDataEvaluator::CompareClusterSet(CCluster& Running, CClusterSet& clusterSet) {
    NormalTemporalData & Data = (NormalTemporalData&)*(Running.GetClusterData());//GetClusterDataAsType<NormalTemporalData>(*(Running.GetClusterData()));
    AbstractLikelihoodCalculator::SCANRATENORMAL_FUNCPTR pRateCheck = gLikelihoodCalculator.gpRateOfInterestNormal;

    int iWindowStart, iMinStartWindow;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd=ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iMinStartWindow = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iWindowStart = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength());
        for (; iWindowStart >= iMinStartWindow; --iWindowStart) {
            //printf("start = %d, end = %d\n", iWindowStart, iWindowEnd);
            Data.gtCases = Data.gpCases[iWindowStart] - Data.gpCases[std::min(iWindowEnd, _extended_period_start)];
            Data.gtCases += Data.gpCases[0] - Data.gpCases[std::max(0, iWindowEnd - _extended_period_start)];
            Data.gtMeasure = Data.gpMeasure[iWindowStart] - Data.gpMeasure[std::min(iWindowEnd, _extended_period_start)];
            Data.gtMeasure += Data.gpMeasure[0] - Data.gpMeasure[std::max(0, iWindowEnd - _extended_period_start)];
            Data.gtMeasureAux = Data.gpMeasureAux[iWindowStart] - Data.gpMeasureAux[std::min(iWindowEnd, _extended_period_start)];
            Data.gtMeasureAux += Data.gpMeasureAux[0] - Data.gpMeasureAux[std::max(0, iWindowEnd - _extended_period_start)];
            if ((gLikelihoodCalculator.*pRateCheck)(Data.gtCases, Data.gtMeasure, Data.gtMeasureAux)) {
                Running.m_nFirstInterval = iWindowStart;
                Running.m_nLastInterval = iWindowEnd;
                Running.m_nRatio = gLikelihoodCalculator.CalcLogLikelihoodRatioNormal(Data.gtCases, Data.gtMeasure, Data.gtMeasureAux);
                clusterSet.update(Running);
            }
        }
    }
}

/** Not implemented - throws prg_error */
void ClosedLoopNormalTemporalDataEvaluator::CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) {
    throw prg_error("CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) not implemented.","ClosedLoopNormalTemporalDataEvaluator");
}

/** Iterates through defined temporal window for accumulated cluster data.
    Calculates greatest loglikelihood ratio among clusterings that have rates
    which we are interested in. Returns greatest loglikelihood ratio. */
double ClosedLoopNormalTemporalDataEvaluator::ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData) {
    NormalTemporalData & Data = (NormalTemporalData&)ClusterData;//GetClusterDataAsType<NormalTemporalData>(ClusterData);
    double dMaxValue(gdDefaultMaximizingValue);
    AbstractLikelihoodCalculator::SCANRATENORMAL_FUNCPTR pRateCheck = gLikelihoodCalculator.gpRateOfInterestNormal;

    //iterate through windows
    int iWindowStart, iMaxStartWindow;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd=ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iMaxStartWindow = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength() + 1);
        for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
            //printf("start = %d, end = %d\n", window_interval->first, window_interval->second);
            Data.gtCases = Data.gpCases[iWindowStart] - Data.gpCases[std::min(iWindowEnd, _extended_period_start)];
            Data.gtCases += Data.gpCases[0] - Data.gpCases[std::max(0, iWindowEnd - _extended_period_start)];
            Data.gtMeasure = Data.gpMeasure[iWindowStart] - Data.gpMeasure[std::min(iWindowEnd, _extended_period_start)];
            Data.gtMeasure += Data.gpMeasure[0] - Data.gpMeasure[std::max(0, iWindowEnd - _extended_period_start)];
            Data.gtMeasureAux = Data.gpMeasureAux[iWindowStart] - Data.gpMeasureAux[std::min(iWindowEnd, _extended_period_start)];
            Data.gtMeasureAux += Data.gpMeasureAux[0] - Data.gpMeasureAux[std::max(0, iWindowEnd - _extended_period_start)];
            if ((gLikelihoodCalculator.*pRateCheck)(Data.gtCases, Data.gtMeasure, Data.gtMeasureAux))
                dMaxValue = std::max(dMaxValue, (gLikelihoodCalculator.*gpCalculationMethod)(Data.gtCases, Data.gtMeasure, Data.gtMeasureAux, 0));
        }
    }
    return dMaxValue;
}

//********** MultiSetNormalTemporalDataEvaluator **********

/** constructor */
MultiSetNormalTemporalDataEvaluator::MultiSetNormalTemporalDataEvaluator(
    const CSaTScanData& DataHub, AbstractLikelihoodCalculator & Calculator, IncludeClustersType eIncludeClustersType
): CTimeIntervals(DataHub, Calculator, eIncludeClustersType) {}

/** Iterates through defined temporal window for accumulated data of 'Running' cluster. Calculates loglikelihood ratio
    of clusters that have rates of which we are interested in and updates clusterset accordingly. */
void MultiSetNormalTemporalDataEvaluator::CompareClusterSet(CCluster& Running, CClusterSet& clusterSet) {
    AbstractMultiSetNormalTemporalData& Data = (AbstractMultiSetNormalTemporalData&)*(Running.GetClusterData());//GetClusterDataAsType<AbstractMultiSetNormalTemporalData>(*(Running.GetClusterData()));
    AbstractLoglikelihoodRatioUnifier & Unifier = gLikelihoodCalculator.GetUnifier();

    int iWindowStart, iMinWindowStart;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd=ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iMinWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iWindowStart = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength());
        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
            Unifier.Reset();
            for (size_t t=0; t < Data.gvSetClusterData.size(); ++t) {
                NormalTemporalData & Datum = *(Data.gvSetClusterData[t]);
                Datum.gtCases = Datum.gpCases[iWindowStart] - Datum.gpCases[iWindowEnd];
                Datum.gtMeasure = Datum.gpMeasure[iWindowStart] - Datum.gpMeasure[iWindowEnd];
                Datum.gtMeasureAux = Datum.gpMeasureAux[iWindowStart] - Datum.gpMeasureAux[iWindowEnd];
                Unifier.AdjoinRatio(gLikelihoodCalculator, Datum.gtCases, Datum.gtMeasure, Datum.gtMeasureAux, t);
            }
            Running.m_nFirstInterval = iWindowStart;
            Running.m_nLastInterval = iWindowEnd;
            Running.m_nRatio = Unifier.GetLoglikelihoodRatio();
            Running._ratio_sets = Unifier.getUnifiedSets();
            clusterSet.update(Running);
        }
    }
}

/** Not implemented - throws prg_error */
void MultiSetNormalTemporalDataEvaluator::CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) {
    throw prg_error("CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) not implemented.","MultiSetNormalTemporalDataEvaluator");
}

/** Iterates through defined temporal window for accumulated cluster data.
    Calculates greatest loglikelihood ratio among clusterings that have rates
    which we are interested in. Returns greatest loglikelihood ratio. */
double MultiSetNormalTemporalDataEvaluator::ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData) {
    AbstractMultiSetNormalTemporalData & Data = (AbstractMultiSetNormalTemporalData&)(ClusterData);//GetClusterDataAsType<AbstractMultiSetNormalTemporalData>(ClusterData);
    AbstractLoglikelihoodRatioUnifier & Unifier = gLikelihoodCalculator.GetUnifier();
    double dRatio(0);

    int iWindowStart, iMaxStartWindow;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd=ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iMaxStartWindow = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength() + 1);
        for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
            Unifier.Reset();
            for (size_t t=0; t < Data.gvSetClusterData.size(); ++t) {
                NormalTemporalData & Datum = *(Data.gvSetClusterData[t]);
                Datum.gtCases = Datum.gpCases[iWindowStart] - Datum.gpCases[iWindowEnd];
                Datum.gtMeasure = Datum.gpMeasure[iWindowStart] - Datum.gpMeasure[iWindowEnd];
                Datum.gtMeasureAux = Datum.gpMeasureAux[iWindowStart] - Datum.gpMeasureAux[iWindowEnd];
                Unifier.AdjoinRatio(gLikelihoodCalculator, Datum.gtCases, Datum.gtMeasure, Datum.gtMeasureAux, t);
            }
            dRatio = std::max(dRatio, Unifier.GetLoglikelihoodRatio());
        }
    }
    return dRatio;
}

//********** ClosedLoopMultiSetNormalTemporalDataEvaluator **********

/** constructor */
ClosedLoopMultiSetNormalTemporalDataEvaluator::ClosedLoopMultiSetNormalTemporalDataEvaluator(
    const CSaTScanData& DataHub, AbstractLikelihoodCalculator & Calculator, IncludeClustersType eIncludeClustersType
): CTimeIntervals(DataHub, Calculator, eIncludeClustersType) {

    // re-define interval period to include extended period
    const ClosedLoopData * dhub = dynamic_cast<const ClosedLoopData*>(&gDataHub);
    if (!dhub) throw prg_error("Unable to dynamic cast CSaTScanData to ClosedLoopData.", "constructor()");
    switch (eIncludeClustersType) {
        case ALLCLUSTERS        : 
            _interval_range = IntervalRange_t(0, dhub->getExtendedPeriodStart() - 1, gDataHub.getMinTimeIntervalCut(), dhub->getNumExtendedTimeIntervals()); break;
        default : throw prg_error("Unknown cluster inclusion type: '%d'.", "Setup()", gDataHub.GetParameters().GetIncludeClustersType());
    };
    _init_interval_range = _interval_range; // store initial range settings
    _extended_period_start = dhub->getExtendedPeriodStart();
}

/** Iterates through defined temporal window for accumulated data of 'Running' cluster. Calculates loglikelihood ratio
    of clusters that have rates of which we are interested in and updates clusterset accordingly. */
void ClosedLoopMultiSetNormalTemporalDataEvaluator::CompareClusterSet(CCluster& Running, CClusterSet& clusterSet) {
    AbstractMultiSetNormalTemporalData& Data = (AbstractMultiSetNormalTemporalData&)*(Running.GetClusterData());//GetClusterDataAsType<AbstractMultiSetNormalTemporalData>(*(Running.GetClusterData()));
    AbstractLoglikelihoodRatioUnifier & Unifier = gLikelihoodCalculator.GetUnifier();

    int iWindowStart, iMinWindowStart;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd=ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iMinWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iWindowStart = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength());
        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
            Unifier.Reset();
            for (size_t t=0; t < Data.gvSetClusterData.size(); ++t) {
                NormalTemporalData & Datum = *(Data.gvSetClusterData[t]);
                //printf("start = %d, end = %d\n", iWindowStart, iWindowEnd);
                Datum.gtCases = Datum.gpCases[iWindowStart] - Datum.gpCases[std::min(iWindowEnd, _extended_period_start)];
                Datum.gtCases += Datum.gpCases[0] - Datum.gpCases[std::max(0, iWindowEnd - _extended_period_start)];
                Datum.gtMeasure = Datum.gpMeasure[iWindowStart] - Datum.gpMeasure[std::min(iWindowEnd, _extended_period_start)];
                Datum.gtMeasure += Datum.gpMeasure[0] - Datum.gpMeasure[std::max(0, iWindowEnd - _extended_period_start)];
                Datum.gtMeasureAux = Datum.gpMeasureAux[iWindowStart] - Datum.gpMeasureAux[std::min(iWindowEnd, _extended_period_start)];
                Datum.gtMeasureAux += Datum.gpMeasureAux[0] - Datum.gpMeasureAux[std::max(0, iWindowEnd - _extended_period_start)];
                Unifier.AdjoinRatio(gLikelihoodCalculator, Datum.gtCases, Datum.gtMeasure, Datum.gtMeasureAux, t);
            }
            Running.m_nFirstInterval = iWindowStart;
            Running.m_nLastInterval = iWindowEnd;
            Running.m_nRatio = Unifier.GetLoglikelihoodRatio();
            Running._ratio_sets = Unifier.getUnifiedSets();
            clusterSet.update(Running);
        }
    }
}

/** Not implemented - throws prg_error */
void ClosedLoopMultiSetNormalTemporalDataEvaluator::CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) {
    throw prg_error("CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) not implemented.","ClosedLoopMultiSetNormalTemporalDataEvaluator");
}

/** Iterates through defined temporal window for accumulated cluster data.
    Calculates greatest loglikelihood ratio among clusterings that have rates
    which we are interested in. Returns greatest loglikelihood ratio. */
double ClosedLoopMultiSetNormalTemporalDataEvaluator::ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData) {
    AbstractMultiSetNormalTemporalData & Data = (AbstractMultiSetNormalTemporalData&)(ClusterData);//GetClusterDataAsType<AbstractMultiSetNormalTemporalData>(ClusterData);
    AbstractLoglikelihoodRatioUnifier & Unifier = gLikelihoodCalculator.GetUnifier();
    double dRatio(0);

    int iWindowStart, iMaxStartWindow;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd=ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iMaxStartWindow = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength() + 1);
        for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
            Unifier.Reset();
            for (size_t t=0; t < Data.gvSetClusterData.size(); ++t) {
                NormalTemporalData & Datum = *(Data.gvSetClusterData[t]);
                //printf("start = %d, end = %d\n", window_interval->first, window_interval->second);
                Datum.gtCases = Datum.gpCases[iWindowStart] - Datum.gpCases[std::min(iWindowEnd, _extended_period_start)];
                Datum.gtCases += Datum.gpCases[0] - Datum.gpCases[std::max(0, iWindowEnd - _extended_period_start)];
                Datum.gtMeasure = Datum.gpMeasure[iWindowStart] - Datum.gpMeasure[std::min(iWindowEnd, _extended_period_start)];
                Datum.gtMeasure += Datum.gpMeasure[0] - Datum.gpMeasure[std::max(0, iWindowEnd - _extended_period_start)];
                Datum.gtMeasureAux = Datum.gpMeasureAux[iWindowStart] - Datum.gpMeasureAux[std::min(iWindowEnd, _extended_period_start)];
                Datum.gtMeasureAux += Datum.gpMeasureAux[0] - Datum.gpMeasureAux[std::max(0, iWindowEnd - _extended_period_start)];
                Unifier.AdjoinRatio(gLikelihoodCalculator, Datum.gtCases, Datum.gtMeasure, Datum.gtMeasureAux, t);
            }
            dRatio = std::max(dRatio, Unifier.GetLoglikelihoodRatio());
        }
    }
    return dRatio;
}

//********** CategoricalTemporalDataEvaluator **********

/** constructor */
CategoricalTemporalDataEvaluator::CategoricalTemporalDataEvaluator(
    const CSaTScanData& DataHub, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType
): CTimeIntervals(DataHub, Calculator, eIncludeClustersType){
    if (eExecutionType == CENTRICALLY) {
        gpCalculationMethod = &AbstractLikelihoodCalculator::CalcLogLikelihoodRatioOrdinal;
        gdDefaultMaximizingValue = 0;
    } else {
        gpCalculationMethod = &AbstractLikelihoodCalculator::CalculateMaximizingValueOrdinal;
        gdDefaultMaximizingValue = -std::numeric_limits<double>::max();
    }
}

/** Iterates through defined temporal window for accumulated data of 'Running' cluster. Calculates loglikelihood ratio
    of clusters that have rates of which we are interested in and updates clusterset accordingly. */
void CategoricalTemporalDataEvaluator::CompareClusterSet(CCluster& Running, CClusterSet& ClusterSet) {
    CategoricalTemporalData & Data = (CategoricalTemporalData&)*(Running.GetClusterData());//GetClusterDataAsType<CategoricalTemporalData>(*(Running.GetClusterData()));

    int iWindowStart, iMinStartWindow;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd=ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iMinStartWindow = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iWindowStart = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength());
        for (; iWindowStart >= iMinStartWindow; --iWindowStart) {
            for (size_t t=0; t < Data.gvCasesPerCategory.size(); ++t)
                Data.gvCasesPerCategory[t] = Data.gppCategoryCases[t][iWindowStart] - Data.gppCategoryCases[t][iWindowEnd];
            Running.m_nFirstInterval = iWindowStart;
            Running.m_nLastInterval = iWindowEnd;
            Running.m_nRatio = gLikelihoodCalculator.CalcLogLikelihoodRatioOrdinal(Data.gvCasesPerCategory);
            ClusterSet.update(Running);
        }
    }
}

/** Not implemented - throws prg_error */
void CategoricalTemporalDataEvaluator::CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) {
    throw prg_error("CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) not implemented.","CategoricalTemporalDataEvaluator");
}

/** Iterates through defined temporal window for accumulated cluster data.
    Calculates greatest loglikelihood ratio among clusterings that have rates
    which we are interested in. Returns greatest loglikelihood ratio. */
double CategoricalTemporalDataEvaluator::ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData) {
    CategoricalTemporalData & Data = (CategoricalTemporalData&)ClusterData;//GetClusterDataAsType<CategoricalTemporalData>(ClusterData);
    double dMaxValue(gdDefaultMaximizingValue);

    int iWindowStart, iMaxStartWindow;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(_interval_range.get<3>(), _interval_range.get<1>() + giMaxWindowLength);
    for (int iWindowEnd=_interval_range.get<2>(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), _interval_range.get<0>());
        iMaxStartWindow = std::min(_interval_range.get<1>() + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength() + 1);
        for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
            for (size_t t=0; t < Data.gvCasesPerCategory.size(); ++t)
                Data.gvCasesPerCategory[t] = Data.gppCategoryCases[t][iWindowStart] - Data.gppCategoryCases[t][iWindowEnd];
            dMaxValue = std::max(dMaxValue, (gLikelihoodCalculator.*gpCalculationMethod)(Data.gvCasesPerCategory, 0));
        }
    }
    return dMaxValue;
}

//********** ClosedLoopCategoricalTemporalDataEvaluator **********

/** constructor */
ClosedLoopCategoricalTemporalDataEvaluator::ClosedLoopCategoricalTemporalDataEvaluator(
    const CSaTScanData& DataHub, AbstractLikelihoodCalculator & Calculator, IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType
): CTimeIntervals(DataHub, Calculator, eIncludeClustersType){
    if (eExecutionType == CENTRICALLY) {
        gpCalculationMethod = &AbstractLikelihoodCalculator::CalcLogLikelihoodRatioOrdinal;
        gdDefaultMaximizingValue = 0;
    } else {
        gpCalculationMethod = &AbstractLikelihoodCalculator::CalculateMaximizingValueOrdinal;
        gdDefaultMaximizingValue = -std::numeric_limits<double>::max();
    }

    // re-define interval period to include extended period
    const ClosedLoopData * dhub = dynamic_cast<const ClosedLoopData*>(&DataHub);
    if (!dhub) throw prg_error("Unable to dynamic cast CSaTScanData to ClosedLoopData.", "constructor()");
    switch (eIncludeClustersType) {
        case ALLCLUSTERS        : 
            _interval_range = IntervalRange_t(0, dhub->getExtendedPeriodStart() - 1, gDataHub.getMinTimeIntervalCut(), dhub->getNumExtendedTimeIntervals()); break;
        default : throw prg_error("Unknown cluster inclusion type: '%d'.", "Setup()", gDataHub.GetParameters().GetIncludeClustersType());
    };
    _init_interval_range = _interval_range; // store initial range settings
    _extended_period_start = dhub->getExtendedPeriodStart();
}

/** Iterates through defined temporal window for accumulated data of 'Running' cluster. Calculates loglikelihood ratio
    of clusters that have rates of which we are interested in and updates clusterset accordingly. */
void ClosedLoopCategoricalTemporalDataEvaluator::CompareClusterSet(CCluster& Running, CClusterSet& ClusterSet) {
    CategoricalTemporalData & Data = (CategoricalTemporalData&)*(Running.GetClusterData());//GetClusterDataAsType<CategoricalTemporalData>(*(Running.GetClusterData()));

    int iWindowStart, iMinStartWindow;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd=ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iMinStartWindow = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iWindowStart = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength());
        for (; iWindowStart >= iMinStartWindow; --iWindowStart) {
            //printf("start = %d, end = %d\n", iWindowStart, iWindowEnd);
            for (size_t t=0; t < Data.gvCasesPerCategory.size(); ++t) {
                Data.gvCasesPerCategory[t] = Data.gppCategoryCases[t][iWindowStart] - Data.gppCategoryCases[t][std::min(iWindowEnd, _extended_period_start)];
                Data.gvCasesPerCategory[t] += Data.gppCategoryCases[t][0] - Data.gppCategoryCases[t][std::max(0, iWindowEnd - _extended_period_start)];
            }
            Running.m_nFirstInterval = iWindowStart;
            Running.m_nLastInterval = iWindowEnd;
            Running.m_nRatio = gLikelihoodCalculator.CalcLogLikelihoodRatioOrdinal(Data.gvCasesPerCategory);
            ClusterSet.update(Running);
        }
    }
}

/** Not implemented - throws prg_error */
void ClosedLoopCategoricalTemporalDataEvaluator::CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) {
    throw prg_error("CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) not implemented.","ClosedLoopCategoricalTemporalDataEvaluator");
}

/** Iterates through defined temporal window for accumulated cluster data.
    Calculates greatest loglikelihood ratio among clusterings that have rates
    which we are interested in. Returns greatest loglikelihood ratio. */
double ClosedLoopCategoricalTemporalDataEvaluator::ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData) {
    CategoricalTemporalData & Data = (CategoricalTemporalData&)ClusterData;//GetClusterDataAsType<CategoricalTemporalData>(ClusterData);
    double dMaxValue(gdDefaultMaximizingValue);

    int iWindowStart, iMaxStartWindow;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(_interval_range.get<3>(), _interval_range.get<1>() + giMaxWindowLength);
    for (int iWindowEnd=_interval_range.get<2>(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), _interval_range.get<0>());
        iMaxStartWindow = std::min(_interval_range.get<1>() + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength() + 1);
        for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
            //printf("start = %d, end = %d\n", iWindowStart, iWindowEnd);
            for (size_t t=0; t < Data.gvCasesPerCategory.size(); ++t) {
                Data.gvCasesPerCategory[t] = Data.gppCategoryCases[t][iWindowStart] - Data.gppCategoryCases[t][std::min(iWindowEnd, _extended_period_start)];
                Data.gvCasesPerCategory[t] += Data.gppCategoryCases[t][0] - Data.gppCategoryCases[t][std::max(0, iWindowEnd - _extended_period_start)];
            }
            dMaxValue = std::max(dMaxValue, (gLikelihoodCalculator.*gpCalculationMethod)(Data.gvCasesPerCategory, 0));
        }
    }
    return dMaxValue;
}

//********** MultiSetCategoricalTemporalDataEvaluator **********

/** constructor */
MultiSetCategoricalTemporalDataEvaluator::MultiSetCategoricalTemporalDataEvaluator(
    const CSaTScanData& DataHub, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType
): CTimeIntervals(DataHub, Calculator, eIncludeClustersType) {}

/** Iterates through defined temporal window for accumulated data of 'Running' cluster. Calculates loglikelihood ratio
    of clusters that have rates of which we are interested in and updates clusterset accordingly. */
void MultiSetCategoricalTemporalDataEvaluator::CompareClusterSet(CCluster& Running, CClusterSet& clusterSet) {
    AbstractMultiSetCategoricalTemporalData & Data = (AbstractMultiSetCategoricalTemporalData&)*(Running.GetClusterData());//GetClusterDataAsType<AbstractMultiSetCategoricalTemporalData>(*(Running.GetClusterData()));
    AbstractLoglikelihoodRatioUnifier & Unifier = gLikelihoodCalculator.GetUnifier();

    int iWindowStart, iMinStartWindow;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(_interval_range.get<3>(), _interval_range.get<1>() + giMaxWindowLength);
    for (int iWindowEnd=_interval_range.get<2>(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iMinStartWindow = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), _interval_range.get<0>());
        iWindowStart = std::min(_interval_range.get<1>() + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength());
        for (; iWindowStart >= iMinStartWindow; --iWindowStart) {
            Unifier.Reset();
            for (size_t t=0; t < Data.gvSetClusterData.size(); ++t) {
                CategoricalTemporalData& Datum = *(Data.gvSetClusterData[t]);
                for (size_t c=0; c < Datum.gvCasesPerCategory.size(); ++c)
                    Datum.gvCasesPerCategory[c] = Datum.gppCategoryCases[c][iWindowStart] - Datum.gppCategoryCases[c][iWindowEnd];
                Unifier.AdjoinRatio(gLikelihoodCalculator, Datum.gvCasesPerCategory, t);
            }
            Running.m_nFirstInterval = iWindowStart;
            Running.m_nLastInterval = iWindowEnd;
            Running.m_nRatio = Unifier.GetLoglikelihoodRatio();
            Running._ratio_sets = Unifier.getUnifiedSets();
            clusterSet.update(Running);
        }
    }
}

/** Not implemented - throws prg_error */
void MultiSetCategoricalTemporalDataEvaluator::CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) {
    throw prg_error("CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) not implemented.","MultiSetCategoricalTemporalDataEvaluator");
}

/** Iterates through defined temporal window for accumulated cluster data.
    Calculates greatest loglikelihood ratio among clusterings that have rates
    which we are interested in. Returns greatest loglikelihood ratio. */
double MultiSetCategoricalTemporalDataEvaluator::ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData) {
    AbstractMultiSetCategoricalTemporalData & Data = (AbstractMultiSetCategoricalTemporalData&)ClusterData;//GetClusterDataAsType<AbstractMultiSetCategoricalTemporalData>(ClusterData);
    AbstractLoglikelihoodRatioUnifier & Unifier = gLikelihoodCalculator.GetUnifier();
    double dRatio(0);

    int iWindowStart, iMaxStartWindow;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(_interval_range.get<3>(), _interval_range.get<1>() + giMaxWindowLength);
    for (int iWindowEnd=_interval_range.get<2>(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), _interval_range.get<0>());
        iMaxStartWindow = std::min(_interval_range.get<1>() + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength() + 1);
        for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
            Unifier.Reset();
            for (size_t t=0; t < Data.gvSetClusterData.size(); ++t) {
                CategoricalTemporalData& Datum = *(Data.gvSetClusterData[t]);
                for (size_t c=0; c < Datum.gvCasesPerCategory.size(); ++c)
                    Datum.gvCasesPerCategory[c] = Datum.gppCategoryCases[c][iWindowStart] - Datum.gppCategoryCases[c][iWindowEnd];
                Unifier.AdjoinRatio(gLikelihoodCalculator, Datum.gvCasesPerCategory, t);
            }
            dRatio = std::max(dRatio, Unifier.GetLoglikelihoodRatio());
        }
    }
    return dRatio;
}

//********** ClosedLoopMultiSetCategoricalTemporalDataEvaluator **********

/** constructor */
ClosedLoopMultiSetCategoricalTemporalDataEvaluator::ClosedLoopMultiSetCategoricalTemporalDataEvaluator(
    const CSaTScanData& DataHub, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType
): CTimeIntervals(DataHub, Calculator, eIncludeClustersType) {

    // re-define interval period to include extended period
    const ClosedLoopData * dhub = dynamic_cast<const ClosedLoopData*>(&gDataHub);
    if (!dhub) throw prg_error("Unable to dynamic cast CSaTScanData to ClosedLoopData.", "constructor()");
    switch (eIncludeClustersType) {
        case ALLCLUSTERS        : 
            _interval_range = IntervalRange_t(0, dhub->getExtendedPeriodStart() - 1, gDataHub.getMinTimeIntervalCut(), dhub->getNumExtendedTimeIntervals()); break;
        default : throw prg_error("Unknown cluster inclusion type: '%d'.", "Setup()", gDataHub.GetParameters().GetIncludeClustersType());
    };
    _init_interval_range = _interval_range; // store initial range settings
    _extended_period_start = dhub->getExtendedPeriodStart();
}

/** Iterates through defined temporal window for accumulated data of 'Running' cluster. Calculates loglikelihood ratio
    of clusters that have rates of which we are interested in and updates clusterset accordingly. */
void ClosedLoopMultiSetCategoricalTemporalDataEvaluator::CompareClusterSet(CCluster& Running, CClusterSet& clusterSet) {
    AbstractMultiSetCategoricalTemporalData & Data = (AbstractMultiSetCategoricalTemporalData&)*(Running.GetClusterData());//GetClusterDataAsType<AbstractMultiSetCategoricalTemporalData>(*(Running.GetClusterData()));
    AbstractLoglikelihoodRatioUnifier & Unifier = gLikelihoodCalculator.GetUnifier();

    int iWindowStart, iMinStartWindow;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(_interval_range.get<3>(), _interval_range.get<1>() + giMaxWindowLength);
    for (int iWindowEnd=_interval_range.get<2>(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iMinStartWindow = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), _interval_range.get<0>());
        iWindowStart = std::min(_interval_range.get<1>() + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength());
        for (; iWindowStart >= iMinStartWindow; --iWindowStart) {
            Unifier.Reset();
            for (size_t t=0; t < Data.gvSetClusterData.size(); ++t) {
                //printf("start = %d, end = %d\n", iWindowStart, iWindowEnd);
                CategoricalTemporalData& Datum = *(Data.gvSetClusterData[t]);
                for (size_t c=0; c < Datum.gvCasesPerCategory.size(); ++c) {
                    Datum.gvCasesPerCategory[c] = Datum.gppCategoryCases[c][iWindowStart] - Datum.gppCategoryCases[c][std::min(iWindowEnd, _extended_period_start)];
                    Datum.gvCasesPerCategory[c] += Datum.gppCategoryCases[c][0] - Datum.gppCategoryCases[c][std::max(0, iWindowEnd - _extended_period_start)];
                }
                Unifier.AdjoinRatio(gLikelihoodCalculator, Datum.gvCasesPerCategory, t);
            }
            Running.m_nFirstInterval = iWindowStart;
            Running.m_nLastInterval = iWindowEnd;
            Running.m_nRatio = Unifier.GetLoglikelihoodRatio();
            Running._ratio_sets = Unifier.getUnifiedSets();
            clusterSet.update(Running);
        }
    }
}

/** Not implemented - throws prg_error */
void ClosedLoopMultiSetCategoricalTemporalDataEvaluator::CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) {
    throw prg_error("CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) not implemented.","ClosedLoopMultiSetCategoricalTemporalDataEvaluator");
}

/** Iterates through defined temporal window for accumulated cluster data.
    Calculates greatest loglikelihood ratio among clusterings that have rates
    which we are interested in. Returns greatest loglikelihood ratio. */
double ClosedLoopMultiSetCategoricalTemporalDataEvaluator::ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData) {
    AbstractMultiSetCategoricalTemporalData & Data = (AbstractMultiSetCategoricalTemporalData&)ClusterData;//GetClusterDataAsType<AbstractMultiSetCategoricalTemporalData>(ClusterData);
    AbstractLoglikelihoodRatioUnifier & Unifier = gLikelihoodCalculator.GetUnifier();
    double dRatio(0);

    int iWindowStart, iMaxStartWindow;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(_interval_range.get<3>(), _interval_range.get<1>() + giMaxWindowLength);
    for (int iWindowEnd=_interval_range.get<2>(); iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), _interval_range.get<0>());
        iMaxStartWindow = std::min(_interval_range.get<1>() + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength() + 1);
        for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
            Unifier.Reset();
            for (size_t t=0; t < Data.gvSetClusterData.size(); ++t) {
                //printf("start = %d, end = %d\n", iWindowStart, iWindowEnd);
                CategoricalTemporalData& Datum = *(Data.gvSetClusterData[t]);
                for (size_t c=0; c < Datum.gvCasesPerCategory.size(); ++c) {
                    Datum.gvCasesPerCategory[c] = Datum.gppCategoryCases[c][iWindowStart] - Datum.gppCategoryCases[c][std::min(iWindowEnd, _extended_period_start)];
                    Datum.gvCasesPerCategory[c] += Datum.gppCategoryCases[c][0] - Datum.gppCategoryCases[c][std::max(0, iWindowEnd - _extended_period_start)];
                }
                Unifier.AdjoinRatio(gLikelihoodCalculator, Datum.gvCasesPerCategory, t);
            }
            dRatio = std::max(dRatio, Unifier.GetLoglikelihoodRatio());
        }
    }
    return dRatio;
}

//********** BatchedTemporalDataEvaluator **********

/** constructor */
BatchedTemporalDataEvaluator::BatchedTemporalDataEvaluator(
    const CSaTScanData& DataHub, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType
) : CTimeIntervals(DataHub, Calculator, eIncludeClustersType) {}

/** Iterates through defined temporal window for accumulated data of 'Running' cluster. Calculates loglikelihood ratio
    of clusters that have rates of which we are interested in and updates clusterset accordingly. */
void BatchedTemporalDataEvaluator::CompareClusterSet(CCluster& Running, CClusterSet& clusterSet) {
    BatchedTemporalData& Data = (BatchedTemporalData&)*(Running.GetClusterData());//GetClusterDataAsType<BatchedTemporalData>(*(Running.GetClusterData()));
    AbstractLikelihoodCalculator::SCANRATEBATCHED_FUNCPTR pRateCheck = gLikelihoodCalculator.gpRateOfInterestBatched;
    BatchedLikelihoodCalculator& batchedCalc = (BatchedLikelihoodCalculator&)gLikelihoodCalculator;
    ProbabilitiesAOI probabilities;

    int iWindowStart, iMinStartWindow;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd = ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iMinStartWindow = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iWindowStart = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength());
        for (; iWindowStart >= iMinStartWindow; --iWindowStart) {
            Data.gtCases = Data.gpCases[iWindowStart] - Data.gpCases[iWindowEnd];
            Data.gBatches = Data.gpBatches[iWindowStart] - Data.gpBatches[iWindowEnd];
            if ((gLikelihoodCalculator.*pRateCheck)(Data.gtCases, batchedCalc.getExpectedForBatches(Data.gBatches))) {
                Data.gtMeasure = Data.gpMeasure[iWindowStart] - Data.gpMeasure[iWindowEnd];
                Data.gtMeasureAux = Data.gpMeasureAux[iWindowStart] - Data.gpMeasureAux[iWindowEnd];
                Data.gtMeasureAux2 = Data.gpMeasureAux2[iWindowStart] - Data.gpMeasureAux2[iWindowEnd];
                Data.gPositiveBatches = Data.gpPositiveBatches[iWindowStart] - Data.gpPositiveBatches[iWindowEnd];
                batchedCalc.CalculateProbabilities(
                    probabilities, Data.gtCases, Data.gtMeasure, Data.gtMeasureAux2, Data.gtMeasureAux, Data.gPositiveBatches
                );
                Running.m_nFirstInterval = iWindowStart;
                Running.m_nLastInterval = iWindowEnd;
                Running.m_nRatio = batchedCalc.getLoglikelihoodRatio(probabilities);
                clusterSet.update(Running);
            }
        }
    }
}

/** Not implemented - throws prg_error */
void BatchedTemporalDataEvaluator::CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) {
    throw prg_error("CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) not implemented.", "BatchedTemporalDataEvaluator");
}

/** Iterates through defined temporal window for accumulated cluster data.
    Calculates greatest loglikelihood ratio among clusterings that have rates
    which we are interested in. Returns greatest loglikelihood ratio. */
double BatchedTemporalDataEvaluator::ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData) {
    BatchedTemporalData& Data = (BatchedTemporalData&)ClusterData;//GetClusterDataAsType<BatchedTemporalData>(ClusterData);
    double dMaxValue(-std::numeric_limits<double>::max());
    AbstractLikelihoodCalculator::SCANRATEBATCHED_FUNCPTR pRateCheck = gLikelihoodCalculator.gpRateOfInterestBatched;
    ProbabilitiesAOI probabilities;
    BatchedLikelihoodCalculator& batchedCalc = (BatchedLikelihoodCalculator&)gLikelihoodCalculator;

    //iterate through windows
    int iWindowStart, iMaxStartWindow;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd = ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iMaxStartWindow = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength() + 1);
        for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
            Data.gtCases = Data.gpCases[iWindowStart] - Data.gpCases[iWindowEnd];
            Data.gBatches = Data.gpBatches[iWindowStart] - Data.gpBatches[iWindowEnd];
            if ((gLikelihoodCalculator.*pRateCheck)(Data.gtCases, batchedCalc.getExpectedForBatches(Data.gBatches))) {
                batchedCalc.CalculateProbabilitiesForSimulation(
                    probabilities, Data.gtCases, 
                    Data.gpMeasure[iWindowStart] - Data.gpMeasure[iWindowEnd],
                    Data.gpMeasureAux2[iWindowStart] - Data.gpMeasureAux2[iWindowEnd],
                    Data.gpMeasureAux[iWindowStart] - Data.gpMeasureAux[iWindowEnd],
                    Data.gpPositiveBatches[iWindowStart] - Data.gpPositiveBatches[iWindowEnd]
                );
                dMaxValue = std::max(dMaxValue, ((BatchedLikelihoodCalculator&)gLikelihoodCalculator).getMaximizingValue(probabilities));
            }
        }
    }
    return dMaxValue;
}

//********** MultiSetBatchedTemporalDataEvaluator **********

/** constructor */
MultiSetBatchedTemporalDataEvaluator::MultiSetBatchedTemporalDataEvaluator(
    const CSaTScanData& DataHub, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType
) : CTimeIntervals(DataHub, Calculator, eIncludeClustersType) {}

/** Iterates through defined temporal window for accumulated data of 'Running' cluster. Calculates loglikelihood ratio
    of clusters that have rates of which we are interested in and updates clusterset accordingly. */
void MultiSetBatchedTemporalDataEvaluator::CompareClusterSet(CCluster& Running, CClusterSet& clusterSet) {
    AbstractMultiSetBatchedTemporalData& Data = (AbstractMultiSetBatchedTemporalData&)*(Running.GetClusterData());
    AbstractLoglikelihoodRatioUnifier& Unifier = gLikelihoodCalculator.GetUnifier();
    AbstractLikelihoodCalculator::SCANRATEMULTISET_FUNCPTR pRateCheck = gLikelihoodCalculator._rate_of_interest_multiset;

    int iWindowStart, iMinWindowStart;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd = ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iMinWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iWindowStart = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength());
        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
            Unifier.Reset();
            for (size_t t = 0; t < Data.gvSetClusterData.size(); ++t) {
                BatchedTemporalData& Datum = *(Data.gvSetClusterData[t]);
                Datum.gtCases = Datum.gpCases[iWindowStart] - Datum.gpCases[iWindowEnd];
                Datum.gtMeasure = Datum.gpMeasure[iWindowStart] - Datum.gpMeasure[iWindowEnd];
                Datum.gtMeasureAux = Datum.gpMeasureAux[iWindowStart] - Datum.gpMeasureAux[iWindowEnd];
                Datum.gtMeasureAux2 = Datum.gpMeasureAux2[iWindowStart] - Datum.gpMeasureAux2[iWindowEnd];
                Datum.gPositiveBatches = Datum.gpPositiveBatches[iWindowStart] - Datum.gpPositiveBatches[iWindowEnd];
                Datum.gBatches = Datum.gpBatches[iWindowStart] - Datum.gpBatches[iWindowEnd];
                Unifier.AdjoinRatio(
                    gLikelihoodCalculator, Datum.gtCases, Datum.gtMeasure, Datum.gtMeasureAux, 
                    Datum.gtMeasureAux2, Datum.gPositiveBatches, Datum.gBatches, t
                );
            }
            if ((gLikelihoodCalculator.*pRateCheck)(Unifier, false)) {
                Running.m_nFirstInterval = iWindowStart;
                Running.m_nLastInterval = iWindowEnd;
                Running.m_nRatio = Unifier.GetLoglikelihoodRatio();
                Running._ratio_sets = Unifier.getUnifiedSets();
                clusterSet.update(Running);
            }
        }
    }
}

/** Not implemented - throws prg_error */
void MultiSetBatchedTemporalDataEvaluator::CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) {
    throw prg_error("CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) not implemented.", "MultiSetBatchedTemporalDataEvaluator");
}

/** Iterates through defined temporal window for accumulated cluster data.
    Calculates greatest loglikelihood ratio among clusterings that have rates
    which we are interested in. Returns greatest loglikelihood ratio. */
double MultiSetBatchedTemporalDataEvaluator::ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData) {
    AbstractMultiSetBatchedTemporalData& Data = (AbstractMultiSetBatchedTemporalData&)(ClusterData);
    AbstractLoglikelihoodRatioUnifier& Unifier = gLikelihoodCalculator.GetUnifier();
    AbstractLikelihoodCalculator::SCANRATEMULTISET_FUNCPTR pRateCheck = gLikelihoodCalculator._rate_of_interest_multiset;
    double dRatio(0);

    int iWindowStart, iMaxStartWindow;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd = ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iMaxStartWindow = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength() + 1);
        for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
            Unifier.Reset();
            for (size_t t = 0; t < Data.gvSetClusterData.size(); ++t) {
                BatchedTemporalData& Datum = *(Data.gvSetClusterData[t]);
                Unifier.AdjoinRatioSimulation(gLikelihoodCalculator, 
                    Datum.gpCases[iWindowStart] - Datum.gpCases[iWindowEnd],
                    Datum.gpMeasure[iWindowStart] - Datum.gpMeasure[iWindowEnd],
                    Datum.gpMeasureAux[iWindowStart] - Datum.gpMeasureAux[iWindowEnd],
                    Datum.gpMeasureAux2[iWindowStart] - Datum.gpMeasureAux2[iWindowEnd],
                    Datum.gpPositiveBatches[iWindowStart] - Datum.gpPositiveBatches[iWindowEnd],
                    Datum.gpBatches[iWindowStart] - Datum.gpBatches[iWindowEnd], t
                );
            }
            if ((gLikelihoodCalculator.*pRateCheck)(Unifier, false))
                dRatio = std::max(dRatio, Unifier.GetLoglikelihoodRatio());
        }
    }
    return dRatio;
}

//********** MultiSetTimeStratifiedBatchedTemporalDataEvaluator **********

/** constructor */
MultiSetTimeStratifiedBatchedTemporalDataEvaluator::MultiSetTimeStratifiedBatchedTemporalDataEvaluator(
    const CSaTScanData& DataHub, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType
) : CTimeIntervals(DataHub, Calculator, eIncludeClustersType) {
    if (DataHub.GetParameters().GetTimeTrendAdjustmentType() != TEMPORAL_STRATIFIED_RANDOMIZATION)
        throw prg_error(
            "MultiSetTimeStratifiedBatchedTemporalDataEvaluator not implemented for selected time trend adjustment type.", "MultiSetTimeStratifiedBatchedTemporalDataEvaluator"
        );
    if (!DataHub.GetParameters().GetIsSpaceTimeAnalysis())
        throw prg_error(
            "MultiSetTimeStratifiedBatchedTemporalDataEvaluator only implemented for space-time analyses.", "MultiSetTimeStratifiedBatchedTemporalDataEvaluator"
        );
}

/** Iterates through defined temporal window for accumulated data of 'Running' cluster. Calculates loglikelihood ratio
    of clusters that have rates of which we are interested in and updates clusterset accordingly. */
void MultiSetTimeStratifiedBatchedTemporalDataEvaluator::CompareClusterSet(CCluster& Running, CClusterSet& clusterSet) {
    AbstractMultiSetBatchedTemporalData& Data = (AbstractMultiSetBatchedTemporalData&)*(Running.GetClusterData());
    AbstractLoglikelihoodRatioUnifier& Unifier = gLikelihoodCalculator.GetUnifier();
    BatchedLikelihoodCalculator& batchedCalc = (BatchedLikelihoodCalculator&)gLikelihoodCalculator;
    AbstractLikelihoodCalculator::SCANRATEMULTISET_FUNCPTR pRateCheck = gLikelihoodCalculator._rate_of_interest_multiset;
    ProbabilitiesRange_t probabilities;

    batchedCalc.clearCache(); // clear cached probabilities from previous iteration
    std::vector<double> cumulative_llr(Data.gvSetClusterData.size(), 0.0);
    std::vector<double> cumulative_expected(Data.gvSetClusterData.size(), 0.0);
    int iWindowStart, iMinWindowStart, iEvaluateStart;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd = ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iMinWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iEvaluateStart = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength());
        iWindowStart = iWindowEnd - 1;
        std::fill(cumulative_llr.begin(), cumulative_llr.end(), 0.0);
        std::fill(cumulative_expected.begin(), cumulative_expected.end(), 0.0);
        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
            Unifier.Reset();
            for (size_t t = 0; t < Data.gvSetClusterData.size(); ++t) {
                BatchedSpaceTimeData& Datum = (BatchedSpaceTimeData&)(*(Data.gvSetClusterData[t]));
                // In order to correctly calculate the accumulated LLR by time interval, we always need to calculate here.
                batchedCalc.CalculateProbabilitiesByTimeInterval(
                    probabilities,
                    Datum.gpCases[iWindowStart] - Datum.gpCases[iWindowStart + 1],
                    Datum.gpMeasure[iWindowStart] - Datum.gpMeasure[iWindowStart + 1],
                    Datum.gpMeasureAux2[iWindowStart] - Datum.gpMeasureAux2[iWindowStart + 1],
                    Datum.gpMeasureAux[iWindowStart] - Datum.gpMeasureAux[iWindowStart + 1],
                    Datum.gpPositiveBatches[iWindowStart] - Datum.gpPositiveBatches[iWindowStart + 1],
                    iWindowStart, t
                );
                // calculate the log-likelihood ratio for this time interval then add to cumulative
                double i_llr = batchedCalc.getLoglikelihoodRatioForInterval(*probabilities, iWindowStart, t);
                if (probabilities->_paoi._pinside < probabilities->_paoi._poutside)
                    cumulative_llr[t] += -1.0 * i_llr;
                else
                    cumulative_llr[t] += i_llr;
                // calculate the expected for this time interval then add to cumulative
                cumulative_expected[t] += batchedCalc.getClusterExpectedAtWindow(
                    Datum.gpBatches[iWindowStart] - Datum.gpBatches[iWindowStart + 1], iWindowStart, t
                );
                // For a prospective scan where the minimum temporal cluster size is greater than one, there will be intervals between
                // the start and end intervals that are not evaluated but contribute to the cumulative LLR.
                if (iWindowStart > iEvaluateStart)
                    continue;
                Datum.gtCases = Datum.gpCases[iWindowStart] - Datum.gpCases[iWindowEnd];
                Datum.gtMeasure = Datum.gpMeasure[iWindowStart] - Datum.gpMeasure[iWindowEnd];
                Datum.gtMeasureAux = Datum.gpMeasureAux[iWindowStart] - Datum.gpMeasureAux[iWindowEnd];
                Datum.gtMeasureAux2 = Datum.gpMeasureAux2[iWindowStart] - Datum.gpMeasureAux2[iWindowEnd];
                Datum.gPositiveBatches = Datum.gpPositiveBatches[iWindowStart] - Datum.gpPositiveBatches[iWindowEnd];
				Datum.gBatches = Datum.gpBatches[iWindowStart] - Datum.gpBatches[iWindowEnd];
                Unifier.AdjoinRatio(cumulative_llr[t], gLikelihoodCalculator, Datum.gtCases, cumulative_expected[t]);
            }
            if ((gLikelihoodCalculator.*pRateCheck)(Unifier, false)) {
                Running.m_nFirstInterval = iWindowStart;
                Running.m_nLastInterval = iWindowEnd;
                Running.m_nRatio = Unifier.GetLoglikelihoodRatio();
                Running._ratio_sets = Unifier.getUnifiedSets();
                clusterSet.update(Running);
            }
        }
    }
}

/** Not implemented - throws prg_error */
void MultiSetTimeStratifiedBatchedTemporalDataEvaluator::CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) {
    throw prg_error("CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) not implemented.", "MultiSetTimeStratifiedBatchedTemporalDataEvaluator");
}

/** Iterates through defined temporal window for accumulated cluster data.
    Calculates greatest loglikelihood ratio among clusterings that have rates
    which we are interested in. Returns greatest loglikelihood ratio. */
double MultiSetTimeStratifiedBatchedTemporalDataEvaluator::ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData) {
    AbstractMultiSetBatchedTemporalData& Data = (AbstractMultiSetBatchedTemporalData&)(ClusterData);
    AbstractLoglikelihoodRatioUnifier& Unifier = gLikelihoodCalculator.GetUnifier();
    BatchedLikelihoodCalculator& batchedCalc = (BatchedLikelihoodCalculator&)gLikelihoodCalculator;
    AbstractLikelihoodCalculator::SCANRATEMULTISET_FUNCPTR pRateCheck = gLikelihoodCalculator._rate_of_interest_multiset;
    double dRatio(0);
    ProbabilitiesRange_t probabilities;

    batchedCalc.clearCache(); // clear cached probabilities from previous iteration
    std::vector<double> cumulative_llr(Data.gvSetClusterData.size(), 0.0);
    std::vector<double> cumulative_expected(Data.gvSetClusterData.size(), 0.0);
    int iWindowStart, iMinWindowStart, iEvaluateStart;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd = ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iMinWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iEvaluateStart = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength());
        iWindowStart = iWindowEnd - 1;
        std::fill(cumulative_llr.begin(), cumulative_llr.end(), 0.0);
        std::fill(cumulative_expected.begin(), cumulative_expected.end(), 0.0);
        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
            Unifier.Reset();
            for (size_t t = 0; t < Data.gvSetClusterData.size(); ++t) {
                BatchedSpaceTimeData& Datum = (BatchedSpaceTimeData&)(*(Data.gvSetClusterData[t]));
                // In order to correctly calculate the accumulated LLR by time interval, we always need to calculate here.
                batchedCalc.CalculateProbabilitiesForSimulationByTimeInterval(
                    probabilities,
                    Datum.gpCases[iWindowStart] - Datum.gpCases[iWindowStart + 1],
                    Datum.gpMeasure[iWindowStart] - Datum.gpMeasure[iWindowStart + 1],
                    Datum.gpMeasureAux2[iWindowStart] - Datum.gpMeasureAux2[iWindowStart + 1],
                    Datum.gpMeasureAux[iWindowStart] - Datum.gpMeasureAux[iWindowStart + 1],
                    Datum.gpPositiveBatches[iWindowStart] - Datum.gpPositiveBatches[iWindowStart + 1],
                    iWindowStart, t
                );
                double i_llr = batchedCalc.getLoglikelihoodRatioForInterval(*probabilities, iWindowStart, t);
                if (probabilities->_paoi._pinside < probabilities->_paoi._poutside)
                    cumulative_llr[t] += -1.0 * i_llr;
                else
                    cumulative_llr[t] += i_llr;
                // calculate the expected for this time interval then add to cumulative
                cumulative_expected[t] += batchedCalc.getClusterExpectedAtWindow(
                    Datum.gpBatches[iWindowStart] - Datum.gpBatches[iWindowStart + 1], iWindowStart, t
                );
                // For a prospective scan where the minimum temporal cluster size is greater than one, there will be intervals between
                // the start and end intervals that are not evaluated but contribute to the cumulative LLR.
                if (iWindowStart > iEvaluateStart)
                    continue;
                Unifier.AdjoinRatio(cumulative_llr[t], gLikelihoodCalculator, 
                    Datum.gpCases[iWindowStart] - Datum.gpCases[iWindowEnd], cumulative_expected[t]
                );
            }
            if ((gLikelihoodCalculator.*pRateCheck)(Unifier, false))
                dRatio = std::max(dRatio, Unifier.GetLoglikelihoodRatio());
        }
    }
    return dRatio;
}

//********** MultiSetTimeStratifiedBatchedTemporalDataEvaluatorEnhanced **********

/** constructor */
MultiSetTimeStratifiedBatchedTemporalDataEvaluatorEnhanced::MultiSetTimeStratifiedBatchedTemporalDataEvaluatorEnhanced(
    const CSaTScanData& DataHub, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType
) : CTimeIntervals(DataHub, Calculator, eIncludeClustersType) {
    if (DataHub.GetParameters().GetTimeTrendAdjustmentType() != TEMPORAL_STRATIFIED_RANDOMIZATION)
        throw prg_error(
            "MultiSetTimeStratifiedBatchedTemporalDataEvaluatorEnhanced not implemented for selected time trend adjustment type.", "MultiSetTimeStratifiedBatchedTemporalDataEvaluatorEnhanced"
        );
    if (!DataHub.GetParameters().GetIsSpaceTimeAnalysis())
        throw prg_error(
            "MultiSetTimeStratifiedBatchedTemporalDataEvaluatorEnhanced only implemented for space-time analyses.", "MultiSetTimeStratifiedBatchedTemporalDataEvaluatorEnhanced"
        );
}

/** Iterates through defined temporal window for accumulated data of 'Running' cluster. Calculates loglikelihood ratio
    of clusters that have rates of which we are interested in and updates clusterset accordingly. */
void MultiSetTimeStratifiedBatchedTemporalDataEvaluatorEnhanced::CompareClusterSet(CCluster& Running, CClusterSet& clusterSet) {
    AbstractMultiSetBatchedTemporalData& Data = (AbstractMultiSetBatchedTemporalData&)*(Running.GetClusterData());
    AbstractLoglikelihoodRatioUnifier& Unifier = gLikelihoodCalculator.GetUnifier();
    BatchedLikelihoodCalculator& batchedCalc = (BatchedLikelihoodCalculator&)gLikelihoodCalculator;
    AbstractLikelihoodCalculator::SCANRATEMULTISET_FUNCPTR pRateCheck = gLikelihoodCalculator._rate_of_interest_multiset;
    BatchedLikelihoodCalculator::ProbabilitiesContainer_t probabilities;

    batchedCalc.clearCache(); // clear cached probabilities from previous iteration
    double cumulative_llr, w_llr, expected;
    int iWindowStart, iMinWindowStart;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd = ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iMinWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iWindowStart = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength());
        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
            Unifier.Reset();
            for (size_t t = 0; t < Data.gvSetClusterData.size(); ++t) {
                BatchedSpaceTimeData& Datum = (BatchedSpaceTimeData&)(*(Data.gvSetClusterData[t]));
                // Calculate the probabilities for each adjustment window.
                batchedCalc.CalculateProbabilitiesForWindow(
                    Datum, iWindowStart, iWindowEnd - 1, probabilities, t
                );
                // Calculate the LLR from the calculated probabilities.
                cumulative_llr = 0;
                for (auto& probability : probabilities) {
                    w_llr = batchedCalc.getLoglikelihoodRatioForRange(*probability, t);
                    if (probability->_paoi._pinside < probability->_paoi._poutside)
                        cumulative_llr += -1.0 * w_llr;
                    else
                        cumulative_llr += w_llr;
                }
                Datum.gtCases = Datum.gpCases[iWindowStart] - Datum.gpCases[iWindowEnd];
                Datum.gtMeasure = Datum.gpMeasure[iWindowStart] - Datum.gpMeasure[iWindowEnd];
                Datum.gtMeasureAux2 = Datum.gpMeasureAux2[iWindowStart] - Datum.gpMeasureAux2[iWindowEnd];
                Datum.gtMeasureAux = Datum.gpMeasureAux[iWindowStart] - Datum.gpMeasureAux[iWindowEnd];
                Datum.gPositiveBatches = Datum.gpPositiveBatches[iWindowStart] - Datum.gpPositiveBatches[iWindowEnd];
                Datum.gBatches = Datum.gpBatches[iWindowStart] - Datum.gpBatches[iWindowEnd];
                Unifier.AdjoinRatio(cumulative_llr, gLikelihoodCalculator, Datum.gtCases, 
                    batchedCalc.getExpectedForBatches(Datum.gBatches, t)
                );
            }
            if ((gLikelihoodCalculator.*pRateCheck)(Unifier, false)) {
                Running.m_nFirstInterval = iWindowStart;
                Running.m_nLastInterval = iWindowEnd;
                Running.m_nRatio = Unifier.GetLoglikelihoodRatio();
                Running._ratio_sets = Unifier.getUnifiedSets();
                clusterSet.update(Running);
            }
        }
    }
}

/** Not implemented - throws prg_error */
void MultiSetTimeStratifiedBatchedTemporalDataEvaluatorEnhanced::CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) {
    throw prg_error("CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) not implemented.", "MultiSetTimeStratifiedBatchedTemporalDataEvaluatorEnhanced");
}

/** Iterates through defined temporal window for accumulated cluster data.
    Calculates greatest loglikelihood ratio among clusterings that have rates
    which we are interested in. Returns greatest loglikelihood ratio. */
double MultiSetTimeStratifiedBatchedTemporalDataEvaluatorEnhanced::ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData) {
    AbstractMultiSetBatchedTemporalData& Data = (AbstractMultiSetBatchedTemporalData&)(ClusterData);
    AbstractLoglikelihoodRatioUnifier& Unifier = gLikelihoodCalculator.GetUnifier();
    BatchedLikelihoodCalculator& batchedCalc = (BatchedLikelihoodCalculator&)gLikelihoodCalculator;
    AbstractLikelihoodCalculator::SCANRATEMULTISET_FUNCPTR pRateCheck = gLikelihoodCalculator._rate_of_interest_multiset;
    double dRatio(0);
    BatchedLikelihoodCalculator::ProbabilitiesContainer_t probabilities;

    batchedCalc.clearCache(); // clear cached probabilities from previous iteration
    double cumulative_llr, w_llr, expected;
    int iWindowStart, iMinWindowStart;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd = ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iMinWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iWindowStart = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength());
        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
            Unifier.Reset();
            for (size_t t = 0; t < Data.gvSetClusterData.size(); ++t) {
                BatchedSpaceTimeData& Datum = (BatchedSpaceTimeData&)(*(Data.gvSetClusterData[t]));
                batchedCalc.CalculateProbabilitiesForWindowForSimulation(
                    Datum, iWindowStart, iWindowEnd - 1, probabilities, t
                );
                cumulative_llr = 0.0;
                for (auto& probability: probabilities) {
                    w_llr = batchedCalc.getLoglikelihoodRatioForRange(*probability, t);
                    if (probability->_paoi._pinside < probability->_paoi._poutside)
                        cumulative_llr += -1.0 * w_llr;
                    else
                        cumulative_llr += w_llr;
                }
                Unifier.AdjoinRatio(cumulative_llr, gLikelihoodCalculator, Datum.gpCases[iWindowStart] - Datum.gpCases[iWindowEnd],
                    batchedCalc.getExpectedForBatches(Datum.gpBatches[iWindowStart] - Datum.gpBatches[iWindowEnd], t)
                );
            }
            if ((gLikelihoodCalculator.*pRateCheck)(Unifier, false))
                dRatio = std::max(dRatio, Unifier.GetLoglikelihoodRatio());
        }
    }
    return dRatio;
}

//********** ClosedLoopBatchedTemporalDataEvaluator **********

/** constructor */
ClosedLoopBatchedTemporalDataEvaluator::ClosedLoopBatchedTemporalDataEvaluator(
    const CSaTScanData& DataHub, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType
) : CTimeIntervals(DataHub, Calculator, eIncludeClustersType) {

    // re-define interval period to include extended period
    const ClosedLoopData* dhub = dynamic_cast<const ClosedLoopData*>(&gDataHub);
    if (!dhub) throw prg_error("Unable to dynamic cast CSaTScanData to ClosedLoopData.", "constructor()");
    switch (eIncludeClustersType) {
        case ALLCLUSTERS:
            _interval_range = IntervalRange_t(0, dhub->getExtendedPeriodStart() - 1, gDataHub.getMinTimeIntervalCut(), dhub->getNumExtendedTimeIntervals()); break;
        default: throw prg_error("Unknown cluster inclusion type: '%d'.", "Setup()", gDataHub.GetParameters().GetIncludeClustersType());
    };
    _init_interval_range = _interval_range; // store initial range settings
    _extended_period_start = dhub->getExtendedPeriodStart();
    gdDefaultMaximizingValue = -std::numeric_limits<double>::max();
}

/** Iterates through defined temporal window for accumulated data of 'Running' cluster. Calculates loglikelihood ratio
    of clusters that have rates of which we are interested in and updates clusterset accordingly. */
void ClosedLoopBatchedTemporalDataEvaluator::CompareClusterSet(CCluster& Running, CClusterSet& clusterSet) {
    BatchedTemporalData& Data = (BatchedTemporalData&)*(Running.GetClusterData());//GetClusterDataAsType<BatchedTemporalData>(*(Running.GetClusterData()));
    AbstractLikelihoodCalculator::SCANRATEBATCHED_FUNCPTR pRateCheck = gLikelihoodCalculator.gpRateOfInterestBatched;
    BatchedLikelihoodCalculator& batchedCalc = (BatchedLikelihoodCalculator&)gLikelihoodCalculator;
    ProbabilitiesAOI probabilities;

    int iWindowStart, iMinStartWindow;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd = ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iMinStartWindow = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iWindowStart = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength());
        for (; iWindowStart >= iMinStartWindow; --iWindowStart) {
            Data.gtCases = Data.gpCases[iWindowStart] - Data.gpCases[std::min(iWindowEnd, _extended_period_start)];
            Data.gtCases += Data.gpCases[0] - Data.gpCases[std::max(0, iWindowEnd - _extended_period_start)];
            Data.gtMeasure = Data.gpMeasure[iWindowStart] - Data.gpMeasure[std::min(iWindowEnd, _extended_period_start)];
            Data.gtMeasure += Data.gpMeasure[0] - Data.gpMeasure[std::max(0, iWindowEnd - _extended_period_start)];
            Data.gtMeasureAux = Data.gpMeasureAux[iWindowStart] - Data.gpMeasureAux[std::min(iWindowEnd, _extended_period_start)];
            Data.gtMeasureAux += Data.gpMeasureAux[0] - Data.gpMeasureAux[std::max(0, iWindowEnd - _extended_period_start)];
            Data.gtMeasureAux2 = Data.gpMeasureAux2[iWindowStart] - Data.gpMeasureAux2[std::min(iWindowEnd, _extended_period_start)];
            Data.gtMeasureAux2 += Data.gpMeasureAux2[0] - Data.gpMeasureAux2[std::max(0, iWindowEnd - _extended_period_start)];
            Data.gPositiveBatches = Data.gpPositiveBatches[iWindowStart] - Data.gpPositiveBatches[std::min(iWindowEnd, _extended_period_start)];
            Data.gPositiveBatches |= Data.gpPositiveBatches[0] - Data.gpPositiveBatches[std::max(0, iWindowEnd - _extended_period_start)];
            Data.gBatches = Data.gpBatches[iWindowStart] - Data.gpBatches[std::min(iWindowEnd, _extended_period_start)];
            Data.gBatches |= Data.gpBatches[0] - Data.gpBatches[std::max(0, iWindowEnd - _extended_period_start)];
            if ((gLikelihoodCalculator.*pRateCheck)(Data.gtCases, batchedCalc.getExpectedForBatches(Data.gBatches))) {
                batchedCalc.CalculateProbabilities(
                    probabilities, Data.gtCases, Data.gtMeasure, Data.gtMeasureAux2, Data.gtMeasureAux, Data.gPositiveBatches
                );
                Running.m_nFirstInterval = iWindowStart;
                Running.m_nLastInterval = iWindowEnd;
                Running.m_nRatio = batchedCalc.getLoglikelihoodRatio(probabilities);
                clusterSet.update(Running);
            }
        }
    }
}

/** Not implemented - throws prg_error */
void ClosedLoopBatchedTemporalDataEvaluator::CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) {
    throw prg_error("CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) not implemented.", "ClosedLoopBatchedTemporalDataEvaluator");
}

/** Iterates through defined temporal window for accumulated cluster data.
    Calculates greatest loglikelihood ratio among clusterings that have rates
    which we are interested in. Returns greatest loglikelihood ratio. */
double ClosedLoopBatchedTemporalDataEvaluator::ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData) {
    BatchedTemporalData& Data = (BatchedTemporalData&)ClusterData;//GetClusterDataAsType<BatchedTemporalData>(ClusterData);
    double dMaxValue(gdDefaultMaximizingValue);
    AbstractLikelihoodCalculator::SCANRATEBATCHED_FUNCPTR pRateCheck = gLikelihoodCalculator.gpRateOfInterestBatched;
    BatchedLikelihoodCalculator& batchedCalc = (BatchedLikelihoodCalculator&)gLikelihoodCalculator;
    ProbabilitiesAOI probabilities;

    //iterate through windows
    int iWindowStart, iMaxStartWindow;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd = ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iMaxStartWindow = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength() + 1);
        for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
            Data.gtCases = Data.gpCases[iWindowStart] - Data.gpCases[std::min(iWindowEnd, _extended_period_start)];
            Data.gtCases += Data.gpCases[0] - Data.gpCases[std::max(0, iWindowEnd - _extended_period_start)];
            Data.gBatches = Data.gpBatches[iWindowStart] - Data.gpBatches[std::min(iWindowEnd, _extended_period_start)];
            Data.gBatches |= Data.gpBatches[0] - Data.gpBatches[std::max(0, iWindowEnd - _extended_period_start)];
            if ((gLikelihoodCalculator.*pRateCheck)(Data.gtCases, batchedCalc.getExpectedForBatches(Data.gBatches))) {
                Data.gtMeasure = Data.gpMeasure[iWindowStart] - Data.gpMeasure[std::min(iWindowEnd, _extended_period_start)];
                Data.gtMeasure += Data.gpMeasure[0] - Data.gpMeasure[std::max(0, iWindowEnd - _extended_period_start)];
                Data.gtMeasureAux = Data.gpMeasureAux[iWindowStart] - Data.gpMeasureAux[std::min(iWindowEnd, _extended_period_start)];
                Data.gtMeasureAux += Data.gpMeasureAux[0] - Data.gpMeasureAux[std::max(0, iWindowEnd - _extended_period_start)];
                Data.gtMeasureAux2 = Data.gpMeasureAux2[iWindowStart] - Data.gpMeasureAux2[std::min(iWindowEnd, _extended_period_start)];
                Data.gtMeasureAux2 += Data.gpMeasureAux2[0] - Data.gpMeasureAux2[std::max(0, iWindowEnd - _extended_period_start)];
                Data.gPositiveBatches = Data.gpPositiveBatches[iWindowStart] - Data.gpPositiveBatches[std::min(iWindowEnd, _extended_period_start)];
                Data.gPositiveBatches |= Data.gpPositiveBatches[0] - Data.gpPositiveBatches[std::max(0, iWindowEnd - _extended_period_start)];
                batchedCalc.CalculateProbabilitiesForSimulation(
                    probabilities, Data.gtCases, Data.gtMeasure, Data.gtMeasureAux2, Data.gtMeasureAux, Data.gPositiveBatches
                );
                dMaxValue = std::max(dMaxValue, batchedCalc.getMaximizingValue(probabilities));
            }
        }
    }
    return dMaxValue;
}

//********** ClosedLoopMultiSetBatchedTemporalDataEvaluator **********

/** constructor */
ClosedLoopMultiSetBatchedTemporalDataEvaluator::ClosedLoopMultiSetBatchedTemporalDataEvaluator(
    const CSaTScanData& DataHub, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType
) : CTimeIntervals(DataHub, Calculator, eIncludeClustersType) {

    // re-define interval period to include extended period
    const ClosedLoopData* dhub = dynamic_cast<const ClosedLoopData*>(&gDataHub);
    if (!dhub) throw prg_error("Unable to dynamic cast CSaTScanData to ClosedLoopData.", "constructor()");
    switch (eIncludeClustersType) {
    case ALLCLUSTERS:
        _interval_range = IntervalRange_t(0, dhub->getExtendedPeriodStart() - 1, gDataHub.getMinTimeIntervalCut(), dhub->getNumExtendedTimeIntervals()); break;
    default: throw prg_error("Unknown cluster inclusion type: '%d'.", "Setup()", gDataHub.GetParameters().GetIncludeClustersType());
    };
    _init_interval_range = _interval_range; // store initial range settings
    _extended_period_start = dhub->getExtendedPeriodStart();
}

/** Iterates through defined temporal window for accumulated data of 'Running' cluster. Calculates loglikelihood ratio
    of clusters that have rates of which we are interested in and updates clusterset accordingly. */
void ClosedLoopMultiSetBatchedTemporalDataEvaluator::CompareClusterSet(CCluster& Running, CClusterSet& clusterSet) {
    AbstractMultiSetBatchedTemporalData& Data = (AbstractMultiSetBatchedTemporalData&)*(Running.GetClusterData());//GetClusterDataAsType<AbstractMultiSetNormalTemporalData>(*(Running.GetClusterData()));
    AbstractLoglikelihoodRatioUnifier& Unifier = gLikelihoodCalculator.GetUnifier();

    int iWindowStart, iMinWindowStart;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd = ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iMinWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iWindowStart = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength());
        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
            Unifier.Reset();
            for (size_t t = 0; t < Data.gvSetClusterData.size(); ++t) {
                BatchedTemporalData& Datum = *(Data.gvSetClusterData[t]);
                //printf("start = %d, end = %d\n", iWindowStart, iWindowEnd);
                Datum.gtCases = Datum.gpCases[iWindowStart] - Datum.gpCases[std::min(iWindowEnd, _extended_period_start)];
                Datum.gtCases += Datum.gpCases[0] - Datum.gpCases[std::max(0, iWindowEnd - _extended_period_start)];
                Datum.gtMeasure = Datum.gpMeasure[iWindowStart] - Datum.gpMeasure[std::min(iWindowEnd, _extended_period_start)];
                Datum.gtMeasure += Datum.gpMeasure[0] - Datum.gpMeasure[std::max(0, iWindowEnd - _extended_period_start)];
                Datum.gtMeasureAux = Datum.gpMeasureAux[iWindowStart] - Datum.gpMeasureAux[std::min(iWindowEnd, _extended_period_start)];
                Datum.gtMeasureAux += Datum.gpMeasureAux[0] - Datum.gpMeasureAux[std::max(0, iWindowEnd - _extended_period_start)];
                Datum.gtMeasureAux2 = Datum.gpMeasureAux2[iWindowStart] - Datum.gpMeasureAux2[std::min(iWindowEnd, _extended_period_start)];
                Datum.gtMeasureAux2 += Datum.gpMeasureAux2[0] - Datum.gpMeasureAux2[std::max(0, iWindowEnd - _extended_period_start)];
                Datum.gPositiveBatches = Datum.gpPositiveBatches[iWindowStart] - Datum.gpPositiveBatches[std::min(iWindowEnd, _extended_period_start)];
                Datum.gPositiveBatches |= Datum.gpPositiveBatches[0] - Datum.gpPositiveBatches[std::max(0, iWindowEnd - _extended_period_start)];
                Datum.gBatches = Datum.gpBatches[iWindowStart] - Datum.gpBatches[std::min(iWindowEnd, _extended_period_start)];
                Datum.gBatches |= Datum.gpBatches[0] - Datum.gpBatches[std::max(0, iWindowEnd - _extended_period_start)];
                Unifier.AdjoinRatio(
                    gLikelihoodCalculator, Datum.gtCases, Datum.gtMeasure, 
                    Datum.gtMeasureAux, Datum.gtMeasureAux2, Datum.gPositiveBatches, Datum.gBatches, t
                );
            }
            Running.m_nFirstInterval = iWindowStart;
            Running.m_nLastInterval = iWindowEnd;
            Running.m_nRatio = Unifier.GetLoglikelihoodRatio();
            Running._ratio_sets = Unifier.getUnifiedSets();
            clusterSet.update(Running);
        }
    }
}

/** Not implemented - throws prg_error */
void ClosedLoopMultiSetBatchedTemporalDataEvaluator::CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) {
    throw prg_error("CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) not implemented.", "ClosedLoopMultiSetBatchedTemporalDataEvaluator");
}

/** Iterates through defined temporal window for accumulated cluster data.
    Calculates greatest loglikelihood ratio among clusterings that have rates
    which we are interested in. Returns greatest loglikelihood ratio. */
double ClosedLoopMultiSetBatchedTemporalDataEvaluator::ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData) {
    AbstractMultiSetBatchedTemporalData& Data = (AbstractMultiSetBatchedTemporalData&)(ClusterData);//GetClusterDataAsType<AbstractMultiSetNormalTemporalData>(ClusterData);
    AbstractLoglikelihoodRatioUnifier& Unifier = gLikelihoodCalculator.GetUnifier();
    double dRatio(0);

    int iWindowStart, iMaxStartWindow;
    gpMaxWindowLengthIndicator->reset();
    int iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (int iWindowEnd = ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iMaxStartWindow = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength() + 1);
        for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
            Unifier.Reset();
            for (size_t t = 0; t < Data.gvSetClusterData.size(); ++t) {
                BatchedTemporalData& Datum = *(Data.gvSetClusterData[t]);
                //printf("start = %d, end = %d\n", window_interval->first, window_interval->second);
                Datum.gtCases = Datum.gpCases[iWindowStart] - Datum.gpCases[std::min(iWindowEnd, _extended_period_start)];
                Datum.gtCases += Datum.gpCases[0] - Datum.gpCases[std::max(0, iWindowEnd - _extended_period_start)];
                Datum.gtMeasure = Datum.gpMeasure[iWindowStart] - Datum.gpMeasure[std::min(iWindowEnd, _extended_period_start)];
                Datum.gtMeasure += Datum.gpMeasure[0] - Datum.gpMeasure[std::max(0, iWindowEnd - _extended_period_start)];
                Datum.gtMeasureAux = Datum.gpMeasureAux[iWindowStart] - Datum.gpMeasureAux[std::min(iWindowEnd, _extended_period_start)];
                Datum.gtMeasureAux += Datum.gpMeasureAux[0] - Datum.gpMeasureAux[std::max(0, iWindowEnd - _extended_period_start)];
                Datum.gtMeasureAux2 = Datum.gpMeasureAux2[iWindowStart] - Datum.gpMeasureAux2[std::min(iWindowEnd, _extended_period_start)];
                Datum.gtMeasureAux2 += Datum.gpMeasureAux2[0] - Datum.gpMeasureAux2[std::max(0, iWindowEnd - _extended_period_start)];
                Datum.gPositiveBatches = Datum.gpPositiveBatches[iWindowStart] - Datum.gpPositiveBatches[std::min(iWindowEnd, _extended_period_start)];
                Datum.gPositiveBatches |= Datum.gpPositiveBatches[0] - Datum.gpPositiveBatches[std::max(0, iWindowEnd - _extended_period_start)];
                Datum.gBatches = Datum.gpBatches[iWindowStart] - Datum.gpBatches[std::min(iWindowEnd, _extended_period_start)];
                Datum.gBatches |= Datum.gpBatches[0] - Datum.gpBatches[std::max(0, iWindowEnd - _extended_period_start)];
                Unifier.AdjoinRatioSimulation(
                    gLikelihoodCalculator, Datum.gtCases, Datum.gtMeasure, Datum.gtMeasureAux, 
                    Datum.gtMeasureAux2, Datum.gPositiveBatches, Datum.gBatches, t
                );
            }
            dRatio = std::max(dRatio, Unifier.GetLoglikelihoodRatio());
        }
    }
    return dRatio;
}

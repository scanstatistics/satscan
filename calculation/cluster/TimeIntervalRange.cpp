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
#include "MultiSetClusterData.h"
#include "CategoricalClusterData.h"
#include "MultiSetCategoricalClusterData.h"
#include "LoglikelihoodRatioUnifier.h"
#include "MultiSetNormalClusterData.h"
#include "SSException.h"
#include "ClosedLoopData.h"

//********** TemporalDataEvaluator **********

/** constructor */
TemporalDataEvaluator::TemporalDataEvaluator(const CSaTScanData& DataHub, AbstractLikelihoodCalculator & Calculator,
                                             IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType)
                      :CTimeIntervals(DataHub, Calculator, eIncludeClustersType) {
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
            if ((gLikelihoodCalculator.*pRateCheck)(Data.gtCases, Data.gtMeasure,0)) {
                Running.m_nRatio = gLikelihoodCalculator.CalcLogLikelihoodRatio(Data.gtCases, Data.gtMeasure);
                Running.m_nFirstInterval = iWindowStart;
                Running.m_nLastInterval = iWindowEnd;
                clusterSet.update(Running);
            }
        }
    }
    clusterSet.maximizeClusterSet();
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
            if ((gLikelihoodCalculator.*pRateCheck)(Data.gtCases, Data.gtMeasure, 0))
                dMaxValue = std::max(dMaxValue, (gLikelihoodCalculator.*gpCalculationMethod)(Data.gtCases, Data.gtMeasure, 0));
        }
    }
    return dMaxValue;
}

//********** ClosedLoopTemporalDataEvaluator **********

/** constructor */
ClosedLoopTemporalDataEvaluator::ClosedLoopTemporalDataEvaluator(const CSaTScanData& DataHub, 
                                                                 AbstractLikelihoodCalculator & Calculator,
                                                                 IncludeClustersType eIncludeClustersType, 
                                                                 ExecutionType eExecutionType)
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
            if ((gLikelihoodCalculator.*pRateCheck)(Data.gtCases, Data.gtMeasure,0)) {
                Running.m_nRatio = gLikelihoodCalculator.CalcLogLikelihoodRatio(Data.gtCases, Data.gtMeasure);
                Running.m_nFirstInterval = iWindowStart;
                Running.m_nLastInterval = iWindowEnd;
                clusterSet.update(Running);
            }
        }
    }
    clusterSet.maximizeClusterSet();
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
            if ((gLikelihoodCalculator.*pRateCheck)(Data.gtCases, Data.gtMeasure, 0))
                dMaxValue = std::max(dMaxValue, (gLikelihoodCalculator.*gpCalculationMethod)(Data.gtCases, Data.gtMeasure, 0));
        }
    }
    return dMaxValue;
}

//********** MultiSetTemporalDataEvaluator **********

/** constructor */
MultiSetTemporalDataEvaluator::MultiSetTemporalDataEvaluator(const CSaTScanData& DataHub,
                                                             AbstractLikelihoodCalculator & Calculator,
                                                             IncludeClustersType eIncludeClustersType)
                              :CTimeIntervals(DataHub, Calculator, eIncludeClustersType) {}

/** Iterates through defined temporal window for accumulated data of 'Running' cluster. Calculates loglikelihood ratio
    of clusters that have rates of which we are interested in and updates clusterset accordingly. */
void MultiSetTemporalDataEvaluator::CompareClusterSet(CCluster& Running, CClusterSet& clusterSet) {
    AbstractMultiSetTemporalData & Data = (AbstractMultiSetTemporalData&)*(Running.GetClusterData());//GetClusterDataAsType<AbstractMultiSetTemporalData>(*(Running.GetClusterData()));
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
                TemporalData & Datum = *(Data.gvSetClusterData[t]);
                Datum.gtCases = Datum.gpCases[iWindowStart] - Datum.gpCases[iWindowEnd];
                Datum.gtMeasure = Datum.gpMeasure[iWindowStart] - Datum.gpMeasure[iWindowEnd];
                Unifier.AdjoinRatio(gLikelihoodCalculator, Datum.gtCases, Datum.gtMeasure, t);
            }
            Running.m_nFirstInterval = iWindowStart;
            Running.m_nLastInterval = iWindowEnd;
            Running.m_nRatio = Unifier.GetLoglikelihoodRatio();
            clusterSet.update(Running);
        }
    }
    clusterSet.maximizeClusterSet();
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
            Running.m_nFirstInterval = iWindowStart;
            Running.m_nLastInterval = iWindowEnd;
            Running.m_nRatio = Unifier.GetLoglikelihoodRatio();
            clusterSet.update(Running);
        }
    }
    clusterSet.maximizeClusterSet();
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
            dRatio = std::max(dRatio, Unifier.GetLoglikelihoodRatio());
        }
    }
    return dRatio;
}

//********** NormalTemporalDataEvaluator **********

/** constructor */
NormalTemporalDataEvaluator::NormalTemporalDataEvaluator(const CSaTScanData& DataHub, AbstractLikelihoodCalculator & Calculator,
                                                         IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType)
                            :CTimeIntervals(DataHub, Calculator, eIncludeClustersType) {
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
            if ((gLikelihoodCalculator.*pRateCheck)(Data.gtCases, Data.gtMeasure, Data.gtMeasureAux, 0)) {
                Running.m_nFirstInterval = iWindowStart;
                Running.m_nLastInterval = iWindowEnd;
                Running.m_nRatio = gLikelihoodCalculator.CalcLogLikelihoodRatioNormal(Data.gtCases, Data.gtMeasure, Data.gtMeasureAux);
                clusterSet.update(Running);
            }
        }
    }
    clusterSet.maximizeClusterSet();
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
            if ((gLikelihoodCalculator.*pRateCheck)(Data.gtCases, Data.gtMeasure, Data.gtMeasureAux, 0))
                dMaxValue = std::max(dMaxValue, (gLikelihoodCalculator.*gpCalculationMethod)(Data.gtCases, Data.gtMeasure, Data.gtMeasureAux, 0));
        }
    }
    return dMaxValue;
}

//********** ClosedLoopNormalTemporalDataEvaluator **********

/** constructor */
ClosedLoopNormalTemporalDataEvaluator::ClosedLoopNormalTemporalDataEvaluator(const CSaTScanData& DataHub, AbstractLikelihoodCalculator & Calculator,
                                                         IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType)
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
            if ((gLikelihoodCalculator.*pRateCheck)(Data.gtCases, Data.gtMeasure, Data.gtMeasureAux, 0)) {
                Running.m_nFirstInterval = iWindowStart;
                Running.m_nLastInterval = iWindowEnd;
                Running.m_nRatio = gLikelihoodCalculator.CalcLogLikelihoodRatioNormal(Data.gtCases, Data.gtMeasure, Data.gtMeasureAux);
                clusterSet.update(Running);
            }
        }
    }
    clusterSet.maximizeClusterSet();
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
            if ((gLikelihoodCalculator.*pRateCheck)(Data.gtCases, Data.gtMeasure, Data.gtMeasureAux, 0))
                dMaxValue = std::max(dMaxValue, (gLikelihoodCalculator.*gpCalculationMethod)(Data.gtCases, Data.gtMeasure, Data.gtMeasureAux, 0));
        }
    }
    return dMaxValue;
}

//********** MultiSetNormalTemporalDataEvaluator **********

/** constructor */
MultiSetNormalTemporalDataEvaluator::MultiSetNormalTemporalDataEvaluator(const CSaTScanData& DataHub,
                                                             AbstractLikelihoodCalculator & Calculator,
                                                             IncludeClustersType eIncludeClustersType)
                                    :CTimeIntervals(DataHub, Calculator, eIncludeClustersType) {}

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
            clusterSet.update(Running);
        }
    }
    clusterSet.maximizeClusterSet();
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
ClosedLoopMultiSetNormalTemporalDataEvaluator::ClosedLoopMultiSetNormalTemporalDataEvaluator(const CSaTScanData& DataHub,
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
            clusterSet.update(Running);
        }
    }
    clusterSet.maximizeClusterSet();
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
CategoricalTemporalDataEvaluator::CategoricalTemporalDataEvaluator(const CSaTScanData& DataHub, AbstractLikelihoodCalculator & Calculator,
                                                                   IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType)
                                 :CTimeIntervals(DataHub, Calculator, eIncludeClustersType){
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
ClosedLoopCategoricalTemporalDataEvaluator::ClosedLoopCategoricalTemporalDataEvaluator(const CSaTScanData& DataHub, AbstractLikelihoodCalculator & Calculator,
                                                                                       IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType)
                                 :CTimeIntervals(DataHub, Calculator, eIncludeClustersType){
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
MultiSetCategoricalTemporalDataEvaluator::MultiSetCategoricalTemporalDataEvaluator(const CSaTScanData& DataHub,
                                                                                   AbstractLikelihoodCalculator& Calculator,
                                                                                   IncludeClustersType eIncludeClustersType)
                                         :CTimeIntervals(DataHub, Calculator, eIncludeClustersType) {}

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
            clusterSet.update(Running);
        }
    }
    clusterSet.maximizeClusterSet();
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
ClosedLoopMultiSetCategoricalTemporalDataEvaluator::ClosedLoopMultiSetCategoricalTemporalDataEvaluator(const CSaTScanData& DataHub,
                                                                                                       AbstractLikelihoodCalculator& Calculator,
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
            clusterSet.update(Running);
        }
    }
    clusterSet.maximizeClusterSet();
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

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

/** constructor */
TemporalDataEvaluator::TemporalDataEvaluator(const CSaTScanData& DataHub, AbstractLikelihoodCalculator & Calculator,
                                             IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType)
                      :CTimeIntervals(DataHub, Calculator, eIncludeClustersType) {
  if (eExecutionType == CENTRICALLY) {
    gpCalculationMethod = &AbstractLikelihoodCalculator::CalcLogLikelihoodRatio;
    gdDefaultMaximizingValue = 0;
  }
  else {
    gpCalculationMethod = &AbstractLikelihoodCalculator::CalculateMaximizingValue;
    gdDefaultMaximizingValue = -std::numeric_limits<double>::max();
  }
}

/** destructor */
TemporalDataEvaluator::~TemporalDataEvaluator() {}

/** Iterates through defined temporal window for accumulated data of 'Running'
    cluster. Calculates loglikelihood ratio of clusters that have rates of which
    we are interested in and compares against current top cluster; re-assigning
    top cluster to running cluster if calculated llr is greater than that defined
    by current top cluster.
    NOTE: Though parameters to this function are base class CCluster objects, only
          CPurelyTemporalCluster and CSpaceTimeCluster objects should be passed.
          It might be appropriate to introduce a common ancestor class for these
          classes someday.                                                      */
void TemporalDataEvaluator::CompareClusters(CCluster & Running, CCluster & TopCluster) {
  int            iWindowStart, iMinWindowStart, iWindowEnd, iMaxEndWindow;
  count_t        tCases;
  measure_t      tMeasure;
  TemporalData & Data = (TemporalData&)*(Running.GetClusterData());//GetClusterDataAsType<TemporalData>(*(Running.GetClusterData()));
  count_t      * pCases = Data.gpCases;
  measure_t    * pMeasure = Data.gpMeasure;
  AbstractLikelihoodCalculator::SCANRATE_FUNCPTR pRateCheck = gLikelihoodCalculator.gpRateOfInterest;

  //iterate through windows
  gpMaxWindowLengthIndicator->Reset();
  iMaxEndWindow = std::min(giEndRange_End, giStartRange_End + giMaxWindowLength);
  for (iWindowEnd=giEndRange_Start; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
     iMinWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->GetNextWindowLength(), giStartRange_Start);
     iWindowStart = std::min(giStartRange_End + 1, iWindowEnd) - 1;
     for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
        tCases = pCases[iWindowStart] - pCases[iWindowEnd];
        tMeasure = pMeasure[iWindowStart] - pMeasure[iWindowEnd];
        if ((gLikelihoodCalculator.*pRateCheck)(tCases, tMeasure,0)) {
          Running.m_nRatio = gLikelihoodCalculator.CalcLogLikelihoodRatio(tCases, tMeasure);
          if (Running.m_nRatio  > TopCluster.m_nRatio) {
            Data.gtCases = tCases;
            Data.gtMeasure = tMeasure;
            TopCluster.CopyEssentialClassMembers(Running);
            TopCluster.m_nFirstInterval = iWindowStart;
            TopCluster.m_nLastInterval = iWindowEnd;
          }
        }
     }
  }
}

/** Iterates through previously defined temporal window for accumulated data of
    AbstractTemporalClusterData object. For each evaluated window, calls method
    CMeasureList::AddMeasure(cases,measure). */
void TemporalDataEvaluator::CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList) {
  int                   iWindowStart, iWindowEnd, iMaxStartWindow, iMaxEndWindow;
  TemporalData        & Data = (TemporalData&)ClusterData;//GetClusterDataAsType<TemporalData>(ClusterData);
  count_t             * pCases = Data.gpCases;
  measure_t           * pMeasure = Data.gpMeasure;

  //iterate through windows
  gpMaxWindowLengthIndicator->Reset();
  iMaxEndWindow = std::min(giEndRange_End, giStartRange_End + giMaxWindowLength);
  for (iWindowEnd=giEndRange_Start; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
     iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->GetNextWindowLength(), giStartRange_Start);
     iMaxStartWindow = std::min(giStartRange_End + 1, iWindowEnd);
     for (; iWindowStart < iMaxStartWindow; ++iWindowStart)
        MeasureList.AddMeasure(pCases[iWindowStart] - pCases[iWindowEnd], pMeasure[iWindowStart] - pMeasure[iWindowEnd]);
  }
}

/** Iterates through defined temporal window for accumulated cluster data.
    Calculates greatest loglikelihood ratio among clusterings that have rates
    which we are interested in. Returns greatest loglikelihood ratio. */
double TemporalDataEvaluator::ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData) {
  int            iWindowStart, iWindowEnd, iMaxStartWindow, iMaxEndWindow;
  TemporalData & Data = (TemporalData&)ClusterData;//GetClusterDataAsType<TemporalData>(ClusterData);
  count_t      * pCases = Data.gpCases;
  measure_t    * pMeasure = Data.gpMeasure;
  double         dMaxValue(gdDefaultMaximizingValue);
  AbstractLikelihoodCalculator::SCANRATE_FUNCPTR pRateCheck = gLikelihoodCalculator.gpRateOfInterest;

  //iterate through windows
  gpMaxWindowLengthIndicator->Reset();
  iMaxEndWindow = std::min(giEndRange_End, giStartRange_End + giMaxWindowLength);
  for (iWindowEnd=giEndRange_Start; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
     iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->GetNextWindowLength(), giStartRange_Start);
     iMaxStartWindow = std::min(giStartRange_End + 1, iWindowEnd);
     for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
        Data.gtCases = pCases[iWindowStart] - pCases[iWindowEnd];
        Data.gtMeasure = pMeasure[iWindowStart] - pMeasure[iWindowEnd];
        if ((gLikelihoodCalculator.*pRateCheck)(Data.gtCases, Data.gtMeasure, 0))
          dMaxValue = std::max(dMaxValue, (gLikelihoodCalculator.*gpCalculationMethod)(Data.gtCases, Data.gtMeasure, 0));
     }
  }
  return dMaxValue;
}

//******************************************************************************

/** constructor */
MultiSetTemporalDataEvaluator::MultiSetTemporalDataEvaluator(const CSaTScanData& DataHub,
                                                             AbstractLikelihoodCalculator & Calculator,
                                                             IncludeClustersType eIncludeClustersType)
                              :CTimeIntervals(DataHub, Calculator, eIncludeClustersType) {}

/** Iterates through defined temporal window for accumulated data of 'Running'
    cluster. Calculates loglikelihood ratio of clusters that have rates of which
    we are interested in and compares against current top cluster; re-assigning
    top cluster to running cluster if calculated llr is greater than that defined
    by current top cluster.
    NOTE: Though parameters to this function are base class CCluster objects, only
          CPurelyTemporalCluster and CSpaceTimeCluster objects should be passed.
          It might be appropriate to introduce a common ancestor class for these
          classes someday.
    NOTE: This algorithm is identical to TemporalDataEvaluator::CompareClusters(...)
          with the deviation being the loop over multipe data sets and process
          of unifying the calculated log likelihood ratios. */
void MultiSetTemporalDataEvaluator::CompareClusters(CCluster & Running, CCluster & TopCluster) {
  int                                 iWindowStart, iMinWindowStart, iWindowEnd, iMaxEndWindow;
  AbstractMultiSetTemporalData      & Data = (AbstractMultiSetTemporalData&)*(Running.GetClusterData());//GetClusterDataAsType<AbstractMultiSetTemporalData>(*(Running.GetClusterData()));
  AbstractLoglikelihoodRatioUnifier & Unifier = gLikelihoodCalculator.GetUnifier();

  //iterate through windows
  gpMaxWindowLengthIndicator->Reset();
  iMaxEndWindow = std::min(giEndRange_End, giStartRange_End + giMaxWindowLength);
  for (iWindowEnd=giEndRange_Start; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
     iMinWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->GetNextWindowLength(), giStartRange_Start);
     iWindowStart = std::min(giStartRange_End + 1, iWindowEnd) - 1;
     for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
        Unifier.Reset();
        for (size_t t=0; t < Data.gvSetClusterData.size(); ++t) {
          TemporalData & Datum = *(Data.gvSetClusterData[t]);
          Datum.gtCases = Datum.gpCases[iWindowStart] - Datum.gpCases[iWindowEnd];
          Datum.gtMeasure = Datum.gpMeasure[iWindowStart] - Datum.gpMeasure[iWindowEnd];
          Unifier.AdjoinRatio(gLikelihoodCalculator, Datum.gtCases, Datum.gtMeasure, t);
        }
        Running.m_nRatio = Unifier.GetLoglikelihoodRatio();
        if (Running.m_nRatio > TopCluster.m_nRatio) {
          TopCluster.CopyEssentialClassMembers(Running);
          TopCluster.m_nFirstInterval = iWindowStart;
          TopCluster.m_nLastInterval = iWindowEnd;
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
  int                                   iWindowStart, iWindowEnd, iMaxStartWindow, iMaxEndWindow;
  AbstractMultiSetTemporalData        & Data = (AbstractMultiSetTemporalData&)ClusterData;//GetClusterDataAsType<AbstractMultiSetTemporalData>(ClusterData);
  AbstractLoglikelihoodRatioUnifier   & Unifier = gLikelihoodCalculator.GetUnifier();
  double                                dRatio(0);

  //iterate through windows
  gpMaxWindowLengthIndicator->Reset();
  iMaxEndWindow = std::min(giEndRange_End, giStartRange_End + giMaxWindowLength);
  for (iWindowEnd=giEndRange_Start; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
     iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->GetNextWindowLength(), giStartRange_Start);
     iMaxStartWindow = std::min(giStartRange_End + 1, iWindowEnd);
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

//******************************************************************************

/** constructor */
NormalTemporalDataEvaluator::NormalTemporalDataEvaluator(const CSaTScanData& DataHub, AbstractLikelihoodCalculator & Calculator,
                                                         IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType)
                            :CTimeIntervals(DataHub, Calculator, eIncludeClustersType) {
  if (eExecutionType == CENTRICALLY) {
    gpCalculationMethod = &AbstractLikelihoodCalculator::CalcLogLikelihoodRatioNormal;
    gdDefaultMaximizingValue = 0;
  }
  else {
    gpCalculationMethod = &AbstractLikelihoodCalculator::CalculateMaximizingValueNormal;
    gdDefaultMaximizingValue = -std::numeric_limits<double>::max();
  }
}

/** Iterates through defined temporal window for accumulated data of 'Running'
    cluster. Calculates loglikelihood ratio of clusters that have rates of which
    we are interested in and compares against current top cluster; re-assigning
    top cluster to running cluster if calculated llr is greater than that defined
    by current top cluster.
    NOTE: Though parameters to this function are base class CCluster objects, only
          CPurelyTemporalCluster and CSpaceTimeCluster objects should be passed.
          It might be appropriate to introduce a common ancestor class for these
          classes someday.
    NOTE: This algorithm is identical to TemporalDataEvaluator::CompareClusters(...)
          with the only deviation being the second measure. */
void NormalTemporalDataEvaluator::CompareClusters(CCluster & Running, CCluster & TopCluster) {
  int                  iWindowStart, iMinStartWindow, iWindowEnd, iMaxEndWindow;
  NormalTemporalData & Data = (NormalTemporalData&)*(Running.GetClusterData());//GetClusterDataAsType<NormalTemporalData>(*(Running.GetClusterData()));
  AbstractLikelihoodCalculator::SCANRATENORMAL_FUNCPTR pRateCheck = gLikelihoodCalculator.gpRateOfInterestNormal;

  //iterate through windows
  gpMaxWindowLengthIndicator->Reset();
  iMaxEndWindow = std::min(giEndRange_End, giStartRange_End + giMaxWindowLength);
  for (iWindowEnd=giEndRange_Start; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
     iMinStartWindow = std::max(iWindowEnd - gpMaxWindowLengthIndicator->GetNextWindowLength(), giStartRange_Start);
     iWindowStart = std::min(giStartRange_End + 1, iWindowEnd) - 1;
     for (; iWindowStart >= iMinStartWindow; --iWindowStart) {
        Data.gtCases = Data.gpCases[iWindowStart] - Data.gpCases[iWindowEnd];
        Data.gtMeasure = Data.gpMeasure[iWindowStart] - Data.gpMeasure[iWindowEnd];
        Data.gtMeasureAux = Data.gpMeasureAux[iWindowStart] - Data.gpMeasureAux[iWindowEnd];
        if ((gLikelihoodCalculator.*pRateCheck)(Data.gtCases, Data.gtMeasure, Data.gtMeasureAux, 0)) {
          Running.m_nRatio = gLikelihoodCalculator.CalcLogLikelihoodRatioNormal(Data.gtCases, Data.gtMeasure, Data.gtMeasureAux);
          if (Running.m_nRatio  > TopCluster.m_nRatio) {
            TopCluster.CopyEssentialClassMembers(Running);
            TopCluster.m_nFirstInterval = iWindowStart;
            TopCluster.m_nLastInterval = iWindowEnd;
          }
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
  int                  iWindowStart, iWindowEnd, iMaxStartWindow, iMaxEndWindow;
  NormalTemporalData & Data = (NormalTemporalData&)ClusterData;//GetClusterDataAsType<NormalTemporalData>(ClusterData);
  double               dMaxValue(gdDefaultMaximizingValue);
  AbstractLikelihoodCalculator::SCANRATENORMAL_FUNCPTR pRateCheck = gLikelihoodCalculator.gpRateOfInterestNormal;

  //iterate through windows
  gpMaxWindowLengthIndicator->Reset();
  iMaxEndWindow = std::min(giEndRange_End, giStartRange_End + giMaxWindowLength);
  for (iWindowEnd=giEndRange_Start; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
     iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->GetNextWindowLength(), giStartRange_Start);
     iMaxStartWindow = std::min(giStartRange_End + 1, iWindowEnd);
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

//******************************************************************************

/** constructor */
MultiSetNormalTemporalDataEvaluator::MultiSetNormalTemporalDataEvaluator(const CSaTScanData& DataHub,
                                                             AbstractLikelihoodCalculator & Calculator,
                                                             IncludeClustersType eIncludeClustersType)
                                    :CTimeIntervals(DataHub, Calculator, eIncludeClustersType) {}

/** Iterates through defined temporal window for accumulated data of 'Running'
    cluster. Calculates loglikelihood ratio of clusters that have rates of which
    we are interested in and compares against current top cluster; re-assigning
    top cluster to running cluster if calculated llr is greater than that defined
    by current top cluster.
    NOTE: Though parameters to this function are base class CCluster objects, only
          CPurelyTemporalCluster and CSpaceTimeCluster objects should be passed.
          It might be appropriate to introduce a common ancestor class for these
          classes someday.
    NOTE: This algorithm is identical to TemporalDataEvaluator::CompareClusters(...)
          with the deviation being the loop over multipe data sets and process
          of unifying the calculated log likelihood ratios. */
void MultiSetNormalTemporalDataEvaluator::CompareClusters(CCluster & Running, CCluster & TopCluster) {
  int                                 iWindowStart, iMinWindowStart, iWindowEnd, iMaxEndWindow;
  AbstractMultiSetNormalTemporalData& Data = (AbstractMultiSetNormalTemporalData&)*(Running.GetClusterData());//GetClusterDataAsType<AbstractMultiSetNormalTemporalData>(*(Running.GetClusterData()));
  AbstractLoglikelihoodRatioUnifier & Unifier = gLikelihoodCalculator.GetUnifier();

  //iterate through windows
  gpMaxWindowLengthIndicator->Reset();
  iMaxEndWindow = std::min(giEndRange_End, giStartRange_End + giMaxWindowLength);
  for (iWindowEnd=giEndRange_Start; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
     iMinWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->GetNextWindowLength(), giStartRange_Start);
     iWindowStart = std::min(giStartRange_End + 1, iWindowEnd) - 1;
     for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
        Unifier.Reset();
        for (size_t t=0; t < Data.gvSetClusterData.size(); ++t) {
          NormalTemporalData & Datum = *(Data.gvSetClusterData[t]);
          Datum.gtCases = Datum.gpCases[iWindowStart] - Datum.gpCases[iWindowEnd];
          Datum.gtMeasure = Datum.gpMeasure[iWindowStart] - Datum.gpMeasure[iWindowEnd];
          Datum.gtMeasureAux = Datum.gpMeasureAux[iWindowStart] - Datum.gpMeasureAux[iWindowEnd];
          Unifier.AdjoinRatio(gLikelihoodCalculator, Datum.gtCases, Datum.gtMeasure, Datum.gtMeasureAux, t);
        }
        Running.m_nRatio = Unifier.GetLoglikelihoodRatio();
        if (Running.m_nRatio > TopCluster.m_nRatio) {
          TopCluster.CopyEssentialClassMembers(Running);
          TopCluster.m_nFirstInterval = iWindowStart;
          TopCluster.m_nLastInterval = iWindowEnd;
        }
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
  int                                   iWindowStart, iWindowEnd, iMaxStartWindow, iMaxEndWindow;
  AbstractMultiSetNormalTemporalData  & Data = (AbstractMultiSetNormalTemporalData&)(ClusterData);//GetClusterDataAsType<AbstractMultiSetNormalTemporalData>(ClusterData);
  AbstractLoglikelihoodRatioUnifier   & Unifier = gLikelihoodCalculator.GetUnifier();
  double                                dRatio(0);

  //iterate through windows
  gpMaxWindowLengthIndicator->Reset();
  iMaxEndWindow = std::min(giEndRange_End, giStartRange_End + giMaxWindowLength);
  for (iWindowEnd=giEndRange_Start; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
     iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->GetNextWindowLength(), giStartRange_Start);
     iMaxStartWindow = std::min(giStartRange_End + 1, iWindowEnd);
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

//******************************************************************************

/** constructor */
CategoricalTemporalDataEvaluator::CategoricalTemporalDataEvaluator(const CSaTScanData& DataHub, AbstractLikelihoodCalculator & Calculator,
                                                                   IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType)
                                 :CTimeIntervals(DataHub, Calculator, eIncludeClustersType){
  if (eExecutionType == CENTRICALLY) {
    gpCalculationMethod = &AbstractLikelihoodCalculator::CalcLogLikelihoodRatioOrdinal;
    gdDefaultMaximizingValue = 0;
  }
  else {
    gpCalculationMethod = &AbstractLikelihoodCalculator::CalculateMaximizingValueOrdinal;
    gdDefaultMaximizingValue = -std::numeric_limits<double>::max();
  }
}

/** Iterates through defined temporal window for accumulated data of 'Running'
    cluster. Calculates loglikelihood ratio of clusters that have rates of which
    we are interested in and compares against current top cluster; re-assigning
    top cluster to running cluster if calculated llr is greater than that defined
    by current top cluster.
    NOTE: Though parameters to this function are base class CCluster objects, only
          CPurelyTemporalCluster and CSpaceTimeCluster objects should be passed.
          It might be appropriate to introduce a common ancestor class for these
          classes someday.
    NOTE: This algorithm is identical to TemporalDataEvaluator::CompareClusters(...)
          with the only deviation being the second measure. */
void CategoricalTemporalDataEvaluator::CompareClusters(CCluster& Running, CCluster& TopCluster) {
  int                       iWindowStart, iMinStartWindow, iWindowEnd, iMaxEndWindow;
  CategoricalTemporalData & Data = (CategoricalTemporalData&)*(Running.GetClusterData());//GetClusterDataAsType<CategoricalTemporalData>(*(Running.GetClusterData()));

  //iterate through windows
  gpMaxWindowLengthIndicator->Reset();
  iMaxEndWindow = std::min(giEndRange_End, giStartRange_End + giMaxWindowLength);
  for (iWindowEnd=giEndRange_Start; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
     iMinStartWindow = std::max(iWindowEnd - gpMaxWindowLengthIndicator->GetNextWindowLength(), giStartRange_Start);
     iWindowStart = std::min(giStartRange_End + 1, iWindowEnd) - 1;
     for (; iWindowStart >= iMinStartWindow; --iWindowStart) {
        for (size_t t=0; t < Data.gvCasesPerCategory.size(); ++t)
          Data.gvCasesPerCategory[t] = Data.gppCategoryCases[t][iWindowStart] - Data.gppCategoryCases[t][iWindowEnd];
        Running.m_nRatio = gLikelihoodCalculator.CalcLogLikelihoodRatioOrdinal(Data.gvCasesPerCategory);
        if (Running.m_nRatio  > TopCluster.m_nRatio) {
          TopCluster.CopyEssentialClassMembers(Running);
          TopCluster.m_nFirstInterval = iWindowStart;
          TopCluster.m_nLastInterval = iWindowEnd;
        }
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
  int                       iWindowStart, iWindowEnd, iMaxStartWindow, iMaxEndWindow;
  CategoricalTemporalData & Data = (CategoricalTemporalData&)ClusterData;//GetClusterDataAsType<CategoricalTemporalData>(ClusterData);
  double                    dMaxValue(gdDefaultMaximizingValue);

  //iterate through windows
  gpMaxWindowLengthIndicator->Reset();
  iMaxEndWindow = std::min(giEndRange_End, giStartRange_End + giMaxWindowLength);
  for (iWindowEnd=giEndRange_Start; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
     iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->GetNextWindowLength(), giStartRange_Start);
     iMaxStartWindow = std::min(giStartRange_End + 1, iWindowEnd);
     for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
        for (size_t t=0; t < Data.gvCasesPerCategory.size(); ++t)
          Data.gvCasesPerCategory[t] = Data.gppCategoryCases[t][iWindowStart] - Data.gppCategoryCases[t][iWindowEnd];
        dMaxValue = std::max(dMaxValue, (gLikelihoodCalculator.*gpCalculationMethod)(Data.gvCasesPerCategory, 0));
     }
  }
  return dMaxValue;
}

//******************************************************************************

/** constructor */
MultiSetCategoricalTemporalDataEvaluator::MultiSetCategoricalTemporalDataEvaluator(const CSaTScanData& DataHub,
                                                                                   AbstractLikelihoodCalculator& Calculator,
                                                                                   IncludeClustersType eIncludeClustersType)
                                         :CTimeIntervals(DataHub, Calculator, eIncludeClustersType) {}

/** Iterates through defined temporal window for accumulated data of 'Running'
    cluster. Calculates loglikelihood ratio of clusters that have rates of which
    we are interested in and compares against current top cluster; re-assigning
    top cluster to running cluster if calculated llr is greater than that defined
    by current top cluster.
    NOTE: Though parameters to this function are base class CCluster objects, only
          CPurelyTemporalCluster and CSpaceTimeCluster objects should be passed.
          It might be appropriate to introduce a common ancestor class for these
          classes someday.
    NOTE: This algorithm is identical to TemporalDataEvaluator::CompareClusters(...)
          with the only deviation being the second measure. */
void MultiSetCategoricalTemporalDataEvaluator::CompareClusters(CCluster& Running, CCluster& TopCluster) {
  int                                       iWindowStart, iMinStartWindow, iWindowEnd, iMaxEndWindow;
  AbstractMultiSetCategoricalTemporalData & Data = (AbstractMultiSetCategoricalTemporalData&)*(Running.GetClusterData());//GetClusterDataAsType<AbstractMultiSetCategoricalTemporalData>(*(Running.GetClusterData()));
  AbstractLoglikelihoodRatioUnifier       & Unifier = gLikelihoodCalculator.GetUnifier();

  //iterate through windows
  gpMaxWindowLengthIndicator->Reset();
  iMaxEndWindow = std::min(giEndRange_End, giStartRange_End + giMaxWindowLength);
  for (iWindowEnd=giEndRange_Start; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
     iMinStartWindow = std::max(iWindowEnd - gpMaxWindowLengthIndicator->GetNextWindowLength(), giStartRange_Start);
     iWindowStart = std::min(giStartRange_End + 1, iWindowEnd) - 1;
     for (; iWindowStart >= iMinStartWindow; --iWindowStart) {
        Unifier.Reset();
        for (size_t t=0; t < Data.gvSetClusterData.size(); ++t) {
          CategoricalTemporalData& Datum = *(Data.gvSetClusterData[t]);
          for (size_t c=0; c < Datum.gvCasesPerCategory.size(); ++c)
             Datum.gvCasesPerCategory[c] = Datum.gppCategoryCases[c][iWindowStart] - Datum.gppCategoryCases[c][iWindowEnd];
          Unifier.AdjoinRatio(gLikelihoodCalculator, Datum.gvCasesPerCategory, t);
        }
        Running.m_nRatio = Unifier.GetLoglikelihoodRatio();
        if (Running.m_nRatio > TopCluster.m_nRatio) {
          TopCluster.CopyEssentialClassMembers(Running);
          TopCluster.m_nFirstInterval = iWindowStart;
          TopCluster.m_nLastInterval = iWindowEnd;
        }
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
  int                                       iWindowStart, iWindowEnd, iMaxStartWindow, iMaxEndWindow;
  AbstractMultiSetCategoricalTemporalData & Data = (AbstractMultiSetCategoricalTemporalData&)ClusterData;//GetClusterDataAsType<AbstractMultiSetCategoricalTemporalData>(ClusterData);
  AbstractLoglikelihoodRatioUnifier       & Unifier = gLikelihoodCalculator.GetUnifier();
  double                                    dRatio(0);

  //iterate through windows
  gpMaxWindowLengthIndicator->Reset();
  iMaxEndWindow = std::min(giEndRange_End, giStartRange_End + giMaxWindowLength);
  for (iWindowEnd=giEndRange_Start; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
     iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->GetNextWindowLength(), giStartRange_Start);
     iMaxStartWindow = std::min(giStartRange_End + 1, iWindowEnd);
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


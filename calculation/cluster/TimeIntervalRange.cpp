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

/** constructor */
TemporalDataEvaluator::TemporalDataEvaluator(const CSaTScanData& Data,
                                     AbstractLikelihoodCalculator & Calculator,
                                     IncludeClustersType  eIncludeClustersType)
                  :CTimeIntervals(Data.GetNumTimeIntervals(), Data.GetTimeIntervalCut(), Data.GetParameters().GetAreaScanRateType()),
                   gData(Data), gLikelihoodCalculator(Calculator) {
  Init();                 
  Setup(Data, eIncludeClustersType);
}	

/** destructor */
TemporalDataEvaluator::~TemporalDataEvaluator() {
  try {
    delete gpMaxWindowLengthIndicator;
  }
  catch (...){}
}

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
  count_t        tCases, tTotalCases(gData.GetTotalCases());
  measure_t      tMeasure, tTotalMeasure(gData.GetTotalMeasure());
  TemporalData & Data =  GetClusterDataAsType<TemporalData>(*(Running.GetClusterData()));
  count_t      * pCases = Data.gpCases;
  measure_t    * pMeasure = Data.gpMeasure;

  //iterate through windows
  gpMaxWindowLengthIndicator->Reset();
  iMaxEndWindow = std::min(giEndRange_End, giStartRange_End + giMaxWindowLength);
  for (iWindowEnd=giEndRange_Start; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
     iMinWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->GetNextWindowLength(), giStartRange_Start);
     iWindowStart = std::min(giStartRange_End + 1, iWindowEnd) - 1;
     for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
        tCases = pCases[iWindowStart] - pCases[iWindowEnd];
        tMeasure = pMeasure[iWindowStart] - pMeasure[iWindowEnd];
        if (fRateOfInterest(tCases, tMeasure, tTotalCases, tTotalMeasure)) {
          Running.m_nRatio = gLikelihoodCalculator.CalcLogLikelihoodRatio(tCases, tMeasure, tTotalCases, tTotalMeasure);
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
  TemporalData        & Data = GetClusterDataAsType<TemporalData>(ClusterData);
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
double TemporalDataEvaluator::ComputeLoglikelihoodRatioClusterData(AbstractTemporalClusterData& ClusterData) {
  int            iWindowStart, iWindowEnd, iMaxStartWindow, iMaxEndWindow;
  count_t        tTotalCases(gData.GetTotalCases());
  measure_t      tTotalMeasure(gData.GetTotalMeasure());
  TemporalData & Data = GetClusterDataAsType<TemporalData>(ClusterData);
  count_t      * pCases = Data.gpCases;
  measure_t    * pMeasure = Data.gpMeasure;
  double         dRatio(0);

  //iterate through windows
  gpMaxWindowLengthIndicator->Reset();
  iMaxEndWindow = std::min(giEndRange_End, giStartRange_End + giMaxWindowLength);
  for (iWindowEnd=giEndRange_Start; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
     iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->GetNextWindowLength(), giStartRange_Start);
     iMaxStartWindow = std::min(giStartRange_End + 1, iWindowEnd);
     for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
        Data.gtCases = pCases[iWindowStart] - pCases[iWindowEnd];
        Data.gtMeasure = pMeasure[iWindowStart] - pMeasure[iWindowEnd];
        if (fRateOfInterest(Data.gtCases, Data.gtMeasure, tTotalCases, tTotalMeasure))
          dRatio = std::max(dRatio, gLikelihoodCalculator.CalcLogLikelihoodRatio(Data.gtCases, Data.gtMeasure, tTotalCases, tTotalMeasure));
     }
  }
  return dRatio;
}

/** internal setup function */
void TemporalDataEvaluator::Setup(const CSaTScanData& Data, IncludeClustersType  eIncludeClustersType) {
  try {
    if (Data.GetParameters().GetIsProspectiveAnalysis() && eIncludeClustersType == ALLCLUSTERS) {
      // For a prospective analysis with IncludeClustersType of ALLCLUSTERS, this situation indicates
      // that this object is being constructed for use in simulations. Perhaps there should be another
      // enumeration item such as ALIVEPROSPECTIVECLUSTERS to remove confusion?  
      giStartRange_Start = 0;
      giStartRange_End = Data.GetNumTimeIntervals();
      giEndRange_Start = Data.GetProspectiveStartIndex();
      giEndRange_End = Data.GetNumTimeIntervals();
      //the maximum window length varies when the analysis is prospective and
      //the maximum is defined as percentage of study period
      if (Data.GetParameters().GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE && Data.GetParameters().GetAdjustForEarlierAnalyses())
        gpMaxWindowLengthIndicator = new ProspectiveMaxWindowLengthIndicator(Data);
      else
        gpMaxWindowLengthIndicator = new FixedMaxWindowLengthIndicator(Data);
    }
    else {
      switch (eIncludeClustersType) {
        case ALLCLUSTERS     : giStartRange_Start = 0;
                               giStartRange_End = Data.GetNumTimeIntervals();
                               giEndRange_Start = 1;
                               giEndRange_End = Data.GetNumTimeIntervals(); break;
        case ALIVECLUSTERS   : giStartRange_Start = 0;
                               giStartRange_End = Data.GetNumTimeIntervals();
                               giEndRange_Start = Data.GetNumTimeIntervals();
                               giEndRange_End = Data.GetNumTimeIntervals(); break;
        case CLUSTERSINRANGE : giStartRange_Start = Data.GetFlexibleWindowStartRangeStartIndex();
                               giStartRange_End = Data.GetFlexibleWindowStartRangeEndIndex();
                               giEndRange_Start = Data.GetFlexibleWindowEndRangeStartIndex();
                               giEndRange_End = Data.GetFlexibleWindowEndRangeEndIndex(); break;
        default :
          ZdGenerateException("Unknown cluster inclusion type: '%d'.", "Setup()", Data.GetParameters().GetIncludeClustersType());
      };
      gpMaxWindowLengthIndicator = new FixedMaxWindowLengthIndicator(Data);
    }
  }
  catch (ZdException &x) {
    delete gpMaxWindowLengthIndicator; gpMaxWindowLengthIndicator=0;
    x.AddCallpath("setup()","TemporalDataEvaluator");
    throw;
  }
}

//******************************************************************************

/** constructor */
MultiSetTemporalDataEvaluator::MultiSetTemporalDataEvaluator(const CSaTScanData& Data,
                                                             AbstractLikelihoodCalculator & Calculator,
                                                             IncludeClustersType eIncludeClustersType)
                              :TemporalDataEvaluator(Data, Calculator, eIncludeClustersType) {}

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
  AbstractMultiSetTemporalData      & Data = GetClusterDataAsType<AbstractMultiSetTemporalData>(*(Running.GetClusterData()));
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
          Unifier.AdjoinRatio(gLikelihoodCalculator, Datum.gtCases, Datum.gtMeasure, Datum.gtTotalCases, Datum.gtTotalMeasure);
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

/** Not implemented - throws ZdException */
void MultiSetTemporalDataEvaluator::CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) {
  ZdGenerateException("CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) not implemented.","MultiSetTemporalDataEvaluator");
}

/** Iterates through defined temporal window for accumulated cluster data.
    Calculates greatest loglikelihood ratio among clusterings that have rates
    which we are interested in. Returns greatest loglikelihood ratio. */
double MultiSetTemporalDataEvaluator::ComputeLoglikelihoodRatioClusterData(AbstractTemporalClusterData& ClusterData) {
  int                                   iWindowStart, iWindowEnd, iMaxStartWindow, iMaxEndWindow;
  AbstractMultiSetTemporalData        & Data =  GetClusterDataAsType<AbstractMultiSetTemporalData>(ClusterData);
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
          Unifier.AdjoinRatio(gLikelihoodCalculator, Datum.gtCases, Datum.gtMeasure, Datum.gtTotalCases, Datum.gtTotalMeasure);
        }
        dRatio = std::max(dRatio, Unifier.GetLoglikelihoodRatio());
     }
  }
  return dRatio;
}

//******************************************************************************

/** constructor */
NormalTemporalDataEvaluator::NormalTemporalDataEvaluator(const CSaTScanData& Data,
                                                         AbstractLikelihoodCalculator & Calculator,
                                                         IncludeClustersType eIncludeClustersType)
                            :TemporalDataEvaluator(Data, Calculator, eIncludeClustersType) {}

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
  count_t              tTotalCases(gData.GetTotalCases());
  measure_t            tTotalMeasure(gData.GetTotalMeasure());
  NormalTemporalData & Data =  GetClusterDataAsType<NormalTemporalData>(*(Running.GetClusterData()));

  //iterate through windows
  gpMaxWindowLengthIndicator->Reset();
  iMaxEndWindow = std::min(giEndRange_End, giStartRange_End + giMaxWindowLength);
  for (iWindowEnd=giEndRange_Start; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
     iMinStartWindow = std::max(iWindowEnd - gpMaxWindowLengthIndicator->GetNextWindowLength(), giStartRange_Start);
     iWindowStart = std::min(giStartRange_End + 1, iWindowEnd) - 1;
     for (; iWindowStart >= iMinStartWindow; --iWindowStart) {
        Data.gtCases = Data.gpCases[iWindowStart] - Data.gpCases[iWindowEnd];
        Data.gtMeasure = Data.gpMeasure[iWindowStart] - Data.gpMeasure[iWindowEnd];
        Data.gtSqMeasure = Data.gpSqMeasure[iWindowStart] - Data.gpSqMeasure[iWindowEnd];
        if (fRateOfInterest(Data.gtCases, Data.gtMeasure, tTotalCases, tTotalMeasure)) {
          Running.m_nRatio = gLikelihoodCalculator.CalcLogLikelihoodRatioNormal(Data.gtCases, Data.gtMeasure, Data.gtSqMeasure, tTotalCases, tTotalMeasure);
          if (Running.m_nRatio  > TopCluster.m_nRatio) {
            TopCluster.CopyEssentialClassMembers(Running);
            TopCluster.m_nFirstInterval = iWindowStart;
            TopCluster.m_nLastInterval = iWindowEnd;
          }
        }
     }
  }
}

/** Not implemented - throws ZdException */
void NormalTemporalDataEvaluator::CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) {
  ZdGenerateException("CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) not implemented.","NormalTemporalDataEvaluator");
}

/** Iterates through defined temporal window for accumulated cluster data.
    Calculates greatest loglikelihood ratio among clusterings that have rates
    which we are interested in. Returns greatest loglikelihood ratio. */
double NormalTemporalDataEvaluator::ComputeLoglikelihoodRatioClusterData(AbstractTemporalClusterData& ClusterData) {
  int                  iWindowStart, iWindowEnd, iMaxStartWindow, iMaxEndWindow;
  count_t              tTotalCases(gData.GetTotalCases());
  measure_t            tTotalMeasure(gData.GetTotalMeasure());
  NormalTemporalData & Data = GetClusterDataAsType<NormalTemporalData>(ClusterData);
  double               dRatio(0);

  //iterate through windows
  gpMaxWindowLengthIndicator->Reset();
  iMaxEndWindow = std::min(giEndRange_End, giStartRange_End + giMaxWindowLength);
  for (iWindowEnd=giEndRange_Start; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
     iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->GetNextWindowLength(), giStartRange_Start);
     iMaxStartWindow = std::min(giStartRange_End + 1, iWindowEnd);
     for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
        Data.gtCases = Data.gpCases[iWindowStart] - Data.gpCases[iWindowEnd];
        Data.gtMeasure = Data.gpMeasure[iWindowStart] - Data.gpMeasure[iWindowEnd];
        Data.gtSqMeasure = Data.gpSqMeasure[iWindowStart] - Data.gpSqMeasure[iWindowEnd];
        if (fRateOfInterest(Data.gtCases, Data.gtMeasure, tTotalCases, tTotalMeasure))
          dRatio = std::max(dRatio, gLikelihoodCalculator.CalcLogLikelihoodRatioNormal(Data.gtCases, Data.gtMeasure, Data.gtSqMeasure, tTotalCases, tTotalMeasure));
     }
  }

  return dRatio;
}

//******************************************************************************

/** constructor */
CategoricalTemporalDataEvaluator::CategoricalTemporalDataEvaluator(const CSaTScanData& DataHub,
                                                                   AbstractLikelihoodCalculator & Calculator,
                                                                   IncludeClustersType eIncludeClustersType)
                                 :TemporalDataEvaluator(DataHub, Calculator, eIncludeClustersType) {

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
  CategoricalTemporalData & Data =  GetClusterDataAsType<CategoricalTemporalData>(*(Running.GetClusterData()));

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

/** Not implemented - throws ZdException */
void CategoricalTemporalDataEvaluator::CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) {
  ZdGenerateException("CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) not implemented.","CategoricalTemporalDataEvaluator");
}

/** Iterates through defined temporal window for accumulated cluster data.
    Calculates greatest loglikelihood ratio among clusterings that have rates
    which we are interested in. Returns greatest loglikelihood ratio. */
double CategoricalTemporalDataEvaluator::ComputeLoglikelihoodRatioClusterData(AbstractTemporalClusterData& ClusterData) {
  int                       iWindowStart, iWindowEnd, iMaxStartWindow, iMaxEndWindow;
  CategoricalTemporalData & Data =  GetClusterDataAsType<CategoricalTemporalData>(ClusterData);
  double                    dRatio(0);    

  //iterate through windows
  gpMaxWindowLengthIndicator->Reset();
  iMaxEndWindow = std::min(giEndRange_End, giStartRange_End + giMaxWindowLength);
  for (iWindowEnd=giEndRange_Start; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
     iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->GetNextWindowLength(), giStartRange_Start);
     iMaxStartWindow = std::min(giStartRange_End + 1, iWindowEnd);
     for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
        for (size_t t=0; t < Data.gvCasesPerCategory.size(); ++t)
          Data.gvCasesPerCategory[t] = Data.gppCategoryCases[t][iWindowStart] - Data.gppCategoryCases[t][iWindowEnd];
        dRatio = std::max(dRatio, gLikelihoodCalculator.CalcLogLikelihoodRatioOrdinal(Data.gvCasesPerCategory));
     }
  }

  return dRatio;
}

//******************************************************************************

/** constructor */
MultiSetCategoricalTemporalDataEvaluator::MultiSetCategoricalTemporalDataEvaluator(const CSaTScanData& DataHub,
                                                                                   AbstractLikelihoodCalculator& Calculator,
                                                                                   IncludeClustersType eIncludeClustersType)
                                         :TemporalDataEvaluator(DataHub, Calculator, eIncludeClustersType) {

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
void MultiSetCategoricalTemporalDataEvaluator::CompareClusters(CCluster& Running, CCluster& TopCluster) {
  int                                       iWindowStart, iMinStartWindow, iWindowEnd, iMaxEndWindow;
  AbstractMultiSetCategoricalTemporalData & Data = GetClusterDataAsType<AbstractMultiSetCategoricalTemporalData>(*(Running.GetClusterData()));
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

/** Not implemented - throws ZdException */
void MultiSetCategoricalTemporalDataEvaluator::CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) {
  ZdGenerateException("CompareMeasures(AbstractTemporalClusterData&, CMeasureList&) not implemented.","MultiSetCategoricalTemporalDataEvaluator");
}

/** Iterates through defined temporal window for accumulated cluster data.
    Calculates greatest loglikelihood ratio among clusterings that have rates
    which we are interested in. Returns greatest loglikelihood ratio. */
double MultiSetCategoricalTemporalDataEvaluator::ComputeLoglikelihoodRatioClusterData(AbstractTemporalClusterData& ClusterData) {
  int                                       iWindowStart, iWindowEnd, iMaxStartWindow, iMaxEndWindow;
  AbstractMultiSetCategoricalTemporalData & Data = GetClusterDataAsType<AbstractMultiSetCategoricalTemporalData>(ClusterData);
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

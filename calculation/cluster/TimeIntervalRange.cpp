//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "TimeIntervalRange.h"
#include "SaTScanData.h"
#include "MeasureList.h"
#include "cluster.h"

/** constructor */
TimeIntervalRange::TimeIntervalRange(const CSaTScanData& Data,
                                     AbstractLikelihoodCalculator & Calculator,
                                     IncludeClustersType  eIncludeClustersType)
                  :CTimeIntervals(Data.m_nTimeIntervals, Data.m_nIntervalCut, Data.GetParameters().GetAreaScanRateType()),
                   gData(Data), gLikelihoodCalculator(Calculator) {
  Init();                 
  Setup(Data, eIncludeClustersType);
}	

/** copy constructor */
TimeIntervalRange::TimeIntervalRange(const TimeIntervalRange & rhs)
                  :CTimeIntervals(rhs), gData(rhs.gData), gLikelihoodCalculator(rhs.gLikelihoodCalculator) {
  *this = rhs;
}

/** destructor */
TimeIntervalRange::~TimeIntervalRange() {
  try {
    delete gpMaxWindowLengthIndicator;
  }
  catch (...){}
}

/** overloaded assignment operator */
TimeIntervalRange & TimeIntervalRange::operator=(const TimeIntervalRange& rhs) {
  giStartRange_End = rhs.giStartRange_End;
  giEndRange_Start = rhs.giEndRange_Start;
  giEndRange_End = rhs.giEndRange_End;
  giMaxWindowLength = rhs.giMaxWindowLength;
  return *this;
}

/** returns newly cloned TimeIntervalRange object */
TimeIntervalRange * TimeIntervalRange::Clone() const {
 return new TimeIntervalRange(*this);
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
void TimeIntervalRange::CompareClusters(CCluster & Running, CCluster & TopCluster) {
  int            iWindowStart, iWindowEnd, iMaxStartWindow, iMaxEndWindow;
  count_t        tCases, tTotalCases(gData.GetTotalCases());
  measure_t      tMeasure, tTotalMeasure(gData.GetTotalMeasure());
  TemporalData * pData = (TemporalData*)Running.GetClusterData();  //dynamic cast?
  count_t      * pCases = pData->gpCases;
  measure_t    * pMeasure = pData->gpMeasure;


  //iterate through windows
  gpMaxWindowLengthIndicator->Reset();
  iMaxEndWindow = std::min(giEndRange_End, giStartRange_End + giMaxWindowLength);
  for (iWindowEnd=giEndRange_Start; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
     iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->GetNextWindowLength(), giStartRange_Start);
     iMaxStartWindow = std::min(giStartRange_End + 1, iWindowEnd);
     for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
        tCases = pCases[iWindowStart] - pCases[iWindowEnd];
        tMeasure = pMeasure[iWindowStart] - pMeasure[iWindowEnd];
        if (fRateOfInterest(tCases, tMeasure, tTotalCases, tTotalMeasure)) {
          Running.m_nRatio = gLikelihoodCalculator.CalcLogLikelihoodRatio(tCases, tMeasure, tTotalCases, tTotalMeasure);
          if (Running.m_nRatio  > TopCluster.m_nRatio) {
            pData->gtCases = tCases;
            pData->gtMeasure = tMeasure;
            TopCluster.AssignAsType(Running);
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
void TimeIntervalRange::CompareMeasures(TemporalData& StreamData, CMeasureList& MeasureList) {
  int                   iWindowStart, iWindowEnd, iMaxStartWindow, iMaxEndWindow;
  count_t             * pCases = StreamData.gpCases;
  measure_t           * pMeasure = StreamData.gpMeasure;


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

/** internal setup function */
void TimeIntervalRange::Setup(const CSaTScanData& Data, IncludeClustersType  eIncludeClustersType) {
  try {
    if (Data.GetParameters().GetIsProspectiveAnalysis() && eIncludeClustersType == ALLCLUSTERS) {
      giStartRange_Start = 0;
      giStartRange_End = Data.m_nTimeIntervals;
      giEndRange_Start = Data.m_nProspectiveIntervalStart;
      giEndRange_End = Data.m_nTimeIntervals;
      //the maximum window length varies when the analysis is prospective and
      //the maximum is defined as percentage of study period
      if (Data.GetParameters().GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE)
        gpMaxWindowLengthIndicator = new ProspectiveMaxWindowLengthIndicator(Data);
      else
        gpMaxWindowLengthIndicator = new FixedMaxWindowLengthIndicator(Data);
    }
    else {
      switch (eIncludeClustersType) {
        case ALLCLUSTERS     : giStartRange_Start = 0;
                               giStartRange_End = Data.m_nTimeIntervals;
                               giEndRange_Start = 1;
                               giEndRange_End = Data.m_nTimeIntervals; break;
        case ALIVECLUSTERS   : giStartRange_Start = 0;
                               giStartRange_End = Data.m_nTimeIntervals;
                               giEndRange_Start = Data.m_nTimeIntervals;
                               giEndRange_End = Data.m_nTimeIntervals; break;
        case CLUSTERSINRANGE : giStartRange_Start = Data.m_nStartRangeStartDateIndex;
                               giStartRange_End = Data.m_nStartRangeEndDateIndex;
                               giEndRange_Start = Data.m_nEndRangeStartDateIndex;
                               giEndRange_End = Data.m_nEndRangeEndDateIndex; break;
        default :
          ZdGenerateException("Unknown cluster inclusion type: '%d'.", "Setup()", Data.GetParameters().GetIncludeClustersType());
      };
      gpMaxWindowLengthIndicator = new FixedMaxWindowLengthIndicator(Data);
    }
  }
  catch (ZdException &x) {
    delete gpMaxWindowLengthIndicator; gpMaxWindowLengthIndicator=0;
    x.AddCallpath("setup()","TimeIntervalRange");
    throw;
  }
}


/** constructor */
NormalTimeIntervalRange::NormalTimeIntervalRange(const CSaTScanData& Data,
                                                 AbstractLikelihoodCalculator & Calculator,
                                                 IncludeClustersType eIncludeClustersType)
                  : TimeIntervalRange(Data, Calculator, eIncludeClustersType) {}

/** copy constructor */
NormalTimeIntervalRange::NormalTimeIntervalRange(const NormalTimeIntervalRange & rhs)
                        :TimeIntervalRange(rhs) {}

/** overloaded assignment operator */
NormalTimeIntervalRange & NormalTimeIntervalRange::operator=(const NormalTimeIntervalRange& rhs) {
  giStartRange_End = rhs.giStartRange_End;
  giEndRange_Start = rhs.giEndRange_Start;
  giEndRange_End = rhs.giEndRange_End;
  giMaxWindowLength = rhs.giMaxWindowLength;
  return *this;
}

/** returns newly cloned TimeIntervalRange object */
NormalTimeIntervalRange * NormalTimeIntervalRange::Clone() const {
 return new NormalTimeIntervalRange(*this);
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
    NOTE: This algorithm is identical to TimeIntervalRange::CompareClusters(...)
          with the only deviation being the second measure. */
void NormalTimeIntervalRange::CompareClusters(CCluster & Running, CCluster & TopCluster) {
  int                  iWindowStart, iWindowEnd, iMaxStartWindow, iMaxEndWindow;
  count_t              tCases, tTotalCases(gData.GetTotalCases());
  measure_t            tMeasure, tMeasure2, tTotalMeasure(gData.GetTotalMeasure());
  NormalTemporalData * pData = (NormalTemporalData*)Running.GetClusterData(); //dynamic cast ?

  //iterate through windows
  gpMaxWindowLengthIndicator->Reset();
  iMaxEndWindow = std::min(giEndRange_End, giStartRange_End + giMaxWindowLength);
  for (iWindowEnd=giEndRange_Start; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
     iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->GetNextWindowLength(), giStartRange_Start);
     iMaxStartWindow = std::min(giStartRange_End + 1, iWindowEnd);
     for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
        pData->gtCases = pData->gpCases[iWindowStart] - pData->gpCases[iWindowEnd];
        pData->gtMeasure = pData->gpMeasure[iWindowStart] - pData->gpMeasure[iWindowEnd];
        pData->gtSqMeasure = pData->gpSqMeasure[iWindowStart] - pData->gpSqMeasure[iWindowEnd];
        if (fRateOfInterest(pData->gtCases, pData->gtMeasure, tTotalCases, tTotalMeasure)) {
          Running.m_nRatio = gLikelihoodCalculator.CalcLogLikelihoodRatioEx(pData->gtCases, pData->gtMeasure, pData->gtSqMeasure, tTotalCases, tTotalMeasure);
          if (Running.m_nRatio  > TopCluster.m_nRatio) {
            TopCluster.AssignAsType(Running);
            TopCluster.m_nFirstInterval = iWindowStart;
            TopCluster.m_nLastInterval = iWindowEnd;
          }
        }
     }
  }
}

/** Not implemented - throws ZdException */
void NormalTimeIntervalRange::CompareMeasures(TemporalData&, CMeasureList&) {
  ZdGenerateException("CompareMeasures() not implemented.","NormalTimeIntervalRange");
}



/** constructor */
MultiStreamTimeIntervalRange::MultiStreamTimeIntervalRange(const CSaTScanData& Data,
                                                           AbstractLikelihoodCalculator & Calculator,
                                                           IncludeClustersType eIncludeClustersType)
                             :TimeIntervalRange(Data, Calculator, eIncludeClustersType) {}

/** copy constructor */
MultiStreamTimeIntervalRange::MultiStreamTimeIntervalRange(const MultiStreamTimeIntervalRange & rhs)
                             :TimeIntervalRange(rhs) {}

/** overloaded assignment operator */
MultiStreamTimeIntervalRange & MultiStreamTimeIntervalRange::operator=(const MultiStreamTimeIntervalRange& rhs) {
  giStartRange_End = rhs.giStartRange_End;
  giEndRange_Start = rhs.giEndRange_Start;
  giEndRange_End = rhs.giEndRange_End;
  giMaxWindowLength = rhs.giMaxWindowLength;
  return *this;
}

/** returns newly cloned TimeIntervalRange object */
MultiStreamTimeIntervalRange * MultiStreamTimeIntervalRange::Clone() const {
 return new MultiStreamTimeIntervalRange(*this);
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
    NOTE: This algorithm is identical to TimeIntervalRange::CompareClusters(...)
          with the deviation being the loop over multipe data sets and process
          of unifying the calculated log likelihood ratios. */
void MultiStreamTimeIntervalRange::CompareClusters(CCluster & Running, CCluster & TopCluster) {
  int                          iWindowStart, iWindowEnd, iMaxStartWindow, iMaxEndWindow;
  MultipleStreamTemporalData * pData = (MultipleStreamTemporalData*)Running.GetClusterData(); //dynamic cast ?
  AbstractLoglikelihoodRatioUnifier & Unifier = gLikelihoodCalculator.GetUnifier();

  //iterate through windows
  gpMaxWindowLengthIndicator->Reset();
  iMaxEndWindow = std::min(giEndRange_End, giStartRange_End + giMaxWindowLength);
  for (iWindowEnd=giEndRange_Start; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
     iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->GetNextWindowLength(), giStartRange_Start);
     iMaxStartWindow = std::min(giStartRange_End + 1, iWindowEnd);
     for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
        Unifier.Reset();
        for (size_t t=0; t < pData->gvStreamData.size(); ++t) {
          TemporalData & Datum = *pData->gvStreamData[t];
          Datum.gtCases = Datum.gpCases[iWindowStart] - Datum.gpCases[iWindowEnd];
          Datum.gtMeasure = Datum.gpMeasure[iWindowStart] - Datum.gpMeasure[iWindowEnd];
          Unifier.AdjoinRatio(gLikelihoodCalculator, Datum.gtCases, Datum.gtMeasure,
                              Datum.gtTotalCases, Datum.gtTotalMeasure);
        }
        Running.m_nRatio = Unifier.GetLoglikelihoodRatio();
        if (Running.m_nRatio > TopCluster.m_nRatio) {
          TopCluster.AssignAsType(Running);
          TopCluster.m_nFirstInterval = iWindowStart;
          TopCluster.m_nLastInterval = iWindowEnd;
        }
     }
  }
}

/** Not implemented - throws ZdException */
void MultiStreamTimeIntervalRange::CompareMeasures(TemporalData&, CMeasureList&) {
  ZdGenerateException("CompareMeasures() not implemented.","MultiStreamTimeIntervalRange");
}


#include "SaTScan.h"
#pragma hdrstop
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

void TimeIntervalRange::CompareClusters(CCluster & Running, CCluster & TopCluster) {
  int            iWindowStart, iWindowEnd, iMaxStartWindow, iMaxEndWindow;
  count_t        tCases, tTotalCases(gData.GetTotalCases());
  measure_t      tMeasure, tTotalMeasure(gData.GetTotalMeasure());
  TemporalData * pData = (TemporalData*)Running.GetClusterData();  //dynamic cast?

  //iterate through windows
  gpMaxWindowLengthIndicator->Reset();
  iMaxEndWindow = std::min(giEndRange_End, giStartRange_End + giMaxWindowLength);
  for (iWindowEnd=giEndRange_Start; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
     iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->GetNextWindowLength(), giStartRange_Start);
     iMaxStartWindow = std::min(giStartRange_End + 1, iWindowEnd);
     for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
        pData->gtCases = pData->gpCases[iWindowStart] - pData->gpCases[iWindowEnd];
        pData->gtMeasure = pData->gpMeasure[iWindowStart] - pData->gpMeasure[iWindowEnd];
        if (fRateOfInterest(pData->gtCases, pData->gtMeasure, tTotalCases, tTotalMeasure)) {
          Running.m_nRatio = gLikelihoodCalculator.CalcLogLikelihoodRatio(pData->gtCases, pData->gtMeasure, tTotalCases, tTotalMeasure);
          if (Running.m_nRatio  > TopCluster.m_nRatio) {
            TopCluster.AssignAsType(Running);
            TopCluster.m_nFirstInterval = iWindowStart;
            TopCluster.m_nLastInterval = iWindowEnd;
          }
        }
     }
  }
}

void TimeIntervalRange::CompareMeasures(AbstractTemporalClusterData * pStreamData, CMeasureList * pMeasureList) {
  int                   iWindowStart, iWindowEnd, iMaxStartWindow, iMaxEndWindow;
  TemporalData        * pData = (TemporalData*)pStreamData;  //dynamic cast?
  

  //iterate through windows
  gpMaxWindowLengthIndicator->Reset();
  iMaxEndWindow = std::min(giEndRange_End, giStartRange_End + giMaxWindowLength);
  for (iWindowEnd=giEndRange_Start; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
     iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->GetNextWindowLength(), giStartRange_Start);
     iMaxStartWindow = std::min(giStartRange_End + 1, iWindowEnd);
     for (; iWindowStart < iMaxStartWindow; ++iWindowStart)
        pMeasureList->AddMeasure(pData->gpCases[iWindowStart] - pData->gpCases[iWindowEnd],
                                 pData->gpMeasure[iWindowStart] - pData->gpMeasure[iWindowEnd]);
  }
}

/** Returns the number of cases that tract attributed to accumulated case count. */
count_t TimeIntervalRange::GetCaseCountForTract(const CCluster & Cluster, tract_t tTract, count_t** pCases) const {
  count_t      tCaseCount;

  if (Cluster.m_nLastInterval == giEndRange_End)
    tCaseCount = pCases[Cluster.m_nFirstInterval][tTract];
  else
    tCaseCount  = pCases[Cluster.m_nFirstInterval][tTract] - pCases[Cluster.m_nLastInterval][tTract];

  return tCaseCount;
}

/** Returns the measure that tract attributed to accumulated measure. */
measure_t TimeIntervalRange::GetMeasureForTract(const CCluster & Cluster, tract_t tTract, measure_t** pMeasure) const {
  measure_t      tMeasure;

  if (Cluster.m_nLastInterval == giEndRange_End)
    tMeasure = pMeasure[Cluster.m_nFirstInterval][tTract];
  else
    tMeasure  = pMeasure[Cluster.m_nFirstInterval][tTract] - pMeasure[Cluster.m_nLastInterval][tTract];

  return tMeasure;
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

/** validates defined window */
void TimeIntervalRange::ValidateWindowRanges(const CSaTScanData& Data) {
/**  see CSaTScanData::SetTimeIntervalRangeIndexes() */
  
//  ZdString      sTimeIntervalType;
//  char          sDate[50], sDate2[50];
//  int           iMaxEndWindow, iWindowStart;
//
//  switch (Data.GetParameters().GetTimeIntervalUnitsType()) {
//    case YEAR  : sTimeIntervalType = "year"; break;
//    case MONTH : sTimeIntervalType = "month"; break;
//    case DAY   : sTimeIntervalType = "day"; break;
//    default: sTimeIntervalType = "none"; break;
//  };
//  if (giMaxWindowLength > 1) sTimeIntervalType += "s";
//
// //check that there will be clusters evaluated...
//  iMaxEndWindow = std::min(giEndRange_End, giStartRange_End + giMaxWindowLength);
//  iWindowStart = std::max(giEndRange_Start - giMaxWindowLength, giStartRange_Start);
//
//  if (iWindowStart >= iMaxEndWindow) {
//    ResolvableException("Error: No clusters will be evaluated.\n"
//        "       Although settings indicate a scanning window range of %s-%s to %s-%s,\n"
//        "       the incorporation of the maximum temporal cluster size of %i %s causes the maximum window end time\n"
//        "       to become %s (%s plus %i %s) and the window start time to become %s\n"
//        "       (%s minus %i %s) which results in an invalid scanning window.\n", "Setup()",
//        Data.GetParameters().GetStartRangeStartDate().c_str(),
//        Data.GetParameters().GetStartRangeEndDate().c_str(),
//       Data.GetParameters().GetEndRangeStartDate().c_str(),
//        Data.GetParameters().GetEndRangeEndDate().c_str(),
//        giMaxWindowLength, sTimeIntervalType.GetCString(),
//        JulianToChar(sDate, gData.GetTimeIntervalStartTimes()[iMaxEndWindow]),
//        Data.GetParameters().GetStartRangeEndDate().c_str(),
//        giMaxWindowLength, sTimeIntervalType.GetCString(),
//        JulianToChar(sDate2, gData.GetTimeIntervalStartTimes()[iWindowStart]),
//        Data.GetParameters().GetEndRangeStartDate().c_str(),
//       giMaxWindowLength, sTimeIntervalType.GetCString());
//  }
//  //The parameter validation checked already whether the end range dates conflicted,
//  //but the maxium temporal cluster size may actually cause the range dates to be
//  //different than the user defined.
//  if (giEndRange_Start > iMaxEndWindow) {
//    ResolvableException("Error: No clusters will be evaluated.\n"
//        "       Although settings indicate a scanning window range of %s-%s to %s-%s,\n"
//        "       the incorporation of the maximum temporal cluster size of %i %s causes the maximum window end time\n"
//        "       to become %s (%s plus %i %s), which does not intersect with requested scanning\n"
//        "       window end range.\n","Setup()",
//        Data.GetParameters().GetStartRangeStartDate().c_str(),
//        Data.GetParameters().GetStartRangeEndDate().c_str(),
//        Data.GetParameters().GetEndRangeStartDate().c_str(),
//        Data.GetParameters().GetEndRangeEndDate().c_str(),
//        giMaxWindowLength, sTimeIntervalType.GetCString(),
//        JulianToChar(sDate, gData.GetTimeIntervalStartTimes()[iMaxEndWindow]),
//        Data.GetParameters().GetStartRangeEndDate().c_str(),
//       giMaxWindowLength, sTimeIntervalType.GetCString());
//  }
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

void NormalTimeIntervalRange::CompareMeasures(AbstractTemporalClusterData * pStreamData, CMeasureList * pMeasureList) {
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

void MultiStreamTimeIntervalRange::CompareClusters(CCluster & Running, CCluster & TopCluster) {
  int                          iWindowStart, iWindowEnd, iMaxStartWindow, iMaxEndWindow;
  MultipleStreamTemporalData * pData = (MultipleStreamTemporalData*)Running.GetClusterData(); //dynamic cast ?

  //iterate through windows
  gpMaxWindowLengthIndicator->Reset();
  iMaxEndWindow = std::min(giEndRange_End, giStartRange_End + giMaxWindowLength);
  for (iWindowEnd=giEndRange_Start; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
     iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->GetNextWindowLength(), giStartRange_Start);
     iMaxStartWindow = std::min(giStartRange_End + 1, iWindowEnd);
     for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
        Running.m_nRatio = 0;
        for (size_t t=0; t < pData->gvStreamData.size(); ++t) {
          TemporalData & Datum = *pData->gvStreamData[t];
          Datum.gtCases = Datum.gpCases[iWindowStart] - Datum.gpCases[iWindowEnd];
          Datum.gtMeasure = Datum.gpMeasure[iWindowStart] - Datum.gpMeasure[iWindowEnd];
          gLikelihoodCalculator.GetUnifier().AdjoinRatio(gLikelihoodCalculator, t, Datum.gtCases, Datum.gtMeasure,
                                                         Datum.gtTotalCases, Datum.gtTotalMeasure);
        }
        Running.m_nRatio = gLikelihoodCalculator.GetUnifier().GetLoglikelihoodRatio();
        if (Running.m_nRatio && Running.m_nRatio > TopCluster.m_nRatio) {
          TopCluster.AssignAsType(Running);
          TopCluster.m_nFirstInterval = iWindowStart;
          TopCluster.m_nLastInterval = iWindowEnd;
        }
     }
  }
}

void MultiStreamTimeIntervalRange::CompareMeasures(AbstractTemporalClusterData * pStreamData, CMeasureList * pMeasureList) {
  ZdGenerateException("CompareMeasures() not implemented.","MultiStreamTimeIntervalRange");
}


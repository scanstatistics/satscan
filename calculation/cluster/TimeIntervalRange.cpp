#include "SaTScan.h"
#pragma hdrstop
#include "TimeIntervalRange.h"
#include "SaTScanData.h"
#include "MeasureList.h"
#include "Cluster.h"

/** constructor */
TimeIntervalRange::TimeIntervalRange(const CSaTScanData& Data)
                  :CTimeIntervals(Data.m_nTimeIntervals, Data.m_nIntervalCut, Data.GetParameters().GetAreaScanRateType()),
                   gData(Data) {
  Setup(Data);
}	

/** copy constructor */
TimeIntervalRange::TimeIntervalRange(const TimeIntervalRange & rhs) : CTimeIntervals(rhs), gData(rhs.gData) {
  *this = rhs;
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

/** Iterates through all possible time windows for runnning cluster, comparing
    against current top cluster. Reassigns top cluster if running cluster ever
    has a greater loglikelihood.*/
void TimeIntervalRange::CompareClusters(CCluster & Running, CCluster & TopShapeCluster, StreamDataContainer_t & StreamData) {

  int           iWindowStart, iWindowEnd, iMaxStartWindow, iMaxEndWindow;
  CModel      & Model(gData.GetProbabilityModel());
  count_t       tCases, tTotalCases(gData.GetTotalCases());
  measure_t     tMeasure, tTotalMeasure(gData.GetTotalMeasure());
  AbstractTemporalClusterStreamData * pStreamData = StreamData[0];

  //iterate through windows
  iMaxEndWindow = std::min(giEndRange_End, giStartRange_End + giMaxWindowLength);
  for (iWindowEnd=giEndRange_Start; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
     iWindowStart = std::max(iWindowEnd - giMaxWindowLength, giStartRange_Start);
     iMaxStartWindow = std::min(giStartRange_End + 1, iWindowEnd);
     for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
        tCases = pStreamData->gpCases[iWindowStart] - pStreamData->gpCases[iWindowEnd];
        tMeasure = pStreamData->gpMeasure[iWindowStart] - pStreamData->gpMeasure[iWindowEnd];
        if (fRateOfInterest(tCases, tMeasure, tTotalCases, tTotalMeasure)) {
          Running.m_nRatio = Model.CalcLogLikelihoodRatio(tCases, tMeasure, tTotalCases, tTotalMeasure, Running.m_DuczmalCorrection);
          if (Running.m_nRatio  > TopShapeCluster.m_nRatio) {
            TopShapeCluster.AssignAsType(Running);
            TopShapeCluster.m_nFirstInterval = iWindowStart;
            TopShapeCluster.m_nLastInterval = iWindowEnd;
            TopShapeCluster.SetCaseCount(0, tCases);
            TopShapeCluster.SetMeasure(0, tMeasure);
          }
        }
     }
  }
}

/** Iterates through all possible time windows for runnning cluster, comparing
    against current top cluster. Reassigns top cluster if running cluster ever
    has a greater loglikelihood.*/
void TimeIntervalRange::CompareClustersEx(CCluster & Running, CCluster & TopShapeCluster, StreamDataContainer_t & StreamData) {

  int           iWindowStart, iWindowEnd, iMaxStartWindow, iMaxEndWindow;
  CModel      & Model(gData.GetProbabilityModel());
  count_t       tCases, tTotalCases(gData.GetTotalCases());
  measure_t     tMeasure, tMeasure2, tTotalMeasure(gData.GetTotalMeasure());
  AbstractTemporalClusterStreamData * pStreamData = StreamData[0];

  //iterate through windows
  iMaxEndWindow = std::min(giEndRange_End, giStartRange_End + giMaxWindowLength);
  for (iWindowEnd=giEndRange_Start; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
     iWindowStart = std::max(iWindowEnd - giMaxWindowLength, giStartRange_Start);
     iMaxStartWindow = std::min(giStartRange_End + 1, iWindowEnd);
     for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
        tCases = pStreamData->gpCases[iWindowStart] - pStreamData->gpCases[iWindowEnd];
        tMeasure = pStreamData->gpMeasure[iWindowStart] - pStreamData->gpMeasure[iWindowEnd];
        tMeasure2 = pStreamData->gpSqMeasure[iWindowStart] - pStreamData->gpSqMeasure[iWindowEnd];
        if (fRateOfInterest(tCases, tMeasure, tTotalCases, tTotalMeasure)) {
          //calculate with second measure
          Running.m_nRatio = Model.CalcLogLikelihoodRatioEx(tCases, tMeasure, tTotalCases, tTotalMeasure, tMeasure2, Running.m_DuczmalCorrection);
          if (Running.m_nRatio  > TopShapeCluster.m_nRatio) {
            TopShapeCluster.AssignAsType(Running);
            TopShapeCluster.m_nFirstInterval = iWindowStart;
            TopShapeCluster.m_nLastInterval = iWindowEnd;
            TopShapeCluster.SetCaseCount(0, tCases);
            TopShapeCluster.SetMeasure(0, tMeasure);
            //set second measure ?
          }
        }
     }
  }
}

void TimeIntervalRange::CompareDataStreamClusters(CCluster & Running, CCluster & TopShapeCluster, StreamDataContainer_t & StreamData) {

  int                                   iWindowStart, iWindowEnd, iMaxStartWindow, iMaxEndWindow;
  CModel                              & ProbabilityModel(gData.GetProbabilityModel());
  AbstractTemporalClusterStreamData   * pStreamData;
  unsigned int                          iStreamSize = StreamData.size();

  //iterate through windows
  iMaxEndWindow = std::min(giEndRange_End, giStartRange_End + giMaxWindowLength);
  for (iWindowEnd=giEndRange_Start; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
     iWindowStart = std::max(iWindowEnd - giMaxWindowLength, giStartRange_Start);
     iMaxStartWindow = std::min(giStartRange_End + 1, iWindowEnd);
     for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
        Running.m_nRatio = 0;
        for (size_t t=0; t < iStreamSize; ++t) {                           
          pStreamData = StreamData[t];
          pStreamData->gCases = pStreamData->gpCases[iWindowStart] - pStreamData->gpCases[iWindowEnd];
          pStreamData->gMeasure = pStreamData->gpMeasure[iWindowStart] - pStreamData->gpMeasure[iWindowEnd];
          if (fRateOfInterest(pStreamData->gCases, pStreamData->gMeasure, pStreamData->gTotalCases, pStreamData->gTotalMeasure))
            Running.m_nRatio += ProbabilityModel.CalcLogLikelihoodRatio(pStreamData->gCases, pStreamData->gMeasure, pStreamData->gTotalCases, pStreamData->gTotalMeasure, Running.m_DuczmalCorrection);
        }
        if (Running.m_nRatio && Running.m_nRatio > TopShapeCluster.m_nRatio) {
          TopShapeCluster.AssignAsType(Running);
          TopShapeCluster.m_nFirstInterval = iWindowStart;
          TopShapeCluster.m_nLastInterval = iWindowEnd;
        }
     }
  }
}

void TimeIntervalRange::CompareDataStreamClustersEx(CCluster & Running, CCluster & TopShapeCluster, StreamDataContainer_t & StreamData) {

  int                                   iWindowStart, iWindowEnd, iMaxStartWindow, iMaxEndWindow;
  CModel                              & ProbabilityModel(gData.GetProbabilityModel());
  AbstractTemporalClusterStreamData   * pStreamData;
  unsigned int                          iStreamSize = StreamData.size();

  //iterate through windows
  iMaxEndWindow = std::min(giEndRange_End, giStartRange_End + giMaxWindowLength);
  for (iWindowEnd=giEndRange_Start; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
     iWindowStart = std::max(iWindowEnd - giMaxWindowLength, giStartRange_Start);
     iMaxStartWindow = std::min(giStartRange_End + 1, iWindowEnd);
     for (; iWindowStart < iMaxStartWindow; ++iWindowStart) {
        Running.m_nRatio = 0;
        for (size_t t=0; t < iStreamSize; ++t) {
          pStreamData = StreamData[t];
          pStreamData->gCases = pStreamData->gpCases[iWindowStart] - pStreamData->gpCases[iWindowEnd];
          pStreamData->gMeasure = pStreamData->gpMeasure[iWindowStart] - pStreamData->gpMeasure[iWindowEnd];
          pStreamData->gSqMeasure = pStreamData->gpSqMeasure[iWindowStart] - pStreamData->gpSqMeasure[iWindowEnd];
          if (fRateOfInterest(pStreamData->gCases, pStreamData->gMeasure, pStreamData->gTotalCases, pStreamData->gTotalMeasure))
            //calculate with second measure
            Running.m_nRatio += ProbabilityModel.CalcLogLikelihoodRatioEx(pStreamData->gCases, pStreamData->gMeasure, pStreamData->gSqMeasure, pStreamData->gTotalCases, pStreamData->gTotalMeasure, Running.m_DuczmalCorrection);
        }
        if (Running.m_nRatio && Running.m_nRatio > TopShapeCluster.m_nRatio) {
          TopShapeCluster.AssignAsType(Running);
          TopShapeCluster.m_nFirstInterval = iWindowStart;
          TopShapeCluster.m_nLastInterval = iWindowEnd;
        }
     }
  }
}

void TimeIntervalRange::ComputeBestMeasures(AbstractTemporalClusterStreamData * pStreamData, CMeasureList & MeasureList) {

  int                                   iWindowStart, iWindowEnd, iMaxStartWindow, iMaxEndWindow;
  AbstractMaxWindowLengthIndicator    * pMWL_Indicator = gData.gpMaxWindowLengthIndicator;

  //iterate through windows
  pMWL_Indicator->Reset();
  iMaxEndWindow = std::min(giEndRange_End, giStartRange_End + giMaxWindowLength);
  for (iWindowEnd=giEndRange_Start; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
     iWindowStart = std::max(iWindowEnd - pMWL_Indicator->GetNextWindowLength(), giStartRange_Start);
     iMaxStartWindow = std::min(giStartRange_End + 1, iWindowEnd);
     for (; iWindowStart < iMaxStartWindow; ++iWindowStart)
        MeasureList.AddMeasure(pStreamData->gpCases[iWindowStart] - pStreamData->gpCases[iWindowEnd],
                               pStreamData->gpMeasure[iWindowStart] - pStreamData->gpMeasure[iWindowEnd]);
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
void TimeIntervalRange::Setup(const CSaTScanData& Data) {
  switch(Data.GetParameters().GetAnalysisType()) {
    case PROSPECTIVEPURELYTEMPORAL :
    case PROSPECTIVESPACETIME      : giStartRange_Start = 0;
                                     giStartRange_End = Data.m_nTimeIntervals;
                                     giEndRange_Start = Data.m_nProspectiveIntervalStart;
                                     giEndRange_End = Data.m_nTimeIntervals; break;
    case PURELYTEMPORAL :
    case SPACETIME      : switch (Data.GetParameters().GetIncludeClustersType()) {
                            case ALLCLUSTERS     : giStartRange_Start = 0;
                                                   giStartRange_End = Data.m_nTimeIntervals;
                                                   giEndRange_Start = 1;
                                                   giEndRange_End = Data.m_nTimeIntervals; break;
                            case ALIVECLUSTERS   : //you can create 'windows' for alive clusters with this class,
                                                   //but it will be slower that using CTIAlive class.
                                                   giStartRange_Start = 0;
                                                   giStartRange_End = Data.m_nTimeIntervals;
                                                   giEndRange_Start = Data.m_nTimeIntervals;
                                                   giEndRange_End = Data.m_nTimeIntervals; break;
                            case CLUSTERSINRANGE : giStartRange_Start = Data.m_nStartRangeStartDateIndex;
                                                   giStartRange_End = Data.m_nStartRangeEndDateIndex;
                                                   giEndRange_Start = Data.m_nEndRangeStartDateIndex;
                                                   giEndRange_End = Data.m_nEndRangeEndDateIndex; break;
                            default : ZdGenerateException("Unknown cluster inclusion type: '%d'.",
                                                          "Setup()", Data.GetParameters().GetIncludeClustersType());
                          };
                          break;
    default : ZdGenerateException("Unknown analysis type for range intervals: '%d';",
                                  "Setup()", Data.GetParameters().GetAnalysisType());
  };
}

/** validates that the defined window */
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
//    SSGenerateException("Error: No clusters will be evaluated.\n"
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
//    SSGenerateException("Error: No clusters will be evaluated.\n"
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



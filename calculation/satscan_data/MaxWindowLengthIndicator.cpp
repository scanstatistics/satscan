//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "MaxWindowLengthIndicator.h"
#include "SaTScanData.h"

/** constructor */
FixedMaxWindowLengthIndicator::FixedMaxWindowLengthIndicator(const CSaTScanData & Data)
                             :AbstractMaxWindowLengthIndicator(), giMaxWindowLength(Data.m_nIntervalCut) {}

/** destructor */
FixedMaxWindowLengthIndicator::~FixedMaxWindowLengthIndicator() {}


/** constructor */
ProspectiveMaxWindowLengthIndicator::ProspectiveMaxWindowLengthIndicator(const CSaTScanData & Data)
                                   :AbstractMaxWindowLengthIndicator() {
  int iMaxEndWindow, iWindowEnd, iIntervalCut, lTimeBetween;


  if (Data.GetParameters().GetMaximumTemporalClusterSizeType() == TIMETYPE)
    ZdGenerateException("For a maximum special cluster size defined as a fixed distance\n"
                        "you should use FixedMaxWindowLengthIndicator.","constructor()");

  gvMaxWindowLengths.push_back(0); //dummy entry

  //prospective analyses always use time range class, provided we are doing replications
  if (Data.GetParameters().GetIsProspectiveAnalysis() && Data.GetParameters().GetNumReplicationsRequested() > 0) {
    iMaxEndWindow = Data.m_nTimeIntervals;
    for (iWindowEnd=Data.m_nProspectiveIntervalStart; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
       lTimeBetween = static_cast<int>(floor(TimeBetween(Data.GetStudyPeriodStartDate(),
                                             Data.GetTimeIntervalStartTimes()[iWindowEnd] - 1,
                                             Data.GetParameters().GetTimeIntervalUnitsType())));
       lTimeBetween = static_cast<int>(lTimeBetween * (Data.GetParameters().GetMaximumTemporalClusterSize()/100.0));
       iIntervalCut = static_cast<int>(floor(lTimeBetween / Data.GetParameters().GetTimeIntervalLength()));
       gvMaxWindowLengths.push_back(iIntervalCut);
    }
  }
  gitr = gvMaxWindowLengths.begin();
}

/** destructor */
ProspectiveMaxWindowLengthIndicator::~ProspectiveMaxWindowLengthIndicator() {}

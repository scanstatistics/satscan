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
  int                   iMaxEndWindow, iWindowEnd, iIntervalCut;
  double                dProspectivePeriodLength, dMaxTemporalLengthInUnits;
  const CParameters   & Parameters(Data.GetParameters());


  if (Data.GetParameters().GetMaximumTemporalClusterSizeType() == TIMETYPE)
    ZdGenerateException("For a maximum special cluster size defined as a fixed distance\n"
                        "you should use FixedMaxWindowLengthIndicator.","constructor()");

  gvMaxWindowLengths.push_back(0); //dummy entry

  //prospective analyses always use time range class, provided we are doing replications
  if (Parameters.GetIsProspectiveAnalysis() && Parameters.GetNumReplicationsRequested() > 0) {
    iMaxEndWindow = Data.m_nTimeIntervals;
    for (iWindowEnd=Data.m_nProspectiveIntervalStart; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
       dProspectivePeriodLength = CalculateNumberOfTimeIntervals(Data.GetStudyPeriodStartDate(),
                                                                 Data.GetTimeIntervalStartTimes()[iWindowEnd] - 1,
                                                                 Parameters.GetTimeAggregationUnitsType(), 1);
                                                     
       dMaxTemporalLengthInUnits = floor(dProspectivePeriodLength * Parameters.GetMaximumTemporalClusterSize()/100.0);
       //now calculate number of those time units a cluster can contain with respects to the specified aggregation length
       iIntervalCut = static_cast<int>(floor(dMaxTemporalLengthInUnits / Parameters.GetTimeAggregationLength()));
       gvMaxWindowLengths.push_back(iIntervalCut);
    }
  }
  gitr = gvMaxWindowLengths.begin();
}

/** destructor */
ProspectiveMaxWindowLengthIndicator::~ProspectiveMaxWindowLengthIndicator() {}

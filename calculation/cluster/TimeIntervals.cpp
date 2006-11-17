//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "TimeIntervals.h"
#include "SSException.h"

CTimeIntervals::CTimeIntervals(const CSaTScanData& DataHub, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType)
               :gDataHub(DataHub), gLikelihoodCalculator(Calculator), gpMaxWindowLengthIndicator(0) {
   Setup(eIncludeClustersType);
}

CTimeIntervals::~CTimeIntervals() {
  try {
    delete gpMaxWindowLengthIndicator;
  }
  catch (...){}
}

/** internal setup function */
void CTimeIntervals::Setup(IncludeClustersType eIncludeClustersType) {
  try {
    giNumIntervals = gDataHub.GetNumTimeIntervals();
    giMaxWindowLength = gDataHub.GetTimeIntervalCut();
    switch(gDataHub.GetParameters().GetExecuteScanRateType()) {
      case LOW        : fRateOfInterest = LowRate;       break;
      case HIGHANDLOW : fRateOfInterest = HighOrLowRate; break;
      default         : fRateOfInterest = HighRate;
    };
    if (gDataHub.GetParameters().GetIsProspectiveAnalysis() && eIncludeClustersType == ALLCLUSTERS) {
      // For a prospective analysis with IncludeClustersType of ALLCLUSTERS, this situation indicates
      // that this object is being constructed for use in simulations. Perhaps there should be another
      // enumeration item such as ALIVEPROSPECTIVECLUSTERS to remove confusion?  
      giStartRange_Start = 0;
      giStartRange_End = gDataHub.GetNumTimeIntervals();
      giEndRange_Start = gDataHub.GetProspectiveStartIndex();
      giEndRange_End = gDataHub.GetNumTimeIntervals();
      //the maximum window length varies when the analysis is prospective and
      //the maximum is defined as percentage of study period
      if (gDataHub.GetParameters().GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE && gDataHub.GetParameters().GetAdjustForEarlierAnalyses())
        gpMaxWindowLengthIndicator = new ProspectiveMaxWindowLengthIndicator(gDataHub);
      else
        gpMaxWindowLengthIndicator = new FixedMaxWindowLengthIndicator(gDataHub);
    }
    else {
      switch (eIncludeClustersType) {
        case ALLCLUSTERS     : giStartRange_Start = 0;
                               giStartRange_End = gDataHub.GetNumTimeIntervals();
                               giEndRange_Start = 1;
                               giEndRange_End = gDataHub.GetNumTimeIntervals(); break;
        case ALIVECLUSTERS   : giStartRange_Start = 0;
                               giStartRange_End = gDataHub.GetNumTimeIntervals();
                               giEndRange_Start = gDataHub.GetNumTimeIntervals();
                               giEndRange_End = gDataHub.GetNumTimeIntervals(); break;
        case CLUSTERSINRANGE : giStartRange_Start = gDataHub.GetFlexibleWindowStartRangeStartIndex();
                               giStartRange_End = gDataHub.GetFlexibleWindowStartRangeEndIndex();
                               giEndRange_Start = gDataHub.GetFlexibleWindowEndRangeStartIndex();
                               giEndRange_End = gDataHub.GetFlexibleWindowEndRangeEndIndex(); break;
        default :
          throw prg_error("Unknown cluster inclusion type: '%d'.", "Setup()", gDataHub.GetParameters().GetIncludeClustersType());
      };
      gpMaxWindowLengthIndicator = new FixedMaxWindowLengthIndicator(gDataHub);
    }
  }
  catch (prg_exception& x) {
    delete gpMaxWindowLengthIndicator; gpMaxWindowLengthIndicator=0;
    x.addTrace("setup()","CTimeIntervals");
    throw;
  }
}


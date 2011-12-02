//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "TimeIntervals.h"
#include "SSException.h"

CTimeIntervals::CTimeIntervals(const CSaTScanData& DataHub, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType)
                :gDataHub(DataHub), _gInfo(*DataHub.GetGInfo()), gLikelihoodCalculator(Calculator) {
   Setup(eIncludeClustersType);
}

/** Sets interval range to that of specified centroid point. If point does not have focus range, 'global' range used. */
void  CTimeIntervals::setIntervalRange(tract_t centerpoint) {
    if (!_gInfo.hasFocusIntervals()) return;

    GInfo::FocusInterval_t focus = _gInfo.retrieveFocusInterval(centerpoint);
    _interval_range = (focus.first ? focus.second : _init_interval_range);
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
            _interval_range = IntervalRange_t(0, gDataHub.GetNumTimeIntervals(), gDataHub.GetProspectiveStartIndex(), gDataHub.GetNumTimeIntervals());
            //the maximum window length varies when the analysis is prospective and
            //the maximum is defined as percentage of study period
            if (gDataHub.GetParameters().GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE && gDataHub.GetParameters().GetAdjustForEarlierAnalyses())
                gpMaxWindowLengthIndicator.reset(new ProspectiveMaxWindowLengthIndicator(gDataHub));
            else
                gpMaxWindowLengthIndicator.reset(new FixedMaxWindowLengthIndicator(gDataHub));
        } else {
            switch (eIncludeClustersType) {
                case ALLCLUSTERS        : _interval_range = IntervalRange_t(0, gDataHub.GetNumTimeIntervals(), 1, gDataHub.GetNumTimeIntervals()); break;
                case ALIVECLUSTERS      : _interval_range = IntervalRange_t(0, gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTimeIntervals(), gDataHub.GetNumTimeIntervals()); break;
                case CLUSTERSINRANGE    : _interval_range = IntervalRange_t(gDataHub.GetFlexibleWindowStartRangeStartIndex(), 
                                                                            gDataHub.GetFlexibleWindowStartRangeEndIndex(), 
                                                                            gDataHub.GetFlexibleWindowEndRangeStartIndex(), 
                                                                            gDataHub.GetFlexibleWindowEndRangeEndIndex()); break;  
                default : throw prg_error("Unknown cluster inclusion type: '%d'.", "Setup()", gDataHub.GetParameters().GetIncludeClustersType());
            };
            gpMaxWindowLengthIndicator.reset(new FixedMaxWindowLengthIndicator(gDataHub));
        }
        _init_interval_range = _interval_range; // store initial range settings
    } catch (prg_exception& x) {
        x.addTrace("setup()","CTimeIntervals");
        throw;
    }
}


//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "ClosedLoopData.h"
#include "PoissonModel.h"
#include "BernoulliModel.h"
#include "SpaceTimePermutationModel.h"
#include "NormalModel.h"
#include "ExponentialModel.h"
#include "RankModel.h"
#include "OrdinalModel.h"
#include "SSException.h"

/** class constructor */
ClosedLoopData::ClosedLoopData(const CParameters& Parameters, BasePrint& PrintDirection) : CPurelyTemporalData(Parameters, PrintDirection) {}

/* Converts julian date to corresponding seasonal / closed loop date. */
Julian ClosedLoopData::convertToSeasonalDate(Julian jdate) const {
    long aggr = gParameters.GetTimeAggregationLength();
    switch (gParameters.GetTimeAggregationUnitsType()) {
        case ::YEAR : return jdate; // seasonal / closed loop analysis is not implemented for time precision of year
        case ::MONTH : {
            UInt jyear, jmonth, jday;
            JulianToMDY(&jmonth, &jday, &jyear, jdate);
            // normalize dates to have same year
            if (gParameters.GetTimeAggregationUnitsType() != ::MONTH)
                throw prg_error("Uncertain how to convert aggregation units for type %d.", "convertToSeasonalDate()", gParameters.GetTimeAggregationUnitsType());
            // taking into account aggregation, determine which date in seasonal year this date will be assigned to
            if (aggr == 1 || aggr == 2) // special case
                return MDYToJulian(jmonth, SEASONAL_DAY, SEASONAL_YEAR);
            else {
                // find position in corresponding year and aggregation slices -- starting from end of year
                for (UInt m=12; ; m-=aggr) {
                    if ((m - aggr + 1) <= jmonth && jmonth <= m) {
                        // found correct month range for date -- find the middle month for range, rounding down
                        MDYToJulian(m - (aggr/2), SEASONAL_DAY, SEASONAL_YEAR);
                    }
                }
            } throw prg_error("Unable to place julian date %u.", "convertToSeasonalDate()", jdate);
        }
        case ::DAY : {
            UInt jyear, jmonth, jday;
            JulianToMDY(&jmonth, &jday, &jyear, jdate);
            if (gParameters.GetTimeAggregationUnitsType() != ::DAY)
                throw prg_error("Uncertain how to convert aggregation units for type %d.", "convertToSeasonalDate()", gParameters.GetTimeAggregationUnitsType());
            if (jmonth == 2/*February*/ && jday == 29/*leap year*/) jday = 28; // we're merging leap year day with February 28
            // taking into account aggregation, determine which date in seasonal year this date will be assigned to
            if (aggr == 1 || aggr == 2) // special case
                return MDYToJulian(jmonth, jday, SEASONAL_YEAR);
            else {
                // find position in corresponding year and aggregation slices -- starting from end of year
                Julian jstart = MDYToJulian(1/*January*/, 1, SEASONAL_YEAR), jend = MDYToJulian(12/*December*/, 31, SEASONAL_YEAR);
                // set the date to the seasonal year
                jdate = MDYToJulian(jmonth, jday, SEASONAL_YEAR);
                for (Julian beginR=jend-aggr+1, endR=jend; ; beginR=std::max(beginR-aggr,jstart), endR-=aggr) {
                    if (beginR <= jdate && jdate <= endR) {
                        // found correct range for date -- find the middle day for range, rounding down
                        return (beginR + (endR - beginR)/2);
                    }
                }
            } throw prg_error("Unable to place julian date %u.", "convertToSeasonalDate()", jdate);
        }
        case ::GENERIC :
            if (gParameters.GetTimeAggregationUnitsType() != ::GENERIC)
                throw prg_error("Uncertain how to convert aggregation units for type %d.", "convertToSeasonalDate()", gParameters.GetTimeAggregationUnitsType());
            // taking into account aggregation, determine which date in connected loop this date will be assigned to
            if (aggr == 1 || aggr == 2) // special case
                return jdate;
            else {
                // find position in corresponding year and aggregation slices -- starting from end of year
                Julian jstart = m_nStartDate, jend = m_nEndDate;
                for (Julian beginR=jend-aggr+1, endR=jend; ; beginR=std::max(beginR-aggr,jstart), endR-=aggr) {
                    if (beginR <= jdate && jdate <= endR) {
                        // found correct range for date -- find the middle day for range, rounding down
                        return (beginR + (endR - beginR)/2);
                    }
                }
            } throw prg_error("Unable to place julian date %u.", "convertToSeasonalDate()", jdate);
        default : throw prg_error("Unknown time precision type %d.", "convertToSeasonalDate()", gParameters.GetPrecisionOfTimesType());
    }
}

/** Input: Date.                                                    **/
/** Returns: Index of the time interval to which the date belongs.   **/
/** If Date does not belong to any time interval, -1 is returned. **/
/** Note: First time interval has index 0.                          **/
int ClosedLoopData::GetTimeIntervalOfDate(Julian Date) const {
  int   i=0;

  //check that date is within study period
  if (Date < _closedloop_time_interval_starttimes[0] || Date >= _closedloop_time_interval_starttimes[_num_time_intervals])
    return -1;

  while (Date >=  _closedloop_time_interval_starttimes[i+1])
       ++i;

  return i;
}

/** Gets time interval index into interval start times array for end date. */
int ClosedLoopData::GetTimeIntervalOfEndDate(Julian EndDate) const {
  int   i, iDateIndex = -1;

  //find index for end date, the interval beyond where date fits
  for (i=_num_time_intervals; i > 0  && iDateIndex == -1; --i)
     if (EndDate <= _closedloop_time_interval_starttimes[i] - 1 && EndDate > _closedloop_time_interval_starttimes[i - 1] - 1)
        iDateIndex = i;
        
  return iDateIndex;
}

std::pair<Julian, Julian> ClosedLoopData::getStrictlyPeriod() const {
    std::pair<Julian, Julian> period;
    switch (gParameters.GetTimeAggregationUnitsType()) {
        case ::YEAR : throw prg_error("SetIntervalStartTimes() not implemented for year precision with closed loop analysis.", "SetIntervalStartTimes()");
        case ::MONTH : {
            // seasonal by months sets the period over the course of a one year period -- month by month
            period.first = MDYToJulian(1, 1, SEASONAL_YEAR);
            period.second = MDYToJulian(12, 31, SEASONAL_YEAR); 
            break;
        }
        case ::DAY : {
            // seasonal by days sets the period over the course of a one year period -- day by day
            period.first = MDYToJulian(1, 1, SEASONAL_YEAR);
            period.second = MDYToJulian(12, 31, SEASONAL_YEAR); 
            break;
        }
        case ::GENERIC : {
            // closed loop by generic units sets the period over the course of defined generic range -- unit by unit
            period.first = m_nStartDate;
            period.second = m_nEndDate;
            break;
        }
        default : throw prg_error("Unknown time aggregation type %d.", "SetIntervalStartTimes()", gParameters.GetTimeAggregationUnitsType());
    }
    return period;
}

/* Calculates the number of time aggregation units to include in potential clusters without exceeding the maximum temporal cluster size.*/
void ClosedLoopData::SetIntervalCut() {
	try {
		/* This is a little different for closed loop analysis since we're not creating the time interval start times in the standard manner.
		   The start times won't involve the time aggregation length -- instead the dates will be every day in 365 year or every month. For
		   generic dates, this will be every unit from start to end.*/
		if (gParameters.GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE) {
			std::pair<Julian, Julian> period = getStrictlyPeriod();
			double dMaxTemporalLengthInUnits = floor(CalculateNumberOfTimeIntervals(period.first, period.second, gParameters.GetTimeAggregationUnitsType(), 1) * gParameters.GetMaximumTemporalClusterSize() / 100.0);
			m_nIntervalCut = static_cast<int>(std::floor(dMaxTemporalLengthInUnits));
		}
		else {
			m_nIntervalCut = static_cast<int>(gParameters.GetMaximumTemporalClusterSize());
		}
        if (m_nIntervalCut==0)
            throw prg_error("The calculated number of time aggregations units in potential clusters is zero.","SetIntervalCut()");
        // Calculate the minimum interval cut -- which is the maximum between the specified minimum and the time aggregation length.
        _min_iterval_cut = static_cast<int>(std::max( static_cast<int>(gParameters.getMinimumTemporalClusterSize()), static_cast<int>(gParameters.GetTimeAggregationLength()) ));
    } catch (prg_exception& x) {
        x.addTrace("SetIntervalCut()","CSaTScanData");
        throw;
    }
}

/** Calculates the time interval start times given study period and time interval
    length. Start times are calculated from the study period end date backwards,
    which means that first time interval could possibly not be the requested time
    interval length. */
void ClosedLoopData::SetIntervalStartTimes() {
    // We still need the non-closed loop intervals for Poisson since it requires non-seasonal structure until
    // after measure calculation and possibly for power estimations.
    if (gParameters.GetProbabilityModelType() == POISSON)
        CPurelyTemporalData::SetIntervalStartTimes();

    // retrieve the period of interest
    std::pair<Julian, Julian> period = getStrictlyPeriod();

    // First calculate the time intervals over standard period
    DecrementableEndDate decrement(period.second, gParameters.GetTimeAggregationUnitsType());
    Julian intervalDate = period.second + 1; //latest interval start time is the day after study period end date

    _closedloop_time_interval_starttimes.clear();
    _closedloop_time_interval_starttimes.push_back(intervalDate);
    intervalDate = decrement.Decrement(1);
    while (intervalDate > period.first) {
        //push interval start time onto vector
        _closedloop_time_interval_starttimes.push_back(intervalDate);
        //find the next prior interval start time from current
        intervalDate = decrement.Decrement(1);
    }
    //push study period start date onto vector
    _closedloop_time_interval_starttimes.push_back(period.first);
    //reverse elements of vector so that elements are ordered: study period start --> 'study period end + 1'
    std::reverse(_closedloop_time_interval_starttimes.begin(), _closedloop_time_interval_starttimes.end());
    //record number of time intervals, not including 'study period end date + 1' date
    _num_time_intervals = (int)_closedloop_time_interval_starttimes.size() - 1;

    if (_num_time_intervals <= 1)
        //This error should be catch in the CParameters validation process.
        throw prg_error("The number of time intervals was calculated as one. Analyses can not be performed on less than two time intervals.\n", "SetIntervalStartTimes()");


    // Now calculate the time intervals including the extended period.
    Julian initial_enddate = period.second;
    // calculate the maximum temporal cluster size for that period
    UInt units;
    if (gParameters.GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE)
        units = static_cast<UInt>(floor(CalculateNumberOfTimeIntervals(period.first, period.second, gParameters.GetTimeAggregationUnitsType(), 1) * gParameters.GetMaximumTemporalClusterSize()/100.0));
    else units = static_cast<UInt>(gParameters.GetMaximumTemporalClusterSize());
    // tack on additional months, days or generic units to end of evaluation period
    switch (gParameters.GetTimeAggregationUnitsType()) {
        case ::YEAR : throw prg_error("SetIntervalStartTimes() not implemented for year precision with closed loop analysis.", "SetIntervalStartTimes()");
        case ::MONTH :
            // now tack on an additional number of months equal to the maximum temporal cluster size
            period.second = MDYToJulian(units, DaysThisMonth(SEASONAL_YEAR + 1, units), SEASONAL_YEAR + 1); break;
        case ::DAY :
            // now tack on an additional number of days equal to the maximum temporal cluster size
            period.second += units - 1; break;
        case ::GENERIC :
            // now tack on an additional numnber of generic units equal to the maximum temporal cluster size
            period.second += units - 1; break;
        default : throw prg_error("Unknown time aggregation type %d.", "SetIntervalStartTimes()", gParameters.GetTimeAggregationUnitsType());
    }

    DecrementableEndDate extended_decrement(period.second, gParameters.GetTimeAggregationUnitsType());
    intervalDate = period.second + 1; //latest interval start time is the day after study period end date

    _extended_time_interval_starttimes.clear();
    _extended_time_interval_starttimes.push_back(intervalDate);
    intervalDate = extended_decrement.Decrement(1);
    while (intervalDate > period.first) {
        //push interval start time onto vector
        _extended_time_interval_starttimes.push_back(intervalDate);
        //find the next prior interval start time from current
        intervalDate = extended_decrement.Decrement(1);
    }
    //push study period start date onto vector
    _extended_time_interval_starttimes.push_back(period.first);
    //reverse elements of vector so that elements are ordered: study period start --> 'study period end + 1'
    std::reverse(_extended_time_interval_starttimes.begin(), _extended_time_interval_starttimes.end());
    //record number of time intervals, not including 'study period end date + 1' date
    _num_extended_time_intervals = (int)_extended_time_interval_starttimes.size() - 1;

    if (_num_extended_time_intervals <= 1)
        //This error should be catch in the CParameters validation process.
        throw prg_error("The number of time intervals was calculated as one. Analyses can not be performed on less than two time intervals.\n", "SetIntervalStartTimes()");

    // set the interval index of the extended period
    _extended_period_start =  getTimeIntervalOfExtendedDate(initial_enddate) + 1;
}

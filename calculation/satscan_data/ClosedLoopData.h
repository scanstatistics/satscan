//*********************************************************************************
#ifndef __CLOSEDLOOPDATA_H
#define __CLOSEDLOOPDATA_H
//*********************************************************************************
#include "PurelyTemporalData.h"

/** Data hub class that derives from base class to define alternate functionality
    for closed loop / seasonal analyses.
    The code to generate data structures for 'number of cases', 'number of
    expected cases', 'simulated cases', etc. is designed to modify a multiple
    dimension array (time intervals by tracts) in a cumulative manner(in respect
    to time). The primary purpose of this class is to direct dataset to set
    corresponding temporal structures from data of multiple dimension array. */
class ClosedLoopData : public CPurelyTemporalData {
    protected:
        int  _num_time_intervals;
        std::vector<Julian> _closedloop_time_interval_starttimes;
        std::vector<Julian> _extended_time_interval_starttimes; /* time interval start times for extended period */
        int _num_extended_time_intervals;
        int _extended_period_start; /* interval in which the  extended period begins */


        std::pair<Julian, Julian>     getStrictlyPeriod() const;
        virtual void                  SetIntervalCut();
        virtual void                  SetIntervalStartTimes();

        int getTimeIntervalOfExtendedDate(Julian Date) const {
            if (Date < _extended_time_interval_starttimes[0] || Date >= _extended_time_interval_starttimes[_num_time_intervals]) return -1;
            int i=0;
            while (Date >= _extended_time_interval_starttimes[i+1]) ++i;
            return i;
        }

    public:
        ClosedLoopData(const CParameters& Parameters, BasePrint& PrintDirection);
        virtual ~ClosedLoopData() {}

        int getExtendedPeriodStart() const {return _extended_period_start;}
        int getNumExtendedTimeIntervals() const {return _num_extended_time_intervals;}
        const std::vector<Julian> & getExtendedTimeIntervalStartTimes() const {return _extended_time_interval_starttimes;}

        virtual Julian convertToSeasonalDate(Julian j) const;
        virtual Julian intervalIndexToJulian(unsigned int intervalIdx) const {
            return _closedloop_time_interval_starttimes.at(intervalIdx);
        }

        virtual const std::vector<Julian> & GetTimeIntervalStartTimes() const {return _closedloop_time_interval_starttimes;}
        virtual int    GetNumTimeIntervals() const {return _num_time_intervals;}
        virtual int    GetTimeIntervalOfDate(Julian Date) const;
        virtual int    GetTimeIntervalOfEndDate(Julian EndDate) const;
};
//*********************************************************************************
#endif

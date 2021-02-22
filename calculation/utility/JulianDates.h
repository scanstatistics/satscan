//*****************************************************************************
#ifndef __JULIANDATE_H
#define __JULIANDATE_H
//*****************************************************************************
#include "SaTScan.h"
#include <string>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/gregorian_calendar.hpp>

/* Date module header file - SaTScan                      */
/*                                                        */
/* Purpose: To define routines used to handle Julian date */
/*          values.                                       */
/* Author : Katherine Rand, IMS                           */
/* Date   : 12/94 - 1/95                                  */

#define MIN_YEAR  1753 //KR980706 100

#define GENERIC_DATE_BASE_YEAR 2000
#define GENERIC_DATE_BASE_MONTH 1
#define GENERIC_DATE_BASE_DAY 1

typedef unsigned long Julian;
typedef boost::date_time::year_month_day_base<unsigned long, unsigned short, unsigned short > simple_ymd_type;
typedef boost::date_time::gregorian_calendar_base<simple_ymd_type, Julian> gregorian_calendar;

double                          CalculateNumberOfTimeIntervals(Julian StartDate, Julian EndDate, DatePrecisionType eUnits, int iIntervalLength);
int                             CharToMDY(UInt* month, UInt* day, UInt* year, const char* szDateString);
Julian                          CharToJulian(const char* szDateString);
Julian                          relativeDateToJulian(const char* szDateString);
UInt                            DaysThisMonth(UInt nYear, UInt nMonth);
double                          IntervalInYears(DatePrecisionType eUnits, long nLength);
bool                            IsDateValid(UInt month, UInt day, UInt year);
bool                            IsLeapYear(UInt year);
char                          * JulianToChar(char* szDateString, Julian JNum);
void                            JulianToMDY(UInt* month, UInt* day, UInt* year, Julian JNum);
std::string                   & JulianToString(std::string& sDate, Julian JNum, DatePrecisionType eDatePrint, const char * sep="/", bool isEndDate=false, bool asSeasonal = false);
std::string                     gregorianToString(boost::gregorian::date dateObj);
boost::gregorian::date          gregorianFromString(const std::string& s);
void                            MDYToChar(char* szDateString, UInt month, UInt day, UInt year);
Julian                          MDYToJulian(UInt m, UInt d, UInt y);
void                            printDateRange(FILE * pFile);
UInt                            getMinimumYear();
void                            printTimeIntervals(const std::vector<Julian>& intervals, DatePrecisionType eDatePrint);
boost::gregorian::greg_weekday  getWeekDay(Julian date);

/** Class that manages the subtraction of lengths of time from a starting end date.
    This class was created to:
    - aid in the calculation of the number of time aggregation units in study period
    - aid in the calculation of time interval start times                     */
class DecrementableEndDate {
  private:
    Julian              gStartingDate;
    unsigned int        giStartingDateDay;
    Julian              gCurrentDate;
    unsigned int        giCurrentDateTargetMonth;
    DatePrecisionType   geDecrementUnits;
    bool                gbHuggingMonthEnd;

    void                Setup();

  public:
    DecrementableEndDate(Julian StartingDate, DatePrecisionType eDecrementUnits);

    Julian              Decrement(unsigned long ulLength);
};

//*****************************************************************************
#endif

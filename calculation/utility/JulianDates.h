//*****************************************************************************
#ifndef __JULIANDATE_H
#define __JULIANDATE_H
//*****************************************************************************
#include "SaTScan.h"
#include <string>

/* Date module header file - SaTScan                      */
/*                                                        */
/* Purpose: To define routines used to handle Julian date */
/*          values.                                       */
/* Author : Katherine Rand, IMS                           */
/* Date   : 12/94 - 1/95                                  */

#define MAX_DT_STR 11
#define MIN_YEAR  1753 //KR980706 100
#define MAX_YEAR  9999
#define MIN_MONTH 1
#define MAX_MONTH 12
#define MIN_DAY   1
#define MAX_DAY   31

typedef unsigned long Julian;

double          CalculateNumberOfTimeIntervals(Julian StartDate, Julian EndDate, DatePrecisionType eUnits, int iIntervalLength);
int             CharToMDY(UInt* month, UInt* day, UInt* year, const char* szDateString);
Julian          CharToJulian(const char* szDateString);
UInt            DaysThisMonth(UInt nYear, UInt nMonth);
double          IntervalInYears(DatePrecisionType eUnits, long nLength);
bool            IsDateValid(UInt month, UInt day, UInt year);
bool            IsLeapYear(UInt year);
char          * JulianToChar(char* szDateString, Julian JNum);
void            JulianToMDY(UInt* month, UInt* day, UInt* year, Julian JNum);
std::string   & JulianToString(std::string& sDate, Julian JNum);
void            MDYToChar(char* szDateString, UInt month, UInt day, UInt year);
Julian          MDYToJulian(UInt m, UInt d, UInt y);
void            PrintJulianDates(const std::vector<Julian>& vJulianDates, const char * sFilename);
void            ShowJulianRange();

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

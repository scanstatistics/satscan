//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "JulianDates.h"
#include "SSException.h"
#include "UtilityFunctions.h"
#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/gregorian_calendar.hpp"

/* Date module - SaTScan                                  */
/*                                                        */
/* Purpose: To define routines used to handle Julian date */
/*          values.                                       */
/* Author : Katherine Rand, IMS                           */
/* Date   : 12/94 - 1/95                                  */

/** Calculates the number of iIntervalLength'th eUnits in StartDate to EndDate+1. */
double CalculateNumberOfTimeIntervals(Julian StartDate, Julian EndDate, DatePrecisionType eUnits, int iIntervalLength) {
  Julian                IntervalStartingDate = EndDate+1;
  double                dDiffDays, dRemainderDays, dNumberOfTimeIntervals=0;
  unsigned int          uiYear, uiMonth, uiDay;
  DecrementableEndDate  DecrementingDate(EndDate, eUnits);

  IntervalStartingDate = DecrementingDate.Decrement(iIntervalLength);
  while (IntervalStartingDate > StartDate) {
      //push interval start time onto vector
      ++dNumberOfTimeIntervals;
      //find the next prior interval start time from current, given length of time intervals
      IntervalStartingDate = DecrementingDate.Decrement(iIntervalLength);
  }
  if (IntervalStartingDate == StartDate)
    ++dNumberOfTimeIntervals;
  else {
    //The slicing up of the study period into specified units left a factional
    //amount of a unit. Calculate that fraction.
    dDiffDays = StartDate - IntervalStartingDate;
    switch (eUnits) {
      case YEAR  : //calculate remainder in terms of days left in year
                   JulianToMDY(&uiMonth, &uiDay, &uiYear, StartDate);
                   dRemainderDays = (IsLeapYear(uiYear) ? 366 : 365) - dDiffDays;
                   dNumberOfTimeIntervals += dRemainderDays/static_cast<double>(IsLeapYear(uiYear) ? 366 : 365);
                   break;
      case MONTH : //calculate remainder in terms of days left in month
                   JulianToMDY(&uiMonth, &uiDay, &uiYear, StartDate);
                   dRemainderDays = DaysThisMonth(uiYear, uiMonth) - dDiffDays;
                   dNumberOfTimeIntervals += dRemainderDays/static_cast<double>(DaysThisMonth(uiYear, uiMonth));
                   break;
      case DAY   :
      case NONE  :
      default    : throw prg_error("Number of time intervals calculated has balance of %lf days for type '%d'.\n",
                                       "CalculateNumberOfTimeIntervals()", dDiffDays, eUnits);
    };
  }
  return dNumberOfTimeIntervals;
}

/** Gets integer <year>/<month>/<day> date string in parts. */
int CharToMDY(UInt* month, UInt* day, UInt* year, const char* szDateString) {
  *month = 0;
  *day   = 0;
  *year  = 0;

  return sscanf(szDateString,"%u/%u/%u", year, month, day);
}

/** Converts <year>/<month>/<day> date string into julian. */
Julian CharToJulian(const char* szDateString) {
  UInt month, day, year;

  if (CharToMDY(&month, &day, &year, szDateString) < 3)
    return(0);

  return MDYToJulian(month, day, year);
}

Julian relativeDateToJulian(const char* szDateString) {
  long relativeDate;

  if (!string_to_type<long>(szDateString, relativeDate))
     throw resolvable_error("Failed to convert generic date '%s' to integer.\n", szDateString);

  boost::gregorian::date baseDate(GENERIC_DATE_BASE_YEAR, GENERIC_DATE_BASE_MONTH, GENERIC_DATE_BASE_DAY);
  Julian initDay = baseDate.julian_day();
  boost::gregorian::date minDate(boost::date_time::min_date_time);
  Julian minDay = minDate.julian_day();
  boost::gregorian::date maxDate(boost::date_time::max_date_time);
  Julian maxDay = maxDate.julian_day();

  if (initDay + relativeDate < minDay || initDay + relativeDate >= maxDay)
    throw resolvable_error("Generic date '%s' is outside of valid range of values [%d, %d].\n", szDateString, -1 * (initDay - minDay), maxDay - initDay - 1);

  boost::gregorian::date_duration d(relativeDate);
  return (baseDate + d).julian_day();
}

/** returns minimum year permitted by system. */
UInt getMinimumYear() {
  boost::gregorian::date minDate(boost::date_time::min_date_time);
  gregorian_calendar::ymd_type ymd_min = gregorian_calendar::from_julian_day_number(minDate.julian_day());
  return ymd_min.year;
}

/** Returns number of days in this month for year. */
UInt DaysThisMonth(UInt nYear, UInt nMonth) {
  return gregorian_calendar::end_of_month_day(nYear, nMonth);
}

/** Returns number of years nLength eUnits equates to. */
double IntervalInYears(DatePrecisionType eUnits, long nLength) {
  double nInterval=0;

  if (eUnits == DAY)
    nInterval = (double)nLength/AVERAGE_DAYS_IN_YEAR;
  else if (eUnits == MONTH)
    nInterval = (double)nLength/12.00;
  else if (eUnits == YEAR)
    nInterval = (double)nLength;

  return nInterval;
}

/** Returns whether a date is valid or not. */
bool IsDateValid(UInt month, UInt day, UInt year) {
  try {
      boost::gregorian::date test(year, month, day);
      return !test.is_special();
  } catch (boost::gregorian::bad_day_of_month&) {
    return false;
  } catch (...) {
    return false;
  }
}

/** Return whether year is a leap year. */
bool IsLeapYear(UInt year) {
  return gregorian_calendar::is_leap_year(year);
}

/** Converts Julian into date string. */
char* JulianToChar(char* szDateString, Julian JNum) {
  UInt month, day, year;

  JulianToMDY(&month, &day, &year, JNum);
  MDYToChar(szDateString, month, day, year);

  return szDateString;
}

/** Converts Julian into date string. */
std::string& JulianToString(std::string& sDate, Julian JNum, DatePrecisionType eDatePrint, const char * sep, bool isEndDate, bool asSeasonal) {
  UInt month, day, year;

  JulianToMDY(&month, &day, &year, JNum);
  sep = sep == 0 ? "/" : sep;
  switch (eDatePrint) {
      case YEAR    :
          if (asSeasonal) throw prg_error("Wrong date precision specified '%d' for seasonal representation.","JulianToString()", eDatePrint);
          printString(sDate, "%u", year);
          break;
      case MONTH   :
          if (asSeasonal) printString(sDate, "%u", month);
          else printString(sDate, "%u%s%u", year, sep, month);
          break;
      case DAY     :
		  if (asSeasonal) {
			  if (isEndDate && month == 2 && day == 28) {
				  /* If February 28 is the end date, it should be manually revised to February 29. Note that even if a 2 day maximum
				     cluster size was specified, a “three day” cluster from Feb 27 to Feb 29 could be detected, as February 29 does 
					 not count when calculating the maximum. */
				  day = 29;
			  }
			  printString(sDate, "%u%s%u", month, sep, day);
		  }
          else printString(sDate, "%u%s%u%s%u", year, sep, month, sep, day);
          break;
      case GENERIC : {
                     boost::gregorian::date baseDate(GENERIC_DATE_BASE_YEAR, GENERIC_DATE_BASE_MONTH, GENERIC_DATE_BASE_DAY);
                     printString(sDate, "%ld", JNum - baseDate.julian_day()); break;
                     }
      case NONE    :
      default      : throw prg_error("Wrong date precision specified '%d'.","JulianToString()", eDatePrint);
  }
  return sDate;
}

/* JulianToMDY converts a Julian day number to a Gregorian calendar date. */
void JulianToMDY(UInt* month, UInt* day, UInt* year, Julian JNum) {
  gregorian_calendar::ymd_type ymd6 = gregorian_calendar::from_julian_day_number(JNum);
  *month = ymd6.month;
  *day   = ymd6.day;
  *year  = ymd6.year;
}

/** Converts date parts in date string. */
void MDYToChar(char* szDateString, UInt month, UInt day, UInt year) {
  sprintf(szDateString,"%u/%u/%u\0", year, month, day);
}

/* MDYToJulian converts a Gregorian calendar date to a Julian day.       */
Julian MDYToJulian(UInt m, UInt d, UInt y) {
  if ( !IsDateValid(m, d, y) )
    return( (Julian)0 );

  boost::gregorian::date getDate(y, m, d);
  return getDate.julian_day();
}

/** Prints julian dates for file. Debug function. */
void printDateRange(FILE * pFile) {
  std::string                           sBuffer;

  try {
    boost::gregorian::date minDate(boost::date_time::min_date_time);
    Julian minDay = minDate.julian_day();
    boost::gregorian::date maxDate(boost::date_time::max_date_time);
    Julian maxDay = maxDate.julian_day();

    for (Julian j=minDay; j <= maxDay; ++j)
       fprintf(pFile, "Julian %lu: %s\n", j, JulianToString(sBuffer, j, DAY).c_str());
  } catch (prg_exception& x) {
    x.addTrace("printDateRange()","JulianDate.cpp");
    throw;
  }
}

/** Debug function to print calculated time intervals. */
void printTimeIntervals(const std::vector<Julian>& intervals, DatePrecisionType eDatePrint) {
    std::string buffer;

    std::cout << "Number of intervals: " << intervals.size() << std::endl;
    for (std::vector<Julian>::const_iterator itr=intervals.begin(); itr != intervals.end(); ++itr) {
        std::cout << JulianToString(buffer, *itr, eDatePrint);
        if (itr + 1 != intervals.end())
            std::cout << " : length in days is " << *(itr + 1) - *itr;
        std::cout << std::endl;
    }
}

boost::gregorian::greg_weekday getWeekDay(Julian date) {
    UInt month, day, year;
    JulianToMDY(&month, &day, &year, date);
    boost::gregorian::date d(year, month, day);
    return d.day_of_week();
}

/** class constructor */
DecrementableEndDate::DecrementableEndDate(Julian StartingDate, DatePrecisionType eDecrementUnits)
                     :gStartingDate(StartingDate + 1),
                      gCurrentDate(StartingDate + 1),
                      geDecrementUnits(eDecrementUnits) {
  Setup();
}

/** Decrements current date by ulLength. */
Julian DecrementableEndDate::Decrement(unsigned long ulLength) {
  Julian        DecrementedDate;
  UInt          nMon1, nDay1, nYear1;
  UInt          nMon2, nYear2;
  long          nTotalMonths;

  JulianToMDY(&nMon1, &nDay1, &nYear1, gCurrentDate);
  //if month of current date does not match its' target month, then the
  //current date was forced to be the first day of following month
  if (giCurrentDateTargetMonth != nMon1)
    //get back to last day of previous month
    JulianToMDY(&nMon1, &nDay1, &nYear1, gCurrentDate - 1);
 
  switch (geDecrementUnits) {
    case DAY   :
    case GENERIC :
      DecrementedDate = gCurrentDate - ulLength;
      JulianToMDY(&nMon1, &nDay1, &nYear1, DecrementedDate);
      giCurrentDateTargetMonth = nMon1;
      gCurrentDate = DecrementedDate;
      break;
    case MONTH :
      //calculate new date
      nTotalMonths = ((nYear1 * 12) + nMon1) - ulLength -1;
      nMon2 = (nTotalMonths % 12) + 1;
      nYear2 = (UInt)floor(static_cast<double>(nTotalMonths / 12));
      //stored calculated target month for next function call
      giCurrentDateTargetMonth = nMon2;
      //if the starting date's day was greater than 28, the day varies from
      //month to month, even year to year for February 
      if (gbHuggingMonthEnd) {
        if (giStartingDateDay > DaysThisMonth(nYear2, nMon2))
          //If the starting date's day is greater than number of days of calculated
          //month/year, make the date roll over to the first day of following month.
          //Note that we never have to be concerned about rolling over to another year
          //since December has 31 days, where giStartingDateDay <= 31.
          DecrementedDate = MDYToJulian(nMon2 + 1, 1, nYear2);
        else
          //Else, the reason we are hugging the end date is because the giStartingDateDay
          //was greater than 28 but possibly not the last day of respective month. This presents
          //the situation where the calculated date's month may have less days than giStartingDateDay.
          DecrementedDate = MDYToJulian(nMon2, std::min(giStartingDateDay, DaysThisMonth(nYear2, nMon2)), nYear2);
      }
      else
        //Else use calculated date.
        DecrementedDate = MDYToJulian(nMon2, std::min(nDay1, DaysThisMonth(nYear2, nMon2)), nYear2);
      break;
    case YEAR  :
      giCurrentDateTargetMonth = nMon1;
      //if the starting date's day was greater than 28, the day varies from
      //month to month, even year to year for February 
      if (gbHuggingMonthEnd) {
        if (giStartingDateDay > DaysThisMonth(nYear1 - ulLength, nMon1))
          //If the starting date's day is greater than number of days of calculated
          //month/year, make the date roll over to the first day of following month.
          //Note that we never have to be concerned about rolling over to another year
          //since December has 31 days, where giStartingDateDay <= 31.
          DecrementedDate = MDYToJulian(nMon1 + 1, 1, nYear1 - ulLength);
        else
          //Else, the reason we are hugging the end date is because the giStartingDateDay
          //was greater than 28 but possibly not the last day of respective month. This presents
          //the situation where the calculated date's month may have less days than giStartingDateDay.
          DecrementedDate = MDYToJulian(nMon1, std::min(giStartingDateDay, DaysThisMonth(nYear1 - ulLength, nMon1)), nYear1 - ulLength);
      }
      else
        //Else use calculated date. 
        DecrementedDate = MDYToJulian(nMon1, std::min(nDay1, DaysThisMonth(nYear1 - ulLength, nMon1)), nYear1 - ulLength);
      break;
    default    :
      throw prg_error("Unknown date precision '%d'.","DecrementDate()", geDecrementUnits);
  }

  gCurrentDate = DecrementedDate;
  return DecrementedDate;
}

/** Internal class setup. */
void DecrementableEndDate::Setup() {
  unsigned int  uiYear, uiMonth, uiDay;

  
  JulianToMDY(&uiMonth, &uiDay, &uiYear, gStartingDate);
  giCurrentDateTargetMonth = uiMonth;
  if (DaysThisMonth(uiYear, uiMonth) == uiDay) {
    gbHuggingMonthEnd = true;
    giStartingDateDay = (uiMonth == 2 && uiDay == 29 ? 29 : 31);
  }
  else if (uiDay > 28) {
    gbHuggingMonthEnd = true;
    giStartingDateDay = uiDay;
  }
  else
    gbHuggingMonthEnd = false;
}

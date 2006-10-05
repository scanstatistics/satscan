#include "SaTScan.h"
#pragma hdrstop
#include "JulianDates.h"
#include "SSException.h"
#include "UtilityFunctions.h"

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
      default    : ZdGenerateException("Number of time intervals calculated has balance of %lf days for type '%d'.\n",
                                       "CalculateNumberOfTimeIntervals()", dDiffDays, eUnits);
    };
  }
  return dNumberOfTimeIntervals;
}

/** Gets YYYY/MM/DD date string in parts. */
int CharToMDY(UInt* month, UInt* day, UInt* year, const char* szDateString) {
  *month = 0;
  *day   = 0;
  *year  = 0;

  return sscanf(szDateString,"%u/%u/%u", year, month, day);
}

/** Converts YYYY/MM/DD date string into julian. */
Julian CharToJulian(const char* szDateString) {
  UInt month, day, year;

  if (CharToMDY(&month, &day, &year, szDateString) < 3)
    return(0);

  return MDYToJulian(month, day, year);
}

/** Returns number of days in this month for year. */
UInt DaysThisMonth(UInt nYear, UInt nMonth) {
  int nMax;

  if      (nMonth==2 && (nYear%4==0 && (nYear%100!=0 || nYear%400==0)))  nMax=29;
  else if (nMonth==2)                                                    nMax=28;
  else if (nMonth==4 || nMonth==6 || nMonth==9 || nMonth==11)            nMax=30;
  else                                                                   nMax=31;

  return nMax;
}

/** Returns number of years nLength eUnits equates to. */
double IntervalInYears(DatePrecisionType eUnits, long nLength) {
  double nInterval=0;

  if (eUnits == DAY)
    nInterval = (double)nLength/365.25;
  else if (eUnits == MONTH)
    nInterval = (double)nLength/12.00;
  else if (eUnits == YEAR)
    nInterval = (double)nLength;

  return nInterval;
}

/** Returns whether a date is valid or not. */
bool IsDateValid(UInt month, UInt day, UInt year) {
  if ( day <= 0 || !( month>=1 && month<=12 ) || !( year>=MIN_YEAR && year<=MAX_YEAR) )
    return(false);

  return day <= DaysThisMonth(year, month);
}

/** Return whether year is a leap year. */
bool IsLeapYear(UInt year) {
  return( (year&3) == 0 && year%100 != 0 || year%400 == 0 );
}

/** Converts Julian into date string. */
char* JulianToChar(char* szDateString, Julian JNum) {
  UInt month, day, year;

  JulianToMDY(&month, &day, &year, JNum);
  MDYToChar(szDateString, month, day, year);

  return szDateString;
}

/** Converts Julian into date string. */
std::string& JulianToString(std::string& sDate, Julian JNum) {
  UInt month, day, year;

  JulianToMDY(&month, &day, &year, JNum);
  printString(sDate, "%u/%u/%u", year, month, day);

  return sDate;
}

/* JulianToMDY converts a Julian day number to a Gregorian calendar date. */
/*  o Algorithm 199 from Communications of the ACM, Volume 6, No. 8,      */
/*       (Aug. 1963), p. 444.                                             */
/*  o 0 is returned if the date is invalid.                               */
/*  o Dates before Sep. 14, 1752 (the start of the Gregorian calendar)    */
/*       will not be accurate.                                            */
void JulianToMDY(UInt* month, UInt* day, UInt* year, Julian JNum) {
  unsigned long d;
  Julian j;
  UInt       m /*= *month*/;
  UInt       D /*= *day*/;
  UInt       y /*= *year*/;

  j = JNum - 1721119L;
  y = (UInt) (((j<<2) - 1) / 146097L);
  j = (j<<2) - 1 - 146097L*y;
  d = (j>>2);
  j = ((d<<2) + 3) / 1461;
  d = (d<<2) + 3 - 1461*j;
  d = (d + 4)>>2;
  m = (UInt)(5*d - 3)/153;
  d = 5*d - 3 - 153*m;
  D = (UInt)((d + 5)/5);
  y = (UInt)(100*y + j);

  if ( m < 10 )
     m += 3;
  else
  {
     m -= 9;
     y++;
  }

  *month = m;
  *day   = D;
  *year  = y;
}

/** Converts date parts in date string. */
void MDYToChar(char* szDateString, UInt month, UInt day, UInt year) {
  sprintf(szDateString,"%u/%u/%u\0", year, month, day);
}

/* MDYToJulian converts a Gregorian calendar date to a Julian day.       */
/*  o Algorithm 199 from Communications of the ACM, Volume 6, No. 8,     */
/*       (Aug. 1963), p. 444.                                            */
/*  o 0 is returned if the date is invalid.                              */
/*  o Dates before Sep. 14, 1752 (the start of the Gregorian calendar)   */
/*       will not be accurate.                                           */
Julian MDYToJulian(UInt m, UInt d, UInt y) {
  unsigned long c, ya;

//KR980706  if( y <= 99 )
//KR980706    y += 1900;

  if( !IsDateValid(m, d, y) )
    return( (Julian)0 );

  if( m > 2 )
    m -= 3;
  else
  {
    m += 9;
    y--;
  }

  c = y / 100;
  ya = y - 100*c;
  return( ((146097L*c)>>2) + ((1461*ya)>>2) + (153*m + 2)/5 + d + 1721119L );
}

/** Prints julian dates for file. Debug function. */
void PrintJulianDates(const std::vector<Julian>& vJulianDates, const char * sFilename) {
  FILE                                * pFile=0;
  std::string                           sBuffer;

  try {
    if ((pFile = fopen(sFilename, "w")) == NULL)
      GenerateResolvableException("Unable to open file.", "PrintJulianDates()");

    for (size_t t=0; t < vJulianDates.size(); ++t)
       fprintf(pFile, "Date %u: %s\n", t + 1, JulianToString(sBuffer, vJulianDates[t]).c_str());
    fclose(pFile); pFile=0;
   }
  catch (ZdException &x) {
    fclose(pFile);
    x.AddCallpath("PrintJulianDates()","JulianDate.cpp");
    throw;
  }
}

/** Prints range of julian dates. */
void   ShowJulianRange() {
  fprintf(stdout, "\n      * Range for date values: year %d - %d, month %d - %d, day %d - %d.\n\n",
                   MIN_YEAR, MAX_YEAR, MIN_MONTH, MAX_MONTH, MIN_DAY, MAX_DAY);

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
      DecrementedDate = gCurrentDate - ulLength;
      JulianToMDY(&nMon1, &nDay1, &nYear1, DecrementedDate);
      giCurrentDateTargetMonth = nMon1;
      gCurrentDate = DecrementedDate;
      break;
    case MONTH :
      //calculate new date
      nTotalMonths = ((nYear1 * 12) + nMon1) - ulLength -1;
      nMon2 = (nTotalMonths % 12) + 1;
      nYear2 = (UInt)floor(nTotalMonths / 12);
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
      ZdGenerateException("Unknown date precision '%d'.","DecrementDate()", geDecrementUnits);
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


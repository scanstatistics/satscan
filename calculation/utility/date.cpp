/* Date module - SaTScan                                  */
/*                                                        */
/* Purpose: To define routines used to handle Julian date */
/*          values.                                       */
/* Author : Katherine Rand, IMS                           */
/* Date   : 12/94 - 1/95                                  */

#include "date.h"

static const unsigned char DaysInMonth[12] =
  { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

Julian CharToJulian(const char* szDateString)
{
  UInt month, day, year;

  if (CharToMDY(&month, &day, &year, szDateString) < 3)
    return(0);

  return( MDYToJulian( month, day, year ) );
}

/* MDYToJulian converts a Gregorian calendar date to a Julian day.       */
/*  o Algorithm 199 from Communications of the ACM, Volume 6, No. 8,     */
/*       (Aug. 1963), p. 444.                                            */
/*  o 0 is returned if the date is invalid.                              */
/*  o Dates before Sep. 14, 1752 (the start of the Gregorian calendar)   */
/*       will not be accurate.                                           */
Julian MDYToJulian(UInt m, UInt d, UInt y)
{
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

int CharToMDY(UInt* month, UInt* day, UInt* year, const char* szDateString)
{
  *month = 0;
  *day   = 0;
  *year  = 0;

  return(sscanf(szDateString,"%u/%u/%u", year, month, day));
}

/* JulianToMDY converts a Julian day number to a Gregorian calendar date. */
/*  o Algorithm 199 from Communications of the ACM, Volume 6, No. 8,      */
/*       (Aug. 1963), p. 444.                                             */
/*  o 0 is returned if the date is invalid.                               */
/*  o Dates before Sep. 14, 1752 (the start of the Gregorian calendar)    */
/*       will not be accurate.                                            */
void JulianToMDY(UInt* month, UInt* day, UInt* year, Julian JNum)
{
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

void   MDYToChar(char* szDateString, UInt month, UInt day, UInt year)
{
  sprintf(szDateString,"%u/%u/%u\0", year, month, day);
}

char* JulianToChar(char* szDateString, Julian JNum)
{
  UInt month, day, year;

  JulianToMDY(&month, &day, &year, JNum);
  MDYToChar(szDateString, month, day, year);

  return(szDateString);
}

bool IsDateValid(UInt month, UInt day, UInt year)
{
  unsigned d;

  if ( day <= 0 || !( month>=1 && month<=12 ) || !( year>=MIN_YEAR && year<=MAX_YEAR) )
    return(false);

  d = DaysInMonth[ month-1 ];
  if ( IsLeapYear(year) && month == 2 )
    d++;

  return( day <= d );
}

bool IsLeapYear(UInt year)
{
  return( (year&3) == 0 && year%100 != 0 || year%400 == 0 );
}

long TimeBetween(Julian nDate1, Julian nDate2, int nUnits)
{
  long   nDifference;
  UInt   nMon1, nDay1, nYear1;
  UInt   nMon2, nDay2, nYear2;

  JulianToMDY(&nMon1, &nDay1, &nYear1, nDate1);
  JulianToMDY(&nMon2, &nDay2, &nYear2, nDate2);

  if (nUnits == DAY)
    nDifference = nDate2 - nDate1 + 1;
  else if (nUnits == MONTH)
    nDifference = (nYear2*12 + nMon2) - (nYear1*12 + nMon1) + 1;
  else if (nUnits == YEAR)
    nDifference = nYear2 - nYear1 + 1 ;
  else if (nUnits == NONE)
    nDifference = 1;

  return (nDifference);
}

Julian DecrementDate(Julian nDate, int nUnits, long nValue)
{
  Julian nNewDate;
  UInt   nMon1, nDay1, nYear1;
  UInt   nMon2, nYear2;
  long   nTotalMonths;

  JulianToMDY(&nMon1, &nDay1, &nYear1, nDate);

  if (nUnits == DAY)
    nNewDate = nDate - nValue;
  else if (nUnits == MONTH)
  {
    nTotalMonths = ((nYear1*12) + nMon1) - nValue -1;
    nMon2        = (nTotalMonths%12)+1;
    nYear2       = (UInt) floor(nTotalMonths/12);
    nNewDate     = MDYToJulian(nMon2, nDay1, nYear2);
  }
  else if (nUnits == YEAR)
    nNewDate = MDYToJulian(nMon1, nDay1, nYear1-nValue);

  return (nNewDate);
}

UInt DaysThisMonth(UInt nYear, UInt nMonth)
{
  int nMax;

  if      (nMonth==2 && (nYear%4==0 && (nYear%100!=0 || nYear%400==0)))  nMax=29;
  else if (nMonth==2)                                                    nMax=28;
  else if (nMonth==4 || nMonth==6 || nMonth==9 || nMonth==11)            nMax=30;
  else                                                                   nMax=31;

  return (nMax);
}

void   ShowJulianRange()
{
  fprintf(stdout, "\n      * Range for date values: year %d - %d, month %d - %d, day %d - %d.\n\n",
                   MIN_YEAR, MAX_YEAR, MIN_MONTH, MAX_MONTH, MIN_DAY, MAX_DAY);

}


double IntervalInYears(int nUnits, long nLength)
{
  double nInterval;

  if (nUnits == DAY)
    nInterval = (double)nLength/365.25;
  else if (nUnits == MONTH)
    nInterval = (double)nLength/12.00;
  else if (nUnits == YEAR)
    nInterval = (double)nLength;

  return (nInterval);
}

// V.2.0.4.4
UInt Ensure4DigitYear(UInt y, char* szLowerBoundDt, char* szUpperBoundDt)
{
  UInt month, day;
  UInt nLowerBound;
  UInt nUpperBound;
  UInt nLowerC;
  UInt nUpperC;
  UInt nLowerD;
  UInt nUpperD;

  UInt nYear4;

  CharToMDY(&month, &day, &nLowerBound, szLowerBoundDt);
  CharToMDY(&month, &day, &nUpperBound, szUpperBoundDt);

  nLowerC = (UInt) floor (nLowerBound / 100);
  nUpperC = (UInt) floor (nUpperBound / 100);
  nLowerD = nLowerBound - nLowerC*100;
  nUpperD = nUpperBound - nUpperC*100;

  if (y > MIN_YEAR)                              // Return y if 4 digit
    nYear4 = y;
  else if (y > 99)
    nYear4 = -2;
  else if (nLowerBound == 0 && nUpperBound == 0) // Assum 1900's if no
    nYear4 = 1900+y;                             // bound specified
  else if (nLowerC == nUpperC)                   // Bound in same century
    nYear4 = nLowerC*100 + y;
  else if (nUpperBound - nLowerBound >= 100)     // Time period > 99 years,
      nYear4 = -1;                               // century can not be determined
  else if (y >= nLowerD)                         // Date in Lower century
    nYear4 = nLowerC*100 + y;
  else if (y <= nUpperD)                         // Date in Upper century
    nYear4 = nUpperC*100 + y;
  else
    nYear4 = -2;

  return nYear4;
}
// V.2.0.4.4


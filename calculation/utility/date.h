/* Date module header file - SaTScan                      */
/*                                                        */
/* Purpose: To define routines used to handle Julian date */
/*          values.                                       */
/* Author : Katherine Rand, IMS                           */
/* Date   : 12/94 - 1/95                                  */

#ifndef _DATE_H
#define _DATE_H

#include <stdio.h>
#include "basic.h"

#define MAX_DT_STR 11

#define MIN_YEAR  1753 //KR980706 100
#define MAX_YEAR  9999
#define MIN_MONTH 1
#define MAX_MONTH 12
#define MIN_DAY   1
#define MAX_DAY   31

#define EMPTY_DT_STR "0000/00/00"

typedef unsigned long Julian;

#ifdef __cplusplus
extern "C" {
#endif

Julian CharToJulian(const char* szDateString);
Julian MDYToJulian(UInt m, UInt d, UInt y);

int    CharToMDY(UInt* month, UInt* day, UInt* year, const char* szDateString);
void   JulianToMDY(UInt* month, UInt* day, UInt* year, Julian JNum);

void   MDYToChar(char* szDateString, UInt month, UInt day, UInt year);
char*  JulianToChar(char* szDateString, Julian JNum);

bool   IsDateValid(UInt month, UInt day, UInt year);
bool   IsLeapYear(UInt year);
UInt   DaysThisMonth(UInt nYear, UInt nMonth);

long   TimeBetween(Julian nDate1, Julian nDate2, int nUnits);
Julian DecrementDate(Julian nDate, int nUnits, long nValue);

void   ShowJulianRange();

double IntervalInYears(int nUnits, long nLength);
UInt Ensure4DigitYear(UInt y, char* szLowerBoundDt, char* szUpperBoundDt);

#ifdef __cplusplus
}
#endif

#endif

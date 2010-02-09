//*****************************************************************************
#ifndef __SATSCAN_H
#define __SATSCAN_H
//*****************************************************************************
#if defined(_MSC_VER) || defined(__BORLANDC__)
#include <Windows.h>
#endif
#include <cstdlib>
#include <stdio.h>
#include <string>
#include <vector>
#include <cctype>
#include <climits>
#include <cmath>
#include <ctime>
#ifdef _WINDOWS_
  #include <io.h>
#else
  #include <unistd.h>
#endif

#ifndef _WINDOWS_
   #define   stricmp strcasecmp
   #define   strnicmp strncasecmp
#endif

#include <utility>
#include <deque>
#ifdef __BORLANDC__
#pragma warn -8012
#pragma warn -8008
#pragma warn -8066
#pragma warn -8055
#endif
#include "boost/dynamic_bitset.hpp"
#define DATE_TIME_INLINE
#include "boost/date_time/posix_time/ptime.hpp"
#undef DATE_TIME_INLINE
#ifdef __BORLANDC__
#pragma warn +8012
#pragma warn +8008
#pragma warn +8066
#pragma warn +8055
#endif

#include "BasePrint.h"

/**********************************************************************
 file: SaTScan.h
 This file contains header information for all SaTScan program files
 **********************************************************************/
/** Current version information. */
#define VERSION_ID      "24"  /** incremental version identifier - this value must
                                  be incremented for each released version in order
                                  for update feature to function correctly */
#define VERSION_MAJOR   "8"
#define VERSION_MINOR   "2"
#define VERSION_RELEASE "0"
#define VERSION_PHASE   "RC 1" /** testing phase name - leave blank for release */
#define VERSION_DATE    "Not Released"

/** PI declaration - this is now defined in math.h but changing may cause
    precision to differ and the programs results. */
#define PI 3.1415926535897932384626433832795028841972
/*radius of earth in km*/
#define EARTH_RADIUS_km 6367
/** comparision tolerance for double precision numbers */
#define DBL_CMP_TOLERANCE 1.0E-9
#ifdef _MSC_VER
  /** default string buffer size for vsnprintf call */
  #define MSC_VSNPRINTF_DEFAULT_BUFFER_SIZE 1000
#endif
/** case count type */
typedef long            count_t;
/** measure type */
typedef double          measure_t;
/** used for tract indeces */
typedef int             tract_t;
/** unsigned int type declaration */
typedef unsigned int    UInt;
/** date precision units */
enum DatePrecisionType { NONE, YEAR=1, MONTH, DAY };
/** va_copy not defined on all compilers */
#if defined(__BORLANDC__) || defined (_MSC_VER) || ( defined(__GNUC__) && (__GNUC__ < 3) )
  #define macro_va_copy(dst,src) dst = src
#else
  #define macro_va_copy(dst,src) va_copy(dst,src);
#endif
//*****************************************************************************
#endif

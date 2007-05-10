//*****************************************************************************
#ifndef __SATSCAN_H
#define __SATSCAN_H
//*****************************************************************************
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <time.h>
#include <float.h>
#ifdef __BORLANDC__
  #include <io.h>
#else
  #include <unistd.h>
#endif

//on linux, the library defines std::auto_ptr, so before zd gets included (and
//declares a std::auto_ptr, we will #define so that zd ends up declaring a
//"std::zd_auto_ptr" instead.
#if defined(LINUX_BASED) && (__GNUC__ == 2) && ( __GNUC_MINOR__ <= 96)
  #include <memory>//declare std::auto_ptr
  #define auto_ptr zd_auto_ptr
  #define auto_ptr_ref zd_auto_ptr_ref
#endif
#include "Salloc.h"//now zd543.h will declare std::zd_auto_ptr.
#include "BasePrint.h"
#include <Basis540Unix.h>
#if defined(LINUX_BASED) && (__GNUC__ == 2) && ( __GNUC_MINOR__ <= 96)
//now take away the defs so that the rest of the code references std::auto_ptr :
  #undef auto_ptr
  #undef auto_ptr_ref
#endif

#include "Toolkit.h"

#include <utility>
#include <deque>
#pragma warn -8012
#pragma warn -8008
#pragma warn -8066
#pragma warn -8055
#include "boost/dynamic_bitset.hpp"
#define DATE_TIME_INLINE
#include "boost/date_time/posix_time/ptime.hpp"
#undef DATE_TIME_INLINE
#pragma warn +8012
#pragma warn +8008
#pragma warn +8066
#pragma warn +8055

/**********************************************************************
 file: SaTScan.h
 This file contains header information for all SaTScan program files
 **********************************************************************/
/** Current version information. */
#define VERSION_ID      "18"  /** incremental version identifier - this value must
                                  be incremented for each released version in order
                                  for update feature to function correctly */
#define VERSION_MAJOR   "7"
#define VERSION_MINOR   "0"
#define VERSION_RELEASE "3"
#define VERSION_PHASE   "" /** testing phase name - leave blank for release */
#define VERSION_DATE    "May 10, 2007"

/** PI declaration - this is now defined in math.h but changing may cause
    precision to differ and the programs results. */
#define PI 3.1415926535897932384626433832795028841972
/*radius of earth in km*/
#define EARTH_RADIUS_km 6367
/** comparision tolerance for double precision numbers */
#define DBL_CMP_TOLERANCE 1.0E-9
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
//*****************************************************************************
#endif

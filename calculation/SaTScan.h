//*****************************************************************************
#ifndef __SATSCAN_H
#define __SATSCAN_H
//*****************************************************************************
#if defined(_MSC_VER)
#define NOMINMAX
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
#include <set>
#ifdef _WINDOWS_
#include <io.h>
#else
#include <unistd.h>
#endif

#ifdef _WINDOWS_
#define stricmp _stricmp
#define strnicmp _strnicmp
#else
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

#include <utility>
#include <deque>
#include "boost/dynamic_bitset.hpp"
#define DATE_TIME_INLINE
#include "boost/date_time/posix_time/ptime.hpp"
#undef DATE_TIME_INLINE

#include "boost/tuple/tuple.hpp"
#include <boost/any.hpp>
#include <boost/dynamic_bitset.hpp>
#include "BasePrint.h"

/**********************************************************************
 file: SaTScan.h
 This file contains header information for all SaTScan program files
 **********************************************************************/
/** Current version information. */
#define VERSION_ID      "49"  /** incremental version identifier - this value must
                                  be incremented for each released version in order
                                  for update feature to function correctly */
#define VERSION_MAJOR   "10"
#define VERSION_MINOR   "2"
#define VERSION_RELEASE "0"
#define VERSION_PHASE   "" /** testing phase name - leave blank for release */
#define VERSION_DATE    "June 2024"

/** PI declaration - this is now defined in math.h but changing may cause
    precision to differ and the programs results. */
#define PI 3.1415926535897932384626433832795028841972
/* convert decimal number of degrees to radians */
#define degrees2radians(dnd) (dnd * PI / 180.0)
/* convert decimal number of degrees to radians */
#define radians2degrees(radian) (radian * 180.0 / PI)
/*radius of earth in km*/
#define EARTH_RADIUS_km 6367
/* Euler constant */
#define EULER 0.5772156649
/** comparision tolerance for double precision numbers */
#define DBL_CMP_TOLERANCE 1.0E-9
/** determines equality between two numbers given some tolerance */
#define macro_equal(x,y,tolerance) (std::fabs(x - y) < tolerance)
/** determines whether number x is less than number y given some tolerance */
#define macro_less_than(x,y,tolerance) (!macro_equal(x,y,tolerance) && x < y)
/** determines whether number x is less than number y given some tolerance */
#define macro_less_than_or_equal(x,y,tolerance) (macro_less_than(x,y,tolerance) || macro_equal(x,y,tolerance))
#ifdef _MSC_VER
  /** default string buffer size for vsnprintf call */
  #define MSC_VSNPRINTF_DEFAULT_BUFFER_SIZE 1000
#endif
/** case count type */
typedef long count_t;
/** measure type */
typedef double measure_t;
/** used for identifier indices */
typedef int tract_t;
/** unsigned int type declaration */
typedef unsigned int UInt;
/** data set index collection type */
typedef std::set<size_t> DataSetIndexes_t;
/** date precision units */
enum DatePrecisionType { NONE, YEAR=1, MONTH, DAY, GENERIC };
/** va_copy not defined on all compilers */
#if defined(__BORLANDC__) || defined (_MSC_VER) || ( defined(__GNUC__) && (__GNUC__ < 3) )
  #define macro_va_copy(dst,src) dst = src
#else
  #define macro_va_copy(dst,src) va_copy(dst,src);
#endif
/** average days in year */
#define AVERAGE_DAYS_IN_YEAR 365.25
/** average days in month */
#define AVERAGE_DAYS_IN_MONTH 30.42
/** round macro */
#define macro_round(x) (x<0?ceil((x)-0.5):floor((x)+0.5))
/** minimum number of simulations to report pvalues */
#define MIN_SIMULATION_RPT_PVALUE 99
/** minimum LLR to report cluster */
#define MIN_CLUSTER_LLR_REPORT 0.001
/** interval range type */
typedef boost::tuple<int,int,int,int>  IntervalRange_t;
/* DateSource types */
enum SourceType {CSV=0, DBASE, SHAPE, EXCEL}; // TODO -- add EXCEL
/* data source fields map container typedef */
typedef std::vector<boost::any> FieldMapContainer_t;
/* line-list types */
enum LinelistType { INDIVIDUAL_ID=0, DESCRIPTIVE_COORD_Y, DESCRIPTIVE_COORD_X, GENERAL_DATA, CATEGORICAL_DATA, CONTINUOUS_DATA, DISCRETE_DATA };
/* data source fields map container typedef for linelist */
typedef boost::tuple<unsigned int, LinelistType, std::string> LinelistTuple_t;
typedef std::vector<LinelistTuple_t> LineListFieldMapContainer_t;
/* arbitrary non-leap year for seasonal dates */
#define SEASONAL_YEAR 2001
/* arbitrary day of month for seasonal dates */
#define SEASONAL_DAY 15

/* standard modulas percentage for frequent time estimations */
#define STANDARD_MODULAS_PERCENT 0.25
/* modulas for frequent time estimations in simulations */
#define SIMULATION_EST_MODULAS 50
/* minimum time for frequent time estimates*/
#define FREQUENT_ESTIMATES_SECONDS 180.0
//*****************************************************************************
#endif

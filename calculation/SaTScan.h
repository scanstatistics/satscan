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
#ifdef INTEL_BASED
  #include <io.h>
#else
  #include <unistd.h>
#endif
#include "Salloc.h"
#include "BasePrint.h"
#include "SSException.h"
#include <Basis540Unix.h>
#include "Toolkit.h"

/**********************************************************************
 file: SaTScan.h
 This file contains header information for all SaTScan program files
 **********************************************************************/
/** Current version information. */
#define VERSION_ID "6"
#define VERSION_NUMBER "4.0_Beta_6"
#define VERSION_DATE "Not Released"
/** New defines for ellipsoid calculations. */
#define MAXELLIPSESHAPES 10
/** PI declaration - this is now defined in math.h but changing may cause
    precision to differ and the programs results. */
#define PI 3.1415926535897932384626433832795028841972
/** Average number of days in year and month. */
#define AVERAGE_DAYS_IN_YEAR 364.2425
#define AVERAGE_DAYS_IN_MONTH (364.2425/12)
/** Maximum length of a line in input file (scanf). */
#define MAX_LINESIZE 4096
/** Maximum string length - used in static allocating of character arrays. */
#define MAX_STR_LEN 150
/** Maximum string length - used in static allocating of character arrays
    for reading file infomation in scanf format. */
#define MAX_LINEITEMSIZE 512
/** Growth increment for GInfo in GridTractCoordinates.cpp */
#define TI_GROW 100
/** Number of clusters to rank */
#define NUM_RANKED 500  //KR-980615 100
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


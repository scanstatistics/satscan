//*****************************************************************************
#ifndef __SATSCAN_H
#define __SATSCAN_H
//*****************************************************************************
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <time.h>
#include <float.h>
#include "Salloc.h"
#include "BasePrint.h"
#include "SSException.h"
#include <Basis540Unix.h>

/**********************************************************************
 file: SaTScan.h
 This file contains header information for all SaTScan program files
 **********************************************************************/

#define SATSCAN_WEBSITE "http:/\/srab.cancer.gov/satscan/"
#define VERSION_NUMBER "v3.0.4"
#define VERSION_DATE "February 6, 2003"

/** New defines for ellipsoid calculations. */
#define MAXELLIPSESHAPES 10
#define PI 3.1415926535897932384626433832795028841972

/** Maximum number of time intervals allowed */
/** Moved from montecarlo.c and cluster.c KR-6/20/97 */
#define MAXTIMEINTERVALS 400 /*KR-6/20/97 120  */

/** Starting size for timesorted[] (in % of MaxCircleSize) */
#define TS_START  200
/** Growth increment for timesorted[] (in % of MaxCircleSize) */
#define TS_GROW   20

/** Max. length of a line in an input file */
#define MAX_LINESIZE 4096
#define MAX_LINEITEMSIZE 512

/** Growth increment for TractInfo in tinfo.c */
#define TI_GROW 100

/** Number of clusters to rank */
#define NUM_RANKED 500  //KR-980615 100

/*#if (sizeof(Julian) == sizeof(long))              /* "Infinite" time value */
/*#define MAX_TIME  LONG_MAX
#elif (sizeof(Julian) == sizeof(int))
#define MAX_TIME  INT_MAX
#elif (sizeof(Julian) == sizeof(short))
#define MAX_TIME  SHRT_MAX
#endif
*/
#define MAX_TIME LONG_MAX

typedef long            count_t;                   /* used for case counts */
typedef double          measure_t;                    /* used for measures */
typedef /*short*/int    tract_t;     /* used for tract indeces (not names) */
typedef unsigned short ushort_tract_t; /* used to create and reference sorted array */

//typedef struct {                                           /* cluster data */
//   double    cd_ratio;        /* likelihood ratio              */
//   count_t   cd_ncases;       /* number of cases               */
//   tract_t   cd_center;       /* center of cluster             */
//   tract_t   cd_ntracts;      /* number of tracts              */
//   Julian    cd_clusterstart; /* start time of cluster         */
//   Julian    cd_clusterend;   /* end time of cluster           */
//   measure_t cd_measure;      /* measure of cluster            */
//   BOOL      cd_bClusterInit; /* Has cluster been initialized? */
//   BOOL      cd_bClusterSet;  /* Has cluster been set?         */
//   int       cd_nClusterType; /* Type of cluster               */
//} cluster_t;

/*KR tract_t GetNeighbor(tract_t t, unsigned nearness); (1/14/97) */
/*KR void    FatalError(char *s);                       (1/14/97) */

void ShowStackAvail(); /*KR-7/23/97*/

#define MAX_STR_LEN 150//150
/*#define PARAMFILENAME "STSDOS.TXT"*/
#define PARAMFILENAME "STSPARAM.PRM"
enum { enSTRING, enINTEGER, enENUM, enFILENAME, enDATE, ebREPLICA };
enum { NONE, YEAR=1, MONTH, DAY };
typedef unsigned int UInt;
//*****************************************************************************
#endif


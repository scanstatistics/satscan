/**********************************************************************
 file: spatscan.h
 This file contains header information for all spatscan program files
 **********************************************************************/

#ifndef SPATSCAN_H
#define SPATSCAN_H

#include <limits.h>
#include <math.h>
#include <time.h>
#include "salloc.h"
#include "date.h"
#include "basic.h"

/* Maximum number of time intervals allowed */
/* Moved from montecarlo.c and cluster.c KR-6/20/97 */
#define MAXTIMEINTERVALS 400 /*KR-6/20/97 120  */

/* Starting size for timesorted[] (in % of MaxCircleSize) */
#define TS_START  200
/* Growth increment for timesorted[] (in % of MaxCircleSize) */
#define TS_GROW   20

/* Max. length of a line in an input file */
#define MAX_LINESIZE 512

/* Growth increment for TractInfo in tinfo.c */
#define TI_GROW 100

/* Number of clusters to rank */
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

#endif

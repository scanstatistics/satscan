// MakeNeighbors.h
// ?Eventually move to data.cpp?

#ifndef __MAKENEIGHBORS_H
#define __MAKENEIGHBORS_H

#include "spatscan.h"

struct tdist {        /* tract-distance structure for calculating Sorted[] */
   tract_t t;     /* tract number     */
	float   dsq;   /* distance squared */
};

short min2(short v1, short v2);

static int CompTd(const void *td1, const void *td2);

void MakeNeighbors(tract_t*  Sorted[],
                   tract_t   NumTracts,
                   tract_t   NumGridTracts,
                   measure_t Measure[],
                   measure_t MaxCircleSize,
                   measure_t nMaxMeasureToKeep,
                   tract_t*  NeighborCounts,
                   int       nDimensions);

#endif

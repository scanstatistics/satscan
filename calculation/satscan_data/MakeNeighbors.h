// MakeNeighbors.h
// ?Eventually move to data.cpp?

#ifndef __MAKENEIGHBORS_H
#define __MAKENEIGHBORS_H

#include "spatscan.h"
#include "tinfo.cpp"
#include "ginfo.cpp"

struct tdist {        /* tract-distance structure for calculating Sorted[] */
   tract_t t;     /* tract number     */
	float   dsq;   /* distance squared */
};

short min2(short v1, short v2);

static int CompTd(const void *td1, const void *td2);
void Transform(double Xold, double Yold, float EllipseAngle, float EllipseShape, double* pXnew, double* pYnew);

void MakeNeighbors(TInfo *pTInfo,
                   GInfo *pGInfo,
                   tract_t***  SortedInt,
                   unsigned short ***SortedUShort,
                   tract_t     NumTracts,
                   tract_t     NumGridTracts,
                   measure_t   Measure[],
                   measure_t   MaxCircleSize,
                   measure_t   nMaxMeasureToKeep,
                   tract_t**   NeighborCounts,
                   int         nDimensions,
                   int         iNumEllipses,
                   double     *pdEShapes,
                   int        *piEAngles,
                   BasePrint *pPrintDirection);

void PrintNeighbors(long lTotalEllipses, tract_t GridTracts, tract_t ***Sorted, BasePrint *pPrintDirection);
#endif

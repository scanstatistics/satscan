// MakeNeighbors.cpp

#include "MakeNeighbors.h"
#include "TimeEstimate.h"
#include "tinfo.h"
#include "ginfo.h"

short min2(short v1, short v2)
{
  if (v1 < v2)
    return(v1);
  else
    return(v2);
}

/**********************************************************************
 Compare two struct td's, according to dsq.
 **********************************************************************/
static int CompTd(const void *td1, const void *td2)
{
   float rtn = (((struct tdist *)td1)->dsq - ((struct tdist *)td2)->dsq);
   if (rtn < 0)   return -1;
   if (rtn > 0)   return 1;
   return 0;
} /* CompTd() */

/**********************************************************************
 Calculate the Sorted[] matrix, such that Sorted[a][b] is the b-th
 closest neighbor to a, and Sorted[a][0] == a.
 **********************************************************************/
void MakeNeighbors(tract_t   *Sorted[],
                   tract_t   NumTracts,
                   tract_t   NumGridTracts,
                   measure_t Measure[],
                   measure_t MaxCircleSize,
                   measure_t nMaxMeasureToKeep,
                   tract_t*  NeighborCounts,
                   int       nDimensions)
{
  tract_t t;                                           /* reference tract */
  tract_t i,j;                                       /* destination tract */
  float *pCoords;  /* coordinates */
  float *pCoords2;  /* coordinates */
  measure_t cummeasure;
  clock_t nStartTime;
  struct tdist *td = (tdist*)Smalloc(NumTracts * sizeof(struct tdist));

  measure_t nMaxMeasure;

  // Actually, if MaxMeasure to be kept, neighbors don't need to be counted...KR-980327
  if (nMaxMeasureToKeep > MaxCircleSize)
    nMaxMeasure = nMaxMeasureToKeep;
  else
    nMaxMeasure = MaxCircleSize;

  nStartTime = clock();

  //pCoords = (float*)Smalloc(nDimensions * sizeof(float));

  for (t = 0; t < NumGridTracts; t++)  // for each tract, ...
  {

    giGetCoords(t, &pCoords);

    for (i = 0; i < NumTracts; i++)  // find distances
    {
      td[i].t = i;
      //pCoords2 = (float*)Smalloc(nDimensions * sizeof(float));
      tiGetCoords(i, &pCoords2);
      td[i].dsq = tiGetDistanceSq(pCoords, pCoords2);
      free(pCoords2);
    }

    qsort(td, NumTracts, sizeof(struct tdist), CompTd);

    // find number of neighbors, allocate memory
    cummeasure = 0;
    for (i=0; i < NumTracts && cummeasure+Measure[td[i].t] <= nMaxMeasure; i++)
    {
      cummeasure += Measure[td[i].t];
      if (cummeasure <= MaxCircleSize)
        NeighborCounts[t]++;
    }

    // NeighborCounts[t] = i;

	 Sorted[t] = (tract_t*)Smalloc((/*NeighborCounts[t]*/i) * sizeof(tract_t));

    /* copy tract numbers */
    for (j = i-1/*NeighborCounts[t]-1*/; j >= 0; j--)
      Sorted[t][j] = td[j].t;

    if(t==9)
      ReportTimeEstimate(nStartTime, NumGridTracts, t+1);

    free(pCoords);
  } /* for t */

  free(td);


}




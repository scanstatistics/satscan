/* calcmsr.c */
#include <stdio.h>
#include <math.h>
#include <float.h>
#include "calcmsr.h"
#include "salloc.h"
#include "date.h"
#include "basic.h"
#include "tinfo.h"
#include "cats.h"
#include "analysis.h"
#include "error.h"

#define DEBUGMEASURE 0

static FILE* pMResult;

int AssignMeasure(
            count_t      *Cases[],
            Julian**     Times,
            tract_t      NumTracts,
				Julian       StartDate,
            Julian       EndDate,
            Julian*      IntervalStart,
            BOOL         bExactTimes,
            int          nTimeAdjust,
            double       nTimeAdjPercent,
				int          nTimeIntervals,
            int          nIntervalUnits,
  				long         nIntervalLength,
            measure_t*** pMeasure,
            count_t*     pTotalCases,
            double*      pTotalPop,
				measure_t*   pTotalMeasure)
{
  int         i, interval;
  double*     pAlpha;
  double*     pRisk;
  int         nCats;
  int         nPops;
  tract_t     nTracts, tract;
  measure_t** m;
  char       *tid;

  /* Create & Use array of interval dates where last interval date = EndDate */
  Julian *IntervalDates = (unsigned long*)Smalloc((nTimeIntervals+1)*sizeof(Julian));
  memcpy(IntervalDates, IntervalStart, (nTimeIntervals+1)*sizeof(Julian));

  printf("Calculating expected number of cases\n");

#if DEBUGMEASURE
  if ((pMResult = fopen("MEASURE.TXT", "w")) == NULL)
  {
	  fprintf(stderr, "  Error: Cannot open output file.\n");
  }
#endif

  tiCalcAlpha(&pAlpha, StartDate, EndDate);

  nTracts = tiGetNumTracts();
  nPops   = tiGetNumPopDates();
  nCats   = catNumCats();

#if DEBUGMEASURE
  DisplayInitialData(StartDate, EndDate, IntervalDates, nTimeIntervals, pAlpha, nPops);
#endif

  CalcRisk(&pRisk, pAlpha, nCats, nTracts, nPops, pTotalPop, pTotalCases);
  Calcm(&m, pRisk, nCats, nTracts, nPops);
  CalcMeasure(pMeasure, m, IntervalDates, StartDate, EndDate, nCats, nTracts, nPops, nTimeIntervals, pTotalMeasure);
  if (nTimeIntervals>1)
    if (nTimeAdjust==1)
      AdjustForDiscreteTimeTrend(pMeasure, Cases, nTracts, nTimeIntervals, pTotalCases, pTotalMeasure);
    else if (nTimeAdjust==2)
      AdjustForPercentageTimeTrend(nTimeAdjPercent, nTimeIntervals, nIntervalUnits, nIntervalLength,
                                   nTracts, pTotalMeasure, pMeasure);

  free(pAlpha);
  free(pRisk);

  for(i=0;i<nPops;i++)
    free(m[i]);
  free(m);



/* Bug report: Check to see that Measure matrix has positive entries. */
#if 1
for (tract=0;tract<NumTracts;tract++) {
   for (interval=0;interval<nTimeIntervals;interval++) {
      if((*pMeasure)[interval][tract]<0) {
			tid = tiGetTid(tract);
			printf("  Error: Negative Measure (%8.4f) in function AssignMeasure(),\n\ttract %d, tractid %s, interval %d.\n",(*pMeasure)[interval][tract], tract, tid, interval);
			FatalError("");
         } /* endif Measure */

      } /* endfor interval*/
   } /* endfor tract*/
#endif


/**** Replaces the raw Measure array with a cummulitative one **************/
#if 0
for(i=0;i<nTimeIntervals;i++) printf("%10.6f\t",(*pMeasure)[i][0]);
printf("\n");
#endif
  for(tract=0; tract<NumTracts; tract++)
     for(i=nTimeIntervals-2;i>=0;i--)
        (*pMeasure)[i][tract]=(*pMeasure)[i+1][tract]+(*pMeasure)[i][tract];
#if 0
for(i=0;i<nTimeIntervals;i++) printf("%10.2f\t",(*pMeasure)[i][0]);
printf("\n");
#endif

#if DEBUGMEASURE
  fprintf(pMResult, "Totals: \n\n");
  fprintf(pMResult, "   Cases      = %li\n   Measure    = %f\n   Population = %f\n", *pTotalCases, *pTotalMeasure, *pTotalPop);
  fclose(pMResult);
#endif

/* Bug check, to ensure that TotalCases=TotalMeasure */
  if(fabs(*pTotalCases-*pTotalMeasure)>0.0001) {
     printf("\n  Error: In function AssignMeasure (calcmsr.c),");
     printf("\n  the total measure is not equal to the total number of cases.");
     printf("\n  TotalCases=%ld,  TotalMeasure=%8.6lf\n",*pTotalCases, *pTotalMeasure);
     FatalError("Program canceled.\n");
     }

  printf("\n");

  free(IntervalDates);

  return(1);
}

int CalcRisk(double** pRisk, double* pAlpha,
             int nCats, tract_t nTracts, int nPops, double* pTotalPop, count_t* pTotalCases)
{
  int      c, n;
  tract_t  t;
  double   nPop;
  count_t  nCaseCount;

  *pRisk = (double*)Smalloc(nCats * sizeof(double));

#if DEBUGMEASURE
  fprintf(pMResult, "Category #    Pop Count           Case Count   Risk\n");
#endif

  *pTotalCases = 0;
  *pTotalPop   = 0;

  for (c=0; c<nCats; c++)
  {
    nPop       = 0;
    nCaseCount = 0;

    for (t=0; t<nTracts; t++)
    {
      nCaseCount = nCaseCount + tiGetCount(t, c);

      if (nCaseCount < 0)
      {
        fprintf(stderr, "  Error: Total cases is greater than maximum allowed.\n");
        FatalError(0);
      }

      for (n=0; n<nPops; n++)
        nPop = nPop + (pAlpha[n]*tiGetPop(t, c, n));
    }
    (*pRisk)[c] = (double)nCaseCount / nPop;
#if DEBUGMEASURE
    fprintf(pMResult, "%i             %f        %li            %f\n",c, nPop, nCaseCount, (*pRisk)[c]);
#endif
	 *pTotalCases += nCaseCount;
    *pTotalPop   += nPop;
  }

#if DEBUGMEASURE
  fprintf(pMResult, "\n");
  fprintf(pMResult, "Total Cases = %li    Total Population = %f\n\n", *pTotalCases, *pTotalPop); 
#endif

  return(1);
}

int Calcm(measure_t*** m, double* pRisk, int nCats, tract_t nTracts, int nPops)
{
  int      c, n;
  tract_t  t;

  *m = (double**)Smalloc(nPops * sizeof(measure_t *));
  for(n=0;n<nPops;n++)
    (*m)[n] = (double*)Smalloc(nTracts * sizeof(measure_t));

  for (n=0; n<nPops; n++)
    for (t=0; t<nTracts; t++)
    {
      (*m)[n][t] = 0.0;

      for (c=0; c<nCats; c++)
        (*m)[n][t] = (*m)[n][t] + (pRisk[c]*tiGetPop(t, c, n));

    }

#if DEBUGMEASURE
  fprintf(pMResult, "Pop\n");
  fprintf(pMResult, "Index  Tract   m\n");
  for (n=0; n<nPops; n++)
  {
	 for (t=0; t<nTracts; t++)
    {
      if (t==0)
        fprintf(pMResult, "%i      ",n);
      else
        fprintf(pMResult, "       ");
      fprintf(pMResult, "%i       %f\n", t, (*m)[n][t]);
    }
  }
  fprintf(pMResult, "\n");
#endif

  return(1);
}

int CalcMeasure(measure_t*** pMeasure, measure_t** m, Julian* IntervalDates, Julian StartDate, Julian EndDate,
                int nCats, tract_t nTracts, int nPops, int nTimeIntervals, measure_t* pTotalMeasure)
{
  int         i;
  int         n;
  tract_t     t;
  measure_t** M;
  int         lower;
  int         upper;
  double      tempRatio, tempSum, temp1, temp2;
  Julian      jLowDate;
  Julian      jLowDatePlus1;
  int         nLowerPlus1;
  long        nTotalYears = EndDate+1-StartDate/*TimeBetween(StartDate, EndDate, DAY)*/;

  M = (double**)Smalloc((nTimeIntervals+1) * sizeof(measure_t *));
  for(i=0;i<nTimeIntervals+1;i++)
	 M[i] = (double*)Smalloc(nTracts * sizeof(measure_t));

  *pMeasure = (double**)Smalloc((nTimeIntervals+1) * sizeof(measure_t *));
  for(i=0;i<nTimeIntervals+1;i++)
	 (*pMeasure)[i] = (double*)Smalloc(nTracts * sizeof(measure_t));

  for (i=0; i<nTimeIntervals+1; i++)
  {
	 tiGetPopUpLowIndex(IntervalDates, i, nTimeIntervals, &upper, &lower);
    jLowDate      = tiGetPopDate(lower);
    nLowerPlus1   = lower+1;
    jLowDatePlus1 = tiGetPopDate(lower+1);

    for (t=0; t<nTracts; t++)
    {
      if (jLowDatePlus1 == -1)
        M[i][t] = m[lower][t];
		else
		{
        temp1     = IntervalDates[i] - jLowDate;
        temp2     = jLowDatePlus1-jLowDate;
        tempRatio = (double)(temp1/temp2);
        M[i][t]   = tempRatio * m[nLowerPlus1][t] + (1 - tempRatio) * m[lower][t];
		}
    } /* nTracts */
  }   /* nTimeIntervals */

  *pTotalMeasure = 0.0;
  for (i=0; i<nTimeIntervals; i++)
  {
    tiGetPopUpLowIndex(IntervalDates, i, nTimeIntervals, &upper, &lower);

    for (t=0; t<nTracts; t++)
    {
      (*pMeasure)[i][t] = 0.0;
		tempSum  = 0.0;
		temp1    = .5*(m[lower][t] + M[i][t])*(IntervalDates[i]-tiGetPopDate(lower));
      temp2    = .5*(m[upper][t] + M[i+1][t])*(tiGetPopDate(upper)-IntervalDates[i+1]);

      for (n=lower; n<upper; n++)
      {
		  tempSum = tempSum + (((m[n][t] + m[n+1][t]) / 2) * (tiGetPopDate(n+1)-tiGetPopDate(n)));
      }
      (*pMeasure)[i][t] = ((tempSum - temp1 - temp2) / nTotalYears);

      *pTotalMeasure += (*pMeasure)[i][t];
    } /* nTracts */
  } /* nTimeIntervals */

#if DEBUGMEASURE
  fprintf(pMResult, "Time  Lower  Upper\n");
  fprintf(pMResult, "Intv  Index  Index    Tract   M           Measure\n");
  for (i=0; i<nTimeIntervals; i++)
  {
	 tiGetPopUpLowIndex(IntervalDates, i, nTimeIntervals, &upper, &lower);
    for (t=0; t<nTracts; t++)
    {
      if (t==0)
        fprintf(pMResult, "%i     %i      %i        ", i, lower, upper);
		else
        fprintf(pMResult, "                      ");
      fprintf(pMResult, "%i       %f    %f\n", t, M[i][t], (*pMeasure)[i][t]);
	 }
  }
  fprintf(pMResult, "\n");
  fprintf(pMResult, "Total Measure = %f\n\n", *pTotalMeasure); 
#endif

  for(i=0;i<nTimeIntervals+1;i++)
    free(M[i]);
  free(M);

  return(1);
}

/*************************************************************************/
/* Adjusts the expected number of cases for a time trend, using the data */
/* itself. For each adjustment interval, which has to be fewer than the  */
/* number of time intervals, all measures are multiplied with a constant */
/* so that the total measure in all adjustment intervals are the same in */
/* proportion to its length in time.                                     */
/* These operations are done on the raw measure matrix rather than the   */
/* later on constructed cumulative measure matrix, while the cumulative  */
/* case matrix is used.                                                  */
/*************************************************************************/

int AdjustForDiscreteTimeTrend(measure_t*** pMeasure,
                               count_t      *Cases[],
                               tract_t      nTracts,
		               			 int          nTimeIntervals,
                               count_t*     pTotalCases,
                               measure_t*   pTotalMeasure)
{
  int     AdjustIntervals;
  int     i,j,jj,k,tract;
  double  sumcases,summeasure;

  AdjustIntervals=nTimeIntervals;
  k=1;j=0;jj=0;

  for(i=0;i<AdjustIntervals;i++) {
	  sumcases=0;
	  summeasure=0;
	  while(j<k*nTimeIntervals/AdjustIntervals && j<nTimeIntervals) {
		  if(j==nTimeIntervals-1)
			  for(tract=0;tract<nTracts;tract++) {
				  sumcases = sumcases + Cases[j][tract];
				  summeasure = summeasure + (*pMeasure)[j][tract];
				  } /* for tract */
		  else for(tract=0;tract<nTracts;tract++) {
				  sumcases = sumcases + (Cases[j][tract]-Cases[j+1][tract]);
              summeasure = summeasure + (*pMeasure)[j][tract];
              } /* for tract */
        j++;
        }  /* while */
#if 0
printf("i=%i sc=%.1lf sm=%.1lf kvot=%.2lf\n",
   i, sumcases,summeasure,(sumcases/summeasure)/((*pTotalCases)/(*pTotalMeasure)));
#endif
     while(jj<k*nTimeIntervals/AdjustIntervals && jj<nTimeIntervals) {
        for(tract=0;tract<nTracts;tract++) (*pMeasure)[jj][tract]=
           (*pMeasure)[jj][tract]*(sumcases/summeasure)/((*pTotalCases)/(*pTotalMeasure));
        jj++;
        }  /* while */
     k++;
	  } /* for i<AdjustIntervals */

  return(1);
}

/*************************************************************************/
/*************************************************************************/
void AdjustForPercentageTimeTrend(double       nTimeAdjPercent,
                                  int          nTimeIntervals,
                                  int          nIntervalUnits,
                                  long         nIntervalLength,
                                  tract_t      nTracts,
                                  measure_t*   pTotalMeasure,
                                  measure_t*** pMeasure)
{
  int    i,t;
  double c;
  double k = IntervalInYears(nIntervalUnits, nIntervalLength);
  double p = 1 + (nTimeAdjPercent/100);
  double nAdjustedMeasure = 0;

  #if DEBUGMEASURE
  fprintf(pMResult, "\nAdjust Measure for Time Trend - %f%% per year.\n\n", nTimeAdjPercent);
  fprintf(pMResult, "\nTime Intv   Tract   Measure\n");
  #endif

  /* Adjust the measure assigned to each interval/tract by yearly percentage */
  for (i=0; i<nTimeIntervals; i++)
    for (t=0; t<nTracts; t++)
    {
      (*pMeasure)[i][t] = (*pMeasure)[i][t]*(pow(p,i*k)) /* * c */ ;
      nAdjustedMeasure += (*pMeasure)[i][t];

      if (nAdjustedMeasure > DBL_MAX)
      {
        printf("  Error: Data overflow due to time trend adjustment.\n");
        FatalError("");
      }
    }


  /* Mutlipy the measure for each interval/tract by constant (c) to obtain */
  /* total adjusted measure (nAdjustedMeasure) equal to previous total     */
  /* measure (*pTotalMeasure).                                             */
  c = (double)(*pTotalMeasure)/nAdjustedMeasure;
  nAdjustedMeasure=0;

  for (i=0; i<nTimeIntervals; i++)
    for (t=0; t<nTracts; t++)
    {
      (*pMeasure)[i][t] = (*pMeasure)[i][t]*c;
      nAdjustedMeasure += (*pMeasure)[i][t];
      #if DEBUGMEASURE
/*      fprintf(pMResult, "%i      %i       %f\n",i,t,(*pMeasure)[i][t]);*/
      fprintf(pMResult, "%f\n",(*pMeasure)[i][t]);
      #endif
    }

  #if DEBUGMEASURE
  fprintf(pMResult, "\nAdjusted Measure Total = %0.2f.\n", nAdjustedMeasure);
  #endif
}

void DisplayInitialData(Julian StartDate, Julian EndDate, Julian* pIntvDates, int nTimeIntervals, double* pAlpha, int nPops)
{
#if DEBUGMEASURE
  int  i;
  char szDate[MAX_DT_STR];

  fprintf(pMResult, "Caluculate Measure\n\n");

  JulianToChar(szDate, StartDate);
  fprintf(pMResult, "\nStart Date = %s\n", szDate);
  JulianToChar(szDate, EndDate);
  fprintf(pMResult, "End Date   = %s\n", szDate);

  DisplayDatesArray(pIntvDates, nTimeIntervals, "Interval Start Dates", pMResult);
  DisplayPopDatesArray(pMResult);

  DisplayAllTractInfo(pMResult);

  fprintf(pMResult, "Pop\nIndex   Alpha\n");
  for (i=0;i<nPops;i++)
    fprintf(pMResult, "%i       %f\n",i, pAlpha[i]);
  fprintf(pMResult, "\n");

#endif
}

/* ValidateMeasures checks measures for all tracts/time intervals to  */
/* determine if any are greater than the maximum circle size.         */
/* For all measures that meet this condition, a message is displayed. */
/* If any measures meet this condition, the function returns a value  */
/* false.                                                             */
/* Function added 5/31/97 by K. Rand                                  */
BOOL ValidateMeasures(measure_t** Measures,
							 measure_t   nTotalMeasure,
							 measure_t   nMaxCircleSize,
							 tract_t     nTracts,
							 int         nTimeIntervals,
							 int         nGeoSize)
{
  int       i;
  tract_t   t;
  char*     tid;
  int       nMinGeoSize;
  BOOL      bError = FALSE;
  BOOL      bErrorThisTract;
  measure_t nMaxMeasure=0;


  for (t=0; t<nTracts; t++)
  {
	 bErrorThisTract=FALSE;

	 for(i=0; i<nTimeIntervals; i++)
	 {
		if (Measures[i][t] > nMaxCircleSize)
		{
		  bErrorThisTract = TRUE;
		  nMaxMeasure     = (Measures[i][t] > nMaxMeasure ? Measures[i][t] : nMaxMeasure);
/*		  fprintf(stderr, "tract= %i, interval=%i, measure=%f, max=%f\n",t,i,Measures[i][t],nMaxCircleSize);*/
		}
	 } /* for i=0-<nTimeIntervals */

	 if (bErrorThisTract)
	 {
		bError = TRUE;
		tid    = tiGetTid(t);
		fprintf(stderr, "The maximum circle size is less than the expected number\n");
		fprintf(stderr, "  of cases in tract %s, therefore the program will not run.\n", tid);
	 }
  } /* for t=0-<nTracts */

  if (bError)
  {
	 nMinGeoSize = (int)ceil( ((float)(nMaxMeasure/nTotalMeasure))*100 );
	 fprintf(stderr, "\nError: To perform an analysis, the maximum geographic\n");
	 fprintf(stderr, "  cluster size needs to be increased to at least %i%%.\n\n", nMinGeoSize);
  }

  return(!bError);
}

/*void AssignTemporalMeasuresAndCases(int         nTimeIntervals,
                                    tract_t     nTracts,
                                    count_t**   Cases,
                                    measure_t** Measure,
                                    count_t**   pPTCases,
                                    measure_t** pPTMeasure)
{
  int i, t;

  *pPTMeasure = (double*)Smalloc((nTimeIntervals+1) * sizeof(measure_t));
  *pPTCases   = (long*)Smalloc((nTimeIntervals+1) * sizeof(count_t));


  for (i=0; i<nTimeIntervals; i++)
  {
    (*pPTMeasure)[i] = 0;
    (*pPTCases)[i] = 0;

    for (t=0; t<nTracts; t++)
    {
      (*pPTMeasure)[i] += Measure[i][t];
      (*pPTCases)[i]   += Cases[i][t];
    }
  }
}

void AssignTemporalCases(int        nTimeIntervals,
                         tract_t    nTracts,
                         count_t**  Cases,
                         count_t*   pPTCases)
{
  int i, t;

  for (i=0; i<nTimeIntervals; i++)
  {
    pPTCases[i] = 0;

    for (t=0; t<nTracts; t++)
      pPTCases[i] += Cases[i][t];
  }

}
*/
BOOL ValidateAllCountsArePossitive(tract_t   nTracts,
                                   int       nTimeIntervals,
                                   count_t** Counts,
                                   count_t   nTotalCount)
{
  tract_t t;
  int     i;
  count_t nSumCount=0;

  for (t=0; t<nTracts; t++)
  {
    for (i=0; i<nTimeIntervals; i++ )
      if (Counts[i][t]<0)
      {
        printf("  Error: Negative value found.\n");
        return(FALSE);
      }
    nSumCount += Counts[0][t];
  }

  if (nSumCount != nTotalCount)
  {
    FatalError("  Error: Totals do not match.\n");
    return(FALSE);
  }

  return(TRUE);
}

BOOL ValidateAllPTCountsArePossitive(tract_t  nTracts,
                                   int      nTimeIntervals,
                                   count_t* Counts,
                                   count_t  nTotalCount)
{
  int     i;
  count_t nSumCount=0;

  for (i=0; i<nTimeIntervals; i++ )
  {
    if (Counts[i]<0)
    {
      printf("  Error: Negative value found.\n");
      return(FALSE);
    }
    nSumCount += Counts[i];
  }

  if (nSumCount != nTotalCount)
  {
    FatalError("  Error: Totals do not match.\n");
    return(FALSE);
  }

  return(TRUE);
}




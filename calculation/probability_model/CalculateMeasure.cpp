#include "SaTScan.h"
#pragma hdrstop
#include "CalculateMeasure.h"
#include "MultipleDimensionArrayHandler.h"

//#define DEBUGMEASURE 0

#ifdef DEBUGMEASURE
static FILE* pMResult;
#endif

/** calculates the risk for each category */
int CalcRisk(const TractHandler *pTInfo, double** pRisk, double* pAlpha, int nCats, 
             tract_t nTracts, int nPops, double* pTotalPop, count_t* pTotalCases, BasePrint *pPrintDirection) {
  int      c, i, n;
  tract_t  t;
  double   nPop;
  count_t  nCaseCount;     

  try {
    *pRisk = (double*)Smalloc(nCats * sizeof(double), pPrintDirection);
#ifdef DEBUGMEASURE
      fprintf(pMResult, "Category #    Pop Count           Case Count   Risk\n");
#endif
    *pTotalCases = 0;
    *pTotalPop   = 0;
    for (c=0; c<nCats; c++) {
       nPop = 0;
       nCaseCount = pTInfo->tiGetCategoryCaseCount(c);
       for (t=0; t<nTracts; t++)
          pTInfo->tiGetAlphaAdjustedPopulation(nPop, t, c, 0, nPops, pAlpha);
       (*pRisk)[c] = (double)nCaseCount/ nPop;
#ifdef DEBUGMEASURE
       fprintf(fp, "%i             %f        %li            %12.25f\n",c, nPop, nCaseCount, (*pRisk)[c]);
#endif
      *pTotalCases += nCaseCount;
       // Check to see if total case or control values have wrapped
       if (*pTotalCases < 0)
         SSGenerateException("Error: Total cases greater than maximum allowed of %ld.\n", "CalcRisk()", std::numeric_limits<count_t>::max());

      *pTotalPop   += nPop;
    }
#ifdef DEBUGMEASURE
  fprintf(pMResult, "\n");
  fprintf(pMResult, "Total Cases = %li    Total Population = %f\n\n", *pTotalCases, *pTotalPop); 
#endif
  }
  catch (ZdException & x) {
    x.AddCallpath("CalcRisk()", "CalculateMeasure.cpp");
    throw;
  }
  return(1);
}

/** Calculates the expected number of cases at a given population date and tract for all categories.
    (*m)[n][t] = expected number of cases at population date index n and tract index t for all categories.
    Scott Hostovich @ July 16,2002 */
int Calcm(const TractHandler *pTInfo, measure_t ** m, double* pRisk, int nCats, tract_t nTracts,
          int nPops, BasePrint *pPrintDirection) {
  int      c, n;
  tract_t  t;

  try {
    for (n=0; n < nPops; ++n)
       for (t=0; t < nTracts; ++t)
          pTInfo->tiGetRiskAdjustedPopulation(m[n][t], t, n, pRisk);
#ifdef DEBUGMEASURE
    fprintf(pMResult, "Pop\n");
    fprintf(pMResult, "Index  Tract   m\n");
    for (n=0; n < nPops; ++n) {
       for (t=0; t<nTracts; t++) {
          if (t==0)
            fprintf(pMResult, "%i      ",n);
          else
            fprintf(pMResult, "       ");
          fprintf(pMResult, "%i       %f\n", t, (*m)[n][t]);
       }
    }
    fprintf(pMResult, "\n");
#endif
  }
  catch (ZdException &x) {
    x.AddCallpath("Calcm()", "CalculateMeasure.cpp");
    throw;
  }
  return(1);
}

/** Knowing the expected number of cases from 'measure_t** m', re-calculates expected number of cases
    taking into account time interval slices.
    (*pMeasure)[i][t] = for time interval index i and tract index t, the expected number of cases for
    all categories. Scott Hostovich @ July 16,2002 */
int CalcMeasure(const TractHandler *pTInfo, measure_t ** pMeasure, measure_t** m, Julian* IntervalDates, 
                Julian StartDate, Julian EndDate, int nCats, tract_t nTracts, int nPops, int nTimeIntervals, 
                measure_t* pTotalMeasure, BasePrint *pPrintDirection) {
  int           i, n, lower, upper, nLowerPlus1;
  tract_t       t;
  measure_t  ** ppM;
  double        tempRatio, tempSum, temp1, temp2;
  Julian        jLowDate, jLowDatePlus1;
  long          nTotalYears = EndDate+1-StartDate/*TimeBetween(StartDate, EndDate, DAY)*/;

  try {
    TwoDimensionArrayHandler<measure_t> M_ArrayHandler(nTimeIntervals+1, nTracts);
    ppM = M_ArrayHandler.GetArray();
    for (i=0; i < nTimeIntervals+1; ++i) {
       pTInfo->tiGetPopUpLowIndex(IntervalDates, i, nTimeIntervals, &upper, &lower);
       jLowDate = pTInfo->tiGetPopDate(lower);
       nLowerPlus1 = lower + 1;
       jLowDatePlus1 = pTInfo->tiGetPopDate(lower+1);
       for (t=0; t < nTracts; ++t) {
          if (jLowDatePlus1 == static_cast<Julian>(-1))
            ppM[i][t] = m[lower][t];
          else {
            temp1     = IntervalDates[i] - jLowDate;
            temp2     = jLowDatePlus1-jLowDate;
            tempRatio = (double)(temp1/temp2);
            ppM[i][t]   = tempRatio * m[nLowerPlus1][t] + (1 - tempRatio) * m[lower][t];
    	  }
       } /* nTracts */
    }   /* nTimeIntervals */
    
    *pTotalMeasure = 0.0;
    for (i=0; i < nTimeIntervals; ++i) {
       pTInfo->tiGetPopUpLowIndex(IntervalDates, i, nTimeIntervals, &upper, &lower);
       for (t=0; t < nTracts; ++t) {
          pMeasure[i][t] = 0.0;
          tempSum  = 0.0;
    	  temp1    = .5*(m[lower][t] + ppM[i][t])*(IntervalDates[i]-pTInfo->tiGetPopDate(lower));
          temp2    = .5*(m[upper][t] + ppM[i+1][t])*(pTInfo->tiGetPopDate(upper)-IntervalDates[i+1]);
          for (n=lower; n < upper; ++n) {
    	     tempSum = tempSum + (((m[n][t] + m[n+1][t]) / 2) * (pTInfo->tiGetPopDate(n+1)-pTInfo->tiGetPopDate(n)));
          }
          pMeasure[i][t] = ((tempSum - temp1 - temp2) / nTotalYears);
          *pTotalMeasure += pMeasure[i][t];
       } /* nTracts */
    } /* nTimeIntervals */

#ifdef DEBUGMEASURE
      fprintf(pMResult, "Time  Lower  Upper\n");
      fprintf(pMResult, "Intv  Index  Index    Tract   M           Measure\n");
      for (i=0; i<nTimeIntervals; i++)
      {
    	 pTInfo->tiGetPopUpLowIndex(IntervalDates, i, nTimeIntervals, &upper, &lower);
        for (t=0; t<nTracts; t++)
        {
          if (t==0)
            fprintf(pMResult, "%i     %i      %i        ", i, lower, upper);
    		else
            fprintf(pMResult, "                      ");
          fprintf(pMResult, "%i       %f    %f\n", t, ppM[i][t], pMeasure[i][t]);
    	 }
      }
      fprintf(pMResult, "\n");
      fprintf(pMResult, "Total Measure = %f\n\n", *pTotalMeasure); 
#endif
  }
  catch (ZdException &x) {
    x.AddCallpath("CalcMeasure()", "CalculateMeasure.cpp");
    throw;
  }
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

   try
      {
      AdjustIntervals=nTimeIntervals;
      k=1;j=0;jj=0;
    
      for(i=0;i<AdjustIntervals;i++)
         {
    	 sumcases=0;
    	 summeasure=0;
    	 while(j<k*nTimeIntervals/AdjustIntervals && j<nTimeIntervals) 
    	    {
    	    if(j==nTimeIntervals-1)
    	       for(tract=0;tract<nTracts;tract++) 
    	          {
    		  sumcases = sumcases + Cases[j][tract];
    		  summeasure = summeasure + (*pMeasure)[j][tract];
    		  } /* for tract */
    	    else 
    	       for(tract=0;tract<nTracts;tract++) 
    	          {
    		  sumcases = sumcases + (Cases[j][tract]-Cases[j+1][tract]);
                  summeasure = summeasure + (*pMeasure)[j][tract];
                  } /* for tract */
            j++;
            }  /* while */
#if 0
   printf("i=%i sc=%.1lf sm=%.1lf kvot=%.2lf\n",
      i, sumcases,summeasure,(sumcases/summeasure)/((*pTotalCases)/(*pTotalMeasure)));
#endif
      while(jj<k*nTimeIntervals/AdjustIntervals && jj<nTimeIntervals) 
         {
         for(tract=0;tract<nTracts;tract++) 
            (*pMeasure)[jj][tract]=(*pMeasure)[jj][tract]*(sumcases/summeasure)/((*pTotalCases)/(*pTotalMeasure));
         jj++;
         }  /* while */
      k++;
      } /* for i<AdjustIntervals */
      }
   catch (ZdException & x)
      {
      x.AddCallpath("AdjustForDiscreteTimeTrend()", "CalculateMeasure.cpp");
      throw;
      }
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
                                  measure_t*** pMeasure,
                                  BasePrint *pPrintDirection)
{
   int    i,t;
   double c;
   double k = IntervalInYears(nIntervalUnits, nIntervalLength);
   double p = 1 + (nTimeAdjPercent/100);
   double nAdjustedMeasure = 0;
 
   try
      {
      #ifdef DEBUGMEASURE
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
           SSGenerateException("  Error: Data overflow due to time trend adjustment.\n", "AdjustForPercentageTimeTrend");
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
          #ifdef DEBUGMEASURE
    /*      fprintf(pMResult, "%i      %i       %f\n",i,t,(*pMeasure)[i][t]);*/
          fprintf(pMResult, "%f\n",(*pMeasure)[i][t]);
          #endif
        }
    
      #ifdef DEBUGMEASURE
      fprintf(pMResult, "\nAdjusted Measure Total = %0.2f.\n", nAdjustedMeasure);
      #endif  
      }
   catch (ZdException & x)
      {
      x.AddCallpath("AdjustForPercentageTimeTrend()", "CalculateMeasure.cpp");
      throw;
      }
}

void DisplayInitialData(Julian StartDate, Julian EndDate, Julian* pIntvDates, int nTimeIntervals, double* pAlpha, int nPops)
{
#ifdef DEBUGMEASURE
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
bool ValidateMeasures(const TractHandler *pTInfo,
                      measure_t** Measures,
		      measure_t   nTotalMeasure,
		      measure_t   nMaxCircleSize,
		      tract_t     nTracts,
		      int         nTimeIntervals,
		      int         nGeoSize,
                      BasePrint *pPrintDirection)
{
   int       i;
   tract_t   t;
   const char*     tid;
   int       nMinGeoSize;
   bool      bError = false;
   bool      bErrorThisTract;
   measure_t nMaxMeasure=0;

   try
      {
      for (t=0; t<nTracts; t++)
      {
         bErrorThisTract=false;
    
    	 for(i=0; i<nTimeIntervals; i++)
    	 {
    		if (Measures[i][t] > nMaxCircleSize)
    		{
    		  bErrorThisTract = true;
    		  nMaxMeasure     = (Measures[i][t] > nMaxMeasure ? Measures[i][t] : nMaxMeasure);
    /*		  fprintf(stderr, "tract= %i, interval=%i, measure=%f, max=%f\n",t,i,Measures[i][t],nMaxCircleSize);*/
    		}
    	 } /* for i=0-<nTimeIntervals */

    	 if (bErrorThisTract)
    	 {
    		bError = true;
                std::string sBuffer;
    		tid    = pTInfo->tiGetTid(t, sBuffer);
                char sMessage[200], sTemp[100];
    		sprintf(sMessage, "The maximum circle size is less than the expected number\n");
    		sprintf(sTemp, "  of cases in tract %s, therefore the program will not run.\n", tid);
                strcat(sMessage, sTemp);
                fprintf(stderr, sMessage);
                //SSGenerateException(sMessage, "ValidateMeasures()");

                //DO NOT THROW EXCEPTION HERE... PRINT THE INFO...
                //EXCEPTION THROW BELOW !!!
                pPrintDirection->SatScanPrintWarning(sMessage);
    	 }
      } /* for t=0-<nTracts */
    
      if (bError)
      {
         char sMessage[200], sTemp[100];
    	 nMinGeoSize = (int)ceil( ((float)(nMaxMeasure/nTotalMeasure))*100 );
    	 sprintf(sMessage, "\nError: To perform an analysis, the maximum geographic\n");
    	 sprintf(sTemp, "  cluster size needs to be increased to at least %i%%.\n\n", nMinGeoSize);
         strcat(sMessage, sTemp);
    	 fprintf(stderr, sMessage);
         SSGenerateException(sMessage, "ValidateMeasures()");
      }
      }
   catch (ZdException & x)
      {
      x.AddCallpath("ValidateMeasures()", "CalculateMeasure.cpp");
      throw;
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
bool ValidateAllCountsArePossitive(tract_t   nTracts,
                                   int       nTimeIntervals,
                                   count_t** Counts,
                                   count_t   nTotalCount,
                                   BasePrint *pPrintDirection)
{
   tract_t t;
   int     i;
   count_t nSumCount=0;

   try
      {
      for (t=0; t<nTracts; t++)
         {
         for (i=0; i<nTimeIntervals; i++ )
            if (Counts[i][t]<0)
            {
            pPrintDirection->SatScanPrintf("Error: Negative value found.\n");
            return(false);
            }
         nSumCount += Counts[0][t];
         }
    
      if (nSumCount != nTotalCount)
        SSGenerateException("Error: Totals do not match.\n", "ValidateAllCountsArePossitive");
      }
   catch (ZdException & x)
      {
      x.AddCallpath("ValidateAllCountsArePossitive()", "CalculateMeasure.cpp");
      throw;
      }
  return(true);
}

bool ValidateAllPTCountsArePossitive(tract_t  nTracts,
                                   int      nTimeIntervals,
                                   count_t* Counts,
                                   count_t  nTotalCount,
                                   BasePrint *pPrintDirection)
{
   int     i;
   count_t nSumCount=0;

   try
      {
      for (i=0; i<nTimeIntervals; i++ )
         {
         if (Counts[i]<0)
            {
            pPrintDirection->SatScanPrintf("Error: Negative value found.\n");
            return(false);
            }
         nSumCount += Counts[i];
         }
    
      if (nSumCount != nTotalCount)
        SSGenerateException("Error: Totals do not match.\n", "ValidateAllPTCountsArePossitive");
      }
   catch (ZdException & x)
      {
      x.AddCallpath("ValidateAllPTCountsArePossitive()", "CalculateMeasure.cpp");
      throw;
      }
  return(true);
}




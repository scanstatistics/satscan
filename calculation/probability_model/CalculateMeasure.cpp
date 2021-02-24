//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "CalculateMeasure.h"
#include "MultipleDimensionArrayHandler.h"
#include "SSException.h"

//#define DEBUGMEASURE 0

#ifdef DEBUGMEASURE
static FILE* pMResult;
#endif

/** Calculates the risk for each population category storing results in vector
    'vRisk'. Sets data set's total case and population counts. */
std::vector<double>& CalcRisk(RealDataSet& Set, const Julian StudyStartDate, const Julian StudyEndDate, std::vector<double>& vRisk, PopulationData * pAltPopulationData) {
  int                   c;
  tract_t               t;
  double                nPop, dTotalPopulation=0;
  count_t               nCaseCount, tTotalCases=0;
  std::vector<double>   vAlpha;
  const PopulationData& population = pAltPopulationData ? *pAltPopulationData : Set.getPopulationData();

  try {
    vRisk.resize(population.GetNumCovariateCategories(), 0);
    //calculate alpha - an array that indicates each population interval's percentage of study period.
    population.CalculateAlpha(vAlpha, StudyStartDate, StudyEndDate);
#ifdef DEBUGMEASURE
      fprintf(pMResult, "Category #    Pop Count           Case Count   Risk\n");
#endif
    for (c=0; c < population.GetNumCovariateCategories(); ++c) {
       nPop = 0;
       nCaseCount = population.GetNumCovariateCategoryCases(c);
       for (t=0; t < (int)Set.getLocationDimension(); ++t)
          population.GetAlphaAdjustedPopulation(nPop, t, c, 0, population.GetNumPopulationDates(), vAlpha);
       if (nPop)
         vRisk[c] = (double)nCaseCount/nPop;
#ifdef DEBUGMEASURE
       fprintf(fp, "%i             %f        %li            %12.25f\n",c, nPop, nCaseCount, vRisk[c]);
#endif
      tTotalCases += nCaseCount;
       // Check to see if total case or control values have wrapped
       if (tTotalCases < 0)
         throw resolvable_error("Error: The total number of cases in data set %u is greater than the maximum allowed of %ld.\n",
                                Set.getSetIndex(), std::numeric_limits<count_t>::max());

      dTotalPopulation += nPop;
    }
#ifdef DEBUGMEASURE
  fprintf(pMResult, "\n");
  fprintf(pMResult, "Total Cases = %li    Total Population = %f\n\n", *pTotalCases, *pTotalPop); 
#endif
    // Only set data set total attributes if using data set's population data, not alternate.
    if (!pAltPopulationData) {
        Set.setTotalCases(tTotalCases);
        Set.setTotalPopulation(dTotalPopulation);
    }
  }
  catch (prg_exception & x) {
    x.addTrace("CalcRisk()", "CalculateMeasure.cpp");
    throw;
  }
  return vRisk;
}

/** Calculates the expected number of cases at a given population date and tract
    for all categories represented by modifying the data sets population
    measure array such that m[n][t] = expected number of cases at population date
    index n and tract index t, for all categories of that tract. */
boost::shared_ptr<TwoDimMeasureArray_t> Calcm(RealDataSet& Set, const Julian StudyStartDate, const Julian StudyEndDate, PopulationData * pAltPopulationData) {
  std::vector<double>                     vRisk;
  PopulationData                        & population = pAltPopulationData ? *pAltPopulationData : Set.getPopulationData();
  int                                     nPops = population.GetNumPopulationDates();
  tract_t                                 nTracts = Set.getLocationDimension();
  boost::shared_ptr<TwoDimMeasureArray_t> pPopMeasure(new TwoDimMeasureArray_t(population.GetNumPopulationDates(), Set.getLocationDimension()));

  try {
    //calculate risk for each population category
    CalcRisk(Set, StudyStartDate, StudyEndDate, vRisk, pAltPopulationData);
    measure_t ** m = pPopMeasure->GetArray();
    for (int n=0; n < nPops; ++n)
       for (tract_t t=0; t < nTracts; ++t)
          population.GetRiskAdjustedPopulation(m[n][t], t, n, vRisk);
#ifdef DEBUGMEASURE
    fprintf(pMResult, "Pop\n");
    fprintf(pMResult, "Index  Tract   m\n");
    for (int n=0; n < nPops; ++n) {
       for (tract_t t=0; t < nTracts; t++) {
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
  catch (prg_exception &x) {
    x.addTrace("Calcm()","CalculateMeasure.cpp");
    throw;
  }
  return pPopMeasure;
}

/** Translates previously calculated population measure array, with supporting
    information, into non-cumulative measure array through interpolation.
    Resulting array is such that:
    ppNonCumulativeMeasure[i][t] = expected number of cases for time interval
    at index i and tract at index t. Caller is responsible for ensuring that
    'vIntervalStartDates' contains a number of elements equaling the number of
    time intervals plus one.*/
void CalcMeasure(RealDataSet& DataSet, const TwoDimMeasureArray_t& PopMeasure, const std::vector<Julian>& vIntervalStartDates, const Julian StartDate, const Julian EndDate, PopulationData * pAltPopulationData) {

  PopulationData      & population = pAltPopulationData ? *pAltPopulationData : DataSet.getPopulationData();
  int                   i, n, lower, upper, nLowerPlus1, nTimeIntervals = DataSet.getIntervalDimension();
  tract_t               t, nTracts = DataSet.getLocationDimension();
  measure_t          ** ppM, ** pPopulationMeasure, ** ppNonCumulativeMeasure, tTotalMeasure=0;
  double                tempRatio, tempSum, temp1, temp2;
  Julian                jLowDate, jLowDatePlus1;
  long                  nTotalDays = EndDate + 1 - StartDate;

  try {
    //get reference to data sets population measure
    pPopulationMeasure = PopMeasure.GetArray();
    //get reference to non-cummulative measure array
    ppNonCumulativeMeasure = DataSet.allocateMeasureData().GetArray();
    //allocate temporary measure array to aide interpolation process
    TwoDimensionArrayHandler<measure_t> M_ArrayHandler(nTimeIntervals+1, nTracts);
    ppM = M_ArrayHandler.GetArray();
    
    for (i=0; i < nTimeIntervals+1; ++i) {
       population.GetPopUpLowIndex(vIntervalStartDates, i, nTimeIntervals, upper, lower);
       jLowDate = population.GetPopulationDate(lower);
       nLowerPlus1 = lower + 1;
       jLowDatePlus1 = population.GetPopulationDate(lower+1);
       for (t=0; t < nTracts; ++t) {
          if (jLowDatePlus1 == static_cast<Julian>(-1))
            ppM[i][t] = pPopulationMeasure[lower][t];
          else {
            temp1     = vIntervalStartDates[i] - jLowDate;
            temp2     = jLowDatePlus1 - jLowDate;
            tempRatio = (double)(temp1/temp2);
            ppM[i][t]   = tempRatio * pPopulationMeasure[nLowerPlus1][t] + (1 - tempRatio) * pPopulationMeasure[lower][t];
    	  }
       }
    }

    for (i=0; i < nTimeIntervals; ++i) {
       population.GetPopUpLowIndex(vIntervalStartDates, i, nTimeIntervals, upper, lower);
       for (t=0; t < nTracts; ++t) {
          ppNonCumulativeMeasure[i][t] = 0.0;
          tempSum  = 0.0;
    	  temp1    = .5*(pPopulationMeasure[lower][t] + ppM[i][t])*(vIntervalStartDates[i]-population.GetPopulationDate(lower));
          temp2    = .5*(pPopulationMeasure[upper][t] + ppM[i+1][t])*(population.GetPopulationDate(upper)-vIntervalStartDates[i+1]);
          for (n=lower; n < upper; ++n) {
    	     tempSum = tempSum + (((pPopulationMeasure[n][t] + pPopulationMeasure[n+1][t]) / 2) * (population.GetPopulationDate(n+1)-population.GetPopulationDate(n)));
          }
          ppNonCumulativeMeasure[i][t] = ((tempSum - temp1 - temp2) / nTotalDays);
          tTotalMeasure += ppNonCumulativeMeasure[i][t];
       }
    }

#ifdef DEBUGMEASURE
      fprintf(pMResult, "Time  Lower  Upper\n");
      fprintf(pMResult, "Intv  Index  Index    Tract   M           Measure\n");
      for (i=0; i<nTimeIntervals; i++)
      {
    	 population.GetPopUpLowIndex(IntervalDates, i, nTimeIntervals, upper, lower);
        for (t=0; t<nTracts; t++)
        {
          if (t==0)
            fprintf(pMResult, "%i     %i      %i        ", i, lower, upper);
    		else
            fprintf(pMResult, "                      ");
          fprintf(pMResult, "%i       %f    %f\n", t, ppM[i][t], ppNonCumulativeMeasure[i][t]);
    	 }
      }
      fprintf(pMResult, "\n");
      fprintf(pMResult, "Total Measure = %f\n\n", *pTotalMeasure); 
#endif
  }
  catch (prg_exception &x) {
    x.addTrace("CalcMeasure()", "CalculateMeasure.cpp");
    throw;
  }
    // Only set data set total attributes if using data set's population data, not alternate.
    if (!pAltPopulationData) {
        DataSet.setTotalMeasure(tTotalMeasure);
    }
}


/*************************************************************************/

void DisplayInitialData(Julian StartDate, Julian EndDate, Julian* pIntvDates, int nTimeIntervals, double* pAlpha, int nPops)
{
#ifdef DEBUGMEASURE
  int  i;
  char szDate[11];

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
    		tid    = pTInfo->getLocations().at(t)->getIndentifier();
                char sMessage[200], sTemp[100];
    		sprintf(sMessage, "The maximum circle size is less than the expected number\n");
    		sprintf(sTemp, "  of cases in tract %s, therefore the program will not run.\n", tid);
                strcat(sMessage, sTemp);
                fprintf(stderr, "%s", sMessage);

                //DO NOT THROW EXCEPTION HERE... PRINT THE INFO...
                //EXCEPTION THROW BELOW !!!
                pPrintDirection->Printf(sMessage, BasePrint::P_WARNING);
    	 }
      } /* for t=0-<nTracts */
    
      if (bError)
      {
         char sMessage[200], sTemp[100];
    	 nMinGeoSize = (int)ceil( ((float)(nMaxMeasure/nTotalMeasure))*100 );
    	 sprintf(sMessage, "\nError: To perform an analysis, the maximum geographic\n");
    	 sprintf(sTemp, "  cluster size needs to be increased to at least %i%%.\n\n", nMinGeoSize);
         strcat(sMessage, sTemp);
    	 fprintf(stderr, "%s", sMessage);
         throw resolvable_error(sMessage);
      }
      }
   catch (prg_exception & x)
      {
      x.addTrace("ValidateMeasures()", "CalculateMeasure.cpp");
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
            pPrintDirection->Printf("Error: Negative value found.\n", BasePrint::P_ERROR);
            return(false);
            }
         nSumCount += Counts[0][t];
         }
    
      if (nSumCount != nTotalCount)
        throw resolvable_error("Error: Totals do not match.\n");
      }
   catch (prg_exception & x)
      {
      x.addTrace("ValidateAllCountsArePossitive()", "CalculateMeasure.cpp");
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
            pPrintDirection->Printf("Error: Negative value found.\n", BasePrint::P_ERROR);
            return(false);
            }
         nSumCount += Counts[i];
         }

      if (nSumCount != nTotalCount)
        throw prg_error("Error: Totals do not match.\n", "ValidateAllPTCountsArePossitive");
      }
   catch (prg_exception & x)
      {
      x.addTrace("ValidateAllPTCountsArePossitive()", "CalculateMeasure.cpp");
      throw;
      }
  return(true);
}




#include "SaTScan.h"
#pragma hdrstop
#include "PoissonModel.h"

/** constructor */
CPoissonModel::CPoissonModel(CParameters& Parameters, CSaTScanData& Data, BasePrint& PrintDirection)
              :CModel(Parameters, Data, PrintDirection) {}

/** destructor */
CPoissonModel::~CPoissonModel() {}

/** Adjusts the expected number of cases for a time trend, using the data
    itself. For each adjustment interval, which has to be fewer than the
    number of time intervals, all measures are multiplied with a constant
    so that the total measure in all adjustment intervals are the same in
    proportion to its length in time.
    These operations are done on the raw measure matrix rather than the
    later on constructed cumulative measure matrix, while the cumulative
    case matrix is used.                                                */
void CPoissonModel::AdjustForNonParameteric(DataStream & thisStream, measure_t ** pNonCumulativeMeasure) {
  int                   i, j, jj, k, tract, AdjustIntervals;
  double                sumcases,summeasure;
  count_t            ** ppCases(thisStream.GetCaseArray());

  AdjustIntervals = gData.m_nTimeIntervals;
  k  = 1;
  j  = 0;
  jj = 0;

  for (i=0; i<AdjustIntervals; i++)
  {
	 sumcases   = 0;
	 summeasure = 0;
    while (j < k*gData.m_nTimeIntervals/AdjustIntervals && j < gData.m_nTimeIntervals)
    {
      if (j == gData.m_nTimeIntervals-1)
        for (tract=0; tract < gData.m_nTracts; tract++)
        {
          sumcases   = sumcases + ppCases[j][tract];
          summeasure = summeasure + (pNonCumulativeMeasure)[j][tract];
        } /* for tract */
      else
        for (tract=0; tract < gData.m_nTracts; tract++)
        {
          sumcases   = sumcases + (ppCases[j][tract]-ppCases[j+1][tract]);
          summeasure = summeasure + (pNonCumulativeMeasure)[j][tract];
        } /* for tract */

        j++;
    }  /* while */

    while (jj < k*gData.m_nTimeIntervals/AdjustIntervals && jj < gData.m_nTimeIntervals)
    {
      for (tract = 0; tract<gData.m_nTracts; tract++)
        (pNonCumulativeMeasure)[jj][tract] =
          (pNonCumulativeMeasure)[jj][tract]*(sumcases/summeasure)/((thisStream.GetTotalCases())/(thisStream.GetTotalMeasure()));

      jj++;
    }  /* while */

    k++;
  } /* for i<AdjustIntervals */

}

/** */
void CPoissonModel::AdjustForLLPercentage(DataStream & thisStream, measure_t ** pNonCumulativeMeasure, double nPercentage)
{
  int    i,t;
  double c;
  double k = IntervalInYears(gParameters.GetTimeIntervalUnitsType(), gParameters.GetTimeIntervalLength());
  double p = 1 + (nPercentage/100);
  double nAdjustedMeasure = 0;

  #if DEBUGMODEL
  fprintf(m_pDebugModelFile, "\nAdjust Measure for Time Trend - %f%% per year.\n\n", nPercentage);
  #endif

  /* Adjust the measure assigned to each interval/tract by yearly percentage */
  for (i=0; i<gData.m_nTimeIntervals; i++)
    for (t=0; t<gData.m_nTracts; t++)
    {
      (pNonCumulativeMeasure)[i][t] = (pNonCumulativeMeasure)[i][t]*(pow(p,i*k)) /* * c */ ;
      nAdjustedMeasure += (pNonCumulativeMeasure)[i][t];

      if (nAdjustedMeasure > DBL_MAX)
        SSGenerateException("Error: Data overflow due to time trend adjustment.\n","AdjustForLLPercentage()");
    }

  /* Mutlipy the measure for each interval/tract by constant (c) to obtain */
  /* total adjusted measure (nAdjustedMeasure) equal to previous total     */
  /* measure (gData.m_nTotalMeasure).                                             */
  c = (double)(thisStream.GetTotalMeasure())/nAdjustedMeasure;
  for (i=0; i<gData.m_nTimeIntervals; ++i)
    for (t=0; t<gData.m_nTracts; ++t)
     pNonCumulativeMeasure[i][t] *= c;
}

void CPoissonModel::AdjustForLogLinear(DataStream & thisStream, measure_t ** pNonCumulativeMeasure)
{
/*  #if DEBUGMODEL
  fprintf(m_pDebugModelFile, "\nCalculate Time Trend and Adjust for Log Linear\n\n");
  #endif
*/
  // Calculate time trend for whole dataset
  thisStream.GetTimeTrend().CalculateAndSet(thisStream.GetPTCasesArray(), thisStream.GetPTMeasureArray(),
                                            gData.m_nTimeIntervals, gParameters.GetTimeTrendConvergence());

  // Global time trend will be recalculated after the measure is adjusted.
  // Therefore this value will be lost unless retain in parameters for
  // display in the report.
  gParameters.SetTimeTrendAdjustmentPercentage(thisStream.GetTimeTrend().m_nBeta);
  AdjustForLLPercentage(thisStream, pNonCumulativeMeasure, thisStream.GetTimeTrend().m_nBeta);  // Adjust Measure             */
}

void CPoissonModel::AdjustMeasure(DataStream & thisStream, measure_t ** ppNonCumulativeMeasure) {
  measure_t     AdjustedTotalMeasure_t=0;
  int           i;
  tract_t       t;

  try {
    if (!gData.ReadAdjustmentsByRelativeRisksFile(ppNonCumulativeMeasure))
      SSGenerateException("\nProblem encountered reading in data.", "AdjustMeasure()");

    if (gData.m_nTimeIntervals > 1) {
      switch (gParameters.GetTimeTrendAdjustmentType()) {
        case NOTADJUSTED               : break;
        case NONPARAMETRIC             : AdjustForNonParameteric(thisStream, ppNonCumulativeMeasure); break;
        case LOGLINEAR_PERC            : AdjustForLLPercentage(thisStream, ppNonCumulativeMeasure, gParameters.GetTimeTrendAdjustmentPercentage()); break;
        case CALCULATED_LOGLINEAR_PERC : thisStream.SetMeasureByTimeIntervalsArray(ppNonCumulativeMeasure);
                                         AdjustForLogLinear(thisStream, ppNonCumulativeMeasure); break;
        case STRATIFIED_RANDOMIZATION  : AdjustForNonParameteric(thisStream, ppNonCumulativeMeasure); break;//this adjustment occurs during randomization also
        default : ZdGenerateException("Unknown time trend adjustment type: '%d'.",
                                      "AdjustMeasure()", gParameters.GetTimeTrendAdjustmentType());
      }
    }

    // Bug check, to ensure that adjusted  total measure equals previously determined total measure
    for (AdjustedTotalMeasure_t=0, i=0; i < gData.m_nTimeIntervals; ++i)
       for (t=0; t < gData.m_nTracts; ++t)
          AdjustedTotalMeasure_t += ppNonCumulativeMeasure[i][t];

    if (fabs(AdjustedTotalMeasure_t - thisStream.GetTotalMeasure()) > 0.0001)
      ZdGenerateException("Error: The adjusted total measure '%8.6lf' is not equal to the total measure '%8.6lf'.\n",
                          "AdjustMeasure()", AdjustedTotalMeasure_t, thisStream.GetTotalMeasure());
  }
  catch (ZdException &x) {
    x.AddCallpath("AdjustMeasure()","CPoissonModel");
    throw;
  }
}

/** Calculates and sets non-cumulative measure array. */
void CPoissonModel::AssignMeasure(DataStream & thisStream, measure_t ** ppNonCumulativeMeasure) {
  int           i, interval;
  tract_t       tract;
  Julian      * pIntervalDates=0;

  try {
    /* Create & Use array of interval dates where last interval date = EndDate */
    pIntervalDates = (unsigned long*)Smalloc((gData.m_nTimeIntervals+1)*sizeof(Julian), &gPrintDirection);
    memcpy(pIntervalDates, gData.GetTimeIntervalStartTimes(), (gData.m_nTimeIntervals+1)*sizeof(Julian));

#ifdef DEBUGMEASURE
    if ((pMResult = fopen("MEASURE.TXT", "w")) == NULL)
      SSGenerateException("Error: Cannot open output file.\n","AssignMeasure()");

    DisplayInitialData(gData.m_nStartDate, gData.m_nEndDate, pIntervalDates, gData.m_nTimeIntervals, pAlpha, nPops);
#endif
    thisStream.SetTotalMeasure(CalcMeasure(thisStream.GetPopulationData(), ppNonCumulativeMeasure,
                                           thisStream.GetPopulationMeasureArray(),
                                           pIntervalDates, gData.m_nStartDate, gData.m_nEndDate,
                                           gData.m_nTracts, gData.m_nTimeIntervals, &gPrintDirection));

    //Check to see that Measure matrix has positive entries.
#if 1
    for (tract=0; tract < gData.m_nTracts; ++tract)
       for (interval=0; interval < gData.m_nTimeIntervals; ++interval)
          if (ppNonCumulativeMeasure[interval][tract] < 0) {
             std::string sBuffer;
    	     ZdGenerateException("Negative measure: %g\ntract %d, tractid %s, interval %d.", "AssignMeasure()",
                                 ppNonCumulativeMeasure[interval][tract], tract, gData.GetTInfo()->tiGetTid(tract, sBuffer), interval);
          }
#endif

#ifdef DEBUGMEASURE
    fprintf(pMResult, "Totals: \n\n");
    fprintf(pMResult, "   Cases      = %li\n   Measure    = %f\n   Population = %f\n", *pTotalCases, *pTotalMeasure, *pTotalPop);
    fclose(pMResult);
#endif

    free(pIntervalDates);
  }
  catch (ZdException & x) {
    free(pIntervalDates);
    x.AddCallpath("AssignMeasure()", "CPoissonModel");
    throw;
  }
}

bool CPoissonModel::CalculateMeasure(DataStream & thisStream) {
  bool                  bResult=true;
  double              * pAlpha=0, * pRisk=0;
  PopulationData      & Population = thisStream.GetPopulationData();

  try {
    //calculate alpha - an array that indicates each population interval's percentage of study period.
    Population.CalculateAlpha(&pAlpha, gData.m_nStartDate, gData.m_nEndDate);
    //calculate risk for each population category
    CalcRisk(thisStream, &pRisk, pAlpha, gData.GetTInfo()->tiGetNumTracts(), &gPrintDirection);
    //allocate 2D array of population dates by number of tracts
    thisStream.AllocatePopulationMeasureArray();
    //calculate expected number of cases in terms of population dates
    Calcm(Population, thisStream.GetPopulationMeasureArray(), pRisk,
          Population.GetNumPopulationCategories(), gData.GetTInfo()->tiGetNumTracts(),
          Population.GetNumPopulationDates(), &gPrintDirection);

    //assign measure, perform adjustments as requested, and set measure array as cumulative
    if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
      thisStream.AllocateNCMeasureArray();
      //calculate non-cumulative measure array
      AssignMeasure(thisStream, thisStream.GetNCMeasureArray());
      //validate that observed and expected agree
      gData.ValidateObservedToExpectedCases(thisStream.GetCaseArray(),
                                            thisStream.GetNCMeasureArray());
      //apply adjustments
      AdjustMeasure(thisStream, thisStream.GetNCMeasureArray());
      //create cumulative measure from non-cumulative measure
      thisStream.SetCumulativeMeasureArrayFromNonCumulative();
      //either reset or set measure by time intervals with non-cumulative measure
      thisStream.SetMeasureByTimeIntervalsArray(thisStream.GetNCMeasureArray());
    }
    else {
      thisStream.AllocateMeasureArray();
      //calculate non-cumulative measure array
      AssignMeasure(thisStream, thisStream.GetMeasureArray());
      //validate that observed and expected agree
      gData.ValidateObservedToExpectedCases(thisStream.GetCaseArray(),
                                            thisStream.GetMeasureArray());
      //apply adjustments
      AdjustMeasure(thisStream, thisStream.GetMeasureArray());
      if (gParameters.GetTimeTrendAdjustmentType() == STRATIFIED_RANDOMIZATION ||
          gParameters.GetTimeTrendAdjustmentType() == CALCULATED_LOGLINEAR_PERC)
        //need measure by time intervals for time stratified adjustment in simulations
        thisStream.SetMeasureByTimeIntervalsArray(thisStream.GetMeasureArray());
      thisStream.SetMeasureArrayAsCumulative();
    }
    // Bug check, to ensure that TotalCases=TotalMeasure
    if (fabs(thisStream.GetTotalCases() - thisStream.GetTotalMeasure()) > 0.0001)
      ZdGenerateException("Error: The total measure '%8.6lf' is not equal to the total number of cases '%ld'.\n",
                          "CalculateMeasure()", thisStream.GetTotalMeasure(), thisStream.GetTotalCases());

    free(pAlpha); pAlpha=0;
    free(pRisk); pRisk=0;
    thisStream.FreePopulationMeasureArray();
  }
  catch (ZdException &x) {
    free(pAlpha); pAlpha=0;
    free(pRisk); pRisk=0;
    thisStream.FreePopulationMeasureArray();
    x.AddCallpath("CalculateMeasure()","CPoissonModel");
    throw;
  }
  return bResult;
}

double CPoissonModel::GetPopulation(int m_iEllipseOffset, tract_t nCenter, tract_t nTracts,
                                    int nStartInterval, int nStopInterval) {
  tract_t T, t;
  int     c, n;
  double* pAlpha = 0;
  PopulationData & Population = gData.GetDataStreamHandler().GetStream(0/*for now*/).GetPopulationData();
  int     ncats;
  int     nPops;
  double  nPopulation = 0.0;

  try {
    ncats = Population.GetNumPopulationCategories();
    nPops = Population.GetNumPopulationDates();
    Population.CalculateAlpha(&pAlpha, gData.m_nStartDate, gData.m_nEndDate);

      for (T = 1; T <= nTracts; T++)
      {
         t = gData.GetNeighbor(m_iEllipseOffset, nCenter, T);
         for (c = 0; c < ncats; c++)
            Population.GetAlphaAdjustedPopulation(nPopulation, t, c, 0, nPops, pAlpha);
      }

      free(pAlpha); pAlpha = 0;
      }
   catch (ZdException & x)
      {
      free(pAlpha);
      x.AddCallpath("GetPopulation()", "CPoissonModel");
      throw;
      }
   return nPopulation;
}

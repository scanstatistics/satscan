#include "SaTScan.h"
#pragma hdrstop
#include "PoissonModel.h"

/** constructor */
CPoissonModel::CPoissonModel(const CParameters& Parameters, CSaTScanData& Data, BasePrint& PrintDirection)
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
void CPoissonModel::AdjustForNonParameteric(RealDataStream & thisStream, measure_t ** pNonCumulativeMeasure) {
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

/** Adjusts passed non-cumulative measure given passed log linear percentage. */
void CPoissonModel::AdjustForLLPercentage(RealDataStream & thisStream, measure_t ** pNonCumulativeMeasure, double nPercentage)
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

/** Calculates time trend for data stream, calls CParameters::SetTimeTrendAdjustmentPercentage()
    with calculated value, and calls AdjustForLLPercentage(). */
void CPoissonModel::AdjustForLogLinear(RealDataStream& thisStream, measure_t ** pNonCumulativeMeasure) {
  CTimeTrend    TimeTrend;

  //Calculate time trend for whole dataset
  TimeTrend.CalculateAndSet(thisStream.GetPTCasesArray(), thisStream.GetPTMeasureArray(),
                            gData.GetNumTimeIntervals(), gParameters.GetTimeTrendConvergence());

  //Cancel analysis execution if calculation of time trend fails for various reasons.
  switch (TimeTrend.GetStatus()) {
    case CTimeTrend::TREND_UNDEF :
      SSGenerateException("Note: Temporal adjustment could not be performed. The calculated time trend is undefined.\n"
                          "      Please run analysis without automatic adjustment of time trends.","AdjustForLogLinear()");
    case CTimeTrend::TREND_INF :
      SSGenerateException("Note: Temporal adjustment could not be performed. The calculated time trend is infinite.\n"
                          "      Please run analysis without automatic adjustment of time trends.","AdjustForLogLinear()");
    case CTimeTrend::TREND_NOTCONVERGED :
      SSGenerateException("Note: Temporal adjustment could not be performed. The time trend does not converge.\n"
                          "      Please run analysis without automatic adjustment of time trends.","AdjustForLogLinear()");
    case CTimeTrend::TREND_NEGATIVE :
      SSGenerateException("Note: Temporal adjustment could not be performed. The calculated time trend is negative.\n"
                          "      Please run analysis without automatic adjustment of time trends.","AdjustForLogLinear()");
    case CTimeTrend::TREND_CONVERGED : break;
    default :
      ZdGenerateException("Unknown time trend status type '%d'.", "AdjustForLogLinear()", TimeTrend.GetStatus());
  };

  TimeTrend.SetAnnualTimeTrend(gParameters.GetTimeIntervalUnitsType(), gParameters.GetTimeIntervalLength());
  AdjustForLLPercentage(thisStream, pNonCumulativeMeasure, TimeTrend.GetAnnualTimeTrend());
  //store calculated time trend adjustment for reporting later
  thisStream.SetCalculatedTimeTrendPercentage(TimeTrend.GetAnnualTimeTrend());
}

/** Adjusts passed non-cumulative measure for parameter specified temporal,
    spatial, and space-time adjustments.                                      */
void CPoissonModel::AdjustMeasure(RealDataStream& thisStream, measure_t** ppNonCumulativeMeasure) {
  measure_t     AdjustedTotalMeasure_t=0;
  int           i;
  tract_t       t;

  try {
    //adjust measure for known realtive risks
    if (gParameters.UseAdjustmentForRelativeRisksFile())
      gData.AdjustForKnownRelativeRisks(thisStream, ppNonCumulativeMeasure);
    //adjustment measure temporally
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
    //adjust measure spatially
    switch (gParameters.GetSpatialAdjustmentType()) {
      case NO_SPATIAL_ADJUSTMENT : break;
      case SPATIALLY_STRATIFIED_RANDOMIZATION : StratifiedSpatialAdjustment(thisStream, ppNonCumulativeMeasure); break;
      default : ZdGenerateException("Unknown spatial adjustment type: '%d'.",
                                    "AdjustMeasure()", gParameters.GetSpatialAdjustmentType());
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

/** Calculates and sets non-cumulative measure array. Validates that no expected
    case counts are negative, throws ZdException. */
void CPoissonModel::AssignMeasure(RealDataStream & thisStream, TwoDimMeasureArray_t& NonCumulativeMeasureHandler) {
  try {
    thisStream.SetTotalMeasure(CalcMeasure(thisStream,
                                           NonCumulativeMeasureHandler,
                                           gData.GetTimeIntervalStartTimes(),
                                           gData.m_nStartDate, gData.m_nEndDate));

    //Validate that all elements of measure array are not negative.  
    measure_t** ppNonCumulativeMeasure = NonCumulativeMeasureHandler.GetArray();
    for (tract_t tract=0; tract < gData.m_nTracts; ++tract)
       for (int interval=0; interval < gData.m_nTimeIntervals; ++interval)
          if (ppNonCumulativeMeasure[interval][tract] < 0) {
             std::string sBuffer;
    	     ZdGenerateException("Negative measure: %g\ntract %d, tractid %s, interval %d.", "AssignMeasure()",
                                 ppNonCumulativeMeasure[interval][tract], tract, gData.GetTInfo()->tiGetTid(tract, sBuffer), interval);
          }

  }
  catch (ZdException &x) {
    x.AddCallpath("AssignMeasure()","CPoissonModel");
    throw;
  }
}

/** Calculates the expected number of cases for data stream. */
void CPoissonModel::CalculateMeasure(RealDataStream& thisStream) {
  try {
    //calculate expected number of cases in terms of population dates
    Calcm(thisStream, gData.m_nStartDate, gData.m_nEndDate);
    //assign measure, perform adjustments as requested, and set measure array as cumulative
    if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
      thisStream.AllocateNCMeasureArray();
      //calculate non-cumulative measure array
      AssignMeasure(thisStream, thisStream.GetNCMeasureArrayHandler());
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
      AssignMeasure(thisStream, thisStream.GetMeasureArrayHandler());
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

    thisStream.FreePopulationMeasureArray();
  }
  catch (ZdException &x) {
    thisStream.FreePopulationMeasureArray();
    x.AddCallpath("CalculateMeasure()","CPoissonModel");
    throw;
  }
}

/** */
double CPoissonModel::GetPopulation(unsigned int iStream, int m_iEllipseOffset, tract_t nCenter,
                                    tract_t nTracts, int nStartInterval, int nStopInterval) const {
  tract_t T, t;
  int     c, n;
  std::vector<double>   vAlpha;
  const PopulationData & Population = gData.GetDataStreamHandler().GetStream(iStream).GetPopulationData();
  int     ncats;
  int     nPops;
  double  nPopulation = 0.0;

  try {
    ncats = Population.GetNumPopulationCategories();
    nPops = Population.GetNumPopulationDates();
    Population.CalculateAlpha(vAlpha, gData.m_nStartDate, gData.m_nEndDate);

      for (T = 1; T <= nTracts; T++)
      {
         t = gData.GetNeighbor(m_iEllipseOffset, nCenter, T);
         for (c = 0; c < ncats; c++)
            Population.GetAlphaAdjustedPopulation(nPopulation, t, c, 0, nPops, vAlpha);
      }

      }
   catch (ZdException & x)
      {
      x.AddCallpath("GetPopulation()", "CPoissonModel");
      throw;
      }
   return nPopulation;
}

/** Adjusts passed non-cumulative measure in a stratified spatial manner.
    Each time, for a particular tract, is multiplied by the total case divided
    by total measure of that tract, across all time intervals. */
void CPoissonModel::StratifiedSpatialAdjustment(RealDataStream& thisStream, measure_t ** ppNonCumulativeMeasure) {
  measure_t     tTotalTractMeasure;
  count_t    ** ppCases = thisStream.GetCaseArray();
  tract_t       t;
  int           i;
  double        dTractAdjustment;


  for (t=0; t < gData.GetNumTracts(); ++t) {
     //calculates total measure for current tract across all time intervals
     for (tTotalTractMeasure=0, i=0; i < gData.GetNumTimeIntervals(); ++i)
        tTotalTractMeasure += ppNonCumulativeMeasure[i][t];
     if (tTotalTractMeasure) {
       dTractAdjustment = ppCases[0][t]/tTotalTractMeasure;
       //now multiply each time interval/tract location by (total cases)/(total measure)
       for (i=0; tTotalTractMeasure && i < gData.GetNumTimeIntervals(); ++i)
          ppNonCumulativeMeasure[i][t] *= dTractAdjustment;
     }
  }
}


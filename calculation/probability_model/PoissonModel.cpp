//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "PoissonModel.h"
#include "SSException.h"

const double CPoissonModel::gTimeTrendConvergence = 0.0000001;

/** constructor */
CPoissonModel::CPoissonModel(CSaTScanData& DataHub)
              :CModel(), gDataHub(DataHub), gParameters(DataHub.GetParameters()) {}

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
void CPoissonModel::AdjustForNonParameteric(RealDataSet& DataSet, measure_t ** pNonCumulativeMeasure) {
  unsigned int          i, j, jj, k, tract, AdjustIntervals;
  double                sumcases,summeasure;
  count_t            ** ppCases(DataSet.GetCaseArray());

  AdjustIntervals = DataSet.GetNumTimeIntervals();
  k  = 1;
  j  = 0;
  jj = 0;

  for (i=0; i<AdjustIntervals; i++)
  {
	 sumcases   = 0;
	 summeasure = 0;
    while (j < k*DataSet.GetNumTimeIntervals()/AdjustIntervals && j < DataSet.GetNumTimeIntervals())
    {
      if (j == DataSet.GetNumTimeIntervals()-1)
        for (tract=0; tract < DataSet.GetNumTracts(); tract++)
        {
          sumcases   = sumcases + ppCases[j][tract];
          summeasure = summeasure + (pNonCumulativeMeasure)[j][tract];
        } /* for tract */
      else
        for (tract=0; tract < DataSet.GetNumTracts(); tract++)
        {
          sumcases   = sumcases + (ppCases[j][tract]-ppCases[j+1][tract]);
          summeasure = summeasure + (pNonCumulativeMeasure)[j][tract];
        } /* for tract */

        j++;
    }  /* while */

    while (jj < k*DataSet.GetNumTimeIntervals()/AdjustIntervals && jj < DataSet.GetNumTimeIntervals())
    {
      for (tract = 0; tract < DataSet.GetNumTracts(); tract++)
        (pNonCumulativeMeasure)[jj][tract] =
          (pNonCumulativeMeasure)[jj][tract]*(sumcases/summeasure)/((DataSet.GetTotalCases())/(DataSet.GetTotalMeasure()));

      jj++;
    }  /* while */

    k++;
  } /* for i<AdjustIntervals */

}

/** Adjusts passed non-cumulative measure given passed log linear percentage. */
void CPoissonModel::AdjustForLLPercentage(RealDataSet& DataSet, measure_t ** ppNonCumulativeMeasure, double nPercentage)
{
  unsigned int    i,t;
  double c;
  double k = IntervalInYears(gParameters.GetTimeAggregationUnitsType(), gParameters.GetTimeAggregationLength());
  double p = 1 + (nPercentage/100);
  double nAdjustedMeasure = 0;

  #if DEBUGMODEL
  fprintf(m_pDebugModelFile, "\nAdjust Measure for Time Trend - %f%% per year.\n\n", nPercentage);
  #endif

  /* Adjust the measure assigned to each interval/tract by yearly percentage */
  for (i=0; i < DataSet.GetNumTimeIntervals(); ++i)
    for (t=0; t < DataSet.GetNumTracts(); ++t) {
      ppNonCumulativeMeasure[i][t] = ppNonCumulativeMeasure[i][t]*(pow(p,i*k)) /* * c */ ;
      if (nAdjustedMeasure > std::numeric_limits<measure_t>::max() - ppNonCumulativeMeasure[i][t])
        GenerateResolvableException("Error: Data overflow occurs when performing the time trend adjustment in data set %u.\n"
                                    "       Please run analysis without the time trend adjustment.\n",
                                    "AdjustForLLPercentage()", DataSet.GetSetIndex());
      nAdjustedMeasure += ppNonCumulativeMeasure[i][t];
    }

  /* Mutlipy the measure for each interval/tract by constant (c) to obtain */
  /* total adjusted measure (nAdjustedMeasure) equal to previous total     */
  /* measure (gData.m_nTotalMeasure).                                             */
  c = (double)(DataSet.GetTotalMeasure())/nAdjustedMeasure;
  for (i=0; i < DataSet.GetNumTimeIntervals(); ++i)
    for (t=0; t < DataSet.GetNumTracts(); ++t)
     ppNonCumulativeMeasure[i][t] *= c;
}

/** Calculates time trend for dataset, calls CParameters::SetTimeTrendAdjustmentPercentage()
    with calculated value, and calls AdjustForLLPercentage(). */
void CPoissonModel::AdjustForLogLinear(RealDataSet& DataSet, measure_t ** pNonCumulativeMeasure) {
  CTimeTrend    TimeTrend;

  //Calculate time trend for whole dataset
  TimeTrend.CalculateAndSet(DataSet.GetCasesPerTimeIntervalArray(), DataSet.GetMeasurePerTimeIntervalArray(),
                            DataSet.GetNumTimeIntervals(), gTimeTrendConvergence);

  //Cancel analysis execution if calculation of time trend fails for various reasons.
  switch (TimeTrend.GetStatus()) {
    case CTimeTrend::TREND_UNDEF :
      GenerateResolvableException("Note: The time trend is undefined and the temporal adjustment could not be performed.\n"
                                  "      Please run analysis without automatic adjustment of time trends.","AdjustForLogLinear()");
    case CTimeTrend::TREND_INF :
      GenerateResolvableException("Note: The time trend is infinite and the temporal adjustment could not be performed.\n"
                                  "      Please run analysis without automatic adjustment of time trends.","AdjustForLogLinear()");
    case CTimeTrend::TREND_NOTCONVERGED :
      GenerateResolvableException("Note: The time trend calculation did not converge and the temporal adjustment could not be performed.\n"
                                  "      Please run analysis without automatic adjustment of time trends.","AdjustForLogLinear()");
    case CTimeTrend::TREND_NEGATIVE :
      GenerateResolvableException("Note: Temporal adjustment could not be performed. The calculated time trend is negative.\n"
                                  "      Please run analysis without automatic adjustment of time trends.","AdjustForLogLinear()");
    case CTimeTrend::TREND_CONVERGED : break;
    default :
      ZdGenerateException("Unknown time trend status type '%d'.", "AdjustForLogLinear()", TimeTrend.GetStatus());
  };

  TimeTrend.SetAnnualTimeTrend(gParameters.GetTimeAggregationUnitsType(), gParameters.GetTimeAggregationLength());
  AdjustForLLPercentage(DataSet, pNonCumulativeMeasure, TimeTrend.GetAnnualTimeTrend());
  //store calculated time trend adjustment for reporting later
  DataSet.SetCalculatedTimeTrendPercentage(TimeTrend.GetAnnualTimeTrend());
}

/** Adjusts passed non-cumulative measure for parameter specified temporal,
    spatial, and space-time adjustments.                                      */
void CPoissonModel::AdjustMeasure(RealDataSet& DataSet, measure_t** ppNonCumulativeMeasure) {
  measure_t     AdjustedTotalMeasure_t=0;
  unsigned int  i, t;

  try {
    //adjust measure for known realtive risks
    if (gParameters.UseAdjustmentForRelativeRisksFile())
      gDataHub.AdjustForKnownRelativeRisks(DataSet, ppNonCumulativeMeasure);
    //adjustment measure temporally
    if (DataSet.GetNumTimeIntervals() > 1) {
      switch (gParameters.GetTimeTrendAdjustmentType()) {
        case NOTADJUSTED               : break;
        case NONPARAMETRIC             : AdjustForNonParameteric(DataSet, ppNonCumulativeMeasure); break;
        case LOGLINEAR_PERC            : AdjustForLLPercentage(DataSet, ppNonCumulativeMeasure, gParameters.GetTimeTrendAdjustmentPercentage()); break;
        case CALCULATED_LOGLINEAR_PERC : DataSet.SetMeasurePerTimeIntervalsArray(ppNonCumulativeMeasure);
                                         AdjustForLogLinear(DataSet, ppNonCumulativeMeasure); break;
        case STRATIFIED_RANDOMIZATION  : AdjustForNonParameteric(DataSet, ppNonCumulativeMeasure); break;//this adjustment occurs during randomization also
        default : ZdGenerateException("Unknown time trend adjustment type: '%d'.",
                                      "AdjustMeasure()", gParameters.GetTimeTrendAdjustmentType());
      }
    }
    //adjust measure spatially
    switch (gParameters.GetSpatialAdjustmentType()) {
      case NO_SPATIAL_ADJUSTMENT : break;
      case SPATIALLY_STRATIFIED_RANDOMIZATION : StratifiedSpatialAdjustment(DataSet, ppNonCumulativeMeasure); break;
      default : ZdGenerateException("Unknown spatial adjustment type: '%d'.",
                                    "AdjustMeasure()", gParameters.GetSpatialAdjustmentType());
    }
    // Bug check, to ensure that adjusted  total measure equals previously determined total measure
    for (AdjustedTotalMeasure_t=0, i=0; i < DataSet.GetNumTimeIntervals(); ++i)
       for (t=0; t < DataSet.GetNumTracts(); ++t)
          AdjustedTotalMeasure_t += ppNonCumulativeMeasure[i][t];
    if (fabs(AdjustedTotalMeasure_t - DataSet.GetTotalMeasure()) > 0.0001)
      ZdGenerateException("Error: The adjusted total measure '%8.6lf' is not equal to the total measure '%8.6lf'.\n",
                          "AdjustMeasure()", AdjustedTotalMeasure_t, DataSet.GetTotalMeasure());
  }
  catch (ZdException &x) {
    x.AddCallpath("AdjustMeasure()","CPoissonModel");
    throw;
  }
}

/** Calculates and sets non-cumulative measure array. Validates that no expected
    case counts are negative, throws ZdException. */
void CPoissonModel::AssignMeasure(RealDataSet& DataSet, TwoDimMeasureArray_t& NonCumulativeMeasureHandler) {
  try {
    DataSet.SetTotalMeasure(CalcMeasure(DataSet, NonCumulativeMeasureHandler, gDataHub.GetTimeIntervalStartTimes(),
                                        gDataHub.GetStudyPeriodStartDate(), gDataHub.GetStudyPeriodEndDate()));

    //Validate that all elements of measure array are not negative.  
    measure_t** ppNonCumulativeMeasure = NonCumulativeMeasureHandler.GetArray();
    for (unsigned int tract=0; tract < DataSet.GetNumTracts(); ++tract)
       for (unsigned int interval=0; interval < DataSet.GetNumTimeIntervals(); ++interval)
          if (ppNonCumulativeMeasure[interval][tract] < 0) {
    	     ZdGenerateException("Negative measure: %g\ntract %d, tractid %s, interval %d.", "AssignMeasure()",
                                 ppNonCumulativeMeasure[interval][tract], tract,
                                 gDataHub.GetTInfo()->getLocations().at(tract)->getIndentifier(), interval);
          }

  }
  catch (ZdException &x) {
    x.AddCallpath("AssignMeasure()","CPoissonModel");
    throw;
  }
}

/** Calculates the expected number of cases for dataset. */
void CPoissonModel::CalculateMeasure(RealDataSet& DataSet) {
  try {
    //calculate expected number of cases in terms of population dates
    Calcm(DataSet, gDataHub.GetStudyPeriodStartDate(), gDataHub.GetStudyPeriodEndDate());
    //assign measure, perform adjustments as requested, and set measure array as cumulative
    if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
      DataSet.AllocateNCMeasureArray();
      //calculate non-cumulative measure array
      AssignMeasure(DataSet, DataSet.GetNCMeasureArrayHandler());
      //validate that observed and expected agree
      gDataHub.ValidateObservedToExpectedCases(DataSet.GetCaseArray(), DataSet.GetNCMeasureArray());
      //apply adjustments
      AdjustMeasure(DataSet, DataSet.GetNCMeasureArray());
      //create cumulative measure from non-cumulative measure
      DataSet.SetCumulativeMeasureArrayFromNonCumulative();
      //either reset or set measure by time intervals with non-cumulative measure
      DataSet.SetMeasurePerTimeIntervalsArray(DataSet.GetNCMeasureArray());
    }
    else {
      DataSet.AllocateMeasureArray();
      //calculate non-cumulative measure array
      AssignMeasure(DataSet, DataSet.GetMeasureArrayHandler());
      //validate that observed and expected agree
      gDataHub.ValidateObservedToExpectedCases(DataSet.GetCaseArray(),
                                            DataSet.GetMeasureArray());
      //apply adjustments
      AdjustMeasure(DataSet, DataSet.GetMeasureArray());
      if (gParameters.GetTimeTrendAdjustmentType() == STRATIFIED_RANDOMIZATION ||
          gParameters.GetTimeTrendAdjustmentType() == CALCULATED_LOGLINEAR_PERC)
        //need measure by time intervals for time stratified adjustment in simulations
        DataSet.SetMeasurePerTimeIntervalsArray(DataSet.GetMeasureArray());
      DataSet.SetMeasureArrayAsCumulative();
    }
    // Bug check, to ensure that TotalCases=TotalMeasure
    if (fabs(DataSet.GetTotalCases() - DataSet.GetTotalMeasure()) > 0.0001)
      ZdGenerateException("Error: The total measure '%8.6lf' is not equal to the total number of cases '%ld'.\n",
                          "CalculateMeasure()", DataSet.GetTotalMeasure(), DataSet.GetTotalCases());

    DataSet.FreePopulationMeasureArray();
  }
  catch (ZdException &x) {
    DataSet.FreePopulationMeasureArray();
    x.AddCallpath("CalculateMeasure()","CPoissonModel");
    throw;
  }
}

/** Returns population as defined in CCluster object. */
double CPoissonModel::GetPopulation(size_t tSetIndex, const CCluster& Cluster, const CSaTScanData& DataHub) const {
  tract_t               T, t;
  int                   c, ncats, nPops;
  std::vector<double>   vAlpha;
  const PopulationData& Population = DataHub.GetDataSetHandler().GetDataSet(tSetIndex).GetPopulationData();
  double                nPopulation = 0.0;

  try {
    ncats = Population.GetNumCovariateCategories();
    nPops = Population.GetNumPopulationDates();
    Population.CalculateAlpha(vAlpha, DataHub.GetStudyPeriodStartDate(), DataHub.GetStudyPeriodEndDate());

      for (T = 1; T <= Cluster.GetNumTractsInCluster(); T++)
      {
         t = DataHub.GetNeighbor(Cluster.GetEllipseOffset(), Cluster.GetCentroidIndex(), T, Cluster.GetCartesianRadius());
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
void CPoissonModel::StratifiedSpatialAdjustment(RealDataSet& DataSet, measure_t ** ppNonCumulativeMeasure) {
  measure_t     tTotalTractMeasure;
  count_t    ** ppCases = DataSet.GetCaseArray();
  unsigned int  i, t;
  double        dTractAdjustment;


  for (t=0; t < DataSet.GetNumTracts(); ++t) {
     //calculates total measure for current tract across all time intervals
     for (tTotalTractMeasure=0, i=0; i < DataSet.GetNumTimeIntervals(); ++i)
        tTotalTractMeasure += ppNonCumulativeMeasure[i][t];
     if (tTotalTractMeasure) {
       dTractAdjustment = ppCases[0][t]/tTotalTractMeasure;
       //now multiply each time interval/tract location by (total cases)/(total measure)
       for (i=0; tTotalTractMeasure && i < DataSet.GetNumTimeIntervals(); ++i)
          ppNonCumulativeMeasure[i][t] *= dTractAdjustment;
     }
  }
}


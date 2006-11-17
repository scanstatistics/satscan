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
void CPoissonModel::AdjustForNonParameteric(RealDataSet& DataSet) {
  unsigned int          i, j, jj, k, tract, AdjustIntervals = DataSet.getIntervalDimension();
  double                sumcases,summeasure;
  count_t            ** ppCases = DataSet.getCaseData().GetArray();
  measure_t          ** pNonCumulativeMeasure = DataSet.getMeasureData().GetArray();

  k  = 1;
  j  = 0;
  jj = 0;

  for (i=0; i<AdjustIntervals; i++)
  {
	 sumcases   = 0;
	 summeasure = 0;
    while (j < k*DataSet.getIntervalDimension()/AdjustIntervals && j < DataSet.getIntervalDimension())
    {
      if (j == DataSet.getIntervalDimension()-1)
        for (tract=0; tract < DataSet.getLocationDimension(); tract++)
        {
          sumcases   = sumcases + ppCases[j][tract];
          summeasure = summeasure + (pNonCumulativeMeasure)[j][tract];
        } /* for tract */
      else
        for (tract=0; tract < DataSet.getLocationDimension(); tract++)
        {
          sumcases   = sumcases + (ppCases[j][tract]-ppCases[j+1][tract]);
          summeasure = summeasure + (pNonCumulativeMeasure)[j][tract];
        } /* for tract */

        j++;
    }  /* while */

    while (jj < k*DataSet.getIntervalDimension()/AdjustIntervals && jj < DataSet.getIntervalDimension())
    {
      for (tract = 0; tract < DataSet.getLocationDimension(); tract++)
        (pNonCumulativeMeasure)[jj][tract] =
          (pNonCumulativeMeasure)[jj][tract]*(sumcases/summeasure)/((DataSet.getTotalCases())/(DataSet.getTotalMeasure()));

      jj++;
    }  /* while */

    k++;
  } /* for i<AdjustIntervals */

}

/** Adjusts passed non-cumulative measure given passed log linear percentage. */
void CPoissonModel::AdjustForLLPercentage(RealDataSet& DataSet, double nPercentage)
{
  unsigned int    i,t;
  double c;
  double k = IntervalInYears(gParameters.GetTimeAggregationUnitsType(), gParameters.GetTimeAggregationLength());
  double p = 1 + (nPercentage/100);
  double nAdjustedMeasure = 0;
  measure_t ** ppNonCumulativeMeasure = DataSet.getMeasureData().GetArray();

  #if DEBUGMODEL
  fprintf(m_pDebugModelFile, "\nAdjust Measure for Time Trend - %f%% per year.\n\n", nPercentage);
  #endif

  /* Adjust the measure assigned to each interval/tract by yearly percentage */
  for (i=0; i < DataSet.getIntervalDimension(); ++i)
    for (t=0; t < DataSet.getLocationDimension(); ++t) {
      ppNonCumulativeMeasure[i][t] = ppNonCumulativeMeasure[i][t]*(pow(p,i*k)) /* * c */ ;
      if (nAdjustedMeasure > std::numeric_limits<measure_t>::max() - ppNonCumulativeMeasure[i][t])
        throw resolvable_error("Error: Data overflow occurs when performing the time trend adjustment in data set %u.\n"
                               "       Please run analysis without the time trend adjustment.\n", DataSet.getSetIndex());
      nAdjustedMeasure += ppNonCumulativeMeasure[i][t];
    }

  /* Mutlipy the measure for each interval/tract by constant (c) to obtain */
  /* total adjusted measure (nAdjustedMeasure) equal to previous total     */
  /* measure (gData.m_nTotalMeasure).                                             */
  c = (double)(DataSet.getTotalMeasure())/nAdjustedMeasure;
  for (i=0; i < DataSet.getIntervalDimension(); ++i)
    for (t=0; t < DataSet.getLocationDimension(); ++t)
     ppNonCumulativeMeasure[i][t] *= c;
}

/** Calculates time trend for dataset, calls CParameters::SetTimeTrendAdjustmentPercentage()
    with calculated value, and calls AdjustForLLPercentage(). */
void CPoissonModel::AdjustForLogLinear(RealDataSet& Set) {
  //purely temporal, not-cumulative measure data not available yet in RealDataSet object, so create temporary.
  std::vector<measure_t> vMeasure_PT_NC(Set.getIntervalDimension());
  measure_t ** ppMeasure = Set.getMeasureData().GetArray();
  for (unsigned int i=0; i < Set.getIntervalDimension(); ++i)
     for (unsigned int j=0; j < Set.getLocationDimension(); ++j)
        vMeasure_PT_NC[i] += ppMeasure[i][j];

  //Calculate time trend for whole dataset
  CTimeTrend TimeTrend;
  TimeTrend.CalculateAndSet(Set.getCaseData_PT_NC(), &vMeasure_PT_NC[0], Set.getIntervalDimension(), gTimeTrendConvergence);

  //Cancel analysis execution if calculation of time trend fails for various reasons.
  switch (TimeTrend.GetStatus()) {
    case CTimeTrend::TREND_UNDEF :
      throw resolvable_error("Note: The time trend is undefined and the temporal adjustment could not be performed.\n"
                             "      Please run analysis without automatic adjustment of time trends.");
    case CTimeTrend::TREND_INF_BEGIN :
    case CTimeTrend::TREND_INF_END :
      throw resolvable_error("Note: The time trend is infinite and the temporal adjustment could not be performed.\n"
                             "      Please run analysis without automatic adjustment of time trends.");
    case CTimeTrend::TREND_NOTCONVERGED :
      throw resolvable_error("Note: The time trend calculation did not converge and the temporal adjustment could not be performed.\n"
                             "      Please run analysis without automatic adjustment of time trends.");
    case CTimeTrend::TREND_CONVERGED : break;
    default : throw prg_error("Unknown time trend status type '%d'.", "AdjustForLogLinear()", TimeTrend.GetStatus());
  };

  TimeTrend.SetAnnualTimeTrend(gParameters.GetTimeAggregationUnitsType(), gParameters.GetTimeAggregationLength());
  AdjustForLLPercentage(Set, TimeTrend.GetAnnualTimeTrend());
  //store calculated time trend adjustment for reporting later
  Set.setCalculatedTimeTrendPercentage(TimeTrend.GetAnnualTimeTrend());
}

/** Adjusts passed non-cumulative measure for parameter specified temporal,
    spatial, and space-time adjustments.                                      */
void CPoissonModel::AdjustMeasure(RealDataSet& DataSet, const TwoDimMeasureArray_t& PopMeasure) {
  measure_t     AdjustedTotalMeasure_t=0, ** ppNonCumulativeMeasure=DataSet.getMeasureData().GetArray();
  unsigned int  i, t;

  try {
    //adjust measure for known realtive risks
    if (gParameters.UseAdjustmentForRelativeRisksFile())
      gDataHub.AdjustForKnownRelativeRisks(DataSet, PopMeasure);
    //adjustment measure temporally
    if (DataSet.getIntervalDimension() > 1) {
      switch (gParameters.GetTimeTrendAdjustmentType()) {
        case NOTADJUSTED               : break;
        case NONPARAMETRIC             : AdjustForNonParameteric(DataSet); break;
        case LOGLINEAR_PERC            : AdjustForLLPercentage(DataSet, gParameters.GetTimeTrendAdjustmentPercentage()); break;
        case CALCULATED_LOGLINEAR_PERC : AdjustForLogLinear(DataSet); break;
        case STRATIFIED_RANDOMIZATION  : AdjustForNonParameteric(DataSet); break;//this adjustment occurs during randomization also
        default : throw prg_error("Unknown time trend adjustment type: '%d'.",
                                  "AdjustMeasure()", gParameters.GetTimeTrendAdjustmentType());
      }
    }
    //adjust measure spatially
    switch (gParameters.GetSpatialAdjustmentType()) {
      case NO_SPATIAL_ADJUSTMENT : break;
      case SPATIALLY_STRATIFIED_RANDOMIZATION : StratifiedSpatialAdjustment(DataSet); break;
      default : throw prg_error("Unknown spatial adjustment type: '%d'.",
                                "AdjustMeasure()", gParameters.GetSpatialAdjustmentType());
    }
    // Bug check, to ensure that adjusted  total measure equals previously determined total measure
    for (AdjustedTotalMeasure_t=0, i=0; i < DataSet.getIntervalDimension(); ++i)
       for (t=0; t < DataSet.getLocationDimension(); ++t)
          AdjustedTotalMeasure_t += ppNonCumulativeMeasure[i][t];
    if (fabs(AdjustedTotalMeasure_t - DataSet.getTotalMeasure()) > 0.0001)
      throw prg_error("Error: The adjusted total measure '%8.6lf' is not equal to the total measure '%8.6lf'.\n",
                      "AdjustMeasure()", AdjustedTotalMeasure_t, DataSet.getTotalMeasure());
  }
  catch (prg_exception &x) {
    x.addTrace("AdjustMeasure()","CPoissonModel");
    throw;
  }
}

/** Calculates the expected number of cases for dataset. */
void CPoissonModel::CalculateMeasure(RealDataSet& Set) {
  try {
    boost::shared_ptr<TwoDimMeasureArray_t> pPopMeasure = Calcm(Set, gDataHub.GetStudyPeriodStartDate(), gDataHub.GetStudyPeriodEndDate());
    CalcMeasure(Set, *pPopMeasure, gDataHub.GetTimeIntervalStartTimes(), gDataHub.GetStudyPeriodStartDate(), gDataHub.GetStudyPeriodEndDate());
    measure_t** ppM = Set.getMeasureData().GetArray(); // validate that all elements of measure array are not negative
    for (unsigned int t=0; t < Set.getLocationDimension(); ++t) for (unsigned int i=0; i < Set.getIntervalDimension(); ++i)
      if (ppM[i][t] < 0) throw prg_error("Negative measure = %g, location %d, interval %d.", "CalculateMeasure()", ppM[i][t], t, i);
    gDataHub.ValidateObservedToExpectedCases(Set); //validate that observed and expected agree
    AdjustMeasure(Set, *pPopMeasure); // apply adjustments
    Set.setMeasureDataToCumulative(); // now we can make the measure data cummulative
    if (fabs(Set.getTotalCases() - Set.getTotalMeasure()) > 0.0001) // bug check total cases == total measure
      throw prg_error("Total measure '%8.6lf' != total cases '%ld'.", "CalculateMeasure()", Set.getTotalMeasure(), Set.getTotalCases());
    if ((gParameters.GetTimeTrendAdjustmentType() == STRATIFIED_RANDOMIZATION ||
        gParameters.GetTimeTrendAdjustmentType() == CALCULATED_LOGLINEAR_PERC) &&
        gParameters.GetAnalysisType() != SPATIALVARTEMPTREND/* SVTTData invokes this method as well */)
      Set.setMeasureData_PT_NC();
  }
  catch (prg_exception &x) {
    x.addTrace("CalculateMeasure()","CPoissonModel");
    throw;
  }
}

/** Returns population as defined in CCluster object. */
double CPoissonModel::GetPopulation(size_t tSetIndex, const CCluster& Cluster, const CSaTScanData& DataHub) const {
  tract_t               T, t;
  int                   c, ncats, nPops;
  std::vector<double>   vAlpha;
  const PopulationData& Population = DataHub.GetDataSetHandler().GetDataSet(tSetIndex).getPopulationData();
  double                nPopulation = 0.0;

  try {
    ncats = Population.GetNumCovariateCategories();
    nPops = Population.GetNumPopulationDates();
    Population.CalculateAlpha(vAlpha, DataHub.GetStudyPeriodStartDate(), DataHub.GetStudyPeriodEndDate());
    for (T = 1; T <= Cluster.GetNumTractsInCluster(); T++) {
       t = DataHub.GetNeighbor(Cluster.GetEllipseOffset(), Cluster.GetCentroidIndex(), T, Cluster.GetCartesianRadius());
       if ((size_t)t < DataHub.GetTInfo()->getLocations().size())
         for (c = 0; c < ncats; c++) Population.GetAlphaAdjustedPopulation(nPopulation, t, c, 0, nPops, vAlpha);
       else {
         std::vector<tract_t> AtomicIndexes;
         DataHub.GetTInfo()->getMetaLocations().getLocations().at((size_t)t - DataHub.GetTInfo()->getLocations().size())->getAtomicIndexes(AtomicIndexes);
         for (size_t a=0; a < AtomicIndexes.size(); ++a)
           for (c = 0; c < ncats; c++) Population.GetAlphaAdjustedPopulation(nPopulation, AtomicIndexes[a], c, 0, nPops, vAlpha);
       }
    }
  }
  catch (prg_exception & x) {
    x.addTrace("GetPopulation()", "CPoissonModel");
    throw;
  }
  return nPopulation;
}

/** Adjusts passed non-cumulative measure in a stratified spatial manner.
    Each time, for a particular tract, is multiplied by the total case divided
    by total measure of that tract, across all time intervals. */
void CPoissonModel::StratifiedSpatialAdjustment(RealDataSet& DataSet) {
  measure_t  ** ppNonCumulativeMeasure=DataSet.getMeasureData().GetArray();
  count_t    ** ppCases = DataSet.getCaseData().GetArray();

  for (unsigned int t=0; t < DataSet.getLocationDimension(); ++t) {
     //calculates total measure for current tract across all time intervals
     measure_t tTotalTractMeasure=0;
     for (unsigned int i=0; i < DataSet.getIntervalDimension(); ++i)
        tTotalTractMeasure += ppNonCumulativeMeasure[i][t];
     if (tTotalTractMeasure) {
       double dTractAdjustment = ppCases[0][t]/tTotalTractMeasure;
       //now multiply each time interval/tract location by (total cases)/(total measure)
       for (unsigned int i=0; tTotalTractMeasure && i < DataSet.getIntervalDimension(); ++i)
          ppNonCumulativeMeasure[i][t] *= dTractAdjustment;
     }
  }
}


//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "PoissonModel.h"
#include "SSException.h"
#include "DateStringParser.h"
#include "DataSource.h"
#include "BasePrint.h"

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
void CPoissonModel::AdjustForTemporalNonParameteric(RealDataSet& DataSet) {
    unsigned int          jlast = DataSet.getIntervalDimension() - 1;
    double                sumcases, summeasure, adjustmentTotal = static_cast<double>(DataSet.getTotalCases()) / DataSet.getTotalMeasure();
    count_t            ** ppCases = DataSet.getCaseData().GetArray();
    measure_t          ** pNonCumulative = DataSet.getMeasureData().GetArray();

    for (unsigned int k=0; k < DataSet.getIntervalDimension(); ++k) {
        // calculate the total cases and measure in curent time interval
	    sumcases = summeasure = 0;
        for (unsigned int tract=0; tract < DataSet.getLocationDimension(); ++tract) {
            sumcases += ppCases[k][tract] - (k == jlast ? 0.0 : ppCases[k + 1][tract]);
            summeasure += pNonCumulative[k][tract];
        }
        // adjust measure for each tract in current time interval
        for (unsigned int tract=0; tract < DataSet.getLocationDimension(); ++tract)
            pNonCumulative[k][tract] *= (sumcases / summeasure) / adjustmentTotal;
    }
}

/* Adjusts measure for trend beta and beta2. For log linear trends, beta2 should be 0. */
void CPoissonModel::AdjustForTrend(RealDataSet& DataSet, double beta, double beta2) {
    double adjustedMeasure = 0;
    measure_t ** ppNonCumulativeMeasure = DataSet.getMeasureData().GetArray();

    /* Adjust the measure assigned to each interval/tract by yearly percentage */
    for (unsigned int i=0; i < DataSet.getIntervalDimension(); ++i)
        for (unsigned int t=0; t < DataSet.getLocationDimension(); ++t) {
            ppNonCumulativeMeasure[i][t] *= exp((beta * i) + (beta2 * pow(i, 2)));
            if (adjustedMeasure > std::numeric_limits<measure_t>::max() - ppNonCumulativeMeasure[i][t])
                throw resolvable_error(
                    "Error: Data overflow occurs when performing the time trend adjustment in data set %u.\n"
                    "       Please run analysis without the time trend adjustment.\n", DataSet.getSetIndex()
                );
            adjustedMeasure += ppNonCumulativeMeasure[i][t];
        }

    double c = (double)(DataSet.getTotalMeasure()) / adjustedMeasure; // total adjusted measure
    /* Mutlipy the measure for each interval/tract by total adjusted measure equal to previous total measure. */
    for (unsigned int i=0; i < DataSet.getIntervalDimension(); ++i)
        for (unsigned int t=0; t < DataSet.getLocationDimension(); ++t)
            ppNonCumulativeMeasure[i][t] *= c;
}

/** Calculates time trend for dataset and adjustment data set measure for that trend. */
void CPoissonModel::AdjustForCalculatedTrend(RealDataSet& Set) {
  //purely temporal, not-cumulative measure data not available yet in RealDataSet object, so create temporary.
  std::vector<measure_t> vMeasure_PT_NC(Set.getIntervalDimension());
  measure_t ** ppMeasure = Set.getMeasureData().GetArray();
  for (unsigned int i=0; i < Set.getIntervalDimension(); ++i)
     for (unsigned int j=0; j < Set.getLocationDimension(); ++j)
        vMeasure_PT_NC[i] += ppMeasure[i][j];

  // Calculate time trend for whole dataset -- either log linear or log quadratic.
  std::auto_ptr<AbstractTimeTrend> TimeTrend(AbstractTimeTrend::getTimeTrend(gParameters));
  TimeTrend->CalculateAndSet(Set.getCaseData_PT_NC(), &vMeasure_PT_NC[0], Set.getIntervalDimension(), gTimeTrendConvergence);

  //Cancel analysis execution if calculation of time trend fails for various reasons.
  switch (TimeTrend->GetStatus()) {
    case AbstractTimeTrend::UNDEFINED         :
      throw resolvable_error("Note: The time trend is undefined and the temporal adjustment could not be performed.\n"
                             "      Please run analysis without automatic adjustment of time trends.");
    case AbstractTimeTrend::NEGATIVE_INFINITY :
    case AbstractTimeTrend::POSITIVE_INFINITY :
      throw resolvable_error("Note: The time trend is infinite and the temporal adjustment could not be performed.\n"
                             "      Please run analysis without automatic adjustment of time trends.");
	case AbstractTimeTrend::SINGULAR_MATRIX   :
      throw prg_error("The time trend of cluster was not calculated because matrix A is singular.\n","AdjustForCalculatedTrend()");
    case AbstractTimeTrend::NOT_CONVERGED     :
      throw prg_error("The time trend did not converge.\n", "AdjustForCalculatedTrend()");
    case AbstractTimeTrend::CONVERGED         : break;
    default : throw prg_error("Unknown time trend status type '%d'.", "AdjustForCalculatedTrend()", TimeTrend->GetStatus());
  };

  // Set the annual trend -- we probably won't use this variable anymore now that we're calculating trend by time aggregation.
  TimeTrend->SetAnnualTimeTrend(gParameters.GetTimeAggregationUnitsType(), gParameters.GetTimeAggregationLength());
  // Adjust the measure of data by the calculated trend.
  AdjustForTrend(Set, TimeTrend->GetBeta(), TimeTrend->GetBeta2());

  //store calculated time trend adjustment for reporting later
  const QuadraticTimeTrend * pQTrend = dynamic_cast<const QuadraticTimeTrend *>(TimeTrend.get());
  if (pQTrend) {
      std::string buffer, buffer2;
      pQTrend->getRiskFunction(buffer, buffer2, gDataHub);
      Set.setCalculatedQuadraticTimeTrend(buffer, buffer2);
  } else
    Set.setCalculatedTimeTrendPercentage(
        TimeTrend->GetTimeTrendByAggregationUnits(
            TimeTrend->GetBeta(), TimeTrend->GetStatus(), 
            gDataHub.GetParameters().GetTimeAggregationUnitsType(), gDataHub.GetParameters().GetTimeAggregationLength()
        )
    );

  TimeTrend->printSeries(Set, gDataHub);
}

/** Adjusts passed non-cumulative measure for parameter specified temporal,
    spatial, and space-time adjustments.                                      */
void CPoissonModel::AdjustMeasure(RealDataSet& DataSet, const TwoDimMeasureArray_t& PopMeasure) {
    try {
        //apply known relative risk adjustments to measure
        if (gParameters.UseAdjustmentForRelativeRisksFile())
            gDataHub.getRiskAdjustments()->apply(PopMeasure, DataSet.getPopulationData(), DataSet.getTotalMeasure(), DataSet.getMeasureData(), &DataSet.getCaseData());
        //apply temporal adjustments to measure
        if (DataSet.getIntervalDimension() > 1) {
            switch (gParameters.GetTimeTrendAdjustmentType()) {
                case TEMPORAL_NOTADJUSTED              : break;
                case LOGLINEAR_PERC                    : {
                    // Derive beta from user specified percentage and time aggregation. (ln(exp(x)) = x)
                    double x = log(static_cast<double>(gParameters.GetTimeTrendAdjustmentPercentage()) / 100.0 + 1.0);
                    double beta = 0.0;
                    switch (gParameters.GetTimeAggregationUnitsType()) {
                        case GENERIC:
                        case YEAR  : beta = x * static_cast<double>(gParameters.GetTimeAggregationLength()); break;
                        case MONTH : beta = x / (12.0 / static_cast<double>(gParameters.GetTimeAggregationLength())); break;
                        case DAY   : beta = x / (AVERAGE_DAYS_IN_YEAR / static_cast<double>(gParameters.GetTimeAggregationLength())); break;
                        default: throw prg_error("AdjustMeasure() unknown aggregation '%d'.", "AdjustMeasure()", gParameters.GetTimeAggregationUnitsType());
                    }
                    fprintf(AppToolkit::getToolkit().openDebugFile(), "Loglinear beta %g \n", beta);
                    fflush(AppToolkit::getToolkit().openDebugFile());
                    AdjustForTrend(DataSet, beta, 0.0);
                    //store calculated time trend adjustment for reporting later
                    DataSet.setCalculatedTimeTrendPercentage(AbstractTimeTrend::GetTimeTrendByAggregationUnits(beta, AbstractTimeTrend::CONVERGED, gParameters.GetTimeAggregationUnitsType(), gParameters.GetTimeAggregationLength()));
                } break;
                case CALCULATED_QUADRATIC              :
                case CALCULATED_LOGLINEAR_PERC         : AdjustForCalculatedTrend(DataSet); break;
                case TEMPORAL_STRATIFIED_RANDOMIZATION :
                    // If performing both the non-parametric spatial adjustment and temporal time stratified, first adjust the spatial first.
                    if (gParameters.GetSpatialAdjustmentType() == SPATIAL_NONPARAMETRIC)
                        AdjustForSpatialNonParameteric(DataSet);
                case TEMPORAL_NONPARAMETRIC: AdjustForTemporalNonParameteric(DataSet); break;
                default : throw prg_error("Unknown time trend adjustment type: '%d'.", "AdjustMeasure()", gParameters.GetTimeTrendAdjustmentType());
            }
        }
        //apply spatial adjusts to measure
        switch (gParameters.GetSpatialAdjustmentType()) {
            case SPATIAL_NOTADJUSTED: break;
            case SPATIAL_NONPARAMETRIC:
                // If performing both the non-parametric spatial adjustment and temporal time stratified, then skip since we've already done this adjustment.
                if (gParameters.GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION)
                    break;
            case SPATIAL_STRATIFIED_RANDOMIZATION: AdjustForSpatialNonParameteric(DataSet); break;
            default : throw prg_error("Unknown spatial adjustment type: '%d'.","AdjustMeasure()", gParameters.GetSpatialAdjustmentType());
        }
        // Bug check, to ensure that adjusted  total measure equals previously determined total measure
        unsigned int i = 0;
        measure_t ** ppNonCumulativeMeasure = DataSet.getMeasureData().GetArray(), adjustedTotalMeasure = 0;
        for (; i < DataSet.getIntervalDimension(); ++i)
            for (unsigned int t=0; t < DataSet.getLocationDimension(); ++t)
                adjustedTotalMeasure += ppNonCumulativeMeasure[i][t];
        if (fabs(adjustedTotalMeasure - DataSet.getTotalMeasure()) > 0.001)
            throw prg_error(
                "Error: The adjusted total measure '%8.6lf' is not equal to the total measure '%8.6lf'.\n", "AdjustMeasure()", 
                adjustedTotalMeasure, DataSet.getTotalMeasure()
            );
    } catch (prg_exception &x) {
        x.addTrace("AdjustMeasure()","CPoissonModel");
        throw;
    }
}

/** Calculates the measure from data set's population data and applies any adjustments. */
boost::shared_ptr<TwoDimMeasureArray_t> CPoissonModel::calculateMeasure(RealDataSet& Set, PopulationData * pAltPopulationData) {
    try {
        boost::shared_ptr<TwoDimMeasureArray_t> pPopMeasure = Calcm(Set, gDataHub.GetStudyPeriodStartDate(), gDataHub.GetStudyPeriodEndDate(), pAltPopulationData);
        CalcMeasure(Set, *pPopMeasure, gDataHub.CSaTScanData::GetTimeIntervalStartTimes(), gDataHub.GetStudyPeriodStartDate(), gDataHub.GetStudyPeriodEndDate(), pAltPopulationData);
        measure_t** ppM = Set.getMeasureData().GetArray(); // validate that all elements of measure array are not negative
        for (unsigned int t=0; t < Set.getLocationDimension(); ++t) {
            for (unsigned int i=0; i < Set.getIntervalDimension(); ++i)
                if (ppM[i][t] < 0) 
                    throw prg_error("Negative measure = %g, location %d, interval %d.", "CalculateMeasure()", ppM[i][t], t, i);
        }
        // validate that observed and expected agree
        if (!gParameters.getPerformPowerEvaluation() || !(gParameters.getPerformPowerEvaluation() && gParameters.getPowerEvaluationMethod() == PE_ONLY_SPECIFIED_CASES))
            gDataHub.ValidateObservedToExpectedCases(Set); 
        return pPopMeasure;
    } catch (prg_exception &x) {
        x.addTrace("calculateMeasure()","CPoissonModel");
        throw;
    }
}

/** Calculates the expected number of cases for dataset. */
void CPoissonModel::CalculateMeasure(RealDataSet& Set, const CSaTScanData& DataHub) {
    try {
        boost::shared_ptr<TwoDimMeasureArray_t> pPopMeasure = calculateMeasure(Set);
        // apply any adjustments -- other than the weekly adjustment
        AdjustMeasure(Set, *pPopMeasure);

        // When using power evaluations, we need to cache adjusted non-cummulative measure data in aux structure.
        // The non-cummulative measure data will be used in the null randomization of power step.
        bool cachePopulationMeasureData = (gParameters.getPerformPowerEvaluation() && gParameters.GetPowerEvaluationSimulationType() == STANDARD);

        // If performing adjustment for day of week, at this point we want to calculate the measure again.
        if (gParameters.getAdjustForWeeklyTrends()) {
            //TwoDimMeasureArray_t dailyMeasure(Set.getMeasureData());

            // Define a new PopulationData that will be grouped day of week covariate.
            boost::shared_ptr<PopulationData> populationData(new PopulationData());
            populationData->SetNumTracts(gDataHub.GetNumTracts());
            // The weekly adjustment will have the population defined on every day in the study period.
            // Since this adjustment requires the time aggregation to be set to 1 day, just iterate through defined time interval dates.
            assert(gParameters.GetTimeAggregationUnitsType() == DAY && gParameters.GetTimeAggregationLength() == 1);
            PopulationData::PopulationDate_t prPopulationDate(0, DAY);
            PopulationData::PopulationDateContainer_t vprPopulationDates;
            for (unsigned int i=0; i < gDataHub.GetTimeIntervalStartTimes().size(); ++i) {
                prPopulationDate.first = gDataHub.GetTimeIntervalStartTimes()[i];
                vprPopulationDates.push_back(prPopulationDate);
            }
            populationData->SetPopulationDates(vprPopulationDates, gDataHub.GetStudyPeriodStartDate(), gDataHub.GetStudyPeriodEndDate(), false/* TODO -- correct? */);
            vprPopulationDates.clear(); //dump memory

            PrintNull nullPrint; // TODO -- replace later.

            // Simulate the reading of the population file -- this time use the dailyMeasure data and date as covariate.
            const short uCovariateIndex=3;
            measure_t ** ppMeasure = Set.getMeasureData().GetArray();
            unsigned int numIntervals=Set.getMeasureData().Get1stDimension();
            unsigned int numLocations=Set.getMeasureData().Get2ndDimension();
            for (unsigned int i=0; i < numIntervals; ++i) {
                prPopulationDate.first = gDataHub.GetTimeIntervalStartTimes()[i];
                unsigned long covariate = prPopulationDate.first % 7; // convert date to day of week covariate
                OneCovariateDataSource dataSource(uCovariateIndex, covariate);
                int categoryIndex = populationData->CreateCovariateCategory(dataSource, uCovariateIndex, nullPrint);
                for (unsigned int t=0; t < numLocations; ++t) {
                    populationData->AddCovariateCategoryPopulation(t, categoryIndex, prPopulationDate, ppMeasure[i][t]);
                }
            }

            // convert dataSet.getPopulationData() case information to populationData. ???
            count_t ** ppCases = Set.getCaseData().GetArray();
            for (unsigned int i=0; i < numIntervals; ++i) {
                std::vector<std::string> covariates;
                covariates.push_back(std::string(""));
                Julian date = gDataHub.GetTimeIntervalStartTimes()[i];
                printString(covariates.back(), "%u", date % 7);
                int category_index = populationData->GetCovariateCategoryIndex(covariates);
                assert(category_index != -1);
                for (unsigned int t=0; t < numLocations; ++t) {
                    populationData->AddCovariateCategoryCaseCount(category_index, ppCases[i][t] - (i == numIntervals - 1 ? 0 : ppCases[i+1][t]));
                }
            }
            // now re-calculate the measure with alternative population data.
            boost::shared_ptr<TwoDimMeasureArray_t> popMeasure = calculateMeasure(Set, populationData.get());
            if (cachePopulationMeasureData) {
                Set.setPopulationMeasureData(*popMeasure, &populationData);
                Set.setMeasureData_Aux(Set.getMeasureData());
            }
        } else if (cachePopulationMeasureData) {
            Set.setPopulationMeasureData(*pPopMeasure);
            Set.setMeasureData_Aux(Set.getMeasureData());
        }

        if (gParameters.GetAnalysisType() == SEASONALTEMPORAL) {
            // At this point, we'll have the measure data (Set.getMeasureData()) and the case data (Set.getCaseData()) in non-seasonal structure.
            // Compress the measure data to seasonal intervals -- the measure data is currently non-cumulative.
            Julian date;
            TwoDimMeasureArray_t measures(DataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts() + gDataHub.GetNumMetaTracts(), 0.0);
            measure_t ** ppMeasure = measures.GetArray();
            for (int i=0; i < DataHub.CSaTScanData::GetNumTimeIntervals() - 1; ++i) {
                measure_t nmeasure = Set.getMeasureData().GetArray()[i][0];
                if (nmeasure) {
                    date = DataHub.convertToSeasonalDate(DataHub.CSaTScanData::GetTimeIntervalStartTimes()[i]);
                    ppMeasure[DataHub.GetTimeIntervalOfDate(date)][0] += nmeasure;
                }
            }
            // Compress the case data to seasonal intervals -- the case data is currently cumulative.
            TwoDimCountArray_t cases(DataHub.GetNumTimeIntervals(), gDataHub.GetNumTracts() + gDataHub.GetNumMetaTracts(), 0);
            count_t ** ppCases = cases.GetArray();
            for (int i=0; i < DataHub.CSaTScanData::GetNumTimeIntervals() - 1; ++i) {
                count_t ncases = Set.getCaseData().GetArray()[i][0] - Set.getCaseData().GetArray()[i + 1][0];
                if (ncases) {
                    date = DataHub.convertToSeasonalDate(DataHub.CSaTScanData::GetTimeIntervalStartTimes()[i]);
                    ppCases[DataHub.GetTimeIntervalOfDate(date)][0] += ncases;
                }
            }

            Set.reassign(cases, measures);
            Set.setCaseDataToCumulative();
        }

        // now we can make the measure data cummulative
        Set.setMeasureDataToCumulative(); 
        if (fabs(Set.getTotalCases() - Set.getTotalMeasure()) > 0.001) // bug check total cases == total measure
            throw prg_error("Total measure '%8.6lf' != total cases '%ld'.", "CalculateMeasure()", Set.getTotalMeasure(), Set.getTotalCases());
        if ((gParameters.GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION ||
            gParameters.GetTimeTrendAdjustmentType() == CALCULATED_LOGLINEAR_PERC) &&
            gParameters.GetAnalysisType() != SPATIALVARTEMPTREND/* SVTTData invokes this method as well */)
            Set.setMeasureData_PT_NC();
    } catch (prg_exception &x) {
        x.addTrace("CalculateMeasure()","CPoissonModel");
        throw;
    }
}

/** Returns population as defined in CCluster object. */
double CPoissonModel::GetPopulation(size_t tSetIndex, const CCluster& Cluster, const CSaTScanData& DataHub) const {
    double population = 0.0;

    try {
        if (!DataHub.GetParameters().UsePopulationFile() || Cluster.GetClusterType() == PURELYTEMPORALCLUSTER)
            throw prg_error("CPoissonModel::GetPopulation() is not implemented when not using population file or purely temporal clusters.", "GetPopulation()");

        for (tract_t tIdx=1; tIdx <= Cluster.GetNumTractsInCluster(); ++tIdx) {
            tract_t ttractIdx = DataHub.GetNeighbor(Cluster.GetEllipseOffset(), Cluster.GetCentroidIndex(), tIdx, Cluster.GetCartesianRadius());
            population += GetLocationPopulation(tSetIndex, ttractIdx, Cluster, DataHub);
        }
    } catch (prg_exception & x) {
        x.addTrace("GetPopulation()", "CPoissonModel");
        throw;
    }
    return population;
}

/** Returns population as defined in CCluster object for specific tract. */
double CPoissonModel::GetLocationPopulation(size_t tSetIndex, tract_t tractIdx, const CCluster& Cluster, const CSaTScanData& DataHub) const {
    const PopulationData& Population = DataHub.GetDataSetHandler().GetDataSet(tSetIndex).getPopulationData();

    // Calcuate alpha -- if not already calcuated for population.
    if (_alpha.empty())
        Population.CalculateAlpha(_alpha, DataHub.GetStudyPeriodStartDate(), DataHub.GetStudyPeriodEndDate());

    double population = 0.0;
    int ncats = Population.GetNumCovariateCategories();
    int nPops = static_cast<int>(Population.GetNumPopulationDates());
    if (static_cast<size_t>(tractIdx) < DataHub.GetTInfo()->getLocations().size()) {
        if (!DataHub.GetIsNullifiedLocation(tractIdx))
            for (int c=0; c < ncats; ++c)
                Population.GetAlphaAdjustedPopulation(population, tractIdx, c, 0, nPops, _alpha);
    } else {
        std::vector<tract_t> AtomicIndexes;
		DataHub.GetTInfo()->getMetaManagerProxy().getIndexes(static_cast<size_t>(tractIdx) - DataHub.GetTInfo()->getLocations().size(), AtomicIndexes);
        for (size_t a=0; a < AtomicIndexes.size(); ++a) {
            if (!DataHub.GetIsNullifiedLocation(AtomicIndexes[a]))
                for (int c=0; c < ncats; ++c)
                    Population.GetAlphaAdjustedPopulation(population, AtomicIndexes[a], c, 0, nPops, _alpha);
        }
    }
    return population;
}

/** Adjusts passed non-cumulative measure in a stratified spatial manner.
    Each time, for a particular tract, is multiplied by the total case divided by total measure of that tract, across all time intervals. */
void CPoissonModel::AdjustForSpatialNonParameteric(RealDataSet& DataSet) {
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


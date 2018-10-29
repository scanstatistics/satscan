//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "UniformTimeModel.h"
#include "SSException.h"
//#include "DateStringParser.h"
#include "DataSource.h"
#include "BasePrint.h"
#include "SaTScanData.h"
#include "PopulationData.h"

/** constructor */
UniformTimeModel::UniformTimeModel(CSaTScanData& DataHub)
              :CModel(), gDataHub(DataHub), gParameters(DataHub.GetParameters()) {}

/** destructor */
UniformTimeModel::~UniformTimeModel() {}

/** Calculates the measure from data set's population data and applies any adjustments. */
boost::shared_ptr<TwoDimMeasureArray_t> UniformTimeModel::calculateMeasure(RealDataSet& Set, PopulationData * pAltPopulationData) {
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
        x.addTrace("calculateMeasure()","UniformTimeModel");
        throw;
    }
}

/** Calculates the expected number of cases for dataset. */
void UniformTimeModel::CalculateMeasure(RealDataSet& Set, const CSaTScanData& DataHub) {

    try {
        boost::shared_ptr<TwoDimMeasureArray_t> pPopMeasure = calculateMeasure(Set);
        // apply any adjustments -- other than the weekly adjustment
        if (gParameters.GetIsSpaceTimeAnalysis())
            StratifiedSpatialAdjustment(Set);

        // Bug check, to ensure that adjusted  total measure equals previously determined total measure
        unsigned int i = 0;
        measure_t AdjustedTotalMeasure_t=0, ** ppNonCumulativeMeasure = Set.getMeasureData().GetArray();
        for (i = 0; i < Set.getIntervalDimension(); ++i)
            for (unsigned int t=0; t < Set.getLocationDimension(); ++t)
                AdjustedTotalMeasure_t += ppNonCumulativeMeasure[i][t];
        if (fabs(AdjustedTotalMeasure_t - Set.getTotalMeasure()) > 0.001)
            throw prg_error("Error: The adjusted total measure '%8.6lf' is not equal to the total measure '%8.6lf'.\n",
                "AdjustMeasure()", AdjustedTotalMeasure_t, Set.getTotalMeasure());

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
            populationData->SetPopulationDates(vprPopulationDates, gDataHub.GetStudyPeriodStartDate(), gDataHub.GetStudyPeriodEndDate(), false);
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

        // now we can make the measure data cummulative
        Set.setMeasureDataToCumulative(); 
        if (fabs(Set.getTotalCases() - Set.getTotalMeasure()) > 0.001) // bug check total cases == total measure
            throw prg_error("Total measure '%8.6lf' != total cases '%ld'.", "CalculateMeasure()", Set.getTotalMeasure(), Set.getTotalCases());
    } catch (prg_exception &x) {
        x.addTrace("CalculateMeasure()","UniformTimeModel");
        throw;
    }
}

/** Returns population as defined in CCluster object. */
double UniformTimeModel::GetPopulation(size_t tSetIndex, const CCluster& Cluster, const CSaTScanData& DataHub) const {
    throw prg_error("UniformTimeModel::GetPopulation() is not implemented.", "GetPopulation()");
}

/** Returns population as defined in CCluster object for specific tract. */
double UniformTimeModel::GetLocationPopulation(size_t tSetIndex, tract_t tractIdx, const CCluster& Cluster, const CSaTScanData& DataHub) const {
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
        DataHub.GetTInfo()->getMetaLocations().getLocations().at(static_cast<size_t>(tractIdx) - DataHub.GetTInfo()->getLocations().size())->getAtomicIndexes(AtomicIndexes);
        for (size_t a=0; a < AtomicIndexes.size(); ++a) {
            if (!DataHub.GetIsNullifiedLocation(AtomicIndexes[a]))
                for (int c=0; c < ncats; ++c)
                    Population.GetAlphaAdjustedPopulation(population, AtomicIndexes[a], c, 0, nPops, _alpha);
        }
    }
    return population;
}

/** Adjusts passed non-cumulative measure in a stratified spatial manner.
    Each time, for a particular tract, is multiplied by the total case divided
    by total measure of that tract, across all time intervals. */
void UniformTimeModel::StratifiedSpatialAdjustment(RealDataSet& DataSet) {
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



#include "fixture_sampledata.h"
#include "AnalysisRun.h"

struct poisson_dayagg_fixture : prm_testset_fixture {
    poisson_dayagg_fixture() : prm_testset_fixture("poisson-adj\\parameters.prm") {}
    virtual ~poisson_dayagg_fixture() {}
};

class testAnalysisRunner : public AnalysisRunner {
protected:
	measure_t _total_measure;
public:
    testAnalysisRunner(const CParameters& Parameters, time_t StartTime, BasePrint& PrintDirection)
        :AnalysisRunner(Parameters, StartTime, PrintDirection) {
        _data_hub->ReadDataFromFiles();
		_total_measure = _data_hub->GetDataSetHandler().GetDataSet(0).getTotalMeasure();
    }
    virtual ~testAnalysisRunner() {}

	// Compares the adjusted measures from this runner with another runner, allowing for a specified tolerance in the comparison.
    void compareAdjustedMeasures(testAnalysisRunner& other, double tolerance) {
		const RealDataSet& dataset1 = _data_hub->GetDataSetHandler().GetDataSet(0);
        const RealDataSet& dataset2 = other._data_hub->GetDataSetHandler().GetDataSet(0);
        BOOST_CHECK(dataset1.getMeasureData().Get1stDimension() == dataset2.getMeasureData().Get1stDimension());
        BOOST_CHECK(dataset1.getMeasureData().Get2ndDimension() == dataset2.getMeasureData().Get2ndDimension());
		unsigned int numIntervals = dataset1.getMeasureData().Get1stDimension(), numTracts = dataset2.getMeasureData().Get2ndDimension();
        measure_t** ppMeasure1 = dataset1.getMeasureData().GetArray();
        measure_t** ppMeasure2 = dataset2.getMeasureData().GetArray();
        for (unsigned int i = 0; i < numIntervals; ++i) {
            for (unsigned int t = 0; t < numTracts; ++t) {
                BOOST_TEST(
                    ppMeasure1[i][t] - (i + 1 < numIntervals ? ppMeasure1[i + 1][t] : 0) == ppMeasure2[i][t] - (i + 1 < numIntervals ? ppMeasure2[i + 1][t] : 0),
                    boost::test_tools::tolerance(tolerance)
                );
            }
        }
		// Total measure and calculated time trend percentage should also be approximately equal within the specified tolerance.
        BOOST_TEST(dataset1.getTotalMeasure() == dataset2.getTotalMeasure(), boost::test_tools::tolerance(tolerance));
        BOOST_TEST(getCalculatedTimeTrendPercentage() == other.getCalculatedTimeTrendPercentage(), boost::test_tools::tolerance(tolerance));
    }

	measure_t getTotalMeasure() const { return _total_measure; }
    double getCalculatedTimeTrendPercentage() const {
        const RealDataSet& dataset = _data_hub->GetDataSetHandler().GetDataSet(0);
        return dataset.getCalculatedTimeTrendPercentage();
	}
};

// Test suite for edge cases and error conditions
BOOST_FIXTURE_TEST_SUITE(AdjustMeasureCompareAdjustmentsByType, poisson_dayagg_fixture)

// Test adjusted measure with year adjustment to expected equivalent day adjustment.
BOOST_AUTO_TEST_CASE(LogLinearPerc_YearToDay) {
    _parameters.SetTimeAggregationUnitsType(DAY);
    _parameters.SetTimeAggregationLength(1); // 1 day time aggregation
    _parameters.SetTimeTrendAdjustmentType(TEMPORAL_NOTADJUSTED);
    testAnalysisRunner runner(_parameters, 0, _print);
    measure_t unadjustedTotalMeasure = runner.getTotalMeasure();

    for (auto perc : { 0.0157, 0.25, 0.5, 0.75, 1.0, 5.0, 10.0 }) {
        for (auto multiplier : { 1.0, -1.0 }) {
            double pecentage = perc * multiplier;
            BOOST_TEST_MESSAGE("YearToDay evaluating pecentage: " << pecentage);
            // Setup parameters for log-linear percentage adjustment with year units and pecentage per year.
            CParameters yearParameters(_parameters);
            yearParameters.SetTimeTrendAdjustmentType(LOGLINEAR_PERC);
            yearParameters.SetTimeTrendAdjustmentPercentage(pecentage);
            yearParameters.setLogLinearTimeTrendAdjUnits(YEAR); // pecentage per year
            // Read data and calculated measures with year adjustment parameters.
            testAnalysisRunner runnerYearAdjustment(yearParameters, 0, _print);
            BOOST_TEST(unadjustedTotalMeasure == runnerYearAdjustment.getTotalMeasure(), boost::test_tools::tolerance(0.0001));
            // Setup parameters for log-linear percentage adjustment with day units and equivalent percentage by days.
            CParameters dayParameters(_parameters);
            dayParameters.SetTimeTrendAdjustmentType(LOGLINEAR_PERC);
            dayParameters.setLogLinearTimeTrendAdjUnits(DAY);
            dayParameters.SetTimeTrendAdjustmentPercentage(pecentage / AVERAGE_DAYS_IN_YEAR); // pecentage/365.25 per year
            // Read data and calculated measures with day adjustment parameters.
            testAnalysisRunner runnerDayAdjustment(dayParameters, 0, _print);
            BOOST_TEST(unadjustedTotalMeasure == runnerDayAdjustment.getTotalMeasure(), boost::test_tools::tolerance(0.0001));
            // compare the adjusted measures from both adjustments, they should be approximately equal within a small tolerance.
            runnerYearAdjustment.compareAdjustedMeasures(runnerDayAdjustment, 0.1);
        }
    }
}

// Test adjusted measure with year adjustment to expected equivalent month adjustment.
BOOST_AUTO_TEST_CASE(LogLinearPerc_YearToMonth) {
    _parameters.SetTimeAggregationUnitsType(DAY);
    _parameters.SetTimeAggregationLength(1); // 1 day time aggregation
    _parameters.SetTimeTrendAdjustmentType(TEMPORAL_NOTADJUSTED);
    testAnalysisRunner runner(_parameters, 0, _print);
    measure_t unadjustedTotalMeasure = runner.getTotalMeasure();

    for (auto perc : { 0.0157, 0.25, 0.5, 0.75, 1.0, 5.0, 10.0 }) {
        for (auto multiplier : { 1.0, -1.0 }) {
            double pecentage = perc * multiplier;
            BOOST_TEST_MESSAGE("YearToMonth evaluating pecentage: " << pecentage);
            // Setup parameters for log-linear percentage adjustment with year units and 1.25% increase per year.
            _parameters.SetTimeTrendAdjustmentType(LOGLINEAR_PERC);
            _parameters.SetTimeTrendAdjustmentPercentage(pecentage);
            _parameters.setLogLinearTimeTrendAdjUnits(YEAR); // pecentage per year
            // Read data and calculated measures with year adjustment parameters.
            testAnalysisRunner runnerYearAdjustment(_parameters, 0, _print);
            BOOST_TEST(unadjustedTotalMeasure == runnerYearAdjustment.getTotalMeasure(), boost::test_tools::tolerance(0.0001));
            // Setup parameters for log-linear percentage adjustment with day units and equivalent percentage by days.
            CParameters monthParameters(_parameters);
            monthParameters.setLogLinearTimeTrendAdjUnits(MONTH);
            monthParameters.SetTimeTrendAdjustmentPercentage(pecentage / 12.0); // pecentage/12.0 increase per month
            // Read data and calculated measures with month adjustment parameters.
            testAnalysisRunner runnerMonthAdjustment(monthParameters, 0, _print);
            BOOST_TEST(unadjustedTotalMeasure == runnerMonthAdjustment.getTotalMeasure(), boost::test_tools::tolerance(0.0001));
            // compare the adjusted measures from both adjustments, they should be approximately equal within a small tolerance.
            runnerYearAdjustment.compareAdjustedMeasures(runnerMonthAdjustment, 0.1);
        }
    }
}

// Test adjusted measure with month adjustment to expected equivalent month adjustment.
BOOST_AUTO_TEST_CASE(LogLinearPerc_MonthToDay) {
    _parameters.SetTimeAggregationUnitsType(DAY);
    _parameters.SetTimeAggregationLength(1); // 1 day time aggregation
    _parameters.SetTimeTrendAdjustmentType(TEMPORAL_NOTADJUSTED);
    testAnalysisRunner runner(_parameters, 0, _print);
    measure_t unadjustedTotalMeasure = runner.getTotalMeasure();

    for (auto perc : { 0.0157, 0.25, 0.5, 0.75, 1.0, 5.0, 10.0 }) {
        for (auto multiplier : { 1.0, -1.0 }) {
            double pecentage = perc * multiplier;
            BOOST_TEST_MESSAGE("MonthToDay evaluating pecentage: " << pecentage);
            // Setup parameters for log-linear percentage adjustment with year units and 1.25% increase per year.
            _parameters.SetTimeTrendAdjustmentType(LOGLINEAR_PERC);
            _parameters.SetTimeTrendAdjustmentPercentage(pecentage);
            _parameters.setLogLinearTimeTrendAdjUnits(MONTH); // pecentage per month
            // Read data and calculated measures with year adjustment parameters.
            testAnalysisRunner runnerMonthAdjustment(_parameters, 0, _print);
            BOOST_TEST(unadjustedTotalMeasure == runnerMonthAdjustment.getTotalMeasure(), boost::test_tools::tolerance(0.001));
            // Setup parameters for log-linear percentage adjustment with day units and equivalent percentage by days.
            CParameters dayParameters(_parameters);
            dayParameters.setLogLinearTimeTrendAdjUnits(DAY);
            dayParameters.SetTimeTrendAdjustmentPercentage(pecentage / AVERAGE_DAYS_IN_MONTH); // pecentage/30.42 per day
            // Read data and calculated measures with day adjustment parameters.
            testAnalysisRunner runnerDayAdjustment(dayParameters, 0, _print);
            BOOST_TEST(unadjustedTotalMeasure == runnerDayAdjustment.getTotalMeasure(), boost::test_tools::tolerance(0.001));
            // compare the adjusted measures from both adjustments, they should be approximately equal within a small tolerance.
            runnerMonthAdjustment.compareAdjustedMeasures(runnerDayAdjustment, 0.1);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

#include <boost/test/unit_test.hpp>

#include "cluster.h"
#include "Parameters.h"
#include "PrintScreen.h"

#include <cmath>

namespace {
    class RecurrenceIntervalDataHub : public CSaTScanData {
    public:
        RecurrenceIntervalDataHub(const CParameters& parameters, BasePrint& print, int num_time_intervals)
            : CSaTScanData(parameters, print) {
            m_nTimeIntervals = num_time_intervals;
            m_nProspectiveIntervalStart = num_time_intervals;
        }

    private:
        void SetProbabilityModel() override {}
    };

    CParameters getProspectiveParameters(DatePrecisionType aggregation_units, long aggregation_length) {
        CParameters parameters;
        parameters.SetAnalysisType(PROSPECTIVESPACETIME);
        parameters.SetPValueReportingType(STANDARD_PVALUE);
        parameters.SetTimeAggregationUnitsType(aggregation_units);
        parameters.SetTimeAggregationLength(aggregation_length);
        parameters.setProspectiveFrequencySelection(SAMEAS_TIMEAGG);
        return parameters;
    }

    SimulationVariables getSimulationVariables(unsigned int simulation_count) {
        SimulationVariables sim_vars;
        sim_vars.set_sim_count_explicit(simulation_count);
        return sim_vars;
    }

    double getPValue(double p_value) {
        return 1.0 - std::pow(1.0 - p_value, 1.0 / 1.0);
    }
}

BOOST_AUTO_TEST_SUITE(cluster_recurrence_interval_suite)

BOOST_AUTO_TEST_CASE(same_as_daily_time_aggregation_uses_standard_pvalue) {
    CParameters parameters = getProspectiveParameters(DAY, 1);
    PrintNull print;
    RecurrenceIntervalDataHub data(parameters, print, 10);
    CCluster cluster;
    SimulationVariables sim_vars = getSimulationVariables(parameters.GetNumReplicationsRequested());

    const double p_value = 1.0 / static_cast<double>(parameters.GetNumReplicationsRequested() + 1);
    const double expected_days = 1.0 / getPValue(p_value);
    const CCluster::RecurrenceInterval_t recurrence = cluster.GetRecurrenceInterval(data, 1, sim_vars);

    BOOST_REQUIRE_CLOSE(recurrence.second, expected_days, 0.000001);
    BOOST_REQUIRE_CLOSE(recurrence.first, expected_days / AVERAGE_DAYS_IN_YEAR, 0.000001);
}

BOOST_AUTO_TEST_CASE(every_x_weekly_frequency_converts_occurrences_to_years_and_days) {
    CParameters parameters = getProspectiveParameters(DAY, 1);
    parameters.setProspectiveFrequencySelection(EVERY_X);
    parameters.setProspectiveFrequencyType(WEEKLY);
    parameters.setProspectiveFrequency(2);

    PrintNull print;
    RecurrenceIntervalDataHub data(parameters, print, 12);
    CCluster cluster;
    SimulationVariables sim_vars = getSimulationVariables(parameters.GetNumReplicationsRequested());

    const double p_value = 1.0 / static_cast<double>(parameters.GetNumReplicationsRequested() + 1);
    const double expected_weeks = 2.0 / getPValue(p_value);
    const CCluster::RecurrenceInterval_t recurrence = cluster.GetRecurrenceInterval(data, 1, sim_vars);

    BOOST_REQUIRE_CLOSE(recurrence.first, expected_weeks / 52.0, 0.000001);
    BOOST_REQUIRE_CLOSE(recurrence.second, (expected_weeks / 52.0) * AVERAGE_DAYS_IN_YEAR, 0.000001);
}

BOOST_AUTO_TEST_CASE(same_as_daily_time_aggregation_equals_every_day) {
    CCluster::RecurrenceInterval_t recurrence_same, recurrence_daily;

    {
        CParameters parameters = getProspectiveParameters(DAY, 1);
        parameters.setProspectiveFrequencySelection(SAMEAS_TIMEAGG);
        //parameters.setProspectiveFrequencyType(DAILY);
        //parameters.setProspectiveFrequency(1);
        RecurrenceIntervalDataHub data(parameters, PrintNull(), 1000);
        recurrence_same = CCluster().GetRecurrenceInterval(data, 1, getSimulationVariables(parameters.GetNumReplicationsRequested()));
    }
    {
        CParameters parameters = getProspectiveParameters(DAY, 1);
        parameters.setProspectiveFrequencySelection(EVERY_X);
        parameters.setProspectiveFrequencyType(DAILY);
        parameters.setProspectiveFrequency(1);
        RecurrenceIntervalDataHub data(parameters, PrintNull(), 1000);
        recurrence_daily = CCluster().GetRecurrenceInterval(data, 1, getSimulationVariables(parameters.GetNumReplicationsRequested()));
    }
    BOOST_REQUIRE(std::abs(recurrence_same.first - recurrence_daily.first) == 0);
    BOOST_REQUIRE(std::abs(recurrence_same.second - recurrence_daily.second) == 0);
}

BOOST_AUTO_TEST_CASE(same_as_daily_time_aggregation_equals_every_month) {
    CCluster::RecurrenceInterval_t recurrence_same, recurrence_month;

    {
        CParameters parameters = getProspectiveParameters(MONTH, 1);
        parameters.setProspectiveFrequencySelection(SAMEAS_TIMEAGG);
        //parameters.setProspectiveFrequencyType(DAILY);
        //parameters.setProspectiveFrequency(1);
        RecurrenceIntervalDataHub data(parameters, PrintNull(), 1000);
        recurrence_same = CCluster().GetRecurrenceInterval(data, 1, getSimulationVariables(parameters.GetNumReplicationsRequested()));
    }
    {
        CParameters parameters = getProspectiveParameters(MONTH, 1);
        parameters.setProspectiveFrequencySelection(EVERY_X);
        parameters.setProspectiveFrequencyType(MONTHLY);
        parameters.setProspectiveFrequency(1);
        RecurrenceIntervalDataHub data(parameters, PrintNull(), 1000);
        recurrence_month = CCluster().GetRecurrenceInterval(data, 1, getSimulationVariables(parameters.GetNumReplicationsRequested()));
    }
    BOOST_REQUIRE(std::abs(recurrence_same.first - recurrence_month.first) == 0);
    BOOST_REQUIRE(std::abs(recurrence_same.second - recurrence_month.second) == 0);
}

BOOST_AUTO_TEST_CASE(same_as_daily_time_aggregation_equals_every_year) {
    CCluster::RecurrenceInterval_t recurrence_same, recurrence_year;

    {
        CParameters parameters = getProspectiveParameters(YEAR, 1);
        parameters.setProspectiveFrequencySelection(SAMEAS_TIMEAGG);
        //parameters.setProspectiveFrequencyType(DAILY);
        //parameters.setProspectiveFrequency(1);
        RecurrenceIntervalDataHub data(parameters, PrintNull(), 1000);
        recurrence_same = CCluster().GetRecurrenceInterval(data, 1, getSimulationVariables(parameters.GetNumReplicationsRequested()));
    }
    {
        CParameters parameters = getProspectiveParameters(YEAR, 1);
        parameters.setProspectiveFrequencySelection(EVERY_X);
        parameters.setProspectiveFrequencyType(YEARLY);
        parameters.setProspectiveFrequency(1);
        RecurrenceIntervalDataHub data(parameters, PrintNull(), 1000);
        recurrence_year = CCluster().GetRecurrenceInterval(data, 1, getSimulationVariables(parameters.GetNumReplicationsRequested()));
    }
    BOOST_REQUIRE(std::abs(recurrence_same.first - recurrence_year.first) == 0);
    BOOST_REQUIRE(std::abs(recurrence_same.second - recurrence_year.second) == 0);
}

BOOST_AUTO_TEST_CASE(every_week_equals_once_every_7_days) {
	CCluster::RecurrenceInterval_t recurrence_weeks, recurrence_days;

    for (unsigned int freq = 1; freq <= 10; ++freq) {
        {
            CParameters parameters = getProspectiveParameters(DAY, 1);
            parameters.setProspectiveFrequencySelection(EVERY_X);
            parameters.setProspectiveFrequencyType(WEEKLY);
            parameters.setProspectiveFrequency(freq);
            RecurrenceIntervalDataHub data(parameters, PrintNull(), 90);
            recurrence_weeks = CCluster().GetRecurrenceInterval(data, 1, getSimulationVariables(parameters.GetNumReplicationsRequested()));
        }
        {
            CParameters parameters = getProspectiveParameters(DAY, 1);
            parameters.setProspectiveFrequencySelection(EVERY_X);
            parameters.setProspectiveFrequencyType(DAILY);
            parameters.setProspectiveFrequency(7 * freq);
            RecurrenceIntervalDataHub data(parameters, PrintNull(), 90);
            recurrence_days = CCluster().GetRecurrenceInterval(data, 1, getSimulationVariables(parameters.GetNumReplicationsRequested()));
        }
        BOOST_REQUIRE(std::abs(recurrence_weeks.first - recurrence_days.first) < 1.0);
        BOOST_REQUIRE(std::abs(recurrence_weeks.second - recurrence_days.second) <= 25.0 * freq); // allow some variation due to rounding and leap years
    }
}

BOOST_AUTO_TEST_CASE(every_year_equals_once_every_12_months) {
    CCluster::RecurrenceInterval_t recurrence_years, recurrence_months;

    {
        CParameters parameters = getProspectiveParameters(DAY, 1);
        parameters.setProspectiveFrequencySelection(EVERY_X);
        parameters.setProspectiveFrequencyType(YEARLY);
        parameters.setProspectiveFrequency(1);
        RecurrenceIntervalDataHub data(parameters, PrintNull(), 1000);
        recurrence_years = CCluster().GetRecurrenceInterval(data, 1, getSimulationVariables(parameters.GetNumReplicationsRequested()));
    }
    {
        CParameters parameters = getProspectiveParameters(DAY, 1);
        parameters.setProspectiveFrequencySelection(EVERY_X);
        parameters.setProspectiveFrequencyType(MONTHLY);
        parameters.setProspectiveFrequency(12);
        RecurrenceIntervalDataHub data(parameters, PrintNull(), 1000);
        recurrence_months = CCluster().GetRecurrenceInterval(data, 1, getSimulationVariables(parameters.GetNumReplicationsRequested()));
    }
    BOOST_REQUIRE(std::abs(recurrence_years.first - recurrence_months.first) < 1.0);
    BOOST_REQUIRE(std::abs(recurrence_years.second - recurrence_months.second) < 1.0);
}

BOOST_AUTO_TEST_CASE(every_year_equals_once_every_52_weeks) {
    CCluster::RecurrenceInterval_t recurrence_years, recurrence_weeks;

    {
        CParameters parameters = getProspectiveParameters(DAY, 1);
        parameters.setProspectiveFrequencySelection(EVERY_X);
        parameters.setProspectiveFrequencyType(YEARLY);
        parameters.setProspectiveFrequency(1);
        RecurrenceIntervalDataHub data(parameters, PrintNull(), 1000);
        recurrence_years = CCluster().GetRecurrenceInterval(data, 1, getSimulationVariables(parameters.GetNumReplicationsRequested()));
    }
    {
        CParameters parameters = getProspectiveParameters(DAY, 1);
        parameters.setProspectiveFrequencySelection(EVERY_X);
        parameters.setProspectiveFrequencyType(WEEKLY);
        parameters.setProspectiveFrequency(52);
        RecurrenceIntervalDataHub data(parameters, PrintNull(), 1000);
        recurrence_weeks = CCluster().GetRecurrenceInterval(data, 1, getSimulationVariables(parameters.GetNumReplicationsRequested()));
    }
    BOOST_REQUIRE(std::abs(recurrence_years.first - recurrence_weeks.first) < 1.0);
    BOOST_REQUIRE(std::abs(recurrence_years.second - recurrence_weeks.second) < 1.0);
}

BOOST_AUTO_TEST_CASE(every_year_equals_once_every_4_quarters) {
    CCluster::RecurrenceInterval_t recurrence_years, recurrence_quarters;

    {
        CParameters parameters = getProspectiveParameters(DAY, 1);
        parameters.setProspectiveFrequencySelection(EVERY_X);
        parameters.setProspectiveFrequencyType(YEARLY);
        parameters.setProspectiveFrequency(1);
        RecurrenceIntervalDataHub data(parameters, PrintNull(), 1000);
        recurrence_years = CCluster().GetRecurrenceInterval(data, 1, getSimulationVariables(parameters.GetNumReplicationsRequested()));
    }
    {
        CParameters parameters = getProspectiveParameters(DAY, 1);
        parameters.setProspectiveFrequencySelection(EVERY_X);
        parameters.setProspectiveFrequencyType(QUARTERLY);
        parameters.setProspectiveFrequency(4);
        RecurrenceIntervalDataHub data(parameters, PrintNull(), 1000);
        recurrence_quarters = CCluster().GetRecurrenceInterval(data, 1, getSimulationVariables(parameters.GetNumReplicationsRequested()));
    }
    BOOST_REQUIRE(std::abs(recurrence_years.first - recurrence_quarters.first) < 1.0);
    BOOST_REQUIRE(std::abs(recurrence_years.second - recurrence_quarters.second) < 1.0);
}

BOOST_AUTO_TEST_CASE(every_year_equals_once_every_365_days) {
    CCluster::RecurrenceInterval_t recurrence_years, recurrence_days;

    {
        CParameters parameters = getProspectiveParameters(DAY, 1);
        parameters.setProspectiveFrequencySelection(EVERY_X);
        parameters.setProspectiveFrequencyType(YEARLY);
        parameters.setProspectiveFrequency(1);
        RecurrenceIntervalDataHub data(parameters, PrintNull(), 1000);
        recurrence_years = CCluster().GetRecurrenceInterval(data, 1, getSimulationVariables(parameters.GetNumReplicationsRequested()));
    }
    {
        CParameters parameters = getProspectiveParameters(DAY, 1);
        parameters.setProspectiveFrequencySelection(EVERY_X);
        parameters.setProspectiveFrequencyType(DAILY);
        parameters.setProspectiveFrequency(AVERAGE_DAYS_IN_YEAR);
        RecurrenceIntervalDataHub data(parameters, PrintNull(), 1000);
        recurrence_days = CCluster().GetRecurrenceInterval(data, 1, getSimulationVariables(parameters.GetNumReplicationsRequested()));
    }
    BOOST_REQUIRE(std::abs(recurrence_years.first - recurrence_days.first) < 1.0);
	BOOST_REQUIRE(std::abs(recurrence_years.second - recurrence_days.second) <= 250.0); // allow some variation due to rounding and leap years
}

BOOST_AUTO_TEST_CASE(twice_per_year_roughly_equals_every_182_days) {
    CCluster::RecurrenceInterval_t recurrence_years, recurrence_days;

    {
        CParameters parameters = getProspectiveParameters(DAY, 1);
        parameters.setProspectiveFrequencySelection(X_TIMES_PER);
        parameters.setProspectiveFrequencyType(YEARLY);
        parameters.setProspectiveFrequency(2);
        RecurrenceIntervalDataHub data(parameters, PrintNull(), 1000);
        recurrence_years = CCluster().GetRecurrenceInterval(data, 1, getSimulationVariables(parameters.GetNumReplicationsRequested()));
    }
    {
        CParameters parameters = getProspectiveParameters(DAY, 1);
        parameters.setProspectiveFrequencySelection(EVERY_X);
        parameters.setProspectiveFrequencyType(DAILY);
        parameters.setProspectiveFrequency(AVERAGE_DAYS_IN_YEAR/2.0);
        RecurrenceIntervalDataHub data(parameters, PrintNull(), 1000);
        recurrence_days = CCluster().GetRecurrenceInterval(data, 1, getSimulationVariables(parameters.GetNumReplicationsRequested()));
    }
    // allow some variation due to rounding and leap years
    BOOST_REQUIRE(std::abs(recurrence_years.first - recurrence_days.first) < 1.8);
    BOOST_REQUIRE(std::abs(recurrence_years.second - recurrence_days.second) <= 625);
}

BOOST_AUTO_TEST_CASE(roughly_364_per_year_equals_every_day) {
    CCluster::RecurrenceInterval_t recurrence_years, recurrence_days;

    {
        CParameters parameters = getProspectiveParameters(DAY, 1);
        parameters.setProspectiveFrequencySelection(X_TIMES_PER);
        parameters.setProspectiveFrequencyType(YEARLY);
        parameters.setProspectiveFrequency(364);
        RecurrenceIntervalDataHub data(parameters, PrintNull(), 1000);
        recurrence_years = CCluster().GetRecurrenceInterval(data, 1, getSimulationVariables(parameters.GetNumReplicationsRequested()));
    }
    {
        CParameters parameters = getProspectiveParameters(DAY, 1);
        parameters.setProspectiveFrequencySelection(EVERY_X);
        parameters.setProspectiveFrequencyType(DAILY);
        parameters.setProspectiveFrequency(1);
        RecurrenceIntervalDataHub data(parameters, PrintNull(), 1000);
        recurrence_days = CCluster().GetRecurrenceInterval(data, 1, getSimulationVariables(parameters.GetNumReplicationsRequested()));
    }
    // allow some variation due to rounding and leap years
    BOOST_REQUIRE(std::abs(recurrence_years.first - recurrence_days.first) < 0.0095);
    BOOST_REQUIRE(std::abs(recurrence_years.second - recurrence_days.second) <= 3.5);
}


/*BOOST_AUTO_TEST_CASE(x_times_per_daily_frequency_uses_fractional_occurrence_length) {
    CParameters parameters = getProspectiveParameters(DAY, 1);
    parameters.setProspectiveFrequencySelection(X_TIMES_PER);
    parameters.setProspectiveFrequencyType(DAILY);
    parameters.setProspectiveFrequency(4);

    PrintNull print;
    RecurrenceIntervalDataHub data(parameters, print, 8);
    CCluster cluster;
    SimulationVariables sim_vars = getSimulationVariables(parameters.GetNumReplicationsRequested());

    const double p_value = 1.0 / static_cast<double>(parameters.GetNumReplicationsRequested() + 1);
    const double expected_days = 0.25 / adjustedPValue(p_value);
    const CCluster::RecurrenceInterval_t recurrence = cluster.GetRecurrenceInterval(data, 1, sim_vars);

    BOOST_REQUIRE_CLOSE(recurrence.second, expected_days, 0.000001);
    BOOST_REQUIRE_CLOSE(recurrence.first, expected_days / AVERAGE_DAYS_IN_YEAR, 0.000001);
}*/

BOOST_AUTO_TEST_CASE(generic_time_aggregation_returns_units_in_both_fields) {
    CParameters parameters = getProspectiveParameters(GENERIC, 3);
    PrintNull print;
    RecurrenceIntervalDataHub data(parameters, print, 10);
    CCluster cluster;
    SimulationVariables sim_vars = getSimulationVariables(parameters.GetNumReplicationsRequested());

    const double p_value = 1.0 / static_cast<double>(parameters.GetNumReplicationsRequested() + 1);
    const double expected_units = 3.0 / getPValue(p_value);
    const CCluster::RecurrenceInterval_t recurrence = cluster.GetRecurrenceInterval(data, 1, sim_vars);

    BOOST_REQUIRE_CLOSE(recurrence.first, expected_units, 0.000001);
    BOOST_REQUIRE_CLOSE(recurrence.second, expected_units, 0.000001);
}

BOOST_AUTO_TEST_CASE(non_prospective_analysis_throws) {
    CParameters parameters;
    parameters.SetPValueReportingType(STANDARD_PVALUE);
    parameters.SetTimeAggregationUnitsType(DAY);
    parameters.SetTimeAggregationLength(1);

    PrintNull print;
    RecurrenceIntervalDataHub data(parameters, print, 10);
    CCluster cluster;
    SimulationVariables sim_vars = getSimulationVariables(parameters.GetNumReplicationsRequested());

    BOOST_CHECK_THROW(cluster.GetRecurrenceInterval(data, 1, sim_vars), prg_exception);
}

BOOST_AUTO_TEST_SUITE_END()

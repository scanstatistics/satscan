
// project files
#include "fixture_sampledata.h"
#include "AnalysisRun.h"
#include "Toolkit.h"
#include "LocationRiskEstimateWriter.h"

/* Test Suite for the closed loop analysis (seasonal). */
BOOST_FIXTURE_TEST_SUITE( closed_loop_analysis_suite, parameter_fixture )

/* Tests execuation of closed loop analysis with Poisson model. */
BOOST_AUTO_TEST_CASE( closed_loop_analysis_poisson_1 ) {
    std::string path(getTestSetFilesPath());
    std::stringstream filename;
    filename << path.c_str() << "\\closed-loop\\poisson.cas";
    _parameters.SetCaseFileName(filename.str().c_str());
    _parameters.SetPrecisionOfTimesType(DAY);
    _parameters.SetStudyPeriodStartDate("2005/1/1");
    _parameters.SetStudyPeriodEndDate("2013/12/31");
    _parameters.SetAnalysisType(SEASONALTEMPORAL);
    _parameters.SetProbabilityModelType(POISSON);
    _parameters.SetTimeAggregationUnitsType(DAY);
    _parameters.SetTimeAggregationLength(1);
    _parameters.setReportGiniOptimizedClusters(false);

    std::string results_user_directory;
    run_analysis("test", results_user_directory, _parameters, _print);
}

BOOST_AUTO_TEST_SUITE_END()

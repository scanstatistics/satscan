
// project files
#include "fixture_sampledata.h"
#include "AnalysisRun.h"
#include "Toolkit.h"
#include "LocationRiskEstimateWriter.h"
#include <fstream>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

/* maximum spatial cluster size less than 1.0 : https://www.squishlist.com/ims/satscan/66455/ */

struct squish66406_analysis_fixture : new_mexico_fixture {
    squish66406_analysis_fixture() : new_mexico_fixture() { 
    }
    virtual ~squish66406_analysis_fixture() {}

    std::string _results_user_directory;
};

/* Test Suite to confirm that maximum spatial cluster size can be less than 1.0. */
BOOST_FIXTURE_TEST_SUITE( squish66455_suite, squish66406_analysis_fixture )

/* Test maximum spatial cluster size less than one with sequential execution. */
BOOST_AUTO_TEST_CASE( squish66455_testcase_sequentially ) {
	_parameters.SetExecutionType(SUCCESSIVELY);

	_parameters.SetMaxSpatialSizeForType(PERCENTOFPOPULATION, 0.001, false);
    run_analysis("test", _results_user_directory, _parameters, _print);

    _parameters.SetMaxSpatialSizeForType(PERCENTOFPOPULATION, 0.25, false);
    run_analysis("test", _results_user_directory, _parameters, _print);

    _parameters.SetMaxSpatialSizeForType(PERCENTOFPOPULATION, 0.99, false);
    run_analysis("test", _results_user_directory, _parameters, _print);
}

/* Test maximum spatial cluster size less than one with centrically execution. */
BOOST_AUTO_TEST_CASE( squish66455_testcase_centrically ) {
	_parameters.SetExecutionType(CENTRICALLY);

	_parameters.SetMaxSpatialSizeForType(PERCENTOFPOPULATION, 0.001, false);
    run_analysis("test", _results_user_directory, _parameters, _print);

    _parameters.SetMaxSpatialSizeForType(PERCENTOFPOPULATION, 0.25, false);
    run_analysis("test", _results_user_directory, _parameters, _print);

    _parameters.SetMaxSpatialSizeForType(PERCENTOFPOPULATION, 0.99, false);
    run_analysis("test", _results_user_directory, _parameters, _print);
}

BOOST_AUTO_TEST_SUITE_END()

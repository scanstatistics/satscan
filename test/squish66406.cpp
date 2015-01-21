
// project files
#include "fixture_sampledata.h"
#include "AnalysisRun.h"
#include "Toolkit.h"
#include "LocationRiskEstimateWriter.h"
#include <fstream>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

/* total population is not adjusted in iterative scan : https://www.squishlist.com/ims/satscan/66406/ */

class testAnalysisRunner : public AnalysisRunner {
    public:
        testAnalysisRunner(const CParameters& Parameters, time_t StartTime, BasePrint& PrintDirection)
            : AnalysisRunner(Parameters, StartTime, PrintDirection) {}
        virtual ~testAnalysisRunner() {}

        virtual bool RepeatAnalysis() {
            double prePopulation = GetDataHub().GetDataSetHandler().GetDataSet(0).getTotalPopulation();
            bool return_value = AnalysisRunner::RepeatAnalysis();
            if (return_value) /* check if repeating analysis, otherwise population is unchanged */
                BOOST_CHECK_GT( prePopulation, GetDataHub().GetDataSetHandler().GetDataSet(0).getTotalPopulation() );
            return return_value;
        }
};

struct squish66406_analysis_fixture : new_mexico_fixture {
    squish66406_analysis_fixture() : new_mexico_fixture() { 
        // nm data set uses the Poisson model and space-time analysis initially
        _parameters.SetAnalysisType(PURELYSPATIAL);
        _parameters.SetIterativeScanning(true);
    }
    virtual ~squish66406_analysis_fixture() {}

    std::string _results_user_directory;
};

/* Test Suite for the Parameters validation class. */
BOOST_FIXTURE_TEST_SUITE( squish66406_suite, squish66406_analysis_fixture )

/* Tests for expected oliveira fields in risk estimates file with neither hierarchical nor gini selected. */
BOOST_AUTO_TEST_CASE( squish66406_testcase ) {
    run_analysis("test", _results_user_directory, _parameters, _print);
}

BOOST_AUTO_TEST_SUITE_END()

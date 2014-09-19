
// project files
#include "fixture_sampledata.h"
#include "PoissonLikelihoodCalculation.h"
#include "PurelySpatialData.h"

struct poisson_loglikelihood_fixture : new_mexico_fixture {
    poisson_loglikelihood_fixture() {
        // create the data hub and read from specified files
        _data_hub.reset(new CPurelySpatialData(_parameters, _print));
        _data_hub->ReadDataFromFiles();
    }
    virtual ~poisson_loglikelihood_fixture() { }

    std::auto_ptr<CSaTScanData> _data_hub;
};

/* Test Suite for the Poisson loglikelihood calculation. */
BOOST_FIXTURE_TEST_SUITE( poisson_loglikelihood_suite, poisson_loglikelihood_fixture )

BOOST_AUTO_TEST_CASE( test_calcLoglikelihood ) {
    BOOST_REQUIRE_CLOSE( PoissonLikelihoodCalculator(*_data_hub).CalcLogLikelihood(10, 1.0), 14.060436, 0.001 );
    BOOST_REQUIRE_CLOSE( PoissonLikelihoodCalculator(*_data_hub).CalcLogLikelihood(2, 1.0), 0.386720, 0.001 );
}

BOOST_AUTO_TEST_SUITE_END()

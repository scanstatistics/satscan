
// project files
#include "fixture_sampledata.h"

/* Test Suite for the Parameters validation class. */
BOOST_AUTO_TEST_SUITE( sampledata_validation_suite )

/* Tests parameters validation of nm (New Mexico) sample data set. */
BOOST_AUTO_TEST_CASE( test_sample_nm ) {
    new_mexico_fixture f;
}

/* Tests parameters validation of NHumberside sample data set. */
BOOST_AUTO_TEST_CASE( test_sample_NHumberside ) {
    nhumberside_fixture f;
}

/* Tests parameters validation of NYCfever sample data set. */
BOOST_AUTO_TEST_CASE( test_sample_NYCfever ) {
    nycfever_fixture f;
}

/* Tests parameters validation of NormalFake sample data set. */
BOOST_AUTO_TEST_CASE( test_sample_NormalFake ) {
    normalfake_fixture f;
}

/* Tests parameters validation of SurvivalFake sample data set. */
BOOST_AUTO_TEST_CASE( test_sample_SurvivalFake ) {
    survivalfake_fixture f;
}

/* Tests parameters validation of MarylandEducation sample data set. */
BOOST_AUTO_TEST_CASE( test_sample_MarylandEducation ) {
    marylandeducation_fixture f;
}

/* Tests parameters validation of ContinuousPoissonFake sample data set. */
BOOST_AUTO_TEST_CASE( test_sample_ContinuousPoissonFake ) {
    continuouspoissonfake_fixture f;
}

BOOST_AUTO_TEST_SUITE_END()

/* Test Suite for the Parameters validation class. */
BOOST_AUTO_TEST_SUITE( parameter_validation_oliveira_suite )

/* Tests parameters validation requires purely spatial analysis with oliveira. */
BOOST_FIXTURE_TEST_CASE( test_oliveira_analysis_type, new_mexico_fixture ) {
    // nm data set uses the Poisson model and space-time analysis initially
    _parameters.setCalculateOliveirasF(true);
    // request either "Location Information" and "Risk Estimates for Each Location" files -- feature diabled otherwise
    _parameters.SetOutputRelativeRisksAscii(true);

    // should fail since Oliveira only implemented for purely spatial analysis
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), false );
    _parameters.SetAnalysisType(PURELYSPATIAL);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );
}

/* Tests parameters validation of oliveira -- oliveira calculation disabled if known of the additional output files are requested. */
BOOST_FIXTURE_TEST_CASE( test_oliveira_additional_output_files, new_mexico_fixture ) {
    // nm data set uses the Poisson model and space-time analysis initially - so set to purely spatial analysis
    _parameters.SetAnalysisType(PURELYSPATIAL);

    // set oliveira parameter to true -- still should validate but validation should be reset of off since
    // we're not requesting either "Location Information" and "Risk Estimates for Each Location" files -- feature disabled otherwise
    _parameters.setCalculateOliveirasF(true);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );
    BOOST_CHECK_EQUAL( _parameters.getCalculateOliveirasF(), false );
    // request "Risk Estimates for Each Location" files
    _parameters.SetOutputRelativeRisksAscii(true);
    _parameters.setCalculateOliveirasF(true);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );
    BOOST_CHECK_EQUAL( _parameters.getCalculateOliveirasF(), true );
}

/* Tests parameters validation of oliveira -- test number of monte carlo replications and oliveira data sets. */
BOOST_FIXTURE_TEST_CASE( test_oliveira_monte_carlos, new_mexico_fixture ) {
    // nm data set uses the Poisson model and space-time analysis initially - so set to purely spatial analysis
    _parameters.SetAnalysisType(PURELYSPATIAL);
    _parameters.setCalculateOliveirasF(true);
    // request "Risk Estimates for Each Location" files
    _parameters.SetOutputRelativeRisksAscii(true);

    // current parameters should validate
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    // test number of monte carlo replications
    _parameters.SetNumberMonteCarloReplications(0);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), false );
    _parameters.SetNumberMonteCarloReplications(9);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), false );
    _parameters.SetNumberMonteCarloReplications(98);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), false );
    _parameters.SetNumberMonteCarloReplications(99);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );
}

/* Tests parameters validation of oliveira -- test number of monte carlo replications and oliveira data sets. */
BOOST_FIXTURE_TEST_CASE( test_oliveira_num_datasets, new_mexico_fixture ) {
    // nm data set uses the Poisson model and space-time analysis initially - so set to purely spatial analysis
    _parameters.SetAnalysisType(PURELYSPATIAL);
    _parameters.setCalculateOliveirasF(true);
    // request "Risk Estimates for Each Location" files
    _parameters.SetOutputRelativeRisksAscii(true);

    // current parameters should validate
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    // test number of oliveira data sets -- should require at least 100 or multiple of 100
    _parameters.setNumRequestedOliveiraSets(0);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), false );
    _parameters.setNumRequestedOliveiraSets(9);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), false );
    _parameters.setNumRequestedOliveiraSets(100);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );
    _parameters.setNumRequestedOliveiraSets(101);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), false );
    _parameters.setNumRequestedOliveiraSets(200);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );
    _parameters.setNumRequestedOliveiraSets(2000);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );
    _parameters.setNumRequestedOliveiraSets(10000);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );
}

/* Tests parameters validation of oliveira -- test p-value cutoff. */
BOOST_FIXTURE_TEST_CASE( test_oliveira_p_value_cutoff, new_mexico_fixture ) {
    // nm data set uses the Poisson model and space-time analysis initially - so set to purely spatial analysis
    _parameters.SetAnalysisType(PURELYSPATIAL);
    _parameters.setCalculateOliveirasF(true);
    // request "Risk Estimates for Each Location" files
    _parameters.SetOutputRelativeRisksAscii(true);

    // current parameters should validate
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    // test p-value cutoff -- required to be value in range [0,1]
    _parameters.setOliveiraPvalueCutoff(0.0);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );
    _parameters.setOliveiraPvalueCutoff(1.0);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );
    _parameters.setOliveiraPvalueCutoff(-0.1);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), false );
    _parameters.setOliveiraPvalueCutoff(1.1);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), false );
}
BOOST_AUTO_TEST_SUITE_END()

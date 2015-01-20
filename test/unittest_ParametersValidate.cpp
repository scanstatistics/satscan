
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
    // ensure assumption that we're not already requesting neither "Location Information" nor "Risk Estimates for Each Location" files
    BOOST_REQUIRE(_parameters.GetOutputAreaSpecificFiles() == false && _parameters.GetOutputRelativeRisksFiles() == false);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );
    BOOST_CHECK_EQUAL( _parameters.getCalculateOliveirasF(), true );
    // validation process should have turned on "Risk Estimates for Each Location" for ascii file
    BOOST_CHECK_EQUAL( _parameters.GetOutputRelativeRisksAscii(), true );
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

/* Tests parameters validation of closed loop analysis. */
BOOST_FIXTURE_TEST_CASE( test_closed_loop_analysis, parameter_fixture ) {
    std::string path(getTestSetFilesPath());
    std::stringstream filename;
    filename << path.c_str() << "\\closed-loop\\poisson.cas";
    _parameters.SetCaseFileName(filename.str().c_str());
    _parameters.SetControlFileName(filename.str().c_str());
    _parameters.SetPopulationFileName(filename.str().c_str());
    _parameters.SetPrecisionOfTimesType(DAY);
    _parameters.SetStudyPeriodStartDate("2000/1/1");
    _parameters.SetStudyPeriodEndDate("2010/12/31");
    _parameters.SetAnalysisType(SEASONALTEMPORAL);
    _parameters.SetTimeAggregationUnitsType(DAY);
    _parameters.SetTimeAggregationLength(1);
    _parameters.setReportGiniOptimizedClusters(false);

    filename.str("");
    filename << GetUserTemporaryDirectory(path).c_str() << "test.txt";
    _parameters.SetOutputFileName(filename.str().c_str());

    // current parameters should validate for Poisson, Bernoulli, Ordinal, Normal, Exponetial, Multinomial, Rank models
    _parameters.SetProbabilityModelType(POISSON);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );
    _parameters.SetProbabilityModelType(BERNOULLI);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );
    _parameters.SetProbabilityModelType(ORDINAL);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );
    _parameters.SetProbabilityModelType(EXPONENTIAL);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );
    _parameters.SetProbabilityModelType(NORMAL);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );
    _parameters.SetProbabilityModelType(CATEGORICAL);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );
    _parameters.SetProbabilityModelType(RANK);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    // closed loop analyses require month, day or generic precision of times
    _parameters.SetPrecisionOfTimesType(NONE);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), false );
    _parameters.SetPrecisionOfTimesType(YEAR);
    _parameters.SetTimeAggregationUnitsType(YEAR);
    _parameters.SetTimeAggregationLength(1);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), false );
    _parameters.SetPrecisionOfTimesType(MONTH);
    _parameters.SetTimeAggregationUnitsType(MONTH);
    _parameters.SetTimeAggregationLength(1);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );
    _parameters.SetPrecisionOfTimesType(DAY);
    _parameters.SetTimeAggregationUnitsType(DAY);
    _parameters.SetTimeAggregationLength(1);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );
    _parameters.SetStudyPeriodStartDate("0");
    _parameters.SetStudyPeriodEndDate("50");
    _parameters.SetPrecisionOfTimesType(GENERIC);
    _parameters.SetTimeAggregationUnitsType(GENERIC);
    _parameters.SetTimeAggregationLength(1);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    // adjustment for weekly trends is not available with this analysis
    _parameters.SetProbabilityModelType(POISSON);
    _parameters.SetPrecisionOfTimesType(DAY);
    _parameters.SetStudyPeriodStartDate("2000/1/1");
    _parameters.SetStudyPeriodEndDate("2010/12/31");
    _parameters.SetTimeAggregationUnitsType(DAY);
    _parameters.SetTimeAggregationLength(1);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );
    _parameters.setAdjustForWeeklyTrends(true);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), false );
    _parameters.setAdjustForWeeklyTrends(false);

    // flexible windows are not implemented for closed loop analyses
    _parameters.SetIncludeClustersType(CLUSTERSINRANGE);
    _parameters.SetStartRangeStartDate("2000/1/1");
    _parameters.SetStartRangeEndDate("2010/12/31");
    _parameters.SetEndRangeStartDate("2000/1/1");
    _parameters.SetEndRangeEndDate("2010/12/31");
    _parameters.SetAnalysisType(PURELYTEMPORAL);
    // validate settings for purely temporal
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );
    // ... but not for closed loop analysis
    _parameters.SetAnalysisType(SEASONALTEMPORAL);
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), false );
    _parameters.SetIncludeClustersType(ALLCLUSTERS);
}

BOOST_AUTO_TEST_SUITE_END()

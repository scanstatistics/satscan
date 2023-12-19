
// project files
#include "fixture_sampledata.h"
#include "AnalysisRun.h"
#include "Toolkit.h"
#include "ClusterInformationWriter.h"
#include <boost/filesystem.hpp>

/* restricted clusters by risk level: https://www.squishlist.com/ims/satscan/66425/ */

/* Test Suite for the option to restrict clusters by risk level. */
BOOST_FIXTURE_TEST_SUITE( restricted_cluster_risk_level_suite, new_mexico_fixture)

/* Test restricting cluster levels for high threshold of 1.0 has no effect.  */
BOOST_AUTO_TEST_CASE( test_restricting_1_no_effect_high_rate ) {
    std::string results_user_directory, buffer1, buffer2;

    _parameters.SetAreaRateType(HIGH);
    _parameters.SetOutputClusterLevelAscii(true);
    _parameters.setRiskLimitHighClusters(false);
    _parameters.setRiskLimitLowClusters(false);
    run_analysis("test1", results_user_directory, _parameters, _print);
    FileName test1_filename(_parameters.GetOutputFileName().c_str());
    test1_filename.setExtension(printString(buffer1, "%s%s", ClusterInformationWriter::CLUSTER_FILE_EXT, ASCIIDataFileWriter::ASCII_FILE_EXT).c_str());

    _parameters.setRiskLimitHighClusters(true);
    _parameters.setRiskThresholdHighClusters(1.0);
    _parameters.setRiskLimitLowClusters(true);
    _parameters.setRiskThresholdLowClusters(1.0);
    run_analysis("test2", results_user_directory, _parameters, _print);
    FileName test2_filename(_parameters.GetOutputFileName().c_str());
    test2_filename.setExtension(printString(buffer2, "%s%s", ClusterInformationWriter::CLUSTER_FILE_EXT, ASCIIDataFileWriter::ASCII_FILE_EXT).c_str());
    BOOST_CHECK_MESSAGE(compare_files(test1_filename.getFullPath(buffer1), test2_filename.getFullPath(buffer2)) == true, "files do not match");
}

/* Test restricting cluster levels for low threshold of 1.0 has no effect.  */
BOOST_AUTO_TEST_CASE(test_restricting_1_no_effect_low_rate) {
    std::string results_user_directory, buffer1, buffer2;

    _parameters.SetAreaRateType(LOW);
    _parameters.SetOutputClusterLevelAscii(true);
    _parameters.setRiskLimitHighClusters(false);
    _parameters.setRiskLimitLowClusters(false);
    run_analysis("test1", results_user_directory, _parameters, _print);
    FileName test1_filename(_parameters.GetOutputFileName().c_str());
    test1_filename.setExtension(printString(buffer1, "%s%s", ClusterInformationWriter::CLUSTER_FILE_EXT, ASCIIDataFileWriter::ASCII_FILE_EXT).c_str());

    _parameters.setRiskLimitHighClusters(true);
    _parameters.setRiskThresholdHighClusters(1.0);
    _parameters.setRiskLimitLowClusters(true);
    _parameters.setRiskThresholdLowClusters(1.0);
    run_analysis("test2", results_user_directory, _parameters, _print);
    FileName test2_filename(_parameters.GetOutputFileName().c_str());
    test2_filename.setExtension(printString(buffer2, "%s%s", ClusterInformationWriter::CLUSTER_FILE_EXT, ASCIIDataFileWriter::ASCII_FILE_EXT).c_str());
    BOOST_CHECK_MESSAGE(compare_files(test1_filename.getFullPath(buffer1), test2_filename.getFullPath(buffer2)) == true, "files do not match");
}

/* Test restricting cluster levels for both low and high thresholds of 1.0 has no effect.  */
BOOST_AUTO_TEST_CASE(test_restricting_1_no_effect_highlow_rate) {
    std::string results_user_directory, buffer1, buffer2;

    _parameters.SetAreaRateType(HIGHANDLOW);
    _parameters.SetOutputClusterLevelAscii(true);
    _parameters.setRiskLimitHighClusters(false);
    _parameters.setRiskLimitLowClusters(false);
    run_analysis("test1", results_user_directory, _parameters, _print);
    FileName test1_filename(_parameters.GetOutputFileName().c_str());
    test1_filename.setExtension(printString(buffer1, "%s%s", ClusterInformationWriter::CLUSTER_FILE_EXT, ASCIIDataFileWriter::ASCII_FILE_EXT).c_str());

    _parameters.setRiskLimitHighClusters(true);
    _parameters.setRiskThresholdHighClusters(1.0);
    _parameters.setRiskLimitLowClusters(true);
    _parameters.setRiskThresholdLowClusters(1.0);
    run_analysis("test2", results_user_directory, _parameters, _print);
    FileName test2_filename(_parameters.GetOutputFileName().c_str());
    test2_filename.setExtension(printString(buffer2, "%s%s", ClusterInformationWriter::CLUSTER_FILE_EXT, ASCIIDataFileWriter::ASCII_FILE_EXT).c_str());
    BOOST_CHECK_MESSAGE(compare_files(test1_filename.getFullPath(buffer1), test2_filename.getFullPath(buffer2)) == true, "files do not match");
}

/* Test restricting cluster levels for high clusters by relative risk.  */
BOOST_AUTO_TEST_CASE(test_restricting_high_rate_relative_risk) {
    std::string results_user_directory, buffer1, buffer2;

    // Perform baseline analysis -- no restricting of clusters by risk level.
    _parameters.SetAreaRateType(HIGH);
    _parameters.setPrintAsciiHeaders(true);
    _parameters.SetOutputClusterLevelAscii(true);
    _parameters.setRiskLimitHighClusters(false);
    _parameters.setRiskLimitLowClusters(false);
    run_analysis("test1", results_user_directory, _parameters, _print);

    // open the cluster information file and get the relative risk of first record
    std::ifstream stream;
    getFileStream(stream, printString(buffer1, "test1%s%s", ClusterInformationWriter::CLUSTER_FILE_EXT, ASCIIDataFileWriter::ASCII_FILE_EXT), results_user_directory);
    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itr = std::find(headers.begin(), headers.end(), std::string(ClusterInformationWriter::RELATIVE_RISK_FIELD));
    if (itr == headers.end()) BOOST_FAIL("relative risk column not found");
    // get the relative risk of the first cluster - MLC
    CSV_Row_t data;
    getCSVRow(stream, data);
    double rr;
    BOOST_CHECK(string_to_type<double>(data.at(std::distance(headers.begin(), itr)).c_str(), rr));
    stream.close();

    // Re-run the analysis but restrict high clusters by risk level greater than MLC relative risk.
    _parameters.setRiskLimitHighClusters(true);
    _parameters.setRiskThresholdHighClusters(rr + 0.1);
    run_analysis("test2", results_user_directory, _parameters, _print);

    // open the cluster information file and confirm that all clusters reported have relative risk greater than threshold
    getFileStream(stream, printString(buffer2, "test2%s%s", ClusterInformationWriter::CLUSTER_FILE_EXT, ASCIIDataFileWriter::ASCII_FILE_EXT), results_user_directory);
    getCSVRow(stream, headers);
    itr = std::find(headers.begin(), headers.end(), std::string(ClusterInformationWriter::RELATIVE_RISK_FIELD));
    if (itr == headers.end()) BOOST_FAIL("relative risk column not found");
    getCSVRow(stream, data);
    while (data.size()) {
            BOOST_CHECK(string_to_type<double>(data.at(std::distance(headers.begin(), itr)).c_str(), rr));
            BOOST_REQUIRE_GT(rr, _parameters.getRiskThresholdHighClusters());
        getCSVRow(stream, data);
    }
    stream.close();
}

/* Test restricting cluster levels for high clusters by observed/expected.  */
BOOST_AUTO_TEST_CASE(test_restricting_high_rate_ode) {
    std::string results_user_directory, buffer1, buffer2;

    // Perform baseline analysis -- no restricting of clusters by risk level.
    _parameters.SetProbabilityModelType(SPACETIMEPERMUTATION);
    _parameters.SetAnalysisType(SPACETIME);
    _parameters.SetAreaRateType(HIGH);
    _parameters.setPrintAsciiHeaders(true);
    _parameters.SetOutputClusterLevelAscii(true);
    _parameters.setRiskLimitHighClusters(false);
    _parameters.setRiskLimitLowClusters(false);
    run_analysis("test1", results_user_directory, _parameters, _print);

    // open the cluster information file and get the observed/expected of first record
    std::ifstream stream;
    getFileStream(stream, printString(buffer1, "test1%s%s", ClusterInformationWriter::CLUSTER_FILE_EXT, ASCIIDataFileWriter::ASCII_FILE_EXT), results_user_directory);
    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itr = std::find(headers.begin(), headers.end(), std::string(ClusterInformationWriter::RELATIVE_RISK_FIELD));
    if (itr != headers.end()) BOOST_FAIL("relative risk column should not exist");
    itr = std::find(headers.begin(), headers.end(), std::string(ClusterInformationWriter::OBSERVED_DIV_EXPECTED_FIELD));
    if (itr == headers.end()) BOOST_FAIL("observed/expected column not found");
    // get the observed/expected of the first cluster - MLC
    CSV_Row_t data;
    getCSVRow(stream, data);
    double ode;
    BOOST_CHECK(string_to_type<double>(data.at(std::distance(headers.begin(), itr)).c_str(), ode));
    stream.close();

    // Re-run the analysis but restrict high clusters by risk level greater than MLC observed/expected.
    _parameters.setRiskLimitHighClusters(true);
    _parameters.setRiskThresholdHighClusters(ode + 0.1);
    run_analysis("test2", results_user_directory, _parameters, _print);

    // open the cluster information file and confirm that all clusters reported have observed/expected greater than threshold
    getFileStream(stream, printString(buffer2, "test2%s%s", ClusterInformationWriter::CLUSTER_FILE_EXT, ASCIIDataFileWriter::ASCII_FILE_EXT), results_user_directory);
    getCSVRow(stream, headers);
    itr = std::find(headers.begin(), headers.end(), std::string(ClusterInformationWriter::RELATIVE_RISK_FIELD));
    if (itr != headers.end()) BOOST_FAIL("relative risk column should not exist");
    itr = std::find(headers.begin(), headers.end(), std::string(ClusterInformationWriter::OBSERVED_DIV_EXPECTED_FIELD));
    if (itr == headers.end()) BOOST_FAIL("observed/expectedcolumn not found");
    getCSVRow(stream, data);
    while (data.size()) {
        BOOST_CHECK(string_to_type<double>(data.at(std::distance(headers.begin(), itr)).c_str(), ode));
        BOOST_REQUIRE_GT(ode, _parameters.getRiskThresholdHighClusters());
        getCSVRow(stream, data);
    }
    stream.close();
}

/* Test restricting cluster levels for low clusters by relative risk.  */
BOOST_AUTO_TEST_CASE(test_restricting_low_rate_relative_risk) {
    std::string results_user_directory, buffer1, buffer2;

    // Perform baseline analysis -- no restricting of clusters by risk level.
    _parameters.SetAreaRateType(LOW);
    _parameters.setPrintAsciiHeaders(true);
    _parameters.SetOutputClusterLevelAscii(true);
    _parameters.setRiskLimitHighClusters(false);
    _parameters.setRiskLimitLowClusters(false);
    run_analysis("test1", results_user_directory, _parameters, _print);

    // open the cluster information file and get the relative risk of first record
    std::ifstream stream;
    getFileStream(stream, printString(buffer1, "test1%s%s", ClusterInformationWriter::CLUSTER_FILE_EXT, ASCIIDataFileWriter::ASCII_FILE_EXT), results_user_directory);
    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itr = std::find(headers.begin(), headers.end(), std::string(ClusterInformationWriter::RELATIVE_RISK_FIELD));
    if (itr == headers.end()) BOOST_FAIL("relative risk column not found");
    // get the relative risk of the first cluster - MLC
    CSV_Row_t data;
    getCSVRow(stream, data);
    double rr;
    BOOST_CHECK(string_to_type<double>(data.at(std::distance(headers.begin(), itr)).c_str(), rr));
    stream.close();

    // Re-run the analysis but restrict low clusters by risk level less than MLC relative risk.
    _parameters.setRiskLimitLowClusters(true);
    _parameters.setRiskThresholdLowClusters(rr - 0.1);
    run_analysis("test2", results_user_directory, _parameters, _print);

    // open the cluster information file and confirm that all clusters reported have relative risk less than threshold
    getFileStream(stream, printString(buffer2, "test2%s%s", ClusterInformationWriter::CLUSTER_FILE_EXT, ASCIIDataFileWriter::ASCII_FILE_EXT), results_user_directory);
    getCSVRow(stream, headers);
    itr = std::find(headers.begin(), headers.end(), std::string(ClusterInformationWriter::RELATIVE_RISK_FIELD));
    if (itr == headers.end()) BOOST_FAIL("relative risk column not found");
    getCSVRow(stream, data);
    while (data.size()) {
        BOOST_CHECK(string_to_type<double>(data.at(std::distance(headers.begin(), itr)).c_str(), rr));
        BOOST_REQUIRE_LT(rr, _parameters.getRiskThresholdHighClusters());
        getCSVRow(stream, data);
    }
    stream.close();
}

/* Test restricting cluster levels for low clusters by observed/expected.  */
BOOST_AUTO_TEST_CASE(test_restricting_low_rate_ode) {
    std::string results_user_directory, buffer1, buffer2;

    // Perform baseline analysis -- no restricting of clusters by risk level.
    _parameters.SetProbabilityModelType(SPACETIMEPERMUTATION);
    _parameters.SetAnalysisType(PROSPECTIVESPACETIME);
    _parameters.SetAreaRateType(LOW);
    _parameters.setPrintAsciiHeaders(true);
    _parameters.SetOutputClusterLevelAscii(true);
    _parameters.setRiskLimitHighClusters(false);
    _parameters.setRiskLimitLowClusters(false);
    run_analysis("test1", results_user_directory, _parameters, _print);

    // open the cluster information file and get the relative risk of first record
    std::ifstream stream;
    getFileStream(stream, printString(buffer1, "test1%s%s", ClusterInformationWriter::CLUSTER_FILE_EXT, ASCIIDataFileWriter::ASCII_FILE_EXT), results_user_directory);
    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itr = std::find(headers.begin(), headers.end(), std::string(ClusterInformationWriter::RELATIVE_RISK_FIELD));
    if (itr != headers.end()) BOOST_FAIL("relative risk should not exist");
    itr = std::find(headers.begin(), headers.end(), std::string(ClusterInformationWriter::OBSERVED_DIV_EXPECTED_FIELD));
    if (itr == headers.end()) BOOST_FAIL("observed/expected column not found");
    // get the observed/expected of the first cluster - MLC
    CSV_Row_t data;
    getCSVRow(stream, data);
    double ode;
    BOOST_CHECK(string_to_type<double>(data.at(std::distance(headers.begin(), itr)).c_str(), ode));
    stream.close();

    // Re-run the analysis but restrict low clusters by risk level less than MLC observed/expected.
    _parameters.setRiskLimitLowClusters(true);
    _parameters.setRiskThresholdLowClusters(ode - 0.1);
    run_analysis("test2", results_user_directory, _parameters, _print);

    // open the cluster information file and confirm that all clusters reported have observed/expected less than threshold
    getFileStream(stream, printString(buffer2, "test2%s%s", ClusterInformationWriter::CLUSTER_FILE_EXT, ASCIIDataFileWriter::ASCII_FILE_EXT), results_user_directory);
    getCSVRow(stream, headers);
    itr = std::find(headers.begin(), headers.end(), std::string(ClusterInformationWriter::RELATIVE_RISK_FIELD));
    if (itr != headers.end()) BOOST_FAIL("relative risk should not exist");
    itr = std::find(headers.begin(), headers.end(), std::string(ClusterInformationWriter::OBSERVED_DIV_EXPECTED_FIELD));
    if (itr == headers.end()) BOOST_FAIL("observed/expected column not found");
    getCSVRow(stream, data);
    while (data.size()) {
        BOOST_CHECK(string_to_type<double>(data.at(std::distance(headers.begin(), itr)).c_str(), ode));
        BOOST_REQUIRE_LT(ode, _parameters.getRiskThresholdHighClusters());
        getCSVRow(stream, data);
    }
    stream.close();
}

/* Test restricting cluster levels for:
     low clusters by relative risk while scanning for high or low rate clusters
     high clusters by relative risk while scanning for high or low rate clusters */
BOOST_AUTO_TEST_CASE(test_restricting_mixed_risk_rate) {
    std::string results_user_directory, buffer;

    // Perform baseline analysis -- no restricting of clusters by risk level, only scanning for high and low clusters.
    _parameters.SetAreaRateType(HIGHANDLOW);
    _parameters.setPrintAsciiHeaders(true);
    _parameters.SetOutputClusterLevelAscii(true);
    _parameters.setRiskLimitHighClusters(false);
    _parameters.setRiskLimitLowClusters(false);
    _parameters.SetNumberMonteCarloReplications(0);
    run_analysis("test1", results_user_directory, _parameters, _print);

    // open the cluster information file and get the relative risk of first record high cluster and low cluster
    std::ifstream stream;
    getFileStream(stream, printString(buffer, "test1%s%s", ClusterInformationWriter::CLUSTER_FILE_EXT, ASCIIDataFileWriter::ASCII_FILE_EXT), results_user_directory);
    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itrRR = std::find(headers.begin(), headers.end(), std::string(ClusterInformationWriter::RELATIVE_RISK_FIELD));
    if (itrRR == headers.end()) BOOST_FAIL("relative risk column not found");
    std::vector<std::string>::iterator itrOBS = std::find(headers.begin(), headers.end(), std::string(ClusterInformationWriter::OBSERVED_FIELD));
    if (itrOBS == headers.end()) BOOST_FAIL("observed column not found");
    std::vector<std::string>::iterator itrEXP = std::find(headers.begin(), headers.end(), std::string(ClusterInformationWriter::EXPECTED_FIELD));
    if (itrEXP == headers.end()) BOOST_FAIL("expected column not found");
    double rr, high_rr=-std::numeric_limits<double>::max(), low_rr=-std::numeric_limits<double>::max(), observed, expected;
    CSV_Row_t data;
    getCSVRow(stream, data);
    while (data.size()) {
        BOOST_CHECK(string_to_type<double>(data.at(std::distance(headers.begin(), itrOBS)).c_str(), observed));
        BOOST_CHECK(string_to_type<double>(data.at(std::distance(headers.begin(), itrEXP)).c_str(), expected));
        BOOST_CHECK(string_to_type<double>(data.at(std::distance(headers.begin(), itrRR)).c_str(), rr));
        if (observed > expected)
            high_rr = std::max(rr, high_rr);
        else if (observed < expected)
            low_rr = std::max(rr, low_rr);
        getCSVRow(stream, data);
    }
    stream.close();
    // Make sure that there are both high and low clusters in the result.
    BOOST_CHECK(high_rr != -std::numeric_limits<double>::max() && low_rr != -std::numeric_limits<double>::max());

    // Re-run the analysis but restrict low clusters by risk level less than low rate cluster's relative risk.
    _parameters.setRiskLimitLowClusters(true);
    _parameters.setRiskThresholdLowClusters(low_rr - 0.1);
    run_analysis("test2", results_user_directory, _parameters, _print);

    // open the cluster information file and confirm that the low risk threshold was enforced but the high rate cluster still exists.
    getFileStream(stream, printString(buffer, "test2%s%s", ClusterInformationWriter::CLUSTER_FILE_EXT, ASCIIDataFileWriter::ASCII_FILE_EXT), results_user_directory);
    double high2_rr = -std::numeric_limits<double>::max(), low2_rr = -std::numeric_limits<double>::max();
    getCSVRow(stream, headers);
    itrRR = std::find(headers.begin(), headers.end(), std::string(ClusterInformationWriter::RELATIVE_RISK_FIELD));
    if (itrRR == headers.end()) BOOST_FAIL("relative risk column not found");
    itrOBS = std::find(headers.begin(), headers.end(), std::string(ClusterInformationWriter::OBSERVED_FIELD));
    if (itrOBS == headers.end()) BOOST_FAIL("observed column not found");
    itrEXP = std::find(headers.begin(), headers.end(), std::string(ClusterInformationWriter::EXPECTED_FIELD));
    if (itrEXP == headers.end()) BOOST_FAIL("expected column not found");
    getCSVRow(stream, data);
    while (data.size()) {
        BOOST_CHECK(string_to_type<double>(data.at(std::distance(headers.begin(), itrOBS)).c_str(), observed));
        BOOST_CHECK(string_to_type<double>(data.at(std::distance(headers.begin(), itrEXP)).c_str(), expected));
        BOOST_CHECK(string_to_type<double>(data.at(std::distance(headers.begin(), itrRR)).c_str(), rr));
        if (observed > expected)
            high2_rr = std::max(rr, high2_rr);
        else if (observed < expected)
            low2_rr = std::max(rr, low2_rr);
        getCSVRow(stream, data);
    }
    stream.close();
    // Make sure that there are both high and low clusters in the result.
    BOOST_CHECK(high2_rr != -std::numeric_limits<double>::max() && low2_rr != -std::numeric_limits<double>::max());
    // Make sure that the low cluster's relative risk is less than or equal to risk restriction.
    BOOST_CHECK(low2_rr <= _parameters.getRiskThresholdLowClusters());

    // Re-run the analysis but restrict low clusters by risk level less than low rate cluster's relative risk.
    _parameters.setRiskLimitLowClusters(false);
    _parameters.setRiskLimitHighClusters(true);
    _parameters.setRiskThresholdHighClusters(high_rr + 0.1);
    run_analysis("test3", results_user_directory, _parameters, _print);

    // open the cluster information file and confirm that the high risk threshold was enforced but the low rate cluster still exists.
    getFileStream(stream, printString(buffer, "test3%s%s", ClusterInformationWriter::CLUSTER_FILE_EXT, ASCIIDataFileWriter::ASCII_FILE_EXT), results_user_directory);
    double high3_rr = -std::numeric_limits<double>::max(), low3_rr = -std::numeric_limits<double>::max();
    getCSVRow(stream, headers);
    itrRR = std::find(headers.begin(), headers.end(), std::string(ClusterInformationWriter::RELATIVE_RISK_FIELD));
    if (itrRR == headers.end()) BOOST_FAIL("relative risk column not found");
    itrOBS = std::find(headers.begin(), headers.end(), std::string(ClusterInformationWriter::OBSERVED_FIELD));
    if (itrOBS == headers.end()) BOOST_FAIL("observed column not found");
    itrEXP = std::find(headers.begin(), headers.end(), std::string(ClusterInformationWriter::EXPECTED_FIELD));
    if (itrEXP == headers.end()) BOOST_FAIL("expected column not found");
    getCSVRow(stream, data);
    while (data.size()) {
        BOOST_CHECK(string_to_type<double>(data.at(std::distance(headers.begin(), itrOBS)).c_str(), observed));
        BOOST_CHECK(string_to_type<double>(data.at(std::distance(headers.begin(), itrEXP)).c_str(), expected));
        BOOST_CHECK(string_to_type<double>(data.at(std::distance(headers.begin(), itrRR)).c_str(), rr));
        if (observed > expected)
            high3_rr = std::max(rr, high3_rr);
        else if (observed < expected)
            low3_rr = std::max(rr, low3_rr);
        getCSVRow(stream, data);
    }
    stream.close();
    // Make sure that there are both high and low clusters in the result.
    BOOST_CHECK(high3_rr != -std::numeric_limits<double>::max() && low3_rr != -std::numeric_limits<double>::max());
    // Make sure that the high cluster's relative risk is more than or equal risk restriction.
    BOOST_CHECK(high3_rr >= _parameters.getRiskThresholdHighClusters());
}

BOOST_AUTO_TEST_SUITE_END()


// project files
#include "fixture_sampledata.h"
#include "AnalysisRun.h"
#include "Toolkit.h"
#include "ParametersValidate.h"
#include "ClusterInformationWriter.h"
#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

/* batched model integration tests */

struct batched_fixture : prm_testset_fixture {
    batched_fixture() : prm_testset_fixture("batched\\batched.prm") { }
    virtual ~batched_fixture() { }
};

/* Test Suite for batched model. */
BOOST_FIXTURE_TEST_SUITE(batched_model_suite, batched_fixture)

/* Test batched model parameter options. */
BOOST_AUTO_TEST_CASE(test_parameter_settings) {
    _parameters.SetAnalysisType(SPACETIME);
    // Test maximum temporal cluster size.
    _parameters.SetMaximumTemporalClusterSize(50);
    BOOST_CHECK(ParametersValidate(_parameters).Validate(_print) == true);
    // maximum should be 90% with no temporal adjustment
    _parameters.SetTimeTrendAdjustmentType(TEMPORAL_NOTADJUSTED);
    _parameters.SetMaximumTemporalClusterSize(100);
    BOOST_CHECK(ParametersValidate(_parameters).Validate(_print) == false);
    _parameters.SetMaximumTemporalClusterSize(90);
    BOOST_CHECK(ParametersValidate(_parameters).Validate(_print) == true);
    _parameters.SetTimeTrendAdjustmentType(TEMPORAL_STRATIFIED_RANDOMIZATION);
    _parameters.SetMaximumTemporalClusterSize(100);
    BOOST_CHECK(ParametersValidate(_parameters).Validate(_print) == true);
}

/* Test batched model analysis results. */
BOOST_AUTO_TEST_CASE(test_retrospective_spacetime_run) {
    std::string results_user_directory, buffer;
    _parameters.SetAnalysisType(SPACETIME);
    _parameters.setPrintAsciiHeaders(true);
    _parameters.SetOutputClusterLevelAscii(true);
    run_analysis("test1", results_user_directory, _parameters, _print);

    // open the cluster information file and confirm key columns
    std::ifstream stream;
    double test_val;
    CSV_Row_t headers, data;
    unsigned int rec_count = 0;
    getFileStream(stream, printString(buffer, "test1%s%s", ClusterInformationWriter::CLUSTER_FILE_EXT, ASCIIDataFileWriter::ASCII_FILE_EXT), results_user_directory);
    getCSVRow(stream, headers);
    auto get_data_at = [&data, &headers](const std::string& label) {
        return data.at(std::distance(headers.begin(), std::find(headers.begin(), headers.end(), label)));
    };
    getCSVRow(stream, data);
    while (data.size()) {
        ++rec_count;
        switch (rec_count) {
            case 1:
                BOOST_CHECK(get_data_at(*(headers.begin() + 1)) == "TempleIsaiah"); // central location
                BOOST_CHECK(get_data_at(*(headers.begin() + 9)) == "7.711344"); // log-likelihood ratio
                BOOST_CHECK(get_data_at(*(headers.begin() + 10)) == "0.00200"); // p-value
                BOOST_CHECK(get_data_at(*(headers.begin() + 11)) == "6"); // number of batches
                BOOST_CHECK(get_data_at(*(headers.begin() + 12)) == "6"); // observed
                buffer = get_data_at(*(headers.begin() + 13)); // expected
                BOOST_CHECK(string_to_type<double>(buffer.c_str(), test_val));
                BOOST_REQUIRE_CLOSE(test_val, 2.56, 0.001);
                buffer = get_data_at(*(headers.begin() + 14)); // ode
                BOOST_CHECK(string_to_type<double>(buffer.c_str(), test_val));
                BOOST_REQUIRE_CLOSE(test_val, 2.34, 0.001);
                buffer = get_data_at(*(headers.begin() + 15)); // probability positive inside
                BOOST_CHECK(string_to_type<double>(buffer.c_str(), test_val));
                BOOST_REQUIRE_CLOSE(test_val, 1, 0.001);
                buffer = get_data_at(*(headers.begin() + 16)); // probability positive outside
                BOOST_CHECK(string_to_type<double>(buffer.c_str(), test_val));
                BOOST_REQUIRE_CLOSE(test_val, 0.0094070467, 0.00000001);
                break;
            case 2:
                BOOST_CHECK(get_data_at(*(headers.begin() + 1)) == "Ananda"); // central location
                BOOST_CHECK(get_data_at(*(headers.begin() + 9)) == "1.615776"); // log-likelihood ratio
                BOOST_CHECK(get_data_at(*(headers.begin() + 10)) == "0.957"); // p-value
                BOOST_CHECK(get_data_at(*(headers.begin() + 11)) == "2"); // number of batches
                BOOST_CHECK(get_data_at(*(headers.begin() + 12)) == "2"); // observed
                buffer = get_data_at(*(headers.begin() + 13)); // expected
                BOOST_CHECK(string_to_type<double>(buffer.c_str(), test_val));
                BOOST_REQUIRE_CLOSE(test_val, 1, 0.001);
                buffer = get_data_at(*(headers.begin() + 14)); // ode
                BOOST_CHECK(string_to_type<double>(buffer.c_str(), test_val));
                BOOST_REQUIRE_CLOSE(test_val, 2, 0.001);
                buffer = get_data_at(*(headers.begin() + 15)); // probability positive inside
                BOOST_CHECK(string_to_type<double>(buffer.c_str(), test_val));
                BOOST_REQUIRE_CLOSE(test_val, 1, 0.001);
                buffer = get_data_at(*(headers.begin() + 16)); // probability positive outside
                BOOST_CHECK(string_to_type<double>(buffer.c_str(), test_val));
                BOOST_REQUIRE_CLOSE(test_val, 0.016665509, 0.00000001);
                break;
        };
        getCSVRow(stream, data);
    }
    stream.close();
    BOOST_CHECK(rec_count == 2);
}

/* Test batched model analysis results. */
BOOST_AUTO_TEST_CASE(test_prospective_spacetime_run) {
    std::string results_user_directory, buffer;
    _parameters.SetAnalysisType(PROSPECTIVESPACETIME);
    _parameters.setPrintAsciiHeaders(true);
    _parameters.SetOutputClusterLevelAscii(true);
    run_analysis("test1", results_user_directory, _parameters, _print);

    // open the cluster information file and confirm key columns
    std::ifstream stream;
    CSV_Row_t headers, data;
    unsigned int rec_count = 0;
    double test_val;
    getFileStream(stream, printString(buffer, "test1%s%s", ClusterInformationWriter::CLUSTER_FILE_EXT, ASCIIDataFileWriter::ASCII_FILE_EXT), results_user_directory);
    getCSVRow(stream, headers);
    auto get_data_at = [&data, &headers](const std::string& label) {
        return data.at(std::distance(headers.begin(), std::find(headers.begin(), headers.end(), label)));
    };
    getCSVRow(stream, data);
    while (data.size()) {
        ++rec_count;
        BOOST_CHECK(get_data_at(*(headers.begin() + 1)) == "TempleIsaiah"); // central location
        BOOST_CHECK(get_data_at(*(headers.begin() + 9)) == "1.711060"); // log-likelihood ratio
        BOOST_CHECK(get_data_at(*(headers.begin() + 10)) == "0.215"); // p-value
        BOOST_CHECK(get_data_at(*(headers.begin() + 12)) == "2"); // number of batches
        BOOST_CHECK(get_data_at(*(headers.begin() + 13)) == "2"); // observed
        buffer = get_data_at(*(headers.begin() + 14)); // expected
        BOOST_CHECK(string_to_type<double>(buffer.c_str(), test_val));
        BOOST_REQUIRE_CLOSE(test_val, 0.93, 0.001);
        buffer = get_data_at(*(headers.begin() + 15)); // ode
        BOOST_CHECK(string_to_type<double>(buffer.c_str(), test_val));
        BOOST_REQUIRE_CLOSE(test_val, 2.16, 0.001);
        buffer = get_data_at(*(headers.begin() + 16)); // probability positive inside
        BOOST_CHECK(string_to_type<double>(buffer.c_str(), test_val));
        BOOST_REQUIRE_CLOSE(test_val, 1, 0.001);
        buffer = get_data_at(*(headers.begin() + 17)); // probability positive outside
        BOOST_CHECK(string_to_type<double>(buffer.c_str(), test_val));
        BOOST_REQUIRE_CLOSE(test_val, 0.016580638, 0.00000001);
        getCSVRow(stream, data);
    }
    stream.close();
    BOOST_CHECK(rec_count == 1);
}

BOOST_AUTO_TEST_SUITE_END()

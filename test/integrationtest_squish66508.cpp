
// project files
#include "fixture_sampledata.h"
#include "AnalysisRun.h"
#include "Toolkit.h"
#include "ClusterInformationWriter.h"
#include "ClusterLocationsWriter.h"
#include <boost/filesystem.hpp>

/* restricted clusters by risk level: https://www.squishlist.com/ims/satscan/66425/ */

/* Test Suite for the option to restrict clusters by risk level. */
BOOST_FIXTURE_TEST_SUITE(report_cluster_population, new_mexico_fixture)

/* Test population reported in cluster information file.  */
BOOST_AUTO_TEST_CASE(test_cluster_information_reported_population) {
    std::string results_user_directory, buffer1, buffer2;

    // Perform baseline analysis -- no restricting of clusters by risk level.
    _parameters.SetAreaRateType(HIGH);
    _parameters.setPrintAsciiHeaders(true);
    _parameters.SetOutputClusterLevelAscii(true);
    run_analysis("test1", results_user_directory, _parameters, _print);

    // open the cluster information file and compare population to expected
    std::ifstream stream;
    getFileStream(stream, printString(buffer1, "test1%s%s", ClusterInformationWriter::CLUSTER_FILE_EXT, ASCIIDataFileWriter::ASCII_FILE_EXT), results_user_directory);
    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itr = std::find(headers.begin(), headers.end(), std::string(ClusterInformationWriter::POPULATION_FIELD));
    if (itr == headers.end()) BOOST_FAIL("population column not found");
    CSV_Row_t data;
    getCSVRow(stream, data);
    double population;
    unsigned int rec_count = 0;
    while (data.size()) {
        ++rec_count;
        BOOST_CHECK(string_to_type<double>(data.at(std::distance(headers.begin(), itr)).c_str(), population));
        switch (rec_count) {
            case 1: BOOST_CHECK_EQUAL(population, 674891.72); break;
            case 2: BOOST_CHECK_EQUAL(population, 52755.68); break;
            case 3: BOOST_CHECK_EQUAL(population, 59349.33); break;
            default: BOOST_FAIL("unexpected record");
        }
        getCSVRow(stream, data);
    }
    stream.close();
}

/* Test population reported in cluster case information file.  */
BOOST_AUTO_TEST_CASE(test_cluster_case_information_reported_population) {
    std::string results_user_directory, buffer1, buffer2;

    // Perform baseline analysis -- no restricting of clusters by risk level.
    _parameters.SetAreaRateType(HIGH);
    _parameters.setPrintAsciiHeaders(true);
    _parameters.SetOutputClusterCaseAscii(true);
    run_analysis("test1", results_user_directory, _parameters, _print);

    // open the cluster information file and compare population to expected
    std::ifstream stream;
    getFileStream(stream, printString(buffer1, "test1%s%s", ClusterInformationWriter::CLUSTERCASE_FILE_EXT, ASCIIDataFileWriter::ASCII_FILE_EXT), results_user_directory);
    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itr = std::find(headers.begin(), headers.end(), std::string(ClusterInformationWriter::POPULATION_FIELD));
    if (itr == headers.end()) BOOST_FAIL("population column not found");
    CSV_Row_t data;
    getCSVRow(stream, data);
    double population;
    unsigned int rec_count = 0;
    while (data.size()) {
        ++rec_count;
        BOOST_CHECK(string_to_type<double>(data.at(std::distance(headers.begin(), itr)).c_str(), population));
        switch (rec_count) {
        case 1: BOOST_CHECK_EQUAL(population, 674891.72); break;
        case 2: BOOST_CHECK_EQUAL(population, 52755.68); break;
        case 3: BOOST_CHECK_EQUAL(population, 59349.33); break;
        default: BOOST_FAIL("unexpected record");
        }
        getCSVRow(stream, data);
    }
    stream.close();
}

/* Test population reported in cluster location information file.  */
BOOST_AUTO_TEST_CASE(test_cluster_location_information_reported_population) {
    std::string results_user_directory, buffer1, buffer2;

    // Perform baseline analysis -- no restricting of clusters by risk level.
    _parameters.SetAreaRateType(HIGH);
    _parameters.setPrintAsciiHeaders(true);
    _parameters.SetOutputAreaSpecificAscii(true);
    run_analysis("test1", results_user_directory, _parameters, _print);

    // open the cluster information file and compare population to expected
    std::ifstream stream;
    getFileStream(stream, printString(buffer1, "test1%s%s", LocationInformationWriter::AREA_SPECIFIC_FILE_EXT, ASCIIDataFileWriter::ASCII_FILE_EXT), results_user_directory);
    CSV_Row_t headers;
    getCSVRow(stream, headers);
    std::vector<std::string>::iterator itrClusterNum = std::find(headers.begin(), headers.end(), std::string(LocationInformationWriter::CLUST_NUM_FIELD));
    if (itrClusterNum == headers.end()) BOOST_FAIL("cluster id column not found");
    std::vector<std::string>::iterator itrClusterPop = std::find(headers.begin(), headers.end(), std::string(LocationInformationWriter::CLU_POPULATION_FIELD));
    if (itrClusterPop == headers.end()) BOOST_FAIL("cluster population column not found");
    std::vector<std::string>::iterator itrClusterLocPop = std::find(headers.begin(), headers.end(), std::string(LocationInformationWriter::LOC_POPULATION_FIELD));
    if (itrClusterLocPop == headers.end()) BOOST_FAIL("cluster loc population column not found");
    CSV_Row_t data;
    getCSVRow(stream, data);
    unsigned int clusterNum;
    double population, loc_population;
    unsigned int rec_count = 0;
    std::vector<double> clusterPopulation, clusterLocPopulation;
    while (data.size()) {
        ++rec_count;
        BOOST_CHECK(string_to_type<unsigned int>(data.at(std::distance(headers.begin(), itrClusterNum)).c_str(), clusterNum));
        BOOST_CHECK(string_to_type<double>(data.at(std::distance(headers.begin(), itrClusterPop)).c_str(), population));
        BOOST_CHECK(string_to_type<double>(data.at(std::distance(headers.begin(), itrClusterLocPop)).c_str(), loc_population));
        if (clusterPopulation.size() < clusterNum) { 
            clusterPopulation.resize(clusterNum, population);
            clusterLocPopulation.resize(clusterNum, loc_population);
        } else
            clusterLocPopulation.at(clusterNum - 1) += loc_population;
        getCSVRow(stream, data);
    }
    stream.close();
    for (size_t i = 0; i < clusterPopulation.size(); ++i)
        BOOST_REQUIRE_CLOSE(clusterPopulation[i], clusterLocPopulation[i], 0.001);
}

BOOST_AUTO_TEST_SUITE_END()

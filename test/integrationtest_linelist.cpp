
// project files
#include "fixture_sampledata.h"
#include "AnalysisRun.h"
#include "Toolkit.h"
#include "ClusterInformationWriter.h"
#include "ClusterLocationsWriter.h"
#include <boost/filesystem.hpp>

/* line list data, cache file and significant cluster line list csv */

struct linelist_fixture : prm_testset_fixture {
    linelist_fixture() : prm_testset_fixture("linelist\\test.prm") { }
    virtual ~linelist_fixture() { }
};

/* Test Suite for case file when line list data is included. */
BOOST_FIXTURE_TEST_SUITE(cluster_linelist_suite, linelist_fixture)

/* Test line list output file - only significant clusters included in csv and associated line list data. */
BOOST_AUTO_TEST_CASE(test_cluster_linelist) {
    std::string results_user_directory;
    std::stringstream filename;
    filename << GetUserTemporaryDirectory(results_user_directory) << "\\test1.event-cache.txt";
    _parameters.setLinelistIndividualsCacheFileName(filename.str().c_str()); // set the cache file for test testing
    remove(_parameters.getLinelistIndividualsCacheFileName().c_str()); // remove the cache file - just in case already exists
    run_analysis("test1", results_user_directory, _parameters, _print);

    // open line list csv and confirm expected headers, expected counts and select data rows/columns
    std::ifstream stream;
    getFileStream(stream, "test1.linelist.csv", results_user_directory);
    CSV_Row_t headers, expected_headers = {
        "Cluster","Hierarchy","New Individual","IndividualID","DescriptiveLongitude",
        "DescriptiveLatitude","Race", "Gender","Age Category","COVID19 Status","Address Type"
    };
    getCSVRow(stream, headers, ",");
    BOOST_CHECK(headers == expected_headers);
    auto getheaderIterator = [&headers](const std::string& label) { return std::find(headers.begin(), headers.end(), label); };
    auto itrCluster = getheaderIterator(headers.front());
    auto itrHierarchy = getheaderIterator(*(headers.begin() + 1));
    auto itrNew = getheaderIterator(*(headers.begin() + 2));
    CSV_Row_t data;
    getCSVRow(stream, data, ",");
    unsigned int cluster_num, rec_count = 0;
    std::map<unsigned int, unsigned int> _cluster_rec_count;
    auto get_data_at = [&data, &headers](const std::string& label) {
        return data.at(std::distance(headers.begin(), std::find(headers.begin(), headers.end(), label)));
    };
    while (data.size()) {
        ++rec_count;
        BOOST_CHECK(string_to_type<unsigned int>(data.at(std::distance(headers.begin(), itrCluster)).c_str(), cluster_num));
        if (_cluster_rec_count.find(cluster_num) == _cluster_rec_count.end()) _cluster_rec_count.emplace(std::make_pair(cluster_num, 0));
        _cluster_rec_count[cluster_num] += 1;
        BOOST_CHECK(data.at(std::distance(headers.begin(), itrHierarchy)) == "Primary");
        BOOST_CHECK(data.at(std::distance(headers.begin(), itrNew)) == "New");
        if (rec_count == 1) { // test all values of first record
            BOOST_CHECK(get_data_at(*(headers.begin() + 3)) == "b68d5911-f018-43a3-b165-4f144739546b");
            BOOST_CHECK(get_data_at(*(headers.begin() + 4)) == "38.37695");
            BOOST_CHECK(get_data_at(*(headers.begin() + 5)) == "-77.08463");
            BOOST_CHECK(get_data_at(*(headers.begin() + 6)) == "American Indian or Alaska Native");
            BOOST_CHECK(get_data_at(*(headers.begin() + 7)) == "Not Specified");
            BOOST_CHECK(get_data_at(*(headers.begin() + 8)) == "Adults (25-64 years)");
            BOOST_CHECK(get_data_at(*(headers.begin() + 9)) == "Suspect");
            BOOST_CHECK(get_data_at(*(headers.begin() + 10)) == "Home");
        }
        getCSVRow(stream, data, ",");
    }
    stream.close();
    BOOST_CHECK(rec_count == 148);
    BOOST_CHECK(_cluster_rec_count[1] == 99);
    BOOST_CHECK(_cluster_rec_count[2] == 49);
    remove(_parameters.getLinelistIndividualsCacheFileName().c_str()); // remove the cache file
}

/* Test values of line list csv with follow-up analysis. */
BOOST_AUTO_TEST_CASE(test_cluster_linelist_followup) {
    std::string results_user_directory;
    std::stringstream filename;
    filename << GetUserTemporaryDirectory(results_user_directory) << "\\test1.event-cache.txt";
    _parameters.setLinelistIndividualsCacheFileName(filename.str().c_str());
    remove(_parameters.getLinelistIndividualsCacheFileName().c_str()); // remove the cache file
    // run first analysis - all line list records will be 'New'
    run_analysis("test1", results_user_directory, _parameters, _print);
    // run follow-up analysis - adding one new case which will show up in first cluster
    _parameters.SetCaseFileName("test-run2.cas", true);
    run_analysis("test1", results_user_directory, _parameters, _print);

    // open line list csv and confirm individuals from first analysis are not 'New' and new record is 'New'
    std::ifstream stream;
    getFileStream(stream, "test1.linelist.csv", results_user_directory);
    CSV_Row_t headers;
    getCSVRow(stream, headers, ",");
    auto itrCluster = std::find(headers.begin(), headers.end(), headers.front());
    auto itrNew = std::find(headers.begin(), headers.end(), *(headers.begin() + 2));
    CSV_Row_t data;
    getCSVRow(stream, data, ",");
    unsigned int cluster_num, rec_count = 0;
    std::map<unsigned int, unsigned int> _cluster_rec_count;
    while (data.size()) {
        ++rec_count;
        BOOST_CHECK(string_to_type<unsigned int>(data.at(std::distance(headers.begin(), itrCluster)).c_str(), cluster_num));
        if (_cluster_rec_count.find(cluster_num) == _cluster_rec_count.end()) _cluster_rec_count.emplace(std::make_pair(cluster_num, 0));
        _cluster_rec_count[cluster_num] += 1;
        // individuals from first analysis should present as not 'New' -- expecting blank value
        if (rec_count < 100) BOOST_CHECK(data.at(std::distance(headers.begin(), itrNew)) == "");
        // newly added case shows up as 100 record (of first cluster)
        if (rec_count == 100) BOOST_CHECK(data.at(std::distance(headers.begin(), itrNew)) == "New");
        getCSVRow(stream, data, ",");
    }
    stream.close();
    BOOST_CHECK(rec_count == 149);
    BOOST_CHECK(_cluster_rec_count[1] == 100);
    BOOST_CHECK(_cluster_rec_count[2] == 49);
    remove(_parameters.getLinelistIndividualsCacheFileName().c_str()); // remove the cache file
}

/* Tests that a cache file is maintained when case file includes linelist data.  */
BOOST_AUTO_TEST_CASE(test_linelist_cache) {
    std::string results_user_directory;
    std::stringstream filename;
    filename << GetUserTemporaryDirectory(results_user_directory) << "\\test2.event-cache.txt";
    _parameters.setLinelistIndividualsCacheFileName(filename.str().c_str());
    remove(_parameters.getLinelistIndividualsCacheFileName().c_str()); // remove the cache file
    run_analysis("test2", results_user_directory, _parameters, _print);

    // open line list csv and confirm expected headers and select data rows
    std::ifstream stream;
    std::set<std::string> cacheValues;
    getFileStream(stream, "test2.event-cache.txt", results_user_directory);
    CSV_Row_t data;
    getCSVRow(stream, data);
    unsigned int rec_count = 0;
    while (data.size()) {
        ++rec_count;
        cacheValues.emplace(data.front());
        getCSVRow(stream, data);
    }
    stream.close();
    BOOST_CHECK(rec_count == 148);

    // run follow-up analysis - adding one new case which will show up in first cluster
    _parameters.SetCaseFileName("test-run2.cas", true);
    run_analysis("test1", results_user_directory, _parameters, _print);
    getFileStream(stream, "test2.event-cache.txt", results_user_directory);
    getCSVRow(stream, data);
    rec_count = 0;
    cacheValues.clear();
    while (data.size()) {
        ++rec_count;
        cacheValues.emplace(data.front());
        getCSVRow(stream, data);
    }
    stream.close();
    BOOST_CHECK(rec_count == 149);

    remove(_parameters.getLinelistIndividualsCacheFileName().c_str()); // remove the cache file
}


BOOST_AUTO_TEST_SUITE_END()

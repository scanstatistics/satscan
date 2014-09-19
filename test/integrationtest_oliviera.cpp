
// project files
#include "fixture_sampledata.h"
#include "AnalysisRun.h"
#include "Toolkit.h"
#include "LocationRiskEstimateWriter.h"
#include <fstream>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

struct new_mexico_analysis_fixture : new_mexico_fixture {
    new_mexico_analysis_fixture() : new_mexico_fixture() { 
        BOOST_TEST_MESSAGE( "setup fixture -- new_mexico_analysis_fixture" );
        // nm data set uses the Poisson model and space-time analysis initially
        _parameters.SetAnalysisType(PURELYSPATIAL);
        _parameters.setCalculateOlivierasF(true);
        // request both "Location Information" and "Risk Estimates for Each Location" files -- feature disabled if neither selected
        _parameters.setPrintAsciiHeaders(true);
        _parameters.SetOutputRelativeRisksAscii(true);
        _parameters.SetOutputAreaSpecificAscii(true);
        // set results file to the user document directory
        std::stringstream filename;
        filename << GetUserDocumentsDirectory(_results_user_directory, "").c_str() << "\\test.txt";
        _parameters.SetOutputFileName(filename.str().c_str());

        time_t startTime;
        time(&startTime);
        AppToolkit::ToolKitCreate(boost::unit_test::framework::master_test_suite().argv[0]);
        AnalysisRunner(_parameters, startTime, _print);
        AppToolkit::ToolKitDestroy();
    }
    virtual ~new_mexico_analysis_fixture() { BOOST_TEST_MESSAGE( "teardown fixture -- new_mexico_analysis_fixture" ); }

    std::string _results_user_directory;
};

/* Test Suite for the Parameters validation class. */
BOOST_FIXTURE_TEST_SUITE( intregration_oliviera_suite, new_mexico_analysis_fixture )

/* Tests parameters validation requires purely spatial analysis with oliviera. */
BOOST_AUTO_TEST_CASE( test_oliviera_risk_estimates_output ) {
    // open the risks estimates file and confirm Oliviera column is present and values match expected for this data set
    std::ifstream stream;
    std::stringstream rr_filename;
    rr_filename << GetUserDocumentsDirectory(_results_user_directory, "").c_str() << "\\test.rr.txt";
    if (!stream.is_open()) stream.open(rr_filename.str().c_str());
    if (!stream) throw std::exception("could not open risks estimates file");

    // check that the oliviera's f column in present in the headers
    std::string line;
    std::getline(stream, line);
    boost::escaped_list_separator<char> separator("\\", "\t\v\f\r\n ", "\"");
    boost::tokenizer<boost::escaped_list_separator<char> > headers(line, separator);
    std::vector<std::string> tokens;
    for (boost::tokenizer<boost::escaped_list_separator<char> >::const_iterator itr=headers.begin(); itr != headers.end(); ++itr) {
        tokens.push_back(*itr);
        boost::trim(tokens.back());
        if (!tokens.back().size()) tokens.pop_back();
    }
    std::vector<std::string>::iterator itr = std::find(tokens.begin(), tokens.end(), std::string(LocationRiskEstimateWriter::OLIVIERA_F_FIELD));
    if (itr == tokens.end()) BOOST_FAIL( "Oliviera'F column not found" );

    // check the expected values for Oliviera's F with this analysis
    size_t oliviera_idx = std::distance(tokens.begin(), itr);
    std::getline(stream, line);
    boost::tokenizer<boost::escaped_list_separator<char> > rec_values(line, separator);
    tokens.clear();
    for (boost::tokenizer<boost::escaped_list_separator<char> >::const_iterator itr=rec_values.begin(); itr != rec_values.end(); ++itr) {
        tokens.push_back(*itr);
        boost::trim(tokens.back());
        if (!tokens.back().size()) tokens.pop_back();
    }
    double oliviera;
    BOOST_CHECK( string_to_type<double>(tokens.at(oliviera_idx).c_str(), oliviera) );
    BOOST_REQUIRE_CLOSE( oliviera, 0.9530, 0.001 );
}

/* Tests parameters validation requires purely spatial analysis with oliviera. */
BOOST_AUTO_TEST_CASE( test_oliviera_location_information_output ) {
    // open the risks estimates file and confirm Oliviera column is present and values match expected for this data set
    std::ifstream stream;
    std::stringstream rr_filename;
    rr_filename << GetUserDocumentsDirectory(_results_user_directory, "").c_str() << "\\test.gis.txt";
    if (!stream.is_open()) stream.open(rr_filename.str().c_str());
    if (!stream) throw std::exception("could not open location information file");

    // check that the oliviera's f column in present in the headers
    std::string line;
    std::getline(stream, line);
    boost::escaped_list_separator<char> separator("\\", "\t\v\f\r\n ", "\"");
    boost::tokenizer<boost::escaped_list_separator<char> > headers(line, separator);
    std::vector<std::string> tokens;
    for (boost::tokenizer<boost::escaped_list_separator<char> >::const_iterator itr=headers.begin(); itr != headers.end(); ++itr) {
        tokens.push_back(*itr);
        boost::trim(tokens.back());
        if (!tokens.back().size()) tokens.pop_back();
    }
    std::vector<std::string>::iterator itr = std::find(tokens.begin(), tokens.end(), std::string(LocationInformationWriter::OLIVIERA_F_FIELD));
    if (itr == tokens.end()) BOOST_FAIL( "Oliviera'F column not found" );

    // check the expected values for Oliviera's F with this analysis
    size_t oliviera_idx = std::distance(tokens.begin(), itr);
    std::getline(stream, line);
    boost::tokenizer<boost::escaped_list_separator<char> > rec_values(line, separator);
    tokens.clear();
    for (boost::tokenizer<boost::escaped_list_separator<char> >::const_iterator itr=rec_values.begin(); itr != rec_values.end(); ++itr) {
        tokens.push_back(*itr);
        boost::trim(tokens.back());
        if (!tokens.back().size()) tokens.pop_back();
    }
    double oliviera;
    BOOST_CHECK( string_to_type<double>(tokens.at(oliviera_idx).c_str(), oliviera) );
    BOOST_REQUIRE_CLOSE( oliviera, 0.7790, 0.001 );
}

BOOST_AUTO_TEST_SUITE_END()

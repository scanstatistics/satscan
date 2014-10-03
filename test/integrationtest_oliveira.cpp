
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
        /*BOOST_TEST_MESSAGE( "setup fixture -- new_mexico_analysis_fixture" );*/
        // nm data set uses the Poisson model and space-time analysis initially
        _parameters.SetAnalysisType(PURELYSPATIAL);
        _parameters.setCalculateOliveirasF(true);
        // request both "Location Information" and "Risk Estimates for Each Location" files -- feature disabled if neither selected
        _parameters.setPrintAsciiHeaders(true);
        _parameters.SetOutputRelativeRisksAscii(true);
        _parameters.SetOutputAreaSpecificAscii(true);
    }
    virtual ~new_mexico_analysis_fixture() { /*BOOST_TEST_MESSAGE( "teardown fixture -- new_mexico_analysis_fixture" );*/ }

    void run_analysis(const std::string& analysis_name) {
        // set results file to the user document directory
        std::stringstream filename;
        filename << GetUserDocumentsDirectory(_results_user_directory, "").c_str() << "\\" << analysis_name.c_str() << ".txt";
        _parameters.SetOutputFileName(filename.str().c_str());

        time_t startTime;
        time(&startTime);
        AppToolkit::ToolKitCreate(boost::unit_test::framework::master_test_suite().argv[0]);
        AnalysisRunner(_parameters, startTime, _print);
        AppToolkit::ToolKitDestroy();
    }

    std::ifstream & getFileStream(std::ifstream& stream, const std::string& filename) {
        std::stringstream rr_filename;
        rr_filename << GetUserDocumentsDirectory(_results_user_directory, "").c_str() << "\\" << filename.c_str();
        stream.open(rr_filename.str().c_str());
        if (!stream) throw std::exception("could not file");
        return stream;
    }

    typedef std::vector<std::string> File_Row_t;
    File_Row_t& getRow(std::ifstream& stream, File_Row_t& row) {
        row.clear();
        std::string line;
        std::getline(stream, line);
        boost::escaped_list_separator<char> separator("\\", "\t\v\f\r\n ", "\"");
        boost::tokenizer<boost::escaped_list_separator<char> > headers(line, separator);
        for (boost::tokenizer<boost::escaped_list_separator<char> >::const_iterator itr=headers.begin(); itr != headers.end(); ++itr) {
            row.push_back(*itr);
            boost::trim(row.back());
            if (!row.back().size()) row.pop_back();
        }
        return row;
    }

    std::string _results_user_directory;
};

/* Test Suite for the Parameters validation class. */
BOOST_FIXTURE_TEST_SUITE( intregration_oliveira_suite, new_mexico_analysis_fixture )

/* Tests for expected oliveira fields in risk estimates file with neither hierarchical nor gini selected. */
BOOST_AUTO_TEST_CASE( test_oliveira_risk_estimates_output_mlc_only ) {
    _parameters.setReportHierarchicalClusters(false);
    _parameters.setReportGiniOptimizedClusters(false);
    run_analysis("test");

    // open the risks estimates file and confirm Oliveira column is present and values match expected for this data set
    std::ifstream stream;
    getFileStream(stream, "test.rr.txt");

    // check that the oliveira's f column in present in the headers
    File_Row_t headers;
    getRow(stream, headers);

    // only expecting LocationRiskEstimateWriter::OLIVEIRA_F_MLC_FIELD
    std::vector<std::string>::iterator itr = std::find(headers.begin(), headers.end(), std::string(LocationRiskEstimateWriter::OLIVEIRA_F_MLC_FIELD));
    if (itr == headers.end()) BOOST_FAIL( "Oliveira'F column not found" );
    // not expecting the other oliveira fields
    if (std::find(headers.begin(), headers.end(), std::string(LocationRiskEstimateWriter::OLIVEIRA_F_HIERARCHICAL_FIELD)) != headers.end()) BOOST_FAIL( "unexpected Oliveira'F column found" );
    if (std::find(headers.begin(), headers.end(), std::string(LocationRiskEstimateWriter::OLIVEIRA_F_GINI_OPTIMAL_FIELD)) != headers.end()) BOOST_FAIL( "unexpected Oliveira'F column found" );
    if (std::find(headers.begin(), headers.end(), std::string(LocationRiskEstimateWriter::OLIVEIRA_F_GINI_MAXIMA_FIELD)) != headers.end()) BOOST_FAIL( "unexpected Oliveira'F column found" );
    if (std::find(headers.begin(), headers.end(), std::string(LocationRiskEstimateWriter::OLIVEIRA_F_HIERARCHICAL_GINI_OPTIMAL_FIELD)) != headers.end()) BOOST_FAIL( "unexpected Oliveira'F column found" );
    if (std::find(headers.begin(), headers.end(), std::string(LocationRiskEstimateWriter::OLIVEIRA_F_HIERARCHICAL_GINI_MAXIMA_FIELD)) != headers.end()) BOOST_FAIL( "unexpected Oliveira'F column found" );

    // check the expected values for Oliveira's F with this analysis
    File_Row_t data;
    getRow(stream, data);
    double oliveira;
    BOOST_CHECK( string_to_type<double>(data.at(std::distance(headers.begin(), itr)).c_str(), oliveira) );
    BOOST_REQUIRE_CLOSE( oliveira, 0.8630, 0.001 );

    stream.close();
}

/* Tests for expected oliveira fields in risk estimates file with hierarchical and gini selected. */
BOOST_AUTO_TEST_CASE( test_oliveira_risk_estimates_output ) {
    _parameters.setReportHierarchicalClusters(true);
    _parameters.setReportGiniOptimizedClusters(true);
    run_analysis("test");

    // open the risks estimates file and confirm Oliveira column is present and values match expected for this data set
    std::ifstream stream;
    getFileStream(stream, "test.rr.txt");

    // check that the oliveira's f column in present in the headers
    File_Row_t headers;
    getRow(stream, headers);

    // expecting all oliveira fields except LocationRiskEstimateWriter::OLIVEIRA_F_MLC_FIELD
    if (std::find(headers.begin(), headers.end(), std::string(LocationRiskEstimateWriter::OLIVEIRA_F_MLC_FIELD)) != headers.end()) BOOST_FAIL( "unexpected Oliveira'F column found" );

    File_Row_t data;
    getRow(stream, data);

    // test existance and value of each expected oliveira column
    double oliveira;
    std::vector<std::string>::iterator itr = std::find(headers.begin(), headers.end(), std::string(LocationRiskEstimateWriter::OLIVEIRA_F_HIERARCHICAL_FIELD));
    if (itr == headers.end()) BOOST_FAIL( "Oliveira'F column not found" );
    BOOST_CHECK( string_to_type<double>(data.at(std::distance(headers.begin(), itr)).c_str(), oliveira) );
    BOOST_REQUIRE_CLOSE( oliveira, 0.8630, 0.001 );

    itr = std::find(headers.begin(), headers.end(), std::string(LocationRiskEstimateWriter::OLIVEIRA_F_GINI_OPTIMAL_FIELD));
    if (itr == headers.end()) BOOST_FAIL( "Oliveira'F column not found" );
    BOOST_CHECK( string_to_type<double>(data.at(std::distance(headers.begin(), itr)).c_str(), oliveira) );
    BOOST_REQUIRE_CLOSE( oliveira, 0.8630, 0.001 );

    itr = std::find(headers.begin(), headers.end(), std::string(LocationRiskEstimateWriter::OLIVEIRA_F_GINI_MAXIMA_FIELD));
    if (itr == headers.end()) BOOST_FAIL( "Oliveira'F column not found" );
    BOOST_CHECK( string_to_type<double>(data.at(std::distance(headers.begin(), itr)).c_str(), oliveira) );
    BOOST_REQUIRE_CLOSE( oliveira, 0.9700, 0.001 );

    itr = std::find(headers.begin(), headers.end(), std::string(LocationRiskEstimateWriter::OLIVEIRA_F_HIERARCHICAL_GINI_OPTIMAL_FIELD));
    if (itr == headers.end()) BOOST_FAIL( "Oliveira'F column not found" );
    BOOST_CHECK( string_to_type<double>(data.at(std::distance(headers.begin(), itr)).c_str(), oliveira) );
    BOOST_REQUIRE_CLOSE( oliveira, 0.8630, 0.001 );

    itr = std::find(headers.begin(), headers.end(), std::string(LocationRiskEstimateWriter::OLIVEIRA_F_HIERARCHICAL_GINI_MAXIMA_FIELD));
    if (itr == headers.end()) BOOST_FAIL( "Oliveira'F column not found" );
    BOOST_CHECK( string_to_type<double>(data.at(std::distance(headers.begin(), itr)).c_str(), oliveira) );
    BOOST_REQUIRE_CLOSE( oliveira, 0.9700, 0.001 );

    stream.close();
}

/* Tests for expected oliveira fields in location information file with neither hierarchical nor gini selected. */
BOOST_AUTO_TEST_CASE( test_oliveira_location_information_output_mlc_only ) {
    _parameters.setReportHierarchicalClusters(false);
    _parameters.setReportGiniOptimizedClusters(false);
    run_analysis("test");

    // open the risks estimates file and confirm Oliveira column is present and values match expected for this data set
    std::ifstream stream;
    getFileStream(stream, "test.gis.txt");

    // check that the oliveira's f column in present in the headers
    File_Row_t headers;
    getRow(stream, headers);

    std::vector<std::string>::iterator itr = std::find(headers.begin(), headers.end(), std::string(LocationInformationWriter::OLIVEIRA_F_MLC_FIELD));
    if (itr == headers.end()) BOOST_FAIL( "Oliveira'F column not found" );
    // not expecting the other oliveira fields
    if (std::find(headers.begin(), headers.end(), std::string(LocationInformationWriter::OLIVEIRA_F_HIERARCHICAL_FIELD)) != headers.end()) BOOST_FAIL( "unexpected Oliveira'F column found" );
    if (std::find(headers.begin(), headers.end(), std::string(LocationInformationWriter::OLIVEIRA_F_GINI_OPTIMAL_FIELD)) != headers.end()) BOOST_FAIL( "unexpected Oliveira'F column found" );
    if (std::find(headers.begin(), headers.end(), std::string(LocationInformationWriter::OLIVEIRA_F_GINI_MAXIMA_FIELD)) != headers.end()) BOOST_FAIL( "unexpected Oliveira'F column found" );
    if (std::find(headers.begin(), headers.end(), std::string(LocationInformationWriter::OLIVEIRA_F_HIERARCHICAL_GINI_OPTIMAL_FIELD)) != headers.end()) BOOST_FAIL( "unexpected Oliveira'F column found" );
    if (std::find(headers.begin(), headers.end(), std::string(LocationInformationWriter::OLIVEIRA_F_HIERARCHICAL_GINI_MAXIMA_FIELD)) != headers.end()) BOOST_FAIL( "unexpected Oliveira'F column found" );

    // check the expected values for Oliveira's F with this analysis
    double oliveira;
    File_Row_t data;
    getRow(stream, data);
    BOOST_CHECK( string_to_type<double>(data.at(std::distance(headers.begin(), itr)).c_str(), oliveira) );
    BOOST_REQUIRE_CLOSE( oliveira, 0.7220, 0.001 );

    stream.close();
}

/* Tests for expected oliveira fields in location information file with hierarchical and gini selected. */
BOOST_AUTO_TEST_CASE( test_oliveira_location_information_output ) {
    _parameters.setReportHierarchicalClusters(true);
    _parameters.setReportGiniOptimizedClusters(true);
    run_analysis("test");

    // open the risks estimates file and confirm Oliveira column is present and values match expected for this data set
    std::ifstream stream;
    getFileStream(stream, "test.gis.txt");

    // check that the oliveira's f column in present in the headers
    File_Row_t header;
    getRow(stream, header);

    // expecting all oliveira fields except LocationRiskEstimateWriter::OLIVEIRA_F_MLC_FIELD
    if (std::find(header.begin(), header.end(), std::string(LocationInformationWriter::OLIVEIRA_F_MLC_FIELD)) != header.end()) BOOST_FAIL( "unexpected Oliveira'F column found" );

    File_Row_t data;
    getRow(stream, data);

    // test existance and value of each expected oliveira column
    double oliveira;
    std::vector<std::string>::iterator itr = std::find(header.begin(), header.end(), std::string(LocationInformationWriter::OLIVEIRA_F_HIERARCHICAL_FIELD));
    if (itr == header.end()) BOOST_FAIL( "Oliveira'F column not found" );
    BOOST_CHECK( string_to_type<double>(data.at(std::distance(header.begin(), itr)).c_str(), oliveira) );
    BOOST_REQUIRE_CLOSE( oliveira, 0.7220, 0.001 );

    itr = std::find(header.begin(), header.end(), std::string(LocationInformationWriter::OLIVEIRA_F_GINI_OPTIMAL_FIELD));
    if (itr == header.end()) BOOST_FAIL( "Oliveira'F column not found" );
    BOOST_CHECK( string_to_type<double>(data.at(std::distance(header.begin(), itr)).c_str(), oliveira) );
    BOOST_REQUIRE_CLOSE( oliveira, 0.7610, 0.001 );

    itr = std::find(header.begin(), header.end(), std::string(LocationInformationWriter::OLIVEIRA_F_GINI_MAXIMA_FIELD));
    if (itr == header.end()) BOOST_FAIL( "Oliveira'F column not found" );
    BOOST_CHECK( string_to_type<double>(data.at(std::distance(header.begin(), itr)).c_str(), oliveira) );
    BOOST_REQUIRE_CLOSE( oliveira, 0.7790, 0.001 );

    itr = std::find(header.begin(), header.end(), std::string(LocationInformationWriter::OLIVEIRA_F_HIERARCHICAL_GINI_OPTIMAL_FIELD));
    if (itr == header.end()) BOOST_FAIL( "Oliveira'F column not found" );
    BOOST_CHECK( string_to_type<double>(data.at(std::distance(header.begin(), itr)).c_str(), oliveira) );
    BOOST_REQUIRE_CLOSE( oliveira, 0.7610, 0.001 );

    itr = std::find(header.begin(), header.end(), std::string(LocationInformationWriter::OLIVEIRA_F_HIERARCHICAL_GINI_MAXIMA_FIELD));
    if (itr == header.end()) BOOST_FAIL( "Oliveira'F column not found" );
    BOOST_CHECK( string_to_type<double>(data.at(std::distance(header.begin(), itr)).c_str(), oliveira) );
    BOOST_REQUIRE_CLOSE( oliveira, 0.7790, 0.001 );

    stream.close();
}

BOOST_AUTO_TEST_SUITE_END()

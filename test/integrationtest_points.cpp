
// project files
#include "fixture_sampledata.h"
#include "AnalysisRun.h"
#include "Toolkit.h"
#include "dBaseFile.h"
#include "ShapeFile.h"
#include <fstream>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

struct points_analysis_fixture : new_mexico_fixture {
    points_analysis_fixture() : new_mexico_fixture() { 
        /*BOOST_TEST_MESSAGE( "setup fixture -- points_analysis_fixture" );*/
        // nm data set uses the Poisson model and space-time analysis initially
        _parameters.SetAnalysisType(PURELYSPATIAL);
        // request both "Location Information - dBase" and "Shapefile for GIS" files -- feature disabled if neither selected
        _parameters.SetOutputAreaSpecificDBase(true);
        _parameters.setOutputShapeFiles(true);
    }
    virtual ~points_analysis_fixture() { /*BOOST_TEST_MESSAGE( "teardown fixture -- points_analysis_fixture" );*/ }

    void run_analysis(const std::string& analysis_name) {
        // set results file to the user document directory
        std::stringstream filename;
        filename << GetUserDocumentsDirectory(_results_user_directory, "").c_str() << "\\" << analysis_name.c_str() << ".txt";
        _parameters.SetOutputFileName(filename.str().c_str());

        time_t startTime;
        time(&startTime);
        AppToolkit::ToolKitCreate(boost::unit_test::framework::master_test_suite().argv[0]);
        AnalysisRunner(_parameters, startTime, _print).Execute();
        AppToolkit::ToolKitDestroy();
    }

    std::ifstream & getFileStream(std::ifstream& stream, const std::string& filename) {
        std::stringstream rr_filename;
        rr_filename << GetUserDocumentsDirectory(_results_user_directory, "").c_str() << "\\" << filename.c_str();
        stream.open(rr_filename.str().c_str());
        if (!stream) throw std::exception("could not file");
        return stream;
    }

    dBaseFile& getDBaseFile(dBaseFile& dbfile, const std::string& filename) {
        std::stringstream rr_filename;
        rr_filename << GetUserDocumentsDirectory(_results_user_directory, "").c_str() << "\\" << filename.c_str();
        dbfile.Open(rr_filename.str().c_str(), "rb");
        if (!dbfile.GetIsOpen()) throw std::exception("could not file");
        return dbfile;
    }

    SHPHandle& getShapeFile(SHPHandle& shp, const std::string& filename) {
        std::stringstream rr_filename;
        rr_filename << GetUserDocumentsDirectory(_results_user_directory, "").c_str() << "\\" << filename.c_str();
        shp = SHPOpen(rr_filename.str().c_str(), "rb");
        if (!shp) throw prg_error("Unable to open file.","getShapeFile()");
        return shp;
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
BOOST_FIXTURE_TEST_SUITE( integration_points_suite, points_analysis_fixture )

/* Tests *.dbf and *.shp files for points */
BOOST_AUTO_TEST_CASE( test_point_shapefiles_output ) {
    run_analysis("test");

    // check that dBase file is created
    dBaseFile dbfile;
    getDBaseFile(dbfile, "test.gis.dbf");

    // check that dBase file contains expected number of records
    BOOST_CHECK_MESSAGE( dbfile.GetNumRecords() == 9, "*.dbf file contains " << dbfile.GetNumRecords() << " records" );

    dbfile.Close();

    // check that *.shp file is created
    SHPHandle shp;
    getShapeFile(shp, "test.gis.shp");

    // check that *.shp file is of point format (i.e. type 1)
    int shapeType;
    SHPGetInfo(shp, NULL, &shapeType, NULL, NULL);
    BOOST_CHECK_MESSAGE( strcmp(SHPTypeName(shapeType), "Point") == 0, "*.shp file is of shape type " << SHPTypeName(shapeType) );

    SHPClose(shp);
}

BOOST_AUTO_TEST_SUITE_END()

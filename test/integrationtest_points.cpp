
// project files
#include "fixture_sampledata.h"
#include "AnalysisRun.h"
#include "Toolkit.h"
#include "dBaseFile.h"
#include "ShapeFile.h"

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

    dBaseFile& getDBaseFile(dBaseFile& dbfile, const std::string& filename) {
        std::stringstream rr_filename;
        rr_filename << GetUserTemporaryDirectory(_results_user_directory).c_str() << "\\" << filename.c_str();
        dbfile.Open(rr_filename.str().c_str(), "rb");
        if (!dbfile.GetIsOpen()) throw std::exception("could not file");
        return dbfile;
    }

    SHPHandle& getShapeFile(SHPHandle& shp, const std::string& filename) {
        std::stringstream rr_filename;
        rr_filename << GetUserTemporaryDirectory(_results_user_directory).c_str() << "\\" << filename.c_str();
        shp = SHPOpen(rr_filename.str().c_str(), "rb");
        if (!shp) throw prg_error("Unable to open file.","getShapeFile()");
        return shp;
    }

    std::string _results_user_directory;
};

/* Test Suite for the Parameters validation class. */
BOOST_FIXTURE_TEST_SUITE( integration_points_suite, points_analysis_fixture )

/* Tests *.dbf and *.shp files for points */
BOOST_AUTO_TEST_CASE( test_point_shapefiles_output ) {
    run_analysis("test", _results_user_directory, _parameters, _print);

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

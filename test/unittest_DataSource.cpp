
// project files
#include "fixture_sampledata.h"
#include "AnalysisRun.h"
#include "Toolkit.h"
#include "DataSource.h"
#include "dBaseFile.h"
#include "ShapeFile.h"

/* Test Suite for the Parameters validation class. */
BOOST_AUTO_TEST_SUITE( data_source_suite )

/* Tests reading standard ascii data source (space delimited) */
BOOST_AUTO_TEST_CASE( ascii_data_source_suite ) {
     PrintNull print;
     std::stringstream filename;
     filename << getTestSetFilesPath().c_str() << "\\datasources\\test.cas";
     AsciiFileDataSource source(filename.str(), print, ' ');

     unsigned int read_records = 0, expected_records = 10;
     while (source.ReadRecord()) {
         ++read_records;
         long num_values = source.GetNumValues();
         BOOST_CHECK_MESSAGE( num_values == 3, "Expecting 3 values but got " << num_values);
     }
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);

     source.GotoFirstRecord();
     read_records = 0;
     while (source.ReadRecord())
         ++read_records;
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);
}

/* Tests reading standard csv data source. */
BOOST_AUTO_TEST_CASE( csv_data_source_suite ) {
     PrintNull print;
     std::stringstream filename;
     filename << getTestSetFilesPath().c_str() << "\\datasources\\test_cas.csv";
     CsvFileDataSource source(filename.str(), print, ",", "\"", 2, true);

     unsigned int read_records = 0, expected_records = 10;
     while (source.ReadRecord()) {
         ++read_records;
         long num_values = source.GetNumValues();
         BOOST_CHECK_MESSAGE( num_values == 3, "Expecting 3 values but got " << num_values);
     }
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);

     source.GotoFirstRecord();
     read_records = 0;
     while (source.ReadRecord())
         ++read_records;
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);
}

/* Tests reading standard csv data source. */
BOOST_AUTO_TEST_CASE( csv_ordinal_data_source_suite ) {
     PrintNull print;
     std::stringstream filename;
     filename << getTestSetFilesPath().c_str() << "\\datasources\\test_case_ordinal.csv";
     CsvFileDataSource source(filename.str(), print, ",", "\"", 2, true);
     std::vector<boost::any> map;
     map.push_back(1);
     map.push_back(2);
     map.push_back(DataSource::BLANK);
     map.push_back(3);
     source.setFieldsMap(map);

     unsigned int read_records = 0, expected_records = 10;
     while (source.ReadRecord()) {
         ++read_records;
         long num_values = source.GetNumValues();
         BOOST_CHECK_MESSAGE( num_values == 3, "Expecting 3 values but got " << num_values);
     }
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);

     source.GotoFirstRecord();
     read_records = 0;
     while (source.ReadRecord())
         ++read_records;
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);
}

/* Tests reading standard dbase data source. */
BOOST_AUTO_TEST_CASE( dbase_data_source_suite ) {
     PrintNull print;
     std::stringstream filename;
     filename << getTestSetFilesPath().c_str() << "\\datasources\\test_cas.dbf";
     dBaseFileDataSource source(filename.str());

     unsigned int read_records = 0, expected_records = 10;
     while (source.ReadRecord()) {
         ++read_records;
         long num_values = source.GetNumValues();
         BOOST_CHECK_MESSAGE( num_values == 5, "Expecting 5 values but got " << num_values);
     }
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);

     source.GotoFirstRecord();
     read_records = 0;
     while (source.ReadRecord())
         ++read_records;
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);
}

/* Tests reading case file as shapefile data source. */
BOOST_AUTO_TEST_CASE( shapefile_case_data_source_suite ) {
     PrintNull print;
     std::stringstream filename;
     filename << getTestSetFilesPath().c_str() << "\\datasources\\test_shapefile.shp";
     ShapeFileDataSource source(filename.str());
     std::vector<boost::any> map;
     map.push_back(DataSource::GENERATEDID);
     map.push_back(DataSource::ONECOUNT);
     source.setFieldsMap(map);

     unsigned int read_records = 0, expected_records = 50;
     while (source.ReadRecord()) {
         ++read_records;
         long num_values = source.GetNumValues();
         BOOST_CHECK_MESSAGE( num_values == 2, "Expecting 2 values but got " << num_values);
     }
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);
}

/* Tests reading population file as shapefile data source. */
BOOST_AUTO_TEST_CASE( shapefile_population_data_source_suite ) {
     PrintNull print;
     std::stringstream filename;
     filename << getTestSetFilesPath().c_str() << "\\datasources\\test_shapefile.shp";
     ShapeFileDataSource source(filename.str());
     std::vector<boost::any> map;
     map.push_back(DataSource::GENERATEDID);
     map.push_back(DataSource::DEFAULT_DATE);
     map.push_back(1);
     source.setFieldsMap(map);

     unsigned int read_records = 0, expected_records = 50;
     while (source.ReadRecord()) {
         ++read_records;
         long num_values = source.GetNumValues();
         BOOST_CHECK_MESSAGE( num_values == 3, "Expecting 3 values but got " << num_values);
     }
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);
}

/* Tests reading coordinates file as shapefile data source. */
BOOST_AUTO_TEST_CASE( shapefile_coordinates_data_source_suite ) {
     PrintNull print;
     std::stringstream filename;
     filename << getTestSetFilesPath().c_str() << "\\datasources\\test_shapefile.shp";
     ShapeFileDataSource source(filename.str());
     std::vector<boost::any> map;
     map.push_back(DataSource::GENERATEDID);
     map.push_back(DataSource::POINTX);
     map.push_back(DataSource::POINTY);
     source.setFieldsMap(map);

     unsigned int read_records = 0, expected_records = 50;
     while (source.ReadRecord()) {
         ++read_records;
         long num_values = source.GetNumValues();
         BOOST_CHECK_MESSAGE( num_values == 3, "Expecting 3 values but got " << num_values);
     }
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);
}

/* Tests reading grid file as shapefile data source. */
BOOST_AUTO_TEST_CASE( shapefile_grid_data_source_suite ) {
     PrintNull print;
     std::stringstream filename;
     filename << getTestSetFilesPath().c_str() << "\\datasources\\test_shapefile.shp";
     ShapeFileDataSource source(filename.str());
     std::vector<boost::any> map;
     map.push_back(DataSource::POINTX);
     map.push_back(DataSource::POINTY);
     source.setFieldsMap(map);

     unsigned int read_records = 0, expected_records = 50;
     while (source.ReadRecord()) {
         ++read_records;
         long num_values = source.GetNumValues();
         BOOST_CHECK_MESSAGE( num_values == 2, "Expecting 2 values but got " << num_values);
     }
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);
}

BOOST_AUTO_TEST_SUITE_END()

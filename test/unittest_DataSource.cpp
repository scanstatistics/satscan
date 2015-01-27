
// project files
#include "fixture_sampledata.h"
#include "AnalysisRun.h"
#include "Toolkit.h"
#include "DataSource.h"
#include "DateStringParser.h"

typedef std::vector<std::pair<unsigned int, const char*> > ExpectedVals_t;

void compareExpected(DataSource& source, const ExpectedVals_t& expectedValues) {
    for (ExpectedVals_t::const_iterator itr=expectedValues.begin(); itr != expectedValues.end(); ++itr) {
        const char * source_value = source.GetValueAt(itr->first);
        if (strnicmp(source_value, itr->second, strlen(itr->second)))
            BOOST_FAIL("Data source value didn't match expected: " << itr->second << " != " << source_value);
    }
}

struct plain_datasource_fixture {
    plain_datasource_fixture() {}
    virtual ~plain_datasource_fixture() { }

    void compare_expected(DataSource& source, unsigned int recIdx, long num_expected=3) {
        long num_values = source.GetNumValues();
        BOOST_CHECK_MESSAGE( num_values == num_expected, "Expecting " << num_expected << " values but got " << num_values);

        ExpectedVals_t exp;
        switch (recIdx) {
            case 1 : exp.push_back(std::make_pair(0,"location1")); exp.push_back(std::make_pair(1,"5")); exp.push_back(std::make_pair(2,"2014/2/23")); break;
            case 2 : exp.push_back(std::make_pair(0,"location2")); exp.push_back(std::make_pair(1,"1")); exp.push_back(std::make_pair(2,"2014/9/26")); break;
            case 3 : exp.push_back(std::make_pair(0,"location3")); exp.push_back(std::make_pair(1,"2")); exp.push_back(std::make_pair(2,"2014/8/17")); break;
            case 4 : exp.push_back(std::make_pair(0,"location4")); exp.push_back(std::make_pair(1,"5")); exp.push_back(std::make_pair(2,"2014/11/8")); break;
            case 5 : exp.push_back(std::make_pair(0,"location5")); exp.push_back(std::make_pair(1,"1")); exp.push_back(std::make_pair(2,"2014/5/7")); break;
            case 6 : exp.push_back(std::make_pair(0,"location6")); exp.push_back(std::make_pair(1,"5")); exp.push_back(std::make_pair(2,"2014/9/9")); break;
            case 7 : exp.push_back(std::make_pair(0,"location7")); exp.push_back(std::make_pair(1,"4")); exp.push_back(std::make_pair(2,"2014/12/2")); break;
            case 8 : exp.push_back(std::make_pair(0,"location8")); exp.push_back(std::make_pair(1,"3")); exp.push_back(std::make_pair(2,"2014/11/9")); break;
            case 9 : exp.push_back(std::make_pair(0,"location9")); exp.push_back(std::make_pair(1,"5")); exp.push_back(std::make_pair(2,"2014/1/17")); break;
            case 10 : exp.push_back(std::make_pair(0,"location10")); exp.push_back(std::make_pair(1,"2")); exp.push_back(std::make_pair(2,"2014/12/27")); break;
        }
        compareExpected(source, exp);
    }
};

/* Test Suite for the Parameters validation class. */
BOOST_AUTO_TEST_SUITE( data_source_suite )

/* Tests reading ascii data source (space delimited) with AsciiFileDataSource. */
BOOST_FIXTURE_TEST_CASE( ascii_data_source1, plain_datasource_fixture ) {
     PrintNull print;
     std::stringstream filename;
     filename << getTestSetFilesPath().c_str() << "\\datasources\\test.cas";
     AsciiFileDataSource source(filename.str(), print, ' ');

     unsigned int read_records = 0, expected_records = 10;
     while (source.ReadRecord()) {
         ++read_records;
         compare_expected(source, read_records);
     }
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);

     source.GotoFirstRecord();
     read_records = 0;
     while (source.ReadRecord()) {
         ++read_records;
         compare_expected(source, read_records);
     }
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);
}

/* Tests reading ascii data source (space delimited) with CsvFileDataSource. */
BOOST_FIXTURE_TEST_CASE( ascii_data_source2, plain_datasource_fixture ) {
     PrintNull print;
     std::stringstream filename;
     filename << getTestSetFilesPath().c_str() << "\\datasources\\test.cas";
     CsvFileDataSource source(filename.str(), print, " ", "\"", 0, false);

     unsigned int read_records = 0, expected_records = 10;
     while (source.ReadRecord()) {
         ++read_records;
         compare_expected(source, read_records);
     }
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);

     source.GotoFirstRecord();
     read_records = 0;
     while (source.ReadRecord()) {
         ++read_records;
         compare_expected(source, read_records);
     }
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);
}

/* Tests reading csv data source. */
BOOST_FIXTURE_TEST_CASE( csv_data_source, plain_datasource_fixture ) {
     PrintNull print;
     std::stringstream filename;
     filename << getTestSetFilesPath().c_str() << "\\datasources\\test_cas.csv";
     CsvFileDataSource source(filename.str(), print, ",", "\"", 2, true);

     unsigned int read_records = 0, expected_records = 10;
     while (source.ReadRecord()) {
         ++read_records;
         compare_expected(source, read_records);
     }
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);

     source.GotoFirstRecord();
     read_records = 0;
     while (source.ReadRecord()) {
        ++read_records;
        compare_expected(source, read_records);
     }
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);
}

/* Tests reading dbase data source. */
BOOST_FIXTURE_TEST_CASE( dbase_data_source, plain_datasource_fixture ) {
     PrintNull print;
     std::stringstream filename;
     filename << getTestSetFilesPath().c_str() << "\\datasources\\test_cas.dbf";
     dBaseFileDataSource source(filename.str());

     unsigned int read_records = 0, expected_records = 10;
     while (source.ReadRecord()) {
         ++read_records;
         compare_expected(source, read_records);
     }
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);

     source.GotoFirstRecord();
     read_records = 0;
     while (source.ReadRecord()) {
         ++read_records;
         compare_expected(source, read_records);
     }
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);
}

/* Tests reading case file as shapefile data source. */
BOOST_AUTO_TEST_CASE( shapefile_data_source_case ) {
     PrintNull print;
     std::stringstream filename;
     filename << getTestSetFilesPath().c_str() << "\\datasources\\test_shapefile.shp";
     ShapeFileDataSource source(filename.str());
     std::vector<boost::any> map;
     map.push_back(DataSource::GENERATEDID);
     map.push_back(DataSource::ONECOUNT);
     source.setFieldsMap(map);

     unsigned int read_records = 0, expected_records = 10;
     while (source.ReadRecord()) {
         ++read_records;
         long num_values = source.GetNumValues();
         BOOST_CHECK_MESSAGE( num_values == 2, "Expecting 2 values but got " << num_values);

         ExpectedVals_t exp;
         switch (read_records) {
            case 1 : exp.push_back(std::make_pair(0,"location1")); exp.push_back(std::make_pair(1,"1")); break;
            case 2 : exp.push_back(std::make_pair(0,"location2")); exp.push_back(std::make_pair(1,"1")); break;
            case 3 : exp.push_back(std::make_pair(0,"location3")); exp.push_back(std::make_pair(1,"1")); break;
            case 4 : exp.push_back(std::make_pair(0,"location4")); exp.push_back(std::make_pair(1,"1")); break;
            case 5 : exp.push_back(std::make_pair(0,"location5")); exp.push_back(std::make_pair(1,"1")); break;
            case 6 : exp.push_back(std::make_pair(0,"location6")); exp.push_back(std::make_pair(1,"1")); break;
            case 7 : exp.push_back(std::make_pair(0,"location7")); exp.push_back(std::make_pair(1,"1")); break;
            case 8 : exp.push_back(std::make_pair(0,"location8")); exp.push_back(std::make_pair(1,"1")); break;
            case 9 : exp.push_back(std::make_pair(0,"location9")); exp.push_back(std::make_pair(1,"1")); break;
            case 10 : exp.push_back(std::make_pair(0,"location10")); exp.push_back(std::make_pair(1,"1")); break;
         }
         compareExpected(source, exp);
     }
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);
}

/* Tests reading population file as shapefile data source. */
BOOST_AUTO_TEST_CASE( shapefile_data_source_population ) {
     PrintNull print;
     std::stringstream filename;
     filename << getTestSetFilesPath().c_str() << "\\datasources\\test_shapefile.shp";
     ShapeFileDataSource source(filename.str());
     std::vector<boost::any> map;
     map.push_back(DataSource::GENERATEDID);
     map.push_back(DataSource::DEFAULT_DATE);
     map.push_back(static_cast<long>(1));
     source.setFieldsMap(map);

     unsigned int read_records = 0, expected_records = 10;
     while (source.ReadRecord()) {
         ++read_records;
         long num_values = source.GetNumValues();
         BOOST_CHECK_MESSAGE( num_values == 3, "Expecting 3 values but got " << num_values);

         ExpectedVals_t exp;
         switch (read_records) {
            case 1 : exp.push_back(std::make_pair(0,"location1")); exp.push_back(std::make_pair(1,DateStringParser::UNSPECIFIED)); exp.push_back(std::make_pair(2,"2081.54")); break;
            case 2 : exp.push_back(std::make_pair(0,"location2")); exp.push_back(std::make_pair(1,DateStringParser::UNSPECIFIED)); exp.push_back(std::make_pair(2,"7842")); break;
            case 3 : exp.push_back(std::make_pair(0,"location3")); exp.push_back(std::make_pair(1,DateStringParser::UNSPECIFIED)); exp.push_back(std::make_pair(2,"1140.65")); break;
            case 4 : exp.push_back(std::make_pair(0,"location4")); exp.push_back(std::make_pair(1,DateStringParser::UNSPECIFIED)); exp.push_back(std::make_pair(2,"107.511")); break;
            case 5 : exp.push_back(std::make_pair(0,"location5")); exp.push_back(std::make_pair(1,DateStringParser::UNSPECIFIED)); exp.push_back(std::make_pair(2,"9659.11")); break;
            case 6 : exp.push_back(std::make_pair(0,"location6")); exp.push_back(std::make_pair(1,DateStringParser::UNSPECIFIED)); exp.push_back(std::make_pair(2,"4990.75")); break;
            case 7 : exp.push_back(std::make_pair(0,"location7")); exp.push_back(std::make_pair(1,DateStringParser::UNSPECIFIED)); exp.push_back(std::make_pair(2,"8702.27")); break;
            case 8 : exp.push_back(std::make_pair(0,"location8")); exp.push_back(std::make_pair(1,DateStringParser::UNSPECIFIED)); exp.push_back(std::make_pair(2,"7099.69")); break;
            case 9 : exp.push_back(std::make_pair(0,"location9")); exp.push_back(std::make_pair(1,DateStringParser::UNSPECIFIED)); exp.push_back(std::make_pair(2,"9122.87")); break;
            case 10 : exp.push_back(std::make_pair(0,"location10")); exp.push_back(std::make_pair(1,DateStringParser::UNSPECIFIED)); exp.push_back(std::make_pair(2,"9964.52")); break;
         }
         compareExpected(source, exp);
     }
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);
}

/* Tests reading coordinates file as shapefile data source. */
BOOST_AUTO_TEST_CASE( shapefile_data_source_coordinates ) {
     PrintNull print;
     std::stringstream filename;
     filename << getTestSetFilesPath().c_str() << "\\datasources\\test_shapefile.shp";
     ShapeFileDataSource source(filename.str());
     std::vector<boost::any> map;
     map.push_back(DataSource::GENERATEDID);
     map.push_back(DataSource::POINTX);
     map.push_back(DataSource::POINTY);
     source.setFieldsMap(map);

     unsigned int read_records = 0, expected_records = 10;
     while (source.ReadRecord()) {
         ++read_records;
         long num_values = source.GetNumValues();
         BOOST_CHECK_MESSAGE( num_values == 3, "Expecting 3 values but got " << num_values);

         ExpectedVals_t exp;
         switch (read_records) {
            case 1 : exp.push_back(std::make_pair(0,"location1")); exp.push_back(std::make_pair(1,"76.6966")); exp.push_back(std::make_pair(2,"23.9392")); break;
            case 2 : exp.push_back(std::make_pair(0,"location2")); exp.push_back(std::make_pair(1,"-70.1657")); exp.push_back(std::make_pair(2,"67.3374")); break;
            case 3 : exp.push_back(std::make_pair(0,"location3")); exp.push_back(std::make_pair(1,"-44.3045")); exp.push_back(std::make_pair(2,"24.1842")); break;
            case 4 : exp.push_back(std::make_pair(0,"location4")); exp.push_back(std::make_pair(1,"63.8561")); exp.push_back(std::make_pair(2,"96.0248")); break;
            case 5 : exp.push_back(std::make_pair(0,"location5")); exp.push_back(std::make_pair(1,"15.2246")); exp.push_back(std::make_pair(2,"-6.18035")); break;
            case 6 : exp.push_back(std::make_pair(0,"location6")); exp.push_back(std::make_pair(1,"31.9006")); exp.push_back(std::make_pair(2,"-72.8662")); break;
            case 7 : exp.push_back(std::make_pair(0,"location7")); exp.push_back(std::make_pair(1,"22.6314")); exp.push_back(std::make_pair(2,"42.6784")); break;
            case 8 : exp.push_back(std::make_pair(0,"location8")); exp.push_back(std::make_pair(1,"28.0106")); exp.push_back(std::make_pair(2,"92.2593")); break;
            case 9 : exp.push_back(std::make_pair(0,"location9")); exp.push_back(std::make_pair(1,"48.5819")); exp.push_back(std::make_pair(2,"95.8949")); break;
            case 10 : exp.push_back(std::make_pair(0,"location10")); exp.push_back(std::make_pair(1,"40.4188")); exp.push_back(std::make_pair(2,"57.8722")); break;
         }
         compareExpected(source, exp);
     }
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);
}

/* Tests reading grid file as shapefile data source. */
BOOST_AUTO_TEST_CASE( shapefile_data_source_grid ) {
     PrintNull print;
     std::stringstream filename;
     filename << getTestSetFilesPath().c_str() << "\\datasources\\test_shapefile.shp";
     ShapeFileDataSource source(filename.str());
     std::vector<boost::any> map;
     map.push_back(DataSource::POINTX);
     map.push_back(DataSource::POINTY);
     source.setFieldsMap(map);

     unsigned int read_records = 0, expected_records = 10;
     while (source.ReadRecord()) {
         ++read_records;
         long num_values = source.GetNumValues();
         BOOST_CHECK_MESSAGE( num_values == 2, "Expecting 2 values but got " << num_values);

         ExpectedVals_t exp;
         switch (read_records) {
            case 1 : exp.push_back(std::make_pair(0,"76.6966")); exp.push_back(std::make_pair(1,"23.9392")); break;
            case 2 : exp.push_back(std::make_pair(0,"-70.1657")); exp.push_back(std::make_pair(1,"67.3374")); break;
            case 3 : exp.push_back(std::make_pair(0,"-44.3045")); exp.push_back(std::make_pair(1,"24.1842")); break;
            case 4 : exp.push_back(std::make_pair(0,"63.8561")); exp.push_back(std::make_pair(1,"96.0248")); break;
            case 5 : exp.push_back(std::make_pair(0,"15.2246")); exp.push_back(std::make_pair(1,"-6.18035")); break;
            case 6 : exp.push_back(std::make_pair(0,"31.9006")); exp.push_back(std::make_pair(1,"-72.8662")); break;
            case 7 : exp.push_back(std::make_pair(0,"22.6314")); exp.push_back(std::make_pair(1,"42.6784")); break;
            case 8 : exp.push_back(std::make_pair(0,"28.0106")); exp.push_back(std::make_pair(1,"92.2593")); break;
            case 9 : exp.push_back(std::make_pair(0,"48.5819")); exp.push_back(std::make_pair(1,"95.8949")); break;
            case 10 : exp.push_back(std::make_pair(0,"40.4188")); exp.push_back(std::make_pair(1,"57.8722")); break;
         }
         compareExpected(source, exp);
     }
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);
}

BOOST_AUTO_TEST_SUITE_END()

struct ordinal_datasource_fixture {
    ordinal_datasource_fixture() {}
    virtual ~ordinal_datasource_fixture() { }

    void compare_expected(DataSource& source, unsigned int recIdx) {
        long num_values = source.GetNumValues();
        BOOST_CHECK_MESSAGE( num_values == 3, "Expecting 3 values but got " << num_values);

        ExpectedVals_t exp;
        switch (recIdx) {
            case 1 : exp.push_back(std::make_pair(0,"location1")); exp.push_back(std::make_pair(1,"1")); exp.push_back(std::make_pair(2,"11")); break;
            case 2 : exp.push_back(std::make_pair(0,"location2")); exp.push_back(std::make_pair(1,"2")); exp.push_back(std::make_pair(2,"22")); break;
            case 3 : exp.push_back(std::make_pair(0,"location3")); exp.push_back(std::make_pair(1,"3")); exp.push_back(std::make_pair(2,"33")); break;
            case 4 : exp.push_back(std::make_pair(0,"location4")); exp.push_back(std::make_pair(1,"4")); exp.push_back(std::make_pair(2,"44")); break;
            case 5 : exp.push_back(std::make_pair(0,"location5")); exp.push_back(std::make_pair(1,"5")); exp.push_back(std::make_pair(2,"55")); break;
            case 6 : exp.push_back(std::make_pair(0,"location6")); exp.push_back(std::make_pair(1,"6")); exp.push_back(std::make_pair(2,"66")); break;
            case 7 : exp.push_back(std::make_pair(0,"location7")); exp.push_back(std::make_pair(1,"7")); exp.push_back(std::make_pair(2,"77")); break;
            case 8 : exp.push_back(std::make_pair(0,"location8")); exp.push_back(std::make_pair(1,"8")); exp.push_back(std::make_pair(2,"88")); break;
            case 9 : exp.push_back(std::make_pair(0,"location9")); exp.push_back(std::make_pair(1,"9")); exp.push_back(std::make_pair(2,"99")); break;
            case 10 : exp.push_back(std::make_pair(0,"location10")); exp.push_back(std::make_pair(1,"10")); exp.push_back(std::make_pair(2,"110")); break;
        }
        compareExpected(source, exp);
    }
};

BOOST_AUTO_TEST_SUITE( data_source_with_blank_column )

/* Tests reading data source where field map includes a blank field. This situation could happen with models
   such as ordinal and a precision of time = none. In other words, the date column is blank in mapping. */
BOOST_FIXTURE_TEST_CASE( csv_data_source_ordinal, ordinal_datasource_fixture ) {
     PrintNull print;
     std::stringstream filename;
     filename << getTestSetFilesPath().c_str() << "\\datasources\\test_case_ordinal.csv";
     CsvFileDataSource source(filename.str(), print, ",", "\"", 2, true);
     std::vector<boost::any> map;
     map.push_back(static_cast<long>(1));
     map.push_back(static_cast<long>(2));
     map.push_back(DataSource::BLANK);
     map.push_back(static_cast<long>(3));
     source.setFieldsMap(map);

     unsigned int read_records = 0, expected_records = 10;
     while (source.ReadRecord()) {
         ++read_records;
         compare_expected(source, read_records);
     }
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);

     source.GotoFirstRecord();
     read_records = 0;
     while (source.ReadRecord()) {
         ++read_records;
         compare_expected(source, read_records);
     }
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);
}

BOOST_AUTO_TEST_SUITE_END()

struct shapefile_datasource_many_columns_fixture {
    shapefile_datasource_many_columns_fixture() {}
    virtual ~shapefile_datasource_many_columns_fixture() { }

    void compare_expected(DataSource& source, unsigned int recIdx, long num_expected=3) {
        long num_values = source.GetNumValues();
        BOOST_CHECK_MESSAGE( num_values == num_expected, "Expecting " << num_expected << " values but got " << num_values);

         ExpectedVals_t exp;
         switch (recIdx) {
            case 1 : exp.push_back(std::make_pair(0,"76.6966")); exp.push_back(std::make_pair(1,"location1")); exp.push_back(std::make_pair(2,"NewYork")); exp.push_back(std::make_pair(3,"location1")); exp.push_back(std::make_pair(4,"+65")); exp.push_back(std::make_pair(5,"5")); exp.push_back(std::make_pair(6,"23.9392")); break;
            case 2 : exp.push_back(std::make_pair(0,"-70.1657")); exp.push_back(std::make_pair(1,"location2")); exp.push_back(std::make_pair(2,"Baltimore")); exp.push_back(std::make_pair(3,"location2")); exp.push_back(std::make_pair(4,"<10")); exp.push_back(std::make_pair(5,"1")); exp.push_back(std::make_pair(6,"67.3374")); break;
            case 3 : exp.push_back(std::make_pair(0,"-44.3045")); exp.push_back(std::make_pair(1,"location3")); exp.push_back(std::make_pair(2,"WashingtonDC")); exp.push_back(std::make_pair(3,"location3")); exp.push_back(std::make_pair(4,"=40")); exp.push_back(std::make_pair(5,"2")); exp.push_back(std::make_pair(6,"24.1842")); break;
            case 4 : exp.push_back(std::make_pair(0,"63.8561")); exp.push_back(std::make_pair(1,"location4")); exp.push_back(std::make_pair(2,"Richmond")); exp.push_back(std::make_pair(3,"location4")); exp.push_back(std::make_pair(4,"<65")); exp.push_back(std::make_pair(5,"5")); exp.push_back(std::make_pair(6,"96.0248")); break;
            case 5 : exp.push_back(std::make_pair(0,"15.2246")); exp.push_back(std::make_pair(1,"location5")); exp.push_back(std::make_pair(2,"TampaBay")); exp.push_back(std::make_pair(3,"location5")); exp.push_back(std::make_pair(4,"60+")); exp.push_back(std::make_pair(5,"1")); exp.push_back(std::make_pair(6,"-6.18035")); break;
            case 6 : exp.push_back(std::make_pair(0,"31.9006")); exp.push_back(std::make_pair(1,"location6")); exp.push_back(std::make_pair(2,"BatonRouge")); exp.push_back(std::make_pair(3,"location6")); exp.push_back(std::make_pair(4,"<12")); exp.push_back(std::make_pair(5,"5")); exp.push_back(std::make_pair(6,"-72.8662")); break;
            case 7 : exp.push_back(std::make_pair(0,"22.6314")); exp.push_back(std::make_pair(1,"location7")); exp.push_back(std::make_pair(2,"SanFransico")); exp.push_back(std::make_pair(3,"location7")); exp.push_back(std::make_pair(4,"<65")); exp.push_back(std::make_pair(5,"4")); exp.push_back(std::make_pair(6,"42.6784")); break;
            case 8 : exp.push_back(std::make_pair(0,"28.0106")); exp.push_back(std::make_pair(1,"location8")); exp.push_back(std::make_pair(2,"Portland")); exp.push_back(std::make_pair(3,"location8")); exp.push_back(std::make_pair(4,"=50")); exp.push_back(std::make_pair(5,"3")); exp.push_back(std::make_pair(6,"92.2593")); break;
            case 9 : exp.push_back(std::make_pair(0,"48.5819")); exp.push_back(std::make_pair(1,"location9")); exp.push_back(std::make_pair(2,"StLouis")); exp.push_back(std::make_pair(3,"location9")); exp.push_back(std::make_pair(4,"+65")); exp.push_back(std::make_pair(5,"5")); exp.push_back(std::make_pair(6,"95.8949")); break;
            case 10 : exp.push_back(std::make_pair(0,"40.4188")); exp.push_back(std::make_pair(1,"location10")); exp.push_back(std::make_pair(2,"Wallkill")); exp.push_back(std::make_pair(3,"location10")); exp.push_back(std::make_pair(4,"<65")); exp.push_back(std::make_pair(5,"2")); exp.push_back(std::make_pair(6,"57.8722")); break;
         }
         compareExpected(source, exp);
    }
};

BOOST_AUTO_TEST_SUITE( data_source_with_extra_column )

BOOST_FIXTURE_TEST_CASE( data_source_extra_cas, plain_datasource_fixture ) {
     PrintNull print;
     std::stringstream filename;
     filename << getTestSetFilesPath().c_str() << "\\datasources\\test_many_columns.cas";
     CsvFileDataSource source(filename.str(), print, " ", "\"", 0, false);
     std::vector<boost::any> map;
     map.push_back(static_cast<long>(5)); // location id
     map.push_back(static_cast<long>(1)); // number of case
     map.push_back(static_cast<long>(7)); // date
     source.setFieldsMap(map);

     unsigned int read_records = 0, expected_records = 10;
     while (source.ReadRecord()) {
         ++read_records;
         compare_expected(source, read_records);
     }
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);

     source.GotoFirstRecord();
     read_records = 0;
     while (source.ReadRecord()) {
         ++read_records;
         compare_expected(source, read_records);
     }
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);
}

BOOST_FIXTURE_TEST_CASE( data_source_extra_csv, plain_datasource_fixture ) {
     PrintNull print;
     std::stringstream filename;
     filename << getTestSetFilesPath().c_str() << "\\datasources\\test_many_columns.csv";
     CsvFileDataSource source(filename.str(), print, ",", "\"", 0, false);
     std::vector<boost::any> map;
     map.push_back(static_cast<long>(5));
     map.push_back(static_cast<long>(1));
     map.push_back(static_cast<long>(7));
     source.setFieldsMap(map);

     unsigned int read_records = 0, expected_records = 10;
     while (source.ReadRecord()) {
         ++read_records;
         compare_expected(source, read_records);
     }
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);

     source.GotoFirstRecord();
     read_records = 0;
     while (source.ReadRecord()) {
         ++read_records;
         compare_expected(source, read_records);
     }
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);
}

BOOST_FIXTURE_TEST_CASE( data_source_extra_dbf, plain_datasource_fixture ) {
     PrintNull print;
     std::stringstream filename;
     filename << getTestSetFilesPath().c_str() << "\\datasources\\test_many_columns.dbf";
     dBaseFileDataSource source(filename.str());
     std::vector<boost::any> map;
     map.push_back(static_cast<long>(5));
     map.push_back(static_cast<long>(1));
     map.push_back(static_cast<long>(7));
     source.setFieldsMap(map);

     unsigned int read_records = 0, expected_records = 10;
     while (source.ReadRecord()) {
         ++read_records;
         compare_expected(source, read_records);
     }
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);

     source.GotoFirstRecord();
     read_records = 0;
     while (source.ReadRecord()) {
         ++read_records;
         compare_expected(source, read_records);
     }
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);
}

BOOST_FIXTURE_TEST_CASE( data_source_extra_shapefile, shapefile_datasource_many_columns_fixture ) {
     PrintNull print;
     std::stringstream filename;
     filename << getTestSetFilesPath().c_str() << "\\datasources\\test_shapefile_many_columns.shp";
     ShapeFileDataSource source(filename.str());
     std::vector<boost::any> map;
     map.push_back(DataSource::POINTX);
     map.push_back(static_cast<long>(5));
     map.push_back(static_cast<long>(4));
     map.push_back(DataSource::GENERATEDID);
     map.push_back(static_cast<long>(9));
     map.push_back(static_cast<long>(1));
     map.push_back(DataSource::POINTY);
     source.setFieldsMap(map);

     unsigned int read_records = 0, expected_records = 10;
     while (source.ReadRecord()) {
         ++read_records;
         compare_expected(source, read_records, 7);
     }
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);

     source.GotoFirstRecord();
     read_records = 0;
     while (source.ReadRecord()) {
         ++read_records;
         compare_expected(source, read_records, 7);
     }
     BOOST_CHECK_MESSAGE( read_records == expected_records, "Expecting " << expected_records << " records got " << read_records);
}

BOOST_AUTO_TEST_SUITE_END()

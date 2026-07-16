#include <boost/test/unit_test.hpp>

#include "FileName.h"

BOOST_AUTO_TEST_SUITE(file_name_suite)

BOOST_AUTO_TEST_CASE(set_extension_normalizes_leading_dot) {
    std::string full_path;
    FileName filename;

    filename.setFileName("analysis");
    filename.setExtension("txt");
    BOOST_CHECK_EQUAL(filename.getExtension(), ".txt");
    BOOST_CHECK_EQUAL(filename.getFullPath(full_path), "analysis.txt");

    filename.setExtension(".csv");
    BOOST_CHECK_EQUAL(filename.getExtension(), ".csv");
    BOOST_CHECK_EQUAL(filename.getFullPath(full_path), "analysis.csv");
}

BOOST_AUTO_TEST_CASE(set_directory_appends_platform_separator) {
    FileName filename;
    filename.setDirectory("results");

    std::string expected("results");
    expected += FileName::getPathSeparator();
    BOOST_CHECK_EQUAL(filename.getDirectory(), expected);
}

BOOST_AUTO_TEST_CASE(set_location_and_file_name_round_trip) {
    FileName filename;
    std::string full_path;
    std::string location;
    const char separator = FileName::getPathSeparator();

    filename.setLocation("C:\\SaTScan\\results");
    filename.setFileName("run.output");
    filename.setExtension("txt");

    BOOST_CHECK_EQUAL(filename.getDrive(), "C:");
    BOOST_CHECK_EQUAL(filename.getDirectory(), std::string("\\SaTScan\\results") + separator);
    BOOST_CHECK_EQUAL(filename.getLocation(location), std::string("C:\\SaTScan\\results") + separator);
    BOOST_CHECK_EQUAL(filename.getFullPath(full_path), std::string("C:\\SaTScan\\results") + separator + "run.output.txt");
}

#ifdef _WINDOWS_
BOOST_AUTO_TEST_CASE(unc_location_splits_server_and_share_as_drive) {
    FileName filename;
    std::string location;

    filename.setLocation("\\\\server\\share\\folder");

    BOOST_CHECK_EQUAL(filename.getDrive(), "\\\\server\\share");
    BOOST_CHECK_EQUAL(filename.getDirectory(), "\\folder\\");
    BOOST_CHECK_EQUAL(filename.getLocation(location), "\\\\server\\share\\folder\\");
}
#endif

BOOST_AUTO_TEST_SUITE_END()

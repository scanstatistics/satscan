
// Boost unit test header
#include <boost/test/unit_test.hpp>

// project files
#include "JulianDates.h"
#include "SSException.h"

/* Test Suite for the Julian date conversion functions. */
BOOST_AUTO_TEST_SUITE( julian_suite )

/* Tests conversion from C string to julian value. */
BOOST_AUTO_TEST_CASE( test_char_to_julian ) {
    BOOST_CHECK_EQUAL( CharToJulian("1900/1/1"), 2415021 );
    BOOST_CHECK_EQUAL( CharToJulian("2000/1/1"), 2451545 );
    BOOST_CHECK_EQUAL( CharToJulian("3000/1/1"), 2816788 );

    // test incorrectly formatted dates return zero
    BOOST_CHECK_EQUAL( CharToJulian("200011"), 0 );
    BOOST_CHECK_EQUAL( CharToJulian("2000-1-1"), 0 );
    BOOST_CHECK_EQUAL( CharToJulian("pickle"), 0 );
}

/* Tests conversion from julian value to C string. */
BOOST_AUTO_TEST_CASE( test_julian_to_char ) {
    char s[11];
    BOOST_CHECK_EQUAL( JulianToChar(s, 2415021), "1900/1/1" );
    BOOST_CHECK_EQUAL( JulianToChar(s, 2451545), "2000/1/1" );
    BOOST_CHECK_EQUAL( JulianToChar(s, 2816788), "3000/1/1" );

    BOOST_CHECK_NE( JulianToChar(s, 2816788), "3001/1/1" );
}

/* Tests conversion from generic date to julian value. */
BOOST_AUTO_TEST_CASE( test_relative_to_julian ) {
    // test lower boundary
    BOOST_CHECK_THROW( relativeDateToJulian("-219146"), resolvable_error );
    BOOST_CHECK_EQUAL( relativeDateToJulian("-200000"), 2251545 );

    BOOST_CHECK_EQUAL( relativeDateToJulian("-50"), 2451495 );
    BOOST_CHECK_EQUAL( relativeDateToJulian("0"), 2451545 );
    BOOST_CHECK_EQUAL( relativeDateToJulian("200"), 2451745 );
    BOOST_CHECK_EQUAL( relativeDateToJulian("6000"), 2457545 );

    // test upper boundary
    BOOST_CHECK_EQUAL( relativeDateToJulian("2921938"), 5373483 );
    BOOST_CHECK_THROW( relativeDateToJulian("2921939"), resolvable_error );

    // test incorrectly formatted dates throw resolvable_error exception
    BOOST_CHECK_THROW( relativeDateToJulian("200,000"), resolvable_error );
    BOOST_CHECK_THROW( relativeDateToJulian("1900/1/1"), resolvable_error );
    BOOST_CHECK_THROW( relativeDateToJulian("pickle"), resolvable_error );
}

BOOST_AUTO_TEST_SUITE_END()

#include <boost/test/unit_test.hpp>

#include "DateStringParser.h"

BOOST_AUTO_TEST_SUITE(date_string_parser_suite)

BOOST_AUTO_TEST_CASE(parse_count_date_accepts_supported_separators) {
    const Julian period_start = MDYToJulian(1, 1, 1990);
    const Julian period_end = MDYToJulian(12, 31, 2000);
    DateStringParser parser(DAY);
    Julian parsed = 0;

    BOOST_CHECK_EQUAL(parser.ParseCountDateString("1995/02/05", DAY, period_start, period_end, parsed), DateStringParser::VALID_DATE);
    BOOST_CHECK_EQUAL(parsed, MDYToJulian(2, 5, 1995));

    BOOST_CHECK_EQUAL(parser.ParseCountDateString("1995-02-05", DAY, period_start, period_end, parsed), DateStringParser::VALID_DATE);
    BOOST_CHECK_EQUAL(parsed, MDYToJulian(2, 5, 1995));

    BOOST_CHECK_EQUAL(parser.ParseCountDateString("1995.02.05", DAY, period_start, period_end, parsed), DateStringParser::VALID_DATE);
    BOOST_CHECK_EQUAL(parsed, MDYToJulian(2, 5, 1995));

    BOOST_CHECK_EQUAL(parser.ParseCountDateString("1995*02*05", DAY, period_start, period_end, parsed), DateStringParser::VALID_DATE);
    BOOST_CHECK_EQUAL(parsed, MDYToJulian(2, 5, 1995));
}

BOOST_AUTO_TEST_CASE(parse_count_date_accepts_month_day_year_format) {
    const Julian period_start = MDYToJulian(1, 1, 1990);
    const Julian period_end = MDYToJulian(12, 31, 2000);
    DateStringParser parser(DAY);
    Julian parsed = 0;

    BOOST_CHECK_EQUAL(parser.ParseCountDateString("02/05/1995", DAY, period_start, period_end, parsed), DateStringParser::VALID_DATE);
    BOOST_CHECK_EQUAL(parsed, MDYToJulian(2, 5, 1995));
}

BOOST_AUTO_TEST_CASE(parse_count_date_reports_lesser_precision) {
    const Julian period_start = MDYToJulian(1, 1, 1990);
    const Julian period_end = MDYToJulian(12, 31, 2000);
    DateStringParser parser(DAY);
    Julian parsed = 0;

    BOOST_CHECK_EQUAL(parser.ParseCountDateString("1995/02", DAY, period_start, period_end, parsed), DateStringParser::LESSER_PRECISION);
}

BOOST_AUTO_TEST_CASE(parse_count_date_reports_ambiguous_two_digit_year) {
    const Julian period_start = MDYToJulian(1, 1, 1900);
    const Julian period_end = MDYToJulian(12, 31, 2000);
    DateStringParser parser(DAY);
    Julian parsed = 0;

    BOOST_CHECK_EQUAL(parser.ParseCountDateString("95/02/05", DAY, period_start, period_end, parsed), DateStringParser::AMBIGUOUS_YEAR);
}

BOOST_AUTO_TEST_CASE(parse_adjustment_date_uses_period_boundary_for_partial_dates) {
    const Julian period_start = MDYToJulian(1, 1, 1990);
    const Julian period_end = MDYToJulian(12, 31, 2000);
    DateStringParser parser(DAY);
    Julian parsed = 0;

    BOOST_CHECK_EQUAL(parser.ParseAdjustmentDateString("1995/02", period_start, period_end, parsed, true), DateStringParser::VALID_DATE);
    BOOST_CHECK_EQUAL(parsed, MDYToJulian(2, 1, 1995));

    BOOST_CHECK_EQUAL(parser.ParseAdjustmentDateString("1995/02", period_start, period_end, parsed, false), DateStringParser::VALID_DATE);
    BOOST_CHECK_EQUAL(parsed, MDYToJulian(2, 28, 1995));
}

BOOST_AUTO_TEST_CASE(parse_population_date_uses_population_defaults) {
    const Julian period_start = MDYToJulian(1, 1, 1990);
    const Julian period_end = MDYToJulian(12, 31, 2000);
    DateStringParser parser(DAY);
    DatePrecisionType precision = NONE;
    Julian parsed = 0;

    BOOST_CHECK_EQUAL(parser.ParsePopulationDateString("1995", period_start, period_end, parsed, precision), DateStringParser::VALID_DATE);
    BOOST_CHECK_EQUAL(precision, YEAR);
    BOOST_CHECK_EQUAL(parsed, MDYToJulian(7, 1, 1995));

    BOOST_CHECK_EQUAL(parser.ParsePopulationDateString("1995/02", period_start, period_end, parsed, precision), DateStringParser::VALID_DATE);
    BOOST_CHECK_EQUAL(precision, MONTH);
    BOOST_CHECK_EQUAL(parsed, MDYToJulian(2, 15, 1995));
}

BOOST_AUTO_TEST_CASE(parse_population_unspecified_uses_period_start) {
    const Julian period_start = MDYToJulian(1, 1, 1990);
    const Julian period_end = MDYToJulian(12, 31, 2000);
    DateStringParser parser(DAY);
    DatePrecisionType precision = NONE;
    Julian parsed = 0;

    BOOST_CHECK_EQUAL(parser.ParsePopulationDateString(" unspecified ", period_start, period_end, parsed, precision), DateStringParser::VALID_DATE);
    BOOST_CHECK_EQUAL(precision, DAY);
    BOOST_CHECK_EQUAL(parsed, period_start);
}

BOOST_AUTO_TEST_SUITE_END()

#include <boost/test/unit_test.hpp>

#include "UtilityFunctions.h"

#include <sstream>
#include <string>
#include <vector>

BOOST_AUTO_TEST_SUITE(utility_functions_suite)

BOOST_AUTO_TEST_CASE(trim_and_lower_modify_strings_in_place) {
    std::string value("  Mixed Case  ");

    BOOST_CHECK_EQUAL(trimString(value), "Mixed Case");
    BOOST_CHECK_EQUAL(lowerString(value), "mixed case");
}

BOOST_AUTO_TEST_CASE(ordinal_suffix_handles_teens_and_regular_suffixes) {
    BOOST_CHECK_EQUAL(ordinal_suffix(1), "st");
    BOOST_CHECK_EQUAL(ordinal_suffix(2), "nd");
    BOOST_CHECK_EQUAL(ordinal_suffix(3), "rd");
    BOOST_CHECK_EQUAL(ordinal_suffix(4), "th");
    BOOST_CHECK_EQUAL(ordinal_suffix(11), "th");
    BOOST_CHECK_EQUAL(ordinal_suffix(12), "th");
    BOOST_CHECK_EQUAL(ordinal_suffix(13), "th");
    BOOST_CHECK_EQUAL(ordinal_suffix(21), "st");
}

BOOST_AUTO_TEST_CASE(csv_string_to_typelist_parses_numbers_and_reports_invalid_tokens) {
    std::vector<int> values;

    BOOST_CHECK(csv_string_to_typelist("1, 2,3", values));
    BOOST_REQUIRE_EQUAL(values.size(), 3);
    BOOST_CHECK_EQUAL(values[0], 1);
    BOOST_CHECK_EQUAL(values[1], 2);
    BOOST_CHECK_EQUAL(values[2], 3);

    BOOST_CHECK_EQUAL(csv_string_to_typelist("1, nope, 3", values), false);
}

BOOST_AUTO_TEST_CASE(typelist_to_csv_string_quotes_values_containing_commas) {
    std::vector<std::string> values;
    values.push_back("alpha");
    values.push_back("beta,gamma");
    values.push_back("delta");

    std::string csv;
    BOOST_CHECK_EQUAL(typelist_to_csv_string(values, csv), "alpha,\"beta,gamma\",delta");

    values.clear();
    BOOST_CHECK_EQUAL(typelist_to_csv_string(values, csv), "");
}

BOOST_AUTO_TEST_CASE(htmlencode_encodes_punctuation_and_optional_spaces) {
    std::string encoded;

    BOOST_CHECK_EQUAL(htmlencode("A&B C", encoded, true), "A&#38;B&#32;C");
    BOOST_CHECK_EQUAL(htmlencode("A&B C", encoded, false), "A&#38;B C");
}

BOOST_AUTO_TEST_CASE(title_case_and_ends_with_cover_basic_edges) {
    BOOST_CHECK_EQUAL(toTitleCase("new_mexico-case"), "New_Mexico-Case");
    BOOST_CHECK(ends_with("results.txt", ".txt"));
    BOOST_CHECK(!ends_with("results.txt", ".csv"));
    BOOST_CHECK(!ends_with("txt", "results.txt"));
}

BOOST_AUTO_TEST_CASE(base64_encode_pads_short_inputs) {
    std::stringstream source;
    std::stringstream destination;

    source << "SaT";
    BOOST_CHECK_EQUAL(base64Encode(source, destination).str(), "U2FU");

    source.str("");
    source.clear();
    source << "Sa";
    BOOST_CHECK_EQUAL(base64Encode(source, destination).str(), "U2E=");
}

BOOST_AUTO_TEST_SUITE_END()

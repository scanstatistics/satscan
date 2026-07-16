#include <boost/test/unit_test.hpp>

#include "MultipleDimensionArrayHandler.h"

#include <vector>

BOOST_AUTO_TEST_SUITE(multiple_dimension_array_handler_suite)

BOOST_AUTO_TEST_CASE(minimal_growth_array_constructs_from_vector_and_copies_deeply) {
    std::vector<int> source;
    source.push_back(3);
    source.push_back(1);
    source.push_back(2);

    MinimalGrowthArray<int> values(source);
    MinimalGrowthArray<int> copy(values);

    values[0] = 9;

    BOOST_CHECK_EQUAL(values.size(), 3);
    BOOST_CHECK_EQUAL(copy.size(), 3);
    BOOST_CHECK_EQUAL(copy[0], 3);
    BOOST_CHECK_EQUAL(copy[1], 1);
    BOOST_CHECK_EQUAL(copy[2], 2);
}

BOOST_AUTO_TEST_CASE(minimal_growth_array_add_can_append_or_sort) {
    std::vector<int> source;
    source.push_back(3);
    source.push_back(1);

    MinimalGrowthArray<int> appended(source);
    appended.add(2, false);
    BOOST_CHECK_EQUAL(appended[0], 3);
    BOOST_CHECK_EQUAL(appended[1], 1);
    BOOST_CHECK_EQUAL(appended[2], 2);

    MinimalGrowthArray<int> sorted(source);
    sorted.add(2, true);
    BOOST_CHECK_EQUAL(sorted[0], 1);
    BOOST_CHECK_EQUAL(sorted[1], 2);
    BOOST_CHECK_EQUAL(sorted[2], 3);
}

BOOST_AUTO_TEST_CASE(minimal_growth_array_at_checks_bounds_and_clear_resets_size) {
    std::vector<int> source;
    source.push_back(1);

    MinimalGrowthArray<int> values(source);

    BOOST_CHECK_EQUAL(values.at(0), 1);
    BOOST_CHECK_THROW(values.at(1), std::out_of_range);

    values.clear();
    BOOST_CHECK_EQUAL(values.size(), 0);
}

BOOST_AUTO_TEST_CASE(two_dimension_array_resizes_second_dimension_and_preserves_values) {
    TwoDimensionArrayHandler<int> values(2, 2, 1);
    values.GetArray()[0][1] = 7;

    values.ResizeSecondDimension(4, 9);

    BOOST_CHECK_EQUAL(values.Get1stDimension(), 2);
    BOOST_CHECK_EQUAL(values.Get2ndDimension(), 4);
    BOOST_CHECK_EQUAL(values.GetArray()[0][1], 7);
    BOOST_CHECK_EQUAL(values.GetArray()[0][2], 9);
    BOOST_CHECK_EQUAL(values.GetArray()[1][3], 9);
}

BOOST_AUTO_TEST_CASE(two_dimension_array_assignment_requires_matching_dimensions) {
    TwoDimensionArrayHandler<int> lhs(1, 2, 1);
    TwoDimensionArrayHandler<int> rhs(1, 3, 1);

    BOOST_CHECK_THROW(lhs = rhs, prg_exception);
}

BOOST_AUTO_TEST_SUITE_END()

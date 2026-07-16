#include <boost/test/unit_test.hpp>

#include "Ini.h"

#include <sstream>

BOOST_AUTO_TEST_SUITE(ini_suite)

BOOST_AUTO_TEST_CASE(ini_line_trims_key_value_and_removes_wrapping_quotes) {
    IniLine line("  key name  =  \"  retained value  \"  ");

    BOOST_CHECK_EQUAL(line.GetKey(), "key name");
    BOOST_CHECK_EQUAL(line.GetValue(), "  retained value  ");
}

BOOST_AUTO_TEST_CASE(ini_section_reads_values_comments_and_defaults) {
    std::stringstream stream;
    stream << ";leading comment\n";
    stream << "enabled = yes\n";
    stream << "count = 42\n";
    stream << "name = SaTScan\n";
    stream << "[Next]\n";

    IniSection section("Settings");
    section.Read(stream);

    BOOST_REQUIRE_EQUAL(section.GetNumLines(), 3);
    BOOST_CHECK_EQUAL(section.GetBool("enabled"), true);
    BOOST_CHECK_EQUAL(section.GetInt("count"), 42);
    BOOST_CHECK_EQUAL(section.GetInt("missing", 7), 7);
    BOOST_CHECK_EQUAL(section.GetComment(0).size(), 1);

    std::string value;
    BOOST_CHECK_EQUAL(section.GetString(value, "name"), "SaTScan");
    BOOST_CHECK_EQUAL(section.GetIsModified(), false);
}

BOOST_AUTO_TEST_CASE(ini_section_setters_update_existing_values_and_add_new_values) {
    IniSection section("Settings");
    section.AddLine("count", "1");

    section.SetInt("count", 2);
    section.SetBool("enabled", true);

    BOOST_CHECK_EQUAL(section.GetInt("count"), 2);
    BOOST_CHECK_EQUAL(section.GetBool("enabled"), true);
    BOOST_CHECK_EQUAL(section.GetNumLines(), 2);
    BOOST_CHECK_EQUAL(section.GetIsModified(), true);
}

BOOST_AUTO_TEST_CASE(ini_section_remove_line_by_key_and_value) {
    IniSection section("Settings");
    section.AddLine("name", "SaTScan");
    section.AddLine("other", "value");

    BOOST_CHECK_EQUAL(section.RemoveLine("name", "SaTScan"), true);
    BOOST_CHECK_EQUAL(section.FindKey("name"), -1);
    BOOST_CHECK_EQUAL(section.GetNumLines(), 1);
    BOOST_CHECK_EQUAL(section.RemoveLine("other", "different"), false);
}

BOOST_AUTO_TEST_CASE(ini_file_round_trips_compact_stream) {
    IniFile file;
    IniSection * section = file.AddSection("Settings");
    section->AddLine("enabled", "true");
    section->AddLine("count", "42");

    std::stringstream written;
    file.Write(written, false, false);

    IniFile reread;
    reread.Read(written);
    const IniSection * reread_section = reread.GetSection("Settings");

    BOOST_REQUIRE(reread_section != 0);
    BOOST_CHECK_EQUAL(reread_section->GetBool("enabled"), true);
    BOOST_CHECK_EQUAL(reread_section->GetInt("count"), 42);
}

BOOST_AUTO_TEST_SUITE_END()

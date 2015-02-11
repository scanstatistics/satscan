
// project files
#include "fixture_sampledata.h"
#include "AnalysisRun.h"
#include "Toolkit.h"
#include "LocationRiskEstimateWriter.h"
#include <fstream>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

#include "IniParameterFileAccess.h"
#include "Ini.h"
#include "IniParameterSpecification.h"

/* data source field map cannot contain negative columns :https://www.squishlist.com/ims/satscan/66420/ */

/* Test Suite for the Parameters validation class. */
BOOST_FIXTURE_TEST_SUITE( squish66420_suite, new_mexico_fixture )

/* Tests that negative values are not permitted in source field map. */
BOOST_AUTO_TEST_CASE( squish66420_testcase ) {
    // read a valid parameter file
    BOOST_CHECK_EQUAL( IniParameterFileAccess(_parameters, _print).Read(_parameters.GetSourceFileName().c_str()), true );
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    // write parameter file out to user directory
    std::string user_directory;
    std::stringstream filename;
    filename << GetUserTemporaryDirectory(user_directory).c_str() << "\\test.prm";
    _parameters.SetSourceFileName(filename.str().c_str());
    IniParameterFileAccess(_parameters, _print).Write(_parameters.GetSourceFileName().c_str());
    // re-read parameter file from user directory
    BOOST_CHECK_EQUAL( IniParameterFileAccess(_parameters, _print).Read(_parameters.GetSourceFileName().c_str()), true );
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    // add data source mapping fields to the case file
    IniFile inifile;
    inifile.Read(_parameters.GetSourceFileName());
    IniSection * section = inifile.GetSection(IniParameterSpecification::Input);
    section->AddLine("CaseFile-SourceType", "0");
    section->AddLine("CaseFile-SourceFieldMap", "1,2,3");
    inifile.Write(_parameters.GetSourceFileName());
    // re-read parameter file from user directory
    BOOST_CHECK_EQUAL( IniParameterFileAccess(_parameters, _print).Read(_parameters.GetSourceFileName().c_str()), true );
    //still should validate
    BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );

    // now update source field map to invalid values -- negative
    IniLine * line = section->GetLine(section->FindKey("CaseFile-SourceFieldMap"));
    line->SetValue("1,2,-3");
    inifile.Write(_parameters.GetSourceFileName());
    // re-read parameter file from user directory -- read should fail
    BOOST_CHECK_EQUAL( IniParameterFileAccess(_parameters, _print).Read(_parameters.GetSourceFileName().c_str()), false );
}

BOOST_AUTO_TEST_SUITE_END()

#ifndef __FIXTURE_PRM_SOURCE_H
#define __FIXTURE_PRM_SOURCE_H

// Boost unit test header
#include <boost/test/unit_test.hpp>

// project files
#include "test_helper.h"
#include "Parameters.h"
#include "ParametersValidate.h"
#include "ParameterFileAccess.h"

struct prm_fixture {
    prm_fixture(const std::string& filename) {
        std::string path;
        // test reading parameters settings from file
        BOOST_CHECK_EQUAL( ParameterAccessCoordinator(_parameters).Read(getParameterFilePath(filename.c_str(), path).c_str(), _print), true );
        // make sure these parameters validated
        BOOST_CHECK_EQUAL( ParametersValidate(_parameters).Validate(_print), true );
    }
    virtual ~prm_fixture() { }

    CParameters _parameters;
    PrintNull _print;
};
#endif

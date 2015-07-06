
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

/* purely temporal analysis, adjustments for known relative risks,  : https://www.squishlist.com/ims/satscan/66440/ */

struct squish66440_analysis_fixture : new_mexico_fixture {
    squish66440_analysis_fixture() : new_mexico_fixture() { 
        // nm data set uses the Poisson model and space-time analysis initially
        _parameters.SetAnalysisType(PURELYTEMPORAL);
        std::stringstream filepath;
        filepath << getTestSetFilesPath().c_str() << "\\squish66440\\nm-no-bernoulli-1986.cas";
        _parameters.SetCaseFileName(filepath.str().c_str());
        _parameters.SetUseAdjustmentForRelativeRisksFile(true);
        filepath.str("");
        filepath << getTestSetFilesPath().c_str() << "\\squish66440\\nm.adj";
        _parameters.SetAdjustmentsByRelativeRisksFilename(filepath.str().c_str());
    }
    virtual ~squish66440_analysis_fixture() {}

    std::string _results_user_directory;
};

/* Test Suite for the purely temporal analysis adjusting for known relative risks at the location level. */
BOOST_FIXTURE_TEST_SUITE( squish66440_suite, squish66440_analysis_fixture )

BOOST_AUTO_TEST_CASE( squish66440_testcase ) {
    run_analysis("test", _results_user_directory, _parameters, _print);
}

BOOST_AUTO_TEST_SUITE_END()

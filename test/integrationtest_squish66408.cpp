
// project files
#include "fixture_sampledata.h"
#include "AnalysisRun.h"
#include "Toolkit.h"
#include "LoglikelihoodRatioWriter.h"
#include <boost/filesystem.hpp>

/* report power evaluation LLR values to separate file https://www.squishlist.com/ims/satscan/66408/ */

struct power_estimation_llr_file_fixture : new_mexico_fixture {
    power_estimation_llr_file_fixture() : new_mexico_fixture() { 
        /*BOOST_TEST_MESSAGE( "setup fixture -- points_analysis_fixture" );*/
        // nm data set uses the Poisson model and space-time analysis initially
        _parameters.SetAnalysisType(PURELYSPATIAL);
        // set performing power evaluations and ha filename
        _parameters.setPerformPowerEvaluation(true);
        std::stringstream ha_filename;
        ha_filename << getTestSetFilesPath().c_str() << "\\nm.ha";
        _parameters.setPowerEvaluationAltHypothesisFilename(ha_filename.str().c_str());
        // set reporting LLR values to file
        _parameters.SetOutputSimLogLikeliRatiosAscii(true);
        _parameters.SetOutputSimLogLikeliRatiosDBase(true);
    }
    virtual ~power_estimation_llr_file_fixture() { /*BOOST_TEST_MESSAGE( "teardown fixture -- power_estimation_llr_file_fixture" );*/ }

    std::string _results_user_directory;
};

/* Test Suite for the power evaluation llr reporting. */
BOOST_FIXTURE_TEST_SUITE( power_estimation_llr_suite, power_estimation_llr_file_fixture )

/* Tests generation of  */
BOOST_AUTO_TEST_CASE( test_power_estimation_llr_files ) {
    run_analysis("test", _results_user_directory, _parameters, _print);

    FileName filename(_parameters.GetOutputFileName().c_str());
    std::string buffer;
    // check for the existance of standard and power evaluation llr files.
    filename.setExtension(printString(buffer, "%s%s", LoglikelihoodRatioWriter::LOG_LIKELIHOOD_FILE_EXT, ASCIIDataFileWriter::ASCII_FILE_EXT).c_str());
    BOOST_CHECK_MESSAGE( boost::filesystem::exists( filename.getFullPath(buffer)) == true, "expecting existance of standard LLR file " << filename.getFullPath(buffer) );
    // confirm the number of expected records
    buffer = filename.getFileName() + filename.getExtension();
    std::ifstream stream;
    unsigned int filelines = getLineCount(getFileStream(stream, buffer, _results_user_directory)), expected = 999;
    BOOST_CHECK_MESSAGE(filelines == expected, "expecting " << expected << " rows, got " << filelines);

    filename.setExtension(printString(buffer, "%s%s", LoglikelihoodRatioWriter::LOG_LIKELIHOOD_FILE_HA_EXT, ASCIIDataFileWriter::ASCII_FILE_EXT).c_str());
    BOOST_CHECK_MESSAGE( boost::filesystem::exists( filename.getFullPath(buffer)) == true, "expecting existance of HA LLR file " << filename.getFullPath(buffer) );
    // confirm the number of expected records
    buffer = filename.getFileName() + filename.getExtension();
    stream.close();
    filelines = getLineCount(getFileStream(stream, buffer, _results_user_directory));
    expected = 1000;
    BOOST_CHECK_MESSAGE(filelines == expected, "expecting " << expected << " rows, got " << filelines);
}

BOOST_AUTO_TEST_SUITE_END()

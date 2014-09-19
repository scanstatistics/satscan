
#include "test_helper.h"
#include <boost/test/unit_test.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

std::string& getDataFilesPath(std::string& path) {
    namespace utf = boost::unit_test::framework;
    char ** argv = utf::master_test_suite().argv;
    int argc = utf::master_test_suite().argc;

    po::variables_map vm;
    po::options_description prg_options("", 200);
    prg_options.add_options()("fixture-datafiles-path,f", po::value<std::string>(), "path to fixture data files");

    try {
        po::store(po::command_line_parser(argc, argv).options(prg_options).style(po::command_line_style::default_style|po::command_line_style::case_insensitive).run(), vm);
        po::notify(vm);
        if (vm.count("fixture-datafiles-path"))
            path = vm["fixture-datafiles-path"].as<std::string>();
    } catch (std::exception& x) {
        path = "";
    }
    return path;
}

std::string& getParameterFilePath(const std::string& filename, std::string& path) {
    // retrieve the data files path to the sample data sets from the command-line arguments
    std::string datafilespath;
    getDataFilesPath(datafilespath);
    // define parameters of an analysis, so that we can read data and create appropriate data structures easily
    std::stringstream filepath;
    filepath << datafilespath.c_str() << "\\" << filename.c_str();
    path = filepath.str().c_str();
    return path;
}

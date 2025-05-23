#ifndef __TEST_HELPER_H
#define __TEST_HELPER_H

#include <string>
#include <vector>
#include <fstream>
#include <boost/program_options.hpp>
#include <algorithm>

namespace po = boost::program_options;
class CParameters;
class BasePrint;

po::options_description& addCustomOptions(po::options_description& prg_options);
std::string getSampleDataFilesPath();
std::string getTestSetFilesPath();
std::ifstream & getFileStream(std::ifstream& stream, const std::string& filename, std::string& results_user_directory);
unsigned int getLineCount(std::ifstream& stream);
void run_analysis(const std::string& analysis_name, std::string& results_user_directory, CParameters& parameters, BasePrint& print);

typedef std::vector<std::string> CSV_Row_t;
CSV_Row_t& getCSVRow(std::ifstream& stream, CSV_Row_t& row, const char * sep="\t\v\f\r\n ", const char * grp="\"");

bool compare_files(const std::string& filename1, const std::string& filename2);
#endif

//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "UtilityFunctions.h"
#include "Parameters.h"
#include "PurelySpatialData.h"
#include "PurelyTemporalData.h"
#include "SpaceTimeData.h"
#include "SVTTData.h"
#include "PrintScreen.h"
#include "AnalysisRun.h"
#include "ParameterFileAccess.h"
#include "ParametersValidate.h"
#include "SSException.h"
#include "ParametersPrint.h"
#include "Toolkit.h"
#include "newmat.h"
#include "ParameterProgramOptions.h"

#include <boost/program_options.hpp>
namespace po = boost::program_options;

void __SaTScanInit(const char * sApplicationFullPath) {
  reserve_memory_cache();
  std::set_new_handler(prg_new_handler);
  AppToolkit::ToolKitCreate(sApplicationFullPath);
}

void __SaTScanExit() {
  release_memory_cache();
  AppToolkit::ToolKitDestroy();
}

void usage_message(std::string program, ParameterProgramOptions::ParamOptContainer_t& opt_descriptions, po::options_description& desc, PrintScreen& console) {
    FileName            exe(program.c_str());
    std::stringstream   message;
    message << std::endl << "Usage: " << exe.getFileName().c_str() << exe.getExtension().c_str() << " <parameter file>(optional) [options]";
    message << std::endl << std::endl << desc;
    for (size_t t=0; t < opt_descriptions.size(); ++t) {
        if (opt_descriptions[t]->get<1>())
            message << std::endl << std::endl << opt_descriptions[t]->get<0>();
        if (opt_descriptions[t]->get<2>().size())
            message << std::endl << "  " << opt_descriptions[t]->get<2>();
    }
    console.Print(message.str().c_str(), BasePrint::P_STDOUT);
}

int main(int argc, char *argv[]) {
  bool verifyParameters=false, printParameters=false, forceCentric=false, allOut=false, standardPvalue=false;
  time_t RunTime;
  CParameters Parameters;
  std::string sMessage;
  PrintScreen Console(false);
  po::variables_map vm;

  try {
    __SaTScanInit(argv[0]);
    time(&RunTime);

    /* general options */
    po::options_description application("", 200);
    application.add_options()
        ("help,h", "Help")
        ("version,v", "Program version")
        ("parameter-file,f", po::value<std::string>(), "Parameter file")
        ("verify-parameters,c", po::bool_switch(&verifyParameters), "Verify parameters only")
        ("print-parameters,p", po::bool_switch(&printParameters), "Print parameters only");

    /* hidden options */
    po::options_description hidden("Hidden options", 200);
    hidden.add_options()("centric,t", po::bool_switch(&forceCentric), "Centric execution (overrides parameter file)")
                        ("all-out,a", po::bool_switch(&allOut), "All output files (overrides parameter file)")
                        ("standard-pvalue,d", po::bool_switch(&standardPvalue), "Report standard p-value (overrides parameter file)")                        
                        ("RandomSeed", po::value<std::string>(), "randomization seed (0 < Seed < 2147483647)")
                        ("RandomlyGenerateSeed", po::value<std::string>(), "randomly generate seed")
                        ;

    /* positional options */
    po::positional_options_description pd; 
    pd.add("parameter-file", 1);
    /* parse program options */
    po::options_description cmdline_options;

    ParameterProgramOptions::ParamOptContainer_t opt_descriptions;
    ParameterProgramOptions parameterOptions(Parameters, Console);
    parameterOptions.getOptions(opt_descriptions);
    for (size_t t=0; t < opt_descriptions.size(); ++t)
        cmdline_options.add(opt_descriptions[t]->get<0>());
    cmdline_options.add(application).add(hidden);
    //usage_message(argv[0], opt_descriptions, application, Console);
    try {
        po::store(po::command_line_parser(argc, argv).options(cmdline_options).style(po::command_line_style::default_style|po::command_line_style::case_insensitive).positional(pd).run(), vm);
        po::notify(vm);
    } catch (std::exception& x) {
        Console.Printf("Program options error: %s\n", BasePrint::P_ERROR, x.what());
        __SaTScanExit();
        return 1;
    }
    /* program options processing */
    if (vm.count("help")) {usage_message(argv[0], opt_descriptions, application, Console); return 0;}
    if (vm.count("version")) {Console.Printf("SaTScan %s.%s.%s %s.\n", BasePrint::P_STDOUT, VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE, VERSION_PHASE); return 0;}
    //if (!vm.count("parameter-file")) {Console.Printf("Missing input parameter-file.\n", BasePrint::P_ERROR); usage_message(argv[0], opt_descriptions, application, Console); return 1;}
    /* read parameter file */
    if (vm.count("parameter-file")) {
        if (!ParameterAccessCoordinator(Parameters).Read(vm["parameter-file"].as<std::string>().c_str(), Console))
            throw resolvable_error("\nThe parameter file contains incorrect settings that prevent SaTScan from continuing.\n"
                                   "Please review above message(s) and modify parameter settings accordingly.");
    }
    /* apply parameter overrides*/
    parameterOptions.setParameterOverrides(vm);
    if (forceCentric) Parameters.SetExecutionType(CENTRICALLY); 
    if (allOut) Parameters.RequestAllAdditionalOutputFiles();
    if (standardPvalue) Parameters.SetPValueReportingType(STANDARD_PVALUE);
    Console.SetSuppressWarnings(Parameters.GetSuppressingWarnings());
    Parameters.SetRunHistoryFilename(AppToolkit::getToolkit().GetRunHistoryFileName());
    /* validate parameters - print errors to console */
    if (!ParametersValidate(Parameters).Validate(Console))
      throw resolvable_error("\nThe parameter file contains incorrect settings that prevent SaTScan from continuing.\n"
                             "Please review above message(s) and modify parameter settings accordingly.");
    /* additional program options processing */
    if (printParameters) {ParametersPrint(Parameters).Print(stdout); return 0;}
    if (verifyParameters) {Console.Printf("Parameters confirmed.\n", BasePrint::P_STDOUT); return 0;}
    //parameterOptions.listOptions();
    //ParameterAccessCoordinator(Parameters).Write(vm["parameter-file"].as<std::string>().c_str(), Console);
    Console.Printf(AppToolkit::getToolkit().GetAcknowledgment(sMessage), BasePrint::P_STDOUT);
    //create analysis runner object and execute analysis
    AnalysisRunner(Parameters, RunTime, Console);
    //report completion
    Console.Printf("\nSaTScan completed successfully.\nThe results have been written to: \n  %s\n\n",  BasePrint::P_STDOUT, Parameters.GetOutputFileName().c_str());
    __SaTScanExit();
  } catch (resolvable_error & x) {
    Console.Printf("%s\n\nJob cancelled.", BasePrint::P_ERROR, x.what());
    __SaTScanExit();
    return 1;
  } catch (prg_exception& x) {
    Console.Printf("\n\nJob cancelled due to an unexpected program error.\n\n"
                   "Please contact technical support with the following information:\n"
                   "%s\n%s\n", BasePrint::P_ERROR, x.what(), x.trace());
    __SaTScanExit();
    return 1;
  } catch (std::bad_alloc &x) {
    Console.Printf("\nSaTScan is unable to perform analysis due to insufficient memory.\n"
                   "Please see 'Memory Requirements' in user guide for suggested solutions.\n", BasePrint::P_ERROR);
    __SaTScanExit();
    return 1;
  } catch (std::exception& x) {
    Console.Printf("\n\nJob cancelled due to an unexpected program error.\n\n"
                   "Please contact technical support with the following information:\n"
                   "%s\n%s\n", BasePrint::P_ERROR, x.what(), "Callpath not available.");
    __SaTScanExit();
    return 1;
  } catch (BaseException& x) {
    Console.Printf("\n\nJob cancelled due to an unexpected program error.\n\n"
                   "Please contact technical support with the following information:\n"
                   "%s\n%s\n", BasePrint::P_ERROR, x.what(), "Callpath not available.");
    __SaTScanExit();
    return 1;
  } catch (...) {
    Console.Printf("\n\nJob cancelled due to an unexpected program error.\n\n"
                   "Please contact technical support with the following information:\n"
                   "Unknown program error encountered.", BasePrint::P_ERROR);
    __SaTScanExit();
    return 1;
  }
  return 0;
} /* main() */


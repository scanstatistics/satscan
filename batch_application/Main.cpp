//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include <time.h>
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

/** Validates arguments of argument list. Throws UsageException if invalid.
    Returns whether options suppress execution of analysis. */
bool validateCommandLineArguments(int argc, char *argv[]) {
  if (argc < 2) throw usage_error(argv[0]);
  for (int i=2; i < argc; ++i) {
    if (!stricmp(argv[i], "-o")) {
      if (argc < i + 2) throw usage_error(argv[0]);
      ++i; //next parameter is assumed to be filename
      continue;
    }
    if (stricmp(argv[i], "-p") && stricmp(argv[i], "-c") &&
        stricmp(argv[i], "-one-cpu") && stricmp(argv[i], "-centric") && stricmp(argv[i], "-all-out"))
      throw usage_error(argv[0]);
  }
  for (int i=2; i < argc; ++i)
     if (!stricmp(argv[i], "-p") || !stricmp(argv[i], "-c")) return true;
  return false;
}

/** Returns index of 'arg' in argument list, returns zero if not found. */
int getCommandLineArgumentIndex(int argc, char *argv[], const char * arg) {
  for (int i=2; i < argc; ++i)
     if (!stricmp(argv[i], arg)) return i;
  return 0;
}

void __SaTScanInit(const char * sApplicationFullPath) {
  reserve_memory_cache();
  std::set_new_handler(prg_new_handler);
  AppToolkit::ToolKitCreate(sApplicationFullPath);
}

void __SaTScanExit() {
  release_memory_cache();
  AppToolkit::ToolKitDestroy();
}

int main(int argc, char *argv[]) {
  int                   i;
  bool                  bExecuting;
  time_t                RunTime;
  CParameters           Parameters;
  std::string           sMessage;
  PrintScreen           Console(false);

  try {
    __SaTScanInit(argv[0]);
    Console.Printf(AppToolkit::getToolkit().GetAcknowledgment(sMessage), BasePrint::P_STDOUT);
    bExecuting = !validateCommandLineArguments(argc, argv);
    time(&RunTime); //get start time
    if (!ParameterAccessCoordinator(Parameters).Read(argv[1], Console))
      throw resolvable_error("\nThe parameter file contains incorrect settings that prevent SaTScan from continuing.\n"
                             "Please review above message(s) and modify parameter settings accordingly.");
    if ((i = getCommandLineArgumentIndex(argc, argv, "-o")) != 0)
      Parameters.SetOutputFileName(argv[++i]); // overide parameter filename, if requested
    if (getCommandLineArgumentIndex(argc, argv, "-one-cpu"))
      Parameters.SetNumParallelProcessesToExecute(1); //override parameter file setting, if requested
    Console.SetSuppressWarnings(Parameters.GetSuppressingWarnings());
    Parameters.SetRunHistoryFilename(AppToolkit::getToolkit().GetRunHistoryFileName());
    //validate parameters - print errors to console
    if (!ParametersValidate(Parameters).Validate(Console))
      throw resolvable_error("\nThe parameter file contains incorrect settings that prevent SaTScan from continuing.\n"
                             "Please review above message(s) and modify parameter settings accordingly.");
    if (getCommandLineArgumentIndex(argc, argv, "-centric") && Parameters.GetPermitsCentricExecution())
      Parameters.SetExecutionType(CENTRICALLY); // overide execution type, if requested
    if (getCommandLineArgumentIndex(argc, argv, "-all-out"))
      Parameters.RequestAllAdditionalOutputFiles(); // overide selected output files, if requested
    if (getCommandLineArgumentIndex(argc, argv, "-p"))
      ParametersPrint(Parameters).Print(stdout);
    if (getCommandLineArgumentIndex(argc, argv, "-c"))
      Console.Printf("Parameters confirmed.\n", BasePrint::P_STDOUT);
    if (bExecuting) {
      //create analysis runner object and execute analysis
      AnalysisRunner(Parameters, RunTime, Console);
      //report completion
      Console.Printf("\nSaTScan completed successfully.\nThe results have been written to: \n  %s\n\n",
                     BasePrint::P_STDOUT, Parameters.GetOutputFileName().c_str());
    }
    __SaTScanExit();
  }
  catch (resolvable_error & x) {
    Console.Printf("%s\n\nJob cancelled.", BasePrint::P_ERROR, x.what());
    __SaTScanExit();
    return 1;
  }
  catch (usage_error & x) {
    Console.Printf(x.what(), BasePrint::P_ERROR);
    __SaTScanExit();
    return 1;
  }
  catch (prg_exception& x) {
    Console.Printf("\n\nJob cancelled due to an unexpected program error.\n\n"
                   "Please contact technical support with the following information:\n"
                   "%s\n%s\n", BasePrint::P_ERROR, x.what(), x.trace());
    __SaTScanExit();
    return 1;
  }
  catch (std::bad_alloc &x) {
    Console.Printf("\nSaTScan is unable to perform analysis due to insufficient memory.\n"
                   "Please see 'Memory Requirements' in user guide for suggested solutions.\n", BasePrint::P_ERROR);
    __SaTScanExit();
    return 1;
  }
  catch (std::exception& x) {
    Console.Printf("\n\nJob cancelled due to an unexpected program error.\n\n"
                   "Please contact technical support with the following information:\n"
                   "%s\n%s\n", BasePrint::P_ERROR, x.what(), "Callpath not available.");
    __SaTScanExit();
    return 1;
  }
  catch (...) {
    Console.Printf("\n\nJob cancelled due to an unexpected program error.\n\n"
                   "Please contact technical support with the following information:\n"
                   "Unknown program error encountered.", BasePrint::P_ERROR);
    __SaTScanExit();
    return 1;
  }
  return 0;
} /* main() */


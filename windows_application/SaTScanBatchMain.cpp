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
#include "DBFFile.h"
#include "AnalysisRun.h"
#include "ParameterFileAccess.h"
#include "ParametersValidate.h"
#include "SSException.h" 

int main(int argc, char *argv[]) {
  int                   i;
  time_t                RunTime;
  CParameters           Parameters;
  ZdString              sMessage;
  PrintScreen           Console(false);

  try {
    BasisInit(); //initialize basis/zero dimension libraries
    BasisSetToolkit(new SaTScanToolkit(argv[0])); //Set toolkit
    ZdGetFileTypeArray()->AddElement(&(DBFFileType::GetDefaultInstance()));
    Console.Printf(GetToolkit().GetAcknowledgment(sMessage), BasePrint::P_STDOUT);
    if (argc < 2)
      GenerateUsageException(argv[0]);
    time(&RunTime); //get start time
    if (!ParameterAccessCoordinator(Parameters).Read(argv[1], Console)) {
      sMessage << ZdString::reset << "\nThe parameter file contains incorrect settings that prevent SaTScan from continuing.\n";
      sMessage << "Please review above message(s) and modify parameter settings accordingly.";
      GenerateResolvableException(sMessage.GetCString(),"main(int,char*)");
    }
    // read options
    for (i=2; i < argc; ++i) {
       if (!stricmp(argv[i], "-o")) {
         if (argc < i + 2)
           GenerateUsageException(argv[0]);
         Parameters.SetOutputFileName(argv[++i]);
       }
       else
         GenerateUsageException(argv[0]);
    }
    Console.SetSuppressWarnings(Parameters.GetSuppressingWarnings());
    Parameters.SetRunHistoryFilename(GetToolkit().GetRunHistoryFileName());
    //validate parameters - print errors to console
    if (! ParametersValidate(Parameters).Validate(Console)) {
      sMessage << ZdString::reset << "\nThe parameter file contains incorrect settings that prevent SaTScan from continuing.\n";
      sMessage << "Please review above message(s) and modify parameter settings accordingly.";
      GenerateResolvableException(sMessage.GetCString(),"main(int,char*)");
    }
    //create analysis runner object and execute analysis
    AnalysisRunner(Parameters, RunTime, Console);

    //report completion
    Console.Printf("\nSaTScan completed successfully.\nThe results have been written to: \n  %s\n\n",
                   BasePrint::P_STDOUT, Parameters.GetOutputFileName().c_str());
    BasisExit();
  }
  catch (ResolvableException & x) {
    Console.Printf("%s\n\nJob cancelled.", BasePrint::P_ERROR, x.GetErrorMessage());
    BasisExit();
    return 1;
  }
  catch (UsageException & x) {
    Console.Printf(x.GetErrorMessage(), BasePrint::P_ERROR);
    BasisExit();
    return 1;
  }
  catch (ZdMemoryException &x) {
    Console.Printf("\nSaTScan is unable to perform analysis due to insufficient memory.\n"
                   "Please see 'Memory Requirements' in user guide for suggested solutions.\n", BasePrint::P_ERROR);
    BasisExit();
    return 1;
  }
  catch (ZdException & x) {
    Console.Printf("\n\nJob cancelled due to an unexpected program error.\n\n"
                   "Please contact technical support with the following information:\n"
                   "%s\n%s\n", BasePrint::P_ERROR, x.GetErrorMessage(), x.GetCallpath());
    BasisExit();
    return 1;
  }
  catch (...) {
    Console.Printf("\n\nJob cancelled due to an unexpected program error.\n\n"
                   "Please contact technical support with the following information:\n"
                   "Unknown program error encountered.", BasePrint::P_ERROR);
    BasisExit();
    return 1;
  }
  return 0;
} /* main() */


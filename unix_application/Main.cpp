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

int main(int argc, char *argv[]) {
  int                   i;
  time_t                RunTime;
  CParameters           Parameters;
  PrintScreen           ConsolePrint;
  ZdString              sMessage;

  try {
    BasisInit(); //initialize basis/zero dimension libraries
    BasisSetToolkit(new SaTScanToolkit(argv[0])); //Set toolkit
    ZdGetFileTypeArray()->AddElement(&(DBFFileType::GetDefaultInstance()));
    ConsolePrint.SatScanPrintf(GetToolkit().GetAcknowledgment(sMessage));
    if (argc < 2)
      GenerateUsageException(argv[0]);
    time(&RunTime); //get start time
    Parameters.Read(argv[1], ConsolePrint);
    if (Parameters.GetErrorOnRead()) {
      sMessage << ZdString::reset << "\nThe parameter file contains incorrect settings that prevent SaTScan from continuing.\n";
      sMessage << "Please review above message(s) and modify parameter settings accordingly.";
      GenerateResolvableException(sMessage.GetCString(),"main(int,char*)");
    }
    // read options
    for (i=2; i < argc; ++i) {
       if (!stricmp(argv[i], "-v")) {
         Parameters.SetOutputClusterLevelAscii(true);
         Parameters.SetOutputAreaSpecificAscii(true);
         if (Parameters.GetProbabiltyModelType() != SPACETIMEPERMUTATION)
           Parameters.SetOutputRelativeRisksAscii(true);
         Parameters.SetOutputSimLogLikeliRatiosAscii(true);
         if (Parameters.GetAnalysisType() == PROSPECTIVEPURELYTEMPORAL || Parameters.GetAnalysisType() == PROSPECTIVESPACETIME)
           //so that we can compare prospective analyses with v3.1 and prior, which didn't have below option
           Parameters.SetAdjustForEarlierAnalyses(true);
       }
       else if (!stricmp(argv[i], "-o")) {
         if (argc < i + 2)
           GenerateUsageException(argv[0]);
         Parameters.SetOutputFileName(argv[++i]);
       }
       else
         GenerateUsageException(argv[0]);
    }
    Parameters.SetRunHistoryFilename(GetToolkit().GetRunHistoryFileName());
    Parameters.SetIsLoggingHistory(GetToolkit().GetLogRunHistory());
    //validate parameters - print errors to console
    if (! Parameters.ValidateParameters(ConsolePrint)) {
      sMessage << ZdString::reset << "\nThe parameter file contains incorrect settings that prevent SaTScan from continuing.\n";
      sMessage << "Please review above message(s) and modify parameter settings accordingly.";
      GenerateResolvableException(sMessage.GetCString(),"main(int,char*)");
    }
    //create analysis runner object
    AnalysisRunner  Runner(Parameters, RunTime, ConsolePrint);
    //execute analysis 
    Runner.Execute();
    //report completion
    ConsolePrint.SatScanPrintf("\nSaTScan completed successfully.\nThe results have been written to: \n  %s\n\n",
                               Parameters.GetOutputFileName().c_str());
    BasisExit();
  }
  catch (ResolvableException & x) {
    ConsolePrint.SatScanPrintf(x.GetErrorMessage());
    ConsolePrint.SatScanPrintf("\n\nJob cancelled.");
    BasisExit();
    return 1;
  }
  catch (UsageException & x) {
    ConsolePrint.SatScanPrintf(x.GetErrorMessage());
    BasisExit();
    return 1;
  }
  catch (ZdMemoryException &x) {
    ConsolePrint.SatScanPrintWarning("\nSaTScan is unable to perform analysis due to insuffient memory.\n");
    ConsolePrint.SatScanPrintWarning("Please see 'Memory Requirements' in user guide for suggested solutions.\n");
    BasisExit();
    return 1;
  }
  catch (ZdException & x) {
    ConsolePrint.SatScanPrintf("Job cancelled due to an unexpected program error.\n");
    ConsolePrint.SatScanPrintf("Please contact technical support with the following information.\n");
    ConsolePrint.SatScanPrintf("Program Error:\n");
    ConsolePrint.SatScanPrintf(x.GetErrorMessage());
    ConsolePrint.SatScanPrintf(x.GetCallpath());
    BasisExit();
    return 1;
  }
  return 0;
} /* main() */


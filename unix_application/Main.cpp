#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------


#include <time.h>
#include "UtilityFunctions.h"
#include "Parameters.h"
#include "PurelySpatialAnalysis.h"
#include "PurelySpatialMonotoneAnalysis.h"
#include "PurelyTemporalAnalysis.h"
#include "SpaceTimeAnalysis.h"
#include "SpaceTimeIncludePurelySpatialAnalysis.h"
#include "SpaceTimeIncludePurelyTemporalAnalysis.h"
#include "SpaceTimeIncludePureAnalysis.h"
#include "SVTTAnalysis.h"
#include "PurelySpatialData.h"
#include "PurelyTemporalData.h"
#include "SpaceTimeData.h"
#include "SVTTData.h"
#include "PrintScreen.h"
#include "DBFFile.h"

int main(int argc, char *argv[]) {
  int                   i;
  time_t                RunTime;
  CSaTScanData        * pData=0;
  CAnalysis           * pAnalysis=0;
  CParameters           Parameters;
  PrintScreen           ConsolePrint;
  ZdString              sMessage;

  try {
    BasisInit(); //initialize basis/zero dimension libraries
    BasisSetToolkit(new SaTScanToolkit(argv[0])); //Set toolkit
    ZdGetFileTypeArray()->AddElement(&(DBFFileType::GetDefaultInstance()));
    ConsolePrint.SatScanPrintf(GetToolkit().GetAcknowledgment(sMessage));
    if (argc < 2)
      GenerateUsageException();
    time(&RunTime); //get start time
    Parameters.Read(argv[1], ConsolePrint);
    if (Parameters.GetErrorOnRead()) {
      sMessage << ZdString::reset << "\nThe parameter file contains incorrect settings that prevent SaTScan from continuing.\n";
      sMessage << "Please review above message(s) and modify parameter settings accordingly.";
      SSGenerateException(sMessage.GetCString(),"main(int,char*)");
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
           GenerateUsageException();
         Parameters.SetOutputFileName(argv[++i]);
       }
       else
         GenerateUsageException();
    }
    //Set run history attributes here
    Parameters.SetRunHistoryFilename(GetToolkit().GetRunHistoryFileName());
    Parameters.SetIsLoggingHistory(GetToolkit().GetLogRunHistory());
    if (! Parameters.ValidateParameters(ConsolePrint)) {
      sMessage << ZdString::reset << "\nThe parameter file contains incorrect settings that prevent SaTScan from continuing.\n";
      sMessage << "Please review above message(s) and modify parameter settings accordingly.";
      SSGenerateException(sMessage.GetCString(),"main(int,char*)");
    }

    switch (Parameters.GetAnalysisType()) {
      case PURELYSPATIAL             : pData = new CPurelySpatialData(&Parameters, &ConsolePrint); break;
      case PROSPECTIVEPURELYTEMPORAL :
      case PURELYTEMPORAL            : pData = new CPurelyTemporalData(&Parameters, &ConsolePrint); break;
      case SPACETIME                 :
      case PROSPECTIVESPACETIME      : pData = new CSpaceTimeData(&Parameters, &ConsolePrint); break;
      case SPATIALVARTEMPTREND       : pData = new CSVTTData(&Parameters, &ConsolePrint); break;
      default : ZdGenerateException("Invalid Analysis Type Encountered.", "main(int,char*)");
    };
    pData->ReadDataFromFiles();
    switch (Parameters.GetAnalysisType()) {
      case PURELYSPATIAL             : if (Parameters.GetRiskType() == STANDARDRISK)
                                         pAnalysis = new CPurelySpatialAnalysis(&Parameters, pData, &ConsolePrint);
                                       else if (Parameters.GetRiskType() == MONOTONERISK)
                                         pAnalysis = new CPSMonotoneAnalysis(&Parameters, pData, &ConsolePrint);
                                       break;
      case PURELYTEMPORAL            :
      case PROSPECTIVEPURELYTEMPORAL : pAnalysis = new CPurelyTemporalAnalysis(&Parameters, pData, &ConsolePrint);
                                       break;
      case SPACETIME                 :
      case PROSPECTIVESPACETIME      : if (Parameters.GetIncludePurelySpatialClusters() && Parameters.GetIncludePurelyTemporalClusters())
                                         pAnalysis = new C_ST_PS_PT_Analysis(&Parameters, pData, &ConsolePrint);
                                       else if (Parameters.GetIncludePurelySpatialClusters())
                                         pAnalysis = new C_ST_PS_Analysis(&Parameters, pData, &ConsolePrint);
                                       else if (Parameters.GetIncludePurelyTemporalClusters())
                                         pAnalysis = new C_ST_PT_Analysis(&Parameters, pData, &ConsolePrint);
                                       else
                                         pAnalysis = new CSpaceTimeAnalysis(&Parameters, pData, &ConsolePrint);
                                       break;
       case SPATIALVARTEMPTREND      : pAnalysis = new CSpatialVarTempTrendAnalysis(&Parameters, pData, &ConsolePrint); break;
       default : ZdGenerateException("Invalid Analysis Type Encountered.", "main(int,char*)");
    }
    if (! pAnalysis->Execute(RunTime))
      SSGenerateException("\nProblem(s) occurred that caused the analysis to stop.\n", "main(int,char*)");
    else
      ConsolePrint.SatScanPrintf("\nSaTScan completed successfully.\nThe results have been written to: \n  %s\n\n",
                                 Parameters.GetOutputFileName().c_str());

    delete pAnalysis; pAnalysis=0;
    delete pData; pData=0;
    BasisExit();
  }
  catch (SSException & x) {
    delete pAnalysis;
    delete pData;
    ConsolePrint.SatScanPrintf(x.GetErrorMessage());
    ConsolePrint.SatScanPrintf("\n\nJob cancelled.");
    BasisExit();
    return 1;
  }
  catch (UsageException & x) {
    delete pAnalysis;
    delete pData;
    ConsolePrint.SatScanPrintf(x.GetErrorMessage());
    BasisExit();
    return 1;
  }
  catch (ZdMemoryException &x) {
    delete pAnalysis;
    delete pData;
    ConsolePrint.SatScanPrintWarning("\nSaTScan is unable to perform analysis due to insuffient memory.\n");
    ConsolePrint.SatScanPrintWarning("Please see 'Memory Requirements' in user guide for suggested solutions.\n");
    BasisExit();
    return 1;
  }
  catch (ZdException & x) {
    delete pAnalysis;
    delete pData;
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


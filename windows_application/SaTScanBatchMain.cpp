#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------

#include <time.h>
#include <process.h>
#include "UtilityFunctions.h"
#include "Parameters.h"
#include "PurelySpatialAnalysis.h"
#include "PurelySpatialMonotoneAnalysis.h"
#include "PurelyTemporalAnalysis.h"
#include "SpaceTimeAnalysis.h"
#include "SpaceTimeIncludePurelySpatialAnalysis.h"
#include "SpaceTimeIncludePurelyTemporalAnalysis.h"
#include "SpaceTimeIncludePureAnalysis.h"
#include "PurelySpatialData.h"
#include "PurelyTemporalData.h"
#include "SpaceTimeData.h"
#include "PrintScreen.h"
#include "DBFFile.h"

int main(int argc, char *argv[]) {
  time_t                RunTime;
  CSaTScanData        * pData=0;
  CAnalysis           * pAnalysis=0;
  CParameters           Parameters;
  PrintScreen           ConsolePrint;
  ZdString              sMessage;

  try {
    BasisInit(); //initialize basis/zero dimension libraries
    ZdGetFileTypeArray()->AddElement( &(DBFFileType::GetDefaultInstance()) );
    ConsolePrint.SatScanPrintf("\nSaTScan %s\n\n", VERSION_NUMBER);
    Parameters.SetPrintDirection(&ConsolePrint);
    if (argc < 2)
      ZdGenerateException("No parameter file specified.\nusage: %s parameter file\n", "main(int,char*)", argv[0]);
    time(&RunTime); //get start time
    Parameters.Read(argv[1]);
    Parameters.ConvertMaxTemporalClusterSizeToType(PERCENTAGETYPE);
    if (! Parameters.ValidateParameters()) {
      sMessage << "The parameter file contains incorrect settings that prevent SaTScan from continuing.\n";
      sMessage << "Please review above message(s) and modify parameter settings accordingly.";
      ZdGenerateException(sMessage.GetCString(),"main(int,char*)");
    }

    switch (Parameters.m_nAnalysisType) {
      case PURELYSPATIAL        : pData = new CPurelySpatialData(&Parameters, &ConsolePrint); break;
      case PURELYTEMPORAL       : pData = new CPurelyTemporalData(&Parameters, &ConsolePrint); break;
      case SPACETIME            : pData = new CSpaceTimeData(&Parameters, &ConsolePrint); break;
      case PROSPECTIVESPACETIME : pData = new CSpaceTimeData(&Parameters, &ConsolePrint); break;
      default                   : ZdGenerateException("Invalid Analysis Type Encountered.", "main(int,char*)");
    };
    pData->ReadDataFromFiles();
    switch (Parameters.m_nAnalysisType) {
      case PURELYSPATIAL        : if (Parameters.m_nRiskFunctionType == STANDARDRISK)
                                    pAnalysis = new CPurelySpatialAnalysis(&Parameters, pData, &ConsolePrint);
                                  else if (Parameters.m_nRiskFunctionType == MONOTONERISK)
                                    pAnalysis = new CPSMonotoneAnalysis(&Parameters, pData, &ConsolePrint);
                                  break;
      case PURELYTEMPORAL       : pAnalysis = new CPurelyTemporalAnalysis(&Parameters, pData, &ConsolePrint);
                                  break;
      case SPACETIME            : if (Parameters.m_bIncludePurelySpatial && Parameters.m_bIncludePurelyTemporal)
                                    pAnalysis = new C_ST_PS_PT_Analysis(&Parameters, pData, &ConsolePrint);
                                  else if (Parameters.m_bIncludePurelySpatial)
                                    pAnalysis = new C_ST_PS_Analysis(&Parameters, pData, &ConsolePrint);
                                  else if (Parameters.m_bIncludePurelyTemporal)
                                    pAnalysis = new C_ST_PT_Analysis(&Parameters, pData, &ConsolePrint);
                                  else
                                    pAnalysis = new CSpaceTimeAnalysis(&Parameters, pData, &ConsolePrint);
                                  break;
      case PROSPECTIVESPACETIME : if (Parameters.m_bIncludePurelySpatial && Parameters.m_bIncludePurelyTemporal)
                                    pAnalysis = new C_ST_PS_PT_Analysis(&Parameters, pData, &ConsolePrint);
                                  else if (Parameters.m_bIncludePurelySpatial)
                                    pAnalysis = new C_ST_PS_Analysis(&Parameters, pData, &ConsolePrint);
                                  else if (Parameters.m_bIncludePurelyTemporal)
                                    pAnalysis = new C_ST_PT_Analysis(&Parameters, pData, &ConsolePrint);
                                  else
                                    pAnalysis = new CSpaceTimeAnalysis(&Parameters, pData, &ConsolePrint);
                                  break;
       default                  : ZdGenerateException("Invalid Analysis Type Encountered.", "main(int,char*)");
    }
    if (! pAnalysis->Execute(RunTime)) {
      sMessage << "\nProblem(s) occurred that caused the analysis to stop.\n";
      ZdGenerateException(sMessage.GetCString(), "main(int,char*)");
    }
    else {
      sMessage << "\nSaTScan completed successfully.\nThe results have been written to: \n  %s\n\n";
      ConsolePrint.SatScanPrintf(sMessage.GetCString(), Parameters.GetOutputFileName().c_str());
    }

    delete pAnalysis; pAnalysis=0;
    delete pData; pData=0;
    BasisExit();
  }
  catch (ZdException & x) {
    delete pAnalysis;
    delete pData;
    ConsolePrint.SatScanPrintf(x.GetErrorMessage());
    ConsolePrint.SatScanPrintf("\n\nJob cancelled.");
    BasisExit();
    exit(1);
  }
  return 1;
} /* main() */


// satscan.cpp

#pragma hdrstop

//---------------------------------------------------------------------------

#pragma argsused


#include <time.h>
#include <process.h>
#include "error.h"
#include "display.h"
#include "param.h"
#include "PSanalysis.h"
#include "PSManalysis.h"
#include "PTanalysis.h"
#include "STanalysis.h"
#include "ST+PSanalysis.h"
#include "ST+PTanalysis.h"
#include "ST+PS+PTanalysis.h"
#include "PSdata.h"
#include "PTdata.h"
#include "STdata.h"

//#define PROFILE_RUN 0

int main(int argc, char *argv[])
{
  char szOutputFilename[400];

  time_t RunTime;
  time(&RunTime);         // Pass to analysis to include in report

  DisplayVersion(stdout, 0);
  fprintf(stdout, "\n");

/*  if (argc != 2)
  {
    FatalError("  Error: Invalid Number of Arguments on Command Line.\n");
  }
  */

//  CParameters* pParameters = new CParameters(argv[1]);
  CParameters* pParameters = new CParameters(true);
//  if (!(pParameters->SetParameters(argv[1])))               // DTG - UNCOMMENT LATER
//    FatalError("");                                         // DTG

 if (!(pParameters->SetParameters("c:\\SatScan\\SatScan V.2.1.3\\SatScan\\stsparam.prm")))
    FatalError("");

  CSaTScanData* pData;

  switch (pParameters->m_nAnalysisType)
  {
    case PURELYSPATIAL  : pData = new CPurelySpatialData(pParameters);  break;
    case PURELYTEMPORAL : pData = new CPurelyTemporalData(pParameters); break;
    case SPACETIME      : pData = new CSpaceTimeData(pParameters);      break;
  };

  pData->ReadDataFromFiles();

  CAnalysis* pAnalysis;

  switch (pParameters->m_nAnalysisType)
  {
    case PURELYSPATIAL  : if (pParameters->m_nRiskFunctionType == STANDARDRISK)
                            pAnalysis = new CPurelySpatialAnalysis(pParameters, pData);
                          else if (pParameters->m_nRiskFunctionType == MONOTONERISK)
                            pAnalysis = new CPSMonotoneAnalysis(pParameters, pData);
                          break;
    case PURELYTEMPORAL : pAnalysis = new CPurelyTemporalAnalysis(pParameters, pData);
                          break;
    case SPACETIME      : if (pParameters->m_bIncludePurelySpatial &&
                              pParameters->m_bIncludePurelyTemporal)
                            pAnalysis = new C_ST_PS_PT_Analysis(pParameters, pData);
                          else if (pParameters->m_bIncludePurelySpatial)
                            pAnalysis = new C_ST_PS_Analysis(pParameters, pData);
                          else if (pParameters->m_bIncludePurelyTemporal)
                            pAnalysis = new C_ST_PT_Analysis(pParameters, pData);
                          else
                            pAnalysis = new CSpaceTimeAnalysis(pParameters, pData);
                          break;
  };

  if (!pAnalysis->Execute(RunTime))
    FatalError(0);
  else
  {
    printf("\nSaTScan completed successfully.\n");
    printf("The results have been written to: \n");
    printf("  %s\n\n",pParameters->m_szOutputFilename);

//  #if !PROFILE_RUN
//  HoldForEnter();
//  #endif

    strcpy(szOutputFilename, pParameters->m_szOutputFilename);

    delete pAnalysis;
    delete pData;
    delete pParameters;

    execlp("Notepad.exe", "Notepad.exe", szOutputFilename, NULL);

    return 0;
  }
  return 1;
} /* main() */


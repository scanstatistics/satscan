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

//#define PROFILE_RUN 0

int main(int argc, char *argv[])
{
   char szOutputFilename[400];
   time_t RunTime;
   char c;
   CSaTScanData* pData        = 0;
   CAnalysis*    pAnalysis    = 0;
   CParameters*  pParams      = 0;
   PrintScreen  *pPrintScreen = 0;

   try
      {
      BasisInit();
      // register zero dimesion libraries' dbf
      ZdGetFileTypeArray()->AddElement( &(DBFFileType::GetDefaultInstance()) );
      time(&RunTime); // Pass to analysis to include in report

      DisplayVersion(stdout, 0);
      fprintf(stdout, "\n");

      pParams = new CParameters(true);
      pPrintScreen = new PrintScreen();
      pParams->SetPrintDirection(pPrintScreen);
      
      if (argc != 2)
         SSGenerateException("  Error: Invalid Number of Arguments on Command Line.\n","main.cpp");

      //pParameters = new CParameters(argv[1]);

      //read and set parameters file - don't validate parameters yet
      if (! pParams->SetParameters(argv[1], false))
         SSGenerateException("Invalid parameter file.","SatScanBatchMain.cpp");
      //convert maximum temoral clusters size to percentage
      pParams->ConvertMaxTemporalClusterSizeToType(PERCENTAGETYPE);
      // now validate parameters
      if (! pParams->ValidateParameters())
         SSGenerateException("Invalid parameter file.","SatScanBatchMain.cpp");

      switch (pParams->m_nAnalysisType)
         {
         case PURELYSPATIAL  : pData = new CPurelySpatialData(pParams, pPrintScreen);  break;
         case PURELYTEMPORAL : pData = new CPurelyTemporalData(pParams, pPrintScreen); break;
         case SPACETIME      : pData = new CSpaceTimeData(pParams, pPrintScreen);      break;
         case PROSPECTIVESPACETIME : pData = new CSpaceTimeData(pParams, pPrintScreen);break;
         default:
             SSGenerateException("Invalid Analysis Type Encountered.", "main.cpp");
         };

      pData->ReadDataFromFiles();

      switch (pParams->m_nAnalysisType)
         {
         case PURELYSPATIAL  : if (pParams->m_nRiskFunctionType == STANDARDRISK)
                                  pAnalysis = new CPurelySpatialAnalysis(pParams, pData, pPrintScreen);
                               else if (pParams->m_nRiskFunctionType == MONOTONERISK)
                                  pAnalysis = new CPSMonotoneAnalysis(pParams, pData, pPrintScreen);
                               break;
         case PURELYTEMPORAL : pAnalysis = new CPurelyTemporalAnalysis(pParams, pData, pPrintScreen);
                               break;
         case SPACETIME      : if (pParams->m_bIncludePurelySpatial &&
                                   pParams->m_bIncludePurelyTemporal)
                                  pAnalysis = new C_ST_PS_PT_Analysis(pParams, pData, pPrintScreen);
                               else if (pParams->m_bIncludePurelySpatial)
                                  pAnalysis = new C_ST_PS_Analysis(pParams, pData, pPrintScreen);
                               else if (pParams->m_bIncludePurelyTemporal)
                                  pAnalysis = new C_ST_PT_Analysis(pParams, pData, pPrintScreen);
                               else
                                  pAnalysis = new CSpaceTimeAnalysis(pParams, pData, pPrintScreen);
                                break;
         case PROSPECTIVESPACETIME      : if (pParams->m_bIncludePurelySpatial &&
                                   pParams->m_bIncludePurelyTemporal)
                                  pAnalysis = new C_ST_PS_PT_Analysis(pParams, pData, pPrintScreen);
                               else if (pParams->m_bIncludePurelySpatial)
                                  pAnalysis = new C_ST_PS_Analysis(pParams, pData, pPrintScreen);
                               else if (pParams->m_bIncludePurelyTemporal)
                                  pAnalysis = new C_ST_PT_Analysis(pParams, pData, pPrintScreen);
                               else
                                  pAnalysis = new CSpaceTimeAnalysis(pParams, pData, pPrintScreen);
                                break;
         }

      if (!pAnalysis->Execute(RunTime))
         SSGenerateException("An Error has occured within the Calculation Module.", "main.cpp");
      else
         {
         pPrintScreen->SatScanPrintf("\nSaTScan completed successfully.\n");
         pPrintScreen->SatScanPrintf("The results have been written to: \n");
         pPrintScreen->SatScanPrintf("  %s\n\n",pParams->GetOutputFileName().c_str());
         //requested removal by Jessica Hartman on Aug. 9,2002
         //pPrintScreen->SatScanPrintf("\nPress <Enter> to exit the SaTScan program.");
         //getc(stdin);
//  #if !PROFILE_RUN
//  HoldForEnter();
//  #endif

         delete pAnalysis;    pAnalysis = 0;
         delete pData;        pData = 0;
         delete pParams;      pParams=0;
         delete pPrintScreen; pPrintScreen=0;
         }
         BasisExit();
      }
   catch (SSException & x)
      {
      delete pAnalysis;
      delete pData;
      delete pParams;
      pPrintScreen->SatScanPrintf("\nAn error has occured!!\n");
      pPrintScreen->SatScanPrintf(x.GetCallpath());
      pPrintScreen->SatScanPrintf(" \n ");
      pPrintScreen->SatScanPrintf(x.GetErrorMessage());
      pPrintScreen->SatScanPrintf("\nProgram terminated.");
      //then print this....
      //pPrintScreen->SatScanPrintf("\nPress <Enter> to exit the SaTScan program.");
      //getc(stdin);
      delete pPrintScreen;
      BasisExit();
      exit(1);
      }
  return 1;
} /* main() */


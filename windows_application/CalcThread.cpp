//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "CalcThread.h"
#include "xmfrmAnalysisRun.h"

#pragma package(smart_init)
//---------------------------------------------------------------------------
//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall EditThread::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------
__fastcall CalcThread::CalcThread(bool CreateSuspended, const CParameters& session, char *pTitle, TfrmAnalysisRun *pProgress)
        : TThread(CreateSuspended)
{
   Init();
   gpParams = new CParameters(session);
   gpPrintWindow = new PrintWindow(pProgress);
   gpParams->SetPrintDirection(gpPrintWindow);
   pProgress->reAnalysisBox->SetFocus();
   gsThreadTitle = pTitle;
   gpFormStatus =  pProgress;
   Priority = tpNormal;  //tpHighest
   FreeOnTerminate = true;

   gpFormStatus->Caption = pTitle;
}
__fastcall CalcThread::~CalcThread()
{
   Free();
}
//------------------------------------------------------------------------------
void CalcThread::Init()
{
   gpParams   = 0;
   gpData     = 0;
   gpAnalysis = 0;
}
//------------------------------------------------------------------------------
void CalcThread::Free()
{
   delete gpData;
   delete gpAnalysis;
   delete gpPrintWindow;

   //deletion of gpData must happen before this
   //or more importantly, gpParams deletion must happen AFTER gpData
   delete gpParams;
}
//------------------------------------------------------------------------------
// Main Thread execution function.
//
void __fastcall CalcThread::Execute()
{
   try
      {
      char szOutputFilename[400];

      time_t RunTime;
      time(&RunTime);         // Pass to analysis to include in report

      DisplayVersion(stdout, 0);
      fprintf(stdout, "\n");

      //Do not need to place this check in the Dos app.  It is automatically
      //run by the Dos app when the parameter file is read in.
      gpParams->SetDisplayParameters(true);
      if (!gpParams->ValidateParameters())
         SSGenerateException("Invalid parameter Encountered.", "CalcThread");

      switch (gpParams->m_nAnalysisType)
         {
         case PURELYSPATIAL  : gpData = new CPurelySpatialData(gpParams, gpPrintWindow);  break;
         case PURELYTEMPORAL : gpData = new CPurelyTemporalData(gpParams, gpPrintWindow); break;
         case SPACETIME      : gpData = new CSpaceTimeData(gpParams, gpPrintWindow);      break;
         case PROSPECTIVESPACETIME : gpData = new CSpaceTimeData(gpParams, gpPrintWindow);break;
         default:
             SSGenerateException("Invalid Analysis Type Encountered.", "CalcThread");
         };
      if (! gpFormStatus->IsJobCanceled())
         gpData->ReadDataFromFiles();

      switch (gpParams->m_nAnalysisType)
      {
         case PURELYSPATIAL  : if (gpParams->m_nRiskFunctionType == STANDARDRISK)
                                  gpAnalysis = new CPurelySpatialAnalysis(gpParams, gpData, gpPrintWindow);
                               else if (gpParams->m_nRiskFunctionType == MONOTONERISK)
                                  gpAnalysis = new CPSMonotoneAnalysis(gpParams, gpData, gpPrintWindow);
                               break;
         case PURELYTEMPORAL : gpAnalysis = new CPurelyTemporalAnalysis(gpParams, gpData, gpPrintWindow);
                               break;
         case SPACETIME      : if (gpParams->m_bIncludePurelySpatial &&
                                   gpParams->m_bIncludePurelyTemporal)
                                  gpAnalysis = new C_ST_PS_PT_Analysis(gpParams, gpData, gpPrintWindow);
                               else if (gpParams->m_bIncludePurelySpatial)
                                  gpAnalysis = new C_ST_PS_Analysis(gpParams, gpData, gpPrintWindow);
                               else if (gpParams->m_bIncludePurelyTemporal)
                                  gpAnalysis = new C_ST_PT_Analysis(gpParams, gpData, gpPrintWindow);
                               else
                                  gpAnalysis = new CSpaceTimeAnalysis(gpParams, gpData, gpPrintWindow);
                                break;
         case PROSPECTIVESPACETIME      : if (gpParams->m_bIncludePurelySpatial &&
                                   gpParams->m_bIncludePurelyTemporal)
                                  gpAnalysis = new C_ST_PS_PT_Analysis(gpParams, gpData, gpPrintWindow);
                               else if (gpParams->m_bIncludePurelySpatial)
                                  gpAnalysis = new C_ST_PS_Analysis(gpParams, gpData, gpPrintWindow);
                               else if (gpParams->m_bIncludePurelyTemporal)
                                  gpAnalysis = new C_ST_PT_Analysis(gpParams, gpData, gpPrintWindow);
                               else
                                  gpAnalysis = new CSpaceTimeAnalysis(gpParams, gpData, gpPrintWindow);
                                break;
      }    

      if (!gpAnalysis->Execute(RunTime))
         {
         if (! gpFormStatus->IsJobCanceled())
            gpPrintWindow->SatScanPrintf("An Error has occured within the Calculation Module.");
            //FatalError("An Error has occured within the Calculation Module", gpPrintWindow);
         }
      else
      {
         gpPrintWindow->SatScanPrintf("\nSaTScan completed successfully.\n");
         gpPrintWindow->SatScanPrintf("The results have been written to: \n");
         gpPrintWindow->SatScanPrintf("  %s\n\n",gpParams->m_szOutputFilename);

         strcpy(szOutputFilename, gpParams->m_szOutputFilename);

         if (gpFormStatus)
            gpFormStatus->LoadFromFile(szOutputFilename);
      }
     
      if (! gpFormStatus->IsJobCanceled())
         {
         //Synchronize((TThreadMethod)PushTheButton)
         gpFormStatus->btnCancel->Caption = "Close";
         gpFormStatus->btnPrint->Enabled = true;
         }
      if (gpFormStatus->reWarningsBox->Lines->Count == 0)
         {
         gpFormStatus->SetPrintWarnings(false);
         gpPrintWindow->SatScanPrintWarning("No warnings or errors encountered.");
         }
      else
         gpFormStatus->SetPrintWarnings(true);

      gpFormStatus->btnPrint->Enabled = true;    
      }
   catch (SSException & x)
      {
      gpFormStatus->CancelJob();
      x.AddCallpath("Execute()", "CalcThread");
      gpPrintWindow->SatScanPrintWarning(x.GetCallpath());
      gpPrintWindow->SatScanPrintWarning(x.GetErrorMessage());
      gpPrintWindow->SatScanPrintf("\nProgram terminated.");
      gpPrintWindow->SatScanPrintWarning("\nEnd of Warnings and Errors");
      gpFormStatus->btnCancel->Caption = "Close";
      gpFormStatus->btnPrint->Enabled = true;
      gpFormStatus->btnEMail->Enabled = true;
      }
}


//---------------------------------------------------------------------------
#include "stsSaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
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

/** Constructor */
__fastcall CalcThread::CalcThread(bool CreateSuspended, const CParameters& session, char *pTitle, TfrmAnalysisRun *pProgress)
                      :TThread(CreateSuspended) {
  try {
    Init();
    Setup(session, pTitle, pProgress);
  }
  catch (ZdException & x) {
    x.AddCallpath("constructor()", "CalcThread");
    throw;
  }
}

/** Destructor */
__fastcall CalcThread::~CalcThread() {
  try {
    Free();
  }
  catch (...){}  
}

/** Synchronizes text output to run analysis form in the main application thread. */
void CalcThread::AddLineToProgress(char * sText) {
  if (sText ) {
    gsPrintString = sText;
    Synchronize((TThreadMethod)&PrintLineToProgress);
  }
}

/** Synchronizes text output to run analysis form in the main application thread. */
void CalcThread::AddWarningToProgress(char * sText) {
  if (sText ) {
    gsPrintString = sText;
    Synchronize((TThreadMethod)&PrintWarningLineToProgress);
  }
}

/** Synchronizes job cancelation with run analysis form in the main application thread.*/
void CalcThread::CancellJob() {
  Synchronize((TThreadMethod)&SetJobCancelled);
}

/** Synchronizes enabling of run analysis form's email button in the main application thread. */
void __fastcall CalcThread::EnableProgressEmailButton(void) {
  gpFormStatus->btnEMail->Enabled = true;
}

/** Synchronizes enabling of run analysis form's print button in the main application thread. */
void __fastcall CalcThread::EnableProgressPrintButton(void) {
  gpFormStatus->btnPrint->Enabled = true;
}

/** Main Thread execution function. */
void __fastcall CalcThread::Execute() {
  try {
    time_t      RunTime;

    time(&RunTime);         // Pass to analysis to include in report

    //Do not need to place this check in the Dos app.  It is automatically
    //run by the Dos app when the parameter file is read in.
    gpParams->SetDisplayParameters(true);
    if (!gpParams->ValidateParameters())
       SSGenerateException("\nInvalid parameter(s) encountered. Job cancelled.", "Execute()");

    switch (gpParams->m_nAnalysisType) {
       case PURELYSPATIAL        : gpData = new CPurelySpatialData(gpParams, gpPrintWindow);  break;
       case PURELYTEMPORAL       : gpData = new CPurelyTemporalData(gpParams, gpPrintWindow); break;
       case SPACETIME            : gpData = new CSpaceTimeData(gpParams, gpPrintWindow);      break;
       case PROSPECTIVESPACETIME : gpData = new CSpaceTimeData(gpParams, gpPrintWindow);break;
       default                   : SSGenerateException("Invalid Analysis Type Encountered.", "Execute()");
    };

    if (! IsCancelled()) {
      gpData->ReadDataFromFiles();
      switch (gpParams->m_nAnalysisType) {
         case PURELYSPATIAL        : if (gpParams->m_nRiskFunctionType == STANDARDRISK)
                                       gpAnalysis = new CPurelySpatialAnalysis(gpParams, gpData, gpPrintWindow);
                                     else if (gpParams->m_nRiskFunctionType == MONOTONERISK)
                                       gpAnalysis = new CPSMonotoneAnalysis(gpParams, gpData, gpPrintWindow);
                                     break;
         case PURELYTEMPORAL       : gpAnalysis = new CPurelyTemporalAnalysis(gpParams, gpData, gpPrintWindow);
                                     break;
         case SPACETIME            : if (gpParams->m_bIncludePurelySpatial && gpParams->m_bIncludePurelyTemporal)
                                       gpAnalysis = new C_ST_PS_PT_Analysis(gpParams, gpData, gpPrintWindow);
                                     else if (gpParams->m_bIncludePurelySpatial)
                                       gpAnalysis = new C_ST_PS_Analysis(gpParams, gpData, gpPrintWindow);
                                     else if (gpParams->m_bIncludePurelyTemporal)
                                       gpAnalysis = new C_ST_PT_Analysis(gpParams, gpData, gpPrintWindow);
                                     else
                                       gpAnalysis = new CSpaceTimeAnalysis(gpParams, gpData, gpPrintWindow);
                                     break;
         case PROSPECTIVESPACETIME : if (gpParams->m_bIncludePurelySpatial && gpParams->m_bIncludePurelyTemporal)
                                       gpAnalysis = new C_ST_PS_PT_Analysis(gpParams, gpData, gpPrintWindow);
                                     else if (gpParams->m_bIncludePurelySpatial)
                                       gpAnalysis = new C_ST_PS_Analysis(gpParams, gpData, gpPrintWindow);
                                     else if (gpParams->m_bIncludePurelyTemporal)
                                       gpAnalysis = new C_ST_PT_Analysis(gpParams, gpData, gpPrintWindow);
                                     else
                                       gpAnalysis = new CSpaceTimeAnalysis(gpParams, gpData, gpPrintWindow);
                                     break;
      };

      if (! gpAnalysis->Execute(RunTime)) {
        if (! IsCancelled())
          gpPrintWindow->SatScanPrintf("An Error has occured within the Calculation Module.");
      }
      else {
        gpPrintWindow->SatScanPrintf("\nSaTScan completed successfully.\n");
        gpPrintWindow->SatScanPrintf("The results have been written to: \n");
        gpPrintWindow->SatScanPrintf("  %s\n\n",gpParams->m_szOutputFilename);
        Synchronize((TThreadMethod)&LoadResultsFromFile);
      }
    }

    Synchronize((TThreadMethod)&ResetProgressCloseButton);
    Synchronize((TThreadMethod)&EnableProgressPrintButton);
    Synchronize((TThreadMethod)&SetProgressWarnings);
    if (IsCancelled())
      // Process is done interacting with run analysis window. So if analysis
      // was cancelled, we can now safely tell run analysis that we are finished
      // with it(i.e. we won't we accessing gpFormStatus anymore).
      Synchronize((TThreadMethod)&ProcessAcknowledgesCancellation);
  }
  catch (ZdException & x) {
    x.AddCallpath("Execute()", "CalcThread");
//    gpPrintWindow->SatScanPrintWarning(x.GetCallpath());
    gpPrintWindow->SatScanPrintWarning(x.GetErrorMessage());
    gpPrintWindow->SatScanPrintWarning("\nEnd of Warnings and Errors");
    Synchronize((TThreadMethod)&ResetProgressCloseButton);
    Synchronize((TThreadMethod)&EnableProgressPrintButton);
    Synchronize((TThreadMethod)&EnableProgressEmailButton);
    CancellJob();
  }
  Synchronize((TThreadMethod)&ProcessSignalsCompletion);
}

/** Cleanup. */
void CalcThread::Free() {
   delete gpData;
   delete gpAnalysis;
   delete gpPrintWindow;

   //deletion of gpData must happen before this
   //or more importantly, gpParams deletion must happen AFTER gpData
   delete gpParams;
}

/** Internal initialization function. */
void CalcThread::Init(){
  gpParams   = 0;
  gpData     = 0;
  gpAnalysis = 0;
  gpFormStatus = 0;
  gpPrintWindow = 0;
  gbJobCanceled = false;
  gsPrintString = 0;
}

/** Synchronizes cancel check in run analysis form in the main application thread. */
bool CalcThread::IsCancelled() {
  Synchronize((TThreadMethod)&GetIsCanceledFromProgress);
  return gbJobCanceled;
}

/** Checks whether job has been cancelled through Run Analysis interface. */
void __fastcall CalcThread::GetIsCanceledFromProgress(void) {
  gbJobCanceled = gpFormStatus->IsJobCanceled();
}

/** Loads analysis run memo field with results of analysis. */
void __fastcall CalcThread::LoadResultsFromFile() {
  try {
    gpFormStatus->LoadFromFile(gpParams->m_szOutputFilename);
  }
  catch (...){/* Put Synchronized exception catch here later - for now just eat errors. */ }
}

/** Adds print string to Run Analysis interface. Prevents printing string
    if job already cancelled. This prevents any messages from being printed
    will in some loop/function that hasn't checked for cancellation. */
void __fastcall CalcThread::PrintLineToProgress(void) {
  try {
    if (! gpFormStatus->IsJobCanceled())
      gpFormStatus->AddLine(gsPrintString);
  }
  catch (...){/* Put Synchronized exception catch here later - for now just eat errors. */ }
}

/** Adds print string to Run Analysis interface. Prevents printing string
    if job already cancelled. This prevents any messages from being printed
    will in some loop/function that hasn't checked for cancellation. */
void __fastcall CalcThread::PrintWarningLineToProgress(void) {
  try {
    if (! gpFormStatus->IsJobCanceled())
      gpFormStatus->AddWarningLine(gsPrintString);
  }
  catch (...){/* Put Synchronized exception catch here later - for now just eat errors. */ }
}

/** Indicates that process have acknowledged cancellation and is in proper state 
    for cancelling by posting message to run analysis window. */
void __fastcall CalcThread::ProcessAcknowledgesCancellation(void) {
  try {
    gpFormStatus->AddLine("Job cancelled by user.");
  }
  catch (...){/* Put Synchronized exception catch here later - for now just eat errors. */ }
}

/** Promise to run analysis window that it is free to close.
    (i.e. gpFormStatus will never be dereferenced again by this class) */
void __fastcall CalcThread::ProcessSignalsCompletion(void) {
  try {
    gpFormStatus->SetCanClose(true);
  }
  catch (...){/* Put Synchronized exception catch here later - for now just eat errors. */ }
}

/** Resets run analysis close button caption. */
void __fastcall CalcThread::ResetProgressCloseButton(void) {
  gpFormStatus->btnCancel->Caption = "Close";
}

/** Sets job cancelled in run analysis window. */
void __fastcall CalcThread::SetJobCancelled(void) {
  try {
    gpFormStatus->CancelJob();
  }
  catch (...){/* Put Synchronized exception catch here later - for now just eat errors. */ }
}

/** Sets warnings in run analysis window. */
void __fastcall CalcThread::SetProgressWarnings(void) {
  try {
    if (gpFormStatus->rteWarningsBox->Lines->Count == 0) {
      gpFormStatus->SetPrintWarnings(false);
      gpFormStatus->AddWarningLine("No warnings or errors encountered.");
    }
    else
     gpFormStatus->SetPrintWarnings(true);
  }
  catch (...){/* Put Synchronized exception catch here later - for now just eat errors. */ }
}

/** Internal setup function. */
void CalcThread::Setup(const CParameters& session, char *pTitle, TfrmAnalysisRun *pProgress) {
  try {
    if (! pTitle || !pProgress )
      SSGenerateException("Null pointer", "Setup()");

    gpParams = new CParameters(session);
    gpPrintWindow = new PrintWindow(*this);
    gpParams->SetPrintDirection(gpPrintWindow);
    gpParams->ConvertMaxTemporalClusterSizeToType(PERCENTAGETYPE);
    gsThreadTitle = pTitle;
    gpFormStatus =  pProgress;
    Priority = tpNormal;  //tpHighest
    FreeOnTerminate = true;
    gsThreadTitle = pTitle;
    SetupProgress();
  }
  catch (ZdException & x) {
    x.AddCallpath("Setup()", "CalcThread");
    throw;
  }
}  

/** Setup for run analysis window to be called in Synchronized call. */
void __fastcall CalcThread::SetupProgress(void) {
  try {
    gpFormStatus->Caption = gsThreadTitle;
    gpFormStatus->rteAnalysisBox->SetFocus();
  }
  catch (...){/* Put Synchronized exception catch here later - for now just eat errors. */ }
}

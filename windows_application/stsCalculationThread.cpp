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
__fastcall CalcThread::CalcThread(TfrmAnalysisRun & Progress, const CParameters & Parameters)
                      :TThread(true), gFormStatus(Progress) {
  try {
    Init();
    Setup(Parameters);
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
  gFormStatus.btnEMail->Enabled = true;
}

/** Synchronizes enabling of run analysis form's print button in the main application thread. */
void __fastcall CalcThread::EnableProgressPrintButton(void) {
  gFormStatus.btnPrint->Enabled = true;
}

/** Main Thread execution function. */
void __fastcall CalcThread::Execute() {
  time_t        RunTime;
  ZdString      Acknowledgment;

  try {
    gpPrintWindow->SatScanPrintf(GetToolkit().GetAcknowledgment(Acknowledgment));

    time(&RunTime);         // Pass to analysis to include in report
    if (!gpParams->ValidateParameters(*gpPrintWindow))
       SSGenerateException("\nInvalid parameter(s) encountered. Job cancelled.", "Execute()");

    switch (gpParameters->GetAnalysisType()) {
       case PURELYSPATIAL             : gpData = new CPurelySpatialData(gpParameters, gpPrintWindow);  break;
       case PURELYTEMPORAL            :
       case PROSPECTIVEPURELYTEMPORAL : gpData = new CPurelyTemporalData(gpParameters, gpPrintWindow); break;
       case SPACETIME                 :
       case PROSPECTIVESPACETIME      : gpData = new CSpaceTimeData(gpParameters, gpPrintWindow);break;
       case SPATIALVARTEMPTREND       : gpData = new CSVTTData(gpParameters, gpPrintWindow); break;
       default : ZdGenerateException("Invalid Analysis Type '%d'.", "Execute()", gpParameters->GetAnalysisType());
    };

    if (! IsCancelled()) {
      gpData->ReadDataFromFiles();
      switch (gpParameters->GetAnalysisType()) {
         case PURELYSPATIAL             : if (gpParameters->GetRiskType() == STANDARDRISK)
                                            gpAnalysis = new CPurelySpatialAnalysis(gpParameters, gpData, gpPrintWindow);
                                          else if (gpParameters->GetRiskType() == MONOTONERISK)
                                            gpAnalysis = new CPSMonotoneAnalysis(gpParameters, gpData, gpPrintWindow);
                                          break;
         case PURELYTEMPORAL            :
         case PROSPECTIVEPURELYTEMPORAL : gpAnalysis = new CPurelyTemporalAnalysis(gpParameters, gpData, gpPrintWindow);
                                          break;
         case SPACETIME                 :
         case PROSPECTIVESPACETIME      : if (gpParameters->GetIncludePurelySpatialClusters() && gpParameters->GetIncludePurelyTemporalClusters())
                                            gpAnalysis = new C_ST_PS_PT_Analysis(gpParameters, gpData, gpPrintWindow);
                                          else if (gpParameters->GetIncludePurelySpatialClusters())
                                            gpAnalysis = new C_ST_PS_Analysis(gpParameters, gpData, gpPrintWindow);
                                          else if (gpParameters->GetIncludePurelyTemporalClusters())
                                            gpAnalysis = new C_ST_PT_Analysis(gpParameters, gpData, gpPrintWindow);
                                          else
                                            gpAnalysis = new CSpaceTimeAnalysis(gpParameters, gpData, gpPrintWindow);
                                          break;
       case SPATIALVARTEMPTREND         : gpAnalysis = new CSpatialVarTempTrendAnalysis(gpParameters, gpData, gpPrintWindow); break;
      };

      if (! gpAnalysis->Execute(RunTime)) {
        if (! IsCancelled())
          gpPrintWindow->SatScanPrintf("An Error has occured within the Calculation Module.");
      }
      else {
        gpPrintWindow->SatScanPrintf("\nSaTScan completed successfully.\n");
        gpPrintWindow->SatScanPrintf("The results have been written to: \n");
        gpPrintWindow->SatScanPrintf("  %s\n\n",gpParameters->GetOutputFileName().c_str());
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
  catch (SSException & x) {
    //handle exceptions that occured from user or data errors
    x.AddCallpath("Execute()", "CalcThread");
    gpPrintWindow->SatScanPrintWarning(x.GetErrorMessage());
    gpPrintWindow->SatScanPrintWarning("\nEnd of Warnings and Errors");
    Synchronize((TThreadMethod)&ResetProgressCloseButton);
    Synchronize((TThreadMethod)&EnableProgressPrintButton);
    Synchronize((TThreadMethod)&EnableProgressEmailButton);
    CancellJob();
  }
  catch (ZdException & x) {
    //handle exceptions that occured from unexcepted program error
    //we will want to include more information, like the callpath, in the future
    x.AddCallpath("Execute()", "CalcThread");
    gpPrintWindow->SatScanPrintWarning("\nProgram Error:\n");
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
   //or more importantly, gpParameters deletion must happen AFTER gpData
   delete gpParameters;
}

/** Internal initialization function. */
void CalcThread::Init(){
  gpParameters   = 0;
  gpData     = 0;
  gpAnalysis = 0;
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
  gbJobCanceled = gFormStatus.IsJobCanceled();
}

/** Loads analysis run memo field with results of analysis. */
void __fastcall CalcThread::LoadResultsFromFile() {
  try {
    gFormStatus.LoadFromFile(const_cast<char*>(gpParameters->GetOutputFileName().c_str()));
  }
  catch (...){/* Put Synchronized exception catch here later - for now just eat errors. */ }
}

/** Adds print string to Run Analysis interface. Prevents printing string
    if job already cancelled. This prevents any messages from being printed
    will in some loop/function that hasn't checked for cancellation. */
void __fastcall CalcThread::PrintLineToProgress(void) {
  try {
    if (! gFormStatus.IsJobCanceled())
      gFormStatus.AddLine(gsPrintString);
  }
  catch (...){/* Put Synchronized exception catch here later - for now just eat errors. */ }
}

/** Adds print string to Run Analysis interface. Prevents printing string
    if job already cancelled. This prevents any messages from being printed
    will in some loop/function that hasn't checked for cancellation. */
void __fastcall CalcThread::PrintWarningLineToProgress(void) {
  try {
    if (! gFormStatus.IsJobCanceled())
      gFormStatus.AddWarningLine(gsPrintString);
  }
  catch (...){/* Put Synchronized exception catch here later - for now just eat errors. */ }
}

/** Indicates that process have acknowledged cancellation and is in proper state 
    for cancelling by posting message to run analysis window. */
void __fastcall CalcThread::ProcessAcknowledgesCancellation(void) {
  try {
    gFormStatus.AddLine("Job cancelled by user.");
  }
  catch (...){/* Put Synchronized exception catch here later - for now just eat errors. */ }
}

/** Promise to run analysis window that it is free to close.
    (i.e. gpFormStatus will never be dereferenced again by this class) */
void __fastcall CalcThread::ProcessSignalsCompletion(void) {
  try {
    gFormStatus.SetCanClose(true);
  }
  catch (...){/* Put Synchronized exception catch here later - for now just eat errors. */ }
}

/** Resets run analysis close button caption. */
void __fastcall CalcThread::ResetProgressCloseButton(void) {
  gFormStatus.btnCancel->Caption = "Close";
}

/** Sets job cancelled in run analysis window. */
void __fastcall CalcThread::SetJobCancelled(void) {
  try {
    gFormStatus.CancelJob();
  }
  catch (...){/* Put Synchronized exception catch here later - for now just eat errors. */ }
}

/** Sets warnings in run analysis window. */
void __fastcall CalcThread::SetProgressWarnings(void) {
  try {
    if (gFormStatus.rteWarningsBox->Lines->Count == 0)
      gFormStatus.AddWarningLine("No warnings or errors encountered.");
  }
  catch (...){/* Put Synchronized exception catch here later - for now just eat errors. */ }
}

/** Internal setup function. */
void CalcThread::Setup(const CParameters& Parameters) {
  try {
    gpParameters = new CParameters(Parameters);
    gpParameters->SetRunHistoryFilename(GetToolkit().GetRunHistoryFileName());
    gpParameters->SetIsLoggingHistory(GetToolkit().GetLogRunHistory());
    gpPrintWindow = new PrintWindow(*this);
    Priority = tpNormal;  
    FreeOnTerminate = false;
  }
  catch (ZdException & x) {
    x.AddCallpath("Setup()", "CalcThread");
    throw;
  }
}  


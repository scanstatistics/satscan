//***************************************************************************
#include "stsSaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "AnalysisRun.h"
#include "ParametersValidate.h"
#include "Toolkit.h"

/** Constructor */
__fastcall CalcThread::CalcThread(TfrmAnalysisRun& Progress, const CParameters& Parameters)
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
void CalcThread::AddLineToProgress(const char * sText) {
  if (sText) {
    gsPrintString = sText;
    Synchronize((TThreadMethod)&PrintLineToProgress);
  }
}

/** Synchronizes text output to run analysis form in the main application thread. */
void CalcThread::AddWarningToProgress(const char * sText) {
  if (sText) {
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

/** Synchronizes enabling of run analysis form's print action in the main application thread. */
void __fastcall CalcThread::EnableProgressPrintAction(void) {
  gFormStatus.SetCanPrint(true);
}

/** Main Thread execution function. */
void __fastcall CalcThread::Execute() {
  time_t        RunTime;
  std::string   Acknowledgment;

  try {
    gpPrintWindow->Printf(AppToolkit::getToolkit().GetAcknowledgment(Acknowledgment), BasePrint::P_STDOUT);

    time(&RunTime);         // Pass to analysis to include in report

    if (!ParametersValidate(*gpParameters).Validate(*gpPrintWindow))
       GenerateResolvableException("\nInvalid parameter(s) encountered. Job cancelled.", "Execute()");

    //create analysis runner object and execute analysis
    AnalysisRunner(*gpParameters, RunTime, *gpPrintWindow);
    
    //report completion
    if (!IsCancelled()) {
      gpPrintWindow->Printf("\nSaTScan completed successfully.\nThe results have been written to: \n"
                            "  %s\n\n", BasePrint::P_STDOUT, gpParameters->GetOutputFileName().c_str());
      Synchronize((TThreadMethod)&LoadResultsFromFile);
    }

    Synchronize((TThreadMethod)&ResetProgressCloseButton);
    Synchronize((TThreadMethod)&EnableProgressPrintAction);
    Synchronize((TThreadMethod)&SetProgressWarnings);
    if (IsCancelled())
      // Process is done interacting with run analysis window. So if analysis
      // was cancelled, we can now safely tell run analysis that we are finished
      // with it(i.e. we won't we accessing gpFormStatus anymore).
      Synchronize((TThreadMethod)&ProcessAcknowledgesCancellation);
  }
  catch (ResolvableException &x) {
    //handle exceptions that occured from user or data errors
    x.AddCallpath("Execute()", "CalcThread");
    gpPrintWindow->Printf("%s\nEnd of Warnings and Errors", BasePrint::P_ERROR, x.GetErrorMessage());
    Synchronize((TThreadMethod)&ResetProgressCloseButton);
    Synchronize((TThreadMethod)&EnableProgressPrintAction);
    Synchronize((TThreadMethod)&EnableProgressEmailButton);
    CancellJob();
  }
  catch (ZdMemoryException &x) {
    //handle memory exceptions
    x.AddCallpath("Execute()", "CalcThread");
    gsProgramErrorCallPath = x.GetCallpath();
    Synchronize((TThreadMethod)&SetProgramErrorCallPath);
    gpPrintWindow->Printf("\nSaTScan is unable to perform analysis due to insuffient memory.\n"
                          "Please see 'Memory Requirements' in user guide for suggested solutions.\n"
                          "\nEnd of Warnings and Errors", BasePrint::P_ERROR);
    Synchronize((TThreadMethod)&ResetProgressCloseButton);
    Synchronize((TThreadMethod)&EnableProgressPrintAction);
    Synchronize((TThreadMethod)&EnableProgressEmailButton);
    CancellJob();
  }
  catch (ZdException &x) {
    //handle exceptions that occured from unexcepted program error
    x.AddCallpath("Execute()", "CalcThread");
    gsProgramErrorCallPath = x.GetCallpath();
    Synchronize((TThreadMethod)&SetProgramErrorCallPath);
    gpPrintWindow->Printf("\nProgram Error Detected:\n%s\nEnd of Warnings and Errors", BasePrint::P_ERROR, x.GetErrorMessage());
    Synchronize((TThreadMethod)&ResetProgressCloseButton);
    Synchronize((TThreadMethod)&EnableProgressPrintAction);
    Synchronize((TThreadMethod)&EnableProgressEmailButton);
    CancellJob();
  }
  catch (std::exception& x) {
    //handle exceptions that occured from unexcepted program error
    gpPrintWindow->Printf("\nProgram Error Detected:\n%s\nEnd of Warnings and Errors", BasePrint::P_ERROR, x.what());
    Synchronize((TThreadMethod)&ResetProgressCloseButton);
    Synchronize((TThreadMethod)&EnableProgressPrintAction);
    Synchronize((TThreadMethod)&EnableProgressEmailButton);
    CancellJob();
  }
  catch (...) {
    gpPrintWindow->Printf("\nUnknown Program Error Encountered\n"
                          "\nEnd of Warnings and Errors", BasePrint::P_ERROR);
    Synchronize((TThreadMethod)&ResetProgressCloseButton);
    Synchronize((TThreadMethod)&EnableProgressPrintAction);
    Synchronize((TThreadMethod)&EnableProgressEmailButton);
    CancellJob();
  }
  Synchronize((TThreadMethod)&ProcessSignalsCompletion);
}

/** Cleanup. */
void CalcThread::Free() {
   delete gpPrintWindow;
   delete gpParameters;
}

/** Internal initialization function. */
void CalcThread::Init(){
  gpParameters   = 0;
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
    gFormStatus.Caption = "Job cancelled";
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

/** Sets call stack string in run analysis class for inclusion in emial message. */
void __fastcall CalcThread::SetProgramErrorCallPath(void) {
  gFormStatus.gsProgramErrorCallPath = gsProgramErrorCallPath;
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
    gpPrintWindow = new PrintWindow(*this, gpParameters->GetSuppressingWarnings());
    gpParameters->SetRunHistoryFilename(AppToolkit::getToolkit().GetRunHistoryFileName());
    Priority = tpNormal;  
    FreeOnTerminate = false;
  }
  catch (ZdException & x) {
    x.AddCallpath("Setup()", "CalcThread");
    throw;
  }
}  


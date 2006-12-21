//******************************************************************************
#include "stsSaTScan.h"
#pragma hdrstop
//******************************************************************************

/** Constructor */
PrintWindow::PrintWindow(CalcThread & CalculationThread, bool bSuppressWarnings)
            :BasePrint(bSuppressWarnings), gCalculationThread(CalculationThread) {}

/** Destructor */
PrintWindow::~PrintWindow(){}

/** Returns whether analysis has been cancelled through run analysis window via calculation thread . */
bool PrintWindow::GetIsCanceled() const {
  return gCalculationThread.IsCancelled();
}

void PrintWindow::PrintError(const char * sMessage) {
  gCalculationThread.AddWarningToProgress(sMessage);
}

void PrintWindow::PrintNotice(const char * sMessage) {
  gCalculationThread.AddWarningToProgress(sMessage);
}

void PrintWindow::PrintStandard(const char * sMessage) {
  gCalculationThread.AddLineToProgress(sMessage);
}

void PrintWindow::PrintWarning(const char * sMessage) {
  gCalculationThread.AddWarningToProgress(sMessage);
}

/** Creates formatted output from variable number of parameter arguments and calls class Print() method. */
void PrintWindow::Printf(const char * sMessage, PrintType ePrintType, ...) {
  if (!sMessage) return;
  try {
    va_list varArgs;
    va_start(varArgs, ePrintType);
    size_t iStringLength = vsnprintf(&gsMessage[0], gsMessage.size(), sMessage, varArgs);
    va_end(varArgs);
    if (iStringLength > gsMessage.size()) {
      gsMessage.resize(iStringLength + 1);
      va_start(varArgs, ePrintType);
      vsnprintf(&gsMessage[0], iStringLength + 1, sMessage, varArgs);
      va_end(varArgs);
    }  
  }
  catch (...) {return;}

  if (gsMessage[strlen(&gsMessage[0])-1] == '\n') gsMessage[strlen(&gsMessage[0])-1] = '\0';
  BasePrint::Print(&gsMessage[0], ePrintType);
}


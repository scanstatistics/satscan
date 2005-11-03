//---------------------------------------------------------------------------
#include "stsSaTScan.h"
#pragma hdrstop

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

void PrintWindow::Print(const char * sMessage, PrintType ePrintType) {
  if (sMessage) {
    if (sMessage[strlen(sMessage)-1] == '\n')
      const_cast<char*>(sMessage)[strlen(sMessage)-1] = '\0';
    BasePrint::Print(sMessage, ePrintType);
  }
}

void PrintWindow::Printf(const char * sMessage, PrintType ePrintType, ...) {
  if (sMessage) {
    va_list   varArgs;
    va_start(varArgs, ePrintType);
    SetMessageFromArgs(varArgs, sMessage);
    va_end (varArgs);
    Print(gsMessage, ePrintType);
  }
}


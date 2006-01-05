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
  va_list   varArgs;
  int       iStringLength;   // Holds the length of the formatted output
  int       iCurrentLength;  // Current length of the buffer

  if (!sMessage) return;


  try {
    iCurrentLength = strlen (gsMessage);
    va_start(varArgs, ePrintType);
    iStringLength = vsnprintf(gsMessage, iCurrentLength + 1, sMessage, varArgs);
    va_end(varArgs);
    if (iStringLength > iCurrentLength) {
      delete [] gsMessage; gsMessage=0;
      gsMessage = new char[iStringLength + 1];
      va_start(varArgs, ePrintType);
      vsnprintf (gsMessage, iStringLength + 1, sMessage, varArgs);
      va_end(varArgs);
    }
  }
  catch (...) {return;}

  if (gsMessage[strlen(gsMessage)-1] == '\n') const_cast<char*>(gsMessage)[strlen(gsMessage)-1] = '\0';
  BasePrint::Print(gsMessage, ePrintType);
}


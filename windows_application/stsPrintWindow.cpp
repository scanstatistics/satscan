//---------------------------------------------------------------------------
#include "stsSaTScan.h"
#pragma hdrstop

/** Constructor */
PrintWindow::PrintWindow(CalcThread & CalculationThread)
            :BasePrint(), gCalculationThread(CalculationThread) {}

/** Destructor */
PrintWindow::~PrintWindow(){}

/** Returns whether analysis has been cancelled through run analysis window via calculation thread . */
bool PrintWindow::GetIsCanceled() {
  return gCalculationThread.IsCancelled();
}

/** Prints text to run analysis window via calculation thread. */
void PrintWindow::PrintLine(char *s) {
  if (s) {//if "s" ends with a "\n" then remove it... ???
    if (s[strlen(s)-1] == '\n')
      s[strlen(s)-1] = '\0';
    gCalculationThread.AddLineToProgress(s);
  }
}

/** Prints to run analysis window via calculation thread. */
void PrintWindow::PrintWarningLine(char *s) {
  if (s) {//if "s" ends with a "\n" then remove it...
    if (s[strlen(s)-1] == '\n')
      s[strlen(s)-1] = '\0';
    gCalculationThread.AddWarningToProgress(s);
   }
}

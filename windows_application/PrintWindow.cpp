//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "PrintWindow.h"

//---------------------------------------------------------------------------
PrintWindow::PrintWindow(TfrmAnalysisRun *pFormStatus)
            : BasePrint()
{
   gpAnalysisRunForm = pFormStatus;
}
//---------------------------------------------------------------------------
PrintWindow::~PrintWindow()
{
}
//---------------------------------------------------------------------------
bool PrintWindow::GetIsCanceled()
{
   return gpAnalysisRunForm->IsJobCanceled();
}
//---------------------------------------------------------------------------
void PrintWindow::PrintLine(char *s)
{
   //if "s" ends with a "\n" then remove it...
   if (s)
      {
      if (s[strlen(s)-1] == '\n')
         s[strlen(s)-1] = '\0';
      gpAnalysisRunForm->AddLine(s);
      }
}
void PrintWindow::PrintWarningLine(char *s)
{
   //if "s" ends with a "\n" then remove it...
   if (s)
      {
      if (s[strlen(s)-1] == '\n')
         s[strlen(s)-1] = '\0';
      gpAnalysisRunForm->AddWarningLine(s);
      }
}//---------------------------------------------------------------------------

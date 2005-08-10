#include <vcl.h>
#pragma hdrstop
#include "zd540.h"

#include "QueueWindow.h"
#include "Main.h"
#include <process.h>
#include<iostream>
#include<istream>
#include<fstream>
#pragma package(smart_init)
#pragma resource "*.dfm"

__fastcall TfrmQueueWindow::TfrmQueueWindow(TComponent* Owner) : TForm(Owner) {}

void TfrmQueueWindow::AddBatch(const char * sExecutableFilename, const char * sParameterFilename) {
  TListItem * pNewListItem = ltvScheduledBatchs->Items->Add();
  pNewListItem->Caption = sExecutableFilename;
  pNewListItem->SubItems->Add(sParameterFilename);
  pNewListItem->SubItems->Add("--");
  pNewListItem->SubItems->Add("--");
}

void __fastcall TfrmQueueWindow::ActionStartBatchesExecute(TObject *Sender)  {
  AnsiString    sCommandLine;
  int           i;
  bool          bResult;
  time_t        StartTime, StopTime;
  double        dExecutionTime, dSeconds,  dMinutes,  dHours;
  AnsiString    sTime, sCurTitle, sText;

  Show();

  if (frmMain->gpFrmOptions->chkSuppressDosWindow->Checked || frmMain->gpFrmOptions->chkMinimizeConsoleWindow->Checked)
    Application->Minimize();

  //reset status to queued
  for (i=0; i < ltvScheduledBatchs->Items->Count; i++)
     ltvScheduledBatchs->Items->Item[i]->SubItems->Strings[1] = "queued";

  sCurTitle = Application->Title;
  for (i=0; i < ltvScheduledBatchs->Items->Count; i++) {
    //ltvScheduledBatchs->Selected = ltvScheduledBatchs->Items->Item[i];
    Application->ProcessMessages();
    sText.printf("%s - Batch Queue [(%d of %d) %s]", sCurTitle.c_str(), i + 1, ltvScheduledBatchs->Items->Count,
                 ExtractFileName(ltvScheduledBatchs->Items->Item[i]->Caption).c_str());
    Application->Title = sText;
    //create process command line
    sCommandLine.sprintf("\"%s\" \"%s\"", // for testing in future -- need ability to have -v option for v4.0 and up
                        ltvScheduledBatchs->Items->Item[i]->Caption.c_str(),
                        ltvScheduledBatchs->Items->Item[i]->SubItems->Strings[0].c_str());
    //mark as running
    ltvScheduledBatchs->Items->Item[i]->SubItems->Strings[1] = "running";
    Application->ProcessMessages();
    //start process                       frmMain
    time(&StartTime);
    bResult = TfrmMain::Execute(sCommandLine, !frmMain->gpFrmOptions->chkSuppressDosWindow->Checked, frmMain->gpFrmOptions->GetThreadPriorityFlags(), frmMain->gpFrmOptions->chkMinimizeConsoleWindow->Checked);
    time(&StopTime);
    if (bResult) {
      //mark as completed
      ltvScheduledBatchs->Items->Item[i]->SubItems->Strings[1] = "completed";
      dExecutionTime = difftime(StopTime, StartTime);
      dHours = floor(dExecutionTime/(60*60));
      dMinutes = floor((dExecutionTime - dHours*60*60)/60);
      dSeconds   = dExecutionTime - (dHours*60*60) - (dMinutes*60);
      sTime.sprintf("%.0f hour%s %.0f minute%s  %.0f second%s", dHours , (0 < dHours && dHours < 1.5 ? "" : "s"),
                    dMinutes , (0 < dMinutes && dMinutes < 1.5 ? "" : "s"), dSeconds , (0.5 < dSeconds && dSeconds < 1.5 ? "" : "s"));
      ltvScheduledBatchs->Items->Item[i]->SubItems->Strings[2] = sTime.c_str();
    }
    else {
      //mark as failed
      ltvScheduledBatchs->Items->Item[i]->SubItems->Strings[1] = "failed";
      ltvScheduledBatchs->Items->Item[i]->SubItems->Strings[2] = "--";
    }
      
    Application->ProcessMessages();
  }
  Application->Title = sCurTitle;
  if (frmMain->gpFrmOptions->chkSuppressDosWindow->Checked || frmMain->gpFrmOptions->chkMinimizeConsoleWindow->Checked)
     Application->Restore();

//  if (gbRunVersionComparison && !gsVersionComparisonApplication.empty() && !access(gsVersionComparisonApplication.c_str(), 00))
//    RunVersionComparison();
}


#include "zd546.h"
#include <vcl.h>
#pragma hdrstop

#include "QueueWindow.h"
#include "QueueItemDefinition.h"
#include "Main.h"

#include <process.h>
#include<iostream>
#include<istream>
#include<fstream>
#pragma package(smart_init)
#pragma resource "*.dfm"

__fastcall TfrmQueueWindow::TfrmQueueWindow(TComponent* Owner) : TForm(Owner) {
  Setup();
}

void TfrmQueueWindow::AddBatch(const char * sExecutableFilename, const char * sParameterFilename, const char * sStatus) {
  TListItem * pNewListItem = ltvScheduledBatchs->Items->Add();
  pNewListItem->Caption = sExecutableFilename;
  pNewListItem->SubItems->Add(sParameterFilename);
  pNewListItem->SubItems->Add(sStatus);
  pNewListItem->SubItems->Add("--");
  EnableStartBatchButton();
  EnableRemoveBatchButton();
  EnableSaveBatchDefinitionsButton();  
}

void __fastcall TfrmQueueWindow::ActionCancelBatchesExecute(TObject *Sender) {
  ActionCancelBatches->Tag = 1;
}

void __fastcall TfrmQueueWindow::ActionCompareOutputFilesExecute(TObject *Sender) {
  int           i, iItem1=-1, iItem2=-1;
  AnsiString    sParameters;
  std::string   OutputFile1, OutputFile2;

  for (i=0; i < ltvScheduledBatchs->Items->Count && iItem2 == -1; i++)
     if (ltvScheduledBatchs->Items->Item[i]->Selected) {
       if (iItem1 == -1)
         iItem1 = i;
       else
         iItem2 = i;
     }

  if (frmMain->gpFrmOptions->edtComparisonApplication->Text.Length() == 0 ||
      access(frmMain->gpFrmOptions->edtComparisonApplication->Text.c_str(), 00)) {
    if (OpenDialog2->Execute())
      frmMain->gpFrmOptions->edtComparisonApplication->Text = OpenDialog2->FileName;
    else
      return;
  }

//  sOutputFile1 = ltvScheduledBatchs->Items->Item[iItem1]->SubItems->Strings[0].c_str();
  GetOutputFilename(ZdFileName(ltvScheduledBatchs->Items->Item[iItem1]->SubItems->Strings[0].c_str()), OutputFile1);
  GetOutputFilename(ZdFileName(ltvScheduledBatchs->Items->Item[iItem2]->SubItems->Strings[0].c_str()), OutputFile2);
//  OutputFile2 = ltvScheduledBatchs->Items->Item[iItem2]->SubItems->Strings[0].c_str();
  sParameters.sprintf("\"%s\" \"%s\"", OutputFile1.c_str(), OutputFile2.c_str());

  HINSTANCE hInst = ShellExecute(NULL, "open", frmMain->gpFrmOptions->edtComparisonApplication->Text.c_str(), sParameters.c_str(), 0, 0);
  if ((int)hInst <= 32)
    Application->MessageBox(sParameters.c_str(), "ShellExecute failed!", MB_OK);
}

void __fastcall TfrmQueueWindow::ActionLoadDefinitionsExecute(TObject *Sender) {
  int           i, j, iCount=0;
  std::string   sExecutableFilename, sParameterFilename, sStatus;

  if (OpenDialog->Execute()) {
    ClearBatches();
    std::ifstream in(OpenDialog->FileName.c_str());
    while (!in.eof()) {
         switch (iCount) {
           case 0 : std::getline(in, sExecutableFilename); iCount++; break;
           case 1 : std::getline(in, sParameterFilename);
                    AddBatch(sExecutableFilename.c_str(), sParameterFilename.c_str(), "queued");
                    iCount = 0; break;
         };
    }
  }
  EnableStartBatchButton();
  EnableRemoveBatchButton();
  EnableSaveBatchDefinitionsButton();
}

void __fastcall TfrmQueueWindow::ActionRemoveBatchExecute(TObject *Sender) {
  TListItem   * pListItem;

  for (int i=0; i < ltvScheduledBatchs->Items->Count; i++) {
     pListItem = ltvScheduledBatchs->Items->Item[i];
     if (pListItem->Selected)
      ltvScheduledBatchs->Items->Delete(pListItem->Index);
  }
  EnableRemoveBatchButton();
  EnableStartBatchButton();
  EnableSaveBatchDefinitionsButton();
}

void __fastcall TfrmQueueWindow::ActionSaveDefinitionsExecute(TObject *Sender) {
  int           i;
  TListItem   * pListItem;

  if (SaveDialog->Execute()) {
    std::ofstream out(SaveDialog->FileName.c_str(), std::ios_base::in|std::ios_base::out|std::ios_base::trunc);
    for (i=0; i < ltvScheduledBatchs->Items->Count; i++) {
       pListItem = ltvScheduledBatchs->Items->Item[i];
       out << pListItem->Caption.c_str() << std::endl;
       out << pListItem->SubItems->Strings[0].c_str() << std::endl;
    }
  }
}

void __fastcall TfrmQueueWindow::ActionScheduleBatchExecute(TObject *Sender) {
  TfrmBatchDefinition * pBatchDefinition;
  AnsiString            sDummy;

  pBatchDefinition = new TfrmBatchDefinition(this, gsLastExe.c_str(), sDummy);
  if (pBatchDefinition->ShowModal() == mrOk)
    AddBatch(pBatchDefinition->GetExecutableFilename(), pBatchDefinition->GetParameterFilename(), "queued");
  gsLastExe = pBatchDefinition->GetExecutableFilename();
  delete pBatchDefinition;
  EnableStartBatchButton();
  EnableRemoveBatchButton();
  EnableSaveBatchDefinitionsButton();
}

void __fastcall TfrmQueueWindow::ActionStartBatchesExecute(TObject *Sender)  {
  if (frmMain->gpFrmOptions->chkSuppressDosWindow->Checked || frmMain->gpFrmOptions->chkMinimizeConsoleWindow->Checked)
    Application->Minimize();

   if (frmMain->gpFrmOptions->GetExecuteThroughBatchFile())
     ExecuteThroughBatchFile();
   else
     ExecuteCreateProcessEachAnalysis();

   if (frmMain->gpFrmOptions->chkSuppressDosWindow->Checked || frmMain->gpFrmOptions->chkMinimizeConsoleWindow->Checked)
     Application->Restore();
}

void TfrmQueueWindow::RunVersionComparison() {
  ZdFileName    fMasterParamFile(Application->ExeName.c_str());
  AnsiString    sCommandLine;

  fMasterParamFile.SetFileName("Master_Parameter_List");
  fMasterParamFile.SetExtension(".prml");

  ZdIO  MasterParameterList(fMasterParamFile.GetFullPath(), ZDIO_OPEN_TRUNC|ZDIO_OPEN_WRITE|ZDIO_OPEN_CREATE);

  for (int i=0; i < ltvScheduledBatchs->Items->Count; ++i)
     MasterParameterList << ltvScheduledBatchs->Items->Item[i]->SubItems->Strings[0].c_str() << "\n";
  MasterParameterList.Close();
}

void TfrmQueueWindow::ClearBatches() {
  ltvScheduledBatchs->Items->Clear();
  EnableStartBatchButton();
  EnableRemoveBatchButton();
  EnableSaveBatchDefinitionsButton();
}

void TfrmQueueWindow::EnableBatchExecutingButtons(bool b) {
  //reset cancel flag
  ActionCancelBatches->Tag = 0;
  ActionCancelBatches->Enabled = b;
  //disable start button until all batches complete
  ActionStartBatches->Enabled = !b;
  ActionScheduleBatch->Enabled = !b;
  ActionSaveDefinitions->Enabled = !b;
  ActionLoadDefinitions->Enabled = !b;
  //mark process running flag
  gProcessRunning = b;
  EnableRemoveBatchButton();
}


void TfrmQueueWindow::EnableCompareFilesButton() {
  ActionCompareOutputFiles->Enabled = ltvScheduledBatchs->SelCount == 2;
}

void TfrmQueueWindow::EnableRemoveBatchButton() {
  ActionRemoveBatch->Enabled = ltvScheduledBatchs->SelCount && !gProcessRunning;
}

void TfrmQueueWindow::EnableSaveBatchDefinitionsButton() {
  ActionSaveDefinitions->Enabled = ltvScheduledBatchs->Items->Count;
}

void TfrmQueueWindow::EnableStartBatchButton() {
  ActionStartBatches->Enabled = ltvScheduledBatchs->Items->Count;
}

void TfrmQueueWindow::ExecuteCreateProcessEachAnalysis() {
  AnsiString    sCommandLine;
  int           i;
  bool          bResult;
  time_t        StartTime, StopTime;
  double        dExecutionTime, dSeconds,  dMinutes,  dHours;
  AnsiString    sTime, sCurTitle, sText;

  Show();

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
}

void TfrmQueueWindow::ExecuteThroughBatchFile() {
  std::ofstream         filestream;
  std::string           sOutputFilename;
  AnsiString            sFilename, sCommand;
  int                   iItemIndex=-1;

  DateSeparator = '_';
  TimeSeparator = '.';
  sFilename.printf("%s%s.runs.bat", ExtractFilePath(Application->ExeName).c_str(), DateTimeToStr(TDateTime::CurrentDateTime()).c_str());

 //create batch file
  filestream.open(sFilename.c_str());
  if (!filestream)
    return;

  Show();
  while (++iItemIndex < ltvScheduledBatchs->Items->Count) {
       if (!FileExists(ltvScheduledBatchs->Items->Item[iItemIndex]->SubItems->Strings[0].c_str())) {
         iItemIndex++;
         continue;
       }
       //get filename that will be the result file
       frmMain->GetResultFileName(ltvScheduledBatchs->Items->Item[iItemIndex]->SubItems->Strings[0].c_str(), sOutputFilename);
       //Execute comparator SatScan using the current Parameter file, but set commandline options for version check
       sCommand.printf("\"%s\" \"%s\" -o \"%s\"", ltvScheduledBatchs->Items->Item[iItemIndex]->Caption.c_str(), ltvScheduledBatchs->Items->Item[iItemIndex]->SubItems->Strings[0].c_str(), sOutputFilename.c_str());
       filestream << sCommand.c_str() << std::endl;
  }
  filestream.close();

  ///execute batch file
  if (!TfrmMain::Execute(sFilename.c_str(), !frmMain->gpFrmOptions->chkSuppressDosWindow->Checked, frmMain->gpFrmOptions->GetThreadPriorityFlags(), frmMain->gpFrmOptions->chkMinimizeConsoleWindow->Checked)) {
    Application->ProcessMessages();
    Application->MessageBox("Batch Operation Failed/Cancelled", "Failed/Cancelled", MB_OK);
    return;
  }
  Application->ProcessMessages();
  sCommand.printf("Batch Operation Successed\nSee file %s for listing of executed analyses.", sFilename.c_str());
  Application->MessageBox(sCommand.c_str(), "Complete", MB_OK);
}

std::string & TfrmQueueWindow::GetOutputFilename(const ZdFileName& ParameterFilename, std::string& OutputFilename) {
  int   count=0;

  //Determine the location of master results file
  ZdIniFile IniFile(ParameterFilename.GetFullPath());
  if (IniFile.GetNumSections()) {
    OutputFilename = IniFile.GetSection("[Output Files]")->GetString("ResultsFile");
    if (OutputFilename.empty())
      OutputFilename = IniFile.GetSection("[Output]")->GetString("ResultsFile");
  }
  else {
    //open parameter file and scan to the 6th line, which is the results filename
    ifstream ParameterFile(ParameterFilename.GetFullPath(), ios::in);
    while (++count < 7)
        std::getline(ParameterFile, OutputFilename);
  }

  //complete path if only filename specified
  if (OutputFilename.find("\\") == OutputFilename.npos)
    OutputFilename.insert(0, ParameterFilename.GetLocation());

  return OutputFilename;
}

void __fastcall TfrmQueueWindow::OnScheduledBatchsSelectItem(TObject *Sender, TListItem *Item, bool Selected) {
  EnableRemoveBatchButton();
  EnableCompareFilesButton();
}

void TfrmQueueWindow::Setup() {
  EnableStartBatchButton();
  EnableRemoveBatchButton();
  EnableSaveBatchDefinitionsButton();
  EnableCompareFilesButton();
  ActionCancelBatches->Enabled = false;
  gProcessRunning = false;
}


void __fastcall TfrmQueueWindow::ltvScheduledBatchsKeyDown(TObject *Sender,WORD &Key, TShiftState Shift) {
 if (Key == VK_DELETE)
   ActionRemoveBatchExecute(Sender);
}
//---------------------------------------------------------------------------


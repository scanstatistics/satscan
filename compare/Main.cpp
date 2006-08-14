//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "Main.h"
#include "QueueWindow.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmMain *frmMain;
#include <systdate.h>

/** constructor */
ParameterResultsInfo::ParameterResultsInfo(const char * sParameterFilename)
                     :geClusterInformation(UNKNOWN), geLocationInformation(UNKNOWN), geRelativeRisks(UNKNOWN),
                      geSimulatedRatios(UNKNOWN), geTimeDifferenceType(INCOMPLETE), guHoursDifferent(0),
                      guMinutesDifferent(0), guSecondsDifferent(0), gfTimeDifferencePercentage(0),
                      guYardStickHours(0), guYardStickMinutes(0), guYardStickSeconds(0),
                      guScrutinizedHours(0), guScrutinizedMinutes(0), guScrutinizedSeconds(0)
 {
  gParameterFilename = sParameterFilename;
}

/** destructor */
ParameterResultsInfo::~ParameterResultsInfo() {}

/** returns whether any results didn't match master set */
bool ParameterResultsInfo::GetHasMisMatches() const {
  return geClusterInformation == NOT_EQUAL || geLocationInformation == NOT_EQUAL ||
         geRelativeRisks == NOT_EQUAL || geSimulatedRatios == NOT_EQUAL;
}

/** returns whther any result files were missing */
bool ParameterResultsInfo::GetHasMissingFiles() const {
  return geClusterInformation < EQUAL || geLocationInformation < EQUAL ||
         geRelativeRisks < EQUAL || geSimulatedRatios < EQUAL;
}

void ParameterResultsInfo::SetResultTimes(unsigned short uYardStickHours, unsigned short uYardStickMinutes, unsigned short uYardStickSeconds,
                                          unsigned short uScrutinizedHours, unsigned short uScrutinizedMinutes, unsigned short uScrutinizedSeconds) {
  guYardStickHours = uYardStickHours;
  guYardStickMinutes = uYardStickMinutes;
  guYardStickSeconds = uYardStickSeconds;
  guScrutinizedHours = uScrutinizedHours;
  guScrutinizedMinutes = uScrutinizedMinutes;
  guScrutinizedSeconds = uScrutinizedSeconds;

  unsigned short        uHours, uMinutes, uSeconds;
  double                dYardStickTimeInSeconds = GetYardStickTimeInSeconds(),
                        dScrutinizedTimeInSeconds = GetScrutinizedTimeInSeconds();

  if (dYardStickTimeInSeconds == dScrutinizedTimeInSeconds) {
    SetTimeDifference(guScrutinizedHours, guScrutinizedMinutes, guScrutinizedSeconds, SAME);
    SetTimeDifferencePercentage(0.0);
  }
  else if (dYardStickTimeInSeconds > dScrutinizedTimeInSeconds) {
    SetTimeDifferencePercentage(std::fabs(100 * dScrutinizedTimeInSeconds/dYardStickTimeInSeconds - 100));
    dYardStickTimeInSeconds = dYardStickTimeInSeconds - dScrutinizedTimeInSeconds;
    uHours = dYardStickTimeInSeconds/3600.0;
    uMinutes = (dYardStickTimeInSeconds - uHours * 3600.0)/60.0;
    uSeconds = (double)(dYardStickTimeInSeconds - uHours * 3600 - uMinutes * 60);
    SetTimeDifference(uHours, uMinutes, uSeconds, FASTER);
  }
  else {
    SetTimeDifferencePercentage(std::fabs(100 * dScrutinizedTimeInSeconds/dYardStickTimeInSeconds - 100));
    dScrutinizedTimeInSeconds = dScrutinizedTimeInSeconds - dYardStickTimeInSeconds;
    uHours = dScrutinizedTimeInSeconds/3600.0;
    uMinutes = (dScrutinizedTimeInSeconds - uHours * 3600.0)/60.0;
    uSeconds = (double)(dScrutinizedTimeInSeconds - uHours * 3600 - uMinutes * 60);
    SetTimeDifference(uHours, uMinutes, uSeconds, SLOWER);
  }
}

/** Set difference in time between master and compare analyses */
void ParameterResultsInfo::SetTimeDifference(unsigned short uHours, unsigned short uMinutes, unsigned short uSeconds, TimeDifferenceType eTimeDifferenceType) {
  guHoursDifferent = uHours;
  guMinutesDifferent = uMinutes;
  guSecondsDifferent = uSeconds;
  geTimeDifferenceType = eTimeDifferenceType;
}

const char * TfrmMain::SCU_REGISTRY_KEY                 = "\\Software\\SaTScanCompareUtility";
const char * TfrmMain::LASTAPPCOMPARATOR_DATA           = "LastComparatorOpenExe";
const char * TfrmMain::LASTAPP_DATA                     = "LastOpenExe";
const char * TfrmMain::PARAMETER_DATA                   = "ParameterFile";
const char * TfrmMain::COMPARE_APP_DATA                 = "CompareProgram";
const char * TfrmMain::COMPARATOR_FILE_EXTENSION        = ".out.yardstick.txt";
const char * TfrmMain::COMPARE_FILE_EXTENSION           = ".out.scrutinized.txt";
const char * TfrmMain::ARCHIVE_APP_DATA                 = "ArchiveApp";
const char * TfrmMain::USE_ARCHIVE_APP_DATA             = "ArchivingResults";
const char * TfrmMain::ARCHIVE_APP_OPTIONS_DATA         = "ArchiveOptions";
const char * TfrmMain::ARCHIVE_DELETE_FILES_DATA        = "DeleteArchivedFiles";
const char * TfrmMain::SUPPRESS_CONSOLE_DATA            = "SuppressDosWindow";
const char * TfrmMain::THREAD_PRIORITY_CLASS_DATA       = "ThreadPriority";
const char * TfrmMain::INACTIVE_MINIMIZED_CONSOLE_DATA  = "InactiveMinimizedConsole";
const char * TfrmMain::EXECUTE_METHOD_DATA              = "ExecuteMethod";
const char * TfrmMain::EXECUTE_OPTIONS_YARDSTICK_DATA   = "YardstickExeOptions";
const char * TfrmMain::EXECUTE_OPTIONS_SCRUTINIZED_DATA = "ScrutinizedExeOptions";


/** constructor */
__fastcall TfrmMain::TfrmMain(TComponent* Owner) : TForm(Owner), gpFrmOptions(0) {
  TRegistry   * pRegistry = new TRegistry;
  int           iSize, iIndex=0;
  AnsiString    sParameterDataName, sParameterData;

  Caption = Application->Title;
  gpFrmOptions = new TfrmOptions(this);
  if (pRegistry->OpenKey(SCU_REGISTRY_KEY, true)) {
    edtYardstickExecutable->Text = pRegistry->ReadString(LASTAPPCOMPARATOR_DATA);
    edtScutinizedExecutable->Text = pRegistry->ReadString(LASTAPP_DATA);
    ltvScheduledBatchs->Items->Clear();
    sParameterDataName.printf("%s%d", PARAMETER_DATA, iIndex);
    sParameterData = pRegistry->ReadString(sParameterDataName);
    while (sParameterData.Length()) {
        ltvScheduledBatchs->Items->Add()->Caption = sParameterData;
        iIndex++;
        sParameterDataName.printf("%s%d", PARAMETER_DATA, iIndex);
        sParameterData = pRegistry->ReadString(sParameterDataName);
    }
    gpFrmOptions->edtComparisonApplication->Text = pRegistry->ReadString(COMPARE_APP_DATA);
    if (pRegistry->GetDataSize(USE_ARCHIVE_APP_DATA) != -1)
      gpFrmOptions->chkArchiveResults->Checked = pRegistry->ReadBool(USE_ARCHIVE_APP_DATA);
    gpFrmOptions->edtArchiveApplication->Text = pRegistry->ReadString(ARCHIVE_APP_DATA);
    gpFrmOptions->edtArchiveApplicationOptions->Text = pRegistry->ReadString(ARCHIVE_APP_OPTIONS_DATA);
    if (pRegistry->GetDataSize(ARCHIVE_DELETE_FILES_DATA) != -1)
      gpFrmOptions->chkDeleteFileAfterArchiving->Checked = pRegistry->ReadBool(ARCHIVE_DELETE_FILES_DATA);
    if (pRegistry->GetDataSize(SUPPRESS_CONSOLE_DATA) != -1)
      gpFrmOptions->chkSuppressDosWindow->Checked = pRegistry->ReadBool(SUPPRESS_CONSOLE_DATA);
    if (pRegistry->GetDataSize(THREAD_PRIORITY_CLASS_DATA) != -1)
      gpFrmOptions->rdoGroupThreadPriority->ItemIndex = pRegistry->ReadInteger(THREAD_PRIORITY_CLASS_DATA);
    if (pRegistry->GetDataSize(INACTIVE_MINIMIZED_CONSOLE_DATA) != -1)
      gpFrmOptions->chkMinimizeConsoleWindow->Checked = pRegistry->ReadBool(INACTIVE_MINIMIZED_CONSOLE_DATA);
    if (pRegistry->GetDataSize(EXECUTE_METHOD_DATA) != -1)
      gpFrmOptions->rdgExecuteMethod->ItemIndex = pRegistry->ReadInteger(EXECUTE_METHOD_DATA);
    edtYardstickOptions->Text = pRegistry->ReadString(EXECUTE_OPTIONS_YARDSTICK_DATA);
    edtScrutinizedOptions->Text = pRegistry->ReadString(EXECUTE_OPTIONS_SCRUTINIZED_DATA);

    pRegistry->CloseKey();
  }
  delete pRegistry;
  gvColumnSortOrder.resize(lstDisplay->Columns->Count, -1);
  EnableSaveResultsAction();
  EnableCompareActions();
  EnableStartAction();
  EnableSaveParametersListAction();
  EnableRemoveParameterAction();
  EnableViewAction();
}

/** destructor */
__fastcall TfrmMain::~TfrmMain() {
  try {
    TRegistry   * pRegistry = new TRegistry;
    int           i;
    AnsiString    sParameterDataName, sParameterData;


    if (pRegistry->OpenKey(SCU_REGISTRY_KEY, true)) {
      pRegistry->WriteString(LASTAPPCOMPARATOR_DATA, edtYardstickExecutable->Text);
      pRegistry->WriteString(LASTAPP_DATA, edtScutinizedExecutable->Text);
      for (i=0; i < ltvScheduledBatchs->Items->Count;) {
        sParameterDataName.printf("%s%d", PARAMETER_DATA, i);
        pRegistry->WriteString(sParameterDataName, ltvScheduledBatchs->Items->Item[i]->Caption);
        ++i;
      }
      //remove existing entries that where not overwritten
      sParameterDataName.printf("%s%d", PARAMETER_DATA, i);
      while (pRegistry->DeleteValue(sParameterDataName))
        sParameterDataName.printf("%s%d", PARAMETER_DATA, ++i);
      pRegistry->WriteString(COMPARE_APP_DATA, gpFrmOptions->edtComparisonApplication->Text);
      pRegistry->WriteBool(USE_ARCHIVE_APP_DATA, gpFrmOptions->chkArchiveResults->Checked);
      pRegistry->WriteString(ARCHIVE_APP_DATA, gpFrmOptions->edtArchiveApplication->Text);
      pRegistry->WriteString(ARCHIVE_APP_OPTIONS_DATA, gpFrmOptions->edtArchiveApplicationOptions->Text);
      pRegistry->WriteBool(ARCHIVE_DELETE_FILES_DATA, gpFrmOptions->chkDeleteFileAfterArchiving->Checked);
      pRegistry->WriteBool(SUPPRESS_CONSOLE_DATA, gpFrmOptions->chkSuppressDosWindow->Checked);
      pRegistry->WriteInteger(THREAD_PRIORITY_CLASS_DATA, gpFrmOptions->rdoGroupThreadPriority->ItemIndex);
      pRegistry->WriteBool(INACTIVE_MINIMIZED_CONSOLE_DATA, gpFrmOptions->chkMinimizeConsoleWindow->Checked);
      pRegistry->WriteInteger(EXECUTE_METHOD_DATA, gpFrmOptions->rdgExecuteMethod->ItemIndex);
      pRegistry->WriteString(EXECUTE_OPTIONS_YARDSTICK_DATA, edtYardstickOptions->Text);
      pRegistry->WriteString(EXECUTE_OPTIONS_SCRUTINIZED_DATA, edtScrutinizedOptions->Text);
      pRegistry->CloseKey();
    }
    delete pRegistry;
    delete gpFrmOptions;
  }
  catch (...){}
}

/** browses for parameter file and adds to list */
void __fastcall TfrmMain::ActionAddParameterFileExecute(TObject *Sender) {
  OpenDialog->FileName =  "";
  OpenDialog->DefaultExt = "*.prm";
  OpenDialog->Filter = "Parameters files (*.prm)|*.prm|Text files (*.txt)|*.txt|All files (*.*)|*.*";
  OpenDialog->FilterIndex = 0;
  OpenDialog->Title = "Add Parameter File";
  OpenDialog->Options << ofAllowMultiSelect;
  if (OpenDialog->Execute())
    for (int i=0; i < OpenDialog->Files->Count; i++)
      ltvScheduledBatchs->Items->Add()->Caption = OpenDialog->Files->Strings[i];
  EnableStartAction();
  EnableSaveParametersListAction();
  EnableRemoveParameterAction();
  EnableViewAction();
}

/** clears parameter list*/
void __fastcall TfrmMain::ActionClearListExecute(TObject *Sender) {
  ltvScheduledBatchs->Items->Clear();
  EnableStartAction();
  EnableSaveParametersListAction();
  EnableRemoveParameterAction();
  EnableViewAction();
}

/** launches compare program for viewing differences in cluster information files */
void __fastcall TfrmMain::ActionCompareClusterInformationExecute(TObject *Sender) {
  std::string   sMaster, sCompare;
  AnsiString    sParameters;

  if (!lstDisplay->Selected)
    return;

  if (!PromptForCompareProgram())
    return;

  const ZdFileName & Ref = gvParameterResultsInfo[(size_t)lstDisplay->Selected->Data].GetFilename();
  GetResultFileName(Ref, sMaster);
  sMaster.insert(sMaster.find_last_of("."),".col");
  GetInQuestionFilename(Ref, sCompare);
  sCompare.insert(sCompare.find_last_of("."),".col");
  //launch comparison program
  sParameters.sprintf("\"%s\" \"%s\"", sMaster.c_str(), sCompare.c_str());
  HINSTANCE hInst = ShellExecute(NULL, "open", gpFrmOptions->edtComparisonApplication->Text.c_str(), sParameters.c_str(), 0, 0);
  if ((int)hInst <= 32)
    Application->MessageBox("Unable to launch comparison program.", "Error", MB_OK);
}

/** launches compare program for viewing differences in location information files */
void __fastcall TfrmMain::ActionCompareLocationInformationExecute(TObject *Sender) {
  std::string   sMaster, sCompare;
  AnsiString    sParameters;

  if (!lstDisplay->Selected)
    return;

  if (!PromptForCompareProgram())
    return;

  const ZdFileName & Ref = gvParameterResultsInfo[(size_t)lstDisplay->Selected->Data].GetFilename();
  GetResultFileName(Ref, sMaster);
  sMaster.insert(sMaster.find_last_of("."),".gis");
  GetInQuestionFilename(Ref, sCompare);
  sCompare.insert(sCompare.find_last_of("."),".gis");
  //launch comparison program
  sParameters.sprintf("\"%s\" \"%s\"", sMaster.c_str(), sCompare.c_str());
  HINSTANCE hInst = ShellExecute(NULL, "open", gpFrmOptions->edtComparisonApplication->Text.c_str(), sParameters.c_str(), 0, 0);
  if ((int)hInst <= 32)
    Application->MessageBox("Unable to launch comparison program.", "Error", MB_OK);
}

/** launches compare program for viewing differences in relative risks files */
void __fastcall TfrmMain::ActionCompareRelativeRisksExecute(TObject *Sender) {
  std::string   sMaster, sCompare;
  AnsiString    sParameters;

  if (!lstDisplay->Selected)
    return;

  if (!PromptForCompareProgram())
    return;

  const ZdFileName & Ref = gvParameterResultsInfo[(size_t)lstDisplay->Selected->Data].GetFilename();
  GetResultFileName(Ref, sMaster);
  sMaster.insert(sMaster.find_last_of("."),".rr");
  GetInQuestionFilename(Ref, sCompare);
  sCompare.insert(sCompare.find_last_of("."),".rr");
  //launch comparison program
  sParameters.sprintf("\"%s\" \"%s\"", sMaster.c_str(), sCompare.c_str());
  HINSTANCE hInst = ShellExecute(NULL, "open", gpFrmOptions->edtComparisonApplication->Text.c_str(), sParameters.c_str(), 0, 0);
  if ((int)hInst <= 32)
    Application->MessageBox("Unable to launch comparison program.", "Error", MB_OK);
}

/** launches compare program for viewing differences in simulated loglikelihood files */
void __fastcall TfrmMain::ActionCompareSimulatedLLRsExecute(TObject *Sender) {
  std::string   sMaster, sCompare;
  AnsiString    sParameters;

  if (!lstDisplay->Selected)
    return;

  if (!PromptForCompareProgram())
    return;

  const ZdFileName & Ref = gvParameterResultsInfo[(size_t)lstDisplay->Selected->Data].GetFilename();
  GetResultFileName(Ref, sMaster);
  sMaster.insert(sMaster.find_last_of("."),".llr");
  GetInQuestionFilename(Ref, sCompare);
  sCompare.insert(sCompare.find_last_of("."),".llr");
  //launch comparison program
  sParameters.sprintf("\"%s\" \"%s\"", sMaster.c_str(), sCompare.c_str());
  HINSTANCE hInst = ShellExecute(NULL, "open", gpFrmOptions->edtComparisonApplication->Text.c_str(), sParameters.c_str(), 0, 0);
  if ((int)hInst <= 32)
    Application->MessageBox("Unable to launch comparison program.", "Error", MB_OK);
}

/** launches compare program for viewing differences in output files */
void __fastcall TfrmMain::ActionCompareResultFilesExecute(TObject *Sender) {
  std::string   sMaster, sCompare;
  AnsiString    sParameters;

  if (!lstDisplay->Selected)
    return;

  if (!PromptForCompareProgram())
      return;

  //launch comparison program
  const ZdFileName & Ref = gvParameterResultsInfo[(size_t)lstDisplay->Selected->Data].GetFilename();
  sParameters.sprintf("\"%s\" \"%s\"", GetResultFileName(Ref, sMaster).c_str(), GetInQuestionFilename(Ref, sCompare).c_str());
  HINSTANCE hInst = ShellExecute(NULL, "open", gpFrmOptions->edtComparisonApplication->Text.c_str(), sParameters.c_str(), 0, 0);
  if ((int)hInst <= 32)
    Application->MessageBox("Unable to launch comparison program.", "Error", MB_OK);
}

/** delete analyses files created */
void __fastcall TfrmMain::ActionDeleteAnalysesFilesExecute(TObject *Sender) {
  std::string   sMaster, sCompare, sDeleteMe;
  TCursor aCursor = Screen->Cursor;

  Screen->Cursor = crHourGlass;
  //add files of each comparison process
  for (int i=0; i < lstDisplay->Items->Count; ++i) {
    const ParameterResultsInfo & Ref = gvParameterResultsInfo[(size_t)lstDisplay->Items->Item[i]->Data];

    //delete results file
    remove(GetResultFileName(Ref.GetFilename(), sMaster).c_str());
    remove(GetInQuestionFilename(Ref.GetFilename(), sCompare).c_str());
    //delete cluster information file
    sDeleteMe = sMaster;
    sDeleteMe.insert(sDeleteMe.find_last_of("."),".col");
    remove(sDeleteMe.c_str());
    sDeleteMe = sCompare;
    sDeleteMe.insert(sDeleteMe.find_last_of("."),".col");
    remove(sDeleteMe.c_str());
    //delete location information file
    sDeleteMe = sMaster;
    sDeleteMe.insert(sDeleteMe.find_last_of("."),".gis");
    remove(sDeleteMe.c_str());
    sDeleteMe = sCompare;
    sDeleteMe.insert(sDeleteMe.find_last_of("."),".gis");
    remove(sDeleteMe.c_str());
    //add relatives risks
    if (Ref.GetRelativeRisksType() == EQUAL || Ref.GetRelativeRisksType() == NOT_EQUAL) {
      sDeleteMe = sMaster;
      sDeleteMe.insert(sDeleteMe.find_last_of("."),".rr");
      remove(sDeleteMe.c_str());
      sDeleteMe = sCompare;
      sDeleteMe.insert(sDeleteMe.find_last_of("."),".rr");
      remove(sDeleteMe.c_str());
    }
    //delete simulated ratios
    sDeleteMe = sMaster;
    sDeleteMe.insert(sDeleteMe.find_last_of("."),".llr");
    remove(sDeleteMe.c_str());
    sDeleteMe = sCompare;
    sDeleteMe.insert(sDeleteMe.find_last_of("."),".llr");
    remove(sDeleteMe.c_str());
  }
  Screen->Cursor = aCursor;
}

/** loads parameters from file */
void __fastcall TfrmMain::ActionLoadParameterListExecute(TObject *Sender){
  std::string   sParameterFilename;

  OpenDialog->FileName =  "";
  OpenDialog->DefaultExt = "*.prml";
  OpenDialog->Filter = "Parameters List files (*.prml)|*.prml|Text files (*.txt)|*.txt|All files (*.*)|*.*";
  OpenDialog->FilterIndex = 0;
  OpenDialog->Title = "Load Parameters List from File";
  OpenDialog->Options >> ofAllowMultiSelect;
  if (OpenDialog->Execute()) {
    ltvScheduledBatchs->Items->Clear();
    std::ifstream in(OpenDialog->FileName.c_str());
    while (!in.eof()) {
         std::getline(in, sParameterFilename);
          TListItem * pItem = ltvScheduledBatchs->Items->Add();
          pItem->Caption = sParameterFilename.c_str();
          pItem->Caption = pItem->Caption.Trim();
          if (!pItem->Caption.Length())
            pItem->Delete();

    }
  }
  EnableStartAction();
  EnableSaveParametersListAction();
  EnableRemoveParameterAction();
  EnableViewAction();
}

/** launches options dialog */
void __fastcall TfrmMain::ActionOptionsExecute(TObject *Sender) {
  gpFrmOptions->ShowModal();
}

/** removes selected parameter file from list */
void __fastcall TfrmMain::ActionRemoveParameterFileExecute(TObject *Sender){
  TListItem   * pListItem;

  for (int i=0; i < ltvScheduledBatchs->Items->Count; i++) {
     pListItem = ltvScheduledBatchs->Items->Item[i];
     if (pListItem->Selected)
      ltvScheduledBatchs->Items->Delete(pListItem->Index);
  }
  EnableStartAction();
  EnableSaveParametersListAction();
  EnableRemoveParameterAction();
  EnableViewAction();
}

/** saves results of comparison to file */
void __fastcall TfrmMain::ActionSaveComparisonStatsExecute(TObject *Sender) {
  SaveDialog->FileName =  "";
  SaveDialog->DefaultExt = "*.txt";
  SaveDialog->Filter = "Text files (*.txt)|*.txt|All files (*.*)|*.*";
  SaveDialog->FilterIndex = 0;
  SaveDialog->Title = "Save Analysis Results to File";
  if (SaveDialog->Execute())
    CreateStatsFile(SaveDialog->FileName.c_str());
}

/** saves parameters to file */
void __fastcall TfrmMain::ActionSaveParametersListExecute(TObject *Sender){
  int           i;
  TListItem   * pListItem;

  SaveDialog->FileName =  "";
  SaveDialog->DefaultExt = "*.prml";
  SaveDialog->Filter = "Parameters List files (*.prml)|*.prml|Text files (*.txt)|*.txt|All files (*.*)|*.*";
  SaveDialog->FilterIndex = 0;
  SaveDialog->Title = "Save Parameters List to File";
  if (SaveDialog->Execute()) {
    std::ofstream out(SaveDialog->FileName.c_str(), std::ios_base::in|std::ios_base::out|std::ios_base::trunc);
    for (i=0; i < ltvScheduledBatchs->Items->Count; i++)
       out << ltvScheduledBatchs->Items->Item[i]->Caption.c_str() << std::endl;
  }
}

/** starts process of comparing output files */
void __fastcall TfrmMain::ActionStartExecute(TObject *Sender) {
  if (gpFrmOptions->chkSuppressDosWindow->Checked || gpFrmOptions->chkMinimizeConsoleWindow->Checked)
    Application->Minimize();

   if (gpFrmOptions->GetExecuteThroughBatchFile())
     ExecuteThroughBatchFile();
   else
     ExecuteCreateProcessEachAnalysis();

   if (gpFrmOptions->chkSuppressDosWindow->Checked || gpFrmOptions->chkMinimizeConsoleWindow->Checked)
     Application->Restore();
}

/** opens selected parameter files in associated text viewer */
void __fastcall TfrmMain::ActionViewParametersExecute(TObject *Sender) {
  HINSTANCE     hReturn;
  TListItem   * pListItem;

  for (int i=0; i < ltvScheduledBatchs->Items->Count; i++) {
     pListItem = ltvScheduledBatchs->Items->Item[i];
     if (pListItem->Selected) {
       //Attempt to open chm user guide.
       hReturn = ShellExecute(Handle, "open", pListItem->Caption.c_str(), NULL, NULL, SW_SHOWNORMAL);
       if (hReturn <= (void*)32)
         Application->MessageBox("Unable to open file.", "Failed open", MB_OK);
     }
  }
}

/** add item to display indicating results of analysis comparison */
void TfrmMain::AddList(const ParameterResultsInfo& ResultsInfo, size_t tPosition) {
  AnsiString    sDisplay;

  TListItem * pListItem = lstDisplay->Items->Add();
  pListItem->Data = (void*)(tPosition);

  if (ResultsInfo.GetHasMisMatches() || ResultsInfo.GetTimeDifferenceType() == SLOWER)
    pListItem->ImageIndex = 2;
  else if (ResultsInfo.GetHasMissingFiles() || ResultsInfo.GetTimeDifferenceType() == INCOMPLETE)
    pListItem->ImageIndex = 1;
  else
    pListItem->ImageIndex = 0;

  pListItem->SubItems->Add(ResultsInfo.GetFilename().GetCompleteFileName());
  pListItem->SubItems->Add(GetDisplayTime(ResultsInfo, sDisplay));
  AddSubItemForType(pListItem, ResultsInfo.GetClusterInformationType());
  AddSubItemForType(pListItem, ResultsInfo.GetLocationInformationType());
  AddSubItemForType(pListItem, ResultsInfo.GetRelativeRisksType());
  AddSubItemForType(pListItem, ResultsInfo.GetSimulatedRatiosType());
}

/** adds sub menu item for compare result type */
void TfrmMain::AddSubItemForType(TListItem * pListItem, CompareType eType) {
  switch (eType) {
    case MASTER_MISSING  : pListItem->SubItems->Add("Master Missing"); break;
    case COMPARE_MISSING : pListItem->SubItems->Add("Compare Missing"); break;
    case EQUAL           : pListItem->SubItems->Add("Equal"); break;
    case NOT_EQUAL       : pListItem->SubItems->Add("Not Equal"); break;
    case NOT_APPLICABLE  : pListItem->SubItems->Add("Not Applicable"); break;
    default              : pListItem->SubItems->Add("Not Compared"); break;
  };
}

/** creates and adds analyses results to arhive file */
void TfrmMain::ArchiveResults() {
  AnsiString    sArchiveFilename, sStatsFilename, sCommand;
  std::string   sMaster, sCompare, sTemp1, sTemp2;

  //create archive name
  DateSeparator = '_';
  TimeSeparator = '.';
  sArchiveFilename.printf("%s%s.zip", ExtractFilePath(Application->ExeName).c_str(), DateTimeToStr(gStartDate).c_str());
  if (!access(sArchiveFilename.c_str(), 00) && remove(sArchiveFilename.c_str()))
    return;

  //print archive filename
  memMessages->Lines->Add("Archive Filename:");
  memMessages->Lines->Add(sArchiveFilename);
  memMessages->Lines->Add("");
  //create analysis stats file
  sStatsFilename.printf("%s%s.stats.txt", ExtractFilePath(Application->ExeName).c_str(), DateTimeToStr(gStartDate).c_str());
  CreateStatsFile(sStatsFilename.c_str());
  sCommand.sprintf("\"%s\" %s \"%s\" \"%s\"",
                   gpFrmOptions->edtArchiveApplication->Text.c_str(),
                   gpFrmOptions->edtArchiveApplicationOptions->Text.c_str(),
                   sArchiveFilename.c_str(),
                   sStatsFilename.c_str());
  Execute(sCommand, false, gpFrmOptions->GetThreadPriorityFlags(), gpFrmOptions->chkMinimizeConsoleWindow->Checked);
  remove(sStatsFilename.c_str());
  //add Excel file
  sStatsFilename.printf("%s%s.speed.compare.xls", ExtractFilePath(Application->ExeName).c_str(), DateTimeToStr(gStartDate).c_str());
  sCommand.sprintf("\"%s\" %s \"%s\" \"%s\"",
                   gpFrmOptions->edtArchiveApplication->Text.c_str(),
                   gpFrmOptions->edtArchiveApplicationOptions->Text.c_str(),
                   sArchiveFilename.c_str(),
                   sStatsFilename.c_str());
  Execute(sCommand, false, gpFrmOptions->GetThreadPriorityFlags(), gpFrmOptions->chkMinimizeConsoleWindow->Checked);

  //add 'ReadMe' file to archive
  if (gpFrmOptions->chkCreateReadMeFile->Checked) {
    sStatsFilename.printf("%sReadMe.txt", ExtractFilePath(Application->ExeName).c_str());
    CreateReadMeFile(sStatsFilename.c_str());
    sCommand.sprintf("\"%s\" %s \"%s\" \"%s\"",
                     gpFrmOptions->edtArchiveApplication->Text.c_str(),
                     gpFrmOptions->edtArchiveApplicationOptions->Text.c_str(),
                     sArchiveFilename.c_str(),
                     sStatsFilename.c_str());

    Execute(sCommand, false, gpFrmOptions->GetThreadPriorityFlags(), gpFrmOptions->chkMinimizeConsoleWindow->Checked);
    remove(sStatsFilename.c_str());
  }
  //add files of each comparison process
  for (int i=0; i < lstDisplay->Items->Count; ++i) {
    const ParameterResultsInfo & Ref = gvParameterResultsInfo[(size_t)lstDisplay->Items->Item[i]->Data];

    //add parameter file
    sCommand.sprintf("\"%s\" %s \"%s\" \"%s\"",
                     gpFrmOptions->edtArchiveApplication->Text.c_str(),
                     gpFrmOptions->edtArchiveApplicationOptions->Text.c_str(),
                     sArchiveFilename.c_str(),
                     ltvScheduledBatchs->Items->Item[i]->Caption.c_str());
    Execute(sCommand, false, gpFrmOptions->GetThreadPriorityFlags(), gpFrmOptions->chkMinimizeConsoleWindow->Checked);

    //add results file
    sCommand.sprintf("\"%s\" %s \"%s\" \"%s\" \"%s\"",
                     gpFrmOptions->edtArchiveApplication->Text.c_str(),
                     gpFrmOptions->edtArchiveApplicationOptions->Text.c_str(),
                     sArchiveFilename.c_str(),
                     GetResultFileName(Ref.GetFilename(), sMaster).c_str(),
                     GetInQuestionFilename(Ref.GetFilename(), sCompare).c_str());
    if (Execute(sCommand, false, gpFrmOptions->GetThreadPriorityFlags(), gpFrmOptions->chkMinimizeConsoleWindow->Checked) && gpFrmOptions->chkDeleteFileAfterArchiving->Checked) {
      remove(sMaster.c_str());
      remove(sCompare.c_str());
    }

    //add cluster information file
    sTemp1 = sMaster;
    sTemp1.insert(sTemp1.find_last_of("."),".col");
    sTemp2 = sCompare;
    sTemp2.insert(sTemp2.find_last_of("."),".col");
    sCommand.sprintf("\"%s\" %s \"%s\" \"%s\" \"%s\"",
                     gpFrmOptions->edtArchiveApplication->Text.c_str(),
                     gpFrmOptions->edtArchiveApplicationOptions->Text.c_str(),
                     sArchiveFilename.c_str(), sTemp1.c_str(), sTemp2.c_str());
    if (Execute(sCommand, false, gpFrmOptions->GetThreadPriorityFlags(), gpFrmOptions->chkMinimizeConsoleWindow->Checked) && gpFrmOptions->chkDeleteFileAfterArchiving->Checked) {
      remove(sTemp1.c_str());
      remove(sTemp2.c_str());
    }

    //add location information file
    sTemp1 = sMaster;
    sTemp1.insert(sTemp1.find_last_of("."),".gis");
    sTemp2 = sCompare;
    sTemp2.insert(sTemp2.find_last_of("."),".gis");
    sCommand.sprintf("\"%s\" %s \"%s\" \"%s\" \"%s\"",
                     gpFrmOptions->edtArchiveApplication->Text.c_str(),
                     gpFrmOptions->edtArchiveApplicationOptions->Text.c_str(),
                     sArchiveFilename.c_str(), sTemp1.c_str(), sTemp2.c_str());
    if (Execute(sCommand, false, gpFrmOptions->GetThreadPriorityFlags(), gpFrmOptions->chkMinimizeConsoleWindow->Checked) && gpFrmOptions->chkDeleteFileAfterArchiving->Checked) {
      remove(sTemp1.c_str());
      remove(sTemp2.c_str());
    }

    //add relatives risks
    if (Ref.GetRelativeRisksType() == EQUAL || Ref.GetRelativeRisksType() == NOT_EQUAL) {
      sTemp1 = sMaster;
      sTemp1.insert(sTemp1.find_last_of("."),".rr");
      sTemp2 = sCompare;
      sTemp2.insert(sTemp2.find_last_of("."),".rr");
      sCommand.sprintf("\"%s\" %s \"%s\" \"%s\" \"%s\"",
                       gpFrmOptions->edtArchiveApplication->Text.c_str(),
                       gpFrmOptions->edtArchiveApplicationOptions->Text.c_str(),
                       sArchiveFilename.c_str(), sTemp1.c_str(), sTemp2.c_str());
      if (Execute(sCommand, false, gpFrmOptions->GetThreadPriorityFlags(), gpFrmOptions->chkMinimizeConsoleWindow->Checked) && gpFrmOptions->chkDeleteFileAfterArchiving->Checked) {
        remove(sTemp1.c_str());
        remove(sTemp2.c_str());
      }
    }

    //add simulated ratios
    sTemp1 = sMaster;
    sTemp1.insert(sTemp1.find_last_of("."),".llr");
    sTemp2 = sCompare;
    sTemp2.insert(sTemp2.find_last_of("."),".llr");
    sCommand.sprintf("\"%s\" %s \"%s\" \"%s\" \"%s\"",
                     gpFrmOptions->edtArchiveApplication->Text.c_str(),
                     gpFrmOptions->edtArchiveApplicationOptions->Text.c_str(),
                     sArchiveFilename.c_str(), sTemp1.c_str(), sTemp2.c_str());
    if (Execute(sCommand, false, gpFrmOptions->GetThreadPriorityFlags(), gpFrmOptions->chkMinimizeConsoleWindow->Checked) && gpFrmOptions->chkDeleteFileAfterArchiving->Checked) {
      remove(sTemp1.c_str());
      remove(sTemp2.c_str());
    }
  }
}

/** Sets open dialog filters and displays for selecting executable. */
void __fastcall TfrmMain::btnBrowseBatchExecutableComparatorClick(TObject *Sender) {
  OpenDialog->FileName =  "";
  OpenDialog->DefaultExt = "*.exe";
  OpenDialog->Filter = "Executable files (*.exe)|*.exe|All files (*.*)|*.*";
  OpenDialog->FilterIndex = 0;
  OpenDialog->Title = "Select SaTScan Batch Executable (comparator)";
  OpenDialog->Options >> ofAllowMultiSelect;
  if (OpenDialog->Execute())
    edtYardstickExecutable->Text = OpenDialog->FileName;
}

/** Sets open dialog filters and displays for selecting executable. */
void __fastcall TfrmMain::btnBrowseBatchExecutableClick(TObject *Sender) {
  OpenDialog->FileName =  "";
  OpenDialog->DefaultExt = "*.exe";
  OpenDialog->Filter = "Executable files (*.exe)|*.exe|All files (*.*)|*.*";
  OpenDialog->FilterIndex = 0;
  OpenDialog->Title = "Select SaTScan Batch Executable";
  OpenDialog->Options >> ofAllowMultiSelect;
  if (OpenDialog->Execute())
    edtScutinizedExecutable->Text = OpenDialog->FileName;
}

/** Sets open dialog filters and displays for selecting parameter list file *//** compare Cluster Information files */
void TfrmMain::CompareClusterInformationFiles(ParameterResultsInfo& ResultsInfo) {
  CompareType     eType;
  std::string     sMaster, sCompare;

  GetResultFileName(ResultsInfo.GetFilename(), sMaster);
  GetInQuestionFilename(ResultsInfo.GetFilename(), sCompare);

  sMaster.insert(sMaster.find_last_of("."),".col");
  sCompare.insert(sCompare.find_last_of("."),".col");
  if (access(sMaster.c_str(), 00))
    eType = MASTER_MISSING;
  else if (access(sCompare.c_str(), 00))
    eType = COMPARE_MISSING;
  else if (CompareTextFiles(sMaster, sCompare))
    eType = EQUAL;
  else
    eType = NOT_EQUAL;

  ResultsInfo.SetClusterInformationType(eType);
}

/** compare Location Information files */
void TfrmMain::CompareLocationInformationFiles(ParameterResultsInfo& ResultsInfo) {
  CompareType     eType;
  std::string     sMaster, sCompare;

  GetResultFileName(ResultsInfo.GetFilename(), sMaster);
  GetInQuestionFilename(ResultsInfo.GetFilename(), sCompare);

  sMaster.insert(sMaster.find_last_of("."),".gis");
  sCompare.insert(sCompare.find_last_of("."),".gis");
  if (access(sMaster.c_str(), 00))
    eType = MASTER_MISSING;
  else if (access(sCompare.c_str(), 00))
    eType = COMPARE_MISSING;
  else if (CompareTextFiles(sMaster, sCompare))
    eType = EQUAL;
  else
    eType = NOT_EQUAL;

  ResultsInfo.SetLocationInformationType(eType);
}

/** compare Relative Risks files */
void TfrmMain::CompareRelativeRisksInformationFiles(ParameterResultsInfo& ResultsInfo) {
  CompareType   eType;
  std::string   sMaster, sCompare, sLineBuffer;
  unsigned int  iPos, iLine=0;

  //not applicable for Space-Time Permutation model
  ZdIniFile IniFile(ResultsInfo.GetFilenameString());
  if (IniFile.GetNumSections())
    sLineBuffer = IniFile.GetSection("[Analysis]")->GetString("ModelType");
  else {
    ifstream ParameterFile(ResultsInfo.GetFilenameString());
    while (++iLine < 22 && std::getline(ParameterFile, sLineBuffer));
    if ((iPos = sLineBuffer.find_first_of("//")) != sLineBuffer.npos)
      sLineBuffer = sLineBuffer.substr(0, iPos);
    sLineBuffer = Trim(sLineBuffer.c_str()).c_str();
  }

  if (sLineBuffer == "2"/* Space-Time Permutation */) {
    eType = NOT_APPLICABLE;
  }
  else {
    GetResultFileName(ResultsInfo.GetFilename(), sMaster);
    GetInQuestionFilename(ResultsInfo.GetFilename(), sCompare);

    sMaster.insert(sMaster.find_last_of("."),".rr");
    sCompare.insert(sCompare.find_last_of("."),".rr");
    if (access(sMaster.c_str(), 00))
      eType = MASTER_MISSING;
    else if (access(sCompare.c_str(), 00))
      eType = COMPARE_MISSING;
    else if (CompareTextFiles(sMaster, sCompare))
      eType = EQUAL;
    else
      eType = NOT_EQUAL;
  }

  ResultsInfo.SetRelativeRisksType(eType);
}

/** Compare results and update TList/ParameterResultsInfo objects. */
void TfrmMain::CompareResultsAndReport() {
  std::vector<ParameterResultsInfo>::iterator     itr=gvParameterResultsInfo.begin();

  for (;itr != gvParameterResultsInfo.end(); ++itr) {
     CompareClusterInformationFiles(*itr);
     CompareLocationInformationFiles(*itr);
     CompareRelativeRisksInformationFiles(*itr);
     CompareSimulatedRatiosFiles(*itr);
     CompareTimes(*itr);
     AddList(*itr, std::distance(gvParameterResultsInfo.begin(), itr));
   }  
}

/** compare Simulated Loglikelihood Ratio files */
void TfrmMain::CompareSimulatedRatiosFiles(ParameterResultsInfo& ResultsInfo) {
  CompareType     eType;
  std::string     sMaster, sCompare;

  GetResultFileName(ResultsInfo.GetFilename(), sMaster);
  GetInQuestionFilename(ResultsInfo.GetFilename(), sCompare);

  sMaster.insert(sMaster.find_last_of("."),".llr");
  sCompare.insert(sCompare.find_last_of("."),".llr");
  if (access(sMaster.c_str(), 00))
    eType = MASTER_MISSING;
  else if (access(sCompare.c_str(), 00))
    eType = COMPARE_MISSING;
  else if (CompareTextFiles(sMaster, sCompare))
    eType = EQUAL;
  else
    eType = NOT_EQUAL;

  ResultsInfo.SetSimulatedRatiosType(eType);
}

/** returns whether text files are equal, ignoring whitespace differences */
bool TfrmMain::CompareTextFiles(const std::string & sMaster, const std::string & sCompare) {
  ifstream correctOutput(sMaster.c_str(), ios::in), output(sCompare.c_str(), ios::in);
  string correctStr, Str;
  bool bEqual;

  bEqual = output && correctOutput;
  while (output && correctOutput && bEqual) {
       //ignore whitespace
       output >> ws;
       correctOutput >> ws;
       //read strings to compare
       output >> Str;
       correctOutput >> correctStr;
       //compare strings
       bEqual = (Str == correctStr);
  }
  return bEqual;
}

/** compares and records time differences between master and this analysis */
void TfrmMain::CompareTimes(ParameterResultsInfo& ResultsInfo) {
  bool                  bMasterDate;
  unsigned short        uHoursY, uMinutesY, uSecondsY, uHoursS, uMinutesS, uSecondsS;
  std::string           sMaster, sCompare;

  GetResultFileName(ResultsInfo.GetFilename(), sMaster);
  if (GetRunTime(sMaster.c_str(), uHoursY, uMinutesY, uSecondsY)) {
    GetInQuestionFilename(ResultsInfo.GetFilename(), sCompare);
    GetRunTime(sCompare.c_str(), uHoursS, uMinutesS, uSecondsS);
    ResultsInfo.SetResultTimes(uHoursY, uMinutesY, uSecondsY, uHoursS, uMinutesS, uSecondsS);
  }
  else
    ResultsInfo.SetTimeDifference(0, 0, 0, INCOMPLETE);
}

/** creates excel spread sheet of results */
void TfrmMain::CreateExcelSheet() {
  CMiniExcel                                            ExcelSheet;
  unsigned int                                          iColumnIndex=0, iRowIndex=0;
  std::vector<ParameterResultsInfo>::const_iterator     itr=gvParameterResultsInfo.begin();
  AnsiString                                            sString, sYardTime, sScrutinizedTime;

  //add column headers
  ExcelSheet(iRowIndex, iColumnIndex) = "Data Set";
  ExcelSheet(iRowIndex, iColumnIndex).setBorder(BORDER_LEFT|BORDER_TOP|BORDER_BOTTOM);
  ExcelSheet(iRowIndex, ++iColumnIndex) = "Yardstick Speed (min)";
  ExcelSheet(iRowIndex, iColumnIndex).setBorder(BORDER_TOP|BORDER_BOTTOM);
  ExcelSheet(iRowIndex, ++iColumnIndex) = "Scrutinized Speed (min)";
  ExcelSheet(iRowIndex, iColumnIndex).setBorder(BORDER_TOP|BORDER_BOTTOM);
  ExcelSheet(iRowIndex, ++iColumnIndex) = "Speed Difference (%)";
  ExcelSheet(iRowIndex, iColumnIndex).setBorder(BORDER_TOP|BORDER_BOTTOM);
  ExcelSheet(iRowIndex, ++iColumnIndex) = "Date";
  ExcelSheet(iRowIndex, iColumnIndex).setBorder(BORDER_TOP|BORDER_BOTTOM);
  ExcelSheet(iRowIndex, ++iColumnIndex) = "Comments";
  ExcelSheet(iRowIndex, iColumnIndex).setBorder(BORDER_RIGHT|BORDER_TOP|BORDER_BOTTOM);

  DateSeparator = '/';
  TimeSeparator = ':';
  for (; itr != gvParameterResultsInfo.end(); ++itr) {
    iColumnIndex=0;
    ExcelSheet(++iRowIndex, iColumnIndex) = itr->GetFilename().GetFileName();
    sYardTime.printf("%.2lf", itr->GetYardStickTimeInMinutes());
    ExcelSheet(iRowIndex, ++iColumnIndex) = sYardTime.ToDouble();
    sScrutinizedTime.printf("%.2lf", itr->GetScrutinizedTimeInMinutes());
    ExcelSheet(iRowIndex, ++iColumnIndex) = sScrutinizedTime.ToDouble();
    sString.printf("%.2lf", 100.0 * sScrutinizedTime.ToDouble()/sYardTime.ToDouble() - 100.0);
    ExcelSheet(iRowIndex, ++iColumnIndex) = sString.ToDouble();
    ExcelSheet(iRowIndex, ++iColumnIndex) = DateTimeToStr(gStartDate).c_str();
  }

  DateSeparator = '_';
  TimeSeparator = '.';
  sString.printf("%s%s.speed.compare.xls", ExtractFilePath(Application->ExeName).c_str(), DateTimeToStr(gStartDate).c_str());
  FILE *f;
  if ((!access(sString.c_str(), 00) && remove(sString.c_str())) || (f = fopen(sString.c_str(),"wb")) == NULL) {
    memMessages->Lines->Add("Could not create Excel filename:");
    memMessages->Lines->Add(sString);
    memMessages->Lines->Add("");
    return;
  }
  ExcelSheet.Write(f);
  fclose(f);
  //print archive filename
  memMessages->Lines->Add("Excel Filename:");
  memMessages->Lines->Add(sString);
  memMessages->Lines->Add("");
}

/** creates 'ReadMe' file */
void TfrmMain::CreateReadMeFile(const char * sFilename) {
  ofstream Output(sFilename);

  for (int i=0; i < gpFrmOptions->memReadMeText->Lines->Count; ++i)
     Output << gpFrmOptions->memReadMeText->Lines->Strings[i].c_str() << endl;
}

/** creates and saves comparison results to file */
void TfrmMain::CreateStatsFile(const char * sFilename) {
 int            i, iNumMisMatch=0,iNumCorrect=0, iNumMissingFiles=0, iNumProgramStop=0;
 TListItem    * pListItem;

  if (sFilename) {
    ofstream Output(sFilename, ios_base::out|ios_base::trunc);

    //print header information
    Output << endl << "Date: " << gStartDate.DateTimeString().c_str() << endl;

    for (i=0; i < lstDisplay->Items->Count; i++)
       switch (lstDisplay->Items->Item[i]->ImageIndex) {
         case 0 : iNumCorrect++; break;
         case 1 : iNumMissingFiles++; break;
         case 2 : iNumMisMatch++; break;
         case 3 : iNumProgramStop++; break;
         //default : error
       };

    Output << "Total Analyses         : " << lstDisplay->Items->Count << endl
           << "Compared Equal         : " << iNumCorrect << endl
           << "Compared Not Equal     : " << iNumMisMatch << endl
           << "Missing Files          : " << iNumMissingFiles << endl
           << "Program Failure/Cancel : " << iNumProgramStop << endl << endl;

    //print results of comparisons
    for (i=0; i < lstDisplay->Items->Count; i++) {
       pListItem = lstDisplay->Items->Item[i];
       Output << "Parameter List Item " << i + 1 << ")" << endl
              << "                    Parameter Filename   : " << pListItem->SubItems->Strings[0].c_str() << endl
              << "                    Speed                : " << pListItem->SubItems->Strings[1].c_str() << endl
              << "                    Cluster Information  : " << pListItem->SubItems->Strings[2].c_str() << endl
              << "                    Location Information : " << pListItem->SubItems->Strings[3].c_str() << endl
              << "                    Relative Risks       : " << pListItem->SubItems->Strings[4].c_str() << endl
              << "                    Simulated Ratios     : " << pListItem->SubItems->Strings[5].c_str() << endl << endl;
    }
  }
}

void __fastcall TfrmMain::edtChangeInput(TObject *Sender) {
  EnableStartAction();
}

/** enables compare results action */
void TfrmMain::EnableCompareActions() {
  if (lstDisplay->Selected) {
    const ParameterResultsInfo & Ref = gvParameterResultsInfo[(size_t)(lstDisplay->Selected->Data)];
    ActionCompareResultFiles->Enabled = true;
    ActionCompareClusterInformation->Enabled = Ref.GetClusterInformationType() ==  EQUAL || Ref.GetClusterInformationType() ==  NOT_EQUAL;
    ActionCompareLocationInformation->Enabled = Ref.GetLocationInformationType() == EQUAL || Ref.GetLocationInformationType() == NOT_EQUAL;
    ActionCompareRelativeRisks->Enabled = Ref.GetRelativeRisksType() == EQUAL || Ref.GetRelativeRisksType() == NOT_EQUAL;
    ActionCompareSimulatedLLRs->Enabled = Ref.GetSimulatedRatiosType() == EQUAL || Ref.GetSimulatedRatiosType() == NOT_EQUAL;
  }
  else {
    ActionCompareResultFiles->Enabled = false;
    ActionCompareClusterInformation->Enabled = false;
    ActionCompareLocationInformation->Enabled = false;
    ActionCompareRelativeRisks->Enabled = false;
    ActionCompareSimulatedLLRs->Enabled = false;
  }
}

void TfrmMain::EnableRemoveParameterAction() {
  ActionRemoveParameterFile->Enabled = ltvScheduledBatchs->SelCount;
}

void TfrmMain::EnableSaveParametersListAction() {
  ActionSaveParametersList->Enabled = ltvScheduledBatchs->Items->Count;
}

/** enables save results action */
void TfrmMain::EnableSaveResultsAction() {
  ActionSaveComparisonStats->Enabled = lstDisplay->Items->Count;
}

/** enables start action */
void TfrmMain::EnableStartAction() {
  ActionStart->Enabled = edtScutinizedExecutable->Text.Length() &&
                         edtYardstickExecutable->Text.Length() &&
                         ltvScheduledBatchs->Items->Count;
}

/** enables view parameters action */
void TfrmMain::EnableViewAction() {
  ActionViewParameters->Enabled = ltvScheduledBatchs->SelCount;
}

/** creates process to execute command */
bool TfrmMain::Execute(const AnsiString & sCommandLine, bool bWindowed, DWORD wThreadPriority, bool bInactiveMinimizedWindow) {
   STARTUPINFO          si;
   PROCESS_INFORMATION  pi;
   unsigned long        lProcessTerminationStatus=-1;

   //initialize structs
   ZeroMemory(&si, sizeof(si));
   si.cb = sizeof(si);
   ZeroMemory(&pi,sizeof(pi));
   //Start the child process.
   si.dwFlags = STARTF_USEFILLATTRIBUTE | STARTF_USESHOWWINDOW;
   si.dwFillAttribute = FOREGROUND_RED| BACKGROUND_RED| BACKGROUND_GREEN| BACKGROUND_BLUE;
   si.wShowWindow = (bInactiveMinimizedWindow && bWindowed ? SW_SHOWMINNOACTIVE : SW_SHOWDEFAULT);
   if(!CreateProcess(NULL,                                         //No module name (use command line).
                     const_cast<char*>(sCommandLine.c_str()),      //Command line.
                     NULL,                                         //Process handle not inheritable.
                     NULL,                                         //Thread handle not inheritable.
                     FALSE,                                        //Set handle inheritance to FALSE.
                     (bWindowed ? CREATE_NEW_CONSOLE : CREATE_NO_WINDOW) | wThreadPriority, //creation flags.
                     NULL,                                         //Use parent's environment block.
                     NULL,                                         //Use parent's starting directory.
                     &si,                                          //Pointer to STARTUPINFO structure.
                     &pi))                                       //Pointer to PROCESS_INFORMATION structure.
     {
      //memMessages->Lines->Add("Process create failed!");
      //memMessages->Lines->Add(sCommandLine.c_str());
      //memMessages->Lines->Add("");
     }

   //Wait until child process exits.
   WaitForSingleObject(pi.hProcess,INFINITE);
   GetExitCodeProcess(pi.hProcess, &lProcessTerminationStatus);

   //Close process and thread handles.
   CloseHandle(pi.hProcess);
   CloseHandle(pi.hThread);

   return (lProcessTerminationStatus == 0 ? true : false);
}

void TfrmMain::ExecuteCreateProcessEachAnalysis() {
   std::string  sComparatorFilename, sCompareFilename;
   AnsiString   sCommand, sCurTitle, sText;
   int          iItemIndex=-1;

   //get start time of execution
   gStartDate = TDateTime::CurrentDateTime();
   DateSeparator = '/';
   TimeSeparator = ':';

   memMessages->Clear();
   lstDisplay->Items->Clear();
   gvParameterResultsInfo.clear();
   gvColumnSortOrder.clear();
   sCurTitle = Application->Title;
   gvColumnSortOrder.resize(lstDisplay->Columns->Count, -1);
   while (++iItemIndex < ltvScheduledBatchs->Items->Count) {
        sText.printf("%s [(%d of %d) %s]", sCurTitle.c_str(), iItemIndex + 1, ltvScheduledBatchs->Items->Count, ExtractFileName(ltvScheduledBatchs->Items->Item[iItemIndex]->Caption).c_str());
        Application->Title = sText;
        ParameterResultsInfo thisParameterInfo(ltvScheduledBatchs->Items->Item[iItemIndex]->Caption.c_str());
        if (!FileExists(thisParameterInfo.GetFilenameString())) {
          memMessages->Lines->Add("Parameter File Missing : ");
          memMessages->Lines->Add(ltvScheduledBatchs->Items->Item[iItemIndex]->Caption);
          iItemIndex++;
          continue;
        }
        //get filename that will be the result file
        GetResultFileName(thisParameterInfo.GetFilename(), sComparatorFilename);
        //Execute comparator SatScan using the current Parameter file, but set commandline options for version check
        sCommand.printf("\"%s\" \"%s\" -o \"%s\" %s", edtYardstickExecutable->Text.c_str(), thisParameterInfo.GetFilenameString(), sComparatorFilename.c_str(), edtYardstickOptions->Text.c_str());
        if (!Execute(sCommand.c_str(), !gpFrmOptions->chkSuppressDosWindow->Checked, gpFrmOptions->GetThreadPriorityFlags(), gpFrmOptions->chkMinimizeConsoleWindow->Checked)) {
          memMessages->Lines->Add("Yardstick Executable Failed/Cancelled : ");
          memMessages->Lines->Add(ltvScheduledBatchs->Items->Item[iItemIndex]->Caption);
          memMessages->Lines->Add("");
          continue;
        }
        //get filename that will be the result file created for comparison
        GetInQuestionFilename(thisParameterInfo.GetFilename(), sCompareFilename);
        //Execute SatScan using the current Parameter file, but set commandline options for version check
        sCommand.printf("\"%s\" \"%s\" -o \"%s\" %s", edtScutinizedExecutable->Text.c_str(), thisParameterInfo.GetFilenameString(), sCompareFilename.c_str(), edtScrutinizedOptions->Text.c_str());
        //execute SaTScan version that is in question
        if (!Execute(sCommand.c_str(), !gpFrmOptions->chkSuppressDosWindow->Checked, gpFrmOptions->GetThreadPriorityFlags(), gpFrmOptions->chkMinimizeConsoleWindow->Checked)) {
          memMessages->Lines->Add("Scrutinized Executable Failed/Cancelled : ");
          memMessages->Lines->Add(ltvScheduledBatchs->Items->Item[iItemIndex]->Caption);
          memMessages->Lines->Add("");
          continue;
        }
        gvParameterResultsInfo.push_back(thisParameterInfo);
   }
   Application->ProcessMessages();
   Application->Title = sCurTitle;
   //compare results
   CompareResultsAndReport();
   //build excel results sheet
   CreateExcelSheet();
   //archive results
   if (gpFrmOptions->chkArchiveResults->Checked)
     ArchiveResults();
   EnableSaveResultsAction();
   //print execution time to message window
   sCommand.printf("Start time: %s    Stop time: %s", DateTimeToStr(gStartDate).c_str(), DateTimeToStr(TDateTime::CurrentDateTime()).c_str());
   memMessages->Lines->Add(sCommand);
   memMessages->SelStart = 0;
}

void TfrmMain::ExecuteThroughBatchFile() {
  std::ofstream         filestream;
  std::string           sComparatorFilename, sCompareFilename;
  AnsiString            sFilename, sCommand, sCurTitle, sText;
  int                   iItemIndex=-1;

  gStartDate = TDateTime::CurrentDateTime();
  DateSeparator = '_';
  TimeSeparator = '.';
  sFilename.printf("%s%s.runs.bat", ExtractFilePath(Application->ExeName).c_str(), DateTimeToStr(gStartDate).c_str());


 //create batch file
  filestream.open(sFilename.c_str());
  if (!filestream)
    return;

   memMessages->Clear();
   lstDisplay->Items->Clear();
   gvParameterResultsInfo.clear();
   sCurTitle = Application->Title;   
   gvColumnSortOrder.clear();
   gvColumnSortOrder.resize(lstDisplay->Columns->Count, -1);
       
   while (++iItemIndex < ltvScheduledBatchs->Items->Count) {
        ParameterResultsInfo thisParameterInfo(ltvScheduledBatchs->Items->Item[iItemIndex]->Caption.c_str());
        if (!FileExists(thisParameterInfo.GetFilenameString())) {
          memMessages->Lines->Add("Parameter File Missing : ");
          memMessages->Lines->Add(ltvScheduledBatchs->Items->Item[iItemIndex]->Caption);
          iItemIndex++;
          continue;
        }
        //get filename that will be the result file
        GetResultFileName(thisParameterInfo.GetFilename(), sComparatorFilename);
        //Execute comparator SatScan using the current Parameter file, but set commandline options for version check
        sCommand.printf("\"%s\" \"%s\" -o \"%s\" %s", edtYardstickExecutable->Text.c_str(), thisParameterInfo.GetFilenameString(), sComparatorFilename.c_str(), edtYardstickOptions->Text.c_str());
        filestream << sCommand.c_str() << std::endl;
        //get filename that will be the result file created for comparison
        GetInQuestionFilename(thisParameterInfo.GetFilename(), sCompareFilename);
        //Execute SatScan using the current Parameter file, but set commandline options for version check
        sCommand.printf("\"%s\" \"%s\" -o \"%s\" %s", edtScutinizedExecutable->Text.c_str(), thisParameterInfo.GetFilenameString(), sCompareFilename.c_str(), edtScrutinizedOptions->Text.c_str());
        filestream << sCommand.c_str() << std::endl;
        gvParameterResultsInfo.push_back(thisParameterInfo);
   }
   filestream.close();

   sText.printf("%s executing %s]", Application->Title.c_str(), ExtractFileName(sFilename).c_str());
   Application->Title = sText;

   ///execute batch file
   if (!Execute(sFilename.c_str(), !gpFrmOptions->chkSuppressDosWindow->Checked, gpFrmOptions->GetThreadPriorityFlags(), gpFrmOptions->chkMinimizeConsoleWindow->Checked)) {
      memMessages->Lines->Add("Batch Operation Failed/Cancelled");
      memMessages->Lines->Add("");
      Application->ProcessMessages();
      Application->Title = sCurTitle;
      return;
   }

   Application->ProcessMessages();
   Application->Title = sCurTitle;
   //compare results
   CompareResultsAndReport();
   //build excel results sheet
   CreateExcelSheet();
   //archive results
   if (gpFrmOptions->chkArchiveResults->Checked)
     ArchiveResults();
   EnableSaveResultsAction();
   //print execution time to message window
   sCommand.printf("Start time: %s    Stop time: %s", DateTimeToStr(gStartDate).c_str(), DateTimeToStr(TDateTime::CurrentDateTime()).c_str());
   memMessages->Lines->Add(sCommand);
   memMessages->SelStart = 0;
}

/** Gets filename of file that will be the alternate results to compare to original */
std::string & TfrmMain::GetComparatorFilename(const ZdFileName & ParameterFilename, std::string & sResultFilename) {
  //defined alternate results filename -- this will be the new file we will compare against
  sResultFilename = ParameterFilename.GetLocation();
  sResultFilename += ParameterFilename.GetFileName();
  sResultFilename += ".";
  DateSeparator = '_';
  TimeSeparator = '.';
  sResultFilename += DateTimeToStr(gStartDate).c_str();
  sResultFilename += COMPARATOR_FILE_EXTENSION;

  return sResultFilename;
}

/** Gets filename of file that will be the alternate results to compare to original */
std::string & TfrmMain::GetInQuestionFilename(const ZdFileName & ParameterFilename, std::string & sResultFilename) {
  //defined alternate results filename -- this will be the new file we will compare against
  sResultFilename = ParameterFilename.GetLocation();
  sResultFilename += ParameterFilename.GetFileName();
  sResultFilename += ".";
  DateSeparator = '_';
  TimeSeparator = '.';
  sResultFilename += DateTimeToStr(gStartDate).c_str();  
  sResultFilename += COMPARE_FILE_EXTENSION;

  return sResultFilename;
}

/** adds analysis speed comparison to display */
AnsiString & TfrmMain::GetDisplayTime(const ParameterResultsInfo& ResultsInfo, AnsiString & sDisplay) {
  switch (ResultsInfo.GetTimeDifferenceType()) {
    case INCOMPLETE : sDisplay.printf("not completed (?)");
                      break;
    case SLOWER     : sDisplay.printf("%i hr %i min % i sec (%.2f%% slower)", ResultsInfo.GetHoursDifferent(),
                                      ResultsInfo.GetMinutesDifferent(), ResultsInfo.GetSecondsDifferent(),
                                      ResultsInfo.GetTimeDifferencePercentage());
                      break;
    case SAME       : sDisplay.printf("%i hr %i min % i sec (same time)", ResultsInfo.GetHoursDifferent(),
                                      ResultsInfo.GetMinutesDifferent(), ResultsInfo.GetSecondsDifferent());
                      break;
    case FASTER     : sDisplay.printf("%i hr %i min % i sec (%.2f%% faster)", ResultsInfo.GetHoursDifferent(),
                                      ResultsInfo.GetMinutesDifferent(), ResultsInfo.GetSecondsDifferent(),
                                      ResultsInfo.GetTimeDifferencePercentage());
                      break;
    //default : error
  };
  return sDisplay;
}

/** Sets result filename as specified by parameter file. If path is missing from
    filename, path of parameter file is inserted as path of result file. */
std::string & TfrmMain::GetResultFileName(const ZdFileName & ParameterFilename, std::string & sResultFilename) {
  return GetComparatorFilename(ParameterFilename, sResultFilename);
}

/** parses out analysis time for execution string from results file */
bool TfrmMain::GetRunTime(const char * sResultFile, unsigned short& uHours, unsigned short& uMinutes, unsigned short& uSeconds) {
   ifstream             file(sResultFile);
   std::string          sLine;
   bool                 bFoundTimeLine=false;
   ZdStringTokenizer    DateTokenizer("", " ");

   if (! file)
     return false;

   while (file && !bFoundTimeLine) {
     std::getline(file, sLine);
     if (sLine.find("Total Running Time : ") != sLine.npos)
       bFoundTimeLine = true;
   }

   if (!bFoundTimeLine)
     return false;

   DateTokenizer.SetString(sLine.substr(strlen("Total Running Time : ")).c_str());

   if (DateTokenizer.GetNumTokens() == 2) {
     uHours = 0;
     uMinutes = 0;
     uSeconds = atoi(DateTokenizer.GetToken(0).GetCString());
   }
   else if (DateTokenizer.GetNumTokens() == 4) {
     uHours = 0;
     uMinutes = atoi(DateTokenizer.GetToken(0).GetCString());
     uSeconds = atoi(DateTokenizer.GetToken(2).GetCString());
   }
   else if (DateTokenizer.GetNumTokens() == 6) {
     uHours = atoi(DateTokenizer.GetToken(0).GetCString());
     uMinutes = atoi(DateTokenizer.GetToken(2).GetCString());
     uSeconds = atoi(DateTokenizer.GetToken(4).GetCString());
   }
   else
     return false;

   return true;
}

void __fastcall TfrmMain::lstDisplayColumnClick(TObject *Sender,TListColumn *Column) {
  giSortColumnIndex = Column->Index;
  if (gvColumnSortOrder[giSortColumnIndex] == -1)
    gvColumnSortOrder[giSortColumnIndex] = 1;
  else
    gvColumnSortOrder[giSortColumnIndex] = -1;
  lstDisplay->AlphaSort();
}

void __fastcall TfrmMain::lstDisplayCompare(TObject *Sender, TListItem *Item1, TListItem *Item2, int Data, int &Compare) {
  if (giSortColumnIndex == 0) {
    if (Item1->ImageIndex == Item2->ImageIndex)
      Compare = 0;
    else if (Item1->ImageIndex < Item2->ImageIndex)
      Compare = -1;
    else
      Compare = 1;
  }
  else
    Compare = CompareText(Item1->SubItems->Strings[giSortColumnIndex - 1], Item2->SubItems->Strings[giSortColumnIndex - 1]);

  Compare *=  gvColumnSortOrder[giSortColumnIndex];
}

void __fastcall TfrmMain::lstDisplaySelectItem(TObject *Sender, TListItem *Item, bool Selected) {
  EnableCompareActions();
}

void __fastcall TfrmMain::lstScheduledBatchsKeyDown(TObject *Sender, WORD &Key, TShiftState Shift) {
 if (Key == VK_DELETE)
   ActionRemoveParameterFileExecute(Sender);
}

void __fastcall TfrmMain::lstScheduledBatchsSelectItem(TObject *Sender, TListItem *Item, bool Selected) {
  EnableRemoveParameterAction();
  EnableViewAction();
}

/** displays open dialog when compare program not set */
bool TfrmMain::PromptForCompareProgram() {
  if (!gpFrmOptions->edtComparisonApplication->Text.Length()) {
    gpFrmOptions->ShowModal();
    return gpFrmOptions->edtComparisonApplication->Text.Length();
  }
  return true;
}

void __fastcall TfrmMain::btnExecuteQueueComparatorClick(TObject *Sender) {
  std::auto_ptr<TfrmQueueWindow> pDialog(new TfrmQueueWindow(this, edtYardstickOptions->Text));

  for (int i=0; i < ltvScheduledBatchs->Items->Count; ++i)
     pDialog->AddBatch(edtYardstickExecutable->Text.c_str(), ltvScheduledBatchs->Items->Item[i]->Caption.c_str());

  pDialog->ShowModal();
}
//---------------------------------------------------------------------------
void __fastcall TfrmMain::btnExecuteQueueQuestionClick(TObject *Sender) {
  std::auto_ptr<TfrmQueueWindow> pDialog(new TfrmQueueWindow(this, edtScrutinizedOptions->Text));

  for (int i=0; i < ltvScheduledBatchs->Items->Count; ++i)
     pDialog->AddBatch(edtScutinizedExecutable->Text.c_str(), ltvScheduledBatchs->Items->Item[i]->Caption.c_str());

  pDialog->ShowModal();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "Main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmMain *frmMain;
#include <systdate.h>

/** constructor */
ParameterResultsInfo::ParameterResultsInfo(const char * sParameterFilename)
                     :geClusterInformation(UNKNOWN), geLocationInformation(UNKNOWN), geRelativeRisks(UNKNOWN),
                      geSimulatedRatios(UNKNOWN), geTimeDifferenceType(INCOMPLETE), guHoursDifferent(0),
                      guMinutesDifferent(0), guSecondsDifferent(0), gfTimeDifferencePercentage(0) {
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

/** Set difference in time between master and compare analyses */
void ParameterResultsInfo::SetTimeDifference(unsigned short uHours, unsigned short uMinutes, unsigned short uSeconds, TimeDifferenceType eTimeDifferenceType) {
  guHoursDifferent = uHours;
  guMinutesDifferent = uMinutes;
  guSecondsDifferent = uSeconds;
  geTimeDifferenceType = eTimeDifferenceType;
}

const char * TfrmMain::SCU_REGISTRY_KEY         = "\\Software\\SaTScanCompareUtility";
const char * TfrmMain::LASTAPPCOMPARATOR_DATA   = "LastComparatorOpenExe";
const char * TfrmMain::LASTAPP_DATA             = "LastOpenExe";
const char * TfrmMain::PARAMETER_DATA           = "ParameterFile";
const char * TfrmMain::COMPARE_APP_DATA         = "CompareProgram";
const char * TfrmMain::COMPARE_FILE_EXTENSION   = ".out.compare.txt";
const char * TfrmMain::ARCHIVE_APP_DATA         = "ArchiveApp";
const char * TfrmMain::USE_ARCHIVE_APP_DATA     = "ArchivingResults";
const char * TfrmMain::ARCHIVE_APP_OPTIONS_DATA = "ArchiveOptions";
const char * TfrmMain::ARCHIVE_DELETE_FILES_DATA= "DeleteArchivedFiles";

/** constructor */
__fastcall TfrmMain::TfrmMain(TComponent* Owner) : TForm(Owner), gpFrmOptions(0) {
  TRegistry   * pRegistry = new TRegistry;
  int           iSize, iIndex=0;
  AnsiString    sParameterDataName, sParameterData;

  gpFrmOptions = new TfrmOptions(this);
  if (pRegistry->OpenKey(SCU_REGISTRY_KEY, true)) {
    edtBatchExecutableComparatorName->Text = pRegistry->ReadString(LASTAPPCOMPARATOR_DATA);
    edtBatchExecutableName->Text = pRegistry->ReadString(LASTAPP_DATA);
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
      pRegistry->WriteString(LASTAPPCOMPARATOR_DATA, edtBatchExecutableComparatorName->Text);
      pRegistry->WriteString(LASTAPP_DATA, edtBatchExecutableName->Text);
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
  GetCompareFilename(Ref, sCompare);
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
  GetCompareFilename(Ref, sCompare);
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
  GetCompareFilename(Ref, sCompare);
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
  GetCompareFilename(Ref, sCompare);
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
  sParameters.sprintf("\"%s\" \"%s\"", GetResultFileName(Ref, sMaster).c_str(), GetCompareFilename(Ref, sCompare).c_str());
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
    remove(GetCompareFilename(Ref.GetFilename(), sCompare).c_str());
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
   std::string  sBuffer, sCompareFilename;
   AnsiString   sCommand, sCurTitle, sText;
   int          iItemIndex=0;
   TDateTime    StartDate;

   //get start time of execution
   StartDate = TDateTime::CurrentDateTime();

   memMessages->Clear();
   lstDisplay->Items->Clear();
   gvParameterResultsInfo.clear();
   gvColumnSortOrder.clear();
   sCurTitle = Application->Title;
   gvColumnSortOrder.resize(lstDisplay->Columns->Count, -1);
   while (iItemIndex < ltvScheduledBatchs->Items->Count) {
        sText.printf("%s [(%d of %d) %s]", sCurTitle.c_str(),
                     iItemIndex + 1, ltvScheduledBatchs->Items->Count,
                     ExtractFileName(ltvScheduledBatchs->Items->Item[iItemIndex]->Caption).c_str());
        Application->Title = sText;
        sBuffer = ltvScheduledBatchs->Items->Item[iItemIndex]->Caption.c_str();
        gvParameterResultsInfo.push_back(ParameterResultsInfo(sBuffer.c_str()));
        if (! FileExists(gvParameterResultsInfo.back().GetFilenameString())) {
          AddList("Parameter File Missing");
          continue;
        }

        //Execute comparator SatScan using the current Parameter file, but set commandline options for version check
        sCommand.printf("\"%s\" \"%s\" -v",
                        edtBatchExecutableComparatorName->Text.c_str(),
                        gvParameterResultsInfo.back().GetFilenameString());
        if (Execute(sCommand.c_str())) {
          Application->ProcessMessages();
          _sleep(2);
          //get filename that will be the result file created for comparison
          GetCompareFilename(gvParameterResultsInfo.back().GetFilename(), sCompareFilename);
          //Execute SatScan using the current Parameter file, but set commandline options for version check
          sCommand.printf("\"%s\" \"%s\" -v -o \"%s\"",
                          edtBatchExecutableName->Text.c_str(),
                          gvParameterResultsInfo.back().GetFilenameString(),
                          sCompareFilename.c_str());
          //execute SaTScan version that is in question
          if (Execute(sCommand.c_str())) {
            CompareClusterInformationFiles();
            CompareLocationInformationFiles();
            CompareRelativeRisksInformationFiles();
            CompareSimulatedRatiosFiles();
            CompareTimes();
            AddList();
          }
          else
            AddList("Program Failed/Cancelled");
        }
        else
          AddList("Comparator Program Failed/Cancelled");
        Application->ProcessMessages();
        _sleep(2);
        iItemIndex++;
   }
   Application->Title = sCurTitle;
   //archive results
   if (gpFrmOptions->chkArchiveResults->Checked)
     ArchiveResults();
   EnableSaveResultsAction();
   //print execution time to message window
   DateSeparator = '/';
   TimeSeparator = ':';
   sCommand.printf("Start time: %s    Stop time: %s", DateTimeToStr(StartDate).c_str(), DateTimeToStr(TDateTime::CurrentDateTime()).c_str());
   memMessages->Lines->Add(sCommand);
   memMessages->SelStart = 0;
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
void TfrmMain::AddList() {
  AnsiString    sDisplay;

  TListItem * pListItem = lstDisplay->Items->Add();
  pListItem->Data = (void*)(gvParameterResultsInfo.size() - 1);

  const ParameterResultsInfo & ref = gvParameterResultsInfo.back();
  if (ref.GetHasMisMatches() || ref.GetTimeDifferenceType() == SLOWER)
    pListItem->ImageIndex = 2;
  else if (ref.GetHasMissingFiles() || ref.GetTimeDifferenceType() == INCOMPLETE)
    pListItem->ImageIndex = 1;
  else
    pListItem->ImageIndex = 0;

  pListItem->SubItems->Add(gvParameterResultsInfo.back().GetFilename().GetCompleteFileName());
  pListItem->SubItems->Add(GetDisplayTime(sDisplay));
  AddSubItemForType(pListItem, gvParameterResultsInfo.back().GetClusterInformationType());
  AddSubItemForType(pListItem, gvParameterResultsInfo.back().GetLocationInformationType());
  AddSubItemForType(pListItem, gvParameterResultsInfo.back().GetRelativeRisksType());
  AddSubItemForType(pListItem, gvParameterResultsInfo.back().GetSimulatedRatiosType());
}

/** adds listview item -- error reporting */
void TfrmMain::AddList(const char * sMessage) {
  AnsiString    sCaption;

  TListItem * pListItem = lstDisplay->Items->Add();
  sCaption.printf("%s <%s>", gvParameterResultsInfo.back().GetFilename().GetCompleteFileName(), sMessage);
  pListItem->Data = (void*)(gvParameterResultsInfo.size() - 1);
  pListItem->ImageIndex = 3;
  pListItem->SubItems->Add(sCaption);
  pListItem->SubItems->Add("-");
  pListItem->SubItems->Add("-");
  pListItem->SubItems->Add("-");
  pListItem->SubItems->Add("-");
  pListItem->SubItems->Add("-");
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
  TDateTime     Date;

  //create archive name
  Date = TDateTime::CurrentDateTime();
  DateSeparator = '_';
  TimeSeparator = '.';
  sArchiveFilename.printf("%s%s.zip", ExtractFilePath(Application->ExeName).c_str(), DateTimeToStr(Date).c_str());
  if (!access(sArchiveFilename.c_str(), 00) && remove(sArchiveFilename.c_str()))
    return;

  //print archive filename
  memMessages->Lines->Add("Archive Filename:");
  memMessages->Lines->Add(sArchiveFilename);
  memMessages->Lines->Add("");
  //create analysis stats file
  sStatsFilename.printf("%s%s.stats.txt", ExtractFilePath(Application->ExeName).c_str(), DateTimeToStr(Date).c_str());
  CreateStatsFile(sStatsFilename.c_str());
  sCommand.sprintf("\"%s\" %s \"%s\" \"%s\"",
                   gpFrmOptions->edtArchiveApplication->Text.c_str(),
                   gpFrmOptions->edtArchiveApplicationOptions->Text.c_str(),
                   sArchiveFilename.c_str(),
                   sStatsFilename.c_str());
  Execute(sCommand, false);
  remove(sStatsFilename.c_str());
  //add 'ReadMe' file to archive
  if (gpFrmOptions->chkCreateReadMeFile->Checked) {
    sStatsFilename.printf("%sReadMe.txt", ExtractFilePath(Application->ExeName).c_str());
    CreateReadMeFile(sStatsFilename.c_str());
    sCommand.sprintf("\"%s\" %s \"%s\" \"%s\"",
                     gpFrmOptions->edtArchiveApplication->Text.c_str(),
                     gpFrmOptions->edtArchiveApplicationOptions->Text.c_str(),
                     sArchiveFilename.c_str(),
                     sStatsFilename.c_str());

    Execute(sCommand, false);
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
    Execute(sCommand, false);

    //add results file
    sCommand.sprintf("\"%s\" %s \"%s\" \"%s\" \"%s\"",
                     gpFrmOptions->edtArchiveApplication->Text.c_str(),
                     gpFrmOptions->edtArchiveApplicationOptions->Text.c_str(),
                     sArchiveFilename.c_str(),
                     GetResultFileName(Ref.GetFilename(), sMaster).c_str(),
                     GetCompareFilename(Ref.GetFilename(), sCompare).c_str());
    if (Execute(sCommand, false) && gpFrmOptions->chkDeleteFileAfterArchiving->Checked) {
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
    if (Execute(sCommand, false) && gpFrmOptions->chkDeleteFileAfterArchiving->Checked) {
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
    if (Execute(sCommand, false) && gpFrmOptions->chkDeleteFileAfterArchiving->Checked) {
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
      if (Execute(sCommand, false) && gpFrmOptions->chkDeleteFileAfterArchiving->Checked) {
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
    if (Execute(sCommand, false) && gpFrmOptions->chkDeleteFileAfterArchiving->Checked) {
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
    edtBatchExecutableComparatorName->Text = OpenDialog->FileName;
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
    edtBatchExecutableName->Text = OpenDialog->FileName;
}

/** Sets open dialog filters and displays for selecting parameter list file *//** compare Cluster Information files */
void TfrmMain::CompareClusterInformationFiles() {
  CompareType     eType;
  std::string     sMaster, sCompare;

  GetResultFileName(gvParameterResultsInfo.back().GetFilename(), sMaster);
  GetCompareFilename(gvParameterResultsInfo.back().GetFilename(), sCompare);

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

  gvParameterResultsInfo.back().SetClusterInformationType(eType);
}

/** compare Location Information files */
void TfrmMain::CompareLocationInformationFiles() {
  CompareType     eType;
  std::string     sMaster, sCompare;

  GetResultFileName(gvParameterResultsInfo.back().GetFilename(), sMaster);
  GetCompareFilename(gvParameterResultsInfo.back().GetFilename(), sCompare);

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

  gvParameterResultsInfo.back().SetLocationInformationType(eType);
}

/** compare Relative Risks files */
void TfrmMain::CompareRelativeRisksInformationFiles() {
  CompareType   eType;
  std::string   sMaster, sCompare, sLineBuffer;
  unsigned int  iPos, iLine=0;

  //not applicable for Space-Time Permutation model
  ZdIniFile IniFile(gvParameterResultsInfo.back().GetFilenameString());
  if (IniFile.GetNumSections())
    sLineBuffer = IniFile.GetSection("[Analysis]")->GetString("ModelType");
  else {
    ifstream ParameterFile(gvParameterResultsInfo.back().GetFilenameString());
    while (++iLine < 22 && std::getline(ParameterFile, sLineBuffer));
    if ((iPos = sLineBuffer.find_first_of("//")) != sLineBuffer.npos)
      sLineBuffer = sLineBuffer.substr(0, iPos);
    sLineBuffer = Trim(sLineBuffer.c_str()).c_str();
  }

  if (sLineBuffer == "2"/* Space-Time Permutation */) {
    eType = NOT_APPLICABLE;
  }
  else {
    GetResultFileName(gvParameterResultsInfo.back().GetFilename(), sMaster);
    GetCompareFilename(gvParameterResultsInfo.back().GetFilename(), sCompare);

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

  gvParameterResultsInfo.back().SetRelativeRisksType(eType);
}

/** compare Simulated Loglikelihood Ratio files */
void TfrmMain::CompareSimulatedRatiosFiles() {
  CompareType     eType;
  std::string     sMaster, sCompare;

  GetResultFileName(gvParameterResultsInfo.back().GetFilename(), sMaster);
  GetCompareFilename(gvParameterResultsInfo.back().GetFilename(), sCompare);

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

  gvParameterResultsInfo.back().SetSimulatedRatiosType(eType);
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
void TfrmMain::CompareTimes() {
  bool                  bMasterDate;
  unsigned short        uHoursM, uMinutesM, uSecondsM, uHoursC, uMinutesC, uSecondsC;
  std::string           sMaster, sCompare;
  float                 fMasterTimeInMinutes, fComareTimeInMinutes;

  GetResultFileName(gvParameterResultsInfo.back().GetFilename(), sMaster);
  bMasterDate = GetRunTime(sMaster.c_str(), uHoursM, uMinutesM, uSecondsM);
  GetCompareFilename(gvParameterResultsInfo.back().GetFilename(), sCompare);
  GetRunTime(sCompare.c_str(), uHoursC, uMinutesC, uSecondsC);

  if (!bMasterDate)
    gvParameterResultsInfo.back().SetTimeDifference(uHoursC, uMinutesC, uSecondsC, INCOMPLETE);
  else {
    fMasterTimeInMinutes = (float)(uHoursM) * 60 + (float)uMinutesM + (float)(uSecondsM)/60;
    fComareTimeInMinutes = (float)(uHoursC) * 60 + (float)uMinutesC + (float)(uSecondsC)/60;

    if (fMasterTimeInMinutes == fComareTimeInMinutes) {
      gvParameterResultsInfo.back().SetTimeDifference(uHoursC, uMinutesC, uSecondsC, SAME);
      gvParameterResultsInfo.back().SetTimeDifferencePercentage(0);
    }
    else if (fMasterTimeInMinutes > fComareTimeInMinutes) {
      gvParameterResultsInfo.back().SetTimeDifferencePercentage(1 - fComareTimeInMinutes/fMasterTimeInMinutes);
      fMasterTimeInMinutes = fMasterTimeInMinutes - fComareTimeInMinutes;
      uHoursM = fMasterTimeInMinutes/60;
      uMinutesM = fMasterTimeInMinutes - uHoursM * 60;
      uSecondsM = (float)(fMasterTimeInMinutes - uHoursM * 60 - uMinutesM) * 60;
      gvParameterResultsInfo.back().SetTimeDifference(uHoursM, uMinutesM, uSecondsM, FASTER);
    }
    else {
      gvParameterResultsInfo.back().SetTimeDifferencePercentage(1 - fMasterTimeInMinutes/fComareTimeInMinutes);
      fComareTimeInMinutes = fComareTimeInMinutes - fMasterTimeInMinutes;
      uHoursC = fComareTimeInMinutes/60;
      uMinutesC = fComareTimeInMinutes - uHoursC * 60;
      uSecondsC = (float)(fComareTimeInMinutes - uHoursC * 60 - uMinutesC) * 60;
      gvParameterResultsInfo.back().SetTimeDifference(uHoursC, uMinutesC, uSecondsC, SLOWER);
    }
  }
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
 TDateTime      Date;

  if (sFilename) {
    ofstream Output(sFilename, ios_base::out|ios_base::trunc);

    //print header information
    Date = TDateTime::CurrentDateTime();
    Output << endl << "Date: " << Date.DateTimeString().c_str() << endl;

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
  ActionStart->Enabled = edtBatchExecutableName->Text.Length() &&
                         edtBatchExecutableComparatorName->Text.Length() &&
                         ltvScheduledBatchs->Items->Count;
}

/** enables view parameters action */
void TfrmMain::EnableViewAction() {
  ActionViewParameters->Enabled = ltvScheduledBatchs->SelCount;
}

/** creates process to execute command */
bool TfrmMain::Execute(const AnsiString & sCommandLine, bool bWindowed) {
   STARTUPINFO          si;
   PROCESS_INFORMATION  pi;
   unsigned long        lProcessTerminationStatus=-1;

   //initialize structs
   ZeroMemory(&si, sizeof(si));
   si.cb = sizeof(si);
   ZeroMemory(&pi,sizeof(pi));
   //Start the child process.
   if(!CreateProcess(NULL,                                         //No module name (use command line).
                     const_cast<char*>(sCommandLine.c_str()),      //Command line.
                     NULL,                                         //Process handle not inheritable.
                     NULL,                                         //Thread handle not inheritable.
                     FALSE,                                        //Set handle inheritance to FALSE.
                     (bWindowed ? CREATE_NEW_CONSOLE : CREATE_NO_WINDOW), //No creation flags.
                     NULL,                                         //Use parent's environment block.
                     NULL,                                         //Use parent's starting directory.
                     &si,                                          //Pointer to STARTUPINFO structure.
                     &pi))                                       //Pointer to PROCESS_INFORMATION structure.
     {
      memMessages->Lines->Add("Process create failed!");
      memMessages->Lines->Add(sCommandLine.c_str());
      memMessages->Lines->Add("");
     }

   //Wait until child process exits.
   WaitForSingleObject(pi.hProcess,INFINITE);
   GetExitCodeProcess(pi.hProcess, &lProcessTerminationStatus);

   //Close process and thread handles.
   CloseHandle(pi.hProcess);
   CloseHandle(pi.hThread);

   return (lProcessTerminationStatus == 0 ? true : false);
}
/** Gets filename of file that will be the alternate results to compare to original */
std::string & TfrmMain::GetCompareFilename(const ZdFileName & ParameterFilename, std::string & sResultFilename) {
  //defined alternate results filename -- this will be the new file we will compare against
  sResultFilename = ParameterFilename.GetLocation();
  sResultFilename += ParameterFilename.GetFileName();
  sResultFilename += COMPARE_FILE_EXTENSION;

  return sResultFilename;
}

/** adds analysis speed comparison to display */
AnsiString & TfrmMain::GetDisplayTime(AnsiString & sDisplay) {
  const ParameterResultsInfo & Ref = gvParameterResultsInfo.back();
  switch (Ref.GetTimeDifferenceType()) {
    case INCOMPLETE : sDisplay.printf("%i hr %i min % i sec - ?", Ref.GetHoursDifferent(),
                                      Ref.GetMinutesDifferent(), Ref.GetSecondsDifferent());
                      break;
    case SLOWER     : sDisplay.printf("%i hr %i min % i sec slower (%.2f)", Ref.GetHoursDifferent(),
                                      Ref.GetMinutesDifferent(), Ref.GetSecondsDifferent(),
                                      Ref.GetTimeDifferencePercentage());
                      break;
    case SAME       : sDisplay.printf("%i hr %i min % i sec - same time", Ref.GetHoursDifferent(),
                                      Ref.GetMinutesDifferent(), Ref.GetSecondsDifferent());
                      break;
    case FASTER     : sDisplay.printf("%i hr %i min % i sec faster (%.2f)", Ref.GetHoursDifferent(),
                                      Ref.GetMinutesDifferent(), Ref.GetSecondsDifferent(),
                                      Ref.GetTimeDifferencePercentage());
                      break;
    //default : error
  };
  return sDisplay;
}

/** Sets result filename as specified by parameter file. If path is missing from
    filename, path of parameter file is inserted as path of result file. */
std::string & TfrmMain::GetResultFileName(const ZdFileName & ParameterFilename, std::string & sResultFilename) {
  int   count=0;

  //Determine the location of master results file
  ZdIniFile IniFile(ParameterFilename.GetFullPath());
  if (IniFile.GetNumSections())
    sResultFilename = IniFile.GetSection("[Output Files]")->GetString("ResultsFile");
  else {
    //open parameter file and scan to the 6th line, which is the results filename
    ifstream ParameterFile(ParameterFilename.GetFullPath(), ios::in);
    while (++count < 7)
         std::getline(ParameterFile, sResultFilename);
  }

  //complete path if only filename specified
  if (sResultFilename.find("\\") == sResultFilename.npos)
    sResultFilename.insert(0, ParameterFilename.GetLocation());

  return sResultFilename;
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


//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "Main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmMain *frmMain;

const char * TfrmMain::SCU_REGISTRY_KEY         = "\\Software\\SaTScanCompareUtility";
const char * TfrmMain::LASTAPP_DATA             = "LastOpenExe";
const char * TfrmMain::LASTPARAMLIST_DATA       = "LastParamList";
const char * TfrmMain::COMPARE_APP_DATA         = "CompareProgram";
const char * TfrmMain::COMPARE_FILE_EXTENSION   = ".out.compare.txt";

/** constructor */
__fastcall TfrmMain::TfrmMain(TComponent* Owner) : TForm(Owner) {
  TRegistry * pRegistry = new TRegistry;
  int iSize;

  if (pRegistry->OpenKey(SCU_REGISTRY_KEY, true)) {
    edtBatchExecutableName->Text = pRegistry->ReadString(LASTAPP_DATA);
    edtParameterListFile->Text = pRegistry->ReadString(LASTPARAMLIST_DATA);
    iSize = pRegistry->GetDataSize(COMPARE_APP_DATA);
    if (iSize > -1) {
      gsComparisonProgram.resize(iSize);
      pRegistry->ReadBinaryData(COMPARE_APP_DATA, (void*)(gsComparisonProgram.c_str()), gsComparisonProgram.size());
    }
    ZdDecrypt(const_cast<char*>(gsComparisonProgram.c_str()), gsComparisonProgram.size(), "24601");
    pRegistry->CloseKey();
  }
  delete pRegistry;
  gvColumnSortOrder.resize(lstDisplay->Columns->Count, -1);
  EnableSaveResultsAction();
  EnableCompareActions();
}

/** destructor */
__fastcall TfrmMain::~TfrmMain() {
  try {
    TRegistry * pRegistry = new TRegistry;

    if (pRegistry->OpenKey(SCU_REGISTRY_KEY, true)) {
      pRegistry->WriteString(LASTAPP_DATA, edtBatchExecutableName->Text);
      pRegistry->WriteString(LASTPARAMLIST_DATA, edtParameterListFile->Text);
      ZdEncrypt(const_cast<char*>(gsComparisonProgram.c_str()), gsComparisonProgram.size(), "24601");
      pRegistry->WriteBinaryData(COMPARE_APP_DATA, (void*)(gsComparisonProgram.c_str()), gsComparisonProgram.size());
      pRegistry->CloseKey();
    }
    delete pRegistry;
  }
  catch (...){}
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
  HINSTANCE hInst = ShellExecute(NULL, "open", gsComparisonProgram.c_str(), sParameters.c_str(), 0, 0);
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
  HINSTANCE hInst = ShellExecute(NULL, "open", gsComparisonProgram.c_str(), sParameters.c_str(), 0, 0);
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
  HINSTANCE hInst = ShellExecute(NULL, "open", gsComparisonProgram.c_str(), sParameters.c_str(), 0, 0);
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
  HINSTANCE hInst = ShellExecute(NULL, "open", gsComparisonProgram.c_str(), sParameters.c_str(), 0, 0);
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
  HINSTANCE hInst = ShellExecute(NULL, "open", gsComparisonProgram.c_str(), sParameters.c_str(), 0, 0);
  if ((int)hInst <= 32)
    Application->MessageBox("Unable to launch comparison program.", "Error", MB_OK);
}

/** saves results of comparison to file */
void __fastcall TfrmMain::ActionSaveComparisonStatsExecute(TObject *Sender) {
 int            i, iNumMisMatch=0,iNumCorrect=0, iNumMissingFiles=0, iNumProgramStop=0;
 TListItem    * pListItem;
 TDateTime      Date;

  if (SaveDialog->Execute()) {
    ofstream Output(SaveDialog->FileName.c_str());

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
              << "                    Parameter Filename   : " << pListItem->Caption.c_str() << endl
              << "                    Cluster Information  : " << pListItem->SubItems->Strings[0].c_str() << endl
              << "                    Location Information : " << pListItem->SubItems->Strings[1].c_str() << endl
              << "                    Relative Risks       : " << pListItem->SubItems->Strings[2].c_str() << endl
              << "                    Simulated Ratios     : " << pListItem->SubItems->Strings[3].c_str() << endl << endl;
    }
  }
}

/** starts process of comparing output files */
void __fastcall TfrmMain::ActionStartExecute(TObject *Sender) {
   std::string  sBuffer, sCompareFilename;
   AnsiString   sCommand;
   ifstream     ParameterListFile;

   //open the file that contains names and parameter files
   ParameterListFile.open((edtParameterListFile->Text).c_str(), ios::in);
   if (!ParameterListFile) {
      Application->MessageBoxA("The Parameter List file cannot be opened.", "ERROR", MB_OK);
      return;
   }

   lstDisplay->Items->Clear();
   gvParameterResultsInfo.clear();
   gvColumnSortOrder.clear();
   gvColumnSortOrder.resize(lstDisplay->Columns->Count, -1);
   while (ParameterListFile.eof()==false) {
        ParameterListFile >> ws;
        std::getline(ParameterListFile, sBuffer);
        gvParameterResultsInfo.push_back(ParameterResultsInfo(sBuffer.c_str()));
        if (! FileExists(gvParameterResultsInfo.back().GetFilenameString())) {
          AddList("Parameter File Missing");
          continue;
        }

        //get filename that will be the result file created for comparison
        GetCompareFilename(gvParameterResultsInfo.back().GetFilename(), sCompareFilename);
        //Execute SatScan using the current Parameter file, but set commandline options for version check
        sCommand.printf("\"%s\" \"%s\" -v -o \"%s\"", edtBatchExecutableName->Text.c_str(),
                        gvParameterResultsInfo.back().GetFilenameString(), sCompareFilename.c_str());
        //execute analysis
        if (Execute(sCommand.c_str())) {
          CompareClusterInformationFiles();
          CompareLocationInformationFiles();
          CompareRelativeRisksInformationFiles();
          CompareSimulatedRatiosFiles();
          AddList();
        }
        else
          AddList("Program Failed/Cancelled");

        Application->ProcessMessages();
        ParameterListFile >> ws;
   }
   ParameterListFile.close();
   EnableSaveResultsAction();
}

/** add item to display indicating results of analysis comparison */
void TfrmMain::AddList() {

  TListItem * pListItem = lstDisplay->Items->Add();
  pListItem->Data = (void*)(gvParameterResultsInfo.size() - 1);

  if (gvParameterResultsInfo.back().GetHasMisMatches())
    pListItem->ImageIndex = 2;
  else if (gvParameterResultsInfo.back().GetHasMissingFiles())
    pListItem->ImageIndex = 1;
  else
    pListItem->ImageIndex = 0;

  pListItem->SubItems->Add(gvParameterResultsInfo.back().GetFilename().GetCompleteFileName());
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

/** Sets open dialog filters and displays for selecting executable. */
void __fastcall TfrmMain::btnBrowseBatchExecutableClick(TObject *Sender) {
  OpenDialog->FileName =  "";
  OpenDialog->DefaultExt = "*.exe";
  OpenDialog->Filter = "Executable files (*.exe)|*.exe|All files (*.*)|*.*";
  OpenDialog->FilterIndex = 0;
  OpenDialog->Title = "Select SaTScan Batch Executable";
  if (OpenDialog->Execute())
    edtBatchExecutableName->Text = OpenDialog->FileName;
}

/** Sets open dialog filters and displays for selecting parameter list file */
void __fastcall TfrmMain::btnBrowseParametersListFileClick(TObject *Sender) {
  OpenDialog->FileName =  "";
  OpenDialog->DefaultExt = "*.prml";
  OpenDialog->Filter = "Parameters List files (*.prml)|*.prml|Text files (*.txt)|*.txt|All files (*.*)|*.*";
  OpenDialog->FilterIndex = 0;
  OpenDialog->Title = "Select Parameters List File";
  if (OpenDialog->Execute())
    edtParameterListFile->Text = OpenDialog->FileName;
}

/** compare Cluster Information files */
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

/** enables save results action */
void TfrmMain::EnableSaveResultsAction() {
  ActionSaveComparisonStats->Enabled = lstDisplay->Items->Count;
}

/** enables start action */
void TfrmMain::EnableStartAction() {
  ActionStart->Enabled = edtBatchExecutableName->Text.Length() && edtParameterListFile->Text.Length();
}

/** creates process to execute command */
bool TfrmMain::Execute(const AnsiString & sCommandLine) {
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
                     /*(chkShowDosWindow->Checked ?*/ CREATE_NEW_CONSOLE /*: CREATE_NO_WINDOW)*/, //No creation flags.
                     NULL,                                         //Use parent's environment block.
                     NULL,                                         //Use parent's starting directory.
                     &si,                                          //Pointer to STARTUPINFO structure.
                     &pi))                                       //Pointer to PROCESS_INFORMATION structure.
     {
      Application->MessageBox(sCommandLine.c_str(), "Process create failed!", MB_OK);
     }

   //Wait until child process exits.
   WaitForSingleObject(pi.hProcess,INFINITE);
   GetExitCodeProcess(pi.hProcess, &lProcessTerminationStatus);

   //Close process and thread handles.
   CloseHandle(pi.hProcess);
   CloseHandle(pi.hThread);

   return (lProcessTerminationStatus == 0 ? true : false);
}

void __fastcall TfrmMain::FormShow(TObject *Sender) {
  lstDisplay->SetFocus();
}

/** Gets filename of file that will be the alternate results to compare to original */
std::string & TfrmMain::GetCompareFilename(const ZdFileName & ParameterFilename, std::string & sResultFilename) {
  //defined alternate results filename -- this will be the new file we will compare against
  sResultFilename = ParameterFilename.GetLocation();
  sResultFilename += ParameterFilename.GetFileName();
  sResultFilename += COMPARE_FILE_EXTENSION;

  return sResultFilename;
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

/** displays open dialog when compare program not set */
bool TfrmMain::PromptForCompareProgram() {
  bool  bReturn=true;

  if (gsComparisonProgram.empty()) {
    OpenDialog->FileName =  "";
    OpenDialog->DefaultExt = "*.exe";
    OpenDialog->Filter = "Executable files (*.exe)|*.exe|All files (*.*)|*.*";
    OpenDialog->FilterIndex = 0;
    OpenDialog->Title = "Select Comparison Program";
    if (OpenDialog->Execute())
      gsComparisonProgram = OpenDialog->FileName.c_str();
    else
      bReturn = false;
  }
  return bReturn;
}


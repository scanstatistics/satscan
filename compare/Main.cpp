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
  EnableSaveResultsAction();
  EnableCompareResultFilesAction();
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

/** launches compare program for viewing differences in output files */
void __fastcall TfrmMain::ActionCompareResultFilesExecute(TObject *Sender) {
  std::string   sOriginal, sCandidate;
  AnsiString    sParameters;

  if (!lstDisplay->Selected)
    return;

  if (gsComparisonProgram.empty()) {
    OpenDialog->FileName =  "";
    OpenDialog->DefaultExt = "*.exe";
    OpenDialog->Filter = "Executable files (*.exe)|*.exe|All files (*.*)|*.*";
    OpenDialog->FilterIndex = 0;
    OpenDialog->Title = "Select Comparison Program";
    if (OpenDialog->Execute())
      gsComparisonProgram = OpenDialog->FileName.c_str();
    else
      return;
  }

  //launch comparison program
  ZdFileName & ref = gvParameterList[lstDisplay->Selected->Index];
  sParameters.sprintf("\"%s\" \"%s\"",
                      GetResultFileName(ref, sOriginal).c_str(), GetCompareFilename(ref, sCandidate).c_str());

  HINSTANCE hInst = ShellExecute(NULL, "open", gsComparisonProgram.c_str(), sParameters.c_str(), 0, 0);
  if ((int)hInst <= 32)
    Application->MessageBox("Unable to launch comparison program.", "Error", MB_OK);

}

/** saves results of comparison to file */
void __fastcall TfrmMain::ActionSaveResultsExecute(TObject *Sender) {
 int            i, iNumMisMatch=0;
 TListItem    * pListItem;
 TDateTime      Date;

 if (SaveDialog->Execute()) {
    ofstream Output(SaveDialog->FileName.c_str());

    //print header information
    Date = TDateTime::CurrentDateTime();
    Output << endl << "Date: " << Date.DateTimeString().c_str() << endl;

    for (i=0; i < lstDisplay->Items->Count; i++)
       if (lstDisplay->Items->Item[i]->ImageIndex == 0)
          iNumMisMatch++;

    Output << "Stats: "<< lstDisplay->Items->Count - iNumMisMatch << " of "
           << lstDisplay->Items->Count << " correct" << endl << endl;

    //print results of comparisons
    for (i=0; i < lstDisplay->Items->Count; i++) {
       pListItem = lstDisplay->Items->Item[i];
       Output << "Parameter List Item " << i + 1 << ")" << endl << "   Parameter Filename: "
              << pListItem->Caption.c_str() << endl << "   Comparison Results: "
              << pListItem->SubItems->Strings[0].c_str() << endl << endl;
    }
  }
}

/** starts process of comparing output files */
void __fastcall TfrmMain::ActionStartExecute(TObject *Sender) {
   std::string  sBuffer, sCorrectOutputFile, sCompareFilename;
   AnsiString   sCommand;
   ifstream     ParameterListFile;
   bool         bResultsIdentical;

   //open the file that contains names and parameter files
   ParameterListFile.open((edtParameterListFile->Text).c_str(), ios::in);
   if (!ParameterListFile) {
      Application->MessageBoxA("The Parameter List file cannot be opened.", "ERROR", MB_OK);
      return;
   }

   lstDisplay->Items->Clear();
   gvParameterList.clear();
   while (ParameterListFile.eof()==false) {
        ParameterListFile >> ws;
        std::getline(ParameterListFile, sBuffer);
        gvParameterList.push_back(ZdFileName(sBuffer.c_str()));
        if (! FileExists(gvParameterList.back().GetFullPath())) {
          AddList(gvParameterList.back().GetCompleteFileName(), "Parameter file does not exist.", true);
          continue;
        }
        //get result filename that parameter file specifies
        GetResultFileName(gvParameterList.back(), sCorrectOutputFile);
        if (access(sCorrectOutputFile.c_str(), 00)) {
          AddList(gvParameterList.back().GetCompleteFileName(), "Original output file missing, nothing to compare against.", true);
          continue;
        }
        //get filename that will be the result file created for comparison
        GetCompareFilename(gvParameterList.back(), sCompareFilename);
        //Execute SatScan using the current Parameter file, but set commandline options for version check
        sCommand.printf("\"%s\" \"%s\" -v -o \"%s\"", edtBatchExecutableName->Text.c_str(),
                        gvParameterList.back().GetFullPath(), sCompareFilename.c_str());
        //execute analysis
        if (Execute(sCommand.c_str())) {
          bResultsIdentical = CompareResultFiles(sCorrectOutputFile, sCompareFilename);
          AddList(gvParameterList.back().GetCompleteFileName(),
                  (bResultsIdentical ? "Identical Results" : "Results do not agree!"),
                  (bResultsIdentical ? false : true));
        }
        else
          AddList(gvParameterList.back().GetCompleteFileName(), "Program Failed", true);

        Application->ProcessMessages();
        ParameterListFile >> ws;
   }
   ParameterListFile.close();
   EnableSaveResultsAction();
}

/** adds new listview item */
void TfrmMain::AddList(const char * sParameterFilename, const char * sMessage, bool bError) {
  TListItem * pListItem = lstDisplay->Items->Add();
  pListItem->Caption = sParameterFilename;
  pListItem->ImageIndex = (bError ? 0 : 1);
  pListItem->SubItems->Add(sMessage);
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

/** returns whether files are equal, ignoring whitespace differences */
bool TfrmMain::CompareResultFiles(std::string & sCorrectFile, std::string & sFileToValidate) {
  ifstream correctOutput(sCorrectFile.c_str(), ios::in), output(sFileToValidate.c_str(), ios::in);
  string correctStr, Str;
  bool bEqual=true;

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
void TfrmMain::EnableCompareResultFilesAction() {
  ActionCompareResultFiles->Enabled = lstDisplay->SelCount;
}

/** enables save results action */
void TfrmMain::EnableSaveResultsAction() {
  ActionSaveResults->Enabled = lstDisplay->Items->Count;
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
    //open parameter file and scan to the 7th line, which is the results filename
    ifstream ParameterFile(ParameterFilename.GetFullPath(), ios::in);
    while (++count < 7)
         std::getline(ParameterFile, sResultFilename);
  }

  //complete path if only filename specified
  if (sResultFilename.find("\\") == sResultFilename.npos)
    sResultFilename.insert(0, ParameterFilename.GetLocation());

  return sResultFilename;
}
void __fastcall TfrmMain::lstDisplaySelectItem(TObject *Sender, TListItem *Item, bool Selected) {
  EnableCompareResultFilesAction();
}


//---------------------------------------------------------------------------
#include "stsSaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
#include "stsOutputFileRegistry.h"
#include "stsFrmStartWindow.h"
#include "stsFrmUpdateCheck.h"
#include "stsFrmDownloadProgress.h"
TfrmMainForm *frmMainForm;

/** contructor */
__fastcall TfrmMainForm::TfrmMainForm(TComponent* Owner): TForm(Owner){
  try {
    Init();
    Setup();
  }
  catch (ZdException & x) {
    x.AddCallpath("constructor()", "TfrmMainForm");
    throw;
  }
}

/** destructor */
__fastcall TfrmMainForm::~TfrmMainForm() {}

/** about action event -- shows aboutbox */
void __fastcall TfrmMainForm::AboutSatscanActionExecute(TObject *Sender) {
  TfrmAbout(this).ShowModal();
}

/** event triggiered when associated advanced features action clicked */
void __fastcall TfrmMainForm::AdvancedParametersActionExecute(TObject *Sender) {
  try {
    TfrmAnalysis * frmAnalysisForm = dynamic_cast<TfrmAnalysis *>(frmMainForm->ActiveMDIChild);
    if (frmAnalysisForm)
      frmAnalysisForm->ShowAdvancedFeaturesDialog();
  }
  catch (ZdException &x) {
    x.AddCallpath("AdvancedParametersActionExecute()","TfrmMainForm");
    DisplayBasisException(this, x);
  }
}

/** close child window action event - triggers active child window's close event */
void __fastcall TfrmMainForm::CloseSessionActionExecute(TObject *Sender) {
  if (ActiveMDIChild)
    ActiveMDIChild->Close();
}

/** enables/disables the appropraite buttons and controls for main window */
void TfrmMainForm::EnableActions(bool bEnable) {
  for (int i=0; i < ActionList->ActionCount; ++i) {
     TAction* pAction = dynamic_cast<TAction*>(ActionList->Actions[i]);
     if (pAction) {
       if (pAction->Category == "All")
         pAction->Enabled = bEnable;
       else if(pAction->Category == "AnalysisRun" || pAction->Category == "Analysis")
         pAction->Enabled = !bEnable;
     }
  }
}

/** execution action event --  shows analysis progress/results window and starts analysis */
void __fastcall TfrmMainForm::ExecuteActionExecute(TObject *Sender) {
  TfrmAnalysis                * frmAnalysis;
  CParameters                 * pSession;
  AnsiString                    sCaption;
  TfrmAnalysisRun             * pAnalysisRun=0;
  CalcThread                  * pThread;
  std::string                   sOutputFileName;

  try {
    //make sure window is a session window
    frmAnalysis = dynamic_cast<TfrmAnalysis *>(frmMainForm->ActiveMDIChild);
    if (frmAnalysis && frmAnalysis->ValidateParams()) {
      pSession = frmAnalysis->GetSession();
      sOutputFileName = pSession->GetOutputFileName();
      // if we don't already have a thread with the result file running then launch one
      if (gRegistry.IsAlreadyRegistered(sOutputFileName))
        ZdException::GenerateNotification("The filename you specified for results file is \ncurrently being written to by another analysis.\n"
                                          "Please choose another results filename." , "Error!");

      sCaption.printf("Running %s", (frmAnalysis->GetFileName() ? frmAnalysis->GetFileName() : "Session"));
      pAnalysisRun = new TfrmAnalysisRun(this, *pSession, sOutputFileName, gRegistry, ActionList);
      pAnalysisRun->Caption = sCaption;
      gRegistry.Register(sOutputFileName);
      pAnalysisRun->LaunchThread();
    }
  }
  catch (ZdException & x) {
    gRegistry.Register(sOutputFileName);
    delete pAnalysisRun;
    x.AddCallpath("ExecuteActionExecute()","TfrmMainForm");
    DisplayBasisException(this, x);
  }
}

/** close form action event -- triggers main form close event */
void __fastcall TfrmMainForm::ExitActionExecute(TObject *Sender) {
  Close();
}

/** closes all child windows -- this method is primarily used when closing the
    application and the user has indicated to close regardless of executing analyses. */
void TfrmMainForm::ForceClose() {
  stsBaseAnalysisChildForm   * pBaseForm;
  int                          i;

  try {
    for (i=0; i < MDIChildCount; ++i) {
       pBaseForm = dynamic_cast<stsBaseAnalysisChildForm*>(MDIChildren[i]);
       if (pBaseForm)
         pBaseForm->CloseForm(true);
       else
         MDIChildren[i]->Close();
    }
  }
  catch (...){}
}

/** returns whether there are actively running analyses */
bool TfrmMainForm::GetAnalysesRunning() {
  int                           i;
  TfrmAnalysisRun             * pAnalysisRun;
  bool                          bReturn(false);

  //loop through to find if there are running analyses
  for (i=0; i < MDIChildCount && !bReturn; ++i) {
     pAnalysisRun = dynamic_cast<TfrmAnalysisRun*>(MDIChildren[i]);
     bReturn = (pAnalysisRun && !pAnalysisRun->GetCanClose());
  }
  return bReturn;
}

/** help action event -- launches html version of user guide */
void __fastcall TfrmMainForm::HelpActionExecute(TObject *Sender) {
  HINSTANCE     hReturn;
  ZdString      sMessage;

  //Attempt to open chm user guide.
  hReturn = ShellExecute(Handle, "open", "SaTScan_Help.chm", NULL, NULL, SW_SHOWNORMAL);
  if (hReturn <= (void*)32) {
    if (hReturn == (void*)SE_ERR_NOASSOC) {
      sMessage  << "SaTScan Help was unable to open. Please note that SaTScan Help "
                   "requires Internet Explorer 4.0 or later installed."
                   "\nPlease contact technical support at website: "
                << GetToolkit().GetWebSite() << " for more information.";
    }
    else {
      sMessage  << "SaTScan Help was unable to open. Help file may be missing or corrupt."
                   "\nPlease contact technical support at website: "
                << GetToolkit().GetWebSite() << ".";
    }
    Application->MessageBox(sMessage.GetCString(), "SaTScan Help", MB_OK);
  }
}

/** import action event -- launches data file importer dialog */
void __fastcall TfrmMainForm::ImportActionExecute(TObject *Sender) {
  try {
    TfrmAnalysis * frmAnalysisForm = dynamic_cast<TfrmAnalysis *>(frmMainForm->ActiveMDIChild);
    if (frmAnalysisForm)
      frmAnalysisForm->LaunchImporter();
  }
  catch (ZdException &x) {
    x.AddCallpath("ImportActionExecute()","TfrmMainForm");
    DisplayBasisException(this, x);
  }
}

/** new session action event -- creates new parameters/session child window */
void __fastcall TfrmMainForm::NewSessionActionExecute(TObject *Sender) {
  try {
    new TfrmAnalysis(this, ActionList);
  }
  catch (ZdException &x) {
    x.AddCallpath("NewSessionActionExecute","TfrmMainForm");
    DisplayBasisException(this, x);
  }
}

/** form activate event -- if first time showing, shows start window dialog */
void __fastcall TfrmMainForm::OnFormActivate(TObject *Sender) {
  try {
    if (gbShowStartWindow) {
      TfrmStartWindow * frmStartWindow = new TfrmStartWindow(this);
      frmStartWindow->ShowModal();
      switch (frmStartWindow->GetOpenType()) {
        case TfrmStartWindow::NEW    : new TfrmAnalysis(this, ActionList); break;
        case TfrmStartWindow::SAVED  : OpenAFile(); break;
        case TfrmStartWindow::LAST   : if (!File_Exists(const_cast<char*>(GetToolkit().GetParameterHistory()[0].c_str())))
                                         ZdException::GenerateNotification("Cannot open file '%s'.", "FormActivate()",
                                                                           GetToolkit().GetParameterHistory()[0].c_str());
                                       new TfrmAnalysis(this, ActionList, const_cast<char*>(GetToolkit().GetParameterHistory()[0].c_str()));
                                       break;
        case TfrmStartWindow::CANCEL : break;
        default : ZdException::GenerateNotification("Unknown operation index % d.",
                                                     "OnFormActivate()", frmStartWindow->GetOpenType());
      }
      delete frmStartWindow; frmStartWindow=0;
      gbShowStartWindow = false;
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("OnFormActivate","TfrmMainForm");
    DisplayBasisException(this, x);
  }
}

/** form close event -- checks whether there are actively running analyses and
    prompts user as to whether to continue closing accordingly. The ForceClose()
    method is used to ensure that all child windows will close. */
void __fastcall TfrmMainForm::OnFormClose(TObject *Sender, TCloseAction &Action) {
  try {
    if (GetAnalysesRunning() &&
        TBMessageBox::Response(this, "Warning", "There are analyses currently executing. "
                                                "Are you sure you want to exit SaTScan?", XBMB_YES|XBMB_CANCEL) == mrCancel)
      Action = caNone;
    else
      ForceClose();
  }
  catch (...){}
}

/** launches browse dialog to select existing parameter file - creates new
    session child window */
void TfrmMainForm::OpenAFile(){
  try {
    OpenDialog1->FileName = "";
    OpenDialog1->DefaultExt = "*.prm";
    OpenDialog1->Filter = "Parameter Files (*.prm)|*.prm|Text Files (*.txt)|*.txt|All Files (*.*)|*.*";   
    OpenDialog1->Title = "Select Parameter File";
    if (OpenDialog1->Execute())
      new TfrmAnalysis(this, ActionList, OpenDialog1->FileName.c_str());
  }
  catch (ZdException &x) {
    x.AddCallpath("OpenAFile()","TfrmMainForm");
    throw;
  }
}

/** open parameter file action event -- calls OpenAFile() */
void __fastcall TfrmMainForm::OpenParameterFileActionExecute(TObject *Sender) {
  try {
    OpenAFile();
  }
  catch (ZdException &x) {
    x.AddCallpath("OpenParameterFileActionExecute","TfrmMainForm");
    DisplayBasisException(this, x);
  }
}

/** print action event -- not defined currently, printing selected in analysis window */
void __fastcall TfrmMainForm::PrintSessionActionExecute(TObject *Sender) {
  //try {
  //}
  //catch (ZdException &x) {
  //  x.AddCallpath("PrintSessionActionExecute","TfrmMainForm");
  //  DisplayBasisException(this, x);
  //}
}

/** print setup action event -- launches print setup dialog */
void __fastcall TfrmMainForm::PrintSetupActionExecute(TObject *Sender) {
  PrinterSetupDialog1->Execute();
}

/** refreshes 'reopen' menu item to reflect possibly updated history list */
void TfrmMainForm::RefreshOpenList() {
  SaTScanToolkit::ParameterHistory_t::const_iterator     itr;
  TMenuItem                                            * pItem=0;
  size_t                                                 t=1;

  mitReopen->Clear();
  const SaTScanToolkit::ParameterHistory_t & Parameters = GetToolkit().GetParameterHistory();
  mitReopen->Enabled = !Parameters.empty();
  for (t=0; t < Parameters.size(); t++) {
     pItem = new TMenuItem(mitReopen);
     pItem->OnClick = ReopenAction->OnExecute;
     pItem->Caption = MinimizeName(Parameters[t].c_str(), Canvas, std::max(Screen->Width/2, 200));
     pItem->Tag = t;
     mitReopen->Add(pItem);
  }
}

/** re-open action event -- attempts to open selected menu item in new
    session/parameters settings child window */
void __fastcall TfrmMainForm::ReopenActionExecute(TObject *Sender) {
  TMenuItem     * pItem;
  std::string     sFileName;

  try {
    pItem = dynamic_cast<TMenuItem *>(Sender);
    if (pItem) {
      sFileName = GetToolkit().GetParameterHistory()[pItem->Tag];
      if (!File_Exists(const_cast<char*>(sFileName.c_str())))
        ZdException::GenerateNotification("Cannot open file '%s'.","ReopenActionExecute()", sFileName.c_str());
      new TfrmAnalysis(this, ActionList, const_cast<char*>(sFileName.c_str()));
    }  
  }
  catch (ZdException &x) {
    x.AddCallpath("ReopenActionExecute","TfrmMainForm");
    DisplayBasisException(this, x);
  }
}

/** - if parameter filename already known, re-writes parameters to file if permissions permit
    - else calls TfrmAnalysis::SaveAs() method */
void TfrmMainForm::Save() {
  TfrmAnalysis *frmBaseForm;

  try {
    frmBaseForm = dynamic_cast<TfrmAnalysis *>(frmMainForm->ActiveMDIChild);
    if (frmBaseForm)
      frmBaseForm->WriteSession();
  }
  catch (ZdException &x) {
    x.AddCallpath("Save()","TfrmMainForm");
    throw;
  }
}

/** calls TfrmAnalysis::SaveAs() method */
void TfrmMainForm::SaveAs() {
  TfrmAnalysis *frmBaseForm;

  try {
    frmBaseForm = dynamic_cast<TfrmAnalysis *>(frmMainForm->ActiveMDIChild);
    if (frmBaseForm)
      frmBaseForm->SaveAs();
  }
  catch (ZdException & x) {
    x.AddCallpath("SaveAs()", "TfrmMainForm");
    throw;
   }
}

/** save action event -- calls Save() method */
void __fastcall TfrmMainForm::SaveSessionActionExecute(TObject *Sender) {
  try {
    Save();
  }
  catch (ZdException & x) {
    x.AddCallpath("SaveSessionActionExecute", "TfrmMainForm");
    DisplayBasisException(this, x);
  }
}

/** save as action event -- calls SaveAs() method */
void __fastcall TfrmMainForm::SaveSessionAsActionExecute(TObject *Sender){
  try {
    SaveAs();
  }
  catch (ZdException & x) {
    x.AddCallpath("SaveSessionAsActionExecute", "TfrmMainForm");
    DisplayBasisException(this, x);
  }
}

/** internal setup function */
void TfrmMainForm::Setup() {
  try {
    EnableActions(true);
    RefreshOpenList();
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup","TfrmMainForm");
    throw;
  }
}

/** version update action event -- Contacts webserver to determine if current
    application version is older than latest update. Prompts user as to whether
    to install update and attempts to download, close application, and launch
    application updater. */
void __fastcall TfrmMainForm::UpdateActionExecute(TObject *Sender) {
  TfrmUpdateCheck             * frmUpdateCheck=0;
  TfrmDownloadProgress        * frmDownloadProgress=0;
  ZdString                      sMessage;

  try {
    frmUpdateCheck = new TfrmUpdateCheck(this);
    frmUpdateCheck->ConnectToServerForUpdateCheck();
    if (frmUpdateCheck->HasUpdates()) {
      sMessage.printf("SaTScan v%s is available. Do you want to install now?", frmUpdateCheck->GetUpdateVersion().GetCString());
      if (TBMessageBox::Response(this, "SaTScan Update Available", sMessage.GetCString(), MB_YESNO) == IDYES) {
        if (GetAnalysesRunning()) {
            TBMessageBox::Response(this, "Error", "SaTScan can not update will analyses are executing.\n"
                                                  "Please cancel or wait for analyses to complete before attempting to update.", XBMB_OK);
          return;
        }
        frmDownloadProgress = new TfrmDownloadProgress(this);
        frmDownloadProgress->Add(frmUpdateCheck->GetUpdateApplicationInfo());
        frmDownloadProgress->Add(frmUpdateCheck->GetUpdateArchiveInfo());
        frmDownloadProgress->DownloadFiles();
        if (frmDownloadProgress->GetDownloadCompleted()) {
          GetToolkit().SetUpdateArchiveFilename(frmUpdateCheck->GetUpdateArchiveInfo().first.GetCString());
          GetToolkit().SetRunUpdateOnTerminate(true);
          Close();
        }
      }
    }
    else
      TBMessageBox::Response(this, "No Updates", "You are running the most current version of SaTScan.", MB_OK);

    delete frmDownloadProgress; frmDownloadProgress=0;
    delete frmUpdateCheck; frmUpdateCheck=0;
  }
  catch (ZdException &x) {
    delete frmDownloadProgress;
    delete frmUpdateCheck;
    x.AddCallpath("UpdateActionExecute","TfrmMainForm");
    DisplayBasisException(this, x);
  }
}


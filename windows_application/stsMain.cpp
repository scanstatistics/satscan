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
//---------------------------------------------------------------------------
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

__fastcall TfrmMainForm::~TfrmMainForm() {}

//---------------------------------------------------------------------------
void __fastcall TfrmMainForm::AboutSatscanActionExecute(TObject *Sender) {
  TfrmAbout(this).ShowModal();
}
//---------------------------------------------------------------------------
void __fastcall TfrmMainForm::CloseSessionActionExecute(TObject *Sender) {
  if (ActiveMDIChild)
    ActiveMDIChild->Close();
}
//---------------------------------------------------------------------------

// enables/disables the appropraite buttons and controls based on their category type
void TfrmMainForm::EnableActions(bool bEnable) {
   for(int i = 0; i < ActionList->ActionCount; ++i) {
      TAction* pAction = dynamic_cast<TAction*>(ActionList->Actions[i]);
      if (pAction) {
         if(pAction->Category == "All")
             pAction->Enabled = bEnable;
         else if(pAction->Category == "AnalysisRun" || pAction->Category == "Analysis")
             pAction->Enabled = !bEnable;
      }
   }
}

void __fastcall TfrmMainForm::ExecuteActionExecute(TObject *Sender) {
  try {
    ExecuteSession();
  }
  catch (ZdException & x) {
    x.AddCallpath("ExecuteActionExecute", "TfrmMainForm");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
void TfrmMainForm::ExecuteSession() {
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
    x.AddCallpath("ExecuteSession()", "TfrmMainForm");
    throw;
  }
}
//---------------------------------------------------------------------------
void __fastcall TfrmMainForm::ExitActionExecute(TObject *Sender) {
  Close();
}
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
void __fastcall TfrmMainForm::HelpActionExecute(TObject *Sender) {
  HINSTANCE     hReturn;
  ZdString      sMessage;

  //Attempt to open chm user guide.
  hReturn = ShellExecute(NULL, "open", "SaTScan_Help.chm", NULL, NULL, SW_SHOWNORMAL);
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
//---------------------------------------------------------------------------
void __fastcall TfrmMainForm::ImportActionExecute(TObject *Sender) {
  try {
    TfrmAnalysis * frmAnalysisForm = dynamic_cast<TfrmAnalysis *>(frmMainForm->ActiveMDIChild);
    if (frmAnalysisForm)
      frmAnalysisForm->LaunchImporter();
  }
  catch (ZdException & x) {
    x.AddCallpath("ImportActionExecute()", "TfrmMainForm");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
void __fastcall TfrmMainForm::NewSessionActionExecute(TObject *Sender) {
  try {
    new TfrmAnalysis(this, ActionList);
  }
  catch (ZdException & x) {
    x.AddCallpath("NewSessionActionExecute", "TfrmMainForm");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
void TfrmMainForm::OpenAFile(){
  try {
    OpenDialog1->FileName = "";
    OpenDialog1->DefaultExt = "*.prm";
    OpenDialog1->Filter = "Parameter Files (*.prm)|*.prm|All Files (*.*)|*.*";   //put more types in here...
    OpenDialog1->Title = "Select Parameter File";
    if (OpenDialog1->Execute())
      new TfrmAnalysis(this, ActionList, OpenDialog1->FileName.c_str());
  }
  catch (ZdException & x) {
    x.AddCallpath("OpenAFile()", "TfrmMainForm");
    throw;
  }
}
//------------------------------------------------------------------------------
void __fastcall TfrmMainForm::OpenParameterFileActionExecute(TObject *Sender) {
  try {
    OpenAFile();
  }
  catch (ZdException & x) {
    x.AddCallpath("OpenParameterFileActionExecute", "TfrmMainForm");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
void __fastcall TfrmMainForm::OutputTextFile1Click(TObject *Sender) {
  int  iHandle;
  long lFileLength;

  try {
    OpenDialog1->FileName = "";
    OpenDialog1->DefaultExt = "*.txt";
    OpenDialog1->Filter = "Output Files (*.txt)|*.txt";   //put more types in here...
    OpenDialog1->Title = "Select Output File";
    if (OpenDialog1->Execute()) {
      //see if the file is too big... set some limit and adhere to it.
      iHandle = open(OpenDialog1->FileName.c_str(), O_RDONLY);
      lFileLength = filelength(iHandle);
      /* close the file */
      close(iHandle);
      if (lFileLength > 500000)
        Application->MessageBox("The output results file is too big for SaTScan to view.  Please use an independent text viewer to review the results file." ,"Warning", MB_OK);
      else
        new TfrmOutputViewer(this, OpenDialog1->FileName.c_str());
      }
  }
  catch (ZdException & x) {
    x.AddCallpath("OutputTextFile1Click", "TfrmMainForm");
    throw;
  }
}
//---------------------------------------------------------------------------
void __fastcall TfrmMainForm::PrintSessionActionExecute(TObject *Sender) {
/*   TfrmAnalysis    *frmBaseForm;
   TfrmAnalysisRun *frmAnalysisRun;
   CParameters     *pSession;
   AnsiString       sCaption;

   try
      {
      // YES.. YES.. YES...  I KNOW I SHOULD HAVE A NICE BASE FORM
      // AND NOT HAVE TO DO THIS DOUBLE DYNAMIC_CAST...   THIS WAS A LAST
      // MINUTE ADDITION...   AND I MEAN LAST MINUTE !!!!
      //
      frmBaseForm = dynamic_cast<TfrmAnalysis *>(frmMainForm->ActiveMDIChild);
      if (frmBaseForm)
         {
         pSession = (CParameters *)frmBaseForm->GetSession();
         //if (pSession)
         //   pSession->Print();
         }
      else
         {
         frmAnalysisRun = dynamic_cast<TfrmAnalysisRun *>(frmMainForm->ActiveMDIChild);
         //if (frmAnalysisRun)
         //    frmAnalysisRun->PrintTarget();
         }
      }
   catch (ZdException & x)
      {
      x.AddCallpath("PrintSessionActionExecute", "TfrmMainForm");
      DisplayBasisException(this, x);
      }  */
}
//---------------------------------------------------------------------------
void __fastcall TfrmMainForm::PrintSetupActionExecute(TObject *Sender) {
  PrinterSetupDialog1->Execute();
}
//---------------------------------------------------------------------------
void TfrmMainForm::RefreshOpenList() {
  SaTScanToolkit::ParameterHistory_t::const_iterator     itr;
  TMenuItem                                            * pItem=0;
  size_t                                                 t=1;

  mitReopen->Clear();
  const SaTScanToolkit::ParameterHistory_t & Parameters = GetToolkit().GetParameterHistory();
  mitReopen->Enabled = !Parameters.empty();
  for (t=0; t < Parameters.size(); t++) {
     pItem = new TMenuItem(mitReopen);
     pItem->OnClick = ActionReopen->OnExecute;
     if (Parameters[t].size() < 100)
       pItem->Caption.printf("&%i  %s", t, Parameters[t].c_str());
     else {
       ZdFileName File(Parameters[t].c_str());
       pItem->Caption.printf("&%i  %s\\ ... %s", t, File.GetDrive(), Parameters[t].substr(Parameters[t].size() - 100).c_str());
     }
     pItem->Tag = t;
     mitReopen->Add(pItem);
  }
}
//---------------------------------------------------------------------------
void TfrmMainForm::Save() {
  TfrmAnalysis *frmBaseForm;
  AnsiString    sFileName;

  try {
    frmBaseForm = dynamic_cast<TfrmAnalysis *>(frmMainForm->ActiveMDIChild);
    if (frmBaseForm) {
      sFileName =  frmBaseForm->GetFileName();
      if (sFileName.IsEmpty())
        frmBaseForm->SaveAs();
      else {
        // make sure that if the file exists it is NOT read only
        // also make sure that the directory can be written to...
        if (access(frmBaseForm->GetFileName(), 02) == 0)
          frmBaseForm->WriteSession();
        else
          Application->MessageBox("Can not save session.  The file is either read only or you do not have write privledges to the directory." ,"Error", MB_OK);
      }
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("Save()", "TfrmMainForm");
    throw;
  }
}
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
void __fastcall TfrmMainForm::SaveSessionActionExecute(TObject *Sender) {
  try {
    Save();
  }
  catch (ZdException & x) {
    x.AddCallpath("SaveSessionActionExecute", "TfrmMainForm");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
void __fastcall TfrmMainForm::SaveSessionAsActionExecute(TObject *Sender){
  try {
    SaveAs();
  }
  catch (ZdException & x) {
    x.AddCallpath("SaveSessionAsActionExecute", "TfrmMainForm");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
void __fastcall TfrmMainForm::UsingHelpActionExecute(TObject *Sender) {
  WinHelp(0, "winhelp.hlp", HELP_HELPONHELP, 0L);
}
//---------------------------------------------------------------------------
void __fastcall TfrmMainForm::ActionReopenExecute(TObject *Sender) {
  TMenuItem     * pItem;
  std::string     sFileName;

  try {
    pItem = dynamic_cast<TMenuItem *>(Sender);
    if (pItem) {
      sFileName = GetToolkit().GetParameterHistory()[pItem->Tag];
      if (!File_Exists(const_cast<char*>(sFileName.c_str())))
        ZdException::GenerateNotification("Cannot open file '%s'.","mitReopenClick()", sFileName.c_str());
      new TfrmAnalysis(this, ActionList, const_cast<char*>(sFileName.c_str()));
    }  
  }
  catch (ZdException & x) {
    x.AddCallpath("mitReopenClick", "TfrmMainForm");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
void __fastcall TfrmMainForm::FormClose(TObject *Sender, TCloseAction &Action) {
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
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
void TfrmMainForm::Setup() {
  EnableActions(true);
  RefreshOpenList();
}

void __fastcall TfrmMainForm::FormActivate(TObject *Sender) {
  try {
    if (gbShowStartWindow) {
      frmStartWindow = new TfrmStartWindow(this);
      if (frmStartWindow->ShowModal() == mrOk) {
         switch (frmStartWindow->GetSelectedItemIndex()) {
           case 0  : new TfrmAnalysis(this, ActionList); break;
           case 1  : OpenAFile(); break;
           case 2  : if (!File_Exists(const_cast<char*>(GetToolkit().GetParameterHistory()[0].c_str())))
                       ZdException::GenerateNotification("Cannot open file '%s'.", "FormActivate()",
                                                         GetToolkit().GetParameterHistory()[0].c_str());
                     new TfrmAnalysis(this, ActionList, const_cast<char*>(GetToolkit().GetParameterHistory()[0].c_str()));
                     break;
           default : ZdException::GenerateNotification("Unknown operation index % d.",
                                                       "FormActivate()", frmStartWindow->GetSelectedItemIndex());
         }
      }
      delete frmStartWindow; frmStartWindow=0;
      gbShowStartWindow = false;
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("FormActivate", "TfrmMainForm");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
void __fastcall TfrmMainForm::ActionUpdateCheckExecute(TObject *Sender) {
  TfrmUpdateCheck             * frmUpdateCheck=0;
  TfrmDownloadProgress        * frmDownloadProgress=0;
  ZdString                      sMessage;

  try {
    frmUpdateCheck = new TfrmUpdateCheck(this);
    frmUpdateCheck->ConnectToServerForUpdateCheck();
    if (frmUpdateCheck->HasUpdates()) {
      sMessage.printf("SaTScan v%s is available.\nDo you want to install now?", frmUpdateCheck->GetUpdateVersion().GetCString());
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
      TBMessageBox::Response(this, "No Updates", "There are no updates at this time. Please try again later.", MB_OK);

    delete frmDownloadProgress; frmDownloadProgress=0;
    delete frmUpdateCheck; frmUpdateCheck=0;
  }
  catch (ZdException &x) {
    delete frmDownloadProgress;
    delete frmUpdateCheck;
    x.AddCallpath("ActionUpdateCheckExecute", "TfrmMainForm");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
#include "stsSaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

#include "stsOutputFileRegistry.h"

TfrmMainForm *frmMainForm;
//---------------------------------------------------------------------------
__fastcall TfrmMainForm::TfrmMainForm(TComponent* Owner): TForm(Owner){
  EnableActions(true);
  RefreshOpenList();
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
  TfrmAnalysis                * frmBaseForm;
  CParameters                 * pSession;
  AnsiString                    sCaption;
  TfrmAnalysisRun             * pAnalysisRun=0;
  CalcThread                  * pThread;
  std::string                   sFileName;

  try {
    //make sure window is a session window
    frmBaseForm = dynamic_cast<TfrmAnalysis *>(frmMainForm->ActiveMDIChild);
    if (frmBaseForm && frmBaseForm->ValidateParams()) {
      pSession = (CParameters *)frmBaseForm->GetSession();
      if (pSession) {
        if (frmBaseForm->GetFileName()) {
          sCaption = "Running ";
          sCaption += frmBaseForm->GetFileName();
        }
        else
          sCaption = "Running Unknown Session Name";

        sFileName = pSession->GetOutputFileName();
        // if we don't already have a thread with the result file running then launch one
        if(!gRegistry.IsAlreadyRegistered(sFileName)) {
           gRegistry.Register(sFileName);
           pAnalysisRun = new TfrmAnalysisRun(this, sFileName, gRegistry, ActionList);
           CalcThread * pThread = new CalcThread(true, *pSession, sCaption.c_str(), *pAnalysisRun);
           pThread->Resume(); // starts the thread
        }
        else   // there's already a thread writing to this output file
           ZdException::GenerateNotification("The filename you specified for results file is \ncurrently being written to by another analysis.\n"
                                             "Please choose another results filename." , "Error!");
      }  // end if session
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ExecuteSession()", "TfrmMainForm");
    delete pAnalysisRun;
    throw;
  }
}
//---------------------------------------------------------------------------
void __fastcall TfrmMainForm::ExitActionExecute(TObject *Sender) {
  Close();
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
void __fastcall TfrmMainForm::OpenAFile(){
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
    for (int i=0; i < MDIChildCount; i++)
       MDIChildren[i]->Close();
  }
  catch (...){}
}
//---------------------------------------------------------------------------


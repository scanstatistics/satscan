//---------------------------------------------------------------------------
#include "stsSaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
#include "stsOutputFileRegistry.h"

/** constructor */
__fastcall TfrmAnalysisRun::TfrmAnalysisRun(TComponent* Owner, const CParameters & Parameters, const std::string& sOutputFileName,
                                            stsOutputFileRegister & Registry, TActionList* theList)
                           :stsBaseAnalysisChildForm (Owner, theList), gRegistry(Registry), gsOutputFileName(sOutputFileName) {
  try {
    Init();
    Setup(Parameters);
  }  
  catch (ZdException & x) {
    x.AddCallpath("constructor()", "TfrmAnalysisRun");
    throw;
  }
}
/** destructor */
__fastcall TfrmAnalysisRun::~TfrmAnalysisRun() {
  try {
    delete gpAnalysisThread; gpAnalysisThread=0;
  }
  catch (...){}
}
/** adds string to analysis output memo control */
void TfrmAnalysisRun::AddLine(char *sLine) {
  rteAnalysisBox->Lines->Add(sLine);
}

/** adds string to warning output memo control */
void TfrmAnalysisRun::AddWarningLine(char *sLine) {
  rteWarningsBox->Lines->Add(sLine);
}

/** indicates analysis is cancelled in interface */
void TfrmAnalysisRun::CancelJob() {
  if (rteWarningsBox->Lines->Count)
    rteAnalysisBox->Lines->Add("Job cancelled. Please review 'Warnings/Errors' window below.");
  else
    rteAnalysisBox->Lines->Add("Job cancelled.");
  btnCancel->Caption = "Close";
  gbCancel = true;
  gRegistry.Release(gsOutputFileName);
  SetCanClose(true);
}

/** enables/disables the appropraite buttons and controls for this window */
void TfrmAnalysisRun::EnableActions(bool bEnable) {
   for(int i = 0; i < gpList->ActionCount; ++i) {
      TAction* pAction = dynamic_cast<TAction*>(gpList->Actions[i]);
      if (pAction) {
         if (pAction->Category == CATEGORY_ALL || pAction->Category == CATEGORY_ANALYSIS_RUN)
             pAction->Enabled = bEnable;
         else if (pAction->Category == CATEGORY_ANALYSIS)
             pAction->Enabled = !bEnable;
      }
   }
}

/** triggers TForm::Close() -- if bForce is true, the thread is forced to terminate */
void TfrmAnalysisRun::CloseForm(bool bForce) {
  if (bForce) ForceThreadTermination();
  Close();
}

/** forces thread termination -- this function should only be called as a last
   resort as it causes a memory leak. When closing the application with active
   threads, this function is useful to kill running threads and the memory leak
   becomes insignificant. Note: The ideal means for terminating a thread would
   be to call it's Terminate() method, but that translates to the thread having
   to constantly checking whether it's terminate property is true. */
void TfrmAnalysisRun::ForceThreadTermination() {
  try {
    gpAnalysisThread->Suspend();
    delete gpAnalysisThread; gpAnalysisThread=0;
    gbCanClose = true;
  }
  catch (...){}
}

/** form close event */
void __fastcall TfrmAnalysisRun::FormClose(TObject *Sender, TCloseAction &Action) {
  if (GetCanClose()) {
    Action = caFree;
    gRegistry.Release(gsOutputFileName);
  }
  else
    Action = caNone;
}

/** internal initialization method */
void TfrmAnalysisRun::Init() {
  gbCanClose=false;
  gbCancel=false;
  gpAnalysisThread=0;
}

/** starts execution of analysis thread */
void TfrmAnalysisRun::LaunchThread() {
  if (! gpAnalysisThread)
    ZdGenerateException("Null thread pointer.","LaunchThread()");

  gpAnalysisThread->Resume();
  rteAnalysisBox->SetFocus();
}

/** Loads analysis results from file into memo control */
void TfrmAnalysisRun::LoadFromFile(const char * sFileName) {
   int  iHandle;
   long lFileLength;

   //see if the file is too big... set some limit and adhere to it.
   iHandle = open(sFileName, O_RDONLY);
   lFileLength = filelength(iHandle);
   /* close the file */
   close(iHandle);
   if (lFileLength > 500000)
      {
      rteAnalysisBox->Clear();
      rteAnalysisBox->Lines->Add("The output results file is too big for SaTScan to view.");
      rteAnalysisBox->Lines->Add("Please use an independent text viewer to review the results file.");
      rteAnalysisBox->Lines->Add(" ");
      rteAnalysisBox->Lines->Add(sFileName);
      }
   else
      rteAnalysisBox->Lines->LoadFromFile(sFileName);

   gRegistry.Release(gsOutputFileName);
}

/** cancel button click event */
void __fastcall TfrmAnalysisRun::OnCancelClick(TObject *Sender) {
  if (btnCancel->Caption == "Close")
    Close();
  else {
    rteAnalysisBox->Lines->Add("Cancelling job, please wait...");
    gbCancel = true;
  }
  gRegistry.Release(gsOutputFileName);
}

/** email button click event - attempts to launch default email application */
void __fastcall TfrmAnalysisRun::OnEMailClick(TObject *Sender) {
   PMapiRecipDesc   pRecipient = 0;
   TMapiMessage     theMapiMessage;
   char *           sMsgTitle = 0;
   long             i, lMsgTitleLength;
   AnsiString       sMessageText;
   unsigned long    ulError;

   ulError = MapiResolveName ( 0, 0, const_cast<char *>(GetToolkit().GetTechnicalSupportEmail()), 0, 0, pRecipient );
   if ( ulError == SUCCESS_SUCCESS ){
      lMsgTitleLength = snprintf ( sMsgTitle, 0, "%s -- %s -- Automated Error Message", Application->Title.c_str(), VERSION_NUMBER );
      sMsgTitle = new char[lMsgTitleLength + 1];
      sprintf ( sMsgTitle, "%s -- %s -- Automated Error Message", Application->Title.c_str(), VERSION_NUMBER );
      sMsgTitle[lMsgTitleLength] = 0;

      sMessageText = "--Job Progress Info--\n";
      sMessageText += rteAnalysisBox->Lines->GetText();
      sMessageText += "\n\n\n--Warnings/Errors Info--\n";
      sMessageText += rteWarningsBox->Lines->GetText();
      sMessageText += "\n\n";
      if (!gsProgramErrorCallPath.empty()) {
        sMessageText += "--Call Path Info--\n\n";
        sMessageText += gsProgramErrorCallPath.c_str();
        sMessageText += "\n\n";
      }
      sMessageText += "\n--End Of Error Message--";

      theMapiMessage.ulReserved = 0;
      theMapiMessage.lpszSubject = sMsgTitle;
      theMapiMessage.lpszNoteText = const_cast<char *>(sMessageText.c_str());
      theMapiMessage.lpszMessageType = 0;
      theMapiMessage.lpszDateReceived = 0;
      theMapiMessage.lpszConversationID = 0;
      theMapiMessage.flFlags = 0;
      theMapiMessage.lpOriginator = NULL;
      theMapiMessage.nRecipCount = 1;
      theMapiMessage.lpRecips = pRecipient;
      theMapiMessage.nFileCount = 0;
      theMapiMessage.lpFiles = NULL;

      ulError = MapiSendMail(0, (unsigned int)this->Handle, theMapiMessage, MAPI_DIALOG | MAPI_LOGON_UI /*| MAPI_NEW_SESSION*/, 0);
      if (ulError != 0)    //returns zero on success
         Application->MessageBox("E-mail was not able to be sent.", "Warning", MB_OK);
   }
   else
      Application->MessageBox("Message Service not available.", "Warning", MB_OK);

   delete [] sMsgTitle; 
   MapiFreeBuffer(pRecipient);
   //bool bLaunched;

   //bLaunched = ((int) ShellExecute(0, "open", "mailto:joinpoint@mail.nih.gov", "test1", "test2", SW_SHOW) > 31);
   //if (!bLaunched)
   //   {
   //   Application->MessageBox("Cannot open Internet browser to view Joinpoint Web site.","Error",MB_OK);
   //   }
}

/** print button click event */
void __fastcall TfrmAnalysisRun::OnPrintClick(TObject *Sender){
   TRichEdit *  rtePrintText;

   if (PrintDialog->Execute()) {
     rtePrintText = new TRichEdit(this);
     rtePrintText->Hide();
     rtePrintText->Parent = this;

     rtePrintText->Lines->Append(rteAnalysisBox->Lines->Text);
     rtePrintText->Lines->Append("");
     rtePrintText->Lines->Append("");
     rtePrintText->Lines->Append("");
     rtePrintText->Lines->Append("WARNINGS / ERRORS ");
     rtePrintText->Lines->Append("");
     rtePrintText->Lines->Append(rteWarningsBox->Lines->Text);
     rtePrintText->Print(Application->Title.c_str());

     delete rtePrintText;
   }
}

/** form activate event */
void __fastcall TfrmAnalysisRun::FormActivate(TObject *Sender) {
  EnableActions(true);
}

/** internal setup function */
void TfrmAnalysisRun::Setup(const CParameters & Parameters) {
  try {
    gpAnalysisThread = new CalcThread(*this, Parameters);
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","TfrmAnalysisRun");
    throw;
  }
}


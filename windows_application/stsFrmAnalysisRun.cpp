//---------------------------------------------------------------------------
#include "stsSaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

#include "stsOutputFileRegistry.h"

TfrmAnalysisRun *frmAnalysisRun;

//---------------------------------------------------------------------------
__fastcall TfrmAnalysisRun::TfrmAnalysisRun(TComponent* Owner, const std::string& sOutputFileName,stsOutputFileRegister & Registry)
                           :TForm(Owner), gRegistry(Registry) {
  Init();
  gsOutputFileName = sOutputFileName;
}
//---------------------------------------------------------------------------
void TfrmAnalysisRun::AddLine(char *sLine) {
  rteAnalysisBox->Lines->Add(sLine);
}
//---------------------------------------------------------------------------
void TfrmAnalysisRun::AddWarningLine(char *sLine) {
  rteWarningsBox->Lines->Add(sLine);
}
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysisRun::FormClose(TObject *Sender, TCloseAction &Action) {
  if (GetCanClose())
    Action = caFree;
  else
    Action = caNone;
  gRegistry.Release(gsOutputFileName);
}
//---------------------------------------------------------------------------
void TfrmAnalysisRun::Init() {
  gbCanClose=false;
  gbCancel=false;
}
//---------------------------------------------------------------------------
void TfrmAnalysisRun::LoadFromFile(char *sFileName) {
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
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysisRun::OnCancelClick(TObject *Sender) {
  if (btnCancel->Caption == "Close")
    Close();
  else {
    rteAnalysisBox->Lines->Add("Cancelling job, please wait...");
    gbCancel = true;
  }
  gRegistry.Release(gsOutputFileName);
}
//---------------------------------------------------------------------------
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
         Application->MessageBox("E-mail was not able to be sent.", "Warning", MB_OKCANCEL);
   }
   else
      Application->MessageBox("Message Service not available.", "Warning", MB_OKCANCEL);

   delete [] sMsgTitle; 
   MapiFreeBuffer(pRecipient);
   //bool bLaunched;

   //bLaunched = ((int) ShellExecute(0, "open", "mailto:joinpoint@mail.nih.gov", "test1", "test2", SW_SHOW) > 31);
   //if (!bLaunched)
   //   {
   //   Application->MessageBox("Cannot open Internet browser to view Joinpoint Web site.","Error",MB_OK);
   //   }
}
//---------------------------------------------------------------------------
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
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
#include "stsSaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmAnalysisRun *frmAnalysisRun;

//---------------------------------------------------------------------------
__fastcall TfrmAnalysisRun::TfrmAnalysisRun(TComponent* Owner) : TForm(Owner) {
  Init();
}
//---------------------------------------------------------------------------
void TfrmAnalysisRun::AddLine(char *sLine) {
  reAnalysisBox->Lines->Add(sLine);
}
//---------------------------------------------------------------------------
void TfrmAnalysisRun::AddWarningLine(char *sLine) {
   // Since the SatScan program prints an error message for each input file
   // record in error, just set some odd max here for now, so it does not
   // print and print and print until it runs out of input file lines...
   // --- This still needs to be revisited.
   if (! gbMaximumWarningsReached) {
     if (reWarningsBox->Lines->Count < gbMaximumWarningsPrinted)
       reWarningsBox->Lines->Add(sLine);
     else {
       reWarningsBox->Lines->Add("Output exceeded limit...");
       gbMaximumWarningsReached = true;
     }
   }
}
//---------------------------------------------------------------------------
void TfrmAnalysisRun::CancelJob() {
  if (reWarningsBox->Lines->Count)
    reAnalysisBox->Lines->Add("Analysis cancelled. Please review 'Warnings/Errors' window below.");
  else
    reAnalysisBox->Lines->Add("Analysis cancelled.");
  btnCancel->Caption = "Close";
  gbCancel = true;
  SetCanClose(true);
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysisRun::FormClose(TObject *Sender, TCloseAction &Action) {
  if (GetCanClose())
    Action = caFree;
  else
    Action = caNone;
}
//---------------------------------------------------------------------------
void TfrmAnalysisRun::Init() {
  gbCanClose=false;
  gbCancel=false;
  gbPrintWarnings=true;
  gbMaximumWarningsReached=false;
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
      reAnalysisBox->Clear();
      reAnalysisBox->Lines->Add("The output results file is too big for SaTScan to view.");
      reAnalysisBox->Lines->Add("Please use an independent text viewer to review the results file.");
      reAnalysisBox->Lines->Add(" ");
      reAnalysisBox->Lines->Add(sFileName);
      }
   else
      reAnalysisBox->Lines->LoadFromFile(sFileName);

   FFileName = sFileName;
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysisRun::OnCancelClick(TObject *Sender) {
  if (btnCancel->Caption == "Close")
    Close();
  else {
    reAnalysisBox->Lines->Add("Cancelling analysis, please wait...");
    gbCancel = true;
  }
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysisRun::OnEMailClick(TObject *Sender) {
   PMapiRecipDesc   pRecipient = 0;
   TMapiMessage     theMapiMessage;
   char *           sMsgTitle = 0;
   long             i, lMsgTitleLength;
   AnsiString       sMessageText;
   unsigned long    ulError;

   ulError = MapiResolveName ( 0, 0, const_cast<char *>(TECH_EMAIL), 0, 0, pRecipient );
   if ( ulError == SUCCESS_SUCCESS ){
      lMsgTitleLength = snprintf ( sMsgTitle, 0, "%s -- %s -- Automated Error Message", Application->Title.c_str(), VERSION_NUMBER );
      sMsgTitle = new char[lMsgTitleLength + 1];
      sprintf ( sMsgTitle, "%s -- %s -- Automated Error Message", Application->Title.c_str(), VERSION_NUMBER );
      sMsgTitle[lMsgTitleLength] = 0;

      sMessageText = "--Job Progress Info--\n";
      sMessageText += reAnalysisBox->Lines->GetText();
      sMessageText += "\n\n\n--Warnings/Errors Info--\n";
      sMessageText += reWarningsBox->Lines->GetText();
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
   if ( PrintDialog1->Execute() )
      {
      // THERE WAS A PROBLEM WITH PRINTING BOTH THE ANALYSIS RICH TEXT
      // BOX AND THE WARNINGS/ERRORS BOX...  SOMEHOW IT WAS PRINTING OVER 1000
      // BLANK PAGES...  I KNOW THAT THIS IS A STUPID WAY TO DO IT, BUT I
      // NEED TO RESOLVE THIS AND GET IT OUT...

      // AND THIS ACTUALLY PRODUCED THE SAME PROBLEM !!!!!
      //
      // IS THIS A BORLAND BUG ?????

      if (gbPrintWarnings)
         {
          RichEdit1->Lines->Clear();
         // LOAD THE MEMO CONTROL WITH TEXT FROM BOTH
         for (int i = 0; i < reAnalysisBox->Lines->Count; i++)
            RichEdit1->Lines->Add(reAnalysisBox->Lines->Strings[i].c_str());
         //Memo1->Lines->Add(reAnalysisBox->Lines->GetText());
         RichEdit1->Lines->Add(" ");
         RichEdit1->Lines->Add(" ");
         RichEdit1->Lines->Add(" ");
         RichEdit1->Lines->Add("WARNINGS / ERRORS ");
         RichEdit1->Lines->Add(" ");
         for (int i = 0; i < reWarningsBox->Lines->Count; i++)
            RichEdit1->Lines->Add(reWarningsBox->Lines->Strings[i].c_str());
         //Memo1->Lines->Add(reAnalysisBox->Lines->GetText());
         //RichEdit1->Lines->Add(EOF);
         RichEdit1->Print("SaTScan Information");
         }
      else
         {
         reAnalysisBox->Print( "Results" );
         //if (gbPrintWarnings)
         //   reWarningsBox->Print("Warnings/Errors");
         }
      //reAnalysisBox->Print( "Results" );
      }
}
//---------------------------------------------------------------------------


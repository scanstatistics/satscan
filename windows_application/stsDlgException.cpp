//Author Dave Hunt, Brian Simpson
//
//---------------------------------------------------------------------------
// BASIS : Exception display unit
//
// BdlgException -- a dialog window for graphical display of exceptions
//
//---------------------------------------------------------------------------
#include "stsSaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
// Using declarations
using std::auto_ptr;

#pragma resource "*.dfm"

// This is a global function which will display an exception dialog.  If 'sAlarmLevelString'
// is NULL, a default message will be displayed.
void DisplayBasisException ( Classes::TComponent * Owner, const SSException &theException, const char *sAlarmLevelString ) {

   // DCH 8/9/2000
   //
   // To make sure that this function doesn't bomb the system, we black-out
   // SSExceptions. ( We don't want to consume system or VCL exceptions : we
   // aren't set up to handle those anyway... )
   try {
      if (theException.GetLevel() == SSException::Notify)
         MessageBox( Owner, "Notification", theException.GetErrorMessage(), MB_OK );
      else {
         auto_ptr<TBdlgException>  pDialog ( new TBdlgException ( Owner, theException, sAlarmLevelString ) );
         pDialog->ShowModal();
      }
   }
   catch ( SSException &theException ) {
      // Chomp, chomp...
   }
}

//DEPRECATED: use function with 'Owner' argument, declared above
// This is a global function which will display an exception dialog.  If 'sAlarmLevelString'
// is NULL, a default message will be displayed.
void DisplayBasisException ( const SSException &theException, const char *sAlarmLevelString ) {

   // DCH 8/9/2000
   //
   // To make sure that this function doesn't bomb the system, we black-out
   // SSExceptions. ( We don't want to consume system or VCL exceptions : we
   // aren't set up to handle those anyway... )
   try {
      if (theException.GetLevel() == SSException::Notify)
         MessageBox( 0, "Notification", theException.GetErrorMessage(), MB_OK );
      else {
         auto_ptr<TBdlgException>  pDialog ( new TBdlgException ( 0, theException, sAlarmLevelString ) );
         pDialog->ShowModal();
      }
   }
   catch ( SSException &theException ) {
      // Chomp, chomp...
   }
}

//deprecated: instead, use single-string-argument function declared above.
// This is a global function which will display an exception dialog
void DisplayBasisException ( const SSException &theException, const char *sNotify, const char *sWarning ) {
   DisplayBasisException ( theException, sNotify );
}



//---------------------------------------------------------------------------
__fastcall TBdlgException::TBdlgException(TComponent* Owner, const SSException &theException, const char * sAlarmLevelString)
        : TForm(Owner)
{
   Setup(theException, sAlarmLevelString);
}
//---------------------------------------------------------------------------
__fastcall TBdlgException::~TBdlgException()
{
}
//---------------------------------------------------------------------------
// Abridge the form to hide the developer-specific information
//<br>preconditions:
//<br>IsAmplified()
//<br>:end preconditions
void TBdlgException::Abridge(){
   try{
      if (! IsAmplified())
         SSException::Generate("dialog is already abridged", "Abridge");

      Bevel1->Visible = false;
      ClientHeight = Bevel1->Top + (Bevel1->Height / 2);
      btnMoreLess->Caption = gsMoreLessButtonCaption_More.c_str();
      rteExtendedInfo->Enabled = false;

      gbIsAmplified = false;
   }
   catch(SSException & e){
      e.AddCallpath("Abridge", "TBdlgExceptionExpandable");
      throw;
   }
}

// Amplify the form to show the developer-specific information
//<br>preconditions:
//<br>! IsAmplified()
//<br>:end preconditions
void TBdlgException::Amplify(){
   try{
      if (IsAmplified())
         SSException::Generate("dialog is already amplified", "Abridge");

      Bevel1->Visible = true;
      ClientHeight = rteExtendedInfo->Top + rteExtendedInfo->Height + 8;
      btnMoreLess->Caption = gsMoreLessButtonCaption_Less.c_str();
      rteExtendedInfo->Enabled = true;

      gbIsAmplified = true;
   }
   catch(SSException & e){
      e.AddCallpath("Amplify", "TBdlgExceptionExpandable");
      throw;
   }
}

// Set position, visibility, and enabled status according to eLevel.
void TBdlgException::ArrangeButtons(SSException::Level eLevel){
   switch(eLevel){
      case SSException::Notify :
         btnPrint->Visible = false;
         btnEmail->Visible = false;
         btnMoreLess->Visible = false;
         break;
      case SSException::Warning :
         btnPrint->Visible = true;
         btnPrint->Enabled = true;
         btnEmail->Visible = false;
         btnMoreLess->Visible = false;
         break;
      default: //Normal, Critical
         btnPrint->Visible = true;
         btnPrint->Enabled = true;
         btnEmail->Visible = true;
        // btnEmail->Enabled = GetEmailButtonCanBeEnabled();
         btnMoreLess->Visible = true;
         btnMoreLess->Enabled = true;
         break;
   }
}

// Can the btnEmail->Enabled be set to true ?
//bool TBdlgException::GetEmailButtonCanBeEnabled(){
//   return ( BasisGetToolkitIsAvailable() &&
//           (strlen(BasisGetToolkit().GetErrorReportDestination()) > 0)
//         );
//}

// Is the form showing the developer-specific information ?
bool TBdlgException::IsAmplified() const {
   return gbIsAmplified;
}

void TBdlgException::OnCreate(SSException::Level eLevel){
   ArrangeButtons(eLevel);
}

// Internal function to handle an EMail click. If no recipient has been set, you
// should never get to this function.
/*void TBdlgException::OnEmailClick() {
   PMapiRecipDesc   pRecipient = 0;
   TMapiMessage     theMapiMessage;
   char *           sMsgTitle = 0;
   long             lMsgTitleLength;
   ZdString         sMessageText;
   unsigned long    ulError;

   ulError = MapiResolveName ( 0, 0, const_cast<char *>(BasisGetToolkit().GetErrorReportDestination()), 0, 0, pRecipient );
   if ( ulError == SUCCESS_SUCCESS ){
      //sprintf ( sMsgTitle, "%s -- Automated Error Message", BBasisProperties::GetApplicationTitle() );
      lMsgTitleLength = snprintf ( sMsgTitle, 0, "%s -- Automated Error Message", Application->Title.c_str() );
      sMsgTitle = new char[lMsgTitleLength + 1];
      sprintf ( sMsgTitle, "%s -- Automated Error Message", Application->Title.c_str() );
      sMsgTitle[lMsgTitleLength] = 0;

      sMessageText << "--User Info--\n" << rteUserInfo->Text.c_str();
      sMessageText << "\n\n";
      sMessageText << "--Extended Info--\n" << rteExtendedInfo->Text.c_str();
      sMessageText << "\n--End Of Error Message--";

      theMapiMessage.ulReserved = 0;
      theMapiMessage.lpszSubject = sMsgTitle;
      theMapiMessage.lpszNoteText = const_cast<char *>(sMessageText.GetCString());
      theMapiMessage.lpszMessageType = 0;
      theMapiMessage.lpszDateReceived = 0;
      theMapiMessage.lpszConversationID = 0;
      theMapiMessage.flFlags = 0;
      theMapiMessage.lpOriginator = NULL;
      theMapiMessage.nRecipCount = 1;
      theMapiMessage.lpRecips = pRecipient;
      theMapiMessage.nFileCount = 0;
      theMapiMessage.lpFiles = NULL;

      ulError = MapiSendMail(0, (unsigned int)this->Handle, theMapiMessage, MAPI_DIALOG | MAPI_LOGON_UI , 0);
      if (ulError != 0)    //returns zero on success
         Application->MessageBox("E-mail was not able to be sent.", "Warning", MB_OKCANCEL);
   }
   else
      Application->MessageBox("Message Service not available.", "Warning", MB_OKCANCEL);

   delete sMsgTitle; sMsgTitle = 0;
   MapiFreeBuffer(pRecipient);
} */

// Amplify or abridge the form, depending on its current amplified state.
void TBdlgException::OnMoreLessClick(){
   if (IsAmplified())
      Abridge();
   else
      Amplify();
}

void TBdlgException::OnPrintClick() {
   int          LogX, LogY;
   TRect        Rect;
   AnsiString   sPrintCaption;
   TRichEdit *  rtePrintText;

   sPrintCaption += Application->Title.c_str();
   sPrintCaption += " -- Automated Error Message";

   rtePrintText = new TRichEdit(this);
   rtePrintText->Parent = this;
   rtePrintText->Lines->Append("--User Info--");
   rtePrintText->Lines->Append(rteUserInfo->Lines->Text);
   rtePrintText->Lines->Append("");
   rtePrintText->Lines->Append("");
   rtePrintText->Lines->Append("--Extended Info--");
   rtePrintText->Lines->Append(rteExtendedInfo->Lines->Text);
   rtePrintText->Lines->Append("");
   rtePrintText->Lines->Append("--End Of Error Message--");

   LogX = GetDeviceCaps(Printer()->Handle, LOGPIXELSX);
   LogY = GetDeviceCaps(Printer()->Handle, LOGPIXELSY);

   rtePrintText->PageRect.Left = LogX/2;
   rtePrintText->PageRect.Top = LogY/2;
   rtePrintText->PageRect.Right = Printer()->PageWidth-LogX/2;
   rtePrintText->PageRect.Bottom = Printer()->PageHeight-LogY/2;
   rtePrintText->Print(sPrintCaption.c_str());

   delete rtePrintText; 
}

// This function sets the dialog box icon according to eLevel.
void TBdlgException::SetIcon ( SSException::Level eLevel ) {
   HICON            hWindowsIcon;    // The handle from windows
   auto_ptr<TIcon>  pIcon ( new TIcon ); // VCL icon

   switch ( eLevel ) {
      case SSException::Notify :
         hWindowsIcon = LoadIcon ( 0, IDI_ASTERISK );
         break;

      case SSException::Warning :
         hWindowsIcon = LoadIcon ( 0, IDI_EXCLAMATION );
         break;

      default :
         hWindowsIcon = LoadIcon ( 0, IDI_HAND );
         break;
   }

   if ( hWindowsIcon ){
      pIcon->Handle = hWindowsIcon;
      imgAlarmLevelImage->Picture->Graphic = pIcon.get();
   }
}

// Internal function which sets the initial state.
void TBdlgException::Setup ( const SSException &theException, const char *sAlarmLevelString ) {
   geLevel = theException.GetLevel();
   gbIsAmplified = true;
   gsMoreLessButtonCaption_More = "More-->>";
   gsMoreLessButtonCaption_Less = "<<--Less";
   Abridge();
   SetIcon(theException.GetLevel());
   ArrangeButtons(theException.GetLevel());
   rteUserInfo->Lines->Append(const_cast<char *>(theException.GetErrorMessage()));
   rteExtendedInfo->Lines->Append("Callpath:");
   rteExtendedInfo->Lines->Append(const_cast<char *>(theException.GetCallpath()));

   switch ( theException.GetLevel() ) {
      case SSException::Notify :
         Caption = "Notification";
         rteAlarmLevelInfo->Text = ( sAlarmLevelString ) ? sAlarmLevelString : "For your information :";
         break;

      case SSException::Warning :
         Caption = "Warning!";
         rteAlarmLevelInfo->Text = ( sAlarmLevelString ) ? sAlarmLevelString : "A potential problem has developed : ";
         break;

      default ://SSException::Normal/SSException::Critical
         rteAlarmLevelInfo->Text = ( sAlarmLevelString ) ? sAlarmLevelString : "An unexpected exception occurred in the program.\nPlease contact technical support with the following information: ";
         break;
   }
}

////////////////////////////////////////////////////////////////////////////////

void __fastcall TBdlgException::btnPrintClick(TObject *Sender)
{
   OnPrintClick();
}
//---------------------------------------------------------------------------

void __fastcall TBdlgException::btnEmailClick(TObject *Sender)
{
  // OnEmailClick();
}
//---------------------------------------------------------------------------

void __fastcall TBdlgException::btnMoreLessClick(TObject *Sender)
{
   OnMoreLessClick();
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
#include "stsSaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmAbout *frmAbout;
//---------------------------------------------------------------------------
__fastcall TfrmAbout::TfrmAbout(TComponent* Owner) : TForm(Owner) {
   lblEmail->Caption = SUBSTANTIVE_SUPPORT_EMAIL;
   lblVersion->Caption = VERSION_NUMBER;
   lblVersionDate->Caption = VERSION_DATE;
   lblWebSite->Caption = SATSCAN_WEBSITE;
}
//---------------------------------------------------------------------------
void __fastcall TfrmAbout::Button1Click(TObject *Sender) {
   Close();        
}
//---------------------------------------------------------------------------

void __fastcall TfrmAbout::lblWebSiteClick(TObject *Sender) {
  try {
    int iResult = ( int )ShellExecute(Handle, "open", SATSCAN_WEBSITE, 0, 0, SW_SHOWDEFAULT);
    if (iResult <= 32)
      ZdException::GenerateNotification("Unable to open SaTScan Web site.\nIf problem persists, please contact technical support.( code %d )", "OnSquishClick()", iResult);
  }
  catch (ZdException &x) {
     x.AddCallpath("lblWebSiteClick()", "TfrmAbout");
     throw;
  }
}
//---------------------------------------------------------------------------

void __fastcall TfrmAbout::lblEmailClick(TObject *Sender) {
   PMapiRecipDesc   pRecipient = 0;
   TMapiMessage     theMapiMessage;
   ZdString         sMessageText, sMessage;
   unsigned long    ulError;

   try {
      Screen->Cursor = crHourGlass;
      ulError = MapiResolveName ( 0, 0, SUBSTANTIVE_SUPPORT_EMAIL, 0, 0, pRecipient );
      if ( ulError == SUCCESS_SUCCESS ){
         theMapiMessage.ulReserved = 0;
         theMapiMessage.lpszSubject = "SaTScan";
         theMapiMessage.lpszNoteText = 0;
         theMapiMessage.lpszMessageType = 0;
         theMapiMessage.lpszDateReceived = 0;
         theMapiMessage.lpszConversationID = 0;
         theMapiMessage.flFlags = 0;
         theMapiMessage.lpOriginator = NULL;
         theMapiMessage.nRecipCount = 1;
         theMapiMessage.lpRecips = pRecipient;
         theMapiMessage.nFileCount = 0;
         theMapiMessage.lpFiles = NULL;

         Screen->Cursor = crDefault;
         ulError = MapiSendMail(0, (unsigned int)this->Handle, theMapiMessage, MAPI_DIALOG | MAPI_LOGON_UI, 0);
         if (ulError != 0) {   //returns zero on success
            switch(ulError) {
               case MAPI_E_AMBIGUOUS_RECIPIENT:
                  sMessage << "Ambiguous recipient";
                  Application->MessageBox(sMessage.GetCString(), "Error!", MB_OK);
                  break;
               case MAPI_E_UNKNOWN_RECIPIENT:
                  sMessage << "Unknown recipient";
                  Application->MessageBox(sMessage.GetCString(), "Error!", MB_OK);
                  break;
               case MAPI_E_INSUFFICIENT_MEMORY:
                  sMessage << "Insuficient memory";
                  Application->MessageBox(sMessage.GetCString(), "Error!", MB_OK);
                  break;
               case MAPI_E_LOGIN_FAILURE:
                  sMessage << "Email login failure";
                  Application->MessageBox(sMessage.GetCString(), "Error!", MB_OK);
                  break;
               case MAPI_E_USER_ABORT:     // user abort, do nothing
                  break;
               default:
                  sMessage << "Email was not able to be sent.";
                  Application->MessageBox(sMessage.GetCString(), "Error!", MB_OK);
                  break;
            }
         }  // end if error
      } // end if success
      else {
         Screen->Cursor = crDefault;
         Application->MessageBox("You must open Outlook or your default email service before\n continuing with this email. Please try again.."
                                 "\nIf this problem persists please contact technical support.", "Warning", MB_OK);
      }

      MapiFreeBuffer(pRecipient);
   }
   catch (ZdException &x) {
      Screen->Cursor = crDefault;
      MapiFreeBuffer(pRecipient);
      x.AddCallpath("OnEmailClick()", "TBdlgTechSupport");
      throw;
   }
}
//---------------------------------------------------------------------------

void __fastcall TfrmAbout::lblLicenseClick(TObject *Sender) {
   try {
    int iResult = ( int )ShellExecute(Handle, "open", SATSCAN_LICENSE, 0, 0, SW_SHOWDEFAULT);
    if (iResult <= 32)
      ZdException::GenerateNotification("Unable to open SaTScan Web site.\nIf problem persists, please contact technical support.( code %d )", "OnSquishClick()", iResult);
  }
  catch (ZdException &x) {
     x.AddCallpath("lblLicenseClick()", "TfrmAbout");
     throw;
  }
}
//---------------------------------------------------------------------------


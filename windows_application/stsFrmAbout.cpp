//---------------------------------------------------------------------------
#include "stsSaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmAbout *frmAbout;
//---------------------------------------------------------------------------
__fastcall TfrmAbout::TfrmAbout(TComponent* Owner) : TForm(Owner) {
  Setup();
}
//---------------------------------------------------------------------------
void __fastcall TfrmAbout::Button1Click(TObject *Sender) {
   Close();
}
//---------------------------------------------------------------------------

void __fastcall TfrmAbout::lblWebSiteClick(TObject *Sender) {
  try {
    int iResult = ( int )ShellExecute(Handle, "open", GetToolkit().GetWebSite(), 0, 0, SW_SHOWDEFAULT);
    if (iResult <= 32)
      ZdException::GenerateNotification("Unable to open SaTScan Web site.\nIf problem persists, please contact technical support.( code %d )", "OnSquishClick()", iResult);
  }
  catch (ZdException &x) {
     x.AddCallpath("lblWebSiteClick()", "TfrmAbout");
     throw;
  }
}
//---------------------------------------------------------------------------
/** internal setup function */
void TfrmAbout::Setup() {
  try {
    lblVersion->Caption = AnsiString("SaTScan v") + VERSION_NUMBER;
    lblVersion->Width = 440;
    lblTitle->Width = 440;
    lblReleaseDate->Caption = AnsiString("Release Date: ") + VERSION_DATE;
    lblWebSite->Caption = GetToolkit().GetWebSite();
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()", "TfrmAbout");
    throw;
  }
}  

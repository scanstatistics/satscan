//---------------------------------------------------------------------------
#include "stsSaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmAbout *frmAbout;
//---------------------------------------------------------------------------
__fastcall TfrmAbout::TfrmAbout(TComponent* Owner)
        : TForm(Owner)
{
   lblEmail->Caption = SUBSTANTIVE_SUPPORT_EMAIL;
   lblVersion->Caption = VERSION_NUMBER;
   lblVersionDate->Caption = VERSION_DATE;
   lblWebSite->Caption = SATSCAN_WEBSITE;   
}
//---------------------------------------------------------------------------
void __fastcall TfrmAbout::Button1Click(TObject *Sender)
{
   Close();        
}
//---------------------------------------------------------------------------





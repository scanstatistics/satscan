//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "TfrmAbout.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmAbout *frmAbout;
//---------------------------------------------------------------------------
__fastcall TfrmAbout::TfrmAbout(TComponent* Owner)
        : TForm(Owner)
{
   lblEmail->Caption = TECH_EMAIL;
   lblVersion->Caption = VERSION_NUMBER;
   lblVersionDate->Caption = VERSION_DATE;   
}
//---------------------------------------------------------------------------
void __fastcall TfrmAbout::Button1Click(TObject *Sender)
{
   Close();        
}
//---------------------------------------------------------------------------





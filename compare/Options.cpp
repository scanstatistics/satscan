//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "Options.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
__fastcall TfrmOptions::TfrmOptions(TComponent* Owner) : TForm(Owner){}
//---------------------------------------------------------------------------
void __fastcall TfrmOptions::btnBrowseComparisonAppClick(TObject *Sender) {
  OpenDialog->FileName =  "";
  OpenDialog->DefaultExt = "*.exe";
  OpenDialog->Filter = "Executable files (*.exe)|*.exe|All files (*.*)|*.*";
  OpenDialog->FilterIndex = 0;
  OpenDialog->Title = "Select Comparison Program";
  if (OpenDialog->Execute())
    edtComparisonApplication->Text = OpenDialog->FileName.c_str();
}
//---------------------------------------------------------------------------
void __fastcall TfrmOptions::btnBrowseArchiveApplicationClick(TObject *Sender) {
  OpenDialog->FileName =  "";
  OpenDialog->DefaultExt = "*.exe";
  OpenDialog->Filter = "Executable files (*.exe)|*.exe|All files (*.*)|*.*";
  OpenDialog->FilterIndex = 0;
  OpenDialog->Title = "Select Archive Program";
  if (OpenDialog->Execute())
    edtArchiveApplication->Text = OpenDialog->FileName.c_str();
}
//---------------------------------------------------------------------------


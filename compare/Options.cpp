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
DWORD TfrmOptions::GetThreadPriorityFlags() const {
   switch (rdoGroupThreadPriority->ItemIndex) {
     case 0 : return HIGH_PRIORITY_CLASS;
     case 1 : return IDLE_PRIORITY_CLASS;
     case 2 : return NORMAL_PRIORITY_CLASS;
     case 3 : return REALTIME_PRIORITY_CLASS;
   };
   return NORMAL_PRIORITY_CLASS;
}
//---------------------------------------------------------------------------
void __fastcall TfrmOptions::chkMinimizeConsoleWindowClick(TObject *Sender) {
  if (chkMinimizeConsoleWindow->Checked) chkSuppressDosWindow->Checked = false;
}
//---------------------------------------------------------------------------
void __fastcall TfrmOptions::chkSuppressDosWindowClick(TObject *Sender) {
  if (chkSuppressDosWindow->Checked) chkMinimizeConsoleWindow->Checked = false;
}
//---------------------------------------------------------------------------



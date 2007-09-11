#include "zd543.h"
#include <vcl.h>
#pragma hdrstop

#include "QueueItemDefinition.h"
#pragma package(smart_init)
#pragma resource "*.dfm"

/** constructor */
__fastcall TfrmBatchDefinition::TfrmBatchDefinition(TComponent* Owner, const AnsiString & ExecutableFilename, const AnsiString & ParameterFilename)
                               :TForm(Owner) {
  SetExecutableFilename(ExecutableFilename.c_str());
  SetParameterFilename(ParameterFilename.c_str());
}

/** launches browse dialog to select executable filename */
void __fastcall TfrmBatchDefinition::BrowseExecutableFilenameClick(TObject *Sender) {
  OpenDialog->FileName =  edtExecutableFileName->Text;
  OpenDialog->DefaultExt = "*.exe";
  OpenDialog->Filter = "Executables (*.exe)|*.exe";
  OpenDialog->FilterIndex = 0;
  OpenDialog->Title = "Select SaTScan Batch Executable";
  if (OpenDialog->Execute())
    SetExecutableFilename(OpenDialog->FileName.c_str());
}

/** launches browse dialog to select parameter filename */
void __fastcall TfrmBatchDefinition::BrowseParameterFilenameClick(TObject *Sender) {
  OpenDialog->FileName =  edtParameterFilename->Text;
  OpenDialog->DefaultExt = "*.prm";
  OpenDialog->Filter = "Parameter File (*.prm)|*.prm|Text files (*.txt)|*.txt|All files (*.*)|*.*";
  OpenDialog->FilterIndex = 0;
  OpenDialog->Title = "Select SaTScan Parameter File";
  if (OpenDialog->Execute()) {
    //AnsiString s;
    //s.printf("%s %s", OpenDialog->FileName.c_str(), edtCommandLineOptions->Text.c_str());
    SetParameterFilename(OpenDialog->FileName.c_str());
  }
}

/** enables Ok button for return dialog close */
void TfrmBatchDefinition::EnableOkButton() {
  btnOk->Enabled = edtExecutableFileName->Text.Length() && edtParameterFilename->Text.Length();
}

/** loads executable filename into interface */
void TfrmBatchDefinition::SetExecutableFilename(const char * sExecutableFilename) {
  edtExecutableFileName->Text = sExecutableFilename;
  gsExecutableFileName = sExecutableFilename;
  EnableOkButton();
}

/** loads parameter filename into interface */
void TfrmBatchDefinition::SetParameterFilename(const char * sParameterFilename) {
  edtParameterFilename->Text = sParameterFilename;
  gsParameterFileName = sParameterFilename;
  EnableOkButton();
}



//---------------------------------------------------------------------------
#include "stsSaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "stsFrmStartWindow.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmStartWindow *frmStartWindow;
//---------------------------------------------------------------------------
__fastcall TfrmStartWindow::TfrmStartWindow(TComponent* Owner) : TForm(Owner) {
  try {
    Setup();
  }
  catch (ZdException & x) {
    x.AddCallpath("constructor()","TfrmStartWindow");
    throw;
  }
}
//---------------------------------------------------------------------------
void __fastcall TfrmStartWindow::rdgOpenChoicesClick(TObject *Sender) {
  giSelectedIndex = rdgOpenChoices->ItemIndex;
}
//---------------------------------------------------------------------------
void TfrmStartWindow::Setup() {
  ZdString      sCaption;

  try {
    sCaption.printf("Welcome to SaTScan v%s", VERSION_NUMBER);
    Caption = sCaption.GetCString();
    rdgOpenChoices->Controls[2]->Enabled = GetToolkit().GetParameterHistory().size();
  }
  catch (ZdException & x) {
    x.AddCallpath("Setup()","TfrmStartWindow");
    throw;
  }
}


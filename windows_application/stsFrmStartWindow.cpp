//---------------------------------------------------------------------------
#include "stsSaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "stsFrmStartWindow.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

/** constructor */
__fastcall TfrmStartWindow::TfrmStartWindow(TComponent* Owner) : TForm(Owner), geOpenType(TfrmStartWindow::NEW) {
  try {
    Setup();
  }
  catch (ZdException & x) {
    x.AddCallpath("constructor()","TfrmStartWindow");
    throw;
  }
}

/** event triggered when 'cancel' button clicked */
void __fastcall TfrmStartWindow::btnCancelClick(TObject *Sender){
  geOpenType = TfrmStartWindow::CANCEL;
  Close();
}

/** event triggered when 'create new session' button clicked */
void __fastcall TfrmStartWindow::btnCreateNewClick(TObject *Sender) {
  geOpenType = TfrmStartWindow::NEW;
  Close();
}

/** event triggered when 'open last session' button clicked */
void __fastcall TfrmStartWindow::btnOpenLastClick(TObject *Sender) {
  geOpenType = TfrmStartWindow::LAST;
  Close();
}

/** event triggered when 'open saved session' button clicked */
void __fastcall TfrmStartWindow::btnOpenSavedClick(TObject *Sender){
  geOpenType = TfrmStartWindow::SAVED;
  Close();
}

/** event triggered when key pressed */
void __fastcall TfrmStartWindow::FormKeyDown(TObject *Sender, WORD &Key,TShiftState Shift) {
  if (Key == VK_ESCAPE) {
    geOpenType = TfrmStartWindow::CANCEL;
    Close();
  }  
}

/** internal setup function */
void TfrmStartWindow::Setup() {
  try {
    btnOpenLast->Enabled = GetToolkit().GetParameterHistory().size();
  }
  catch (ZdException & x) {
    x.AddCallpath("Setup()","TfrmStartWindow");
    throw;
  }
}




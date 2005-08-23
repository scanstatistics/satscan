//---------------------------------------------------------------------------
#include "stsSaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

TfrmOutputViewer *frmOutputViewer;
//---------------------------------------------------------------------------
__fastcall TfrmOutputViewer::TfrmOutputViewer(TComponent* Owner, char *sOutputFileName)
        : TForm(Owner)
{
   if (sOutputFileName)
     LoadFromFile(sOutputFileName);
}
//---------------------------------------------------------------------------
void __fastcall TfrmOutputViewer::btnCloseClick(TObject *Sender)
{
   Close();
}
//---------------------------------------------------------------------------
void __fastcall TfrmOutputViewer::FormClose(TObject *Sender,
      TCloseAction &Action)
{
   Action = caFree;
}
//---------------------------------------------------------------------------
void TfrmOutputViewer::AddLine(char *sLine)
{
  reOutputBox->Lines->Add(sLine);
}

void TfrmOutputViewer::LoadFromFile(char *sFileName)
{
   reOutputBox->Lines->LoadFromFile(sFileName);
   FFileName = sFileName;
}
void __fastcall TfrmOutputViewer::btnPrintClick(TObject *Sender)
{
if (! FFileName.IsEmpty())
      if ( PrintDialog1->Execute() ) reOutputBox->Print( FFileName );
}
//---------------------------------------------------------------------------


void __fastcall TfrmOutputViewer::FormCanResize(TObject *Sender,
      int &NewWidth, int &NewHeight, bool &Resize)
{
   //just need to adjust bottom of the box
  reOutputBox->Height =  NewHeight - 76;

}


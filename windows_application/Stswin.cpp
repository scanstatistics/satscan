//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Stswin.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmMainForm *frmMainForm;
//---------------------------------------------------------------------------
__fastcall TfrmMainForm::TfrmMainForm(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TfrmMainForm::sbEditSessionClick(TObject *Sender)
{
   try
      {
      new TfrmAnalysis(this);
      }
   catch (SSException & x)
      {
      x.AddCallpath("sbEditSessionClick", "TForm1");
      DisplayBasisException(x);
      }
}
//---------------------------------------------------------------------------
void __fastcall TfrmMainForm::New1Click(TObject *Sender)
{
   try
      {
      new TfrmAnalysis(this);
       }
   catch (SSException & x)
      {
      x.AddCallpath("New1Click", "TForm1");
      DisplayBasisException(x);
      }
}
//---------------------------------------------------------------------------

void __fastcall TfrmMainForm::Close1Click(TObject *Sender)
{
   if (ActiveMDIChild)
     ActiveMDIChild->Close();         
}
//---------------------------------------------------------------------------
void __fastcall TfrmMainForm::OpenAFile()
{
   try
      {
      OpenDialog1->FileName = "";
      OpenDialog1->DefaultExt = "*.prm";
      OpenDialog1->Filter = "Parameter Files (*.prm)|*.prm";   //put more types in here...
      OpenDialog1->Title = "Select Parameter File";
      if (OpenDialog1->Execute())
         {
         new TfrmAnalysis(this, OpenDialog1->FileName.c_str());
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("sbEditSessionClick", "TForm1");
      throw;
      }
}
//---------------------------------------------------------------------------

void __fastcall TfrmMainForm::sbOpenClick(TObject *Sender)
{
   try
      {
      OpenAFile();
      }
   catch (SSException & x)
      {
      x.AddCallpath("sbOpenClick", "TForm1");
      DisplayBasisException(x);
      }
}
//---------------------------------------------------------------------------

void __fastcall TfrmMainForm::AboutSatScan1Click(TObject *Sender)
{
   TfrmAbout *pAbout;
   pAbout = new TfrmAbout(this);
   pAbout->ShowModal();
   delete pAbout;
}
//---------------------------------------------------------------------------

void __fastcall TfrmMainForm::sbHelpClick(TObject *Sender)
{
   Application->HelpCommand(HELP_FINDER, 0L);        
}
//---------------------------------------------------------------------------

void __fastcall TfrmMainForm::Contents1Click(TObject *Sender)
{
   Application->HelpCommand(HELP_FINDER, 0L);        
}
//---------------------------------------------------------------------------

void __fastcall TfrmMainForm::UsingHelp1Click(TObject *Sender)
{
   WinHelp(0, "winhelp.hlp", HELP_HELPONHELP, 0L);        
}
//---------------------------------------------------------------------------

void __fastcall TfrmMainForm::sbExecuteClick(TObject *Sender)
{
   ExecuteSession();        
}
//---------------------------------------------------------------------------

void __fastcall TfrmMainForm::Execute1Click(TObject *Sender)
{
   ExecuteSession();
}
//---------------------------------------------------------------------------
//------------------------------------------------------------------------------
void TfrmMainForm::ExecuteSession()
{
   TfrmAnalysis *frmBaseForm;
   CParameters  *pSession;
   AnsiString    sCaption;
   //TseFrmReportViewer *pReportForm;

   try
      {
      //make sure window is a session window
      frmBaseForm = dynamic_cast<TfrmAnalysis *>(frmMainForm->ActiveMDIChild);
      if (frmBaseForm && frmBaseForm->ValidateParams())
         {
         pSession = (CParameters *)frmBaseForm->GetSession();
         if (pSession)
            {
            //see if you can open up the output file...  see if another Thread is using it !!1
            //access(pSession->m_szOutputFilename)
            if (frmBaseForm->GetFileName())
               {
               sCaption = "Running ";
               sCaption += frmBaseForm->GetFileName();
               }
            else
               sCaption = "Running Unknown Session Name";
            //new CalcThread(false, *pSession, "Some Thread Title");
            new CalcThread(false, *pSession, sCaption.c_str(), new TfrmAnalysisRun(0));
            }
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("sbOpenClick", "TForm1");
      DisplayBasisException(x);
      }
}
//---------------------------------------------------------------------------
void __fastcall TfrmMainForm::Save1Click(TObject *Sender)
{
   Save();
}
//---------------------------------------------------------------------------
void TfrmMainForm::Save()
{
   TfrmAnalysis *frmBaseForm;
   AnsiString    sFileName;

   try
      {
      frmBaseForm = dynamic_cast<TfrmAnalysis *>(frmMainForm->ActiveMDIChild);
      if (frmBaseForm)
         {
         sFileName =  frmBaseForm->GetFileName();
         if (sFileName.IsEmpty())
            frmBaseForm->SaveAs();
         else
            {
            // make sure that if the file exists it is NOT read only
            // also make sure that the directory can be written to...
            if (access(frmBaseForm->GetFileName(), 02) == 0)
               frmBaseForm->WriteSession();
            else
               MessageBox(NULL, "Can not save session.  The file is either read only or you do not have write privledges to the directory." ,"Error", MB_OK);
            }
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("Save", "TForm1");
      DisplayBasisException(x);
      }
}
//---------------------------------------------------------------------------
void TfrmMainForm::SaveAs()
{
   TfrmAnalysis *frmBaseForm;

   try
      {
      frmBaseForm = dynamic_cast<TfrmAnalysis *>(frmMainForm->ActiveMDIChild);
      if (frmBaseForm)
         {
         frmBaseForm->SaveAs();
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("SaveAs", "TForm1");
      throw;
      }
}
//---------------------------------------------------------------------------
void __fastcall TfrmMainForm::SaveAs1Click(TObject *Sender)
{
  SaveAs();        
}
//---------------------------------------------------------------------------
void TfrmMainForm::CreateViewer(char *sFileName)
{
   new TFormViewer(this);
}
//---------------------------------------------------------------------------
void __fastcall TfrmMainForm::sbSaveClick(TObject *Sender)
{
   Save();
}
//---------------------------------------------------------------------------

void __fastcall TfrmMainForm::PrintSetup1Click(TObject *Sender)
{
   PrinterSetupDialog1->Execute();
}
//---------------------------------------------------------------------------

void __fastcall TfrmMainForm::Print1Click(TObject *Sender)
{
/*   TfrmAnalysis    *frmBaseForm;
   TfrmAnalysisRun *frmAnalysisRun;
   CParameters     *pSession;
   AnsiString       sCaption;

   try
      {
      // YES.. YES.. YES...  I KNOW I SHOULD HAVE A NICE BASE FORM
      // AND NOT HAVE TO DO THIS DOUBLE DYNAMIC_CAST...   THIS WAS A LAST
      // MINUTE ADDITION...   AND I MEAN LAST MINUTE !!!!
      //
      frmBaseForm = dynamic_cast<TfrmAnalysis *>(frmMainForm->ActiveMDIChild);
      if (frmBaseForm)
         {
         pSession = (CParameters *)frmBaseForm->GetSession();
         //if (pSession)
         //   pSession->Print();
         }
      else
         {
         frmAnalysisRun = dynamic_cast<TfrmAnalysisRun *>(frmMainForm->ActiveMDIChild);
         //if (frmAnalysisRun)
         //    frmAnalysisRun->PrintTarget();
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("Print1Click", "TForm1");
      DisplayBasisException(x);
      }  */
}
//---------------------------------------------------------------------------

void __fastcall TfrmMainForm::ParameterFile1Click(TObject *Sender)
{
   try
      {
      OpenAFile();
      }
   catch (SSException & x)
      {
      x.AddCallpath("ParameterFile1Click", "TForm1");
      DisplayBasisException(x);
      }
}
//---------------------------------------------------------------------------

void __fastcall TfrmMainForm::OutputTextFile1Click(TObject *Sender)
{
   int  iHandle;
   long lFileLength;

   try
      {
      OpenDialog1->FileName = "";
      OpenDialog1->DefaultExt = "*.txt";
      OpenDialog1->Filter = "Output Files (*.txt)|*.txt";   //put more types in here...
      OpenDialog1->Title = "Select Output File";
      if (OpenDialog1->Execute())
         {
         //see if the file is too big... set some limit and adhere to it.
         iHandle = open(OpenDialog1->FileName.c_str(), O_RDONLY);
         lFileLength = filelength(iHandle);
         /* close the file */
         close(iHandle);
         if (lFileLength > 500000)
            MessageBox(NULL, "The output results file is too big for SaTScan to view.  Please use an independent text viewer to review the results file." ,"Warning", MB_OK);
         else
            new TfrmOutputViewer(this, OpenDialog1->FileName.c_str());
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("OutputTextFile1Click", "TForm1");
      throw;
      }
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------

#ifndef StswinH
#define StswinH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <Buttons.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
#include <io.h>
#include <stdio.h>
#include <fcntl.h>

#include "TfrmAnalysis.h"
#include "TfrmAbout.h"
#include "CalcThread.h"
#include "xmfrmAnalysisRun.h"
#include "xbDlgException.h"
#include "xmfrmFileViewer.h"
#include <Dialogs.hpp>

//---------------------------------------------------------------------------
class TfrmMainForm : public TForm
{
__published:	// IDE-managed Components
        TStatusBar *barStatusBar;
        TMainMenu *MainMenu1;
        TMenuItem *File1;
        TMenuItem *Help1;
        TMenuItem *Open1;
        TMenuItem *Save1;
        TMenuItem *Exit1;
        TMenuItem *Session1;
        TMenuItem *Execute1;
        TMenuItem *Contents1;
        TMenuItem *UsingHelp1;
        TMenuItem *N1;
        TMenuItem *AboutSatScan1;
        TMenuItem *Exit2;
        TBevel *Bevel1;
        TPanel *pnlSpeedButton;
        TSpeedButton *sbOpen;
        TSpeedButton *sbSave;
        TSpeedButton *sbHelp;
        TSpeedButton *sbExecute;
        TSpeedButton *sbEditSession;
        TBevel *Bevel2;
        TBevel *Bevel3;
        TBevel *Bevel4;
        TMenuItem *New1;
        TMenuItem *Close1;
        TMenuItem *N2;
        TMenuItem *SaveAs1;
        TMenuItem *Print1;
        TMenuItem *PrintSetup1;
        TMenuItem *N3;
        TOpenDialog *OpenDialog1;
        TSaveDialog *SaveDialog1;
        TPrintDialog *PrintDialog1;
        TPrinterSetupDialog *PrinterSetupDialog1;
        TMenuItem *ParameterFile1;
        TMenuItem *OutputTextFile1;
        void __fastcall sbEditSessionClick(TObject *Sender);
        void __fastcall New1Click(TObject *Sender);
        void __fastcall Close1Click(TObject *Sender);
        void __fastcall sbOpenClick(TObject *Sender);
        void __fastcall AboutSatScan1Click(TObject *Sender);
        void __fastcall sbHelpClick(TObject *Sender);
        void __fastcall Contents1Click(TObject *Sender);
        void __fastcall UsingHelp1Click(TObject *Sender);
        void __fastcall sbExecuteClick(TObject *Sender);
        void __fastcall Execute1Click(TObject *Sender);
        void __fastcall Save1Click(TObject *Sender);
        void __fastcall SaveAs1Click(TObject *Sender);
        void __fastcall sbSaveClick(TObject *Sender);
        void __fastcall PrintSetup1Click(TObject *Sender);
        void __fastcall Print1Click(TObject *Sender);
        void __fastcall ParameterFile1Click(TObject *Sender);
        void __fastcall OutputTextFile1Click(TObject *Sender);
private:	// User declarations
        void __fastcall OpenAFile();
        void            ExecuteSession();
        void            Save();
        void            SaveAs();
        
public:		// User declarations
        __fastcall TfrmMainForm(TComponent* Owner);
        void       CreateViewer(char *sFileName);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmMainForm *frmMainForm;
//---------------------------------------------------------------------------
#endif

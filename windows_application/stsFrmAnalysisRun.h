//---------------------------------------------------------------------------
#ifndef stsFrmAnalysisRunH
#define stsFrmAnalysisRunH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <ComCtrls.hpp>
#include <Controls.hpp>
#include <Dialogs.hpp>
#include <StdCtrls.hpp>
#include <Mapi.hpp>       // VCL Header
#include <Printers.hpp>   // VCL Header
#include <ExtCtrls.hpp>
#include <fcntl.h>
//---------------------------------------------------------------------------
class TfrmAnalysisRun : public TForm
{
__published:	// IDE-managed Components
        TRichEdit *reAnalysisBox;
        TPrintDialog *PrintDialog1;
        TSplitter *Splitter1;
        TPanel *Panel1;
        TRichEdit *reWarningsBox;
        TLabel *lbWarning;
        TPanel *Panel2;
        TButton *btnPrint;
        TButton *btnCancel;
        TButton *btnEMail;
        TRichEdit *RichEdit1;
        void __fastcall btnCancelClick(TObject *Sender);
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
        void __fastcall btnPrintClick(TObject *Sender);
        void __fastcall FormResize(TObject *Sender);
        void __fastcall btnEMailClick(TObject *Sender);
private:	// User declarations
         bool       gbCancel;
         AnsiString FFileName;
         bool       gbPrintWarnings;
public:		// User declarations
        __fastcall TfrmAnalysisRun(TComponent* Owner);
        void  AddLine(char *sLine);
        void  AddWarningLine(char *sLine);
        bool  IsJobCanceled();
        void  LoadFromFile(char *sFileName);
        void  CancelJob();
        void  SetPrintWarnings(bool bPrintWarnings);
        void  StopRun();
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmAnalysisRun *frmAnalysisRun;
//---------------------------------------------------------------------------
#endif

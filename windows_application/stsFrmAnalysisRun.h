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
//#include <Printers.hpp>   // VCL Header
#include <ExtCtrls.hpp>
#include <fcntl.h>
//---------------------------------------------------------------------------
//class CalcThread;
class stsOutputFileRegister;

class TfrmAnalysisRun : public TForm {
  friend class CalcThread;
  
  __published:	// IDE-managed Components
        TSplitter *Splitter1;
        TPanel *Panel1;
        TLabel *lbWarning;
        TPanel *Panel2;
        TButton *btnPrint;
        TButton *btnCancel;
        TButton *btnEMail;
        TRichEdit *rteAnalysisBox;
        TRichEdit *rteWarningsBox;
        TPrintDialog *PrintDialog;
        void __fastcall OnCancelClick(TObject *Sender);
        void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
        void __fastcall OnPrintClick(TObject *Sender);
        void __fastcall OnEMailClick(TObject *Sender);
  private:	// User declarations
    bool                        gbCancel;
    bool                        gbCanClose;
    stsOutputFileRegister     & gRegistry;
    std::string                 gsOutputFileName;

    void                Init();

  protected:
    void                AddLine(char *sLine);
    void                AddWarningLine(char *sLine);
    bool                IsJobCanceled() const {return gbCancel;}
    bool                GetCanClose() const {return gbCanClose;}
    void                LoadFromFile(char *sFileName);
    void                CancelJob();
    void                SetCanClose(bool b) {gbCanClose=b;}

  public:		// User declarations
            __fastcall TfrmAnalysisRun(TComponent* Owner, const std::string& sOutputFileName, stsOutputFileRegister & Registry);
    virtual __fastcall ~TfrmAnalysisRun(){}
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmAnalysisRun *frmAnalysisRun;
//---------------------------------------------------------------------------
#endif

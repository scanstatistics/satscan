//---------------------------------------------------------------------------
#ifndef stsFrmAnalysisRunH
#define stsFrmAnalysisRunH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <ComCtrls.hpp>
#include <Controls.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
#include <StdCtrls.hpp>
#include "Parameters.h"
#include "stsBaseAnalysisChildForm.h"

class stsOutputFileRegister;

class TfrmAnalysisRun : public stsBaseAnalysisChildForm  {
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
        void __fastcall FormActivate(TObject *Sender);
  private:	// User declarations
    bool                        gbCancel;
    bool                        gbCanClose;
    stsOutputFileRegister     & gRegistry;
    std::string                 gsOutputFileName;
    CalcThread                * gpAnalysisThread;  

    void                        Init();
    void                        Setup(const CParameters & Parameters);

  protected:
    void                        AddLine(char *sLine);
    void                        AddWarningLine(char *sLine);
    void                        CancelJob();
    virtual void                EnableActions(bool bEnable);
    void                        ForceThreadTermination();
    bool                        IsJobCanceled() const {return gbCancel;}
    void                        LoadFromFile(const char * sFileName);
    void                        SetCanClose(bool b) {gbCanClose=b;}

  public:		// User declarations
            __fastcall TfrmAnalysisRun(TComponent* Owner, const CParameters & Parameters, const std::string& sOutputFileName,
                                       stsOutputFileRegister & Registry, TActionList* theList);
    virtual __fastcall ~TfrmAnalysisRun();

    virtual void                CloseForm(bool bForce=false);
    bool                        GetCanClose() const {return gbCanClose;}
    void                        LaunchThread();
};
#endif

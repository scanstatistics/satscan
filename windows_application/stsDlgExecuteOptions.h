//******************************************************************************
#ifndef stsDlgExecuteOptionsH
#define stsDlgExecuteOptionsH
//******************************************************************************
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//******************************************************************************

class TfrmAnalysis; /** forward class declaration */

/** Options dialog permitting user to specify run-time options. */
class TdlgExecutionOptions : public TForm {
  __published:	// IDE-managed Components
    TPanel *Panel1;
    TPanel *Panel2;
    TGroupBox *grpProccessOptions;
    TRadioButton *rdoAllProcessors;
    TRadioButton *rdoAtMostProcesses;
    TStaticText *StaticText1;
    TEdit *edtAtMostProcesses;
    TButton *btnOk;
    TButton *btnCancel;
        TCheckBox *chkLogRunHistory;
    void __fastcall btnOkClick(TObject *Sender);
    void __fastcall edtAtMostProcessesAssociatedControlClick(TObject *Sender);
    void __fastcall edtAtMostProcessesExit(TObject *Sender);
    void __fastcall edtAtMostProcessesKeyPress(TObject *Sender, char &Key);

  private:	
    TfrmAnalysis       & gfrmAnalysis;

  public:
    __fastcall TdlgExecutionOptions(TComponent* Owner, TfrmAnalysis& frmAnalysis);
};
//******************************************************************************
#endif


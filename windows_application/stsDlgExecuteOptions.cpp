//******************************************************************************
#include "stsSaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "stsDlgExecuteOptions.h"
#include "stsFrmAnalysisParameters.h"
#pragma resource "*.dfm"

/** constructor */
__fastcall TdlgExecutionOptions::TdlgExecutionOptions(TComponent* Owner, TfrmAnalysis& frmAnalysis)
                                :TForm(Owner), gfrmAnalysis(frmAnalysis) {
  if (frmAnalysis.gParameters.GetNumRequestedParallelProcesses()) {
    edtAtMostProcesses->Text = frmAnalysis.gParameters.GetNumRequestedParallelProcesses();
    rdoAtMostProcesses->Checked = true;
  }
  else
    rdoAllProcessors->Checked = true;
  chkLogRunHistory->Checked = frmAnalysis.gParameters.GetIsLoggingHistory();
}

/** TButton OnClick event - sets CParameter object of TfrmAnalysis object. */
void __fastcall TdlgExecutionOptions::btnOkClick(TObject *Sender) {
  if (!edtAtMostProcesses->Text.Length() || edtAtMostProcesses->Text.Trim().ToInt() == 0)
    edtAtMostProcesses->Text = 1;

  if (rdoAllProcessors->Checked)
    gfrmAnalysis.gParameters.SetNumParallelProcessesToExecute(0);
  else
    gfrmAnalysis.gParameters.SetNumParallelProcessesToExecute(edtAtMostProcesses->Text.Trim().ToInt());
  gfrmAnalysis.gParameters.SetIsLoggingHistory(chkLogRunHistory->Checked);  
}

/** OnClick event - sets 'rdoAtMostProcesses' to checked and focused if not checked. */
void __fastcall TdlgExecutionOptions::edtAtMostProcessesAssociatedControlClick(TObject *Sender) {
  if (!rdoAtMostProcesses->Checked) {
    rdoAtMostProcesses->Checked = true;
    rdoAtMostProcesses->SetFocus();
  }
}

/** TEdit OnExit event - resets text to '1' if blank or equal to zero. */
void __fastcall TdlgExecutionOptions::edtAtMostProcessesExit(TObject *Sender) {
  if (!edtAtMostProcesses->Text.Length() || edtAtMostProcesses->Text.Trim().ToInt() == 0)
    edtAtMostProcesses->Text = 1;
}

/** TEdit key press filter. */
void __fastcall TdlgExecutionOptions::edtAtMostProcessesKeyPress(TObject *Sender, char &Key) {
  if (!strchr("0123456789\b", Key))
    Key = 0;
}

void __fastcall TdlgExecutionOptions::edtAtMostProcessesEnter(TObject *Sender) {
  if (!rdoAtMostProcesses->Checked) rdoAtMostProcesses->Checked = true;
}


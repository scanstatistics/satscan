#include "stsSaTScan.h"
#pragma hdrstop

#include "stsBaseAnalysisChildForm.h"

const char*  CATEGORY_ALL                       = "All";
const char*  CATEGORY_ANALYSIS                  = "Analysis";
const char*  CATEGORY_ANALYSIS_RUN              = "AnalysisRun";
const char*  CATEGORY_ANALYSIS_RUN_PRINT        = "AnalysisRunPrint";

__fastcall stsBaseAnalysisChildForm::stsBaseAnalysisChildForm(TComponent* Owner, TActionList* pList) : TForm(Owner), gpList(pList) {
}

__fastcall stsBaseAnalysisChildForm::~stsBaseAnalysisChildForm() {
   if (frmMainForm->MDIChildCount - 1 == 0) {
     for(int i = 0; i < gpList->ActionCount; ++i) {
        TAction* pAction = dynamic_cast<TAction*>(gpList->Actions[i]);
        if (pAction) {
           if(pAction->Category == CATEGORY_ALL)
               pAction->Enabled = true;
           else
               pAction->Enabled = false;
        }
     }
   }  
}
#include "stsSaTScan.h"
#pragma hdrstop

#include "stsBaseAnalysisChildForm.h"

const char*  CATEGORY_ALL          = "All";
const char*  CATEGORY_ANALYSIS     = "Analysis";
const char*  CATEGORY_ANALYSIS_RUN = "AnalysisRun";

__fastcall stsBaseAnalysisChildForm::stsBaseAnalysisChildForm(TComponent* Owner, TActionList* pList) : TForm(Owner), gpList(pList) {
}

__fastcall stsBaseAnalysisChildForm::~stsBaseAnalysisChildForm() {
   for(int i = 0; i < gpList->ActionCount; ++i) {
      TAction* pAction = dynamic_cast<TAction*>(gpList->Actions[i]);
      if (pAction) {
         if(pAction->Category == CATEGORY_ALL)
             pAction->Enabled = true;
         else if(pAction->Category == CATEGORY_ANALYSIS_RUN || pAction->Category == CATEGORY_ANALYSIS)
             pAction->Enabled = false;
      }
   }
}
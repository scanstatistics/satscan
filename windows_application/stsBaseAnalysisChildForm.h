# ifndef stsBaseAnalysisChildForm_H 
# define stsBaseAnalysisChildForm_H

#include <ActnList.hpp>

class stsBaseAnalysisChildForm : public TForm {
   public :
      virtual __fastcall stsBaseAnalysisChildForm(TComponent* Owner, TActionList* pList);
      virtual __fastcall ~stsBaseAnalysisChildForm();

   protected :
      TActionList	*gpList;

      virtual void EnableActions(bool bEnable) = 0;
};

static const AnsiString  CATEGORY_ALL            = "All";
static const AnsiString  CATEGORY_ANALYSIS       = "Analysis";
static const AnsiString  CATEGORY_ANALYSIS_RUN   = "AnalysisRun";

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

#endif
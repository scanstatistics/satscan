# ifndef stsBaseAnalysisChildForm_H 
# define stsBaseAnalysisChildForm_H

#include <ActnList.hpp>

extern const char*  CATEGORY_ALL;
extern const char*  CATEGORY_ANALYSIS;
extern const char*  CATEGORY_ANALYSIS_RUN;

class stsBaseAnalysisChildForm : public TForm {
   public :
      virtual __fastcall stsBaseAnalysisChildForm(TComponent* Owner, TActionList* pList);
      virtual __fastcall ~stsBaseAnalysisChildForm();

      virtual void      CloseForm(bool bForce=false) {Close();}

   protected :
      TActionList     * gpList;

      virtual void      EnableActions(bool bEnable) = 0;
};

#endif
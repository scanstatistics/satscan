# ifndef stsBaseAnalysisChildForm_H 
# define stsBaseAnalysisChildForm_H

#include <ActnList.hpp>

extern const char*  CATEGORY_ALL;
extern const char*  CATEGORY_ANALYSIS;
extern const char*  CATEGORY_ANALYSIS_RUN;
extern const char*  CATEGORY_ANALYSIS_RUN_PRINT;

class stsBaseAnalysisChildForm : public TForm {
   public :
      virtual __fastcall stsBaseAnalysisChildForm(TComponent* Owner, TActionList* pList);
      virtual __fastcall ~stsBaseAnalysisChildForm();

   protected :
      TActionList     * gpList;

      virtual void      EnableActions(bool bEnable) = 0;
};

#endif
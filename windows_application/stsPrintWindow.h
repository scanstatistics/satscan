//---------------------------------------------------------------------------
#ifndef stsPrintWindowH
#define stsPrintWindowH
//---------------------------------------------------------------------------
#include "BasePrint.h"
//---------------------------------------------------------------------------
class PrintWindow : public BasePrint
{
   private:
      TfrmAnalysisRun *gpAnalysisRunForm;
   public:
      PrintWindow(TfrmAnalysisRun *pFormStatus);
      ~PrintWindow();

      bool GetIsCanceled();
      void PrintLine(char *s);
      void PrintWarningLine(char *s);
};
//---------------------------------------------------------------------------
#endif

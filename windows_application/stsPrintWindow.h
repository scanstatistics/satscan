//---------------------------------------------------------------------------
#ifndef stsPrintWindowH
#define stsPrintWindowH
//---------------------------------------------------------------------------
#include "BasePrint.h"
//---------------------------------------------------------------------------
class CalcThread;
class PrintWindow : public BasePrint {
  private:
    CalcThread        & gCalculationThread;

   public:
     PrintWindow(CalcThread & CalculationThread);
     ~PrintWindow();

     bool               GetIsCanceled();
     void               PrintLine(char *s);
     void               PrintWarningLine(char *s);
};
//---------------------------------------------------------------------------
#endif

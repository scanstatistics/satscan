//---------------------------------------------------------------------------
#ifndef stsPrintWindowH
#define stsPrintWindowH
//---------------------------------------------------------------------------
#include "BasePrint.h"
//---------------------------------------------------------------------------

/** Print direction class that quietly suppresses printing messages. */
class PrintNull : public BasePrint {
   public:
     PrintNull() : BasePrint() {}
     ~PrintNull() {}

     bool               GetIsCanceled() const {return false;}
     void               PrintLine(char*) {}
     void               PrintWarningLine(char*) {}
};

/** Print direction class that directs messages to calculation thread so
    that communication with analysis run window is synchronized with main
    VCL thread. */
class CalcThread;
class PrintWindow : public BasePrint {
  private:
    CalcThread        & gCalculationThread;

   public:
     PrintWindow(CalcThread & CalculationThread);
     ~PrintWindow();

     bool               GetIsCanceled() const;
     void               PrintLine(char *s);
     void               PrintWarningLine(char *s);
};
//---------------------------------------------------------------------------
#endif

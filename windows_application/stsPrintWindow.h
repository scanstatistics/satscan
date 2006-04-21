//******************************************************************************
#ifndef stsPrintWindowH
#define stsPrintWindowH
//******************************************************************************
#include "BasePrint.h"

/** Print direction class that directs messages to calculation thread so
    that communication with analysis run window is synchronized with main
    VCL thread. */
class CalcThread;
class PrintWindow : public BasePrint {
  protected:
    CalcThread        & gCalculationThread;

    virtual void        PrintError(const char * sMessage);
    virtual void        PrintNotice(const char * sMessage);
    virtual void        PrintStandard(const char * sMessage);
    virtual void        PrintWarning(const char * sMessage);

   public:
     PrintWindow(CalcThread & CalculationThread, bool bSuppressWarnings);
     virtual ~PrintWindow();

     bool               GetIsCanceled() const;
     virtual void       Printf(const char * sMessage, PrintType ePrintType, ...);
};
//******************************************************************************
#endif


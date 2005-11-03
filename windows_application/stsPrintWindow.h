//---------------------------------------------------------------------------
#ifndef stsPrintWindowH
#define stsPrintWindowH
//---------------------------------------------------------------------------
#include "BasePrint.h"
//---------------------------------------------------------------------------

/** Print direction class that quietly suppresses printing messages. */
class PrintNull : public BasePrint {
   protected:
    virtual void        PrintError(const char * sMessage) {}
    virtual void        PrintNotice(const char * sMessage) {}
    virtual void        PrintStandard(const char * sMessage) {}
    virtual void        PrintWarning(const char * sMessage) {}

   public:
     PrintNull(bool bSuppressWarnings=true) : BasePrint(bSuppressWarnings) {}
     virtual ~PrintNull() {}

     bool               GetIsCanceled() const {return false;}
     virtual void       Print(const char * sMessage, PrintType ePrintType) {}
     virtual void       Printf(const char * sMessage, PrintType ePrintType, ...) {}
};

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
     virtual void       Print(const char * sMessage, PrintType ePrintType);
     virtual void       Printf(const char * sMessage, PrintType ePrintType, ...);
};
//---------------------------------------------------------------------------
#endif

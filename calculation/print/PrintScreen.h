//*****************************************************************************
#ifndef __PrintScreen_H
#define __PrintScreen_H
//*****************************************************************************
#include "BasePrint.h"

class PrintScreen : public BasePrint {
  protected:
    inline void PrintError(const char * sMessage) {printf(sMessage);}
    inline void PrintNotice(const char * sMessage) {printf(sMessage);}
    inline void PrintStandard(const char * sMessage) {printf(sMessage);}
    inline void PrintWarning(const char * sMessage) {printf(sMessage);}
  
  public:
    PrintScreen(bool bSuppressWarnings);
    virtual ~PrintScreen();

    inline bool GetIsCanceled() const {return false;}
};
//*****************************************************************************
#endif

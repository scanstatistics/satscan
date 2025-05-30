//*****************************************************************************
#ifndef __PrintScreen_H
#define __PrintScreen_H
//*****************************************************************************
#include "BasePrint.h"

class PrintScreen : public BasePrint {
  public:
    PrintScreen(bool bSuppressWarnings);
    virtual ~PrintScreen();

    inline void PrintError(const char * sMessage) { fprintf(stderr, "%s", sMessage); }
    inline void PrintNotice(const char * sMessage) { fprintf(stderr, "%s", sMessage); }
    inline void PrintStandard(const char * sMessage) { fprintf(stdout, "%s", sMessage); }
    inline void PrintWarning(const char * sMessage) { fprintf(stderr, "%s", sMessage); }

    inline bool GetIsCanceled() const {return false;}
};
//*****************************************************************************
#endif

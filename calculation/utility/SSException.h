//*****************************************************************************
#ifndef  __EXCEPTIONCLASS_H
#define  __EXCEPTIONCLASS_H
//*****************************************************************************
#include "zd540.h"

/** SaTScan exception class
    ZdException : program error, unexpected program problem
    SSException and derived : an issue that the user can resolve
                              problem with user input, data, etc.  */
class SSException : public virtual ZdException {
  public:
    SSException ( va_list varArgs, const char *sMessage, const char *sSourceModule, Level iLevel );
    virtual ~SSException();
};

void SSGenerateException(const char * sMessage, const char * sSourceModule, ... );
//*****************************************************************************
#endif

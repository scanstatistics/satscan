//*****************************************************************************
#ifndef  __EXCEPTIONCLASS_H
#define  __EXCEPTIONCLASS_H
//*****************************************************************************
#include "zd543.h"

/** SaTScan exception class
    ZdException : program error, unexpected program problem
    SSException and derived : an issue that the user can resolve
                              problem with user input, data, etc.  */
class SSException : public ZdException {
  public:
    SSException ( va_list varArgs, const char *sMessage, const char *sSourceModule, Level iLevel );
};

void SSGenerateException(const char * sMessage, const char * sSourceModule, ... );


class UsageException : public ZdException {
  public:
    UsageException();
};

void GenerateUsageException();
//*****************************************************************************
#endif

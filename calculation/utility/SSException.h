//*****************************************************************************
#ifndef  __EXCEPTIONCLASS_H
#define  __EXCEPTIONCLASS_H
//*****************************************************************************
#include "zd543.h"

/** SaTScan exception class
    ZdException : program error, unexpected program problem
    ResolvableException and derived : an issue that the user can resolve
                                      problem with user input, data, etc.  */
class ResolvableException : public ZdException {
  public:
    ResolvableException( va_list varArgs, const char *sMessage, const char *sSourceModule, Level iLevel );
};

void GenerateResolvableException(const char * sMessage, const char * sSourceModule, ... );


class UsageException : public ZdException {
  public:
    UsageException(const char * sExecutableFullpathName);
};

void GenerateUsageException(const char * sExecutableFullpathName);
//*****************************************************************************
#endif

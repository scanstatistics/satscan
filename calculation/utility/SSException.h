//*****************************************************************************
#ifndef  __EXCEPTIONCLASS_H
#define  __EXCEPTIONCLASS_H
//*****************************************************************************
#include "SaTScan.h"

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

class program_error : public std::exception {
   protected:
     std::string                __what;
     std::string                __trace;

   public:
     program_error(const std::string& what_arg);
     program_error(const char * format, ...);
     virtual ~program_error () throw() {}

     void                       addTrace(const char * file, int line);
     virtual const char       * trace () const throw() {return __trace.c_str();}
     virtual const char       * what () const throw() {return __what.c_str();}
};

#define appendTrace(x)   x.addTrace(__FILE__, __LINE__)
//*****************************************************************************
#endif

//*****************************************************************************
#ifndef  __EXCEPTIONCLASS_H
#define  __EXCEPTIONCLASS_H
//*****************************************************************************
#include <algorithm>
#include <cstdarg>
#include <cmath>
#include <deque>
#include <functional>
#include <map>
#include <set>
#include <strstream>
#include <stdexcept>
#include <typeinfo>
#include <vector>
#include <stdio.h>
#include "zd540.h"

#ifdef INTEL_BASED
   #include <limits>     //*** Solaris doesn't have the C++ limits!
   #include <new.h>
   #include <windows.h>
   #include <sys/stat.h>
#else
   #include <synch.h>
   #include <errno.h>
   #include <sys/stat.h>

   //extern char*   strupr(char* s);
   //extern char*   strrev(char* s);
#endif


//--- Should probably have : virtual public std::exception
class SSException : public virtual ZdException
{
  public:
   SSException ( const char * sMessage, const char * sSourceModule, Level iLevel, ... );
   SSException ( const char * sMessage = 0, const char * sSourceModule = 0 );
   SSException ( va_list varArgs, const char *sMessage, const char *sSourceModule, Level iLevel );
   virtual ~SSException();

   static void GenerateNotification ( const char *sMessage, const char *sSourceModule, ... );
   static void GenerateWarning ( const char *sMessage, const char *sSourceModule, ... );

};

void SSGenerateException(const char * sMessage, const char * sSourceModule, ... );
void SSGenerateWarning(const char * sMessage, const char * sSourceModule, ... );
void ssGenerateOSException ( const char *sMessage, bool bPrepend, const char *sSourceModule, ... );

//*****************************************************************************
#endif

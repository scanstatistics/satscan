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
class SSException
{
public:
   enum Level { Normal, Warning, Critical, Notify};

protected:
   char              * gsMessage;
   char              * gsModule;
   char              * gsCallpath;
   int               giLevel;

   void              Copy(const SSException & rhs);
   void              Init();
   void              SetData ( va_list varArgs, const char *sMessage, const char *sSourceModule, Level iLevel );

   // For standard exceptions,
   virtual const char *what() const throw();

public:

   // constructors
   SSException ( const std::exception &theMessage, const char *sSourceFunction, const char *sSourceModule );
   SSException(const char * sMessage = 0, const char * sSourceModule = 0, Level iLevel = Normal, ... );
   SSException ( va_list varArgs, const char *sMessage, const char *sSourceModule, Level iLevel );
   SSException(const SSException &rhs);
   virtual ~SSException();

   SSException & operator= (const SSException &rhs);

   void                    AddCallpath(const char * sFunction, const char * sClass = 0, const char * sMessage = 0);
   void                    AddMessage(const char * sMessage, bool bPrepend);
   void                    AddOSError( bool bPrepend );
   const char *            GetCallpath() const;
   const char *            GetErrorMessage() const;
   Level                   GetLevel() const;
   const char *            GetSourceModule() const;
   void                    SetCallpath(const char * sCallpath);
   void                    SetLevel(Level iLevel);
   void                    SetErrorMessage(const char * sMessage, ... );
   void                    SetErrorMessage ( va_list varArgs, const char *sMessage );
   void                    SetErrorMessageExplicit( const char * sMessage );
   void                    SetSourceModule(const char * sSourceModule);

   static void GenerateNotification ( const char *sMessage, const char *sSourceModule, ... );
   static void GenerateWarning ( const char *sMessage, const char *sSourceModule, ... );
   static void Generate ( const char *sMessage, const char *sSourceModule,  ... );
};

void SSGenerateException(const char * sMessage, const char * sSourceModule, ... );
void SSGenerateWarning(const char * sMessage, const char * sSourceModule, ... );
void ssGenerateOSException ( const char *sMessage, bool bPrepend, const char *sSourceModule, ... );

//*****************************************************************************
#endif

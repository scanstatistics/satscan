#pragma hdrstop
#include "SSException.h" 
//------------------------------------------------------------------------------
// THIS IS JUST A COPY OF THE ZD EXCEPTION CLASS...   THIS PROJECT DOES NOT
// USE ZD, BUT THE EXCEPTION CLASS IS NICE AND WAS INCLUDED HERE...
//  ALL I DID WAS RENAME THE CLASS...  SINCE I COULD NOT DERIVE IT OFF OF ZD
//------------------------------------------------------------------------------
//Author Kevin J. Meagher

//ClassDesc Begin ZdException
// This is the class for all ZdExceptions.  It contains two strings, a message and a module,
// both of which are defined by the calling class.  It also provides a traceback method though
// the gsCallpath string for Zd runtime information.  All ZdExceptions should be generated
// through the ZdGenerateException C function.
//ClassDesc End

// This constructor extracts what() from a std::exception and sets the message
// of the ZdException to that value.
SSException::SSException ( const std::exception &theMessage, const char *sSourceFunction, const char *sSourceModule )
{
   Init();
   if ( theMessage.what() )
      SetErrorMessageExplicit ( theMessage.what() );

   if (sSourceModule)
      SetSourceModule(sSourceModule);

   if ( sSourceFunction )
      AddCallpath ( sSourceFunction, sSourceModule );
}
// Constructor. sMessage may be an sprintf-style string ( e.g. "%d out of %d" ).
// If it is, the arguments for sMessage occur after iLevel. For example,
//
// ZdException a ( "%d out of %d", "ZdClass", ZdException::Normal, 12, 13 );
SSException::SSException( const char * sMessage, const char * sSourceModule, Level iLevel, ... )
{
   va_list   varArgs;

   Init();
   va_start ( varArgs, iLevel );
   SetData ( varArgs, sMessage, sSourceModule, iLevel );
   va_end ( varArgs );
}

// The constructor takes two optional arguments, a message and the module that is throwing the
// exception.  Either or both of these can be ommitted. The function allows the sMessage parameter
// to be an sprintf-style string. The arguments for the string are supplied by varArgs.
SSException::SSException(va_list varArgs, const char * sMessage, const char * sSourceModule, Level iLevel )
{
   Init();
   SetData ( varArgs, sMessage, sSourceModule, iLevel );
}

//Copy constructor
SSException::SSException(const SSException &rhs)
{
   Init();
   Copy(rhs);
}

//Overloaded assignment operator
SSException &SSException::operator= (const SSException &rhs)
{
   if (this != &rhs)
      Copy(rhs);
   return (*this);
}

SSException::~SSException()
{
   delete [] gsMessage;
   delete [] gsModule;
   delete [] gsCallpath;
}

// This function will add the function name sFunction to the current callpath.  The two parameters
// sClass and sMessage are optional and provide additional debug information.
void SSException::AddCallpath(const char * sFunction, const char * sClass, const char * sMessage)
{
   char     * sTemp;
   int      iNewLength;

   // Calculate the space needed to add these strings to the callpath
   iNewLength = strlen(gsCallpath) + 32;    // extra for literal text added below in sprintf
   if (sFunction)
      iNewLength += strlen(sFunction);
   if (sClass)
      iNewLength += strlen(sClass);
   if (sMessage)
      iNewLength += strlen(sMessage);
   try
      {
      // Allocate new space for additional callback information
      sTemp = new char[iNewLength];
      // Copy existing callpath to the new space
      strcpy(sTemp, gsCallpath);
      if (strlen(sTemp))
         strcat(sTemp, "\r\n");
      // delete the old call path
      delete [] gsCallpath;
      gsCallpath = 0;
      // append the parameters to the existing callpath
      if (sFunction)
         strcat(sTemp, sFunction);
      if (sClass)
         sprintf(sTemp + strlen(sTemp), " of %s", sClass);
      if (sMessage)
         sprintf(sTemp + strlen(sTemp), " (%s)", sMessage);
      // all done, make this the official callpath
      gsCallpath = sTemp;
      }
   catch (...) {};
}

// This function will add the string sMessage to the end of current exception message.  No
// additional characters are added between the two messages.
void SSException::AddMessage(const char * sMessage, bool bPrepend)
{
   char * sTemp;

   try
      {
      if (sMessage)
         {
         sTemp = new char[strlen(gsMessage) + strlen(sMessage) + 2];  // NULL
         if (bPrepend)
            sprintf(sTemp, "%s%s", sMessage, gsMessage);
         else // append
            sprintf(sTemp, "%s%s", gsMessage, sMessage);
         delete [] gsMessage;
         gsMessage = sTemp;
         }
      }
   catch (...) {};
}

#ifdef INTEL_BASED
// This function appends the last OS error message to the exception string.
void SSException::AddOSError ( bool bPrepend )
{
   char sError[4096];

   FormatMessage ( FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, sError, 4096, 0 );
   AddMessage ( sError, bPrepend );
}
#else
// This function appends the last OS error message to the exception string.
void SSException::AddOSError ( bool bPrepend )
{
   AddMessage ( strerror ( errno ), bPrepend );
}
#endif

// Internal function to copy a ZdException
void SSException::Copy(const SSException & rhs)
{
   SetErrorMessageExplicit(rhs.gsMessage);
   SetSourceModule(rhs.gsModule);
   SetCallpath(rhs.gsCallpath);
   SetLevel((Level)rhs.giLevel);
}

// This function is used to returns a pointer to the function history string.
const char * SSException::GetCallpath() const
{
   return gsCallpath;
}

// This function is used to returns a pointer to the message string.
const char * SSException::GetErrorMessage() const
{
   return gsMessage;
}

// This function returns the exception level.
SSException::Level SSException::GetLevel() const
{
   return (Level)giLevel;
}

// This function is used to returns a pointer to the module string.
const char * SSException::GetSourceModule() const
{
   return gsModule;
}

// This internal function initializes the class variables.
void SSException::Init()
{
   gsMessage  = new char[1];
   gsModule   = new char[1];
   gsCallpath = new char[1];
   gsMessage[0] = gsModule[0] = gsCallpath[0] = 0;
}

// This function sets the current exception callpath.  If a NULL is passed in, the current
// callpath is cleared.
void SSException::SetCallpath(const char * sCallpath)
{
   try
      {
      if (sCallpath && (sCallpath != gsCallpath) )
         {
         delete [] gsCallpath;
         gsCallpath = 0;
         gsCallpath = new char[strlen(sCallpath) + 1];
         strcpy(gsCallpath, sCallpath);
         }
      else
         gsCallpath[0] = 0;
      }
   catch (...) {};
}

// Internal function which performs the actual function of the two var arg
// constructors.
void SSException::SetData ( va_list varArgs, const char *sMessage, const char *sSourceModule, Level iLevel )
{
   if (sMessage)
      SetErrorMessage(varArgs, sMessage);
   if (sSourceModule)
      SetSourceModule(sSourceModule);
   SetLevel(iLevel);
}

// sprintf-style SetErrorMessage. For example,
//
// SetErrorMessage ( "%d bad in \"%s\"", 14, "Bob" );
void SSException::SetErrorMessage(const char * sMessage, ... )
{
   va_list   varArgs;

   va_start ( varArgs, sMessage );
   SetErrorMessage ( varArgs, sMessage );
   va_end ( varArgs );
}

//*** There are two different versions of this function because vsnprintf behaves
//*** in completely different ways on the two platforms. Under BCB 5, vsnprintf()
//*** will return the length of the required buffer if the length parameter is
//*** zero and otherwise returns the number of bytes output. Under Solaris,
//*** vsnprintf() will return -1 if a zero is passed and otherwise _always_
//*** returns the number of bytes output.

#if defined ( INTEL_BASED )
// This function sets the current exception message.  If a NULL is passed in, the current
// message is cleared.
void SSException::SetErrorMessage(va_list varArgs, const char * sMessage )
{
   int   iCurrentLength;  // Current length of the buffer
   int   iStringLength;   // Holds the length of the formatted output

   try
      {
      if (sMessage && (sMessage != gsMessage) )
         {
         iCurrentLength = strlen ( gsMessage );

         // vsnprintf will calculate the required length, not including the NULL,
         // for the format string when given a NULL pointer and a zero length as
         // the first two parameters.
         iStringLength = vsnprintf ( 0, 0, sMessage, varArgs );

         if ( iStringLength > iCurrentLength )
            {
            delete [] gsMessage; gsMessage = 0;
            gsMessage = new char[iStringLength + 1];
            }

         vsnprintf ( gsMessage, iStringLength + 1, sMessage, varArgs );
         }
      else
         gsMessage[0] = 0;
      }
   catch (...) {};
}
#else
// This function sets the current exception message.  If a NULL is passed in, the current
// message is cleared.
void SSException::SetErrorMessage(va_list varArgs, const char * sMessage )
{
   int   iCurrentLength;  // Current length of the buffer
   int   iStringLength;   // Holds the length of the formatted output

   try
      {
      if (sMessage && (sMessage != gsMessage) )
         {
         iCurrentLength = strlen ( gsMessage );

         // vsnprintf will always return the length needed to format the string.
         iStringLength = vsnprintf ( gsMessage, iCurrentLength + 1, sMessage, varArgs );

         if ( iStringLength > iCurrentLength )
            {
            delete [] gsMessage; gsMessage = 0;
            gsMessage = new char[iStringLength + 1];

            vsnprintf ( gsMessage, iStringLength + 1, sMessage, varArgs );
            }
         }
      else
         gsMessage[0] = 0;
      }
   catch (...) {};
}
#endif

// non-sprintf-style SetErrorMessage.
void SSException::SetErrorMessageExplicit(const char * sMessage )
{
   delete[] gsMessage;
   gsMessage = new char[strlen(sMessage) + 1];
   strcpy(gsMessage, sMessage);
}

// This function sets the level of the exception
// This function returns the exception level.
void SSException::SetLevel(Level iLevel)
{
   giLevel = iLevel;
}

// This function sets the current exception module.  If a NULL is passed in, the current
// module is cleared.
void SSException::SetSourceModule(const char * sSourceModule)
{
   try
      {
      if (sSourceModule && (sSourceModule != gsModule) )
         {
         delete [] gsModule;
         gsModule = 0;
         gsModule = new char[strlen(sSourceModule) + 1];
         strcpy(gsModule, sSourceModule);
         }
      else
         gsModule[0] = 0;
      }
   catch (...) {};
}

// Overloaded what() from std::exception. Returns GetErrorMessage().
const char *SSException::what() const throw()
   {
      return gsMessage;
   }

void SSException::Generate ( const char *sMessage, const char *sSourceModule, ... )
{
   va_list      varArgs;
   va_start ( varArgs, sSourceModule );

   SSException  theException ( varArgs, sMessage, sSourceModule, Normal );
   va_end ( varArgs );

   throw theException;
}

void SSException::GenerateNotification ( const char *sMessage, const char *sSourceModule, ... )
{
   va_list      varArgs;
   va_start ( varArgs, sSourceModule );

   SSException  theException ( varArgs, sMessage, sSourceModule, Notify );
   va_end ( varArgs );

   throw theException;
}

void SSException::GenerateWarning ( const char *sMessage, const char *sSourceModule, ... )
{
   va_list      varArgs;
   va_start ( varArgs, sSourceModule );

   SSException  theException ( varArgs, sMessage, sSourceModule, Warning );
   va_end ( varArgs );

   throw theException;
}

// This function will throw the exception with the parameters.  It is equivalent to
// throw ZdException(...), but includes the ability to format the message string.
// This function should be used to generate all Zd Exceptions within ZD.
void SSGenerateException(const char * sMessage, const char * sSourceModule, ... )   // DCH
{
   va_list      varArgs;
   va_start ( varArgs, sSourceModule );

   SSException  theException ( varArgs, sMessage, sSourceModule, SSException::Normal );
   va_end ( varArgs );

   throw theException;
}

void ssGenerateOSException ( const char *sMessage, bool bPrepend, const char *sSourceModule, ... )
{
   va_list      varArgs;
   va_start ( varArgs, sSourceModule );

   SSException  theException ( varArgs, sMessage, sSourceModule, SSException::Normal );
   theException.AddOSError ( bPrepend);
   va_end ( varArgs );

   throw theException;
}

// This function will throw the exception with the parameters.  It is equivalent to
// throw ZdException(...), but includes the ability to format the message string.
// This function should be used to generate all Zd Exceptions within ZD.
void SSGenerateWarning(const char * sMessage, const char * sSourceModule, ... )   // DCH
{
   va_list      varArgs;
   va_start ( varArgs, sSourceModule );

   SSException  theException ( varArgs, sMessage, sSourceModule, SSException::Warning );
   va_end ( varArgs );

   throw theException;
}



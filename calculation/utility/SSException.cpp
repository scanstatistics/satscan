#include "SaTScan.h"
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


// Constructor. This construct allows sprintf-style message. The arguments
// for the message occur after iLevel. For example,
//
// throw BException ( "Record %d of \"%s\" bad", "BFileModel", ZdException::Normal, 13, "answer.txd" );
SSException::SSException(const char * sMessage, const char * sSourceModule, Level iLevel, ... )
          : ZdException ( 0, sSourceModule, iLevel ) {
   va_list   varArgs;

   va_start ( varArgs, iLevel );
   SetData ( varArgs, sMessage, sSourceModule, iLevel );
   va_end ( varArgs );
}

// Constructor. This construct does not allow sprintf-style messages. Level defaults
// to Normal.
SSException::SSException(const char * sMessage, const char * sSourceModule )
          : ZdException ( sMessage, sSourceModule, ZdException::Normal ) {
   SetErrorMessage(sMessage);
   SetSourceModule(sSourceModule);
}

// Construct. This is an alternate constructor for when the varArgs list for sMessage
// has already been prepared. Primarily, this will be used by derived classes.
SSException::SSException ( va_list varArgs, const char *sMessage, const char *sSourceModule, Level iLevel )
          : ZdException ( varArgs, sMessage, sSourceModule, iLevel ) {
   SetData ( varArgs, sMessage, sSourceModule, iLevel );
}

// Destructor.
SSException::~SSException() {}

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



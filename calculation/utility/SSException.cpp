#include "SaTScan.h"
#pragma hdrstop
#include "SSException.h" 

/**  Construct. This is an alternate constructor for when the varArgs list for sMessage
     has already been prepared. Primarily, this will be used by derived classes.        */
SSException::SSException ( va_list varArgs, const char *sMessage, const char *sSourceModule, Level iLevel )
          : ZdException ( varArgs, sMessage, sSourceModule, iLevel ) {
   SetData ( varArgs, sMessage, sSourceModule, iLevel );
}

/** Destructor. */
SSException::~SSException() {}

/**  This function will throw the exception with the parameters.  It is equivalent to
     throw ZdException(...), but includes the ability to format the message string.
     This function should be used to generate all Zd Exceptions within ZD.             */
void SSGenerateException(const char * sMessage, const char * sSourceModule, ... ) {
  va_list      varArgs;
  va_start ( varArgs, sSourceModule );

  SSException  theException ( varArgs, sMessage, sSourceModule, SSException::Normal );
  va_end ( varArgs );

  throw theException;
}


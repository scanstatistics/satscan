//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "SSException.h" 

/**  Construct. This is an alternate constructor for when the varArgs list for sMessage
     has already been prepared. Primarily, this will be used by derived classes.        */
ResolvableException::ResolvableException ( va_list varArgs, const char *sMessage, const char *sSourceModule, Level iLevel )
                    :ZdException ( varArgs, sMessage, sSourceModule, iLevel ) {}

/**  This function will throw the exception with the parameters.  It is equivalent to
     throw ZdException(...), but includes the ability to format the message string.
     This function should be used to generate all Zd Exceptions within ZD.             */
void GenerateResolvableException(const char * sMessage, const char * sSourceModule, ... ) {
  va_list      varArgs;
  va_start ( varArgs, sSourceModule );

  ResolvableException  theException ( varArgs, sMessage, sSourceModule, ResolvableException::Normal );
  va_end ( varArgs );

  throw theException;
}


/**  Construct. This is an alternate constructor for when the varArgs list for sMessage
     has already been prepared. Primarily, this will be used by derived classes.        */
UsageException::UsageException(const char * sExecutableFullpathName) : ZdException () {
   ZdString s;
   
   s.printf("usage: %s [parameters file] [options]\n\n"
            "options: -v -- suppress instance particular information in results\n"
            "         -o -- ignore parameter result filename setting, use next parameter\n"
            "               ex. c:\\>satscan.exe c:\\parameters.prm -o c:\\alternate.out.txt\n",
            ZdFileName(sExecutableFullpathName).GetCompleteFileName());
                   	
   SetErrorMessage(s.GetCString());
}

/**  This function will throw the exception with the parameters.  It is equivalent to
     throw ZdException(...), but includes the ability to format the message string.
     This function should be used to generate all Zd Exceptions within ZD.             */
void GenerateUsageException(const char * sExecutableFullpathName) {
  throw UsageException(sExecutableFullpathName);
}


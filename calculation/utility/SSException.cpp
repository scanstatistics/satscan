//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "SSException.h"
#include "UtilityFunctions.h" 

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
   std::string s;

   printString(s, "usage: %s [parameters file] [options]\n\n"
                  "options: -o     -- ignore parameter result filename setting, use next parameter\n"
                  "                   ex. c:\\>satscan.exe c:\\parameters.prm -o c:\\alternate.out.txt\n"
                  "         -c     -- confirm parameter file correctness only (does not perform analysis)\n"
                  "         -p     -- print parameter settings to screen (does not perform analysis)\n",
            ZdFileName(sExecutableFullpathName).GetCompleteFileName());

   SetErrorMessage(s.c_str());
}

/**  This function will throw the exception with the parameters.  It is equivalent to
     throw ZdException(...), but includes the ability to format the message string.
     This function should be used to generate all Zd Exceptions within ZD.             */
void GenerateUsageException(const char * sExecutableFullpathName) {
  throw UsageException(sExecutableFullpathName);
}

program_error::program_error(const std::string& what_arg) : std::exception() {
  __what=what_arg;
}

program_error::program_error(const char * format, ...) : std::exception() {
  va_list varArgs;
  va_start (varArgs, format);
  printStringArgs(__what, varArgs, format);
  va_end(varArgs);
}

/** ::addTrace(__FILE__, __LINE__); */
void program_error::addTrace(const char * file, int line) {
  std::string temp;
  printString(temp, "file: %s\nline: %d\n", file, line);
  __trace.append(temp);
}


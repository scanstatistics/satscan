//---------------------------------------------------------------------------

#ifndef BasePrintH
#define BasePrintH

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

class BasePrint
{
   private:
     char *gsMessage;

     void PrintMessage( va_list varArgs, const char *sMessage );
   public:
      BasePrint();
     ~BasePrint();

   virtual bool GetIsCanceled() = 0;
   void SatScanPrintf(const char * sMessage, ... );
   void SatScanPrintWarning(const char * sMessage, ... );
   virtual void PrintLine(char *) = 0;
   virtual void PrintWarningLine(char *) = 0;
};

//---------------------------------------------------------------------------
#endif

//---------------------------------------------------------------------------

#include "BasePrint.h"

//---------------------------------------------------------------------------
BasePrint::BasePrint()
{
   gsMessage = 0;
}
//---------------------------------------------------------------------------
BasePrint::~BasePrint()
{
   if (gsMessage)
      delete [] gsMessage;
}
//---------------------------------------------------------------------------
/*bool BasePrint::GetIsCanceled()
{
   return false;
} */
//---------------------------------------------------------------------------
/*void BasePrint::PrintLine(char *s)
{
   printf(s);
} */
//---------------------------------------------------------------------------
void BasePrint::SatScanPrintf(const char * sMessage, ... )
{
   va_list   varArgs;

   va_start ( varArgs, sMessage );
   PrintMessage ( varArgs, sMessage );
   PrintLine(gsMessage);
   va_end ( varArgs );
}

void BasePrint::SatScanPrintWarning(const char * sMessage, ... )
{
   va_list   varArgs;

   va_start ( varArgs, sMessage );
   PrintMessage ( varArgs, sMessage );
   PrintWarningLine(gsMessage);
   va_end ( varArgs );
}
//---------------------------------------------------------------------------
//*** There are two different versions of this function because vsnprintf behaves
//*** in completely different ways on the two platforms. Under BCB 5, vsnprintf()
//*** will return the length of the required buffer if the length parameter is
//*** zero and otherwise returns the number of bytes output. Under Solaris,
//*** vsnprintf() will return -1 if a zero is passed and otherwise _always_
//*** returns the number of bytes output.
//---------------------------------------------------------------------------
#ifdef INTEL_BASED
// This function sets the current exception message.  If a NULL is passed in, the current
// message is cleared.
void BasePrint::PrintMessage(va_list varArgs, const char * sMessage)
{
   int   iStringLength;   // Holds the length of the formatted output

   try
      {
      if (sMessage)
         {
         // vsnprintf will calculate the required length, not including the NULL,
         // for the format string when given a NULL pointer and a zero length as
         // the first two parameters.
         if (gsMessage)
            delete [] gsMessage;
         iStringLength = vsnprintf ( 0, 0, sMessage, varArgs );
         gsMessage = new char[iStringLength + 1];
         vsnprintf ( gsMessage, iStringLength + 1, sMessage, varArgs );
         //PrintLine(sNewMessage);
         //delete [] sNewMessage; sNewMessage = 0;
         }
      }
   catch (...) {};
}
#else
// This function sets the current exception message.  If a NULL is passed in, the current
// message is cleared.
void BasePrint::PrintMessage(va_list varArgs, const char * sMessage )
{
   int   iStringLength;   // Holds the length of the formatted output
   char  * sNewMessage = 0;

   try
      {
      if (sMessage)
         {
         if (gsMessage)
            delete [] gsMessage;
         // vsnprintf will always return the length needed to format the string.
         iStringLength = vsnprintf ( sNewMessage, iCurrentLength + 1, sMessage, varArgs );
         gsMessage = new char[iStringLength + 1];
         vsnprintf ( gsMessage, iStringLength + 1, sMessage, varArgs );
         //printf(sNewMessage); //new line
         //delete [] sNewMessage; sNewMessage = 0;
         }
      }
   catch (...) {};
}
#endif
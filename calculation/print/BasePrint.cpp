#include "SaTScan.h"
#pragma hdrstop
#include "BasePrint.h"

const int MAX_INPUT_FILE_WARNING_LIMIT = 75;

//---------------------------------------------------------------------------
BasePrint::BasePrint()
{
   SetImpliedInputFileType(CASEFILE);
   gsMessage = new char[1];
   gsMessage[0] = 0;
}
//---------------------------------------------------------------------------
BasePrint::~BasePrint() {
  try {
    delete [] gsMessage;
  }
  catch (...){}
}

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

// function for printing out input file warning messages, this function will print out MAX_INPUT_FILE_WARNING_LIMIT
// number of input file messages from each input file type, then will print a warning telling the user to check the
// input file format
// pre : none
// post : increments the counter in the global map for the message type (or starts a new counter if not found) and
//       if the number of messages for that file type is less than the maximum then it just prints as normal
void BasePrint::PrintInputWarning(const char* sMessage, ...)
{
   bool bPrintAsNormal(true);
   std::map<eInputFileType, int>::iterator iter = gInputFileWarningsMap.find(geInputFileType);

   if (iter == gInputFileWarningsMap.end()) {
      gInputFileWarningsMap.insert(std::make_pair(geInputFileType, 1));
   }
   else {
      iter->second++;

      // print the excessive warning message on the MAX_INPUT_FILE_WARNING_LIMIT time - else print nothing past -- AJV
      if (iter->second == MAX_INPUT_FILE_WARNING_LIMIT) {
         bPrintAsNormal = false;
         std::string message;
         message = "Error: Excessive number of warnings in  ";
         message += GetImpliedFileTypeString().c_str();
         message += ".\n";
         PrintWarningLine(const_cast<char*>(message.c_str()));
      }
      else if(iter->second > MAX_INPUT_FILE_WARNING_LIMIT)
         bPrintAsNormal = false;
   }

   if(bPrintAsNormal) {  // just some code stolen from the SatScanPrintWarning function
      va_list   varArgs;

      va_start ( varArgs, sMessage );
      PrintMessage ( varArgs, sMessage );
      PrintWarningLine(gsMessage);
      va_end ( varArgs );
   }
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
void BasePrint::PrintMessage(va_list varArgs, const char * sMessage) {
  int   iStringLength;   // Holds the length of the formatted output
  int   iCurrentLength;  // Current length of the buffer

  try {
    // vsnprintf will calculate the required length, not including the NULL,
    // for the format string when given a NULL pointer and a zero length as
    // the first two parameters.

    iCurrentLength = strlen (gsMessage);

    iStringLength = vsnprintf(gsMessage, iCurrentLength + 1, sMessage, varArgs);
    //iStringLength = vsnprintf ( 0, 0, sMessage, varArgs );

    if ( iStringLength > iCurrentLength ) {
      delete [] gsMessage; gsMessage=0;
      gsMessage = new char[iStringLength + 1];
      vsnprintf (gsMessage, iStringLength + 1, sMessage, varArgs);
    }
  }
  catch (...) {}
}
#else
// This function sets the current exception message.  If a NULL is passed in, the current
// message is cleared.
void BasePrint::PrintMessage(va_list varArgs, const char * sMessage ) {
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

void BasePrint::SetImpliedInputFileType(eInputFileType eType, unsigned int) {
  geInputFileType = eType;
  switch (eType) {
    case CASEFILE         : gsInputFileString = "case file"; break;
    case CONTROLFILE      : gsInputFileString = "control file"; break;
    case POPFILE          : gsInputFileString = "population file"; break;
    case COORDFILE        : gsInputFileString = "coordinates file"; break;
    case GRIDFILE         : gsInputFileString = "grid file"; break;
    case MAXCIRCLEPOPFILE : gsInputFileString = "max circle size file"; break;
    case ADJ_BY_RR_FILE   : gsInputFileString = "adjustments file"; break;
    default : ZdException::GenerateNotification("Invalid input file type warning message!", "SetImpliedInputFileType()");
  }

//  if (iDataSet) {
//    ZdString s;
//    s.printf("%s (input set %u)", gsInputFileString.c_str(), iDataSet);
//    gsInputFileString = s.GetCString();
//    gsInputFileString += "(input set ";
//    gsInputFileString += itoa(iDataSet;
//    gsInputFileString += ")";
//  }
}


//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "BasePrint.h"

const int BasePrint::MAX_READ_ERRORS = 75;

/** constructor */
BasePrint::BasePrint(bool bSuppressWarnings) : gbSuppressWarnings(bSuppressWarnings) {
   SetImpliedInputFileType(CASEFILE);
   gsMessage = new char[1];
   gsMessage[0] = 0;
}

/** destructor */
BasePrint::~BasePrint() {
  try {
    delete [] gsMessage;
  }
  catch (...){}
}

/** Returns indication of whether maximum number of read errors have been printed
    through this object. */
bool BasePrint::GetMaximumReadErrorsPrinted() const {
  std::map<eInputFileType, int>::const_iterator iter = gInputFileWarningsMap.find(geInputFileType);

  return (iter == gInputFileWarningsMap.end() ? false : iter->second == MAX_READ_ERRORS);
}

/** Directs message to appropriate output based  upon PrintType. */
void BasePrint::Print(const char * sMessage, PrintType ePrintType) {
   switch (ePrintType) {
     case P_STDOUT    : PrintStandard(sMessage); break;
     case P_NOTICE    : if (!gbSuppressWarnings)
                          PrintNotice(sMessage);
                        break;
     case P_WARNING   : if (!gbSuppressWarnings)
                          PrintWarning(sMessage);
                        break;
     case P_READERROR : PrintReadError(sMessage); break;
     case P_ERROR     :
     default          : PrintError(sMessage);
   };
}

/** Creates formatted output from variable number of parameter arguments and calls class Print() method. */
void BasePrint::Printf(const char * sMessage, PrintType ePrintType, ...) {
  va_list       varArgs;
  int           iStringLength, iCurrentLength;

  if (!sMessage || sMessage == gsMessage) return;

  try {
    iCurrentLength = strlen (gsMessage);
    va_start(varArgs, ePrintType);
    iStringLength = vsnprintf(gsMessage, iCurrentLength + 1, sMessage, varArgs);
    va_end(varArgs);
    if (iStringLength > iCurrentLength) {
      delete [] gsMessage; gsMessage=0;
      gsMessage = new char[iStringLength + 1];
      va_start(varArgs, ePrintType);
      vsnprintf (gsMessage, iStringLength + 1, sMessage, varArgs);
      va_end(varArgs);
    }
  }
  catch (...) {}

  Print(gsMessage, ePrintType);
}

// function for printing out input file warning messages, this function will print out MAX_READ_ERRORS
// number of input file messages from each input file type, then will print a warning telling the user to check the
// input file format
// pre : none
// post : increments the counter in the global map for the message type (or starts a new counter if not found) and
//       if the number of messages for that file type is less than the maximum then it just prints as normal
void BasePrint::PrintReadError(const char * sMessage) {
   bool bPrintAsNormal(true);
   std::map<eInputFileType, int>::iterator iter = gInputFileWarningsMap.find(geInputFileType);

   if (iter == gInputFileWarningsMap.end())
      gInputFileWarningsMap.insert(std::make_pair(geInputFileType, 1));
   else {
     iter->second++;
     // print the excessive warning message on the MAX_READ_ERRORS time - else print nothing past -- AJV
     if (iter->second == MAX_READ_ERRORS) {
       bPrintAsNormal = false;
       std::string message;
       message = "Error: Excessive number of errors reading ";
       message += GetImpliedFileTypeString().c_str();
       message += " data.\n";
       PrintError(message.c_str());
     }
     else if(iter->second > MAX_READ_ERRORS)
       bPrintAsNormal = false;
   }

   if (bPrintAsNormal)
     PrintError(sMessage);
}

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


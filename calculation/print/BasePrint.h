//*****************************************************************************
#ifndef __BasePrint_H
#define __BasePrint_H
//*****************************************************************************
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <map>

class BasePrint
{
   public:
     enum eInputFileType { CASEFILE, CONTROLFILE, POPFILE, COORDFILE, GRIDFILE };
   private:
     char *gsMessage;
     std::map<eInputFileType, int>   gInputFileWarningsMap;

     void GetInputFileType(eInputFileType eType, std::string& sName);
     void PrintMessage( va_list varArgs, const char *sMessage );
   
   public:
     BasePrint();
     ~BasePrint();

     virtual bool GetIsCanceled() = 0;
     void SatScanPrintf(const char * sMessage, ... );
     void SatScanPrintWarning(const char * sMessage, ... );
     void SatScanPrintInputFileWarning(eInputFileType eType, const char* sMessage, ...);
     virtual void PrintLine(char *) = 0;
     virtual void PrintWarningLine(char *) = 0;
};

//*****************************************************************************
#endif

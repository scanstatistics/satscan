//*****************************************************************************
#ifndef __BasePrint_H
#define __BasePrint_H
//*****************************************************************************
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <map>

class BasePrint {
  public:
    enum eInputFileType {CASEFILE, CONTROLFILE, POPFILE, COORDFILE, GRIDFILE, MAXCIRCLEPOPFILE, RELATIVERISKFILE};
  private:
    char                              * gsMessage;
    eInputFileType                      geInputFileType;
    std::map<eInputFileType, int>       gInputFileWarningsMap;

    void                                Init() {geInputFileType = CASEFILE;}
    void                                PrintMessage( va_list varArgs, const char *sMessage );

  public:
    BasePrint();
    ~BasePrint();

    eInputFileType                      GetImpliedInputFileType() const {return geInputFileType;}
    static std::string                & GetInputFileType(eInputFileType eType, std::string& sName);
    virtual bool                        GetIsCanceled() = 0;
    void                                PrintInputWarning(const char* sMessage, ...);
    virtual void                        PrintLine(char *) = 0;
    virtual void                        PrintWarningLine(char *) = 0;
    void                                SatScanPrintf(const char * sMessage, ... );
    void                                SatScanPrintWarning(const char * sMessage, ... );
    void                                SetImpliedInputFileType(eInputFileType eType) {geInputFileType = eType;}
};

//*****************************************************************************
#endif

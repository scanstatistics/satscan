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
    enum eInputFileType {CASEFILE, CONTROLFILE, POPFILE, COORDFILE, GRIDFILE, MAXCIRCLEPOPFILE, ADJ_BY_RR_FILE};
  private:
    char                              * gsMessage;
    eInputFileType                      geInputFileType;
    std::string                         gsInputFileString;
    std::map<eInputFileType, int>       gInputFileWarningsMap;

    void                                PrintMessage( va_list varArgs, const char *sMessage );

  public:
    BasePrint();
    ~BasePrint();

    eInputFileType                      GetImpliedInputFileType() const {return geInputFileType;}
    const std::string                 & GetImpliedFileTypeString() const {return gsInputFileString;}  
    virtual bool                        GetIsCanceled() const = 0;
    void                                PrintInputWarning(const char* sMessage, ...);
    virtual void                        PrintLine(char *) = 0;
    virtual void                        PrintWarningLine(char *) = 0;
    void                                SatScanPrintf(const char * sMessage, ... );
    void                                SatScanPrintWarning(const char * sMessage, ... );
    void                                SetImpliedInputFileType(eInputFileType eType, unsigned int iDataSet=0);
};

//*****************************************************************************
#endif

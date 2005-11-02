//******************************************************************************
#ifndef __BasePrint_H
#define __BasePrint_H
//******************************************************************************
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <map>

class BasePrint {
  public:
    enum eInputFileType {CASEFILE, CONTROLFILE, POPFILE, COORDFILE, GRIDFILE, MAXCIRCLEPOPFILE, ADJ_BY_RR_FILE};
    enum PrintType {P_STDOUT=0, P_WARNING, P_ERROR, P_READERROR, P_NOTICE};

  protected:
    static const int                    MAX_READ_ERRORS;
    char                              * gsMessage;
    eInputFileType                      geInputFileType;
    std::string                         gsInputFileString;
    std::map<eInputFileType, int>       gInputFileWarningsMap;
    bool                                gbSuppressWarnings;

    virtual void                        PrintError(const char * sMessage) = 0;
    virtual void                        PrintNotice(const char * sMessage) = 0;
    virtual void                        PrintReadError(const char * sMessage);
    virtual void                        PrintStandard(const char * sMessage) = 0;
    virtual void                        PrintWarning(const char * sMessage) = 0;
    void                                SetMessageFromArgs(va_list varArgs, const char * sMessage);

  public:
    BasePrint(bool bSuppressWarnings);
    virtual ~BasePrint();

    eInputFileType                      GetImpliedInputFileType() const {return geInputFileType;}
    const std::string                 & GetImpliedFileTypeString() const {return gsInputFileString;}
    virtual bool                        GetIsCanceled() const = 0;
    virtual void                        Print(const char * sMessage, PrintType ePrintType);
    virtual void                        Printf(const char * sMessage, PrintType ePrintType, ...);
    void                                SetImpliedInputFileType(eInputFileType eType, unsigned int iDataSet=0);
};

//******************************************************************************
#endif


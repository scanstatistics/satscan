//******************************************************************************
#ifndef __BasePrint_H
#define __BasePrint_H
//******************************************************************************
#include <cstdarg>
#include <map>

class PrintProxy;

class BasePrint {
    friend class PrintProxy;

  public:
    enum eInputFileType {
        CASEFILE, CONTROLFILE, POPFILE, COORDFILE, GRIDFILE, MAXCIRCLEPOPFILE, ADJ_BY_RR_FILE,
        LOCATION_NEIGHBORS_FILE, META_LOCATIONS_FILE, NETWORK_FILE, MULTIPLE_LOCATIONS
    };
    enum PrintType {P_STDOUT=0, P_WARNING, P_ERROR, P_READERROR, P_NOTICE, P_PARAMERROR};

  protected:
    int                                 giMaximumReadErrors;
    std::vector<char>                   gsMessage;
    eInputFileType                      geInputFileType;
    std::string                         gsInputFileString;
    std::map<eInputFileType, int>       gInputFileWarningsMap;
    bool                                gbSuppressWarnings;

  public:
    BasePrint(bool bSuppressWarnings);
    virtual ~BasePrint();

    virtual void                        PrintError(const char * sMessage) = 0;
    virtual void                        PrintNotice(const char * sMessage) = 0;
    virtual void                        PrintReadError(const char * sMessage);
    virtual void                        PrintStandard(const char * sMessage) = 0;
    virtual void                        PrintWarning(const char * sMessage) = 0;

    const char *                        getSourceFilenameForType(eInputFileType eType) const;
    eInputFileType                      GetImpliedInputFileType() const {return geInputFileType;}
    const std::string                 & GetImpliedFileTypeString() const {return gsInputFileString;}
    virtual bool                        GetIsCanceled() const = 0;
    bool                                GetMaximumReadErrorsPrinted() const;
    virtual void                        Print(const char * sMessage, PrintType ePrintType);
    virtual void                        Printf(const char * sMessage, PrintType ePrintType, ...);
    virtual void                        ReportDrilldownResults(const char * drilldown_resultfile, const char * parent_resultfile, unsigned int significantClusters) {}
    void                                SetImpliedInputFileType(eInputFileType eType, bool clearWarningCount=false);
    int                                 getMaximumReadErrors() const { return giMaximumReadErrors; }
    void                                SetMaximumReadErrors(int iMaximumReadErrors) {giMaximumReadErrors=iMaximumReadErrors;}
    bool                                isSuppressingWarnings() const { return gbSuppressWarnings; }
    void                                SetSuppressWarnings(bool b) {gbSuppressWarnings=b;}
};

/** Print direction class that quietly suppresses printing messages. */
class PrintNull : public BasePrint {
   protected:
    virtual void        PrintError(const char * sMessage) {}
    virtual void        PrintNotice(const char * sMessage) {}
    virtual void        PrintStandard(const char * sMessage) {}
    virtual void        PrintWarning(const char * sMessage) {}

   public:
     PrintNull(bool bSuppressWarnings=true) : BasePrint(bSuppressWarnings) {}
     virtual ~PrintNull() {}

     bool               GetIsCanceled() const {return false;}
     virtual void       Print(const char * sMessage, PrintType ePrintType) {}
     virtual void       Printf(const char * sMessage, PrintType ePrintType, ...) {}
};
//******************************************************************************
#endif


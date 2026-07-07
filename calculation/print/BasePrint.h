//******************************************************************************
#ifndef __BasePrint_H
#define __BasePrint_H
//******************************************************************************
#include <cstdarg>
#include <map>

class PrintProxy;

class ResultsNode {
private:
    std::string _filename;
	std::shared_ptr<ResultsNode> _parent;
    std::vector<std::shared_ptr<ResultsNode>> _children;

public:
    ResultsNode(const std::string& filename) :_filename(filename) {}

    void addChild(std::shared_ptr<ResultsNode> child);
	const std::vector<std::shared_ptr<ResultsNode>>& getChildren() const { return _children; }
	const std::string& getFilename() const { return _filename; }
	std::shared_ptr<ResultsNode> getParent() const { return _parent; }
	void setParent(std::shared_ptr<ResultsNode> parent) { _parent = parent; }
};

class BasePrint {
    friend class PrintProxy;

  public:
    enum eInputFileType {
        CASEFILE, CONTROLFILE, POPFILE, COORDFILE, GRIDFILE, MAXCIRCLEPOPFILE, ADJ_BY_RR_FILE,
        LOCATION_NEIGHBORS_FILE, META_LOCATIONS_FILE, NETWORK_FILE, MULTIPLE_LOCATIONS
    };
    enum PrintType {P_STDOUT=0, P_WARNING, P_ERROR, P_READERROR, P_NOTICE, P_PARAMERROR};
    typedef std::map<std::string, std::shared_ptr<ResultsNode>> ResultNodeContainer_t;

  protected:
    int                                 giMaximumReadErrors;
    std::vector<char>                   gsMessage;
    eInputFileType                      geInputFileType;
    std::string                         gsInputFileString;
    std::map<eInputFileType, int>       gInputFileWarningsMap;
    bool                                gbSuppressWarnings;
    ResultNodeContainer_t               _result_nodes;

  public:
    BasePrint(bool bSuppressWarnings);
    virtual ~BasePrint();

    virtual void                        PrintError(const char * sMessage) = 0;
    virtual void                        PrintNotice(const char * sMessage) = 0;
    virtual void                        PrintReadError(const char * sMessage);
    virtual void                        PrintStandard(const char * sMessage) = 0;
    virtual void                        PrintWarning(const char * sMessage) = 0;

    const ResultNodeContainer_t       & getResultsNodes() const { return _result_nodes; }
    void                                printResultsNodes() const;
    const char *                        getSourceFilenameForType(eInputFileType eType) const;
    eInputFileType                      GetImpliedInputFileType() const {return geInputFileType;}
    const std::string                 & GetImpliedFileTypeString() const {return gsInputFileString;}
    virtual bool                        GetIsCanceled() const = 0;
    bool                                GetMaximumReadErrorsPrinted() const;
    virtual void                        Print(const char * sMessage, PrintType ePrintType);
    virtual void                        Printf(const char * sMessage, PrintType ePrintType, ...);
    virtual void                        ReportDrilldownResults(const char* drilldown_resultfile, const char* parent_resultfile, unsigned int significantClusters);
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


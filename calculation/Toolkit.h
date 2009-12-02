//******************************************************************************
#ifndef __AppToolkit_H
#define __AppToolkit_H
//******************************************************************************
#include <list>
#include "RunTimeComponents.h"
#include "Ini.h"
               
/** Class for managing structure of IniFile object in the context of the toolkit. */               
class IniSession {
  protected:
    IniFile     gIniFile;

  public:
    IniSession() {}
    virtual ~IniSession() {}

    bool                exists(const char * sSectionName, const char * sKeyName) const;
    const char        * get(const char * sSectionName, const char * sKeyName) const;
    void                read(const std::string& file);
    void                set(const char * sSectionName, const char * sKeyName, const char * sValue);
    void                write(const std::string& file) const;
};

/** Application global toolkit. */
class AppToolkit {
  public:
    typedef std::vector<std::string> ParameterHistory_t;
  
  private:
    static AppToolkit         * gpToolKit;

    // system file
    static const char         * gsSystemIniFileName;
    static const char         * gsRunHistory;
    static const char         * gsHistoryFileNameProperty;
    static const char         * gsParameterHistory;
    static const char         * gsParameterNameProperty;
    static const size_t         giMaximumParameterHistoryItems;
    static const char         * gsLastDirectory;
    static const char         * gsLastDirectoryPathProperty;
    static const char         * gsLastImportDestinationDirectoryProperty;
    static const char         * gsDebugFileName;

    // default defines
    static const char         * gsDefaultRunHistoryFileName;
    static const char         * gsDefaultSaTScanWebSite;
    static const char         * gsDefaultSubstantiveSupportEmail;
    static const char         * gsDefaultTechnicalSupportEmail;

    std::string                 gsSystemFileName;
    std::string                 gsApplicationFullPath;
    ParameterHistory_t          gvParameterHistory;
    bool                        gbRunUpdateOnTerminate;
    std::string                 gsUpdateArchiveFilename;
    std::string                 gsVersion;
    RunTimeComponentManager     gRunTimeComponentManager;
    IniSession                  gSession;
    FILE                      * gpDebugLog;

    bool                        InsureLastDirectoryPath();
    bool                        InsureLastImportDestinationDirectoryPath();
    bool                        InsureRunHistoryFileName();
    void                        InsureSessionStructure();
    void                        ReadParametersHistory();
    void                        SetLastDirectory(const char * sLastDirectory);
    void                        Setup(const char * sApplicationFullPath);
    void                        WriteParametersHistory();

  public:
    AppToolkit(const char * sApplicationFullPath);
    virtual ~AppToolkit();

   void                         AddParameterToHistory(const char * sParameterFileName);
   void                         closeDebugFile();
   const char                 * GetAcknowledgment(std::string & Acknowledgment) const;
   const char                 * GetApplicationFullPath() const;
   FILE                       * getDebugFile();
   const char                 * GetLastDirectory() /*const*/;
   const char                 * GetLastImportDirectory() /*const*/;
   const ParameterHistory_t   & GetParameterHistory() const {return gvParameterHistory;}
   const char                 * GetRunHistoryFileName() /*const*/;
   RunTimeComponentManager    & GetRunTimeComponentManager() { return gRunTimeComponentManager;}
   bool                         GetRunUpdateOnTerminate() const {return gbRunUpdateOnTerminate;}
   const char                 * GetSubstantiveSupportEmail() const;
   const char                 * GetSystemIniFileName() const {return gsSystemIniFileName;}
   const char                 * GetTechnicalSupportEmail() const;
   const char                 * GetUpdateArchiveFilename() const {return gsUpdateArchiveFilename.c_str();}
   const char                 * GetVersion() const {return gsVersion.c_str();}
   const char                 * GetWebSite() const;
   bool                         is64Bit() const;
   FILE                       * openDebugFile(); 
   void                         SetLastImportDirectory(const char * sLastDirectory);
   void                         SetRunUpdateOnTerminate(bool b) {gbRunUpdateOnTerminate = b;}
   void                         SetUpdateArchiveFilename(const char * sArchiveFile) {gsUpdateArchiveFilename = sArchiveFile;}

   static AppToolkit     &      getToolkit() {return *gpToolKit;}
   static void                  ToolKitCreate(const char * sApplicationFullPath);
   static void                  ToolKitDestroy();
};

#ifdef RPRTCMPT_RUNTIMES
  #define macroRunTimeManagerInit()     AppToolkit::getToolkit().GetRunTimeComponentManager().Initialize()
  #define macroRunTimeManagerPrint(p)   AppToolkit::getToolkit().GetRunTimeComponentManager().Print(p)
  #define macroRunTimeStartSerial(p)    AppToolkit::getToolkit().GetRunTimeComponentManager().StartSerialComponent(p)
  #define macroRunTimeStopSerial()      AppToolkit::getToolkit().GetRunTimeComponentManager().StopSerialComponent()
  #define macroRunTimeStartFocused(p)   AppToolkit::getToolkit().GetRunTimeComponentManager().StartFocused(p)
  #define macroRunTimeStopFocused(p)    AppToolkit::getToolkit().GetRunTimeComponentManager().StopFocused(p)
#else
  #define macroRunTimeManagerInit()     ((void)0)
  #define macroRunTimeManagerPrint(p)    ((void)0)
  #define macroRunTimeStartSerial(p)    ((void)0)
  #define macroRunTimeStopSerial()     ((void)0)
  #define macroRunTimeStartFocused(p)   ((void)0)
  #define macroRunTimeStopFocused(p)    ((void)0)
#endif

//******************************************************************************
#endif

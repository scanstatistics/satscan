//******************************************************************************
#ifndef __AppToolkit_H
#define __AppToolkit_H
//******************************************************************************
#include <list>
#include "RunTimeComponents.h"

#ifdef __BATCH_COMPILE
  #include "IniSession.h"
#else
  #include <Basis540Unix.h>
#endif

class AppToolkit {
  public:
    typedef std::vector<std::string> ParameterHistory_t;
  
  private:
    static AppToolkit         * gpToolKit;

    // system file
    static const char         * gsSystemIniFileName;
    static const char         * gsHistoryFileNameProperty;
    static const char         * gsParameterNameProperty;
    static const size_t         giMaximumParameterHistoryItems;
    static const char         * gsLastDirectoryProperty;
    static const char         * gsLastImportDestinationDirectoryProperty;

    // default defines
    static const char         * gsDefaultRunHistoryFileName;
    static const char         * gsDefaultSaTScanWebSite;
    static const char         * gsDefaultSubstantiveSupportEmail;
    static const char         * gsDefaultTechnicalSupportEmail;

    ZdString                    gsSystemFileName;
    ZdString                    gsApplicationFullPath;
    ParameterHistory_t          gvParameterHistory;
    bool                        gbRunUpdateOnTerminate;
    ZdString                    gsUpdateArchiveFilename;
    ZdString                    gsVersion;
    RunTimeComponentManager     gRunTimeComponentManager;
    BZdIniSession               gSession;

    bool                        InsureLastDirectoryPath();
    bool                        InsureLastImportDestinationDirectoryPath();
    bool                        InsureRunHistoryFileName();
    bool                        InsureSessionProperty(const char * sSessionProperty, const char * sDefaultValue);
    void                        InsureSessionStructure();
    void                        ReadParametersHistory();
    void                        SetLastDirectory(const char * sLastDirectory);
    void                        Setup(const char * sApplicationFullPath);
    void                        WriteParametersHistory();

  public:
    AppToolkit(const char * sApplicationFullPath);
    virtual ~AppToolkit();

   void                         AddParameterToHistory(const char * sParameterFileName);
   const char                 * GetAcknowledgment(ZdString & Acknowledgment) const;
   const char                 * GetApplicationFullPath() const;
   const char                 * GetLastDirectory() /*const*/;
   const char                 * GetLastImportDirectory() /*const*/;
   const ParameterHistory_t   & GetParameterHistory() const {return gvParameterHistory;}
   const char                 * GetRunHistoryFileName() /*const*/;
   RunTimeComponentManager    & GetRunTimeComponentManager() { return gRunTimeComponentManager;}
   bool                         GetRunUpdateOnTerminate() const {return gbRunUpdateOnTerminate;}
   const char                 * GetSubstantiveSupportEmail() const;
   const char                 * GetSystemIniFileName() const {return gsSystemIniFileName;}
   const char                 * GetTechnicalSupportEmail() const;
   const ZdString             & GetUpdateArchiveFilename() const {return gsUpdateArchiveFilename;}
   const char                 * GetVersion() const {return gsVersion.GetCString();}
   const char                 * GetWebSite() const;
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

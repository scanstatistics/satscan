//******************************************************************************
#ifndef __AppToolkit_H
#define __AppToolkit_H
//******************************************************************************
#include <list>
#include "RunTimeComponents.h"
#include "Ini.h"
#include <boost/filesystem.hpp>
               
/** Class for managing structure of IniFile object in the context of the toolkit. */               
class IniSession {
  protected:
    IniFile     _ini_file;

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
  private:
    static AppToolkit         * _tool_kit;

    // ini file
    static const char         * _ini_filename;
    static const char         * _mail_server;
    static const char         * _mail_servername_property;
    static const char         * _mail_additional_property;
    static const char         * _mail_from_property;
    static const char         * _mail_reply_property;
    static const char         * _debug_filename;
    static const char         * _multiple_analysis_filename;

    // default defines
    static const char         * _default_run_history_filename;
    static const char         * _webSite;
    static const char         * _substantive_support_email;
    static const char         * _technical_support_email;

    boost::filesystem::path     _ini_filepath;
    boost::filesystem::path     _satscan_appdata_folder;
    boost::filesystem::path     _multi_analysis_filepath;
    std::string                 _application_fullpath;
    std::string                 _version;
    RunTimeComponentManager     _runtime_component_manager;
    IniSession                  _session;
    FILE                      * _debug_log;

    void                        Setup(const char * sApplicationFullPath);

  public:
    AppToolkit(const char * sApplicationFullPath);
    virtual ~AppToolkit();

    boost::filesystem::path     run_history_filename;
    std::string                 mail_servername;
    std::string                 mail_additional;
    std::string                 mail_from;
    std::string                 mail_reply;

   void                         closeDebugFile();
   const char                 * GetAcknowledgment(std::string & Acknowledgment) const;
   const char                 * GetApplicationFullPath() const;
   RunTimeComponentManager    & GetRunTimeComponentManager() { return _runtime_component_manager;}
   const char                 * GetSubstantiveSupportEmail() const;
   const char                 * GetSystemIniFileName() const {return _ini_filename;}
   const char                 * GetTechnicalSupportEmail() const;
   const char                 * GetVersion() const {return _version.c_str();}
   const char                 * GetWebSite() const;
   bool                         is64Bit() const;
   FILE                       * openDebugFile(); 
   std::string                  getMultipleAnalysisFullpath() const { return _multi_analysis_filepath.string(); }
   void                         refreshSession();

   static AppToolkit     &      getToolkit() {return *_tool_kit;}
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

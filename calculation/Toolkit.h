//*****************************************************************************
#ifndef __SaTScanToolkit_H
#define __SaTScanToolkit_H
//*****************************************************************************
#include "DBFFile.h"
#include <list>

class SaTScanToolkit : public BToolkit {
  public:
    typedef std::vector<std::string> ParameterHistory_t;
  
  private:
    // system file
    static const char         * gsSystemIniFileName;
    static const char         * gsHistoryFileNameProperty;
    static const char         * gsParameterNameProperty;
    static const size_t         giMaximumParameterHistoryItems;
    static const char         * gsLastDirectoryProperty;

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

    bool                        InsureLastDirectoryPath();
    bool                        InsureRunHistoryFileName();
    bool                        InsureSessionProperty(const char * sSessionProperty, const char * sDefaultValue);
    void                        InsureSessionStructure();
    void                        ReadParametersHistory();
   void                         SetLastDirectory(const char * sLastDirectory);
    void                        Setup(const char * sApplicationFullPath);
    void                        WriteParametersHistory();

  public:
    SaTScanToolkit(const char * sApplicationFullPath);
    virtual ~SaTScanToolkit();

   void                         AddParameterToHistory(const char * sParameterFileName);
   const char                 * GetAcknowledgment(ZdString & Acknowledgment) const;
   const char                 * GetApplicationFullPath() const;
   const char                 * GetLastDirectory() /*const*/;
   bool                         GetLogRunHistory() const;
   const ParameterHistory_t   & GetParameterHistory() const {return gvParameterHistory;}
   const char                 * GetRunHistoryFileName() /*const*/;
   bool                         GetRunUpdateOnTerminate() const {return gbRunUpdateOnTerminate;}
   const char                 * GetSubstantiveSupportEmail() const;
   const char                 * GetSystemIniFileName() const {return gsSystemIniFileName;}
   const char                 * GetTechnicalSupportEmail() const;
   const ZdString             & GetUpdateArchiveFilename() const {return gsUpdateArchiveFilename;}
   const char                 * GetVersion() const {return gsVersion.GetCString();}
   const char                 * GetWebSite() const;
   void                         SetRunUpdateOnTerminate(bool b) {gbRunUpdateOnTerminate = b;}
   void                         SetUpdateArchiveFilename(const char * sArchiveFile) {gsUpdateArchiveFilename = sArchiveFile;} 
};

SaTScanToolkit & GetToolkit();

//*****************************************************************************
#endif

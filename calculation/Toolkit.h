//*****************************************************************************
#ifndef __SaTScanToolkit_H
#define __SaTScanToolkit_H
//*****************************************************************************
#include "DBFFile.h"

class SaTScanToolkit : public BToolkit {
  private:
    // system file
    static const char         * gsSystemIniFileName;
    static const char         * gsHistoryFileNameProperty;
    static const char         * gsLoggingProperty;
    static const char         * gsSaTScanWebSiteProperty;
    static const char         * gsSaTScanLicenceWebSiteProperty;
    static const char         * gsSubstantiveSupportEmailProperty;
    static const char         * gsTechnicalSupportEmailProperty;

    // default defines
    static const char         * gsDefaultRunHistoryFileName;
    static const char         * gsDefaultSaTScanWebSite;
    static const char         * gsDefaultSaTScanLicenceWebSite;
    static const char         * gsDefaultSubstantiveSupportEmail;
    static const char         * gsDefaultTechnicalSupportEmail;

    ZdString                    gsSystemFileName;

    bool                        InsureRunHistoryFileName();
    bool                        InsureSessionProperty(const char * sSessionProperty, const char * sDefaultValue);
    void                        InsureSessionStructure();
    void                        Setup();

  public:
    SaTScanToolkit();
    virtual ~SaTScanToolkit();

   const char                 * GetAcknowledgment(ZdString & Acknowledgment) /*const*/;  
   const char                 * GetLicenceWebSite() /*const*/;
   bool                         GetLogRunHistory() /*const*/;
   const char                 * GetRunHistoryFileName() /*const*/;
   const char                 * GetSubstantiveSupportEmail() /*const*/;
   const char                 * GetTechnicalSupportEmail() /*const*/;
   const char                 * GetWebSite() /*const*/;
};

SaTScanToolkit & GetToolkit();

//*****************************************************************************
#endif

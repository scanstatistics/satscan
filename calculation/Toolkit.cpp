//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------

/** system filename */
const char * SaTScanToolkit::gsSystemIniFileName = "system.ini";
/** ini section property name for run history filename */
const char * SaTScanToolkit::gsHistoryFileNameProperty = "[RunHistory].FileName";
/** ini section property name for logging run history */
//const char * SaTScanToolkit::gsLoggingProperty = "[RunHistory].LogHistory";
/** ini section property name for website */
//const char * SaTScanToolkit::gsSaTScanWebSiteProperty = "[Internet].WebsiteURL";
/** ini section property name for license website */
//const char * SaTScanToolkit::gsSaTScanLicenceWebSiteProperty = "[Internet].LicenceURL";
/** ini section property name for substantive support email */
//const char * SaTScanToolkit::gsSubstantiveSupportEmailProperty = "[Email].SubstantiveSupportEmail";
/** ini section property name for technical support email */
//const char * SaTScanToolkit::gsTechnicalSupportEmailProperty = "[Email].TechnicalSupportEmail";
/** analysis history filename */
const char * SaTScanToolkit::gsDefaultRunHistoryFileName = "AnalysisHistory";
/** Default website. */
#ifdef INTEL_BASED
const char * SaTScanToolkit::gsDefaultSaTScanWebSite = "http:/\/www.satscan.org/";
#else
const char * SaTScanToolkit::gsDefaultSaTScanWebSite = "http://www.satscan.org/";
#endif
/** Default Substantive Support Email. */
const char * SaTScanToolkit::gsDefaultSubstantiveSupportEmail = "kulldorff@satscan.org";
/** Default Technical Support Email. */
const char * SaTScanToolkit::gsDefaultTechnicalSupportEmail = "satscan@imsweb.com";

/** constructor */
SaTScanToolkit::SaTScanToolkit(const char * sApplicationFullPath):BToolkit() {
  try {
    Setup(sApplicationFullPath);
  }
  catch (ZdException& x) {
    x.AddCallpath("constructor()", "SaTScanToolkit");
    throw;
  }
}

/** destructor */
SaTScanToolkit::~SaTScanToolkit() {}

/** Returns indicator of run history logging. */
bool SaTScanToolkit::GetLogRunHistory() const {
  /** NOTE: This is not a requested feature, the hook is in place. */
  return true;//GetSession().GetProperty(gsLoggingProperty)->GetBool(true);
}

/** Returns acknowledgment statement indicating program version, website, and
    brief declaration of usage agreement. */
const char * SaTScanToolkit::GetAcknowledgment(ZdString & Acknowledgment) const {
  try {
    Acknowledgment << ZdString::reset << "You are running SaTScan v" << VERSION_NUMBER;
    Acknowledgment << ".\n\nSaTScan is free, available for download from ";
    Acknowledgment << GetWebSite() << ".\nIt may be used free of charge as long as proper ";
    Acknowledgment << "citations are given\nto both the SaTScan software and the underlying ";
    Acknowledgment << "statistical methodology.\n\n";
  }
  catch (ZdException& x) {
    x.AddCallpath("GetAcknowledgment()", "SaTScanToolkit");
    throw;
  }
  return Acknowledgment.GetCString();
}

/** Returns applications full path */
const char * SaTScanToolkit::GetApplicationFullPath() const {
  return gsApplicationFullPath.GetCString();
}

/** Returns run history filename. */
const char * SaTScanToolkit::GetRunHistoryFileName() /*const*/ {
  return GetSession().GetProperty(gsHistoryFileNameProperty)->GetValue();
}

/** Returns substantive support email address. */
const char * SaTScanToolkit::GetSubstantiveSupportEmail() const {
  //return GetSession().GetProperty(gsSubstantiveSupportEmailProperty)->GetValue();
  return gsDefaultSubstantiveSupportEmail;
}
/** Returns substantive support email address. */
const char * SaTScanToolkit::GetTechnicalSupportEmail() const {
  //return GetSession().GetProperty(gsTechnicalSupportEmailProperty)->GetValue();
  return gsDefaultTechnicalSupportEmail;
}

/** Returns website URL. */
const char * SaTScanToolkit::GetWebSite() const {
  //return GetSession().GetProperty(gsSaTScanWebSiteProperty)->GetValue();
  return gsDefaultSaTScanWebSite;
}

/** Insures run history filename section in ZdIniSession. */
bool SaTScanToolkit::InsureRunHistoryFileName() {
  ZdFileName            FileName;
  ZdString              sDefaultHistoryFileName;
  long                  lPosition;
  BSessionProperty    * pProperty;
  bool                  bUpdatedSection=false;

  try {
    sDefaultHistoryFileName << ZdFileName(gsApplicationFullPath.GetCString()).GetLocation() << gsDefaultRunHistoryFileName;
    sDefaultHistoryFileName << DBFFileType::GetDefaultInstance().GetFileTypeExtension();

    //run history filename property
    lPosition = GetSession().FindProperty(gsHistoryFileNameProperty);
    if (lPosition == -1) {
      GetSession().AddProperty(gsHistoryFileNameProperty, sDefaultHistoryFileName.GetCString());
      bUpdatedSection = true;
    }
    else {
      //property exists but does it have a value?
      pProperty = GetSession().GetProperty(lPosition);
      if (!pProperty->GetValue() || !strlen(pProperty->GetValue())) {
        GetSession().AddProperty(gsHistoryFileNameProperty, sDefaultHistoryFileName.GetCString());
        bUpdatedSection = true;
      }
      else {
        FileName.SetFullPath(pProperty->GetValue());
        //validate extension
        if (strcmp(FileName.GetExtension(), DBFFileType::GetDefaultInstance().GetFileTypeExtension())) {
          FileName.SetExtension(DBFFileType::GetDefaultInstance().GetFileTypeExtension());
          pProperty->SetValue(FileName.GetFullPath());
          bUpdatedSection = true;
        }
        //validate filename
        if (!strlen(FileName.GetFileName())) {
          FileName.SetFileName(gsDefaultRunHistoryFileName);
          pProperty->SetValue(FileName.GetFullPath());
          bUpdatedSection = true;
        }
        //validate path
        if (access(FileName.GetLocation(), 00) < 0) {
          pProperty->SetValue(sDefaultHistoryFileName);
          bUpdatedSection = true;
        }
      }
    }
  }
  catch (ZdException& x) {
    x.AddCallpath("InsureRunHistoryFileName()", "SaTScanToolkit");
    throw;
  }
  return bUpdatedSection;
}

/** */
bool SaTScanToolkit::InsureSessionProperty(const char * sSessionProperty, const char * sDefaultValue) {
  long                  lPosition;
  BSessionProperty    * pProperty;
  bool                  bUpdatedSection=false;

  try {
    //log history property
    lPosition = GetSession().FindProperty(sSessionProperty);
    if (lPosition == -1) {
      GetSession().AddProperty(sSessionProperty, sDefaultValue);
      bUpdatedSection = true;
    }
    else {
      //property exists but does it have a value?
      pProperty = GetSession().GetProperty(lPosition);
      if (!pProperty->GetValue() || !strlen(pProperty->GetValue())) {
        GetSession().AddProperty(sSessionProperty, sDefaultValue);
        bUpdatedSection = true;
      }
    }
  }
  catch (ZdException& x) {
    x.AddCallpath("InsureSessionProperty()", "SaTScanToolkit");
    throw;
  }
  return bUpdatedSection;
}

/** Insures that all section keys are present and writes to disk if
    necessary and permissions permit. */
void SaTScanToolkit::InsureSessionStructure() {
  bool                  bNeedsWrite=false;

  try {
    if (InsureRunHistoryFileName())
      bNeedsWrite = true;
    //if (InsureSessionProperty(gsLoggingProperty, "true"))
    //  bNeedsWrite = true;
    //if (InsureSessionProperty(gsSaTScanWebSiteProperty, gsDefaultSaTScanWebSite))
    // bNeedsWrite = true;
    //if (InsureSessionProperty(gsSubstantiveSupportEmailProperty, gsDefaultSubstantiveSupportEmail))
    //  bNeedsWrite = true;
    //if (InsureSessionProperty(gsTechnicalSupportEmailProperty, gsDefaultTechnicalSupportEmail))
    //  bNeedsWrite = true;

    //Write to same directory as executable, when needed.
    if (bNeedsWrite) {
      try {
        GetSession().Write(gsSystemFileName.GetCString());
      }
      catch (ZdException & x){
         /* File not currently writable.
            NOTE: We really want to catch ZdFileOpenFailedException exceptions here
            but ZdIniSession is causing initial thrown exception type to be lost. */
      }
    }
  }
  catch (ZdException& x) {
    x.AddCallpath("InsureSessionStructure()", "SaTScanToolkit");
    throw;
  }
}

/** internal setup */
void SaTScanToolkit::Setup(const char * sApplicationFullPath) {
  try {
    //set application full path
    gsApplicationFullPath = sApplicationFullPath;
    //Set system ini located at same directory as executable.
    gsSystemFileName << ZdFileName(gsApplicationFullPath.GetCString()).GetLocation() << gsSystemIniFileName;
    
    try {
      //Open or create system ini file.
      GetSession().Read(gsSystemFileName.GetCString(), ZDIO_OPEN_READ);
    }
    catch (ZdException & x){
      /* can't read ini file: missing/corrupt/permissions? */
    }

    InsureSessionStructure();
    SetErrorReportDestination(GetTechnicalSupportEmail());
  }
  catch (ZdException& x) {
    x.AddCallpath("Setup()", "SaTScanToolkit");
    throw;
  }
}

/** Returns referance to toolkit. */
SaTScanToolkit & GetToolkit() {
  return dynamic_cast<SaTScanToolkit&>(BasisGetToolkit());
}


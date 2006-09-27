//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "Toolkit.h"
#include "DBFFile.h"

/** system filename */
const char * AppToolkit::gsSystemIniFileName = "system.ini";
/** ini section property name for run history filename */
const char * AppToolkit::gsHistoryFileNameProperty = "[RunHistory].FileName";
/** ini section property name for parameter history filenames */
const char * AppToolkit::gsParameterNameProperty = "[ParameterHistory].Parameter";
/** maximum number of parameter history items to record */
const size_t AppToolkit::giMaximumParameterHistoryItems = 10;
/** last directory browsed */
const char * AppToolkit::gsLastDirectoryProperty = "[LastDirectory].Path";
/** last import destination directory browsed */
const char * AppToolkit::gsLastImportDestinationDirectoryProperty = "[LastDirectory].ImportDestination";

/** ini section property name for logging run history */
//const char * AppToolkit::gsLoggingProperty = "[RunHistory].LogHistory";
/** ini section property name for website */
//const char * AppToolkit::gsSaTScanWebSiteProperty = "[Internet].WebsiteURL";
/** ini section property name for license website */
//const char * AppToolkit::gsSaTScanLicenceWebSiteProperty = "[Internet].LicenceURL";
/** ini section property name for substantive support email */
//const char * AppToolkit::gsSubstantiveSupportEmailProperty = "[Email].SubstantiveSupportEmail";
/** ini section property name for technical support email */
//const char * AppToolkit::gsTechnicalSupportEmailProperty = "[Email].TechnicalSupportEmail";
/** analysis history filename */
const char * AppToolkit::gsDefaultRunHistoryFileName = "AnalysisHistory";
/** Default website. */
#ifdef INTEL_BASED
const char * AppToolkit::gsDefaultSaTScanWebSite = "http:/\/www.satscan.org/";
#else
const char * AppToolkit::gsDefaultSaTScanWebSite = "http://www.satscan.org/";
#endif
/** Default Substantive Support Email. */
const char * AppToolkit::gsDefaultSubstantiveSupportEmail = "kulldorff@satscan.org";
/** Default Technical Support Email. */
const char * AppToolkit::gsDefaultTechnicalSupportEmail = "techsupport@satscan.org";
AppToolkit * AppToolkit::gpToolKit = 0;

void AppToolkit::ToolKitCreate(const char * sApplicationFullPath) {
  AppToolkit::gpToolKit = new AppToolkit(sApplicationFullPath);
}

void AppToolkit::ToolKitDestroy() {
  try {delete AppToolkit::gpToolKit; AppToolkit::gpToolKit=0;}catch(...){}
}

/** constructor */
AppToolkit::AppToolkit(const char * sApplicationFullPath) {
  try {
    Setup(sApplicationFullPath);
  }
  catch (ZdException& x) {
    x.AddCallpath("constructor()", "AppToolkit");
    throw;
  }
}

/** destructor */
AppToolkit::~AppToolkit() {
  try {
    ZdString  theDrive, theDirectory;
    ZdFileName::GetCurDrive(theDrive);
    theDrive << ZdFileName::GetCurDirectory(theDirectory);
    SetLastDirectory(theDrive.GetCString());
  }
  catch (...){}
}

/** Adds parameter filename to parameter history. */
void AppToolkit::AddParameterToHistory(const char * sParameterFileName) {
  std::vector<std::string>::iterator      itr;

  try {
    if (sParameterFileName && strlen(sParameterFileName)) {
      if (gvParameterHistory.empty())
        gvParameterHistory.push_back(sParameterFileName);
      else {
         itr = std::find(gvParameterHistory.begin(), gvParameterHistory.end(), sParameterFileName);
         if (itr == gvParameterHistory.begin())
           return;
         else if (itr == gvParameterHistory.end()) {
           if (gvParameterHistory.size() == giMaximumParameterHistoryItems)
             gvParameterHistory.pop_back();
           gvParameterHistory.insert(gvParameterHistory.begin(), sParameterFileName);
         }
         else {
           gvParameterHistory.erase(itr);
           gvParameterHistory.insert(gvParameterHistory.begin(), sParameterFileName);
         }
      }
      WriteParametersHistory();
    }
  }
  catch (ZdException& x) {
    x.AddCallpath("AddParameterToHistory()", "AppToolkit");
    throw;
  }
}

/** Returns acknowledgment statement indicating program version, website, and
    brief declaration of usage agreement. */
const char * AppToolkit::GetAcknowledgment(ZdString & Acknowledgment) const {
  try {
    Acknowledgment << ZdString::reset << "You are running SaTScan v" << GetVersion();
    Acknowledgment << ".\n\nSaTScan is free, available for download from ";
    Acknowledgment << GetWebSite() << ".\nIt may be used free of charge as long as proper ";
    Acknowledgment << "citations are given\nto both the SaTScan software and the underlying ";
    Acknowledgment << "statistical methodology.\n\n";
  }
  catch (ZdException& x) {
    x.AddCallpath("GetAcknowledgment()", "AppToolkit");
    throw;
  }
  return Acknowledgment.GetCString();
}

/** Returns applications full path */
const char * AppToolkit::GetApplicationFullPath() const {
  return gsApplicationFullPath.GetCString();
}

/** last opened directory */
const char * AppToolkit::GetLastDirectory() /*const*/ {
  return gSession.GetProperty(gsLastDirectoryProperty)->GetValue();
}

const char * AppToolkit::GetLastImportDirectory() /*const*/ {
  return gSession.GetProperty(gsLastImportDestinationDirectoryProperty)->GetValue();
}

/** Returns run history filename. */
const char * AppToolkit::GetRunHistoryFileName() /*const*/ {
  return gSession.GetProperty(gsHistoryFileNameProperty)->GetValue();
}

/** Returns substantive support email address. */
const char * AppToolkit::GetSubstantiveSupportEmail() const {
  //return gSession.GetProperty(gsSubstantiveSupportEmailProperty)->GetValue();
  return gsDefaultSubstantiveSupportEmail;
}
/** Returns substantive support email address. */
const char * AppToolkit::GetTechnicalSupportEmail() const {
  //return gSession.GetProperty(gsTechnicalSupportEmailProperty)->GetValue();
  return gsDefaultTechnicalSupportEmail;
}

/** Returns website URL. */
const char * AppToolkit::GetWebSite() const {
  //return gSession.GetProperty(gsSaTScanWebSiteProperty)->GetValue();
  return gsDefaultSaTScanWebSite;
}

/** Insures last directory path section in ZdIniSession. */
bool AppToolkit::InsureLastDirectoryPath() {
  ZdFileName            FileName;
  ZdString              sDefaultPath;
  long                  lPosition;
  BSessionProperty    * pProperty;
  bool                  bUpdatedSection=false;

  try {
    sDefaultPath << ZdFileName(gsApplicationFullPath.GetCString()).GetLocation() << "sample data" << ZDFILENAME_SLASH;
    if (access(sDefaultPath.GetCString(), 00) < 0)
      sDefaultPath = ZdFileName(gsApplicationFullPath.GetCString()).GetLocation();

    //run history filename property
    lPosition = gSession.FindProperty(gsLastDirectoryProperty);
    if (lPosition == -1) {
      gSession.AddProperty(gsLastDirectoryProperty, sDefaultPath.GetCString());
      bUpdatedSection = true;
    }
    else {
      //property exists but does it have a value?
      pProperty = gSession.GetProperty(lPosition);
      if (!pProperty->GetValue() || !strlen(pProperty->GetValue())) {
        gSession.AddProperty(gsLastDirectoryProperty, sDefaultPath.GetCString());
        bUpdatedSection = true;
      }
      else {
        FileName.SetFullPath(pProperty->GetValue());
        if (strlen(FileName.GetFileName()) || strlen(FileName.GetExtension())) {
          pProperty->SetValue(sDefaultPath.GetCString());
          bUpdatedSection = true;
        }
        //validate path
        else if (access(FileName.GetLocation(), 00) < 0) {
          pProperty->SetValue(sDefaultPath.GetCString());        
          bUpdatedSection = true;
        }
      }
    }
  }
  catch (ZdException& x) {
    x.AddCallpath("InsureLastDirectoryPath()", "AppToolkit");
    throw;
  }
  return bUpdatedSection;
}

/** Insures last imported directory path section in ZdIniSession. */
bool AppToolkit::InsureLastImportDestinationDirectoryPath() {
  ZdFileName            FileName;
  ZdString              sDefaultPath;
  long                  lPosition;
  BSessionProperty    * pProperty;
  bool                  bUpdatedSection=false;

  try {
    sDefaultPath = getenv("TMP") ? getenv("TMP") : "";
    lPosition = gSession.FindProperty(gsLastImportDestinationDirectoryProperty);
    if (lPosition == -1) {
      gSession.AddProperty(gsLastImportDestinationDirectoryProperty, sDefaultPath.GetCString());
      bUpdatedSection = true;
    }
    else {
      //property exists but does it have a value?
      pProperty = gSession.GetProperty(lPosition);
      if (!pProperty->GetValue() || !strlen(pProperty->GetValue())) {
        gSession.AddProperty(gsLastImportDestinationDirectoryProperty, sDefaultPath.GetCString());
        bUpdatedSection = true;
      }
      else {
        FileName.SetFullPath(pProperty->GetValue());
        //validate path
        if (access(FileName.GetLocation(), 00) < 0) {
          pProperty->SetValue(sDefaultPath.GetCString());        
          bUpdatedSection = true;
        }
      }
    }
  }
  catch (ZdException& x) {
    x.AddCallpath("InsureLastImportDestinationDirectoryPath()", "AppToolkit");
    throw;
  }
  return bUpdatedSection;
}

/** Insures run history filename section in ZdIniSession. */
bool AppToolkit::InsureRunHistoryFileName() {
  ZdFileName            FileName;
  ZdString              sDefaultHistoryFileName;
  long                  lPosition;
  BSessionProperty    * pProperty;
  bool                  bUpdatedSection=false;

  try {
    sDefaultHistoryFileName << ZdFileName(gsApplicationFullPath.GetCString()).GetLocation() << gsDefaultRunHistoryFileName;
    sDefaultHistoryFileName << DBFFileType::GetDefaultInstance().GetFileTypeExtension();

    //run history filename property
    lPosition = gSession.FindProperty(gsHistoryFileNameProperty);
    if (lPosition == -1) {
      gSession.AddProperty(gsHistoryFileNameProperty, sDefaultHistoryFileName.GetCString());
      bUpdatedSection = true;
    }
    else {
      //property exists but does it have a value?
      pProperty = gSession.GetProperty(lPosition);
      if (!pProperty->GetValue() || !strlen(pProperty->GetValue())) {
        gSession.AddProperty(gsHistoryFileNameProperty, sDefaultHistoryFileName.GetCString());
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
    x.AddCallpath("InsureRunHistoryFileName()", "AppToolkit");
    throw;
  }
  return bUpdatedSection;
}

/** */
bool AppToolkit::InsureSessionProperty(const char * sSessionProperty, const char * sDefaultValue) {
  long                  lPosition;
  BSessionProperty    * pProperty;
  bool                  bUpdatedSection=false;

  try {
    //log history property
    lPosition = gSession.FindProperty(sSessionProperty);
    if (lPosition == -1) {
      gSession.AddProperty(sSessionProperty, sDefaultValue);
      bUpdatedSection = true;
    }
    else {
      //property exists but does it have a value?
      pProperty = gSession.GetProperty(lPosition);
      if (!pProperty->GetValue() || !strlen(pProperty->GetValue())) {
        gSession.AddProperty(sSessionProperty, sDefaultValue);
        bUpdatedSection = true;
      }
    }
  }
  catch (ZdException& x) {
    x.AddCallpath("InsureSessionProperty()", "AppToolkit");
    throw;
  }
  return bUpdatedSection;
}

/** Insures that all section keys are present and writes to disk if
    necessary and permissions permit. */
void AppToolkit::InsureSessionStructure() {
  bool                  bNeedsWrite=false;

  try {
    if (InsureRunHistoryFileName())
      bNeedsWrite = true;
    if (InsureLastDirectoryPath())
      bNeedsWrite = true;
    if (InsureLastImportDestinationDirectoryPath())
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
        gSession.Write(gsSystemFileName.GetCString());
      }
      catch (ZdException & x){
         /* File not currently writable.
            NOTE: We really want to catch ZdFileOpenFailedException exceptions here
            but ZdIniSession is causing initial thrown exception type to be lost. */
      }
    }
  }
  catch (ZdException& x) {
    x.AddCallpath("InsureSessionStructure()", "AppToolkit");
    throw;
  }
}

/** reads parameter history from ini file */
void AppToolkit::ReadParametersHistory() {
  long                  lPosition;
  BSessionProperty    * pProperty;
  ZdString              sParameterSectionName;
  int                   iItem=0;

  try {
    sParameterSectionName.printf("%s%i", gsParameterNameProperty, iItem++);
    lPosition = gSession.FindProperty(sParameterSectionName.GetCString());
    while (lPosition != -1) {
      pProperty = gSession.GetProperty(lPosition);
      if (pProperty->GetValue() && strlen(pProperty->GetValue()))
        gvParameterHistory.push_back(pProperty->GetValue());
      sParameterSectionName.printf("%s%i", gsParameterNameProperty, iItem++);
      lPosition = gSession.FindProperty(sParameterSectionName.GetCString());
    }
  }
  catch (ZdException& x) {
    x.AddCallpath("ReadParametersHistory()","AppToolkit");
    throw;
  }
}

void AppToolkit::SetLastDirectory(const char * sLastDirectory) {
  gSession.AddProperty(gsLastDirectoryProperty, sLastDirectory);
  gSession.Write(gsSystemFileName.GetCString());
}

void AppToolkit::SetLastImportDirectory(const char * sLastDirectory) {
  gSession.AddProperty(gsLastImportDestinationDirectoryProperty, sLastDirectory);
  gSession.Write(gsSystemFileName.GetCString());
}

/** internal setup */
void AppToolkit::Setup(const char * sApplicationFullPath) {
  try {
    gbRunUpdateOnTerminate = false;
    //set application full path
    gsApplicationFullPath = sApplicationFullPath;
    //Set system ini located at same directory as executable.
    gsSystemFileName << ZdFileName(gsApplicationFullPath.GetCString()).GetLocation() << gsSystemIniFileName;
    try {
      //Open or create system ini file.
      gSession.Read(gsSystemFileName.GetCString(), ZDIO_OPEN_READ);
    }
    catch (ZdException & x){
      /* can't read ini file: missing/corrupt/permissions? */
    }

    InsureSessionStructure();
    //SetErrorReportDestination(GetTechnicalSupportEmail());
    ReadParametersHistory();
    gsVersion.printf("%s.%s%s%s%s%s", VERSION_MAJOR, VERSION_MINOR,
                     (!strcmp(VERSION_RELEASE, "0") ? "" : "."),
                     (!strcmp(VERSION_RELEASE, "0") ? "" : VERSION_RELEASE),
                     (strlen(VERSION_PHASE) ? " " : ""), VERSION_PHASE);
  }
  catch (ZdException& x) {
    x.AddCallpath("Setup()", "AppToolkit");
    throw;
  }
}

/** Writes parameter history to ini file */
void AppToolkit::WriteParametersHistory() {
  ZdString                        sParameterSectionName;
  ParameterHistory_t::iterator    itr;
  int                             iItem=0;

  try {
    for (itr=gvParameterHistory.begin(); itr != gvParameterHistory.end(); itr++) {
       sParameterSectionName.printf("%s%i", gsParameterNameProperty, iItem++);
       gSession.AddProperty(sParameterSectionName.GetCString(), itr->c_str());
    }
    gSession.Write(gsSystemFileName.GetCString());
  }
  catch (ZdException& x) {
    x.AddCallpath("WriteParametersHistory()","AppToolkit");
    throw;
  }
}


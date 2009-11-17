//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "Toolkit.h"
#include "dBaseFile.h"
#include "UtilityFunctions.h"
#include "FileName.h"
#include "SSException.h"

/** checks for existance of section/key */
bool IniSession::exists(const char * sSectionName, const char * sKeyName) const {
  long section_index = gIniFile.GetSectionIndex(sSectionName);
  if (section_index == -1)
    return false;
  return (gIniFile.GetSection(section_index)->FindKey(sKeyName) == -1 ? false :true);
}

/** returns value associated with section/key - throws prg_error if not found */
const char * IniSession::get(const char * sSectionName, const char * sKeyName) const {
  long section_index = gIniFile.GetSectionIndex(sSectionName);
  if (section_index == -1)
    throw prg_error("Section '%s' does not exist.", "GetValue()", sSectionName);
  const IniSection * pSection = gIniFile.GetSection(section_index);
  long key_index = pSection->FindKey(sKeyName);
  if (key_index == -1)
    throw prg_error("Key '%s' does not exist in section '%s'.", "GetValue()", sKeyName, sSectionName);
  return pSection->GetLine(key_index)->GetValue();
}

/** reads ini sections from file */
void IniSession::read(const std::string& file) {
  gIniFile.Clear();
  gIniFile.Read(file);
}

/** sets value associated with section/key - adds section and/or key if non-existant */
void IniSession::set(const char * sSectionName, const char * sKeyName, const char * sValue) {
  IniSection * pSection;

  long section_index = gIniFile.GetSectionIndex(sSectionName);
  if (section_index == -1)
    pSection = gIniFile.AddSection(sSectionName);
  else
    pSection = gIniFile.GetSection(section_index);
  long key_index = pSection->FindKey(sKeyName);
  if (key_index == -1)
    pSection->AddLine(sKeyName, sValue);
  else
    pSection->GetLine(key_index)->SetValue(sValue);
}

/** writes ini sections to file */
void IniSession::write(const std::string& file) const {
  gIniFile.Write(file);
}

/** system filename */
const char * AppToolkit::gsSystemIniFileName = "system.ini";
/** ini section property name for run history filename */
const char * AppToolkit::gsRunHistory = "[RunHistory]";
const char * AppToolkit::gsHistoryFileNameProperty = "FileName";
/** ini section property name for parameter history filenames */
const char * AppToolkit::gsParameterHistory = "[ParameterHistory]";
const char * AppToolkit::gsParameterNameProperty = "Parameter";
/** maximum number of parameter history items to record */
const size_t AppToolkit::giMaximumParameterHistoryItems = 10;
/** last directory browsed */
const char * AppToolkit::gsLastDirectory = "[LastDirectory]";
const char * AppToolkit::gsLastDirectoryPathProperty = "Path";
/** last import destination directory browsed */
const char * AppToolkit::gsLastImportDestinationDirectoryProperty = "ImportDestination";
/** debug file name */
const char * AppToolkit::gsDebugFileName = "_debug";

/** analysis history filename */
const char * AppToolkit::gsDefaultRunHistoryFileName = "AnalysisHistory";
/** Default website. */
const char * AppToolkit::gsDefaultSaTScanWebSite = "http://www.satscan.org/";
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
AppToolkit::AppToolkit(const char * sApplicationFullPath) : gpDebugLog(0) {
  try {
    Setup(sApplicationFullPath);
  }
  catch (prg_exception& x) {
    x.addTrace("constructor()", "AppToolkit");
    throw;
  }
}

/** destructor */
AppToolkit::~AppToolkit() {
  try {
    std::string theDrive, theDirectory;
    FileName::getCurDrive(theDrive);
    theDrive += FileName::getCurDirectory(theDirectory);
    SetLastDirectory(theDrive.c_str());
    closeDebugFile();
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
  catch (prg_exception& x) {
    x.addTrace("AddParameterToHistory()", "AppToolkit");
    throw;
  }
}

/** Close debug file handle. */
void AppToolkit::closeDebugFile() {
   if (gpDebugLog) fclose(gpDebugLog);
}


/** Returns acknowledgment statement indicating program version, website, and
    brief declaration of usage agreement. */
const char * AppToolkit::GetAcknowledgment(std::string & Acknowledgment) const {
  printString(Acknowledgment, "You are running SaTScan v%s%s.\n\nSaTScan is free, available for download from %s"
                              ".\nIt may be used free of charge as long as proper "
                              "citations are given\nto both the SaTScan software and the underlying "
                              "statistical methodology.\n\n", GetVersion(), is64Bit() ? " (64-bit)" : "",GetWebSite());
  return Acknowledgment.c_str();
}

/** Returns applications full path */
const char * AppToolkit::GetApplicationFullPath() const {
  return gsApplicationFullPath.c_str();
}

/** last opened directory */
const char * AppToolkit::GetLastDirectory() /*const*/ {
  return gSession.get(gsLastDirectory, gsLastDirectoryPathProperty);
}

const char * AppToolkit::GetLastImportDirectory() /*const*/ {
  return gSession.get(gsLastDirectory, gsLastImportDestinationDirectoryProperty);
}

/** Returns run history filename. */
const char * AppToolkit::GetRunHistoryFileName() /*const*/ {
  return gSession.get(gsRunHistory, gsHistoryFileNameProperty);
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

/** Insures last directory path section in IniSession. */
bool AppToolkit::InsureLastDirectoryPath() {
  FileName              app(gsApplicationFullPath.c_str());
  std::string           buffer;
  std::stringstream     sDefaultPath;
  bool                  bUpdatedSection=false;

  try {
    sDefaultPath << app.getLocation(buffer) << "sample data" << FileName::SLASH;
    if (access(sDefaultPath.str().c_str(), 00) < 0) {
      sDefaultPath.str("");
      sDefaultPath << app.getLocation(buffer);
    }

    //run history filename property
    if (!gSession.exists(gsLastDirectory, gsLastDirectoryPathProperty)) {
      gSession.set(gsLastDirectory, gsLastDirectoryPathProperty, sDefaultPath.str().c_str());
      bUpdatedSection = true;
    }
    else {
      //property exists but does it have a value?
      std::string value = gSession.get(gsLastDirectory, gsLastDirectoryPathProperty);
      if (value.size() == 0) {
        gSession.set(gsLastDirectory, gsLastDirectoryPathProperty, sDefaultPath.str().c_str());
        bUpdatedSection = true;
      }
      else {
        app.setFullPath(value.c_str());
        if (strlen(app.getFileName().c_str()) || strlen(app.getExtension().c_str())) {
          gSession.set(gsLastDirectory, gsLastDirectoryPathProperty, sDefaultPath.str().c_str());
          bUpdatedSection = true;
        }
        //validate path
        else if (access(app.getLocation(buffer).c_str(), 00) < 0) {
          gSession.set(gsLastDirectory, gsLastDirectoryPathProperty, sDefaultPath.str().c_str());  
          bUpdatedSection = true;
        }
      }
    }
  }
  catch (prg_exception& x) {
    x.addTrace("InsureLastDirectoryPath()", "AppToolkit");
    throw;
  }
  return bUpdatedSection;
}

/** Insures last imported directory path section in IniSession. */
bool AppToolkit::InsureLastImportDestinationDirectoryPath() {
  FileName              path;
  std::string           sDefaultPath, buffer;
  bool                  bUpdatedSection=false;

  try {
    sDefaultPath = getenv("TMP") ? getenv("TMP") : "";
    if (!gSession.exists(gsLastDirectory, gsLastImportDestinationDirectoryProperty)) {
      gSession.set(gsLastDirectory, gsLastImportDestinationDirectoryProperty, sDefaultPath.c_str());
      bUpdatedSection = true;
    }
    else {
      //property exists but does it have a value?
      std::string value = gSession.get(gsLastDirectory, gsLastImportDestinationDirectoryProperty);
      if (value.size() == 0) {
        gSession.set(gsLastDirectory, gsLastImportDestinationDirectoryProperty, sDefaultPath.c_str());
        bUpdatedSection = true;
      }
      else {
        path.setFullPath(value.c_str());
        //validate path
        if (access(path.getLocation(buffer).c_str(), 00) < 0) {
          gSession.set(gsLastDirectory, gsLastImportDestinationDirectoryProperty, sDefaultPath.c_str());     
          bUpdatedSection = true;
        }
      }
    }
  }
  catch (prg_exception& x) {
    x.addTrace("InsureLastImportDestinationDirectoryPath()", "AppToolkit");
    throw;
  }
  return bUpdatedSection;
}

/** Insures run history filename section in IniSession. */
bool AppToolkit::InsureRunHistoryFileName() {
  FileName              app(gsApplicationFullPath.c_str());
  std::string           buffer;
  std::stringstream     sDefaultHistoryFileName;
  bool                  bUpdatedSection=false;

  try {
    sDefaultHistoryFileName << app.getLocation(buffer) << gsDefaultRunHistoryFileName;
    sDefaultHistoryFileName << dBaseFile::GetFileTypeExtension();

    //run history filename property
    if (!gSession.exists(gsRunHistory, gsHistoryFileNameProperty)) {
      gSession.set(gsRunHistory, gsHistoryFileNameProperty, sDefaultHistoryFileName.str().c_str());
      bUpdatedSection = true;
    }
    else {
      //property exists but does it have a value?
      std::string value = gSession.get(gsRunHistory, gsHistoryFileNameProperty);
      if (value.size() == 0) {
        gSession.set(gsRunHistory, gsHistoryFileNameProperty, sDefaultHistoryFileName.str().c_str());
        bUpdatedSection = true;
      }
      else {
        app.setFullPath(value.c_str());
        //validate extension
        if (strcmp(app.getExtension().c_str(), dBaseFile::GetFileTypeExtension())) {
          app.setExtension(dBaseFile::GetFileTypeExtension());
          gSession.set(gsRunHistory, gsHistoryFileNameProperty, app.getFullPath(buffer).c_str());
          bUpdatedSection = true;
        }
        //validate filename
        if (!strlen(app.getFileName().c_str())) {
          app.setFileName(gsDefaultRunHistoryFileName);
          gSession.set(gsRunHistory, gsHistoryFileNameProperty, app.getFullPath(buffer).c_str());
          bUpdatedSection = true;
        }
        //validate path
        if (access(app.getLocation(buffer).c_str(), 00) < 0) {
          gSession.set(gsRunHistory, gsHistoryFileNameProperty, sDefaultHistoryFileName.str().c_str());
          bUpdatedSection = true;
        }
      }
    }
  }
  catch (prg_exception& x) {
    x.addTrace("InsureRunHistoryFileName()", "AppToolkit");
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
    //Write to same directory as executable, when needed.
    if (bNeedsWrite) {
      try {
        gSession.write(gsSystemFileName.c_str());
      }
      catch (prg_exception& x){
      /* If we are unable to write to file, that's fine; we'll just use defaults
         next time IniFile is used. */
      }
    }
  }
  catch (prg_exception& x) {
    x.addTrace("InsureSessionStructure()", "AppToolkit");
    throw;
  }
}

/** Returns whether binary is 64-bit. */
bool AppToolkit::is64Bit() const {
  return sizeof(int *) == 8;
}

/** Returns file handle to global debug file. */
FILE * AppToolkit::openDebugFile() {
  try {
	if (!gpDebugLog) {
      std::string filename;
      filename = FileName(gsApplicationFullPath.c_str()).getLocation(filename);
      filename += gsDebugFileName;
      filename += gsVersion;
      if ((gpDebugLog = fopen(filename.c_str(), /*"a"*/"w")) == NULL)
        throw resolvable_error("Error: Debug file '%s' could not be created.\n", filename.c_str());
	}
  }
  catch (prg_exception& x) {
    x.addTrace("openDebugFile()", "AppToolkit");
    throw;
  }
  return gpDebugLog;
}


/** reads parameter history from ini file */
void AppToolkit::ReadParametersHistory() {
  std::string           parametername;
  int                   iItem=0;

  try {
    printString(parametername, "%s%i", gsParameterNameProperty, iItem++);
    while (gSession.exists(gsParameterHistory, parametername.c_str())) {
      std::string value = gSession.get(gsParameterHistory, parametername.c_str());
      if (value.size()) gvParameterHistory.push_back(value);
      printString(parametername, "%s%i", gsParameterNameProperty, iItem++);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("ReadParametersHistory()","AppToolkit");
    throw;
  }
}

void AppToolkit::SetLastDirectory(const char * sLastDirectory) {
  gSession.set(gsLastDirectory, gsLastDirectoryPathProperty, sLastDirectory);
  gSession.write(gsSystemFileName);
}

void AppToolkit::SetLastImportDirectory(const char * sLastDirectory) {
  gSession.set(gsLastDirectory, gsLastImportDestinationDirectoryProperty, sLastDirectory);
  gSession.write(gsSystemFileName);
}

/** internal setup */
void AppToolkit::Setup(const char * sApplicationFullPath) {
  try {
    gbRunUpdateOnTerminate = false;
    //set application full path
    gsApplicationFullPath = sApplicationFullPath;
    //Set system ini located at same directory as executable.
    std::string buffer;
    gsSystemFileName = FileName(gsApplicationFullPath.c_str()).getLocation(buffer);
    gsSystemFileName += gsSystemIniFileName;
    try {
      //Open or create system ini file.
      gSession.read(gsSystemFileName);
    }
    catch (prg_exception& x) {
      /* If we are unable to read the file, that's fine; we'll just use defaults
         assigned to in memory IniFile. */
    }

    InsureSessionStructure();
    //SetErrorReportDestination(GetTechnicalSupportEmail());
    ReadParametersHistory();
    printString(gsVersion, "%s.%s%s%s%s%s", VERSION_MAJOR, VERSION_MINOR,
                          (!strcmp(VERSION_RELEASE, "0") ? "" : "."),
                          (!strcmp(VERSION_RELEASE, "0") ? "" : VERSION_RELEASE),
                          (strlen(VERSION_PHASE) ? " " : ""), VERSION_PHASE);
  }
  catch (prg_exception& x) {
    x.addTrace("Setup()", "AppToolkit");
    throw;
  }
}

/** Writes parameter history to ini file */
void AppToolkit::WriteParametersHistory() {
  std::string                     sParameterSectionName;
  ParameterHistory_t::iterator    itr;
  int                             iItem=0;

  try {
    for (itr=gvParameterHistory.begin(); itr != gvParameterHistory.end(); itr++) {
       printString(sParameterSectionName, "%s%i", gsParameterNameProperty, iItem++);
       gSession.set(gsParameterHistory, sParameterSectionName.c_str(), itr->c_str());
    }
    gSession.write(gsSystemFileName.c_str());
  }
  catch (prg_exception& x) {
    x.addTrace("WriteParametersHistory()","AppToolkit");
    throw;
  }
}


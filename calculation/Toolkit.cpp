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
    long section_index = _ini_file.GetSectionIndex(sSectionName);
    if (section_index == -1)
        return false;
    return (_ini_file.GetSection(section_index)->FindKey(sKeyName) == -1 ? false :true);
}

/** returns value associated with section/key - throws prg_error if not found */
const char * IniSession::get(const char * sSectionName, const char * sKeyName) const {
    long section_index = _ini_file.GetSectionIndex(sSectionName);
    if (section_index == -1)
        throw prg_error("Section '%s' does not exist.", "GetValue()", sSectionName);
    const IniSection * pSection = _ini_file.GetSection(section_index);
    long key_index = pSection->FindKey(sKeyName);
    if (key_index == -1)
        throw prg_error("Key '%s' does not exist in section '%s'.", "GetValue()", sKeyName, sSectionName);
    return pSection->GetLine(key_index)->GetValue();
}

/** reads ini sections from file */
void IniSession::read(const std::string& file) {
    _ini_file.Clear();
    _ini_file.Read(file);
}

/** sets value associated with section/key - adds section and/or key if non-existant */
void IniSession::set(const char * sSectionName, const char * sKeyName, const char * sValue) {
    IniSection * pSection;

    long section_index = _ini_file.GetSectionIndex(sSectionName);
    if (section_index == -1)
        pSection = _ini_file.AddSection(sSectionName);
    else
        pSection = _ini_file.GetSection(section_index);
    long key_index = pSection->FindKey(sKeyName);
    if (key_index == -1)
        pSection->AddLine(sKeyName, sValue);
    else
        pSection->GetLine(key_index)->SetValue(sValue);
}

/** writes ini sections to file */
void IniSession::write(const std::string& file) const {
    _ini_file.Write(file);
}

/** system filename */
const char * AppToolkit::_ini_filename = "satscan.ini";
/** ini section property name for run history filename */
const char * AppToolkit::_run_history = "[RunHistory]";
const char * AppToolkit::_history_filename_property = "FileName";
/** debug file name */
const char * AppToolkit::_debug_filename = "_debug_";
/** multiple analysis file name */
const char * AppToolkit::_multiple_analysis_filename = "batch-settings.xml";

/** analysis history filename */
const char * AppToolkit::_default_run_history_filename = "AnalysisHistory";
/** Default website. */
const char * AppToolkit::_webSite = "https://www.satscan.org/";
/** Default Substantive Support Email. */
const char * AppToolkit::_substantive_support_email = "kulldorff@satscan.org";
/** Default Technical Support Email. */
const char * AppToolkit::_technical_support_email = "techsupport@satscan.org";
AppToolkit * AppToolkit::_tool_kit = 0;

void AppToolkit::ToolKitCreate(const char * sApplicationFullPath) {
    AppToolkit::_tool_kit = new AppToolkit(sApplicationFullPath);
}

void AppToolkit::ToolKitDestroy() {
    try {delete AppToolkit::_tool_kit; AppToolkit::_tool_kit =0;}catch(...){}
}

/** constructor */
AppToolkit::AppToolkit(const char * sApplicationFullPath) : _debug_log(0) {
    try {
        Setup(sApplicationFullPath);
    } catch (prg_exception& x) {
        x.addTrace("constructor()", "AppToolkit");
        throw;
    }
}

/** destructor */
AppToolkit::~AppToolkit() {
    try {
        closeDebugFile();
    } catch (...){}
}

/** Close debug file handle. */
void AppToolkit::closeDebugFile() {
    if (_debug_log) fclose(_debug_log);
}

/** Returns acknowledgment statement indicating program version, website, and
    brief declaration of usage agreement. */
const char * AppToolkit::GetAcknowledgment(std::string & Acknowledgment) const {
    printString(Acknowledgment, "You are running SaTScan v%s%s.\n\nSaTScan is free, available for download from %s"
                                  ".\nIt may be used free of charge as long as proper "
                                  "citations are given\nto both the SaTScan software and the underlying "
                                  "statistical methodology.\n\n", GetVersion(), is64Bit() ? " " : " (32-bit)", GetWebSite());
    return Acknowledgment.c_str();
}

/** Returns applications full path */
const char * AppToolkit::GetApplicationFullPath() const {
    return _application_fullpath.c_str();
}

/** Returns run history filename. */
const char * AppToolkit::GetRunHistoryFileName() /*const*/ {
    return _session.get(_run_history, _history_filename_property);
}

/** Returns substantive support email address. */
const char * AppToolkit::GetSubstantiveSupportEmail() const {
    return _substantive_support_email;
}
/** Returns substantive support email address. */
const char * AppToolkit::GetTechnicalSupportEmail() const {
    return _technical_support_email;
}

/** Returns website URL. */
const char * AppToolkit::GetWebSite() const {
    return _webSite;
}

/** Insures run history filename section in IniSession. */
bool AppToolkit::ensureRunHistory() {
    FileName app(_satscan_appdata_folder.string().c_str());
    std::string buffer;
    std::stringstream defaultfilename;
    bool bUpdatedSection=false;

    try {
        defaultfilename << app.getLocation(buffer) << _default_run_history_filename << dBaseFile::GetFileTypeExtension();

        //run history filename property
        if (!_session.exists(_run_history, _history_filename_property)) {
            _session.set(_run_history, _history_filename_property, defaultfilename.str().c_str());
            bUpdatedSection = true;
        } else {
            //property exists but does it have a value?
            std::string value = _session.get(_run_history, _history_filename_property);
            if (value.size() == 0) {
                _session.set(_run_history, _history_filename_property, defaultfilename.str().c_str());
                bUpdatedSection = true;
            }  else {
                app.setFullPath(value.c_str());
                //validate extension
                if (strcmp(app.getExtension().c_str(), dBaseFile::GetFileTypeExtension())) {
                    app.setExtension(dBaseFile::GetFileTypeExtension());
                    _session.set(_run_history, _history_filename_property, app.getFullPath(buffer).c_str());
                    bUpdatedSection = true;
                }
                //validate filename
                if (!strlen(app.getFileName().c_str())) {
                    app.setFileName(_default_run_history_filename);
                    _session.set(_run_history, _history_filename_property, app.getFullPath(buffer).c_str());
                    bUpdatedSection = true;
                }
                //validate path
                if (access(app.getLocation(buffer).c_str(), 00) < 0) {
                    _session.set(_run_history, _history_filename_property, defaultfilename.str().c_str());
                    bUpdatedSection = true;
                }
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("ensureRunHistory()", "AppToolkit");
        throw;
    }
    return bUpdatedSection;
}

/** Insures that all section keys are present and writes to disk if
    necessary and permissions permit. */
void AppToolkit::InsureSessionStructure() {
    bool bNeedsWrite=false;

    try {
        if (ensureRunHistory()) {
            try {
                _session.write(_ini_filepath.string());
            } catch (prg_exception& x){
                /* If we are unable to write to file, that's fine; we'll just use defaults next time IniFile is used. */
            }
        }
    } catch (prg_exception& x) {
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
	    if (!_debug_log) {
            std::string filename;
            filename = FileName(_satscan_appdata_folder.string().c_str()).getLocation(filename);
            filename += _debug_filename;
            filename += ".log";
            if ((_debug_log = fopen(filename.c_str(), /*"a"*/"w")) == NULL)
                throw resolvable_error("Error: Debug file '%s' could not be created.\n", filename.c_str());
	    }
    } catch (prg_exception& x) {
        x.addTrace("openDebugFile()", "AppToolkit");
        throw;
    }
    return _debug_log;
}

/** internal setup */
void AppToolkit::Setup(const char * sApplicationFullPath) {
    try {
        _application_fullpath = sApplicationFullPath;

        // define base directory for user files
        FileName applicationnamepath(sApplicationFullPath);
        std::string buffer, app_path;

        _satscan_appdata_folder = boost::filesystem::path(GetUserDirectory(buffer, applicationnamepath.getLocation(app_path)).c_str());
        _satscan_appdata_folder.concat("/").concat(".satscan").concat("/").make_preferred();
        boost::filesystem::create_directory(_satscan_appdata_folder);
        // define filename of ini settings file
        _ini_filepath = _satscan_appdata_folder;
        _ini_filepath.concat(_ini_filename);
        _multi_analysis_filepath = _satscan_appdata_folder;
        _multi_analysis_filepath.concat(_multiple_analysis_filename);
        try {
            //Open or create system ini file.
            _session.read(_ini_filepath.string());
        } catch (prg_exception& x) {
        /* If we are unable to read the file, that's fine; we'll just use defaults assigned to in memory IniFile. */
        }

        InsureSessionStructure();
        printString(_version, "%s.%s%s%s%s%s", VERSION_MAJOR, VERSION_MINOR,
                             (!strcmp(VERSION_RELEASE, "0") ? "" : "."),
                             (!strcmp(VERSION_RELEASE, "0") ? "" : VERSION_RELEASE),
                             (strlen(VERSION_PHASE) ? " " : ""), VERSION_PHASE);
    } catch (prg_exception& x) {
        x.addTrace("Setup()", "AppToolkit");
        throw;
    }
}

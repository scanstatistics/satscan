//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop                                                        
//***************************************************************************
#include "IniParameterFileAccess.h"
#include "RandomNumberGenerator.h"
#include "UtilityFunctions.h"
#include "DataSource.h"
#include <boost/algorithm/string/join.hpp>

/** constructor */
IniParameterFileAccess::IniParameterFileAccess(CParameters& Parameters, BasePrint& PrintDirection)
                       :AbtractParameterFileAccess(Parameters, PrintDirection), gpSpecifications(0) {}

/** destructor */
IniParameterFileAccess::~IniParameterFileAccess() {
    try {
        delete gpSpecifications;
    } catch (...){}
}

/** Returns key string for specified parameter type. */
const char * IniParameterFileAccess::GetParameterLabel(ParameterType eParameterType) const {
    const char * sSectionName, * sKey;
    GetSpecifications().GetParameterIniInfo(eParameterType,  &sSectionName, &sKey);
    return sKey;
}

/** Return reference to IniParameterSpecification object. Throws exception if not allocated. */
const IniParameterSpecification & IniParameterFileAccess::GetSpecifications() const {
    try {
        if (!gpSpecifications)
            throw prg_error("Specifications object not allocated.", "GetSpecifications()");
    } catch (prg_exception& x) {
        x.addTrace("GetSpecifications()","IniParameterFileAccess");
        throw;
    }
    return *gpSpecifications;
}

/** Reads parameters from ini file and sets associated CParameter object. */
bool IniParameterFileAccess::Read(const char* sFilename) {
    try {
        gvParametersMissingDefaulted.clear();
        gbReadStatusError = false;
        gParameters.SetAsDefaulted();
        gParameters.SetSourceFileName(sFilename);

        IniFile SourceFile;
        SourceFile.Read(gParameters.GetSourceFileName());
        gpSpecifications = new IniParameterSpecification(SourceFile, gParameters);

        for (ParameterType eType=ANALYSISTYPE; eType <= gParameters.giNumParameters; eType = ParameterType(eType + 1))
            ReadIniParameter(SourceFile, eType);
        ReadObservableRegionSettings(SourceFile);
        ReadMultipleDataSetsSettings(SourceFile);
        ReadInputSourceSettings(SourceFile);
    } catch (prg_exception& x) {
        x.addTrace("Read(const char* sFilename)","IniParameterFileAccess");
        throw;
    }
    return !gbReadStatusError;
}

/** Reads parameters from stringstream and sets associated CParameter object. */
bool IniParameterFileAccess::Read(std::stringstream& stream) {
    try {
        gvParametersMissingDefaulted.clear();
        gbReadStatusError = false;
        gParameters.SetAsDefaulted();

        IniFile SourceFile;
        SourceFile.Read(stream);
        gpSpecifications = new IniParameterSpecification(SourceFile, gParameters);

        for (ParameterType eType = ANALYSISTYPE; eType <= gParameters.giNumParameters; eType = ParameterType(eType + 1))
            ReadIniParameter(SourceFile, eType);
        ReadObservableRegionSettings(SourceFile);
        ReadMultipleDataSetsSettings(SourceFile);
        ReadInputSourceSettings(SourceFile);
    }
    catch (prg_exception& x) {
        x.addTrace("Read(std::stringstream& stream)", "IniParameterFileAccess");
        throw;
    }
    return !gbReadStatusError;
}

/** Reads parameter from ini file and sets in CParameter object. If parameter specification not
    found or ini section/key not found in file, marks as defaulted. */
void IniParameterFileAccess::ReadIniParameter(const IniFile& SourceFile, ParameterType eParameterType) {
    long lSectionIndex, lKeyIndex=-1;
    const char * sSectionName, * sKey;
    try {
        if (GetSpecifications().GetParameterIniInfo(eParameterType, &sSectionName, &sKey)) {
            if ((lSectionIndex = SourceFile.GetSectionIndex(sSectionName)) > -1) {
                const IniSection  * pSection = SourceFile.GetSection(lSectionIndex);
                if ((lKeyIndex = pSection->FindKey(sKey)) > -1)
                    SetParameter(eParameterType, std::string(pSection->GetLine(lKeyIndex)->GetValue()), gPrintDirection);
            }
        }
        //if (lKeyIndex == -1)
        //  MarkAsMissingDefaulted(eParameterType, gPrintDirection);
    } catch (prg_exception& x) {
        x.addTrace("ReadIniParameter()","IniParameterFileAccess");
        throw;
    }
}

/** Reads parameter from ini file and returns all found key values in vector for
    those parameters that have optional additional keys, such as files with
    multiple datasets. */
std::vector<std::string>& IniParameterFileAccess::ReadIniParameter(const IniFile& SourceFile, ParameterType eParameterType, std::vector<std::string>& vParameters, size_t iSuffixIndex) const {
    long lSectionIndex, lKeyIndex;
    std::string sNextKey;
    const char * sSectionName, * sKey;

    vParameters.clear();
    if (GetSpecifications().GetMultipleParameterIniInfo(eParameterType, &sSectionName, &sKey)) {
        //read possibly other dataset case source
        if ((lSectionIndex = SourceFile.GetSectionIndex(sSectionName)) > -1) {
            const IniSection  * pSection = SourceFile.GetSection(lSectionIndex);
            printString(sNextKey, "%s%i", sKey, iSuffixIndex);
            while ((lKeyIndex = pSection->FindKey(sNextKey.c_str())) > -1) {
                vParameters.push_back(std::string(pSection->GetLine(lKeyIndex)->GetValue()));
                printString(sNextKey, "%s%i", sKey, ++iSuffixIndex);
            }
        }
    }
    return vParameters;
}

/** Reads parameter settings grouped under 'Mutliple Data Sets'. */
void IniParameterFileAccess::ReadMultipleDataSetsSettings(const IniFile& SourceFile) {
    std::vector<std::string> vFilenames;
    size_t t, iMostDataSets=1;
    try {
        ReadIniParameter(SourceFile, MULTI_DATASET_PURPOSE_TYPE);
        ReadIniParameter(SourceFile, CASEFILE, vFilenames, 2);
        for (t=0; t < vFilenames.size(); ++t)
            gParameters.SetCaseFileName(vFilenames[t].c_str(), true, t + 2);
        iMostDataSets = std::max(iMostDataSets, vFilenames.size() + 1);
        ReadIniParameter(SourceFile, CONTROLFILE, vFilenames, 2);
        iMostDataSets = std::max(iMostDataSets, vFilenames.size() + 1);
        for (t=0; t < vFilenames.size(); ++t)
            gParameters.SetControlFileName(vFilenames[t].c_str(), true, t + 2);
        ReadIniParameter(SourceFile, POPFILE, vFilenames, 2);
        iMostDataSets = std::max(iMostDataSets, vFilenames.size() + 1);
        for (t=0; t < vFilenames.size(); ++t)
            gParameters.SetPopulationFileName(vFilenames[t].c_str(), true, t + 2);
        //Synchronize collections of dataset filesnames so that we can ask for
        //any file of a particular dataset, even if blank. This keeps the same behavior
        //as when there was only one dataset.
        gParameters.setNumFileSets(iMostDataSets);
    } catch (prg_exception& x) {
        x.addTrace("ReadMultipleDataSetsSettings()","IniParameterFileAccess");
        throw;
    }
}

/** Reads parameter settings grouped under 'Regions Window'. */
void IniParameterFileAccess::ReadObservableRegionSettings(const IniFile& SourceFile) {
    std::vector<std::string>      inequalities;
    try {
        ReadIniParameter(SourceFile, OBSERVABLE_REGIONS, inequalities, 1);
        for (size_t t=0; t < inequalities.size(); ++t)
            gParameters.AddObservableRegion(inequalities[t].c_str(), t, t == 0);
    } catch (prg_exception& x) {
        x.addTrace("ReadObservableRegionSettings()","IniParameterFileAccess");
        throw;
    }
}

/* Reads optional input source settings. */
void IniParameterFileAccess::ReadInputSourceSettings(const IniFile& SourceFile) {
    const char * section, * multiple_sets_section, * key;
    std::string buffer;

    try {
        // section name for multiple data sets
        if (!GetSpecifications().GetParameterIniInfo(MULTI_DATASET_PURPOSE_TYPE, &multiple_sets_section, &key) && gParameters.getNumFileSets() > 1)
            throw prg_error("Unable to determine section for multiple data sets.", "ReadInputSourceSettings()");

        // case file
        if (GetSpecifications().GetParameterIniInfo(CASEFILE, &section, &key)) {
            CParameters::InputSource source;
            if (ReadInputSourceSection(SourceFile, section, key, source))
                gParameters.defineInputSource(CASEFILE, source);
            // case for multiple data sets
            for (unsigned int setIdx=1; setIdx < gParameters.getNumFileSets(); ++setIdx) {
                printString(buffer, "%s%u", key, (setIdx + 1));
                CParameters::InputSource sourceset;
                if (ReadInputSourceSection(SourceFile, multiple_sets_section, buffer.c_str(), sourceset))
                    gParameters.defineInputSource(CASEFILE, sourceset, setIdx + 1);
            }
        }
        // control file
        if (GetSpecifications().GetParameterIniInfo(CONTROLFILE, &section, &key)) {
            CParameters::InputSource source;
            if (ReadInputSourceSection(SourceFile, section, key, source))
                gParameters.defineInputSource(CONTROLFILE, source);
            // control for multiple data sets
            for (unsigned int setIdx=1; setIdx < gParameters.getNumFileSets(); ++setIdx) {
                printString(buffer, "%s%u", key, (setIdx + 1));
                CParameters::InputSource sourceset;
                if (ReadInputSourceSection(SourceFile, multiple_sets_section, buffer.c_str(), sourceset))
                    gParameters.defineInputSource(CONTROLFILE, sourceset, setIdx + 1);
            }
        }
        // population file
        if (GetSpecifications().GetParameterIniInfo(POPFILE, &section, &key)) {
            CParameters::InputSource source;
            if (ReadInputSourceSection(SourceFile, section, key, source))
                gParameters.defineInputSource(POPFILE, source);
            // population for multiple data sets
            for (unsigned int setIdx=1; setIdx < gParameters.getNumFileSets(); ++setIdx) {
                printString(buffer, "%s%u", key, (setIdx + 1));
                CParameters::InputSource sourceset;
                if (ReadInputSourceSection(SourceFile, multiple_sets_section, buffer.c_str(), sourceset))
                    gParameters.defineInputSource(POPFILE, sourceset, setIdx + 1);
            }
        }
        // coordinates file
        if (GetSpecifications().GetParameterIniInfo(COORDFILE, &section, &key)) {
            CParameters::InputSource source;
            if (ReadInputSourceSection(SourceFile, section, key, source))
                gParameters.defineInputSource(COORDFILE, source);
        }
        // grid file
        if (GetSpecifications().GetParameterIniInfo(GRIDFILE, &section, &key)) {
            CParameters::InputSource source;
            if (ReadInputSourceSection(SourceFile, section, key, source))
                gParameters.defineInputSource(GRIDFILE, source);
        }
        // maximum circle population file
        if (GetSpecifications().GetParameterIniInfo(MAXCIRCLEPOPFILE, &section, &key)) {
            CParameters::InputSource source;
            if (ReadInputSourceSection(SourceFile, section, key, source))
                gParameters.defineInputSource(MAXCIRCLEPOPFILE, source);
        }
        // adjustments file
        if (GetSpecifications().GetParameterIniInfo(ADJ_BY_RR_FILE, &section, &key)) {
            CParameters::InputSource source;
            if (ReadInputSourceSection(SourceFile, section, key, source))
                gParameters.defineInputSource(ADJ_BY_RR_FILE, source);
        }
        // network file
        if (GetSpecifications().GetParameterIniInfo(NETWORK_FILE, &section, &key)) {
            CParameters::InputSource source;
            if (ReadInputSourceSection(SourceFile, section, key, source))
                gParameters.defineInputSource(NETWORK_FILE, source);
        }
        // multiple locations file
        if (GetSpecifications().GetParameterIniInfo(MULTIPLE_LOCATIONS_FILE, &section, &key)) {
            CParameters::InputSource source;
            if (ReadInputSourceSection(SourceFile, section, key, source))
                gParameters.defineInputSource(MULTIPLE_LOCATIONS_FILE, source);
        }
    } catch (prg_exception& x) {
        x.addTrace("ReadInputSourceSettings()","IniParameterFileAccess");
        throw;
    }
}

/* Reads key/values in passed source section. */
bool IniParameterFileAccess::ReadInputSourceSection(const IniFile& SourceFile, const char* sectionName, const char* keyPrefix, CParameters::InputSource& source) {
    long lSectionIndex, lKeyIndex=-1;
    std::string key, buffer;
    bool anykeys=false;
    std::string type, map, delimiter, group, skip, header, llmap;

    if ((lSectionIndex = SourceFile.GetSectionIndex(sectionName)) > -1) {
        const IniSection  * pSection = SourceFile.GetSection(lSectionIndex);
        // source file type
        printString(key, "%s-%s", keyPrefix, IniParameterSpecification::SourceType);
        if ((lKeyIndex = pSection->FindKey(key.c_str())) > -1) {
            anykeys=true;
            type = pSection->GetLine(lKeyIndex)->GetValue();
        }
        // fields map
        printString(key, "%s-%s", keyPrefix, IniParameterSpecification::SourceFieldMap);
        source.clearFieldsMap();
        if ((lKeyIndex = pSection->FindKey(key.c_str())) > -1) {
            anykeys=true;
            map = pSection->GetLine(lKeyIndex)->GetValue();
        }
        // delimiter
        printString(key, "%s-%s", keyPrefix, IniParameterSpecification::SourceDelimiter);
        if ((lKeyIndex = pSection->FindKey(key.c_str())) > -1) {
            anykeys=true;
            delimiter = pSection->GetLine(lKeyIndex)->GetValue();
        } 
        // grouper
        printString(key, "%s-%s", keyPrefix, IniParameterSpecification::SourceGrouper);
        if ((lKeyIndex = pSection->FindKey(key.c_str())) > -1) {
            anykeys=true;
            group = pSection->GetLine(lKeyIndex)->GetValue();
        } 
        // skip
        printString(key, "%s-%s", keyPrefix, IniParameterSpecification::SourceSkip);
        if ((lKeyIndex = pSection->FindKey(key.c_str())) > -1) {
            anykeys=true;
            skip = pSection->GetLine(lKeyIndex)->GetValue();
         }
        // first row header
        printString(key, "%s-%s", keyPrefix, IniParameterSpecification::SourceFirstRowHeader);
        if ((lKeyIndex = pSection->FindKey(key.c_str())) > -1) {
            anykeys=true;
            header = pSection->GetLine(lKeyIndex)->GetValue();
        }
        // line list fields map
        source.clearLinelistFieldsMap();
        printString(key, "%s-%s", keyPrefix, IniParameterSpecification::SourceLinelistFieldMap);
        if ((lKeyIndex = pSection->FindKey(key.c_str())) > -1) {
            anykeys = true;
            llmap = pSection->GetLine(lKeyIndex)->GetValue();
        }
        if (anykeys) {
            setInputSource(source, gPrintDirection, trimString(type), trimString(map), trimString(delimiter), trimString(group), trimString(skip), trimString(header), trimString(llmap));
        }
    }
    return anykeys;
}

void IniParameterFileAccess::writeSections(IniFile& ini, const IniParameterSpecification& specification) {
    const IniParameterSpecification * hold = gpSpecifications;
    try {
        gpSpecifications = &specification;

        //write settings as provided in main graphical interface
        WriteInputSettings(ini);
        WriteAnalysisSettings(ini);
        WriteOutputSettings(ini);

        //write settings as provided in advanced features of graphical interface
        WriteObservableRegionSettings(ini);
        WriteMultipleDataSetsSettings(ini);
        WriteDataCheckingSettings(ini);
        WriteLocationNetworkSettings(ini);
        WriteSpatialNeighborsSettings(ini);
        WriteSpatialWindowSettings(ini);
        WriteTemporalWindowSettings(ini);
        WriteClusterRestrictionsSettings(ini);
        WriteSpaceAndTimeAdjustmentSettings(ini);
        WriteInferenceSettings(ini);
        WriteDrilldownSettings(ini);
        WriteMiscellaneousAnalysisSettings(ini);
        WritePowerEvaluationsSettings(ini);
        WriteSpatialOutputSettings(ini);
        WriteTemporalGraphSettings(ini);
        WriteOtherOutputSettings(ini);
        WriteLineListSettings(ini);
        WriteEmailAlertSettings(ini);

        //write settings as provided only through user mofication of parameter file and batch executable
        WriteEllipticScanSettings(ini);
        WritePowerSimulationsSettings(ini);
        WriteRunOptionSettings(ini);
        WriteSystemSettings(ini);
        gpSpecifications = hold;
    } catch (...) {
        gpSpecifications = hold;
        throw;
    }
}

/** Writes parameters of associated CParameters object to ini file, of most recent
    format specification. */
void IniParameterFileAccess::Write(const char* sFilename) {
    try {
        IniFile WriteFile;
        gParameters.SetSourceFileName(sFilename);
        gpSpecifications = new IniParameterSpecification();
        writeSections(WriteFile, *gpSpecifications);
        WriteFile.Write(gParameters.GetSourceFileName());
    } catch (prg_exception& x) {
        x.addTrace("Write()", "IniParameterFileAccess");
        throw;
    }
}

void IniParameterFileAccess::Write(std::stringstream& stream) {
    try {
        IniFile WriteFile;
        gpSpecifications = new IniParameterSpecification();
        writeSections(WriteFile, *gpSpecifications);
        WriteFile.Write(stream, false, false);
    }
    catch (prg_exception& x) {
        x.addTrace("Write()", "IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under 'Other Output'. */
void IniParameterFileAccess::WriteOtherOutputSettings(IniFile& WriteFile) {
    std::string s;
    try {
        WriteIniParameter(WriteFile, CLUSTER_SIGNIFICANCE_BY_RI, GetParameterString(CLUSTER_SIGNIFICANCE_BY_RI, s).c_str(), GetParameterComment(CLUSTER_SIGNIFICANCE_BY_RI));
        WriteIniParameter(WriteFile, CLUSTER_SIGNIFICANCE_RI_VALUE, GetParameterString(CLUSTER_SIGNIFICANCE_RI_VALUE, s).c_str(), GetParameterComment(CLUSTER_SIGNIFICANCE_RI_VALUE));
        WriteIniParameter(WriteFile, CLUSTER_SIGNIFICANCE_RI_TYPE, GetParameterString(CLUSTER_SIGNIFICANCE_RI_TYPE, s).c_str(), GetParameterComment(CLUSTER_SIGNIFICANCE_RI_TYPE));
        WriteIniParameter(WriteFile, CLUSTER_SIGNIFICANCE_BY_PVAL, GetParameterString(CLUSTER_SIGNIFICANCE_BY_PVAL, s).c_str(), GetParameterComment(CLUSTER_SIGNIFICANCE_BY_PVAL));
        WriteIniParameter(WriteFile, CLUSTER_SIGNIFICANCE_PVAL_VALUE, GetParameterString(CLUSTER_SIGNIFICANCE_PVAL_VALUE, s).c_str(), GetParameterComment(CLUSTER_SIGNIFICANCE_PVAL_VALUE));
        WriteIniParameter(WriteFile, REPORT_CRITICAL_VALUES, GetParameterString(REPORT_CRITICAL_VALUES, s).c_str(), GetParameterComment(REPORT_CRITICAL_VALUES));
        WriteIniParameter(WriteFile, REPORT_RANK, GetParameterString(REPORT_RANK, s).c_str(), GetParameterComment(REPORT_RANK));
        WriteIniParameter(WriteFile, PRINT_ASCII_HEADERS, GetParameterString(PRINT_ASCII_HEADERS, s).c_str(), GetParameterComment(PRINT_ASCII_HEADERS));
        WriteIniParameter(WriteFile, USER_DEFINED_TITLE, GetParameterString(USER_DEFINED_TITLE, s).c_str(), GetParameterComment(USER_DEFINED_TITLE));
    } catch (prg_exception& x) {
        x.addTrace("WriteOtherOutputSettings()","IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under 'Border Analysis'. */
void IniParameterFileAccess::WriteMiscellaneousAnalysisSettings(IniFile& WriteFile) {
    std::string s;
    try {
        WriteIniParameter(WriteFile, CALCULATE_OLIVEIRA, GetParameterString(CALCULATE_OLIVEIRA, s).c_str(), GetParameterComment(CALCULATE_OLIVEIRA));
        WriteIniParameter(WriteFile, NUM_OLIVEIRA_SETS, GetParameterString(NUM_OLIVEIRA_SETS, s).c_str(), GetParameterComment(NUM_OLIVEIRA_SETS));
        WriteIniParameter(WriteFile, OLIVEIRA_CUTOFF, GetParameterString(OLIVEIRA_CUTOFF, s).c_str(), GetParameterComment(OLIVEIRA_CUTOFF));
        WriteIniParameter(WriteFile, PROSPECTIVE_FREQ_TYPE, GetParameterString(PROSPECTIVE_FREQ_TYPE, s).c_str(), GetParameterComment(PROSPECTIVE_FREQ_TYPE));
        WriteIniParameter(WriteFile, PROSPECTIVE_FREQ, GetParameterString(PROSPECTIVE_FREQ, s).c_str(), GetParameterComment(PROSPECTIVE_FREQ));
    } catch (prg_exception& x) {
        x.addTrace("WriteMiscellaneousAnalysisSettings()","IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under 'Line List'. */
void IniParameterFileAccess::WriteLineListSettings(IniFile& WriteFile) {
    std::string s;
    try {
        WriteIniParameter(WriteFile, LL_INDIVIDUALS_CACHE_FILE, GetParameterString(LL_INDIVIDUALS_CACHE_FILE, s).c_str(), GetParameterComment(LL_INDIVIDUALS_CACHE_FILE));
    } catch (prg_exception& x) {
        x.addTrace("WriteLineListSettings()", "IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under 'Alerts'. */
void IniParameterFileAccess::WriteEmailAlertSettings(IniFile& WriteFile) {
    std::string s;
    try {
        WriteIniParameter(WriteFile, EMAIL_RESULTS_SUMMARY, GetParameterString(EMAIL_RESULTS_SUMMARY, s).c_str(), GetParameterComment(EMAIL_RESULTS_SUMMARY));
        WriteIniParameter(WriteFile, EMAIL_ALWAYS_RCPTS, GetParameterString(EMAIL_ALWAYS_RCPTS, s).c_str(), GetParameterComment(EMAIL_ALWAYS_RCPTS));
        WriteIniParameter(WriteFile, EMAIL_SIGNIFICANT_RCPTS, GetParameterString(EMAIL_SIGNIFICANT_RCPTS, s).c_str(), GetParameterComment(EMAIL_SIGNIFICANT_RCPTS));
        WriteIniParameter(WriteFile, EMAIL_SUBJECT_NO_SIGNIFICANT, GetParameterString(EMAIL_SUBJECT_NO_SIGNIFICANT, s).c_str(), GetParameterComment(EMAIL_SUBJECT_NO_SIGNIFICANT));
        WriteIniParameter(WriteFile, EMAIL_BODY_NO_SIGNIFICANT, GetParameterString(EMAIL_BODY_NO_SIGNIFICANT, s).c_str(), GetParameterComment(EMAIL_BODY_NO_SIGNIFICANT));
        WriteIniParameter(WriteFile, EMAIL_SUBJECT_SIGNIFICANT, GetParameterString(EMAIL_SUBJECT_SIGNIFICANT, s).c_str(), GetParameterComment(EMAIL_SUBJECT_SIGNIFICANT));
        WriteIniParameter(WriteFile, EMAIL_BODY_SIGNIFICANT, GetParameterString(EMAIL_BODY_SIGNIFICANT, s).c_str(), GetParameterComment(EMAIL_BODY_SIGNIFICANT));
        WriteIniParameter(WriteFile, EMAIL_ATTACH_RESULTS, GetParameterString(EMAIL_ATTACH_RESULTS, s).c_str(), GetParameterComment(EMAIL_ATTACH_RESULTS));
    } catch (prg_exception& x) {
        x.addTrace("WriteEmailAlertSettings()", "IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under 'Analysis'. */
void IniParameterFileAccess::WriteAnalysisSettings(IniFile& WriteFile) {
    std::string s;
    try {
        WriteIniParameter(WriteFile, ANALYSISTYPE, GetParameterString(ANALYSISTYPE, s).c_str(), GetParameterComment(ANALYSISTYPE));
        WriteIniParameter(WriteFile, MODEL, GetParameterString(MODEL, s).c_str(), GetParameterComment(MODEL));
        WriteIniParameter(WriteFile, SCANAREAS, GetParameterString(SCANAREAS, s).c_str(), GetParameterComment(SCANAREAS));
        WriteIniParameter(WriteFile, TIME_AGGREGATION_UNITS, GetParameterString(TIME_AGGREGATION_UNITS, s).c_str(), GetParameterComment(TIME_AGGREGATION_UNITS));
        WriteIniParameter(WriteFile, TIME_AGGREGATION, GetParameterString(TIME_AGGREGATION, s).c_str(), GetParameterComment(TIME_AGGREGATION));
    } catch (prg_exception& x) {
        x.addTrace("WriteAnalysisSettings()","IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under 'Spatial Output'. */
void IniParameterFileAccess::WriteSpatialOutputSettings(IniFile& WriteFile) {
    std::string s;
    try {
        WriteIniParameter(WriteFile, LAUNCH_MAP_VIEWER, GetParameterString(LAUNCH_MAP_VIEWER, s).c_str(), GetParameterComment(LAUNCH_MAP_VIEWER));
        WriteIniParameter(WriteFile, COMPRESS_KML_OUTPUT, GetParameterString(COMPRESS_KML_OUTPUT, s).c_str(), GetParameterComment(COMPRESS_KML_OUTPUT));
        WriteIniParameter(WriteFile, INCLUDE_LOCATIONS_KML, GetParameterString(INCLUDE_LOCATIONS_KML, s).c_str(), GetParameterComment(INCLUDE_LOCATIONS_KML));
        WriteIniParameter(WriteFile, LOCATIONS_THRESHOLD_KML, GetParameterString(LOCATIONS_THRESHOLD_KML, s).c_str(), GetParameterComment(LOCATIONS_THRESHOLD_KML));
        WriteIniParameter(WriteFile, REPORT_HIERARCHICAL_CLUSTERS, GetParameterString(REPORT_HIERARCHICAL_CLUSTERS, s).c_str(), GetParameterComment(REPORT_HIERARCHICAL_CLUSTERS));
        WriteIniParameter(WriteFile, CRITERIA_SECOND_CLUSTERS, GetParameterString(CRITERIA_SECOND_CLUSTERS, s).c_str(), GetParameterComment(CRITERIA_SECOND_CLUSTERS));
        WriteIniParameter(WriteFile, REPORT_GINI_CLUSTERS, GetParameterString(REPORT_GINI_CLUSTERS, s).c_str(), GetParameterComment(REPORT_GINI_CLUSTERS));
        WriteIniParameter(WriteFile, GINI_INDEX_REPORT_TYPE, GetParameterString(GINI_INDEX_REPORT_TYPE, s).c_str(), GetParameterComment(GINI_INDEX_REPORT_TYPE));
        WriteIniParameter(WriteFile, SPATIAL_MAXIMA, GetParameterString(SPATIAL_MAXIMA, s).c_str(), GetParameterComment(SPATIAL_MAXIMA));
        WriteIniParameter(WriteFile, GINI_INDEX_PVALUE_CUTOFF, GetParameterString(GINI_INDEX_PVALUE_CUTOFF, s).c_str(), GetParameterComment(GINI_INDEX_PVALUE_CUTOFF));
        WriteIniParameter(WriteFile, REPORT_GINI_COEFFICENTS, GetParameterString(REPORT_GINI_COEFFICENTS, s).c_str(), GetParameterComment(REPORT_GINI_COEFFICENTS));
        WriteIniParameter(WriteFile, USE_REPORTED_GEOSIZE, GetParameterString(USE_REPORTED_GEOSIZE, s).c_str(), GetParameterComment(USE_REPORTED_GEOSIZE));
        WriteIniParameter(WriteFile, MAXGEOPOPATRISK_REPORTED, GetParameterString(MAXGEOPOPATRISK_REPORTED, s).c_str(), GetParameterComment(MAXGEOPOPATRISK_REPORTED));
        WriteIniParameter(WriteFile, USE_MAXGEOPOPFILE_REPORTED, GetParameterString(USE_MAXGEOPOPFILE_REPORTED, s).c_str(), GetParameterComment(USE_MAXGEOPOPFILE_REPORTED));
        WriteIniParameter(WriteFile, MAXGEOPOPFILE_REPORTED, GetParameterString(MAXGEOPOPFILE_REPORTED, s).c_str(), GetParameterComment(MAXGEOPOPFILE_REPORTED));
        WriteIniParameter(WriteFile, USE_MAXGEODISTANCE_REPORTED, GetParameterString(USE_MAXGEODISTANCE_REPORTED, s).c_str(), GetParameterComment(USE_MAXGEODISTANCE_REPORTED));
        WriteIniParameter(WriteFile, MAXGEODISTANCE_REPORTED, GetParameterString(MAXGEODISTANCE_REPORTED, s).c_str(), GetParameterComment(MAXGEODISTANCE_REPORTED));
        WriteIniParameter(WriteFile, OUTPUT_GOOGLE_MAP, GetParameterString(OUTPUT_GOOGLE_MAP, s).c_str(), GetParameterComment(OUTPUT_GOOGLE_MAP));
    } catch (prg_exception& x) {
        x.addTrace("WriteSpatialOutputSettings()","IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under 'Data Checking'. */
void IniParameterFileAccess::WriteDataCheckingSettings(IniFile& WriteFile) {
    std::string  s;
    try {
        WriteIniParameter(WriteFile, STUDYPERIOD_DATACHECK, GetParameterString(STUDYPERIOD_DATACHECK, s).c_str(), GetParameterComment(STUDYPERIOD_DATACHECK));
        WriteIniParameter(WriteFile, COORDINATES_DATACHECK, GetParameterString(COORDINATES_DATACHECK, s).c_str(), GetParameterComment(COORDINATES_DATACHECK));
    } catch (prg_exception& x) {
        x.addTrace("WriteDataCheckingSettings()","IniParameterFileAccess");
        throw;
    }
}

void IniParameterFileAccess::WriteDrilldownSettings(IniFile& WriteFile) {
	std::string  s;
	try {
		WriteIniParameter(WriteFile, PERFORM_STANDARD_DRILLDOWN, GetParameterString(PERFORM_STANDARD_DRILLDOWN, s).c_str(), GetParameterComment(PERFORM_STANDARD_DRILLDOWN));
		WriteIniParameter(WriteFile, PERFORM_BERNOULLI_DRILLDOWN, GetParameterString(PERFORM_BERNOULLI_DRILLDOWN, s).c_str(), GetParameterComment(PERFORM_BERNOULLI_DRILLDOWN));
		WriteIniParameter(WriteFile, DRILLDOWN_MIN_LOCATIONS, GetParameterString(DRILLDOWN_MIN_LOCATIONS, s).c_str(), GetParameterComment(DRILLDOWN_MIN_LOCATIONS));
		WriteIniParameter(WriteFile, DRILLDOWN_MIN_CASES, GetParameterString(DRILLDOWN_MIN_CASES, s).c_str(), GetParameterComment(DRILLDOWN_MIN_CASES));
		WriteIniParameter(WriteFile, DRILLDOWN_PVLAUE_CUTOFF, GetParameterString(DRILLDOWN_PVLAUE_CUTOFF, s).c_str(), GetParameterComment(DRILLDOWN_PVLAUE_CUTOFF));
		WriteIniParameter(WriteFile, DRILLDOWN_ADJ_WEEKLY_TRENDS, GetParameterString(DRILLDOWN_ADJ_WEEKLY_TRENDS, s).c_str(), GetParameterComment(DRILLDOWN_ADJ_WEEKLY_TRENDS));
	} catch (prg_exception& x) {
		x.addTrace("WriteDrilldownSettings()", "IniParameterFileAccess");
		throw;
	}
}

/** Writes parameter settings grouped under 'Elliptic Scan'. */
void IniParameterFileAccess::WriteEllipticScanSettings(IniFile& WriteFile) {
    std::string s;
    try {
        WriteIniParameter(WriteFile, ESHAPES, GetParameterString(ESHAPES, s).c_str(), GetParameterComment(ESHAPES));
        WriteIniParameter(WriteFile, ENUMBERS, GetParameterString(ENUMBERS, s).c_str(), GetParameterComment(ENUMBERS));
    } catch (prg_exception& x) {
        x.addTrace("WriteEllipticScanSettings()","IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under 'Inference'. */
void IniParameterFileAccess::WriteInferenceSettings(IniFile& WriteFile) {
    std::string  s;
    try {
        WriteIniParameter(WriteFile, PVALUE_REPORT_TYPE, GetParameterString(PVALUE_REPORT_TYPE, s).c_str(), GetParameterComment(PVALUE_REPORT_TYPE));
        WriteIniParameter(WriteFile, EARLY_TERM_THRESHOLD, GetParameterString(EARLY_TERM_THRESHOLD, s).c_str(), GetParameterComment(EARLY_TERM_THRESHOLD));
        WriteIniParameter(WriteFile, REPORT_GUMBEL, GetParameterString(REPORT_GUMBEL, s).c_str(), GetParameterComment(REPORT_GUMBEL));
        WriteIniParameter(WriteFile, REPLICAS, GetParameterString(REPLICAS, s).c_str(), GetParameterComment(REPLICAS));
        WriteIniParameter(WriteFile, ADJ_FOR_EALIER_ANALYSES, GetParameterString(ADJ_FOR_EALIER_ANALYSES, s).c_str(), GetParameterComment(ADJ_FOR_EALIER_ANALYSES));
        WriteIniParameter(WriteFile, START_PROSP_SURV, GetParameterString(START_PROSP_SURV, s).c_str(), GetParameterComment(START_PROSP_SURV));
        WriteIniParameter(WriteFile, ITERATIVE, GetParameterString(ITERATIVE, s).c_str(), GetParameterComment(ITERATIVE));
        WriteIniParameter(WriteFile, ITERATIVE_NUM, GetParameterString(ITERATIVE_NUM, s).c_str(), GetParameterComment(ITERATIVE_NUM));
        WriteIniParameter(WriteFile, ITERATIVE_PVAL, GetParameterString(ITERATIVE_PVAL, s).c_str(), GetParameterComment(ITERATIVE_PVAL));
    } catch (prg_exception& x) {
        x.addTrace("WriteInferenceSettings()","IniParameterFileAccess");
        throw;
    }
}

void IniParameterFileAccess::WriteClusterRestrictionsSettings(IniFile& WriteFile) {
    std::string  s;
    try {
        WriteIniParameter(WriteFile, RISK_LIMIT_HIGH_CLUSTERS, GetParameterString(RISK_LIMIT_HIGH_CLUSTERS, s).c_str(), GetParameterComment(RISK_LIMIT_HIGH_CLUSTERS));
        WriteIniParameter(WriteFile, RISK_THESHOLD_HIGH_CLUSTERS, GetParameterString(RISK_THESHOLD_HIGH_CLUSTERS, s).c_str(), GetParameterComment(RISK_THESHOLD_HIGH_CLUSTERS));
        WriteIniParameter(WriteFile, RISK_LIMIT_LOW_CLUSTERS, GetParameterString(RISK_LIMIT_LOW_CLUSTERS, s).c_str(), GetParameterComment(RISK_LIMIT_LOW_CLUSTERS));
        WriteIniParameter(WriteFile, RISK_THESHOLD_LOW_CLUSTERS, GetParameterString(RISK_THESHOLD_LOW_CLUSTERS, s).c_str(), GetParameterComment(RISK_THESHOLD_LOW_CLUSTERS));
        WriteIniParameter(WriteFile, MIN_CASES_LOWRATE_CLUSTERS, GetParameterString(MIN_CASES_LOWRATE_CLUSTERS, s).c_str(), GetParameterComment(MIN_CASES_LOWRATE_CLUSTERS));
        WriteIniParameter(WriteFile, MIN_CASES_HIGHRATE_CLUSTERS, GetParameterString(MIN_CASES_HIGHRATE_CLUSTERS, s).c_str(), GetParameterComment(MIN_CASES_HIGHRATE_CLUSTERS));
    }
    catch (prg_exception& x) {
        x.addTrace("WriteClusterRestrictionsSettings()", "IniParameterFileAccess");
        throw;
    }
}

void IniParameterFileAccess::WriteInputSource(IniFile& WriteFile, ParameterType eParameterType, const CParameters::InputSource * source) {
    const char  * sSectionName, * sKey;
    std::string buffer, key;

    try {
        if (source) {
            if (GetSpecifications().GetParameterIniInfo(eParameterType, &sSectionName, &sKey)) {
                IniSection * pSection = WriteFile.GetSection(sSectionName);
                WriteInputSource(WriteFile, *(WriteFile.GetSection(sSectionName)), sKey, source);
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("WriteInputSource()","IniParameterFileAccess");
        throw;
    }
}

/** Write InputSource object to ini file. */
void IniParameterFileAccess::WriteInputSource(IniFile& WriteFile, IniSection& section, const std::string& basekey, const CParameters::InputSource * source) {
    std::string buffer, key;

    try {
        if (source) {
            section.AddComment("source type (CSV=0, DBASE=1, SHAPE=2)");
            printString(key, "%s-%s", basekey.c_str(), IniParameterSpecification::SourceType);
            section.AddLine(key.c_str(), AsString(buffer, source->getSourceType()).c_str());

            if (source->getFieldsMap().size()) {
                printString(buffer, "source field map (comma separated list of integers, %s, %s, %s, %s)",
                            IniParameterSpecification::SourceFieldMapOneCount,
                            IniParameterSpecification::SourceFieldMapGeneratedId,
                            IniParameterSpecification::SourceFieldMapShapeX,
                            IniParameterSpecification::SourceFieldMapShapeY);
                section.AddComment(buffer.c_str());
                std::stringstream s;
                for (FieldMapContainer_t::const_iterator itr=source->getFieldsMap().begin(); itr != source->getFieldsMap().end(); ++itr) {
                    if (itr->type() == typeid(long)) {
                        s << boost::any_cast<long>(*itr);
                    } else if (itr->type() == typeid(DataSource::FieldType)) {
                        switch (boost::any_cast<DataSource::FieldType>(*itr)) {
                            case DataSource::ONECOUNT : s << IniParameterSpecification::SourceFieldMapOneCount; break;
                            case DataSource::GENERATEDID : s << IniParameterSpecification::SourceFieldMapGeneratedId; break;
                            case DataSource::DEFAULT_DATE : s << IniParameterSpecification::SourceFieldMapUnspecifiedPopulationDate; break;
                            case DataSource::BLANK : break;
                            default : throw prg_error("Unknown type '%s'.", "WriteInputSource()", boost::any_cast<DataSource::FieldType>(*itr));
                        }
                    } else if (itr->type() == typeid(DataSource::ShapeFieldType)) {
                        switch (boost::any_cast<DataSource::ShapeFieldType>(*itr)) {
                            case DataSource::POINTX   : s << IniParameterSpecification::SourceFieldMapShapeX; break;
                            case DataSource::POINTY   : s << IniParameterSpecification::SourceFieldMapShapeY; break;
                            default : throw prg_error("Unknown type '%s'.", "WriteInputSource()", boost::any_cast<DataSource::ShapeFieldType>(*itr));
                        }
                    } else {
                        throw prg_error("Unknown type '%s'.", "WriteInputSource()", itr->type().name());
                    }
                    if ((itr+1) != source->getFieldsMap().end()) {s << ",";}
                }
                    printString(key, "%s-%s", basekey.c_str(), IniParameterSpecification::SourceFieldMap);
                    section.AddLine(key.c_str(), s.str().c_str());
            }

            if (source->getSourceType() == CSV) {
                section.AddComment("csv source delimiter (leave empty for space or tab delimiter)");
                printString(key, "%s-%s", basekey.c_str(), IniParameterSpecification::SourceDelimiter);
                section.AddLine(key.c_str(), source->getDelimiter().c_str());

                section.AddComment("csv source group character");
                printString(key, "%s-%s", basekey.c_str(), IniParameterSpecification::SourceGrouper);
                section.AddLine(key.c_str(), source->getGroup().c_str());

                section.AddComment("csv source skip initial lines (i.e. meta data)");
                printString(key, "%s-%s", basekey.c_str(), IniParameterSpecification::SourceSkip);
                section.AddLine(key.c_str(), AsString(buffer, source->getSkip()).c_str());

                section.AddComment("csv source first row column header");
                printString(key, "%s-%s", basekey.c_str(), IniParameterSpecification::SourceFirstRowHeader);
                section.AddLine(key.c_str(), AsString(buffer, source->getFirstRowHeader()).c_str());
            }

			if (source->getLinelistFieldsMap().size()) {
				printString(buffer, "source line list field map (comma separated list of <column idx>:<variable type>:<variable name>)");
				section.AddComment(buffer.c_str());
				std::stringstream s;
				for (auto itr = source->getLinelistFieldsMap().begin(); itr != source->getLinelistFieldsMap().end(); ++itr) {
					if (itr != source->getLinelistFieldsMap().begin()) { s << ","; }
					s << itr->get<0>() << ":" << itr->get<1>() << ":\"" << itr->get<2>() << "\"";
				}
				printString(key, "%s-%s", basekey.c_str(), IniParameterSpecification::SourceLinelistFieldMap);
				section.AddLine(key.c_str(), s.str().c_str());
			}
        }
    } catch (prg_exception& x) {
        x.addTrace("WriteInputSource()","IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under 'Locations Network'. */
void IniParameterFileAccess::WriteLocationNetworkSettings(IniFile& WriteFile) {
    std::string  s;
    try {
        GetParameterString(NETWORK_FILE, s);
        WriteIniParameter(WriteFile, NETWORK_FILE, s.c_str(), GetParameterComment(NETWORK_FILE));
        if (s.size()) WriteInputSource(WriteFile, NETWORK_FILE, gParameters.getInputSource(NETWORK_FILE));
        WriteIniParameter(WriteFile, USE_NETWORK_FILE, GetParameterString(USE_NETWORK_FILE, s).c_str(), GetParameterComment(USE_NETWORK_FILE));
    }
    catch (prg_exception& x) {
        x.addTrace("WriteLocationNetworkSettings()", "IniParameterFileAccess");
        throw;
    }
}

/** Writes specified comment and value to file for parameter type. */
void IniParameterFileAccess::WriteIniParameter(IniFile& WriteFile, ParameterType eParameterType, const char* sValue, const char* sComment) {
    const char  * sSectionName, * sKey;
    try {
        if (GetSpecifications().GetParameterIniInfo(eParameterType, &sSectionName, &sKey)) {
            IniSection *  pSection = WriteFile.GetSection(sSectionName);
            if (sComment) pSection->AddComment(sComment);
            pSection->AddLine(sKey, sValue);
        } else throw prg_error("Unknown parameter type '%d'.", "WriteIniParameters()", eParameterType);
    } catch (prg_exception& x) {
        x.addTrace("WriteIniParameter(IniFile,ParameterType,const char*,const char*)","IniParameterFileAccess");
        throw;
    }
}

/** Writes specified comment and value to file as specified section/key names. */
void IniParameterFileAccess::WriteIniParameterAsKey(IniFile& WriteFile, const char* sSectionName, const char * sKey, const char* sValue, const char* sComment) {
    try {
        IniSection *  pSection = WriteFile.GetSection(sSectionName);
        if (sComment) pSection->AddComment(sComment);
        pSection->AddLine(sKey, sValue);
    } catch (prg_exception& x) {
        x.addTrace("WriteIniParameterAsKey()","IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under 'Input'. */
void IniParameterFileAccess::WriteInputSettings(IniFile& WriteFile) {
    std::string  s;
    const char  * sSectionName, * sKey;

    try {
        GetParameterString(CASEFILE, s);
        WriteIniParameter(WriteFile, CASEFILE, s.c_str(), GetParameterComment(CASEFILE));
        if (s.size()) WriteInputSource(WriteFile, CASEFILE, gParameters.getInputSource(CASEFILE));

        GetParameterString(CONTROLFILE, s);
        WriteIniParameter(WriteFile, CONTROLFILE, s.c_str(), GetParameterComment(CONTROLFILE));
        if (s.size()) WriteInputSource(WriteFile, CONTROLFILE, gParameters.getInputSource(CONTROLFILE));

        WriteIniParameter(WriteFile, PRECISION, GetParameterString(PRECISION, s).c_str(), GetParameterComment(PRECISION));
        WriteIniParameter(WriteFile, STARTDATE, GetParameterString(STARTDATE, s).c_str(), GetParameterComment(STARTDATE));
        WriteIniParameter(WriteFile, ENDDATE, GetParameterString(ENDDATE, s).c_str(), GetParameterComment(ENDDATE));

        GetParameterString(POPFILE, s);
        WriteIniParameter(WriteFile, POPFILE, s.c_str(), GetParameterComment(POPFILE));
        if (s.size()) WriteInputSource(WriteFile, POPFILE, gParameters.getInputSource(POPFILE));

        GetParameterString(COORDFILE, s);
        WriteIniParameter(WriteFile, COORDFILE, s.c_str(), GetParameterComment(COORDFILE));
        if (s.size()) WriteInputSource(WriteFile, COORDFILE, gParameters.getInputSource(COORDFILE));

        WriteIniParameter(WriteFile, SPECIALGRID, GetParameterString(SPECIALGRID, s).c_str(), GetParameterComment(SPECIALGRID));

        GetParameterString(GRIDFILE, s);
        WriteIniParameter(WriteFile, GRIDFILE, s.c_str(), GetParameterComment(GRIDFILE));
        if (s.size()) WriteInputSource(WriteFile, GRIDFILE, gParameters.getInputSource(GRIDFILE));

        WriteIniParameter(WriteFile, COORDTYPE, GetParameterString(COORDTYPE, s).c_str(), GetParameterComment(COORDTYPE));
    } catch (prg_exception& x) {
        x.addTrace("WriteInputSettings()","IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under 'Multiple Data Sets'. */
void IniParameterFileAccess::WriteMultipleDataSetsSettings(IniFile& WriteFile) {
    std::string   s, sComment;
    const char  * sSectionName, * sBaseKey;
    try {
        WriteIniParameter(WriteFile, MULTI_DATASET_PURPOSE_TYPE, AsString(s, gParameters.GetMultipleDataSetPurposeType()).c_str(),
                          " multiple data sets purpose type (0=Multivariate, 1=Adjustment)");

        for (size_t t=1; t < gParameters.getNumFileSets(); ++t) {
            if (GetSpecifications().GetMultipleParameterIniInfo(CASEFILE, &sSectionName, &sBaseKey)) {
                printString(s, "%s%i", sBaseKey, t + 1);
                printString(sComment, " case data filename (additional data set %i)", t + 1);
                WriteIniParameterAsKey(WriteFile, sSectionName, s.c_str(), gParameters.GetCaseFileName(t + 1).c_str(), sComment.c_str());
                if (gParameters.GetCaseFileName(t + 1).size()) WriteInputSource(WriteFile, *(WriteFile.GetSection(sSectionName)), s, gParameters.getInputSource(CASEFILE, t+1));
            }
            if (GetSpecifications().GetMultipleParameterIniInfo(CONTROLFILE, &sSectionName, &sBaseKey)) {
                printString(s, "%s%i", sBaseKey, t + 1);
                printString(sComment, " control data filename (additional data set %i)", t + 1);
                WriteIniParameterAsKey(WriteFile, sSectionName, s.c_str(), gParameters.GetControlFileName(t + 1).c_str(), sComment.c_str());
                if (gParameters.GetControlFileName(t + 1).size()) WriteInputSource(WriteFile, *(WriteFile.GetSection(sSectionName)), s, gParameters.getInputSource(CONTROLFILE, t+1));
            }
            if (GetSpecifications().GetMultipleParameterIniInfo(POPFILE, &sSectionName, &sBaseKey)) {
                printString(s, "%s%i", sBaseKey, t + 1);
                printString(sComment, " population data filename (additional data set %i)", t + 1);
                WriteIniParameterAsKey(WriteFile, sSectionName, s.c_str(), gParameters.GetPopulationFileName(t + 1).c_str(), sComment.c_str());
                if (gParameters.GetPopulationFileName(t + 1).size()) WriteInputSource(WriteFile, *(WriteFile.GetSection(sSectionName)), s, gParameters.getInputSource(POPFILE, t+1));
            }
        }
    } catch (prg_exception& x) {
        x.addTrace("WriteMultipleDataSetsSettings()","IniParameterFileAccess");
        throw;
    }
}

/**  Writes parameter settings grouped under 'Region Window'. */
void IniParameterFileAccess::WriteObservableRegionSettings(IniFile& WriteFile) {
  std::string   s, sComment;
  const char  * sSectionName, * sBaseKey;
  try {
    if (GetSpecifications().GetMultipleParameterIniInfo(OBSERVABLE_REGIONS, &sSectionName, &sBaseKey)) {
        for (size_t t=0; t < gParameters.getObservableRegions().size(); ++t) {
         printString(s, "%s%i", sBaseKey, t + 1);
         printString(sComment, " inequalities list -- defines bound region");
         WriteIniParameterAsKey(WriteFile, sSectionName, s.c_str(), gParameters.getObservableRegions()[t].c_str(), sComment.c_str());
      }
    }
  } catch (prg_exception& x) {
    x.addTrace("WriteObservableRegionSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Writes parameter settings grouped under 'Output'. */
void IniParameterFileAccess::WriteOutputSettings(IniFile& WriteFile) {
  std::string s;
  try {
    WriteIniParameter(WriteFile, OUTPUTFILE, GetParameterString(OUTPUTFILE, s).c_str(), GetParameterComment(OUTPUTFILE));
    WriteIniParameter(WriteFile, OUTPUT_KML, GetParameterString(OUTPUT_KML, s).c_str(), GetParameterComment(OUTPUT_KML));
    WriteIniParameter(WriteFile, OUTPUT_SHAPEFILES, GetParameterString(OUTPUT_SHAPEFILES, s).c_str(), GetParameterComment(OUTPUT_SHAPEFILES));
    WriteIniParameter(WriteFile, OUTPUT_CARTESIAN_GRAPH, GetParameterString(OUTPUT_CARTESIAN_GRAPH, s).c_str(), GetParameterComment(OUTPUT_CARTESIAN_GRAPH));
    WriteIniParameter(WriteFile, OUTPUT_MLC_ASCII, GetParameterString(OUTPUT_MLC_ASCII, s).c_str(), GetParameterComment(OUTPUT_MLC_ASCII));
    WriteIniParameter(WriteFile, OUTPUT_MLC_DBASE, GetParameterString(OUTPUT_MLC_DBASE, s).c_str(), GetParameterComment(OUTPUT_MLC_DBASE));
    WriteIniParameter(WriteFile, OUTPUT_MLC_CASE_ASCII, GetParameterString(OUTPUT_MLC_CASE_ASCII, s).c_str(), GetParameterComment(OUTPUT_MLC_CASE_ASCII));
    WriteIniParameter(WriteFile, OUTPUT_MLC_CASE_DBASE, GetParameterString(OUTPUT_MLC_CASE_DBASE, s).c_str(), GetParameterComment(OUTPUT_MLC_CASE_DBASE));
    WriteIniParameter(WriteFile, OUTPUT_AREAS_ASCII, GetParameterString(OUTPUT_AREAS_ASCII, s).c_str(), GetParameterComment(OUTPUT_AREAS_ASCII));
    WriteIniParameter(WriteFile, OUTPUT_AREAS_DBASE, GetParameterString(OUTPUT_AREAS_DBASE, s).c_str(), GetParameterComment(OUTPUT_AREAS_DBASE));
    WriteIniParameter(WriteFile, OUTPUT_RR_ASCII, GetParameterString(OUTPUT_RR_ASCII, s).c_str(), GetParameterComment(OUTPUT_RR_ASCII));
    WriteIniParameter(WriteFile, OUTPUT_RR_DBASE, GetParameterString(OUTPUT_RR_DBASE, s).c_str(), GetParameterComment(OUTPUT_RR_DBASE));
    WriteIniParameter(WriteFile, OUTPUT_SIM_LLR_ASCII, GetParameterString(OUTPUT_SIM_LLR_ASCII, s).c_str(), GetParameterComment(OUTPUT_SIM_LLR_ASCII));
    WriteIniParameter(WriteFile, OUTPUT_SIM_LLR_DBASE, GetParameterString(OUTPUT_SIM_LLR_DBASE, s).c_str(), GetParameterComment(OUTPUT_SIM_LLR_DBASE));
  } catch (prg_exception& x) {
    x.addTrace("WriteOutputSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Writes parameter settings grouped under 'Power Evaluations'. */
void IniParameterFileAccess::WritePowerEvaluationsSettings(IniFile& WriteFile) {
    std::string s;
    try {
        WriteIniParameter(WriteFile, POWER_EVALUATION, GetParameterString(POWER_EVALUATION, s).c_str(), GetParameterComment(POWER_EVALUATION));
        WriteIniParameter(WriteFile, PE_METHOD_TYPE, GetParameterString(PE_METHOD_TYPE, s).c_str(), GetParameterComment(PE_METHOD_TYPE));
        WriteIniParameter(WriteFile, PE_COUNT, GetParameterString(PE_COUNT, s).c_str(), GetParameterComment(PE_COUNT));
        WriteIniParameter(WriteFile, PE_CRITICAL_TYPE, GetParameterString(PE_CRITICAL_TYPE, s).c_str(), GetParameterComment(PE_CRITICAL_TYPE));
        WriteIniParameter(WriteFile, POWER_05, GetParameterString(POWER_05, s).c_str(), GetParameterComment(POWER_05));
        WriteIniParameter(WriteFile, POWER_01, GetParameterString(POWER_01, s).c_str(), GetParameterComment(POWER_01));
        WriteIniParameter(WriteFile, POWER_001, GetParameterString(POWER_001, s).c_str(), GetParameterComment(POWER_001));
        WriteIniParameter(WriteFile, PE_ESTIMATION_TYPE, GetParameterString(PE_ESTIMATION_TYPE, s).c_str(), GetParameterComment(PE_ESTIMATION_TYPE));
        WriteIniParameter(WriteFile, PE_POWER_REPLICAS, GetParameterString(PE_POWER_REPLICAS, s).c_str(), GetParameterComment(PE_POWER_REPLICAS));
        WriteIniParameter(WriteFile, PE_ALT_HYPOTHESIS_FILE, GetParameterString(PE_ALT_HYPOTHESIS_FILE, s).c_str(), GetParameterComment(PE_ALT_HYPOTHESIS_FILE));
        WriteIniParameter(WriteFile, PE_SIMULATION_TYPE, GetParameterString(PE_SIMULATION_TYPE, s).c_str(), GetParameterComment(PE_SIMULATION_TYPE));
        WriteIniParameter(WriteFile, PE_SIMULATION_SOURCEFILE, GetParameterString(PE_SIMULATION_SOURCEFILE, s).c_str(), GetParameterComment(PE_SIMULATION_SOURCEFILE));
        WriteIniParameter(WriteFile, PE_OUTPUT_SIMUALTION_DATA, GetParameterString(PE_OUTPUT_SIMUALTION_DATA, s).c_str(), GetParameterComment(PE_OUTPUT_SIMUALTION_DATA));
        WriteIniParameter(WriteFile, PE_SIMUALTION_OUTPUTFILE, GetParameterString(PE_SIMUALTION_OUTPUTFILE, s).c_str(), GetParameterComment(PE_SIMUALTION_OUTPUTFILE));
    } catch (prg_exception& x) {
        x.addTrace("WritePowerEvaluationsSettings()","IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under '[Power Simulations]'. */
void IniParameterFileAccess::WritePowerSimulationsSettings(IniFile& WriteFile) {
    std::string s;
    try {
        WriteIniParameter(WriteFile, SIMULATION_TYPE, GetParameterString(SIMULATION_TYPE, s).c_str(), GetParameterComment(SIMULATION_TYPE));
        WriteIniParameter(WriteFile, SIMULATION_SOURCEFILE, GetParameterString(SIMULATION_SOURCEFILE, s).c_str(), GetParameterComment(SIMULATION_SOURCEFILE));
        WriteIniParameter(WriteFile, OUTPUT_SIMULATION_DATA, GetParameterString(OUTPUT_SIMULATION_DATA, s).c_str(), GetParameterComment(OUTPUT_SIMULATION_DATA));
        WriteIniParameter(WriteFile, SIMULATION_DATA_OUTFILE, GetParameterString(SIMULATION_DATA_OUTFILE, s).c_str(), GetParameterComment(SIMULATION_DATA_OUTFILE));
    } catch (prg_exception& x) {
        x.addTrace("WritePowerSimulationsSettings()","IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under 'Run Options'. */
void IniParameterFileAccess::WriteRunOptionSettings(IniFile& WriteFile) {
    std::string s;
    try {
        WriteIniParameter(WriteFile, NUM_PROCESSES, GetParameterString(NUM_PROCESSES, s).c_str(), GetParameterComment(NUM_PROCESSES));
        WriteIniParameter(WriteFile, SUPPRESS_WARNINGS, GetParameterString(SUPPRESS_WARNINGS, s).c_str(), GetParameterComment(SUPPRESS_WARNINGS));
        WriteIniParameter(WriteFile, LOG_HISTORY, GetParameterString(LOG_HISTORY, s).c_str(), GetParameterComment(LOG_HISTORY));
        WriteIniParameter(WriteFile, EXECUTION_TYPE, GetParameterString(EXECUTION_TYPE, s).c_str(), GetParameterComment(EXECUTION_TYPE));
        // since randomly generating seed is a hidden parameter, only write to file if user has switched from default;
        if (gParameters.GetIsRandomlyGeneratingSeed())
            WriteIniParameter(WriteFile, RANDOMLY_GENERATE_SEED, GetParameterString(RANDOMLY_GENERATE_SEED, s).c_str(), GetParameterComment(RANDOMLY_GENERATE_SEED));
        // since randomization seed is a hidden parameter, only write to file if user had specified one originally;
        // which we'll determine by whether it is different than default seed
        if (gParameters.GetRandomizationSeed() != RandomNumberGenerator::glDefaultSeed)
            WriteIniParameter(WriteFile, RANDOMIZATION_SEED, GetParameterString(RANDOMIZATION_SEED, s).c_str(), GetParameterComment(RANDOMIZATION_SEED));
        //WriteIniParameter(WriteFile, TIMETRENDCONVRG, GetParameterString(TIMETRENDCONVRG, s).c_str(), GetParameterComment(TIMETRENDCONVRG));  //---  until SVTT is available, don't write
    } catch (prg_exception& x) {
        x.addTrace("WriteRunOptionSettings()","IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under 'Space And Time Adjustment' Window. */
void IniParameterFileAccess::WriteSpaceAndTimeAdjustmentSettings(IniFile& WriteFile) {
    std::string  s;
    try {
        WriteIniParameter(WriteFile, TIMETREND, GetParameterString(TIMETREND, s).c_str(), GetParameterComment(TIMETREND));
        WriteIniParameter(WriteFile, TIMETRENDPERC, GetParameterString(TIMETRENDPERC, s).c_str(), GetParameterComment(TIMETRENDPERC));
        WriteIniParameter(WriteFile, TIME_TREND_TYPE, GetParameterString(TIME_TREND_TYPE, s).c_str(), GetParameterComment(TIME_TREND_TYPE));
        WriteIniParameter(WriteFile, ADJUST_WEEKLY_TRENDS, GetParameterString(ADJUST_WEEKLY_TRENDS, s).c_str(), GetParameterComment(ADJUST_WEEKLY_TRENDS));
        WriteIniParameter(WriteFile, SPATIAL_ADJ_TYPE, GetParameterString(SPATIAL_ADJ_TYPE, s).c_str(), GetParameterComment(SPATIAL_ADJ_TYPE));
        WriteIniParameter(WriteFile, USE_ADJ_BY_RR_FILE, GetParameterString(USE_ADJ_BY_RR_FILE, s).c_str(), GetParameterComment(USE_ADJ_BY_RR_FILE));
        GetParameterString(ADJ_BY_RR_FILE, s);
        WriteIniParameter(WriteFile, ADJ_BY_RR_FILE, s.c_str(), GetParameterComment(ADJ_BY_RR_FILE));
        if (s.size()) WriteInputSource(WriteFile, ADJ_BY_RR_FILE, gParameters.getInputSource(ADJ_BY_RR_FILE));
    } catch (prg_exception& x) {
        x.addTrace("WriteSpaceAndTimeAdjustmentSettings()","IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under 'Spatial Neighbors'. */
void IniParameterFileAccess::WriteSpatialNeighborsSettings(IniFile& WriteFile) {
    std::string  s;
    try {
        WriteIniParameter(WriteFile, USE_LOCATION_NEIGHBORS_FILE, GetParameterString(USE_LOCATION_NEIGHBORS_FILE, s).c_str(), GetParameterComment(USE_LOCATION_NEIGHBORS_FILE));
        WriteIniParameter(WriteFile, LOCATION_NEIGHBORS_FILE, GetParameterString(LOCATION_NEIGHBORS_FILE, s).c_str(), GetParameterComment(LOCATION_NEIGHBORS_FILE));
        WriteIniParameter(WriteFile, USE_META_LOCATIONS_FILE, GetParameterString(USE_META_LOCATIONS_FILE, s).c_str(), GetParameterComment(USE_META_LOCATIONS_FILE));
        WriteIniParameter(WriteFile, META_LOCATIONS_FILE, GetParameterString(META_LOCATIONS_FILE, s).c_str(), GetParameterComment(META_LOCATIONS_FILE));
        WriteIniParameter(WriteFile, MULTIPLE_COORDINATES_TYPE, GetParameterString(MULTIPLE_COORDINATES_TYPE, s).c_str(), GetParameterComment(MULTIPLE_COORDINATES_TYPE));
        GetParameterString(MULTIPLE_LOCATIONS_FILE, s);
        WriteIniParameter(WriteFile, MULTIPLE_LOCATIONS_FILE, s.c_str(), GetParameterComment(MULTIPLE_LOCATIONS_FILE));
        if (s.size()) WriteInputSource(WriteFile, MULTIPLE_LOCATIONS_FILE, gParameters.getInputSource(MULTIPLE_LOCATIONS_FILE));
    } catch (prg_exception& x) {
        x.addTrace("WriteSpatialNeighborsSettings(IniFile)","IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under 'Spatial Window'. */
void IniParameterFileAccess::WriteSpatialWindowSettings(IniFile& WriteFile) {
    std::string  s;
    try {
        WriteIniParameter(WriteFile, MAXGEOPOPATRISK, GetParameterString(MAXGEOPOPATRISK, s).c_str(), GetParameterComment(MAXGEOPOPATRISK));
        WriteIniParameter(WriteFile, USE_MAXGEOPOPFILE, GetParameterString(USE_MAXGEOPOPFILE, s).c_str(), GetParameterComment(USE_MAXGEOPOPFILE));
        WriteIniParameter(WriteFile, MAXGEOPOPFILE, GetParameterString(MAXGEOPOPFILE, s).c_str(), GetParameterComment(MAXGEOPOPFILE));
        GetParameterString(MAXCIRCLEPOPFILE, s);
        WriteIniParameter(WriteFile, MAXCIRCLEPOPFILE, s.c_str(), GetParameterComment(MAXCIRCLEPOPFILE));
        if (s.size()) WriteInputSource(WriteFile, MAXCIRCLEPOPFILE, gParameters.getInputSource(MAXCIRCLEPOPFILE));
        WriteIniParameter(WriteFile, USE_MAXGEODISTANCE, GetParameterString(USE_MAXGEODISTANCE, s).c_str(), GetParameterComment(USE_MAXGEODISTANCE));
        WriteIniParameter(WriteFile, MAXGEODISTANCE, GetParameterString(MAXGEODISTANCE, s).c_str(), GetParameterComment(MAXGEODISTANCE));
        WriteIniParameter(WriteFile, PURETEMPORAL, GetParameterString(PURETEMPORAL, s).c_str(), GetParameterComment(PURETEMPORAL));
        WriteIniParameter(WriteFile, WINDOW_SHAPE, GetParameterString(WINDOW_SHAPE, s).c_str(), GetParameterComment(WINDOW_SHAPE));
        WriteIniParameter(WriteFile, NON_COMPACTNESS_PENALTY, GetParameterString(NON_COMPACTNESS_PENALTY, s).c_str(), GetParameterComment(NON_COMPACTNESS_PENALTY));
        WriteIniParameter(WriteFile, RISKFUNCTION, GetParameterString(RISKFUNCTION, s).c_str(), GetParameterComment(RISKFUNCTION));
    } catch (prg_exception& x) {
        x.addTrace("WriteSpatialWindowSettings()","IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under '[System]'. */
void IniParameterFileAccess::WriteSystemSettings(IniFile& WriteFile) {
    std::string s;
    try {
        WriteIniParameter(WriteFile, CREATION_VERSION, GetParameterString(CREATION_VERSION, s).c_str(), GetParameterComment(CREATION_VERSION));
    } catch (prg_exception& x) {
        x.addTrace("WriteSystemSettings()","IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under 'Temporal Graph'. */
void IniParameterFileAccess::WriteTemporalGraphSettings(IniFile& WriteFile) {
    std::string  s;
    try {
        WriteIniParameter(WriteFile, OUTPUT_TEMPORAL_GRAPH, GetParameterString(OUTPUT_TEMPORAL_GRAPH, s).c_str(), GetParameterComment(OUTPUT_TEMPORAL_GRAPH));
        WriteIniParameter(WriteFile, TEMPORAL_GRAPH_REPORT_TYPE, GetParameterString(TEMPORAL_GRAPH_REPORT_TYPE, s).c_str(), GetParameterComment(TEMPORAL_GRAPH_REPORT_TYPE));
        WriteIniParameter(WriteFile, TEMPORAL_GRAPH_MLC_COUNT, GetParameterString(TEMPORAL_GRAPH_MLC_COUNT, s).c_str(), GetParameterComment(TEMPORAL_GRAPH_MLC_COUNT));
        WriteIniParameter(WriteFile, TEMPORAL_GRAPH_CUTOFF, GetParameterString(TEMPORAL_GRAPH_CUTOFF, s).c_str(), GetParameterComment(TEMPORAL_GRAPH_CUTOFF));
    } catch (prg_exception& x) {
        x.addTrace("WriteTemporalGraphSettings()","IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under 'Temporal Window'. */
void IniParameterFileAccess::WriteTemporalWindowSettings(IniFile& WriteFile) {
    std::string  s;
    try {
        WriteIniParameter(WriteFile, MIN_TEMPORAL_CLUSTER, GetParameterString(MIN_TEMPORAL_CLUSTER, s).c_str(), GetParameterComment(MIN_TEMPORAL_CLUSTER));
        WriteIniParameter(WriteFile, MAX_TEMPORAL_TYPE, GetParameterString(MAX_TEMPORAL_TYPE, s).c_str(), GetParameterComment(MAX_TEMPORAL_TYPE));
        WriteIniParameter(WriteFile, TIMESIZE, GetParameterString(TIMESIZE, s).c_str(), GetParameterComment(TIMESIZE));
        WriteIniParameter(WriteFile, PURESPATIAL, GetParameterString(PURESPATIAL, s).c_str(), GetParameterComment(PURESPATIAL));
        WriteIniParameter(WriteFile, CLUSTERS, GetParameterString(CLUSTERS, s).c_str(), GetParameterComment(CLUSTERS));
        WriteIniParameter(WriteFile, INTERVAL_STARTRANGE, GetParameterString(INTERVAL_STARTRANGE, s).c_str(), GetParameterComment(INTERVAL_STARTRANGE));
        WriteIniParameter(WriteFile, INTERVAL_ENDRANGE, GetParameterString(INTERVAL_ENDRANGE, s).c_str(), GetParameterComment(INTERVAL_ENDRANGE));
    } catch (prg_exception& x) {
        x.addTrace("WriteTemporalWindowSettings()","IniParameterFileAccess");
        throw;
    }
}

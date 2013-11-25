//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop                                                        
//***************************************************************************
#include "IniParameterFileAccess.h"
#include "RandomNumberGenerator.h"
                                                                       
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
    } catch (prg_exception& x) {
        x.addTrace("Read()","IniParameterFileAccess");
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
        gParameters.SetNumDataSets(iMostDataSets);
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

/** Writes parameters of associated CParameters object to ini file, of most recent
    format specification. */
void IniParameterFileAccess::Write(const char* sFilename) {
    try {
        IniFile WriteFile;
        gParameters.SetSourceFileName(sFilename);
        gpSpecifications = new IniParameterSpecification();

        //write settings as provided in main graphical interface
        WriteInputSettings(WriteFile);
        WriteAnalysisSettings(WriteFile);
        WriteOutputSettings(WriteFile);

        //write settings as provided in advanced features of graphical interface
        WriteObservableRegionSettings(WriteFile);
        WriteMultipleDataSetsSettings(WriteFile);
        WriteDataCheckingSettings(WriteFile);
        WriteSpatialNeighborsSettings(WriteFile);
        WriteSpatialWindowSettings(WriteFile);
        WriteTemporalWindowSettings(WriteFile);
        WriteSpaceAndTimeAdjustmentSettings(WriteFile);
        WriteInferenceSettings(WriteFile);
        WritePowerEvaluationsSettings(WriteFile);
        WriteSpatialOutputSettings(WriteFile);
        WriteTemporalGraphSettings(WriteFile);
        WriteOtherOutputSettings(WriteFile);

        //write settings as provided only through user mofication of parameter file and batch executable
        WriteEllipticScanSettings(WriteFile);
        WritePowerSimulationsSettings(WriteFile);
        WriteRunOptionSettings(WriteFile);
        WriteSystemSettings(WriteFile);

        WriteFile.Write(gParameters.GetSourceFileName());
    } catch (prg_exception& x) {
        x.addTrace("Write()", "IniParameterFileAccess");
        throw;
    }
}

/** Writes parameter settings grouped under 'Other Output'. */
void IniParameterFileAccess::WriteOtherOutputSettings(IniFile& WriteFile) {
    std::string s;
    try {
        WriteIniParameter(WriteFile, REPORT_CRITICAL_VALUES, GetParameterString(REPORT_CRITICAL_VALUES, s).c_str(), GetParameterComment(REPORT_CRITICAL_VALUES));
        WriteIniParameter(WriteFile, REPORT_RANK, GetParameterString(REPORT_RANK, s).c_str(), GetParameterComment(REPORT_RANK));
        WriteIniParameter(WriteFile, PRINT_ASCII_HEADERS, GetParameterString(PRINT_ASCII_HEADERS, s).c_str(), GetParameterComment(PRINT_ASCII_HEADERS));
    } catch (prg_exception& x) {
        x.addTrace("WriteOtherOutputSettings()","IniParameterFileAccess");
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
        WriteIniParameter(WriteFile, LAUNCH_KML_VIEWER, GetParameterString(LAUNCH_KML_VIEWER, s).c_str(), GetParameterComment(LAUNCH_KML_VIEWER));
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
        x.addTrace("WriteIniParameters()","IniParameterFileAccess");
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
    try {
        WriteIniParameter(WriteFile, CASEFILE, GetParameterString(CASEFILE, s).c_str(), GetParameterComment(CASEFILE));
        WriteIniParameter(WriteFile, CONTROLFILE, GetParameterString(CONTROLFILE, s).c_str(), GetParameterComment(CONTROLFILE));
        WriteIniParameter(WriteFile, PRECISION, GetParameterString(PRECISION, s).c_str(), GetParameterComment(PRECISION));
        WriteIniParameter(WriteFile, STARTDATE, GetParameterString(STARTDATE, s).c_str(), GetParameterComment(STARTDATE));
        WriteIniParameter(WriteFile, ENDDATE, GetParameterString(ENDDATE, s).c_str(), GetParameterComment(ENDDATE));
        WriteIniParameter(WriteFile, POPFILE, GetParameterString(POPFILE, s).c_str(), GetParameterComment(POPFILE));
        WriteIniParameter(WriteFile, COORDFILE, GetParameterString(COORDFILE, s).c_str(), GetParameterComment(COORDFILE));
        WriteIniParameter(WriteFile, SPECIALGRID, GetParameterString(SPECIALGRID, s).c_str(), GetParameterComment(SPECIALGRID));
        WriteIniParameter(WriteFile, GRIDFILE, GetParameterString(GRIDFILE, s).c_str(), GetParameterComment(GRIDFILE));
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

        for (size_t t=1; t < gParameters.GetNumDataSets(); ++t) {
            if (GetSpecifications().GetMultipleParameterIniInfo(CASEFILE, &sSectionName, &sBaseKey)) {
                printString(s, "%s%i", sBaseKey, t + 1);
                printString(sComment, " case data filename (additional data set %i)", t + 1);
                WriteIniParameterAsKey(WriteFile, sSectionName, s.c_str(), gParameters.GetCaseFileName(t + 1).c_str(), sComment.c_str());
            }
            if (GetSpecifications().GetMultipleParameterIniInfo(CONTROLFILE, &sSectionName, &sBaseKey)) {
                printString(s, "%s%i", sBaseKey, t + 1);
                printString(sComment, " control data filename (additional data set %i)", t + 1);
                WriteIniParameterAsKey(WriteFile, sSectionName, s.c_str(), gParameters.GetControlFileName(t + 1).c_str(), sComment.c_str());
            }
            if (GetSpecifications().GetMultipleParameterIniInfo(POPFILE, &sSectionName, &sBaseKey)) {
                printString(s, "%s%i", sBaseKey, t + 1);
                printString(sComment, " population data filename (additional data set %i)", t + 1);
                WriteIniParameterAsKey(WriteFile, sSectionName, s.c_str(), gParameters.GetPopulationFileName(t + 1).c_str(), sComment.c_str());
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
        WriteIniParameter(WriteFile, ADJ_BY_RR_FILE, GetParameterString(ADJ_BY_RR_FILE, s).c_str(), GetParameterComment(ADJ_BY_RR_FILE));
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
    } catch (prg_exception& x) {
        x.addTrace("WriteSpatialNeighborsSettings()","IniParameterFileAccess");
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
        WriteIniParameter(WriteFile, MAXCIRCLEPOPFILE, GetParameterString(MAXCIRCLEPOPFILE, s).c_str(), GetParameterComment(MAXCIRCLEPOPFILE));
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

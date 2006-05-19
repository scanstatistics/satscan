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
  }
  catch (...){}  
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
      ZdGenerateException("Specifications object not allocated.", "GetSpecifications()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetSpecifications()","IniParameterFileAccess");
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

    ZdIniFile SourceFile(sFilename, true, false, ZDIO_OPEN_READ|ZDIO_SREAD);
    gpSpecifications = new IniParameterSpecification(SourceFile, gParameters);

    for (ParameterType eType=ANALYSISTYPE; eType <= gParameters.giNumParameters; eType = ParameterType(eType + 1))
       ReadIniParameter(SourceFile, eType);
    ReadMultipleDataSetsSettings(SourceFile);
  }
  catch (ZdException &x) {
    x.AddCallpath("Read()","IniParameterFileAccess");
    throw;
  }
  return !gbReadStatusError;
}

/** Reads parameter from ini file and sets in CParameter object. If parameter specification not
    found or ini section/key not found in file, marks as defaulted. */
void IniParameterFileAccess::ReadIniParameter(const ZdIniFile& SourceFile, ParameterType eParameterType) {
  long          lSectionIndex, lKeyIndex=-1;
  const char  * sSectionName, * sKey;

  try {
    if (GetSpecifications().GetParameterIniInfo(eParameterType, &sSectionName, &sKey)) {
      if ((lSectionIndex = SourceFile.GetSectionIndex(sSectionName)) > -1) {
        const ZdIniSection  * pSection = SourceFile.GetSection(lSectionIndex);
        if ((lKeyIndex = pSection->FindKey(sKey)) > -1)
          SetParameter(eParameterType, ZdString(pSection->GetLine(lKeyIndex)->GetValue()), gPrintDirection);
      }
    }
    //if (lKeyIndex == -1)
    //  MarkAsMissingDefaulted(eParameterType, gPrintDirection);
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadIniParameter()","IniParameterFileAccess");
    throw;
  }
}

/** Reads parameter from ini file and returns all found key values in vector for
    those parameters that have optional additional keys, such as files with
    multiple datasets. */
std::vector<ZdString>& IniParameterFileAccess::ReadIniParameter(const ZdIniFile& SourceFile, ParameterType eParameterType, std::vector<ZdString>& vParameters) const {
  long                  lSectionIndex, lKeyIndex;
  ZdString              sNextKey;
  const char          * sSectionName, * sKey;
  size_t                iDataSets=2;

  vParameters.clear();
  if (GetSpecifications().GetMultipleParameterIniInfo(eParameterType, &sSectionName, &sKey)) {
    //read possibly other dataset case source
    if ((lSectionIndex = SourceFile.GetSectionIndex(sSectionName)) > -1) {
      const ZdIniSection  * pSection = SourceFile.GetSection(lSectionIndex);
      sNextKey.printf("%s%i", sKey, iDataSets);
      while ((lKeyIndex = pSection->FindKey(sNextKey)) > -1) {
           vParameters.push_back(ZdString(pSection->GetLine(lKeyIndex)->GetValue()));
           sNextKey.printf("%s%i", sKey, ++iDataSets);
      }
    }
  }

  return vParameters;
}

/** Reads parameter settings grouped under 'Mutliple Data Sets'. */
void IniParameterFileAccess::ReadMultipleDataSetsSettings(const ZdIniFile& SourceFile) {
  std::vector<ZdString>         vFilenames;
  size_t                        t, iMostDataSets=1;

  try {
    ReadIniParameter(SourceFile, MULTI_DATASET_PURPOSE_TYPE);
    ReadIniParameter(SourceFile, CASEFILE, vFilenames);
    for (t=0; t < vFilenames.size(); ++t)
      gParameters.SetCaseFileName(vFilenames[t].GetCString(), true, t + 2);
    iMostDataSets = std::max(iMostDataSets, vFilenames.size() + 1);
    ReadIniParameter(SourceFile, CONTROLFILE, vFilenames);
    iMostDataSets = std::max(iMostDataSets, vFilenames.size() + 1);
    for (t=0; t < vFilenames.size(); ++t)
      gParameters.SetControlFileName(vFilenames[t].GetCString(), true, t + 2);
    ReadIniParameter(SourceFile, POPFILE, vFilenames);
    iMostDataSets = std::max(iMostDataSets, vFilenames.size() + 1);
    for (t=0; t < vFilenames.size(); ++t)
      gParameters.SetPopulationFileName(vFilenames[t].GetCString(), true, t + 2);
    //Synchronize collections of dataset filesnames so that we can ask for
    //any file of a particular dataset, even if blank. This keeps the same behavior
    //as when there was only one dataset.
    gParameters.SetNumDataSets(iMostDataSets);
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadMultipleDataSetsSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Writes parameters of associated CParameters object to ini file, of most recent
    format specification. */
void IniParameterFileAccess::Write(const char* sFilename) {
  try {
    ZdIniFile WriteFile(sFilename);
    WriteFile.Clear();
    gParameters.SetSourceFileName(sFilename);
    gpSpecifications = new IniParameterSpecification();

    //write settings as provided in main graphical interface
    WriteInputSettings(WriteFile);
    WriteAnalysisSettings(WriteFile);
    WriteOutputSettings(WriteFile);
    //write settings as provided in advanced features of graphical interface
    WriteMultipleDataSetsSettings(WriteFile);
    WriteDataCheckingSettings(WriteFile);
    WriteSpatialWindowSettings(WriteFile);
    WriteTemporalWindowSettings(WriteFile);
    WriteSpaceAndTimeAdjustmentSettings(WriteFile);
    WriteInferenceSettings(WriteFile);
    WriteClustersReportedSettings(WriteFile);
    
    //write settings as provided only through user mofication of parameter file and batch executable
    WriteEllipticScanSettings(WriteFile);
    WriteIsotonicScanSettings(WriteFile);
    WritePowerSimulationsSettings(WriteFile);
    WriteRunOptionSettings(WriteFile);
    WriteSystemSettings(WriteFile);

    WriteFile.Write();
  }
  catch (ZdException &x) {
    x.AddCallpath("Write()", "IniParameterFileAccess");
    throw;
  }
}

/** Writes parameter settings grouped under 'Analysis'. */
void IniParameterFileAccess::WriteAnalysisSettings(ZdIniFile& WriteFile) {
  ZdString      s;

  try {
    WriteIniParameter(WriteFile, ANALYSISTYPE, GetParameterString(ANALYSISTYPE, s), GetParameterComment(ANALYSISTYPE));
    WriteIniParameter(WriteFile, MODEL, GetParameterString(MODEL, s), GetParameterComment(MODEL));
    WriteIniParameter(WriteFile, SCANAREAS, GetParameterString(SCANAREAS, s), GetParameterComment(SCANAREAS));
    WriteIniParameter(WriteFile, TIME_AGGREGATION_UNITS, GetParameterString(TIME_AGGREGATION_UNITS, s), GetParameterComment(TIME_AGGREGATION_UNITS));
    WriteIniParameter(WriteFile, TIME_AGGREGATION, GetParameterString(TIME_AGGREGATION, s), GetParameterComment(TIME_AGGREGATION));
    WriteIniParameter(WriteFile, REPLICAS, GetParameterString(REPLICAS, s), GetParameterComment(REPLICAS));
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteAnalysisSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Writes parameter settings grouped under 'Clusters Reported'. */
void IniParameterFileAccess::WriteClustersReportedSettings(ZdIniFile& WriteFile) {
  ZdString      s;

  try {
    WriteIniParameter(WriteFile, CRITERIA_SECOND_CLUSTERS, GetParameterString(CRITERIA_SECOND_CLUSTERS, s), GetParameterComment(CRITERIA_SECOND_CLUSTERS));
    WriteIniParameter(WriteFile, USE_REPORTED_GEOSIZE, GetParameterString(USE_REPORTED_GEOSIZE, s), GetParameterComment(USE_REPORTED_GEOSIZE));
    WriteIniParameter(WriteFile, MAXGEOPOPATRISK_REPORTED, GetParameterString(MAXGEOPOPATRISK_REPORTED, s), GetParameterComment(MAXGEOPOPATRISK_REPORTED));
    WriteIniParameter(WriteFile, MAXGEOPOPFILE_REPORTED, GetParameterString(MAXGEOPOPFILE_REPORTED, s), GetParameterComment(MAXGEOPOPFILE_REPORTED));
    WriteIniParameter(WriteFile, MAXGEODISTANCE_REPORTED, GetParameterString(MAXGEODISTANCE_REPORTED, s), GetParameterComment(MAXGEODISTANCE_REPORTED));
    WriteIniParameter(WriteFile, USE_MAXGEOPOPFILE_REPORTED, GetParameterString(USE_MAXGEOPOPFILE_REPORTED, s), GetParameterComment(USE_MAXGEOPOPFILE_REPORTED));
    WriteIniParameter(WriteFile, USE_MAXGEODISTANCE_REPORTED, GetParameterString(USE_MAXGEODISTANCE_REPORTED, s), GetParameterComment(USE_MAXGEODISTANCE_REPORTED));
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteClustersReportedSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Writes parameter settings grouped under 'Data Checking'. */
void IniParameterFileAccess::WriteDataCheckingSettings(ZdIniFile& WriteFile) {
  ZdString      s;

  try {
    WriteIniParameter(WriteFile, STUDYPERIOD_DATACHECK, GetParameterString(STUDYPERIOD_DATACHECK, s), GetParameterComment(STUDYPERIOD_DATACHECK));
    WriteIniParameter(WriteFile, COORDINATES_DATACHECK, GetParameterString(COORDINATES_DATACHECK, s), GetParameterComment(COORDINATES_DATACHECK));
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteDataCheckingSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Writes parameter settings grouped under '[Elliptic Scan]'. */
void IniParameterFileAccess::WriteEllipticScanSettings(ZdIniFile& WriteFile) {
  ZdString      s;

  try {
    WriteIniParameter(WriteFile, ESHAPES, GetParameterString(ESHAPES, s), GetParameterComment(ESHAPES));
    WriteIniParameter(WriteFile, ENUMBERS, GetParameterString(ENUMBERS, s), GetParameterComment(ENUMBERS));
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteEllipticScanSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Writes parameter settings grouped under 'Inference'. */
void IniParameterFileAccess::WriteInferenceSettings(ZdIniFile& WriteFile) {
  ZdString      s;

  try {
    WriteIniParameter(WriteFile, START_PROSP_SURV, GetParameterString(START_PROSP_SURV, s), GetParameterComment(START_PROSP_SURV));
    WriteIniParameter(WriteFile, EARLY_SIM_TERMINATION, GetParameterString(EARLY_SIM_TERMINATION, s), GetParameterComment(EARLY_SIM_TERMINATION));
    WriteIniParameter(WriteFile, ADJ_FOR_EALIER_ANALYSES, GetParameterString(ADJ_FOR_EALIER_ANALYSES, s), GetParameterComment(ADJ_FOR_EALIER_ANALYSES));
    WriteIniParameter(WriteFile, REPORT_CRITICAL_VALUES, GetParameterString(REPORT_CRITICAL_VALUES, s), GetParameterComment(REPORT_CRITICAL_VALUES));
    WriteIniParameter(WriteFile, SEQUENTIAL, GetParameterString(SEQUENTIAL, s), GetParameterComment(SEQUENTIAL));
    WriteIniParameter(WriteFile, SEQNUM, GetParameterString(SEQNUM, s), GetParameterComment(SEQNUM));
    WriteIniParameter(WriteFile, SEQPVAL, GetParameterString(SEQPVAL, s), GetParameterComment(SEQPVAL));
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteSpaceAndTimeAdjustmentSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Writes specified comment and value to file for parameter type. */
void IniParameterFileAccess::WriteIniParameter(ZdIniFile& WriteFile, ParameterType eParameterType, const char* sValue, const char* sComment) {
  const char  * sSectionName, * sKey;

  try {
    if (GetSpecifications().GetParameterIniInfo(eParameterType, &sSectionName, &sKey)) {
      ZdIniSection *  pSection = WriteFile.GetSection(sSectionName);
      if (sComment) pSection->AddComment(sComment);
      pSection->AddLine(sKey, sValue);
    }
    else ZdException::Generate("Unknown parameter type '%d'.", "WriteIniParameters()", eParameterType);
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteIniParameters()","IniParameterFileAccess");
    throw;
  }
}

/** Writes specified comment and value to file as specified section/key names. */
void IniParameterFileAccess::WriteIniParameterAsKey(ZdIniFile& WriteFile, const char* sSectionName, const char * sKey, const char* sValue, const char* sComment) {
  try {
    ZdIniSection *  pSection = WriteFile.GetSection(sSectionName);
    if (sComment) pSection->AddComment(sComment);
    pSection->AddLine(sKey, sValue);
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteIniParameterAsKey()","IniParameterFileAccess");
    throw;
  }
}

/** Writes parameter settings grouped under '[Isotonic Scan]'. */
void IniParameterFileAccess::WriteIsotonicScanSettings(ZdIniFile& WriteFile) {
  ZdString      s;

  try {
    WriteIniParameter(WriteFile, RISKFUNCTION, GetParameterString(RISKFUNCTION, s), GetParameterComment(RISKFUNCTION));
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteIsotonicScanSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Writes parameter settings grouped under 'Input'. */
void IniParameterFileAccess::WriteInputSettings(ZdIniFile& WriteFile) {
  ZdString      s;

  try {
    WriteIniParameter(WriteFile, CASEFILE, GetParameterString(CASEFILE, s), GetParameterComment(CASEFILE));
    WriteIniParameter(WriteFile, CONTROLFILE, GetParameterString(CONTROLFILE, s), GetParameterComment(CONTROLFILE));
    WriteIniParameter(WriteFile, POPFILE, GetParameterString(POPFILE, s), GetParameterComment(POPFILE));
    WriteIniParameter(WriteFile, COORDFILE, GetParameterString(COORDFILE, s), GetParameterComment(COORDFILE));
    WriteIniParameter(WriteFile, SPECIALGRID, GetParameterString(SPECIALGRID, s), GetParameterComment(SPECIALGRID));
    WriteIniParameter(WriteFile, GRIDFILE, GetParameterString(GRIDFILE, s), GetParameterComment(GRIDFILE));
    WriteIniParameter(WriteFile, PRECISION, GetParameterString(PRECISION, s), GetParameterComment(PRECISION));
    WriteIniParameter(WriteFile, COORDTYPE, GetParameterString(COORDTYPE, s), GetParameterComment(COORDTYPE));
    WriteIniParameter(WriteFile, STARTDATE, GetParameterString(STARTDATE, s), GetParameterComment(STARTDATE));
    WriteIniParameter(WriteFile, ENDDATE, GetParameterString(ENDDATE, s), GetParameterComment(ENDDATE));
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteInputSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Writes parameter settings grouped under 'Multiple Data Sets'. */
void IniParameterFileAccess::WriteMultipleDataSetsSettings(ZdIniFile& WriteFile) {
  ZdString      s, sComment;
  const char  * sSectionName, * sBaseKey;

  try {
    WriteIniParameter(WriteFile, MULTI_DATASET_PURPOSE_TYPE, AsString(s, gParameters.GetMultipleDataSetPurposeType()),
                      " multiple data sets purpose type (multivariate=0, adjustment=1)");

    if (GetSpecifications().GetMultipleParameterIniInfo(CASEFILE, &sSectionName, &sBaseKey)) {
      for (size_t t=1; t < gParameters.GetNumDataSets(); ++t) {
         s.printf("%s%i", sBaseKey, t + 1);
         sComment.printf(" case data filename (additional data set %i)", t + 1);
         WriteIniParameterAsKey(WriteFile, sSectionName, s.GetCString(), gParameters.GetCaseFileName(t + 1).c_str(), sComment.GetCString());
      }
    }
    if (GetSpecifications().GetMultipleParameterIniInfo(CONTROLFILE, &sSectionName, &sBaseKey)) {
      for (size_t t=1; t < gParameters.GetNumDataSets(); ++t) {
         s.printf("%s%i", sBaseKey, t + 1);
         sComment.printf(" control data filename (additional data set %i)", t + 1);
         WriteIniParameterAsKey(WriteFile, sSectionName, s.GetCString(), gParameters.GetControlFileName(t + 1).c_str(), sComment.GetCString());
      }
    }
    if (GetSpecifications().GetMultipleParameterIniInfo(POPFILE, &sSectionName, &sBaseKey)) {
      for (size_t t=1; t < gParameters.GetNumDataSets(); ++t) {
         s.printf("%s%i", sBaseKey, t + 1);
         sComment.printf(" population data filename (additional data set %i)", t + 1);
         WriteIniParameterAsKey(WriteFile, sSectionName, s.GetCString(), gParameters.GetPopulationFileName(t + 1).c_str(), sComment.GetCString());
      }
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteMultipleDataSetsSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Writes parameter settings grouped under 'Output'. */
void IniParameterFileAccess::WriteOutputSettings(ZdIniFile& WriteFile) {
  ZdString      s;

  try {
    WriteIniParameter(WriteFile, OUTPUTFILE, GetParameterString(OUTPUTFILE, s), GetParameterComment(OUTPUTFILE));
    WriteIniParameter(WriteFile, OUTPUT_SIM_LLR_ASCII, GetParameterString(OUTPUT_SIM_LLR_ASCII, s), GetParameterComment(OUTPUT_SIM_LLR_ASCII));
    WriteIniParameter(WriteFile, OUTPUT_SIM_LLR_DBASE, GetParameterString(OUTPUT_SIM_LLR_DBASE, s), GetParameterComment(OUTPUT_SIM_LLR_DBASE));
    WriteIniParameter(WriteFile, OUTPUT_RR_ASCII, GetParameterString(OUTPUT_RR_ASCII, s), GetParameterComment(OUTPUT_RR_ASCII));
    WriteIniParameter(WriteFile, OUTPUT_RR_DBASE, GetParameterString(OUTPUT_RR_DBASE, s), GetParameterComment(OUTPUT_RR_DBASE));
    WriteIniParameter(WriteFile, OUTPUT_AREAS_ASCII, GetParameterString(OUTPUT_AREAS_ASCII, s), GetParameterComment(OUTPUT_AREAS_ASCII));
    WriteIniParameter(WriteFile, OUTPUT_AREAS_DBASE, GetParameterString(OUTPUT_AREAS_DBASE, s), GetParameterComment(OUTPUT_AREAS_DBASE));
    WriteIniParameter(WriteFile, OUTPUT_MLC_ASCII, GetParameterString(OUTPUT_MLC_ASCII, s), GetParameterComment(OUTPUT_MLC_ASCII));
    WriteIniParameter(WriteFile, OUTPUT_MLC_DBASE, GetParameterString(OUTPUT_MLC_DBASE, s), GetParameterComment(OUTPUT_MLC_DBASE));
    WriteIniParameter(WriteFile, OUTPUT_MLC_CASE_ASCII, GetParameterString(OUTPUT_MLC_CASE_ASCII, s), GetParameterComment(OUTPUT_MLC_CASE_ASCII));
    WriteIniParameter(WriteFile, OUTPUT_MLC_CASE_DBASE, GetParameterString(OUTPUT_MLC_CASE_DBASE, s), GetParameterComment(OUTPUT_MLC_CASE_DBASE));
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteOutputSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Writes parameter settings grouped under '[Power Simulations]'. */
void IniParameterFileAccess::WritePowerSimulationsSettings(ZdIniFile& WriteFile) {
  ZdString      s;

  try {
    WriteIniParameter(WriteFile, POWERCALC, GetParameterString(POWERCALC, s), GetParameterComment(POWERCALC));
    WriteIniParameter(WriteFile, POWERX, GetParameterString(POWERX, s), GetParameterComment(POWERX));
    WriteIniParameter(WriteFile, POWERY, GetParameterString(POWERY, s), GetParameterComment(POWERY));
    WriteIniParameter(WriteFile, SIMULATION_TYPE, GetParameterString(SIMULATION_TYPE, s), GetParameterComment(SIMULATION_TYPE));
    WriteIniParameter(WriteFile, SIMULATION_SOURCEFILE, GetParameterString(SIMULATION_SOURCEFILE, s), GetParameterComment(SIMULATION_SOURCEFILE));
    WriteIniParameter(WriteFile, OUTPUT_SIMULATION_DATA, GetParameterString(OUTPUT_SIMULATION_DATA, s), GetParameterComment(OUTPUT_SIMULATION_DATA));
    WriteIniParameter(WriteFile, SIMULATION_DATA_OUTFILE, GetParameterString(SIMULATION_DATA_OUTFILE, s), GetParameterComment(SIMULATION_DATA_OUTFILE));
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteSpaceAndTimeAdjustmentSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Writes parameter settings grouped under '[Run Options]'. */
void IniParameterFileAccess::WriteRunOptionSettings(ZdIniFile& WriteFile) {
  ZdString      s;

  try {
    WriteIniParameter(WriteFile, VALIDATE, GetParameterString(VALIDATE, s), GetParameterComment(VALIDATE));
    // since randomization seed is a hidden parameter, only write to file if user had specified one originally;
    // which we'll determine by whether it is different than default seed
    if (gParameters.GetRandomizationSeed() != RandomNumberGenerator::glDefaultSeed)                  
      WriteIniParameter(WriteFile, RANDOMIZATION_SEED, GetParameterString(RANDOMIZATION_SEED, s), GetParameterComment(RANDOMIZATION_SEED));
    //WriteIniParameter(WriteFile, TIMETRENDCONVRG, GetParameterString(TIMETRENDCONVRG, s), GetParameterComment(TIMETRENDCONVRG));  //---  until SVTT is available, don't write
    WriteIniParameter(WriteFile, EXECUTION_TYPE, GetParameterString(EXECUTION_TYPE, s), GetParameterComment(EXECUTION_TYPE));
    WriteIniParameter(WriteFile, NUM_PROCESSES, GetParameterString(NUM_PROCESSES, s), GetParameterComment(NUM_PROCESSES));
    WriteIniParameter(WriteFile, LOG_HISTORY, GetParameterString(LOG_HISTORY, s), GetParameterComment(LOG_HISTORY));
    WriteIniParameter(WriteFile, SUPPRESS_WARNINGS, GetParameterString(SUPPRESS_WARNINGS, s), GetParameterComment(SUPPRESS_WARNINGS));
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteRunOptionSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Writes parameter settings grouped under 'Spatial Window'. */
void IniParameterFileAccess::WriteSpaceAndTimeAdjustmentSettings(ZdIniFile& WriteFile) {
  ZdString      s;

  try {
    WriteIniParameter(WriteFile, TIMETREND, GetParameterString(TIMETREND, s), GetParameterComment(TIMETREND));
    WriteIniParameter(WriteFile, TIMETRENDPERC, GetParameterString(TIMETRENDPERC, s), GetParameterComment(TIMETRENDPERC));
    WriteIniParameter(WriteFile, ADJ_BY_RR_FILE, GetParameterString(ADJ_BY_RR_FILE, s), GetParameterComment(ADJ_BY_RR_FILE));
    WriteIniParameter(WriteFile, USE_ADJ_BY_RR_FILE, GetParameterString(USE_ADJ_BY_RR_FILE, s), GetParameterComment(USE_ADJ_BY_RR_FILE));
    WriteIniParameter(WriteFile, SPATIAL_ADJ_TYPE, GetParameterString(SPATIAL_ADJ_TYPE, s), GetParameterComment(SPATIAL_ADJ_TYPE));
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteSpaceAndTimeAdjustmentSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Writes parameter settings grouped under 'Spatial Window'. */
void IniParameterFileAccess::WriteSpatialWindowSettings(ZdIniFile& WriteFile) {
  ZdString      s;

  try {
    WriteIniParameter(WriteFile, MAXGEOPOPATRISK, GetParameterString(MAXGEOPOPATRISK, s), GetParameterComment(MAXGEOPOPATRISK));
    WriteIniParameter(WriteFile, MAXGEOPOPFILE, GetParameterString(MAXGEOPOPFILE, s), GetParameterComment(MAXGEOPOPFILE));
    WriteIniParameter(WriteFile, MAXGEODISTANCE, GetParameterString(MAXGEODISTANCE, s), GetParameterComment(MAXGEODISTANCE));
    WriteIniParameter(WriteFile, USE_MAXGEOPOPFILE, GetParameterString(USE_MAXGEOPOPFILE, s), GetParameterComment(USE_MAXGEOPOPFILE));
    WriteIniParameter(WriteFile, USE_MAXGEODISTANCE, GetParameterString(USE_MAXGEODISTANCE, s), GetParameterComment(USE_MAXGEODISTANCE));
    WriteIniParameter(WriteFile, PURETEMPORAL, GetParameterString(PURETEMPORAL, s), GetParameterComment(PURETEMPORAL));
    WriteIniParameter(WriteFile, MAXCIRCLEPOPFILE, GetParameterString(MAXCIRCLEPOPFILE, s), GetParameterComment(MAXCIRCLEPOPFILE));
    WriteIniParameter(WriteFile, WINDOW_SHAPE, GetParameterString(WINDOW_SHAPE, s), GetParameterComment(WINDOW_SHAPE));
    WriteIniParameter(WriteFile, NON_COMPACTNESS_PENALTY, GetParameterString(NON_COMPACTNESS_PENALTY, s), GetParameterComment(NON_COMPACTNESS_PENALTY));
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteSpatialWindowSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Writes parameter settings grouped under '[System]'. */
void IniParameterFileAccess::WriteSystemSettings(ZdIniFile& WriteFile) {
  ZdString      s;

  try {
    WriteIniParameter(WriteFile, CREATION_VERSION, GetParameterString(CREATION_VERSION, s), GetParameterComment(CREATION_VERSION));
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteSystemSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Writes parameter settings grouped under 'Spatial Window'. */
void IniParameterFileAccess::WriteTemporalWindowSettings(ZdIniFile& WriteFile) {
  ZdString      s;

  try {
    WriteIniParameter(WriteFile, TIMESIZE, GetParameterString(TIMESIZE, s), GetParameterComment(TIMESIZE));
    WriteIniParameter(WriteFile, PURESPATIAL, GetParameterString(PURESPATIAL, s), GetParameterComment(PURESPATIAL));
    WriteIniParameter(WriteFile, MAX_TEMPORAL_TYPE, GetParameterString(MAX_TEMPORAL_TYPE, s), GetParameterComment(MAX_TEMPORAL_TYPE));
    WriteIniParameter(WriteFile, CLUSTERS, GetParameterString(CLUSTERS, s), GetParameterComment(CLUSTERS));
    WriteIniParameter(WriteFile, INTERVAL_STARTRANGE, GetParameterString(INTERVAL_STARTRANGE, s), GetParameterComment(INTERVAL_STARTRANGE));
    WriteIniParameter(WriteFile, INTERVAL_ENDRANGE, GetParameterString(INTERVAL_ENDRANGE, s), GetParameterComment(INTERVAL_ENDRANGE));
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteTemporalWindowSettings()","IniParameterFileAccess");
    throw;
  }
}


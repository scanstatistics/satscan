//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop                                                        
//***************************************************************************
#include "IniParameterFileAccess.h"
                                                                       
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

    ZdIniFile SourceFile(sFilename, true, false, ZDIO_OPEN_READ|ZDIO_SREAD);
    gpSpecifications = new IniParameterSpecification(SourceFile);

    gParameters.SetAsDefaulted();
    gParameters.SetSourceFileName(sFilename);

    ReadSystemSettings(SourceFile);

    //read settings as provided in main graphical interface
    ReadInputSettings(SourceFile);
    ReadAnalysisSettings(SourceFile);
    ReadOutputSettings(SourceFile);
    //read settings as provided in advanced features of graphical interface
    ReadMultipleDataSetsSettings(SourceFile);
    ReadSpatialWindowSettings(SourceFile);
    ReadTemporalWindowSettings(SourceFile);
    ReadSpaceAndTimeAdjustmentSettings(SourceFile);
    ReadInferenceSettings(SourceFile);
    ReadClustersReportedSettings(SourceFile);

    //read settings as provided only through user mofication of parameter file and batch executable
    ReadEllipticScanSettings(SourceFile);
    ReadIsotonicScanSettings(SourceFile);
    ReadSequentialScanSettings(SourceFile);
    ReadPowerSimulationsSettings(SourceFile);
    ReadBatchModeFeaturesSettings(SourceFile);
  }
  catch (ZdException &x) {
    x.AddCallpath("Read()","IniParameterFileAccess");
    throw;
  }
  return !gbReadStatusError;
}

/** Reads parameter settings grouped under 'Analysis'. */
void IniParameterFileAccess::ReadAnalysisSettings(const ZdIniFile& SourceFile) {
  try {
    ReadIniParameter(SourceFile, ANALYSISTYPE);
    ReadIniParameter(SourceFile, MODEL);
    ReadIniParameter(SourceFile, SCANAREAS);
    ReadIniParameter(SourceFile, TIME_AGGREGATION_UNITS);
    ReadIniParameter(SourceFile, TIME_AGGREGATION);
    ReadIniParameter(SourceFile, REPLICAS);
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadAnalysisSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Reads parameter settings grouped under '[BatchMode Features]'. */
void IniParameterFileAccess::ReadBatchModeFeaturesSettings(const ZdIniFile& SourceFile) {
  try {
    ReadIniParameter(SourceFile, VALIDATE);
    //ReadIniParameter(SourceFile, TIMETRENDCONVRG); //--- until SVTT is available, don't write
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadBatchModeFeaturesSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Reads parameter settings grouped under 'Clusters Reported'. */
void IniParameterFileAccess::ReadClustersReportedSettings(const ZdIniFile& SourceFile) {
  try {
    ReadIniParameter(SourceFile, CRITERIA_SECOND_CLUSTERS);
    ReadIniParameter(SourceFile, REPORTED_GEOSIZE);
    ReadIniParameter(SourceFile, USE_REPORTED_GEOSIZE);
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadClustersReportedSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Reads parameter settings grouped under '[Elliptic Scan]'. */
void IniParameterFileAccess::ReadEllipticScanSettings(const ZdIniFile& SourceFile) {
  try {
    ReadIniParameter(SourceFile, ELLIPSES);
    ReadIniParameter(SourceFile, ESHAPES);
    ReadIniParameter(SourceFile, ENUMBERS);
    ReadIniParameter(SourceFile, NON_COMPACTNESS_PENALTY);
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadEllipticScanSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Reads parameter settings grouped under 'inference'. */
void IniParameterFileAccess::ReadInferenceSettings(const ZdIniFile& SourceFile) {
  try {
    ReadIniParameter(SourceFile, START_PROSP_SURV);
    ReadIniParameter(SourceFile, EARLY_SIM_TERMINATION);
    ReadIniParameter(SourceFile, ADJ_FOR_EALIER_ANALYSES);
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadInferenceSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Reads parameter settings grouped under '[Isotonic Scan]'. */
void IniParameterFileAccess::ReadIsotonicScanSettings(const ZdIniFile& SourceFile) {
  try {
    ReadIniParameter(SourceFile, RISKFUNCTION);
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadIsotonicScanSettings()","IniParameterFileAccess");
    throw;
  }
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
    multiple data streams. */
std::vector<ZdString>& IniParameterFileAccess::ReadIniParameter(const ZdIniFile& SourceFile, ParameterType eParameterType, std::vector<ZdString>& vParameters) const {
  long                  lSectionIndex, lKeyIndex;
  ZdString              sNextKey;
  const char          * sSectionName, * sKey;
  size_t                iStream=2;

  vParameters.clear();
  if (GetSpecifications().GetMultipleParameterIniInfo(eParameterType, &sSectionName, &sKey)) {
    //read possibly other data stream case source
    if ((lSectionIndex = SourceFile.GetSectionIndex(sSectionName)) > -1) {
      const ZdIniSection  * pSection = SourceFile.GetSection(lSectionIndex);
      sNextKey.printf("%s%i", sKey, iStream);
      while ((lKeyIndex = pSection->FindKey(sNextKey)) > -1) {
           vParameters.push_back(ZdString(pSection->GetLine(lKeyIndex)->GetValue()));
           sNextKey.printf("%s%i", sKey, ++iStream);
      }
    }
  }
  
  return vParameters;
}

/** Reads parameter settings grouped under 'Input'. */
void IniParameterFileAccess::ReadInputSettings(const ZdIniFile& SourceFile) {
  try {
    ReadIniParameter(SourceFile, CASEFILE);
    ReadIniParameter(SourceFile, CONTROLFILE);
    ReadIniParameter(SourceFile, POPFILE);
    ReadIniParameter(SourceFile, COORDFILE);
    ReadIniParameter(SourceFile, SPECIALGRID);
    ReadIniParameter(SourceFile, GRIDFILE);
    ReadIniParameter(SourceFile, PRECISION);
    ReadIniParameter(SourceFile, COORDTYPE);
    ReadIniParameter(SourceFile, STARTDATE);
    ReadIniParameter(SourceFile, ENDDATE);
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadInputSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Reads parameter settings grouped under 'Mutliple Data Sets'. */
void IniParameterFileAccess::ReadMultipleDataSetsSettings(const ZdIniFile& SourceFile) {
  std::vector<ZdString>         vFilenames;
  size_t                        t, iMostStreams=1;

  try {
    ReadIniParameter(SourceFile, MULTI_DATASET_PURPOSE_TYPE);
    ReadIniParameter(SourceFile, CASEFILE, vFilenames);
    for (t=0; t < vFilenames.size(); ++t)
      gParameters.SetCaseFileName(vFilenames[t].GetCString(), false, t + 2);
    iMostStreams = std::max(iMostStreams, vFilenames.size() + 1);
    ReadIniParameter(SourceFile, CONTROLFILE, vFilenames);
    iMostStreams = std::max(iMostStreams, vFilenames.size() + 1);
    for (t=0; t < vFilenames.size(); ++t)
      gParameters.SetControlFileName(vFilenames[t].GetCString(), false, t + 2);
    ReadIniParameter(SourceFile, POPFILE, vFilenames);
    iMostStreams = std::max(iMostStreams, vFilenames.size() + 1);
    for (t=0; t < vFilenames.size(); ++t)
      gParameters.SetPopulationFileName(vFilenames[t].GetCString(), false, t + 2);
    //Synchronize collections of data stream filesnames so that we can ask for
    //any file of a particular stream, even if blank. This keeps the same behavior
    //as when there was only one data stream.
    gParameters.SetNumDataStreams(iMostStreams);
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadMultipleDataSetsSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Reads parameter settings grouped under 'Output'. */
void IniParameterFileAccess::ReadOutputSettings(const ZdIniFile& SourceFile) {
  try {
    ReadIniParameter(SourceFile, OUTPUTFILE);
    ReadIniParameter(SourceFile, OUTPUT_SIM_LLR_ASCII);
    ReadIniParameter(SourceFile, OUTPUT_RR_ASCII);
    ReadIniParameter(SourceFile, OUTPUT_AREAS_ASCII);
    ReadIniParameter(SourceFile, OUTPUT_MLC_ASCII);
    ReadIniParameter(SourceFile, OUTPUT_MLC_DBASE);
    ReadIniParameter(SourceFile, OUTPUT_AREAS_DBASE);
    ReadIniParameter(SourceFile, OUTPUT_RR_DBASE);
    ReadIniParameter(SourceFile, OUTPUT_SIM_LLR_DBASE);
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadOutputSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Reads parameter settings grouped under '[Power Simulations]'. */
void IniParameterFileAccess::ReadPowerSimulationsSettings(const ZdIniFile& SourceFile) {
  try {
    ReadIniParameter(SourceFile, POWERCALC);
    ReadIniParameter(SourceFile, POWERX);
    ReadIniParameter(SourceFile, POWERY);
    ReadIniParameter(SourceFile, SIMULATION_TYPE);
    ReadIniParameter(SourceFile, SIMULATION_SOURCEFILE);
    ReadIniParameter(SourceFile, OUTPUT_SIMULATION_DATA);
    ReadIniParameter(SourceFile, SIMULATION_DATA_OUTFILE);
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadPowerSimulationsSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Reads parameter settings grouped under '[Sequential Scan]. */
void IniParameterFileAccess::ReadSequentialScanSettings(const ZdIniFile& SourceFile) {
  try {
    ReadIniParameter(SourceFile, SEQUENTIAL);
    ReadIniParameter(SourceFile, SEQNUM);
    ReadIniParameter(SourceFile, SEQPVAL);
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadSequentialScanSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Reads parameter settings grouped under 'Space and Time Adjustments'. */
void IniParameterFileAccess::ReadSpaceAndTimeAdjustmentSettings(const ZdIniFile& SourceFile) {
  try {
    ReadIniParameter(SourceFile, TIMETREND);
    ReadIniParameter(SourceFile, TIMETRENDPERC);
    ReadIniParameter(SourceFile, ADJ_BY_RR_FILE);
    ReadIniParameter(SourceFile, USE_ADJ_BY_RR_FILE);
    ReadIniParameter(SourceFile, SPATIAL_ADJ_TYPE);
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadSpaceAndTimeAdjustmentSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Reads parameter settings grouped under 'Spatial Window'. */
void IniParameterFileAccess::ReadSpatialWindowSettings(const ZdIniFile& SourceFile) {
  try {
    ReadIniParameter(SourceFile, GEOSIZE);
    ReadIniParameter(SourceFile, PURETEMPORAL);
    ReadIniParameter(SourceFile, MAX_SPATIAL_TYPE);
    ReadIniParameter(SourceFile, MAXCIRCLEPOPFILE);
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadSpatialWindowSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Reads parameter settings grouped under '[System]'. */
void IniParameterFileAccess::ReadSystemSettings(const ZdIniFile& SourceFile) {
  try {
    ReadIniParameter(SourceFile, CREATION_VERSION);
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadSystemSettings()", "CParameters");
    throw;
  }
}

/** Reads parameter settings grouped under 'Temporal Window'. */
void IniParameterFileAccess::ReadTemporalWindowSettings(const ZdIniFile& SourceFile) {
  try {
    ReadIniParameter(SourceFile, TIMESIZE);
    ReadIniParameter(SourceFile, PURESPATIAL);
    ReadIniParameter(SourceFile, MAX_TEMPORAL_TYPE);
    ReadIniParameter(SourceFile, CLUSTERS);
    ReadIniParameter(SourceFile, INTERVAL_STARTRANGE);
    ReadIniParameter(SourceFile, INTERVAL_ENDRANGE);
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadTemporalWindowSettings()","IniParameterFileAccess");
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
    WriteSpatialWindowSettings(WriteFile);
    WriteTemporalWindowSettings(WriteFile);
    WriteSpaceAndTimeAdjustmentSettings(WriteFile);
    WriteInferenceSettings(WriteFile);
    WriteClustersReportedSettings(WriteFile);
    
    //write settings as provided only through user mofication of parameter file and batch executable
    WriteEllipticScanSettings(WriteFile);
    WriteIsotonicScanSettings(WriteFile);
    WriteSequentialScanSettings(WriteFile);
    WritePowerSimulationsSettings(WriteFile);
    WriteBatchModeFeaturesSettings(WriteFile);
    WriteSystemSettings(WriteFile);

    WriteFile.Write();
  }
  catch (ZdException &x) {
    x.AddCallpath("Write()", "IniParameterFileAccess");
    throw;
  }
}

/** Reads parameter settings grouped under 'Analysis'. */
void IniParameterFileAccess::WriteAnalysisSettings(ZdIniFile& WriteFile) {
  ZdString      s;

  try {
    WriteIniParameter(WriteFile, ANALYSISTYPE, AsString(s, gParameters.GetAnalysisType()),
                      " analysis type (1=Purely Spatial, 2=Purely Temporal, 3=Retrospective"
                      " Space-Time, 4=Prospective Space-Time, 5=N/A, 6=Prospective Purely Temporal)");
    WriteIniParameter(WriteFile, MODEL, AsString(s, gParameters.GetProbabiltyModelType()),
                      " model type (0=Poisson, 1=Bernoulli, 2=Space-Time Permutation)");
    WriteIniParameter(WriteFile, SCANAREAS, AsString(s, gParameters.GetAreaScanRateType()),
                      " scan areas (1=High, 2=Low, 3=High or Low)");
    WriteIniParameter(WriteFile, TIME_AGGREGATION_UNITS, AsString(s, gParameters.GetTimeAggregationUnitsType()),
                      " time aggregation units (0=None, 1=Year, 2=Month, 3=Day)");
    WriteIniParameter(WriteFile, TIME_AGGREGATION, AsString(s, (int)gParameters.GetTimeAggregationLength()),
                      " time aggregation length (positive integer)");
    WriteIniParameter(WriteFile, REPLICAS, AsString(s, gParameters.GetNumReplicationsRequested()),
                      " Monte Carlo replications (0, 9, 999, n999)");
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteAnalysisSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Reads parameter settings grouped under '[BatchMode Features]'. */
void IniParameterFileAccess::WriteBatchModeFeaturesSettings(ZdIniFile& WriteFile) {
  ZdString      s;

  try {
    WriteIniParameter(WriteFile, VALIDATE, AsString(s, gParameters.GetValidatingParameters()),
                      " validate parameters prior to analysis execution? (y/n)");
    //WriteIniParameter(WriteFile, TIMETRENDCONVRG, AsString(s, gParameters.GetTimeTrendConvergence()),
    //                  " time trend convergence for SVTT analysis (> 0)");  //---  until SVTT is available, don't write
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteBatchModeFeaturesSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Reads parameter settings grouped under 'Clusters Reported'. */
void IniParameterFileAccess::WriteClustersReportedSettings(ZdIniFile& WriteFile) {
  ZdString      s;

  try {
    WriteIniParameter(WriteFile, CRITERIA_SECOND_CLUSTERS, AsString(s, gParameters.GetCriteriaSecondClustersType()),
                      " criteria for reporting secondary clusters(0=NoGeoOverlap, 1=NoCentersInOther, 2=NoCentersInMostLikely,"
                      "  3=NoCentersInLessLikely, 4=NoPairsCentersEachOther, 5=NoRestrictions)");
    WriteIniParameter(WriteFile, REPORTED_GEOSIZE, AsString(s, gParameters.GetMaximumReportedGeoClusterSize()),
                      " max reported geographic size (< max geographical cluster size%)");
    WriteIniParameter(WriteFile, USE_REPORTED_GEOSIZE, AsString(s, gParameters.GetRestrictingMaximumReportedGeoClusterSize()),
                      " restrict reported clusters to maximum geographical cluster size? (y/n)");
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteClustersReportedSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Reads parameter settings grouped under '[Elliptic Scan]'. */
void IniParameterFileAccess::WriteEllipticScanSettings(ZdIniFile& WriteFile) {
  ZdString      s;

  try {
    WriteIniParameter(WriteFile, ELLIPSES, AsString(s, gParameters.GetNumRequestedEllipses()),
                      " number of ellipses to scan, other than circle (0-10)");
    s << ZdString::reset;
    for (int i=0; i < gParameters.GetNumRequestedEllipses(); ++i)
       s << (i == 0 ? "" : ",") << gParameters.GetEllipseShapes()[i];
    WriteIniParameter(WriteFile, ESHAPES, s.GetCString(),
                      " elliptic shapes - one value for each ellipse (comma separated decimal values)");
    s << ZdString::reset;
    for (int i=0; i < gParameters.GetNumRequestedEllipses(); ++i)
       s << (i == 0 ? "" : ",") << gParameters.GetEllipseRotations()[i];
    WriteIniParameter(WriteFile, ENUMBERS, s.GetCString(),
                      " elliptic angles - one value for each ellipse (comma separated integer values)");
    WriteIniParameter(WriteFile, NON_COMPACTNESS_PENALTY, AsString(s, gParameters.GetNonCompactnessPenalty()),
                      " elliptic non-compactness penalty? (y/n)");
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteEllipticScanSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Reads parameter settings grouped under 'Inference'. */
void IniParameterFileAccess::WriteInferenceSettings(ZdIniFile& WriteFile) {
  ZdString      s;

  try {
    WriteIniParameter(WriteFile, START_PROSP_SURV, gParameters.GetProspectiveStartDate().c_str(),
                      " prospective surveillance start date (YYYY/MM/DD)");
    WriteIniParameter(WriteFile, EARLY_SIM_TERMINATION, AsString(s, gParameters.GetTerminateSimulationsEarly()),
                      " terminate simulations early for large p-values? (y/n)");
    WriteIniParameter(WriteFile, ADJ_FOR_EALIER_ANALYSES, AsString(s, gParameters.GetAdjustForEarlierAnalyses()),
                      " adjust for earlier analyses(prospective analyses only)? (y/n)");
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

/** Reads parameter settings grouped under '[Isotonic Scan]'. */
void IniParameterFileAccess::WriteIsotonicScanSettings(ZdIniFile& WriteFile) {
  ZdString      s;

  try {
    WriteIniParameter(WriteFile, RISKFUNCTION, AsString(s, gParameters.GetRiskType()), " isotonic scan (0=Standard, 1=Monotone)");
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteIsotonicScanSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Reads parameter settings grouped under 'Input'. */
void IniParameterFileAccess::WriteInputSettings(ZdIniFile& WriteFile) {
  ZdString      s;

  try {
    WriteIniParameter(WriteFile, CASEFILE, gParameters.GetCaseFileName().c_str(), " case data filename");
    WriteIniParameter(WriteFile, CONTROLFILE, gParameters.GetControlFileName().c_str(), " control data filename");
    WriteIniParameter(WriteFile, POPFILE, gParameters.GetPopulationFileName().c_str(), " population data filename");
    WriteIniParameter(WriteFile, COORDFILE, gParameters.GetCoordinatesFileName().c_str(), " coordinate data filename");
    WriteIniParameter(WriteFile, SPECIALGRID, AsString(s, gParameters.UseSpecialGrid()), " use grid file? (y/n)");
    WriteIniParameter(WriteFile, GRIDFILE, gParameters.GetSpecialGridFileName().c_str(), " grid data filename");
    WriteIniParameter(WriteFile, PRECISION, AsString(s, gParameters.GetPrecisionOfTimesType()),
                      " time precision (0=None, 1=Year, 2=Month, 3=Day)");
    WriteIniParameter(WriteFile, COORDTYPE, AsString(s, gParameters.GetCoordinatesType()),
                      " coordinate type (0=Cartesian, 1=latitude/longitude)");
    WriteIniParameter(WriteFile, STARTDATE, gParameters.GetStudyPeriodStartDate().c_str(),
                      " study period start date (YYYY/MM/DD)");
    WriteIniParameter(WriteFile, ENDDATE, gParameters.GetStudyPeriodEndDate().c_str(),
                      " study period end date (YYYY/MM/DD)");
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteInputSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Reads parameter settings grouped under 'Multiple Data Sets'. */
void IniParameterFileAccess::WriteMultipleDataSetsSettings(ZdIniFile& WriteFile) {
  ZdString      s, sComment;
  const char  * sSectionName, * sBaseKey;

  try {
    WriteIniParameter(WriteFile, MULTI_DATASET_PURPOSE_TYPE, AsString(s, gParameters.GetMultipleDataStreamPurposeType()),
                      " multiple data sets purpose type (multivariate=0, adjustment=1)");

    if (GetSpecifications().GetMultipleParameterIniInfo(CASEFILE, &sSectionName, &sBaseKey)) {
      for (size_t t=1; t < gParameters.GetNumDataStreams(); ++t) {
         s.printf("%s%i", sBaseKey, t + 1);
         sComment.printf(" case data filename (additional data set %i)", t + 1);
         WriteIniParameterAsKey(WriteFile, sSectionName, s.GetCString(), gParameters.GetCaseFileName(t + 1).c_str(), sComment.GetCString());
      }
    }
    if (GetSpecifications().GetMultipleParameterIniInfo(CONTROLFILE, &sSectionName, &sBaseKey)) {
      for (size_t t=1; t < gParameters.GetNumDataStreams(); ++t) {
         s.printf("%s%i", sBaseKey, t + 1);
         sComment.printf(" control data filename (additional data set %i)", t + 1);
         WriteIniParameterAsKey(WriteFile, sSectionName, s.GetCString(), gParameters.GetControlFileName(t + 1).c_str(), sComment.GetCString());
      }
    }
    if (GetSpecifications().GetMultipleParameterIniInfo(POPFILE, &sSectionName, &sBaseKey)) {
      for (size_t t=1; t < gParameters.GetNumDataStreams(); ++t) {
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

/** Reads parameter settings grouped under 'Output'. */
void IniParameterFileAccess::WriteOutputSettings(ZdIniFile& WriteFile) {
  ZdString      s;

  try {
    WriteIniParameter(WriteFile, OUTPUTFILE, gParameters.GetOutputFileName().c_str(),
                      " analysis results output filename");
    WriteIniParameter(WriteFile, OUTPUT_SIM_LLR_ASCII, AsString(s, gParameters.GetOutputSimLoglikeliRatiosAscii()),
                      " output simulated log likelihoods ratios in ASCII format? (y/n)");
    WriteIniParameter(WriteFile, OUTPUT_SIM_LLR_DBASE, AsString(s, gParameters.GetOutputSimLoglikeliRatiosDBase()),
                      " output simulated log likelihoods ratios in dBase format? (y/n)");
    WriteIniParameter(WriteFile, OUTPUT_RR_ASCII, AsString(s, gParameters.GetOutputRelativeRisksAscii()),
                      " output relative risks in ASCII format? (y/n)");
    WriteIniParameter(WriteFile, OUTPUT_RR_DBASE, AsString(s, gParameters.GetOutputRelativeRisksDBase()),
                      " output relative risks in dBase format? (y/n)");
    WriteIniParameter(WriteFile, OUTPUT_AREAS_ASCII, AsString(s, gParameters.GetOutputAreaSpecificAscii()),
                      " output location information in ASCII format? (y/n)");
    WriteIniParameter(WriteFile, OUTPUT_AREAS_DBASE, AsString(s, gParameters.GetOutputAreaSpecificDBase()),
                      " output location information in dBase format? (y/n)");
    WriteIniParameter(WriteFile, OUTPUT_MLC_ASCII, AsString(s, gParameters.GetOutputClusterLevelAscii()),
                      " output cluster information in ASCII format? (y/n)");
    WriteIniParameter(WriteFile, OUTPUT_MLC_DBASE, AsString(s, gParameters.GetOutputClusterLevelDBase()),
                      " output cluster information in dBase format? (y/n)");
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteOutputSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Reads parameter settings grouped under '[Power Simulations]'. */
void IniParameterFileAccess::WritePowerSimulationsSettings(ZdIniFile& WriteFile) {
  ZdString      s;

  try {
    WriteIniParameter(WriteFile, POWERCALC, AsString(s, gParameters.GetIsPowerCalculated()),
                      " p-values for 2 pre-specified log likelihood ratios? (y/n)");
    WriteIniParameter(WriteFile, POWERX, AsString(s, gParameters.GetPowerCalculationX()),
                      " power calculation log likelihood ratio (no. 1)");
    WriteIniParameter(WriteFile, POWERY, AsString(s, gParameters.GetPowerCalculationY()),
                      " power calculation log likelihood ratio (no. 2)");
    WriteIniParameter(WriteFile, SIMULATION_TYPE, AsString(s, gParameters.GetSimulationType()),
                      " simulation methods (Null Randomization=0, HA Randomization=1, File Import=2)");
    WriteIniParameter(WriteFile, SIMULATION_SOURCEFILE, gParameters.GetSimulationDataSourceFilename().c_str(),
                      " simulation data input file name (with File Import=2)");
    WriteIniParameter(WriteFile, OUTPUT_SIMULATION_DATA, AsString(s, gParameters.GetOutputSimulationData()),
                      " print simulation data to file? (y/n)");
    WriteIniParameter(WriteFile, SIMULATION_DATA_OUTFILE, gParameters.GetSimulationDataOutputFilename().c_str(),
                      " simulation data output filename");
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteSpaceAndTimeAdjustmentSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Reads parameter settings grouped under '[Sequential Scan]'. */
void IniParameterFileAccess::WriteSequentialScanSettings(ZdIniFile& WriteFile) {
  ZdString      s;

  try {
    WriteIniParameter(WriteFile, SEQUENTIAL, AsString(s, gParameters.GetIsSequentialScanning()),
                     " perform sequential scans? (y/n)");
    WriteIniParameter(WriteFile, SEQNUM, AsString(s, gParameters.GetNumSequentialScansRequested()),
                      " maximum iterations for sequential scan (0-32000)");
    WriteIniParameter(WriteFile, SEQPVAL, AsString(s, gParameters.GetSequentialCutOffPValue()),
                      " max p-value for sequential scan before cutoff (0.000-1.000)");
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteSequentialScanSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Reads parameter settings grouped under 'Spatial Window'. */
void IniParameterFileAccess::WriteSpaceAndTimeAdjustmentSettings(ZdIniFile& WriteFile) {
  ZdString      s;

  try {
    WriteIniParameter(WriteFile, TIMETREND, AsString(s, gParameters.GetTimeTrendAdjustmentType()),
                      " time trend adjustment type (0=None, 1=Nonparametric, 2=LogLinearPercentage,"
                      " 3=CalculatedLogLinearPercentage, 4=TimeStratifiedRandomization)");
    WriteIniParameter(WriteFile, TIMETRENDPERC, AsString(s, gParameters.GetTimeTrendAdjustmentPercentage()),
                      " time trend adjustment percentage (>-100)");
    WriteIniParameter(WriteFile, ADJ_BY_RR_FILE, gParameters.GetAdjustmentsByRelativeRisksFilename().c_str(),
                      " adjustments by known relative risks file name (with HA Randomization=1 or ...)");
    WriteIniParameter(WriteFile, USE_ADJ_BY_RR_FILE, AsString(s, gParameters.UseAdjustmentForRelativeRisksFile()),
                      " use adjustments by known relative risks file? (y/n)");
    WriteIniParameter(WriteFile, SPATIAL_ADJ_TYPE, AsString(s, gParameters.GetSpatialAdjustmentType()),
                      " Spatial Adjustments Type (no spatial adjustment=0, spatially stratified randomization=1)");
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteSpaceAndTimeAdjustmentSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Reads parameter settings grouped under 'Spatial Window'. */
void IniParameterFileAccess::WriteSpatialWindowSettings(ZdIniFile& WriteFile) {
  ZdString      s;

  try {
    WriteIniParameter(WriteFile, GEOSIZE, AsString(s, gParameters.GetMaximumGeographicClusterSize()),
                      " maximum geographic cluster size (<=50%)");
    WriteIniParameter(WriteFile, PURETEMPORAL, AsString(s, gParameters.GetIncludePurelyTemporalClusters()),
                      " include purely temporal clusters? (y/n)");
    WriteIniParameter(WriteFile, MAX_SPATIAL_TYPE, AsString(s, gParameters.GetMaxGeographicClusterSizeType()),
                      " how max spatial size should be interpretted (0=Percentage, 1=Distance, 2=Percentage of max circle population file)");
    WriteIniParameter(WriteFile, MAXCIRCLEPOPFILE, gParameters.GetMaxCirclePopulationFileName().c_str(),
                      " maximum circle size filename");
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteSpatialWindowSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Reads parameter settings grouped under '[System]'. */
void IniParameterFileAccess::WriteSystemSettings(ZdIniFile& WriteFile) {
  ZdString      s;

  try {
    s.printf("%s.%s.%s", VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE);
    WriteIniParameter(WriteFile, CREATION_VERSION, s.GetCString());
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteSystemSettings()","IniParameterFileAccess");
    throw;
  }
}

/** Reads parameter settings grouped under 'Spatial Window'. */
void IniParameterFileAccess::WriteTemporalWindowSettings(ZdIniFile& WriteFile) {
  ZdString      s;

  try {
    WriteIniParameter(WriteFile, TIMESIZE, AsString(s, gParameters.GetMaximumTemporalClusterSize()),
                     " maximum temporal cluster size (<=90%)");
    WriteIniParameter(WriteFile, PURESPATIAL, AsString(s, gParameters.GetIncludePurelySpatialClusters()),
                      " include purely spatial clusters? (y/n)");
    WriteIniParameter(WriteFile, MAX_TEMPORAL_TYPE, AsString(s, gParameters.GetMaximumTemporalClusterSizeType()),
                      " how max temporal size should be interpretted (0=Percentage, 1=Time)");
    WriteIniParameter(WriteFile, CLUSTERS, AsString(s, gParameters.GetIncludeClustersType()),
                      " temporal clusters evaluated (0=All, 1=Alive, 2=Flexible Window)");
    s.printf("%s,%s", gParameters.GetStartRangeStartDate().c_str(), gParameters.GetStartRangeEndDate().c_str());
    WriteIniParameter(WriteFile, INTERVAL_STARTRANGE, s,
                      " flexible temporal window start range (YYYY/MM/DD,YYYY/MM/DD)");
    s.printf("%s,%s", gParameters.GetEndRangeStartDate().c_str(), gParameters.GetEndRangeEndDate().c_str());
    WriteIniParameter(WriteFile, INTERVAL_ENDRANGE, s,
                      " flexible temporal window end range (YYYY/MM/DD,YYYY/MM/DD)");
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteTemporalWindowSettings()","IniParameterFileAccess");
    throw;
  }
}


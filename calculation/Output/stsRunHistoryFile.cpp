//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************

// This class keeps a file log for each run of the SaTScan program which includes information
// specified by the client. For each instance of the class, a new, unnique run number will be
// recorded in the file and the pertinent data will be updated once the analysis is complete.

#include "AbstractDataFileWriter.h"
#include "AnalysisRun.h"
#include "stsRunHistoryFile.h"
#include "Toolkit.h"

const int       OUTPUT_FILE_FIELD_LENGTH        = 254;

const char* RUN_NUMBER_FIELD            = "RUN_NUM";
const char* RUN_TIME_FIELD              = "RUN_TIME";
const char* OUTPUT_FILE_FIELD           = "OUT_FILE";
const char* PROB_MODEL_FIELD            = "PROB_MODEL";
const char* RATES_FIELD                 = "RATES";
const char* COORD_TYPE_FIELD            = "COORD_TYPE";
const char* ANALYSIS_TYPE_FIELD         = "ANALYSIS";
const char* NUM_CASES_FIELD             = "NUM_CASES";
const char* TOTAL_POP_FIELD             = "TOTAL_POP";
const char* NUM_GEO_AREAS_FIELD         = "NUM_AREAS";
const char* PRECISION_TIMES_FIELD       = "TIME_PREC";
const char* MAX_GEO_EXTENT_FIELD        = "GEO_EXTENT";
const char* MAX_TIME_EXTENT_FIELD       = "TEMPOR_EXT";
const char* TIME_TREND_ADJUSTMENT_FIELD = "TREND_ADJ";
const char* COVARIATES_FIELD            = "COVAR";
const char* GRID_FILE_FIELD             = "GRID_FILE";
const char* START_DATE_FIELD            = "START_DATE";
const char* END_DATE_FIELD              = "END_DATE";
const char* ALIVE_ONLY_FIELD            = "ALIVE_ONLY";
const char* INTERVAL_FIELD              = "INTERVAL";
const char* MONTE_CARLO_FIELD           = "MONTECARLO";
const char* CUTOFF_001_FIELD            = "CUTOFF_001";
const char* CUTOFF_005_FIELD            = "CUTOFF_005";
const char* NUM_SIGNIF_005_FIELD        = "SIGNIF_005";
const char* P_VALUE_FIELD               = "P_VALUE";
const char* ADDITIONAL_OUTPUT_FILES_FIELD        = "ADDIT_OUT";

// constructor
stsRunHistoryFile::stsRunHistoryFile(const CParameters& Parameters, BasePrint& PrintDirection)
                  :gpPrintDirection(&PrintDirection) {
  SetFileName(AppToolkit::getToolkit().run_history_filename.string());
  unsigned short   uwOffset(0);     // offset is altered by the CreateNewField function
  AbstractDataFileWriter::CreateField(gvFields, RUN_NUMBER_FIELD, FieldValue::NUMBER_FLD, 8, 0, uwOffset, 0);
  AbstractDataFileWriter::CreateField(gvFields, RUN_TIME_FIELD, FieldValue::ALPHA_FLD, 32, 0, uwOffset, 0);
  AbstractDataFileWriter::CreateField(gvFields, ANALYSIS_TYPE_FIELD, FieldValue::ALPHA_FLD, 32, 0, uwOffset, 0);
  AbstractDataFileWriter::CreateField(gvFields, PROB_MODEL_FIELD, FieldValue::ALPHA_FLD, 32, 0, uwOffset, 0);
  AbstractDataFileWriter::CreateField(gvFields, RATES_FIELD, FieldValue::ALPHA_FLD, 16, 0, uwOffset, 0);
  AbstractDataFileWriter::CreateField(gvFields, GRID_FILE_FIELD, FieldValue::BOOLEAN_FLD, 1, 0, uwOffset, 0);
  AbstractDataFileWriter::CreateField(gvFields, MAX_GEO_EXTENT_FIELD, FieldValue::ALPHA_FLD, 32, 0, uwOffset, 0);
  AbstractDataFileWriter::CreateField(gvFields, MAX_TIME_EXTENT_FIELD, FieldValue::ALPHA_FLD, 32, 0, uwOffset, 0);
  AbstractDataFileWriter::CreateField(gvFields, INTERVAL_FIELD, FieldValue::ALPHA_FLD, 32, 0, uwOffset, 0);
  AbstractDataFileWriter::CreateField(gvFields, ALIVE_ONLY_FIELD, FieldValue::ALPHA_FLD, 8, 0, uwOffset, 0);
  AbstractDataFileWriter::CreateField(gvFields, TIME_TREND_ADJUSTMENT_FIELD, FieldValue::ALPHA_FLD, 20, 0, uwOffset, 0);
  AbstractDataFileWriter::CreateField(gvFields, COVARIATES_FIELD, FieldValue::NUMBER_FLD, 3, 0, uwOffset, 0);
  AbstractDataFileWriter::CreateField(gvFields, MONTE_CARLO_FIELD, FieldValue::NUMBER_FLD, 8, 0, uwOffset, 0);
  AbstractDataFileWriter::CreateField(gvFields, NUM_GEO_AREAS_FIELD, FieldValue::NUMBER_FLD, 8, 0, uwOffset, 0);
  AbstractDataFileWriter::CreateField(gvFields, COORD_TYPE_FIELD, FieldValue::ALPHA_FLD, 16, 0, uwOffset, 0);
  AbstractDataFileWriter::CreateField(gvFields, START_DATE_FIELD, FieldValue::ALPHA_FLD, 16, 0, uwOffset, 0);
  AbstractDataFileWriter::CreateField(gvFields, END_DATE_FIELD, FieldValue::ALPHA_FLD, 16, 0, uwOffset, 0);
  AbstractDataFileWriter::CreateField(gvFields, PRECISION_TIMES_FIELD, FieldValue::ALPHA_FLD, 16, 0, uwOffset, 0);
  AbstractDataFileWriter::CreateField(gvFields, NUM_CASES_FIELD, FieldValue::NUMBER_FLD, 8, 0, uwOffset, 0);
  AbstractDataFileWriter::CreateField(gvFields, TOTAL_POP_FIELD, FieldValue::NUMBER_FLD, 16, 0, uwOffset, 0);
  AbstractDataFileWriter::CreateField(gvFields, P_VALUE_FIELD, FieldValue::NUMBER_FLD, 12, 5, uwOffset, 5);
  AbstractDataFileWriter::CreateField(gvFields, NUM_SIGNIF_005_FIELD, FieldValue::NUMBER_FLD, 8, 0, uwOffset, 0);
  AbstractDataFileWriter::CreateField(gvFields, CUTOFF_001_FIELD, FieldValue::NUMBER_FLD, 8, 3, uwOffset, 3);
  AbstractDataFileWriter::CreateField(gvFields, CUTOFF_005_FIELD, FieldValue::NUMBER_FLD, 8, 3, uwOffset, 3);
  AbstractDataFileWriter::CreateField(gvFields, OUTPUT_FILE_FIELD, FieldValue::ALPHA_FLD, OUTPUT_FILE_FIELD_LENGTH, 0, uwOffset, 0);
  AbstractDataFileWriter::CreateField(gvFields, ADDITIONAL_OUTPUT_FILES_FIELD, FieldValue::ALPHA_FLD, OUTPUT_FILE_FIELD_LENGTH, 0, uwOffset, 0);

  // if we don't have one then create it
  if (access(gsFilename.c_str(), 00))
     CreateRunHistoryFile();
}

// destructor
stsRunHistoryFile::~stsRunHistoryFile() {}

// creates the run history file
// pre: txd file doesn't not already exist
// post: will create the txd file with the appropraite fields
void stsRunHistoryFile::CreateRunHistoryFile() {
  dBaseFile File;
  File.PackFields(gvFields);
  File.Create(gsFilename.c_str(), gvFields);
  File.Close();
}

// converter function to turn the iType into a legible string for printing
// pre :  eAnalysisType is contained in (PURELYSPATIAL, PURELYTEMPORAL, SPACETIME, PROSPECTIVESPACETIME)
// post : string will be assigned a formatted value based on iType
std::string& stsRunHistoryFile::GetAnalysisTypeString(std::string& sTempValue, AnalysisType eAnalysisType) {
  switch(eAnalysisType) {
    case PURELYSPATIAL             : sTempValue = "Purely Spatial"; break;
    case PURELYTEMPORAL            : sTempValue = "Purely Temporal"; break;
    case SPACETIME                 : sTempValue = "Space Time";  break;
    case PROSPECTIVESPACETIME      : sTempValue = "Prospective Space Time"; break;
    case SPATIALVARTEMPTREND       : sTempValue = "Spatial Variation/Temporal Trend"; break;
    case PROSPECTIVEPURELYTEMPORAL : sTempValue = "Prospective Purely Temporal"; break;
    case SEASONALTEMPORAL          : sTempValue = "Seasonal Temporal"; break;
    default : throw prg_error("Invalid analysis type in the run history file.", "stsRunHistoryFile");
  }
  return sTempValue;
}

// converts the iPrecision into a legible string for printing
// pre : 0 <= iPrecision <= 3
// post : string is assigned a formatted value based on iPrecision
std::string& stsRunHistoryFile::GetCasePrecisionString(std::string& sTempValue, int iPrecision) {
  switch (iPrecision) {
     case 0: sTempValue = "None"; break;
     case 1: sTempValue = "Year"; break;
     case 2: sTempValue = "Month"; break;
     case 3: sTempValue = "Day";  break;
     default : throw prg_error("Invalid case time precision in run history file.", "GetCasePrecisionString");
  }
  return sTempValue;
}

// formats include clusters type to string to be written to file
// pre:  eAnalysisType type is an element of (PURELYSPATIAL, PURELYTEMPORAL, ...) enum defined in CParamaters
// post: will return a "n/a" string if PurelySpatial or Prospective SpaceTime analysis, else will return
//       "true" or "false" string
std::string& stsRunHistoryFile::GetIncludeClustersTypeString(std::string& sTempValue, AnalysisType eAnalysisType, IncludeClustersType eIncludeClustersType) {
  if (eAnalysisType == PURELYSPATIAL || eAnalysisType == PROSPECTIVESPACETIME || eAnalysisType == PROSPECTIVEPURELYTEMPORAL)
    sTempValue = "n/a";
  else
    sTempValue = (eIncludeClustersType == ALIVECLUSTERS ? "true" : "false");
  return sTempValue;
}

// basically a converter function which converts the Interval units from the way we store
// them as ints to a legible string to be printed in the file
// pre : 0 <= iUnits <= 3, sTempValue has been allocated
// post: will assign the appropraite value to the string so that it can be printed
std::string& stsRunHistoryFile::GetIntervalUnitsString(std::string& sTempValue, int iUnits, long lLength, AnalysisType eAnalysisType) {
  if (eAnalysisType == PURELYSPATIAL)
    sTempValue = "n/a";
  else {
    printString(sTempValue, "%d", lLength);
    switch (iUnits) {
      case 0: sTempValue += "None"; break;
      case 1: sTempValue += "Year"; break;
      case 2: sTempValue += "Month"; break;
      case 3: sTempValue += "Day"; break;
       default: throw prg_error("Invalid interval units in run history file.", "GetIntervalUnitsString()");
    }
  }
  return sTempValue;
}

// sets up the string to be outputted in max geo extent field
// pre: none
// post: sets sTempValue to the number and units of max geo extent
std::string& stsRunHistoryFile::GetMaxGeoExtentString(std::string& sTempValue, const CParameters& params) {
  if (params.GetAnalysisType() == PURELYTEMPORAL || params.UseLocationNeighborsFile())
     sTempValue = "n/a";
  else {
    if (params.GetAnalysisType() == PROSPECTIVESPACETIME && params.GetAdjustForEarlierAnalyses()) {
      if (params.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false))
        printString(sTempValue, "%.2lf%%", params.GetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false));
      else
        printString(sTempValue, "%.2lf %s", params.GetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false),
                    (params.GetCoordinatesType() == CARTESIAN ? "Cartesian Units" : "Kilometers"));
    } else
      printString(sTempValue, "%.2lf%%", params.GetMaxSpatialSizeForType(PERCENTOFPOPULATION, false));
  }
  return sTempValue;
}

// sets up the string to be outputted in max temporal extent field
// pre: none
// post: sets sTempValue to the number and units of max temporal extent
std::string& stsRunHistoryFile::GetMaxTemporalExtentString(std::string& sTempValue, const CParameters& params) {
  if (params.GetAnalysisType() == PURELYSPATIAL)
    sTempValue = "n/a";
  else {
    printString(sTempValue, "%.2f",  params.GetMaximumTemporalClusterSize());
    sTempValue += " ";
    if (params.GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE)
      sTempValue += "%";
    else {
      if (params.GetTimeAggregationUnitsType() == DAY)
        sTempValue += "Days";
      else if(params.GetTimeAggregationUnitsType() == MONTH)
        sTempValue += "Months";
      else
       sTempValue += "Years";
    }
  }
  return sTempValue;
}

// a converter function to convert the stored int into a legible string to be printed
// pre: eProbabiltyModelType conatined in (POISSON, BERNOULLI, SPACETIMEPERMUTATION) and sTempValue allocated
// post : string will contain the formatted value for printing
std::string& stsRunHistoryFile::GetProbabilityModelString(std::string& sTempValue, ProbabilityModelType eProbabilityModelType) {
  switch(eProbabilityModelType) {
    case POISSON : sTempValue = "Discrete Poisson";  break;
    case BERNOULLI : sTempValue = "Bernoulli";  break;
    case SPACETIMEPERMUTATION : sTempValue = "Space Time Permutation"; break;
    case NORMAL : sTempValue = "Normal"; break;
    case EXPONENTIAL : sTempValue = "Exponential"; break;
    case RANK : sTempValue = "Rank"; break;
    case UNIFORMTIME:  sTempValue = "Uniform Time"; break;
    case CATEGORICAL : sTempValue = "Multinomial"; break;
    case ORDINAL : sTempValue = "Ordinal"; break;
    case BATCHED:  sTempValue = "Batched"; break;
    case HOMOGENEOUSPOISSON : sTempValue = "Continuous Poisson"; break;
    default : throw prg_error("Invalid probability model in the run history file.", "stsRunHistoryFile");
  }
  return sTempValue;
}

// converter function to make a legible string for printing
// pre : eAreaRateType conatined in (HIGH, LOW, HIGHANDLOW) and sTempValue allocated
// post : will assign the appropraite formated value to sTempValue
std::string& stsRunHistoryFile::GetRatesString(std::string& sTempValue, AnalysisType eAnalysisType, AreaRateType eAreaRateType) {
  if (eAnalysisType == SPATIALVARTEMPTREND)
    sTempValue = "n/a";
  else {
    switch (eAreaRateType) {
      case HIGH       : sTempValue = "High"; break;
      case LOW        : sTempValue = "Low"; break;
      case HIGHANDLOW : sTempValue = "Both"; break;
      default : throw prg_error("Invalid rate defined in run history file.", "stsRunHistoryFile");
    }
  }
  return sTempValue;
}

// converts the iType to a legible string for printing
//  pre : iType is conatined in (TEMPORAL_NOTADJUSTED, TEMPORAL_NONPARAMETRIC, LOGLINEAR_PERC, CALCULATED_LOGLINEAR_PERC)
// post : string will be assigned a formatted value based upon iType
std::string& stsRunHistoryFile::GetTimeAdjustmentString(
    std::string& sTempValue, int iType, AnalysisType eAnalysisType, ProbabilityModelType eProbabilityModelType
) {
  if (eProbabilityModelType == SPACETIMEPERMUTATION)
    sTempValue = "n/a";
  else if (eProbabilityModelType == POISSON && eAnalysisType == PURELYSPATIAL)
    sTempValue = "n/a";
  else if (eProbabilityModelType == BERNOULLI && eAnalysisType != SPATIALVARTEMPTREND)
    sTempValue = "n/a";
  else {
    switch(iType) {
      case TEMPORAL_NOTADJUSTED              : sTempValue = "None"; break;
      case TEMPORAL_NONPARAMETRIC            : sTempValue = "Non-parametric"; break;
      case LOGLINEAR_PERC                    : sTempValue = "Linear"; break;
      case CALCULATED_LOGLINEAR_PERC         : sTempValue = "Log Linear"; break;
      case TEMPORAL_STRATIFIED_RANDOMIZATION : sTempValue = "Time Stratified"; break;
      default : throw prg_error("Invalid time trend adjuestment type in run history file.", "stsRunHistoryFile");
    }
  }
  return sTempValue;
}

// although the name implies an oxymoron, this function will record a new run into the history file
// pre: none
// post: records the run history to the file
void stsRunHistoryFile::LogNewHistory(const AnalysisExecution& analysisExecution) {
    std::string buffer;
    const CParameters & params(analysisExecution.getParameters());
    dBaseFile logFile(gsFilename.c_str(),  true);
    std::auto_ptr<dBaseRecord> pRecord(logFile.GetNewRecord());

    //  run number field
    SetDoubleField(*pRecord, (double)(logFile.GetNumRecords() + 1), GetFieldNumber(gvFields, RUN_NUMBER_FIELD));
    // run time and date field
    buffer = ctime(analysisExecution.getStartTime());
    SetStringField(*pRecord, StripCRLF(buffer), GetFieldNumber(gvFields, RUN_TIME_FIELD));
    // output file name field
    buffer = params.GetOutputFileName().c_str();
    SetStringField(*pRecord, StripCRLF(buffer), GetFieldNumber(gvFields, OUTPUT_FILE_FIELD));
    SetAdditionalOutputFileNameString(buffer, params);
    SetStringField(*pRecord, buffer, GetFieldNumber(gvFields, ADDITIONAL_OUTPUT_FILES_FIELD));
    // probability model field
    SetStringField(*pRecord, GetProbabilityModelString(buffer, params.GetProbabilityModelType()), GetFieldNumber(gvFields, PROB_MODEL_FIELD));
    // rates(high, low or both) field
    SetStringField(*pRecord, GetRatesString(buffer, params.GetAnalysisType(), params.GetAreaScanRateType()), GetFieldNumber(gvFields, RATES_FIELD));
    // coordinate type field
    if (!params.UseLocationNeighborsFile() && (params.UseCoordinatesFile() || params.UseSpecialGrid()))
        buffer = ((params.GetCoordinatesType() == CARTESIAN) ? "Cartesian" : "LatLong");
    else
        buffer = "n/a";
    SetStringField(*pRecord, buffer, GetFieldNumber(gvFields, COORD_TYPE_FIELD));
    // analysis type field
    SetStringField(*pRecord, GetAnalysisTypeString(buffer, params.GetAnalysisType()), GetFieldNumber(gvFields, ANALYSIS_TYPE_FIELD));
    SetDoubleField(*pRecord, (double)analysisExecution.getDataHub().GetTotalCases(), GetFieldNumber(gvFields, NUM_CASES_FIELD));
    SetDoubleField(*pRecord, analysisExecution.getDataHub().GetTotalPopulationCount(), GetFieldNumber(gvFields, TOTAL_POP_FIELD));
    SetDoubleField(*pRecord, (double)analysisExecution.getDataHub().GetNumIdentifiers(), GetFieldNumber(gvFields, NUM_GEO_AREAS_FIELD));
    // precision of case times field
    buffer = (params.GetPrecisionOfTimesType() == NONE ? "No" : "Yes");
    SetStringField(*pRecord, buffer, GetFieldNumber(gvFields, PRECISION_TIMES_FIELD));
    //  max geographic extent field
    SetStringField(*pRecord, GetMaxGeoExtentString(buffer, params), GetFieldNumber(gvFields, MAX_GEO_EXTENT_FIELD));
    // max temporal extent field
    SetStringField(*pRecord, GetMaxTemporalExtentString(buffer, params), GetFieldNumber(gvFields, MAX_TIME_EXTENT_FIELD));
    // time trend adjustment field
    SetStringField(*pRecord, 
        GetTimeAdjustmentString(buffer, params.GetTimeTrendAdjustmentType(), params.GetAnalysisType(), params.GetProbabilityModelType()),
        GetFieldNumber(gvFields, TIME_TREND_ADJUSTMENT_FIELD)
    );
    // covariates number
    SetDoubleField(*pRecord,
        (double)analysisExecution.getDataHub().GetDataSetHandler().GetDataSet(0/*for now*/).getPopulationData().GetNumCovariateCategories(),
        GetFieldNumber(gvFields, COVARIATES_FIELD)
    );
    SetBoolField(*pRecord, params.UseSpecialGrid(), GetFieldNumber(gvFields, GRID_FILE_FIELD)); // special grid file used field
    SetStringField(*pRecord, params.GetStudyPeriodStartDate().c_str(), GetFieldNumber(gvFields, START_DATE_FIELD));  // start date field
    SetStringField(*pRecord, params.GetStudyPeriodEndDate().c_str(), GetFieldNumber(gvFields, END_DATE_FIELD)); // end date field
    SetStringField(*pRecord, 
        GetIncludeClustersTypeString(buffer, params.GetAnalysisType(), params.GetIncludeClustersType()), 
        GetFieldNumber(gvFields, ALIVE_ONLY_FIELD)
    );
    // interval field
    SetStringField(*pRecord, 
        GetIntervalUnitsString(buffer, params.GetTimeAggregationUnitsType(), params.GetTimeAggregationUnitsType(), params.GetAnalysisType()), 
        GetFieldNumber(gvFields, INTERVAL_FIELD)
    );
    // p-value field
    bool bPrintPValue = params.GetNumReplicationsRequested() >= MIN_SIMULATION_RPT_PVALUE;
    if (bPrintPValue) {
        double dTopClusterRatio = 0;
        if (analysisExecution.getLargestMaximaClusterCollection().GetNumClustersRetained()) {
            const CCluster & topCluster = analysisExecution.getLargestMaximaClusterCollection().GetTopRankedCluster();
            dTopClusterRatio = topCluster.getReportingPValue(params, analysisExecution.getSimVariables(), true);
        }
        SetDoubleField(*pRecord, dTopClusterRatio, GetFieldNumber(gvFields, P_VALUE_FIELD));
    } else
        pRecord->PutBlank(GetFieldNumber(gvFields, P_VALUE_FIELD));
    SetDoubleField(*pRecord, (double)analysisExecution.getNumSimulationsExecuted(), GetFieldNumber(gvFields, MONTE_CARLO_FIELD));
    if (!params.GetIsIterativeScanning() && bPrintPValue && analysisExecution.getIsCalculatingSignificantRatios()) {
       SetDoubleField(*pRecord, analysisExecution.getSimRatio01(), GetFieldNumber(gvFields, CUTOFF_001_FIELD)); // 0.01 cutoff field
       SetDoubleField(*pRecord, analysisExecution.getSimRatio05(), GetFieldNumber(gvFields, CUTOFF_005_FIELD)); // 0.05 cutoff field
       SetDoubleField(*pRecord, (double)analysisExecution.getNumSignificantAt005(), GetFieldNumber(gvFields, NUM_SIGNIF_005_FIELD));  // number of clusters significant at tthe .05 llr cutoff field
    } else {
        pRecord->PutBlank(GetFieldNumber(gvFields, CUTOFF_001_FIELD));
        pRecord->PutBlank(GetFieldNumber(gvFields, CUTOFF_005_FIELD));
        pRecord->PutBlank(GetFieldNumber(gvFields, NUM_SIGNIF_005_FIELD));
    }
    logFile.DataAppend(*pRecord);
    logFile.Close();
}

// small helper function for replacing the extension of a filename and appending it to the tempstring
// pre : sSourceFileName is the name of the file to replace the extension on
// post : will replace the result output filename's extension with the replacement extension, if an extension
//        doesn't exist on the original filename, then it will just tack on the extension THEN will append the output filename
//        to the tempstring as long as it still fits within the fieldsize space
void stsRunHistoryFile::ReplaceExtensionAndAppend(std::string& sOutputFileNames, const FileName& sSourceFileName, const std::string& sReplacementExtension) {
  std::string  sWorkString(sSourceFileName.getFileName() + sReplacementExtension);

  // if the temp string plus the work string lengths are less than the field width then append the work string
  // to temp string, else just print ',...'
  if ((sOutputFileNames.size() + sWorkString.size()) < (OUTPUT_FILE_FIELD_LENGTH - 5)) {
     sOutputFileNames += (sOutputFileNames.size() > 0 ? ", " : "");
     sOutputFileNames += sWorkString;
  }
  else if(sOutputFileNames.find("...") == sOutputFileNames.npos) {
     sOutputFileNames += (sOutputFileNames.size() > 0 ? "," : "");
     sOutputFileNames += "...";
  }
}

// Creates the string to be outputed as the additional output filename string in the file
// pre : none
// post : sTempValue will contain the names of the additional output files
void stsRunHistoryFile::SetAdditionalOutputFileNameString(std::string& sOutputFileNames, const CParameters& params) {
   FileName   sResultFile(params.GetOutputFileName().c_str());

   sOutputFileNames.clear();
   if (params.GetOutputSimLoglikeliRatiosAscii() && params.GetNumReplicationsRequested())
     ReplaceExtensionAndAppend(sOutputFileNames, sResultFile, ".llr.txt");
   if (params.GetOutputSimLoglikeliRatiosDBase() && params.GetNumReplicationsRequested())
     ReplaceExtensionAndAppend(sOutputFileNames, sResultFile, ".llr.dbf");

   if (params.GetOutputRelativeRisksAscii())
     ReplaceExtensionAndAppend(sOutputFileNames, sResultFile, ".rr.txt");
   if (params.GetOutputRelativeRisksDBase())
     ReplaceExtensionAndAppend(sOutputFileNames, sResultFile, ".rr.dbf");

   if (params.GetOutputAreaSpecificAscii())
     ReplaceExtensionAndAppend(sOutputFileNames, sResultFile, ".gis.txt");
   if (params.GetOutputAreaSpecificDBase())
     ReplaceExtensionAndAppend(sOutputFileNames, sResultFile, ".gis.dbf");

   if (params.GetOutputClusterLevelAscii())
     ReplaceExtensionAndAppend(sOutputFileNames, sResultFile, ".col.txt");
   if (params.GetOutputClusterLevelDBase())
     ReplaceExtensionAndAppend(sOutputFileNames, sResultFile, ".col.dbf");

   if (params.GetOutputClusterCaseAscii())
     ReplaceExtensionAndAppend(sOutputFileNames, sResultFile, ".cci.txt");
   if (params.GetOutputClusterCaseDBase())
     ReplaceExtensionAndAppend(sOutputFileNames, sResultFile, ".cci.dbf");
}

// sets the global filename variable
// pre: none
// post: makes sure the filename has a .dbf extension - will modify the filename if it does not
void stsRunHistoryFile::SetFileName(const std::string& sFileName) {
  FileName   File(sFileName.c_str());
  if (File.getExtension().size() == 0 || stricmp(File.getExtension().c_str(), ".dbf"))
    File.setExtension(".dbf");
  File.getFullPath(gsFilename);
}

// strips the carriage return and line feed off of the string because some File's don't like them embedded in fields
// pre: none
// post: returns the string by reference without the CR or LF
std::string& stsRunHistoryFile::StripCRLF(std::string& sStore) {
  sStore.erase(sStore.find_last_not_of('\n')+1);
  sStore.erase(sStore.find_last_not_of('\r')+1);
  return sStore;
}





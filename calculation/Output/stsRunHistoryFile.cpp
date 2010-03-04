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
#ifdef __BORLANDC__
  #include <syncobjs.hpp>
#endif  

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
  SetFileName(Parameters.GetRunHistoryFilename());
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
void stsRunHistoryFile::GetAnalysisTypeString(std::string& sTempValue, AnalysisType eAnalysisType) {
  switch(eAnalysisType) {
    case PURELYSPATIAL             : sTempValue = "Purely Spatial"; break;
    case PURELYTEMPORAL            : sTempValue = "Purely Temporal"; break;
    case SPACETIME                 : sTempValue = "Space Time";  break;
    case PROSPECTIVESPACETIME      : sTempValue = "Prospective Space Time"; break;
    case SPATIALVARTEMPTREND       : sTempValue = "Spatial Variation/Temporal Trend"; break;
    case PROSPECTIVEPURELYTEMPORAL : sTempValue = "Prospective Purely Temporal"; break;
    default : throw prg_error("Invalid analysis type in the run history file.", "stsRunHistoryFile");
  }
}

// converts the iPrecision into a legible string for printing
// pre : 0 <= iPrecision <= 3
// post : string is assigned a formatted value based on iPrecision
void stsRunHistoryFile::GetCasePrecisionString(std::string& sTempValue, int iPrecision) {
  switch (iPrecision) {
     case 0: sTempValue = "None"; break;
     case 1: sTempValue = "Year"; break;
     case 2: sTempValue = "Month"; break;
     case 3: sTempValue = "Day";  break;
     default : throw prg_error("Invalid case time precision in run history file.", "GetCasePrecisionString");
  }
}

// formats include clusters type to string to be written to file
// pre:  eAnalysisType type is an element of (PURELYSPATIAL, PURELYTEMPORAL, ...) enum defined in CParamaters
// post: will return a "n/a" string if PurelySpatial or Prospective SpaceTime analysis, else will return
//       "true" or "false" string
void stsRunHistoryFile::GetIncludeClustersTypeString(std::string& sTempValue, AnalysisType eAnalysisType, IncludeClustersType eIncludeClustersType) {
  if (eAnalysisType == PURELYSPATIAL || eAnalysisType == PROSPECTIVESPACETIME || eAnalysisType == PROSPECTIVEPURELYTEMPORAL)
    sTempValue = "n/a";
  else
    sTempValue = (eIncludeClustersType == ALIVECLUSTERS ? "true" : "false");
}

// basically a converter function which converts the Interval units from the way we store
// them as ints to a legible string to be printed in the file
// pre : 0 <= iUnits <= 3, sTempValue has been allocated
// post: will assign the appropraite value to the string so that it can be printed
void stsRunHistoryFile::GetIntervalUnitsString(std::string& sTempValue, int iUnits, long lLength, AnalysisType eAnalysisType) {
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
}

// sets up the string to be outputted in max geo extent field
// pre: none
// post: sets sTempValue to the number and units of max geo extent
void stsRunHistoryFile::GetMaxGeoExtentString(std::string& sTempValue, const CParameters& params) {
  if (params.GetAnalysisType() == PURELYTEMPORAL || params.UseLocationNeighborsFile())
     sTempValue = "n/a";
  else {
    if (params.GetAnalysisType() == PROSPECTIVESPACETIME && params.GetAdjustForEarlierAnalyses()) {
      if (params.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false))
        printString(sTempValue, "%.2lf%%", params.GetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false));
      else
        printString(sTempValue, "%.2lf %s", params.GetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false),
                    (params.GetCoordinatesType() == CARTESIAN ? "Cartesian Units" : "Kilometers"));
    }
    else
      printString(sTempValue, "%.2lf%%", params.GetMaxSpatialSizeForType(PERCENTOFPOPULATION, false));
  }
}

// sets up the string to be outputted in max temporal extent field
// pre: none
// post: sets sTempValue to the number and units of max temporal extent
void stsRunHistoryFile::GetMaxTemporalExtentString(std::string& sTempValue, const CParameters& params) {
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
}

// a converter function to convert the stored int into a legible string to be printed
// pre: eProbabiltyModelType conatined in (POISSON, BERNOULLI, SPACETIMEPERMUTATION) and sTempValue allocated
// post : string will contain the formatted value for printing
void stsRunHistoryFile::GetProbabilityModelString(std::string& sTempValue, ProbabilityModelType eProbabilityModelType) {
  switch(eProbabilityModelType) {
    case POISSON : sTempValue = "Discrete Poisson";  break;
    case BERNOULLI : sTempValue = "Bernoulli";  break;
    case SPACETIMEPERMUTATION : sTempValue = "Space Time Permutation"; break;
    case NORMAL : sTempValue = "Normal"; break;
    case EXPONENTIAL : sTempValue = "Exponential"; break;
    case RANK : sTempValue = "Rank"; break;
    case CATEGORICAL : sTempValue = "Multinomial"; break;
    case ORDINAL : sTempValue = "Ordinal"; break;
    case HOMOGENEOUSPOISSON : sTempValue = "Continuous Poisson"; break;
    default : throw prg_error("Invalid probability model in the run history file.", "stsRunHistoryFile");
  }
}

// converter function to make a legible string for printing
// pre : eAreaRateType conatined in (HIGH, LOW, HIGHANDLOW) and sTempValue allocated
// post : will assign the appropraite formated value to sTempValue
void stsRunHistoryFile::GetRatesString(std::string& sTempValue, AnalysisType eAnalysisType, AreaRateType eAreaRateType) {
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
}

// converts the iType to a legible string for printing
//  pre : iType is conatined in (NOTADJUSTED, NONPARAMETRIC, LOGLINEAR_PERC, CALCULATED_LOGLINEAR_PERC)
// post : string will be assigned a formatted value based upon iType
void stsRunHistoryFile::GetTimeAdjustmentString(std::string& sTempValue, int iType, AnalysisType eAnalysisType,
                                                ProbabilityModelType eProbabilityModelType) {
  if (eProbabilityModelType == SPACETIMEPERMUTATION)
    sTempValue = "n/a";
  else if (eProbabilityModelType == POISSON && eAnalysisType == PURELYSPATIAL)
    sTempValue = "n/a";
  else if (eProbabilityModelType == BERNOULLI && eAnalysisType != SPATIALVARTEMPTREND)
    sTempValue = "n/a";
  else {
    switch(iType) {
      case NOTADJUSTED               : sTempValue = "None"; break;
      case NONPARAMETRIC             : sTempValue = "Non-parametric"; break;
      case LOGLINEAR_PERC            : sTempValue = "Linear"; break;
      case CALCULATED_LOGLINEAR_PERC : sTempValue = "Log Linear"; break;
      case STRATIFIED_RANDOMIZATION  : sTempValue = "Time Stratified"; break;
      default : throw prg_error("Invalid time trend adjuestment type in run history file.", "stsRunHistoryFile");
    }
  }
}

// although the name implies an oxymoron, this function will record a new run into the history file
// pre: none
// post: records the run history to the file
void stsRunHistoryFile::LogNewHistory(const AnalysisRunner& AnalysisRun) {
   std::string                  sTempValue, sInterval;
   std::auto_ptr<dBaseFile>     pFile;
   bool                         bFound(false);
   double                       dTopClusterRatio=0;
   
#if defined(__BORLANDC__) && !defined(__BATCH_COMPILE)
      std::auto_ptr<TCriticalSection> pSection(new TCriticalSection());
      pSection->Acquire();
#endif

      const CParameters & params(AnalysisRun.GetDataHub().GetParameters());

      // NOTE: I'm going to document the heck out of this section for two reasons :
      // 1) in case they change the run specs on us at any time
      // 2) to present my assumptions about the output data in case any happen to be incorrect
      // , so bear with me - AJV 9/3/2002

      pFile.reset(new dBaseFile(gsFilename.c_str(),  true));
      std::auto_ptr<dBaseRecord> pRecord(pFile->GetNewRecord());

      // NOTE : ordering in which the data is added does not matter here in this function due to the use of the
      // GetFieldNumber function which finds the appropraite field number for the SetField functions and inserts
      // the data in that field - ordering is determined solely in the CreateRunHistoryFile function by the order the
      // fields are added to the vector in that function

      //  run number field
      SetDoubleField(*pRecord, (double)(pFile->GetNumRecords() + 1), GetFieldNumber(gvFields, RUN_NUMBER_FIELD));

      // run time and date field
      sTempValue = ctime(AnalysisRun.GetStartTime());
      StripCRLF(sTempValue);
      SetStringField(*pRecord, sTempValue, GetFieldNumber(gvFields, RUN_TIME_FIELD));

      // output file name field
      sTempValue = params.GetOutputFileName().c_str();
      StripCRLF(sTempValue);
      SetStringField(*pRecord, sTempValue, GetFieldNumber(gvFields, OUTPUT_FILE_FIELD));
      SetAdditionalOutputFileNameString(sTempValue, params);
      SetStringField(*pRecord, sTempValue, GetFieldNumber(gvFields, ADDITIONAL_OUTPUT_FILES_FIELD));

      // probability model field
      GetProbabilityModelString(sTempValue, params.GetProbabilityModelType());
      SetStringField(*pRecord, sTempValue, GetFieldNumber(gvFields, PROB_MODEL_FIELD));

      // rates(high, low or both) field
      GetRatesString(sTempValue, params.GetAnalysisType(), params.GetAreaScanRateType());
      SetStringField(*pRecord, sTempValue, GetFieldNumber(gvFields, RATES_FIELD));

      // coordinate type field
      if (!params.UseLocationNeighborsFile() && (params.UseCoordinatesFile() || params.UseSpecialGrid()))
        sTempValue = ((params.GetCoordinatesType() == CARTESIAN) ? "Cartesian" : "LatLong");
      else
        sTempValue = "n/a";
      SetStringField(*pRecord, sTempValue, GetFieldNumber(gvFields, COORD_TYPE_FIELD));

      // analysis type field
      GetAnalysisTypeString(sTempValue, params.GetAnalysisType());
      SetStringField(*pRecord, sTempValue, GetFieldNumber(gvFields, ANALYSIS_TYPE_FIELD));

      SetDoubleField(*pRecord, (double)AnalysisRun.GetDataHub().GetTotalCases(), GetFieldNumber(gvFields, NUM_CASES_FIELD));   // total number of cases field
      SetDoubleField(*pRecord, AnalysisRun.GetDataHub().GetTotalPopulationCount(), GetFieldNumber(gvFields, TOTAL_POP_FIELD));  // total population field
      SetDoubleField(*pRecord, (double)AnalysisRun.GetDataHub().GetNumTracts(), GetFieldNumber(gvFields, NUM_GEO_AREAS_FIELD));     // number of geographic areas field

      // precision of case times field
      sTempValue = (params.GetPrecisionOfTimesType() == NONE ? "No" : "Yes");
      SetStringField(*pRecord, sTempValue, GetFieldNumber(gvFields, PRECISION_TIMES_FIELD));

      //  max geographic extent field
      GetMaxGeoExtentString(sTempValue, params);
      SetStringField(*pRecord, sTempValue, GetFieldNumber(gvFields, MAX_GEO_EXTENT_FIELD));

      // max temporal extent field
      GetMaxTemporalExtentString(sTempValue, params);
      SetStringField(*pRecord, sTempValue, GetFieldNumber(gvFields, MAX_TIME_EXTENT_FIELD));

      // time trend adjustment field
      GetTimeAdjustmentString(sTempValue, params.GetTimeTrendAdjustmentType(), params.GetAnalysisType(), params.GetProbabilityModelType());
      SetStringField(*pRecord, sTempValue, GetFieldNumber(gvFields, TIME_TREND_ADJUSTMENT_FIELD));

      // covariates number
      SetDoubleField(*pRecord,
                     (double)AnalysisRun.GetDataHub().GetDataSetHandler().GetDataSet(0/*for now*/).getPopulationData().GetNumCovariateCategories(),
                     GetFieldNumber(gvFields, COVARIATES_FIELD));

      SetBoolField(*pRecord, params.UseSpecialGrid(), GetFieldNumber(gvFields, GRID_FILE_FIELD)); // special grid file used field
      SetStringField(*pRecord, params.GetStudyPeriodStartDate().c_str(), GetFieldNumber(gvFields, START_DATE_FIELD));  // start date field
      SetStringField(*pRecord, params.GetStudyPeriodEndDate().c_str(), GetFieldNumber(gvFields, END_DATE_FIELD)); // end date field

      GetIncludeClustersTypeString(sTempValue, params.GetAnalysisType(), params.GetIncludeClustersType());
      SetStringField(*pRecord, sTempValue, GetFieldNumber(gvFields, ALIVE_ONLY_FIELD)); // alive clusters only field

      // interval field
      GetIntervalUnitsString(sTempValue, params.GetTimeAggregationUnitsType(), params.GetTimeAggregationUnitsType(), params.GetAnalysisType());
      SetStringField(*pRecord, sTempValue, GetFieldNumber(gvFields, INTERVAL_FIELD));

      // p-value field
      bool bPrintPValue = params.GetNumReplicationsRequested() >= MIN_SIMULATION_RPT_PVALUE;
      if(bPrintPValue) {
         if (AnalysisRun.GetClusterContainer().GetNumClustersRetained()) {
            const CCluster & topCluster = AnalysisRun.GetClusterContainer().GetTopRankedCluster();
            double p_value = 0.0;
            if (params.GetPValueReportingType() == GUMBEL_PVALUE) {
                std::pair<double,double> p = topCluster.GetGumbelPValue(AnalysisRun.GetSimVariables());
                p_value = std::max(p.first,p.second);
            } else {
                p_value = topCluster.GetPValue(params, AnalysisRun.GetSimVariables(), true);
            }
            dTopClusterRatio = p_value;
         }
         SetDoubleField(*pRecord, dTopClusterRatio, GetFieldNumber(gvFields, P_VALUE_FIELD));
      }
      else
         pRecord->PutBlank(GetFieldNumber(gvFields, P_VALUE_FIELD));
      SetDoubleField(*pRecord, (double)AnalysisRun.GetNumSimulationsExecuted(), GetFieldNumber(gvFields, MONTE_CARLO_FIELD));  // monte carlo  replications field

      if(!params.GetIsIterativeScanning() && bPrintPValue && AnalysisRun.GetIsCalculatingSignificantRatios()) {    // only print 0.01 and 0.05 cutoffs if pVals are printed, else this would result in access underrun - AJV
         SetDoubleField(*pRecord, AnalysisRun.GetSimRatio01(), GetFieldNumber(gvFields, CUTOFF_001_FIELD)); // 0.01 cutoff field
         SetDoubleField(*pRecord, AnalysisRun.GetSimRatio05(), GetFieldNumber(gvFields, CUTOFF_005_FIELD)); // 0.05 cutoff field
         SetDoubleField(*pRecord, (double)AnalysisRun.GetNumSignificantAt005(), GetFieldNumber(gvFields, NUM_SIGNIF_005_FIELD));  // number of clusters significant at tthe .05 llr cutoff field
      }
      else {
         pRecord->PutBlank(GetFieldNumber(gvFields, CUTOFF_001_FIELD));
         pRecord->PutBlank(GetFieldNumber(gvFields, CUTOFF_005_FIELD));
         pRecord->PutBlank(GetFieldNumber(gvFields, NUM_SIGNIF_005_FIELD));
      }

      pFile->DataAppend(*pRecord);
      pFile->Close();

#if defined(__BORLANDC__) && !defined(__BATCH_COMPILE)
      pSection->Release();
#endif
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
void stsRunHistoryFile::StripCRLF(std::string& sStore) {
  sStore.erase(sStore.find_last_not_of('\n')+1);
  sStore.erase(sStore.find_last_not_of('\r')+1);
}





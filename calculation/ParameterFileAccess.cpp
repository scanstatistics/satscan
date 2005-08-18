//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "ParameterFileAccess.h"
#include "ScanLineParameterFileAccess.h"
#include "IniParameterFileAccess.h"

/** constructor */
ParameterAccessCoordinator::ParameterAccessCoordinator(CParameters& Parameters)
                           :gParameters(Parameters) {}

/** destructor */
ParameterAccessCoordinator::~ParameterAccessCoordinator() {}

/** Determines format of parameter file and invokes particular parameter reader
    class to read parameters from file. */
bool ParameterAccessCoordinator::Read(const char* sFilename, BasePrint& PrintDirection) {
  bool  bSuccess=false;
  try {
    if (access(sFilename, 04) == -1)
      GenerateResolvableException("Unable to open parameter file '%s'.\n", "Read()", sFilename);

    if (ZdIniFile(sFilename, true, false).GetNumSections())
      bSuccess = IniParameterFileAccess(gParameters, PrintDirection).Read(sFilename);
    else
      bSuccess = ScanLineParameterFileAccess(gParameters, PrintDirection).Read(sFilename);
  }
  catch (ZdException &x) {
    GenerateResolvableException("Unable to read parameters from file '%s'.\n", "Read()", sFilename);
  }
  return bSuccess;
}

/** Writes parameters to ini file in most recent format. */
void ParameterAccessCoordinator::Write(const char * sFilename, BasePrint& PrintDirection /* ability to specify a version to write as ?*/) {
  try {
   IniParameterFileAccess(gParameters, PrintDirection).Write(sFilename);
   //ScanLineParameterFileAccess(gParameters, PrintDirection).Write(sFilename);
  }
  catch (ZdException &x) {
    x.AddCallpath("Write()","ParameterAccessCoordinator");
    throw;
  }
}


/** constructor */
AbtractParameterFileAccess::AbtractParameterFileAccess(CParameters& Parameters, BasePrint& PrintDirection)
                           :gParameters(Parameters), gPrintDirection(PrintDirection) {}

/** destructor */
AbtractParameterFileAccess::~AbtractParameterFileAccess() {}

/** Returns constant char pointer to parameters comment string. */
const char * AbtractParameterFileAccess::GetParameterComment(ParameterType eParameterType) const {
  try {
    switch (eParameterType) {
      case ANALYSISTYPE             : return " analysis type (1=Purely Spatial, 2=Purely Temporal, 3=Retrospective Space-Time, 4=Prospective Space-Time, 5=N/A, 6=Prospective Purely Temporal)";
      case SCANAREAS                : return " scan areas (1=High, 2=Low, 3=High or Low)";
      case CASEFILE                 : return " case data filename";
      case POPFILE                  : return " population data filename";
      case COORDFILE                : return " coordinate data filename";
      case OUTPUTFILE               : return " analysis results output filename";
      case PRECISION                : return " time precision (0=None, 1=Year, 2=Month, 3=Day)";
      case DIMENSION                : return " n/a";
      case SPECIALGRID              : return " use grid file? (y/n)";
      case GRIDFILE                 : return " grid data filename";
      case GEOSIZE                  : return " maximum geographic cluster size (<=50%)";
      case STARTDATE                : return " study period start date (YYYY/MM/DD)";
      case ENDDATE                  : return " study period end date (YYYY/MM/DD)";
      case CLUSTERS                 : return " temporal clusters evaluated (0=All, 1=Alive, 2=Flexible Window)";
      case EXACTTIMES               : return " n/a";
      case TIME_AGGREGATION_UNITS   : return " time aggregation units (0=None, 1=Year, 2=Month, 3=Day)";
      case TIME_AGGREGATION         : return " time aggregation length (positive integer)";
      case PURESPATIAL              : return " include purely spatial clusters? (y/n)";
      case TIMESIZE                 : return " maximum temporal cluster size (<=90%)";
      case REPLICAS                 : return " Monte Carlo replications (0, 9, 999, n999)";
      case MODEL                    : return " model type (0=Poisson, 1=Bernoulli, 2=Space-Time Permutation, 3=Ordinal, 4=Exponential)";
      case RISKFUNCTION             : return " isotonic scan (0=Standard, 1=Monotone)";
      case POWERCALC                : return " p-values for 2 pre-specified log likelihood ratios? (y/n)";
      case POWERX                   : return " power calculation log likelihood ratio (no. 1)";
      case POWERY                   : return " power calculation log likelihood ratio (no. 2)";
      case TIMETREND                : return " time trend adjustment type (0=None, 1=Nonparametric, 2=LogLinearPercentage, 3=CalculatedLogLinearPercentage, 4=TimeStratifiedRandomization)";
      case TIMETRENDPERC            : return " time trend adjustment percentage (>-100)";
      case PURETEMPORAL             : return " include purely temporal clusters? (y/n)";
      case CONTROLFILE              : return " control data filename";
      case COORDTYPE                : return " coordinate type (0=Cartesian, 1=latitude/longitude)";
      case OUTPUT_SIM_LLR_ASCII     : return " output simulated log likelihoods ratios in ASCII format? (y/n)";
      case SEQUENTIAL               : return " perform sequential scans? (y/n)";
      case SEQNUM                   : return " maximum iterations for sequential scan (0-32000)";
      case SEQPVAL                  : return " max p-value for sequential scan before cutoff (0.000-1.000)";
      case VALIDATE                 : return " validate parameters prior to analysis execution? (y/n)";
      case OUTPUT_RR_ASCII          : return " output relative risks in ASCII format? (y/n)";
      case ELLIPSES                 : return " number of ellipses to scan, other than circle (0-10)";
      case ESHAPES                  : return " elliptic shapes - one value for each ellipse (comma separated decimal values)";
      case ENUMBERS                 : return " elliptic angles - one value for each ellipse (comma separated integer values)";
      case START_PROSP_SURV         : return " prospective surveillance start date (YYYY/MM/DD)";
      case OUTPUT_AREAS_ASCII       : return " output location information in ASCII format? (y/n)";
      case OUTPUT_MLC_ASCII         : return " output cluster information in ASCII format? (y/n)";
      case CRITERIA_SECOND_CLUSTERS : return " criteria for reporting secondary clusters(0=NoGeoOverlap, 1=NoCentersInOther, 2=NoCentersInMostLikely,  3=NoCentersInLessLikely, 4=NoPairsCentersEachOther, 5=NoRestrictions)";
      case MAX_TEMPORAL_TYPE        : return " how max temporal size should be interpretted (0=Percentage, 1=Time)";
      case MAX_SPATIAL_TYPE         : return " how max spatial size should be interpretted (0=Percentage, 1=Distance, 2=Percentage of max circle population file)";
      case RUN_HISTORY_FILENAME     : return " n/a";
      case OUTPUT_MLC_DBASE         : return " output cluster information in dBase format? (y/n)";
      case OUTPUT_AREAS_DBASE       : return " output location information in dBase format? (y/n)";
      case OUTPUT_RR_DBASE          : return " output relative risks in dBase format? (y/n)";
      case OUTPUT_SIM_LLR_DBASE     : return " output simulated log likelihoods ratios in dBase format? (y/n)";
      case NON_COMPACTNESS_PENALTY  : return " elliptic non-compactness penalty? (y/n)";
      case INTERVAL_STARTRANGE      : return " flexible temporal window start range (YYYY/MM/DD,YYYY/MM/DD)";
      case INTERVAL_ENDRANGE        : return " flexible temporal window end range (YYYY/MM/DD,YYYY/MM/DD)";
      case TIMETRENDCONVRG	    : return " time trend convergence for SVTT analysis (> 0)";
      case MAXCIRCLEPOPFILE         : return " maximum circle size filename";
      case EARLY_SIM_TERMINATION    : return " terminate simulations early for large p-values? (y/n)";
      case REPORTED_GEOSIZE         : return " max reported geographic size (< max geographical cluster size%)";
      case USE_REPORTED_GEOSIZE     : return " restrict reported clusters to maximum geographical cluster size? (y/n)";
      case SIMULATION_TYPE          : return " simulation methods (Null Randomization=0, HA Randomization=1, File Import=2)";
      case SIMULATION_SOURCEFILE    : return " simulation data input file name (with File Import=2)";
      case ADJ_BY_RR_FILE           : return " adjustments by known relative risks file name (with HA Randomization=1 or ...)";
      case OUTPUT_SIMULATION_DATA   : return " print simulation data to file? (y/n)";
      case SIMULATION_DATA_OUTFILE  : return " simulation data output filename";
      case ADJ_FOR_EALIER_ANALYSES  : return " adjust for earlier analyses(prospective analyses only)? (y/n)";
      case USE_ADJ_BY_RR_FILE       : return " use adjustments by known relative risks file? (y/n)";
      case SPATIAL_ADJ_TYPE         : return " Spatial Adjustments Type (no spatial adjustment=0, spatially stratified randomization=1)";
      case MULTI_DATASET_PURPOSE_TYPE : return " multiple data sets purpose type (multivariate=0, adjustment=1)";
      case CREATION_VERSION         : return " system setting - do not modify";
      case RANDOMIZATION_SEED       : return " randomization seed (0 < Seed < 2147483647)";
      case REPORT_CRITICAL_VALUES   : return " report critical values for .01 and .05? (y/n)";
      case EXECUTION_TYPE           : return " analysis execution method  (Automatic=0, Successively=1, Centrically=2)";
      case NUM_PROCESSES            : return " number of parallel processes to execute (All Processors=0, At Most X Processors=x)";
      case LOG_HISTORY              : return " log analysis run to history file? (y/n)";
      default : ZdGenerateException("Unknown parameter enumeration %d.","GetParameterComment()", eParameterType);
    };
  }
  catch (ZdException & x) {
    x.AddCallpath("GetParameterComment()","AbtractParameterFileAccess");
    throw;
 }
 return 0;
}

/** Assigns string representation to passed string class for parameter. */
ZdString & AbtractParameterFileAccess::GetParameterString(ParameterType eParameterType, ZdString& s) const {
  try {
    switch (eParameterType) {
      case ANALYSISTYPE             : return AsString(s, gParameters.GetAnalysisType());
      case SCANAREAS                : return AsString(s, gParameters.GetAreaScanRateType());
      case CASEFILE                 : s = gParameters.GetCaseFileName().c_str(); return s;
      case POPFILE                  : s = gParameters.GetPopulationFileName().c_str(); return s;
      case COORDFILE                : s = gParameters.GetCoordinatesFileName().c_str(); return s;
      case OUTPUTFILE               : s = gParameters.GetOutputFileName().c_str(); return s;
      case PRECISION                : return AsString(s, gParameters.GetPrecisionOfTimesType());
      case DIMENSION                : s = " n/a"; return s;
      case SPECIALGRID              : return AsString(s, gParameters.UseSpecialGrid());
      case GRIDFILE                 : s = gParameters.GetSpecialGridFileName().c_str(); return s;
      case GEOSIZE                  : return AsString(s, gParameters.GetMaximumGeographicClusterSize());
      case STARTDATE                : s = gParameters.GetStudyPeriodStartDate().c_str(); return s;
      case ENDDATE                  : s = gParameters.GetStudyPeriodEndDate().c_str(); return s;
      case CLUSTERS                 : return AsString(s, gParameters.GetIncludeClustersType());
      case EXACTTIMES               : s = " n/a"; return s;
      case TIME_AGGREGATION_UNITS   : return AsString(s, gParameters.GetTimeAggregationUnitsType());
      case TIME_AGGREGATION         : return AsString(s, (int)gParameters.GetTimeAggregationLength());
      case PURESPATIAL              : return AsString(s, gParameters.GetIncludePurelySpatialClusters());
      case TIMESIZE                 : return AsString(s, gParameters.GetMaximumTemporalClusterSize());
      case REPLICAS                 : return AsString(s, gParameters.GetNumReplicationsRequested());
      case MODEL                    : return AsString(s, gParameters.GetProbabilityModelType());
      case RISKFUNCTION             : return AsString(s, gParameters.GetRiskType());
      case POWERCALC                : return AsString(s, gParameters.GetIsPowerCalculated());
      case POWERX                   : return AsString(s, gParameters.GetPowerCalculationX());
      case POWERY                   : return AsString(s, gParameters.GetPowerCalculationY());
      case TIMETREND                : return AsString(s, gParameters.GetTimeTrendAdjustmentType());
      case TIMETRENDPERC            : return AsString(s, gParameters.GetTimeTrendAdjustmentPercentage());
      case PURETEMPORAL             : return AsString(s, gParameters.GetIncludePurelyTemporalClusters());
      case CONTROLFILE              : s = gParameters.GetControlFileName().c_str(); return s;
      case COORDTYPE                : return AsString(s, gParameters.GetCoordinatesType());
      case OUTPUT_SIM_LLR_ASCII     : return AsString(s, gParameters.GetOutputSimLoglikeliRatiosAscii());
      case SEQUENTIAL               : return AsString(s, gParameters.GetIsSequentialScanning());
      case SEQNUM                   : return AsString(s, gParameters.GetNumSequentialScansRequested());
      case SEQPVAL                  : return AsString(s, gParameters.GetSequentialCutOffPValue());
      case VALIDATE                 : return AsString(s, gParameters.GetValidatingParameters());
      case OUTPUT_RR_ASCII          : return AsString(s, gParameters.GetOutputRelativeRisksAscii());
      case ELLIPSES                 : return AsString(s, gParameters.GetNumRequestedEllipses());
      case ESHAPES                  : s << ZdString::reset;
                                      for (int i=0; i < gParameters.GetNumRequestedEllipses(); ++i)
                                         s << (i == 0 ? "" : ",") << gParameters.GetEllipseShapes()[i];
                                      return s;
      case ENUMBERS                 :  s << ZdString::reset;
                                      for (int i=0; i < gParameters.GetNumRequestedEllipses(); ++i)
                                         s << (i == 0 ? "" : ",") << gParameters.GetEllipseRotations()[i];
                                      return s;
      case START_PROSP_SURV         : s = gParameters.GetProspectiveStartDate().c_str(); return s;
      case OUTPUT_AREAS_ASCII       : return AsString(s, gParameters.GetOutputAreaSpecificAscii());
      case OUTPUT_MLC_ASCII         : return AsString(s, gParameters.GetOutputAreaSpecificAscii());
      case CRITERIA_SECOND_CLUSTERS : return AsString(s, gParameters.GetCriteriaSecondClustersType());
      case MAX_TEMPORAL_TYPE        : return AsString(s, gParameters.GetMaximumTemporalClusterSizeType());
      case MAX_SPATIAL_TYPE         : return AsString(s, gParameters.GetMaxGeographicClusterSizeType());
      case RUN_HISTORY_FILENAME     : s = " n/a"; return s;
      case OUTPUT_MLC_DBASE         : return AsString(s, gParameters.GetOutputClusterLevelDBase());
      case OUTPUT_AREAS_DBASE       : return AsString(s, gParameters.GetOutputAreaSpecificDBase());
      case OUTPUT_RR_DBASE          : return AsString(s, gParameters.GetOutputRelativeRisksDBase());
      case OUTPUT_SIM_LLR_DBASE     : return AsString(s, gParameters.GetOutputSimLoglikeliRatiosDBase());
      case NON_COMPACTNESS_PENALTY  : return AsString(s, gParameters.GetNonCompactnessPenalty());
      case INTERVAL_STARTRANGE      : s.printf("%s,%s", gParameters.GetStartRangeStartDate().c_str(), gParameters.GetStartRangeEndDate().c_str());
                                      return s;
      case INTERVAL_ENDRANGE        : s.printf("%s,%s", gParameters.GetEndRangeStartDate().c_str(), gParameters.GetEndRangeEndDate().c_str());
                                      return s;
      case TIMETRENDCONVRG	    : return AsString(s, gParameters.GetTimeTrendConvergence());
      case MAXCIRCLEPOPFILE         : s = gParameters.GetMaxCirclePopulationFileName().c_str(); return s;
      case EARLY_SIM_TERMINATION    : return AsString(s, gParameters.GetTerminateSimulationsEarly());
      case REPORTED_GEOSIZE         : return AsString(s, gParameters.GetMaximumReportedGeoClusterSize());
      case USE_REPORTED_GEOSIZE     : return AsString(s, gParameters.GetRestrictingMaximumReportedGeoClusterSize());
      case SIMULATION_TYPE          : return AsString(s, gParameters.GetSimulationType());
      case SIMULATION_SOURCEFILE    : s = gParameters.GetSimulationDataSourceFilename().c_str(); return s;
      case ADJ_BY_RR_FILE           : s = gParameters.GetAdjustmentsByRelativeRisksFilename().c_str(); return s;
      case OUTPUT_SIMULATION_DATA   : return AsString(s, gParameters.GetOutputSimulationData());
      case SIMULATION_DATA_OUTFILE  : s = gParameters.GetSimulationDataOutputFilename().c_str(); return s;
      case ADJ_FOR_EALIER_ANALYSES  : return AsString(s, gParameters.GetAdjustForEarlierAnalyses());
      case USE_ADJ_BY_RR_FILE       : return AsString(s, gParameters.UseAdjustmentForRelativeRisksFile());
      case SPATIAL_ADJ_TYPE         : return AsString(s, gParameters.GetSpatialAdjustmentType());
      case MULTI_DATASET_PURPOSE_TYPE : return AsString(s, gParameters.GetMultipleDataSetPurposeType());
      case CREATION_VERSION         : s.printf("%s.%s.%s", VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE); return s;
      case RANDOMIZATION_SEED       : return AsString(s, (int)gParameters.GetRandomizationSeed());
      case REPORT_CRITICAL_VALUES   : return AsString(s, gParameters.GetReportCriticalValues());
      case EXECUTION_TYPE           : return AsString(s, gParameters.GetExecutionType());
      case NUM_PROCESSES            : return AsString(s, gParameters.GetNumRequestedParallelProcesses());
      case LOG_HISTORY              : return AsString(s, gParameters.GetIsLoggingHistory());
      default : ZdGenerateException("Unknown parameter enumeration %d.","GetParameterComment()", eParameterType);
    };
  }
  catch (ZdException & x) {
    x.AddCallpath("GetParameterString()","AbtractParameterFileAccess");
    throw;
 }
 return s;
}

/** Prints message to print direction that parameter was missing when read from
    parameter file and that a default value as assigned. */
void AbtractParameterFileAccess::MarkAsMissingDefaulted(ParameterType eParameterType, BasePrint& PrintDirection) {
  ZdString      sDefaultValue, sParameterLineLabel;

  try {
    switch (eParameterType) {
      case ANALYSISTYPE             : sDefaultValue = gParameters.GetAnalysisType(); break;
      case SCANAREAS                : sDefaultValue = gParameters.GetAreaScanRateType(); break;
      case CASEFILE                 : sDefaultValue = "<blank>"; break;
      case POPFILE                  : sDefaultValue = "<blank>"; break;
      case COORDFILE                : sDefaultValue = "<blank>"; break;
      case OUTPUTFILE               : sDefaultValue = "<blank>"; break;
      case PRECISION                : sDefaultValue = gParameters.GetPrecisionOfTimesType(); break;
      case DIMENSION                : /*  */ break;
      case SPECIALGRID              : sDefaultValue = (gParameters.UseSpecialGrid() ? "y" : "n"); break;
      case GRIDFILE                 : sDefaultValue = "<blank>"; break;
      case GEOSIZE                  : sDefaultValue = gParameters.GetMaximumGeographicClusterSize(); break;
      case STARTDATE                : sDefaultValue = gParameters.GetStudyPeriodStartDate().c_str(); break;
      case ENDDATE                  : sDefaultValue = gParameters.GetStudyPeriodEndDate().c_str(); break;
      case CLUSTERS                 : sDefaultValue = gParameters.GetIncludeClustersType(); break;
      case EXACTTIMES               : /* no longer used */ break;
      case TIME_AGGREGATION_UNITS   : sDefaultValue = gParameters.GetTimeAggregationUnitsType(); break;
      case TIME_AGGREGATION         : sDefaultValue = gParameters.GetTimeAggregationLength(); break;
      case PURESPATIAL              : sDefaultValue = (gParameters.GetIncludePurelySpatialClusters() ? "y" : "n"); break;
      case TIMESIZE                 : sDefaultValue = gParameters.GetMaximumTemporalClusterSize(); break;
      case REPLICAS                 : sDefaultValue << gParameters.GetNumReplicationsRequested(); break;
      case MODEL                    : sDefaultValue = gParameters.GetProbabilityModelType(); break;
      case RISKFUNCTION             : sDefaultValue = gParameters.GetRiskType(); break;
      case POWERCALC                : sDefaultValue = (gParameters.GetIsPowerCalculated() ? "y" : "n"); break;
      case POWERX                   : sDefaultValue = gParameters.GetPowerCalculationX(); break;
      case POWERY                   : sDefaultValue = gParameters.GetPowerCalculationY(); break;
      case TIMETREND                : sDefaultValue = gParameters.GetTimeTrendAdjustmentType(); break;
      case TIMETRENDPERC            : sDefaultValue = gParameters.GetTimeTrendAdjustmentPercentage(); break;
      case PURETEMPORAL             : sDefaultValue = (gParameters.GetIncludePurelyTemporalClusters() ? "y" : "n"); break;
      case CONTROLFILE              : sDefaultValue = "<blank>"; break;
      case COORDTYPE                : sDefaultValue = gParameters.GetCoordinatesType(); break;
      case OUTPUT_SIM_LLR_ASCII     : sDefaultValue = (gParameters.GetOutputSimLoglikeliRatiosAscii() ? "y" : "n"); break;
      case SEQUENTIAL               : sDefaultValue = (gParameters.GetIsSequentialScanning() ? "y" : "n"); break;
      case SEQNUM                   : sDefaultValue << gParameters.GetNumSequentialScansRequested(); break;
      case SEQPVAL                  : sDefaultValue = gParameters.GetSequentialCutOffPValue(); break;
      case VALIDATE                 : sDefaultValue = (gParameters.GetValidatingParameters() ? "y" : "n"); break;
      case OUTPUT_RR_ASCII          : sDefaultValue = (gParameters.GetOutputRelativeRisksAscii() ? "y" : "n"); break;
      case ELLIPSES                 : sDefaultValue = gParameters.GetNumRequestedEllipses(); break;
      case ESHAPES                  : sDefaultValue = "<blank>"; break;
      case ENUMBERS                 : sDefaultValue = "<blank>"; break;
      case START_PROSP_SURV         : sDefaultValue = gParameters.GetProspectiveStartDate().c_str(); break;
      case OUTPUT_AREAS_ASCII       : sDefaultValue = (gParameters.GetOutputAreaSpecificAscii() ? "y" : "n"); break;
      case OUTPUT_MLC_ASCII         : sDefaultValue = (gParameters.GetOutputClusterLevelAscii() ? "y" : "n"); break;
      case CRITERIA_SECOND_CLUSTERS : sDefaultValue = gParameters.GetCriteriaSecondClustersType(); break;
      case MAX_TEMPORAL_TYPE        : sDefaultValue = gParameters.GetMaximumTemporalClusterSizeType(); break;
      case MAX_SPATIAL_TYPE         : sDefaultValue = gParameters.GetMaxGeographicClusterSizeType(); break;
      case RUN_HISTORY_FILENAME     : /* no longer read in from parameter file */ break;
      case OUTPUT_MLC_DBASE         : sDefaultValue = (gParameters.GetOutputClusterLevelDBase() ? "y" : "n"); break;
      case OUTPUT_AREAS_DBASE       : sDefaultValue = (gParameters.GetOutputAreaSpecificDBase() ? "y" : "n"); break;
      case OUTPUT_RR_DBASE          : sDefaultValue = (gParameters.GetOutputRelativeRisksDBase() ? "y" : "n"); break;
      case OUTPUT_SIM_LLR_DBASE     : sDefaultValue = (gParameters.GetOutputSimLoglikeliRatiosDBase() ? "y" : "n"); break;
      case NON_COMPACTNESS_PENALTY  : sDefaultValue = (gParameters.GetNonCompactnessPenalty() ? "y" : "n"); break;
      case INTERVAL_STARTRANGE      : sDefaultValue.printf("%s,%s", gParameters.GetStartRangeStartDate().c_str(), gParameters.GetStartRangeEndDate().c_str());
                                      break;
      case INTERVAL_ENDRANGE        : sDefaultValue.printf("%s,%s", gParameters.GetEndRangeStartDate().c_str(), gParameters.GetEndRangeEndDate().c_str());
                                      break;
      case TIMETRENDCONVRG	    : sDefaultValue = gParameters.GetTimeTrendConvergence(); break;
      case MAXCIRCLEPOPFILE         : sDefaultValue = "<blank>"; break;
      case EARLY_SIM_TERMINATION    : sDefaultValue = (gParameters.GetTerminateSimulationsEarly() ? "y" : "n"); break;
      case REPORTED_GEOSIZE         : sDefaultValue = gParameters.GetMaximumReportedGeoClusterSize(); break;
      case USE_REPORTED_GEOSIZE     : sDefaultValue = (gParameters.GetRestrictingMaximumReportedGeoClusterSize() ? "y" : "n"); break;
      case SIMULATION_TYPE          : sDefaultValue = gParameters.GetSimulationType(); break;
      case SIMULATION_SOURCEFILE    : sDefaultValue = "<blank>"; break;
      case ADJ_BY_RR_FILE           : sDefaultValue = "<blank>"; break;
      case OUTPUT_SIMULATION_DATA   : sDefaultValue = (gParameters.GetOutputSimulationData() ? "y" : "n"); break;
      case SIMULATION_DATA_OUTFILE  : sDefaultValue = "<blank>"; break;
      case ADJ_FOR_EALIER_ANALYSES  : sDefaultValue = (gParameters.GetAdjustForEarlierAnalyses() ? "y" : "n"); break;
      case USE_ADJ_BY_RR_FILE       : sDefaultValue = (gParameters.UseAdjustmentForRelativeRisksFile() ? "y" : "n"); break;
      case SPATIAL_ADJ_TYPE         : sDefaultValue = gParameters.GetSpatialAdjustmentType(); break;
      case MULTI_DATASET_PURPOSE_TYPE : sDefaultValue = gParameters.GetMultipleDataSetPurposeType(); break;
      case CREATION_VERSION         : sDefaultValue.printf("%u.%u.%u", gParameters.GetCreationVersion().iMajor,
                                                           gParameters.GetCreationVersion().iMinor, gParameters.GetCreationVersion().iRelease); break;
      case RANDOMIZATION_SEED       : break; //this parameter is not advertised
      case REPORT_CRITICAL_VALUES   : sDefaultValue = (gParameters.GetReportCriticalValues() ? "y" : "n"); break;
      case EXECUTION_TYPE           : sDefaultValue = gParameters.GetExecutionType(); break;
      case NUM_PROCESSES            : sDefaultValue << gParameters.GetNumRequestedParallelProcesses(); break;
      case LOG_HISTORY              : sDefaultValue = (gParameters.GetIsLoggingHistory() ? "y" : "n"); break;
      default : InvalidParameterException::Generate("Unknown parameter enumeration %d.","MarkAsMissingDefaulted()", eParameterType);
    };

    if (sDefaultValue.GetLength()) {
      gvParametersMissingDefaulted.push_back(static_cast<int>(eParameterType)); //and default retained.
      PrintDirection.SatScanPrintWarning("Warning: The parameter '%s' is missing from the parameter file,\n"
                                         "         defaulted value '%s' assigned.\n",
                                         GetParameterLabel(eParameterType), sDefaultValue.GetCString());
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("MarkAsMissingDefaulted()","AbtractParameterFileAccess");
    throw;
 }
}

/** Attempts to interpret passed string as a boolean value. Throws InvalidParameterException. */
bool AbtractParameterFileAccess::ReadBoolean(const ZdString& sValue, ParameterType eParameterType) const {
  bool          bReadResult;

  try {
    if (sValue.GetIsEmpty()) {
      InvalidParameterException::Generate("Error: Parameter '%s' is not set.\n", "ReadBoolean()", GetParameterLabel(eParameterType));
    }
    else if (!(!stricmp(sValue.GetCString(),"y")   || !stricmp(sValue.GetCString(),"n") ||
               !strcmp(sValue.GetCString(),"1")    || !strcmp(sValue.GetCString(),"0")   ||
               !stricmp(sValue.GetCString(),"yes")  || !stricmp(sValue.GetCString(),"no"))) {
      InvalidParameterException::Generate("Error: For parameter '%s', setting '%s' is invalid. Valid values are 'y' or 'n'.\n",
                                          "ReadBoolean()", GetParameterLabel(eParameterType), sValue.GetCString());
    }
    else
      bReadResult = (!stricmp(sValue.GetCString(),"y") || !stricmp(sValue.GetCString(),"yes") || !strcmp(sValue.GetCString(),"1"));
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadBoolean()","AbtractParameterFileAccess");
    throw;
  }
  return bReadResult;
}


/** Set date parameter with passed string using appropriate set function. */
void AbtractParameterFileAccess::ReadDate(const ZdString& sValue, ParameterType eParameterType) const {
 try {
   switch (eParameterType) {
     case START_PROSP_SURV      : //As a legacy of the old parameters code,
                                  //we need to check that the length of the
                                  //string is not one. The prospective start
                                  //date took the line position of an extra
                                  //parameter as seen in old file
                                  //"0                     // Extra Parameter #4".
                                  //We don't want to produce an error for
                                  //an invalid parameter that the user didn't
                                  //miss set. So, treat a value of "0" as blank.
                                  if (sValue == "0")
                                    gParameters.SetProspectiveStartDate("");
                                  else
                                    gParameters.SetProspectiveStartDate(sValue);
                                  break;
     case STARTDATE             : gParameters.SetStudyPeriodStartDate(sValue); break;
     case ENDDATE               : gParameters.SetStudyPeriodEndDate(sValue); break;
     default : ZdException::Generate("Parameter enumeration '%d' is not listed for date read.\n","ReadDate()", eParameterType);
   };
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadDate()","AbtractParameterFileAccess");
    throw;
  }
}

/** Attempts to interpret passed string as comma separated string of dates. Throws InvalidParameterException. */
void AbtractParameterFileAccess::ReadDateRange(const ZdString& sValue, ParameterType eParameterType, DateRange_t& Range) const {
  int                   i, iNumTokens;

  try {
    if (sValue.GetLength()) {
      ZdStringTokenizer     Tokenizer(sValue, ",");
      iNumTokens = Tokenizer.GetNumTokens();
      if (iNumTokens != 2)
        InvalidParameterException::Generate("Error: For parameter '%s', %d values specified but should have 2.\n",
                                            "ReadDateRange()", GetParameterLabel(eParameterType), iNumTokens);
      Range.first = Tokenizer.GetNextToken().GetCString();
      Range.second = Tokenizer.GetNextToken().GetCString();
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadDateRange()","AbtractParameterFileAccess");
    throw;
  }
}

/** Attempts to interpret passed string as a double value. Throws InvalidParameterException. */
double AbtractParameterFileAccess::ReadDouble(const ZdString & sValue, ParameterType eParameterType) const {
  double        dReadResult;

  try {
    if (sValue.GetIsEmpty()) {
      InvalidParameterException::Generate("Error: Parameter '%s' is not set.\n", "ReadDouble()",
                                          GetParameterLabel(eParameterType));
    }
    else if (sscanf(sValue.GetCString(), "%lf", &dReadResult) != 1) {
      InvalidParameterException::Generate("Error: For parameter '%s', setting '%s' is not a valid real number.\n", "ReadDouble()",
                                          GetParameterLabel(eParameterType), sValue.GetCString());
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadDouble()","AbtractParameterFileAccess");
    throw;
  }
  return dReadResult;
}

/** Attempts to interpret passed string as a space/comma delimited string of integers that represent
    the number of rotations ellipse will make. No attempt to convert is made if no
    ellipses defined.  Throws InvalidParameterException. */
void AbtractParameterFileAccess::ReadEllipseRotations(const ZdString& sParameter) const {
  int                   i, iNumTokens, iReadNumberRotations;
  ZdString              sLabel;

  try {
    if (sParameter.GetLength() && gParameters.GetNumRequestedEllipses()) {
      ZdStringTokenizer     Tokenizer(sParameter, (sParameter.Find(',') == -1 ? " " : "," ));
      iNumTokens = Tokenizer.GetNumTokens();
      for (i=0; i < iNumTokens; i++) {
         if (sscanf(Tokenizer.GetToken(i).GetCString(), "%i", &iReadNumberRotations))
           gParameters.SetNumberEllipsoidRotations(iReadNumberRotations);
         else
           InvalidParameterException::Generate("Error: For parameter '%s', setting '%s' is not an integer.\n", "ReadEllipseRotations()",
                                               GetParameterLabel(ENUMBERS), Tokenizer.GetToken(i).GetCString());
      }
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadEllipseRotations()","AbtractParameterFileAccess");
    throw;
  }
}

/** Attempts to interpret passed string as a space delimited string of integers that represent
    the shape of each ellipsoid. No attempt to convert is made if there are no
    ellipses defined.  Throws InvalidParameterException. */
void AbtractParameterFileAccess::ReadEllipseShapes(const ZdString& sParameter) const {
  int                   i, iNumTokens;
  double                dReadShape;

  try {
    if (sParameter.GetLength() && gParameters.GetNumRequestedEllipses()) {
      ZdStringTokenizer     Tokenizer(sParameter, (sParameter.Find(',') == -1 ? " " : "," ));
      iNumTokens = Tokenizer.GetNumTokens();
      for (i=0; i < iNumTokens; i++) {
         if (sscanf(Tokenizer.GetToken(i).GetCString(), "%lf", &dReadShape))
           gParameters.SetEllipsoidShape(dReadShape);
         else
           InvalidParameterException::Generate("Error: For parameter '%s', setting '%s' is not an decimal number.\n",
                                               "ReadEllipseShapes()", GetParameterLabel(ESHAPES), Tokenizer.GetToken(i).GetCString());
      }
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadEllipseShapes()","AbtractParameterFileAccess");
    throw;
  }
}

/** Attempts to validate integer as enumeration within specified range. Throws InvalidParameterException. */
int AbtractParameterFileAccess::ReadEnumeration(int iValue, ParameterType eParameterType, int iLow, int iHigh) const {
  try {
    if (iValue < iLow || iValue > iHigh)
      InvalidParameterException::Generate("Error: For parameter '%s', setting '%d' is out of range(%d - %d).\n", "SetCoordinatesType()",
                                          GetParameterLabel(eParameterType), iValue, iLow, iHigh);
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadEnumeration()","AbtractParameterFileAccess");
    throw;
  }
  return iValue;
}

/** Attempts to interpret passed string as a float value. Throws InvalidParameterException. */
float AbtractParameterFileAccess::ReadFloat(const ZdString& sValue, ParameterType eParameterType) const {
  float         fReadResult;

  try {
    if (sValue.GetIsEmpty()) {
      InvalidParameterException::Generate("Error: Parameter '%s' is not set.\n",
                                          "ReadFloat()", GetParameterLabel(eParameterType));
    }
    else if (sscanf(sValue.GetCString(), "%f", &fReadResult) != 1) {
      InvalidParameterException::Generate("Error: For parameter '%s', setting '%s' is not a valid real number.\n",
                                          "ReadFloat()", GetParameterLabel(eParameterType), sValue.GetCString());
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadFloat()","AbtractParameterFileAccess");
    throw;
  }
  return fReadResult;
}


/** Attempts to interpret passed string as an integer value. Throws InvalidParameterException. */
int AbtractParameterFileAccess::ReadInt(const ZdString& sValue, ParameterType eParameterType) const {
  int           iReadResult;

  try {
    if (sValue.GetIsEmpty()) {
      InvalidParameterException::Generate("Error: Parameter '%s' is not set.\n",
                                          "ReadInt()", GetParameterLabel(eParameterType));
    }
    else if (sscanf(sValue.GetCString(), "%i", &iReadResult) != 1) {
      InvalidParameterException::Generate("Error: For parameter '%s', setting '%s' is not a valid integer.\n",
                                         "ReadInt()", GetParameterLabel(eParameterType), sValue.GetCString());
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadInt()","AbtractParameterFileAccess");
    throw;
  }
  return iReadResult;
}

/** Attempts to interpret passed string as an integer value. Throws InvalidParameterException. */
int AbtractParameterFileAccess::ReadUnsignedInt(const ZdString& sValue, ParameterType eParameterType) const {
  int           iReadResult;

  try {
   if (sValue.GetIsEmpty()) {
     InvalidParameterException::Generate("Error: Parameter '%s' is not set.\n",
                                         "ReadUnsignedInt()", GetParameterLabel(eParameterType));
   }
   else if (sscanf(sValue.GetCString(), "%u", &iReadResult) != 1) {
     InvalidParameterException::Generate("Error: For parameter '%s', setting '%s' is not a valid integer.\n",
                                         "ReadUnsignedInt()", GetParameterLabel(eParameterType), sValue.GetCString());
   }
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadUnsignedInt()","AbtractParameterFileAccess");
    throw;
  }
  return iReadResult;
}

/** Attempts to interpret passed string as version number of format '#.#.#'. Throws InvalidParameterException. */
void AbtractParameterFileAccess::ReadVersion(const ZdString& sValue) const {
  CParameters::CreationVersion       vVersion;

   if (sValue.GetIsEmpty())
     InvalidParameterException::Generate("Error: Parameter '%s' is not set.\n", "ReadVersion()", GetParameterLabel(CREATION_VERSION));
   else if (sscanf(sValue.GetCString(), "%u.%u.%u", &vVersion.iMajor, &vVersion.iMinor, &vVersion.iRelease) < 3)
    InvalidParameterException::Generate("Error: Parameter '%s' is not set.\n", "ReadVersion()", GetParameterLabel(CREATION_VERSION));
  gParameters.SetVersion(vVersion);
}

/** Calls appropriate read and set function for parameter type. */
void AbtractParameterFileAccess::SetParameter(ParameterType eParameterType, const ZdString& sParameter, BasePrint& PrintDirection) {
  int           iValue;
  DateRange_t   Range;

  try {
    switch (eParameterType) {
      case ANALYSISTYPE              : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, PURELYSPATIAL, PROSPECTIVEPURELYTEMPORAL);
                                       gParameters.SetAnalysisType((AnalysisType)iValue); break;
      case SCANAREAS                 : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, HIGH, HIGHANDLOW);
                                       gParameters.SetAreaRateType((AreaRateType)iValue); break;
      case CASEFILE                  : gParameters.SetCaseFileName(sParameter.GetCString(), true); break;
      case POPFILE                   : gParameters.SetPopulationFileName(sParameter.GetCString(), true); break;
      case COORDFILE                 : gParameters.SetCoordinatesFileName(sParameter.GetCString(), true); break;
      case OUTPUTFILE                : gParameters.SetOutputFileName(sParameter.GetCString(), true); break;
      case PRECISION                 : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, NONE, DAY);
                                       gParameters.SetPrecisionOfTimesType((DatePrecisionType)iValue); break;
      case DIMENSION                 : //Dimensions no longer read from file.
                                       break;
      case SPECIALGRID               : gParameters.SetUseSpecialGrid(ReadBoolean(sParameter, eParameterType)); break;
      case GRIDFILE                  : gParameters.SetSpecialGridFileName(sParameter.GetCString(), true); break;
      case GEOSIZE                   : gParameters.SetMaximumGeographicClusterSize(ReadFloat(sParameter, eParameterType)); break;
      case STARTDATE                 : ReadDate(sParameter, eParameterType); break;
      case ENDDATE                   : ReadDate(sParameter, eParameterType); break;
      case CLUSTERS                  : gParameters.SetIncludeClustersType((IncludeClustersType)ReadInt(sParameter, eParameterType)); break;
      case EXACTTIMES                : //No longer used. No documentation as to previous usage.
                                       break;
      case TIME_AGGREGATION_UNITS    : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, NONE, DAY);
                                       gParameters.SetTimeAggregationUnitsType((DatePrecisionType)iValue); break;
      case TIME_AGGREGATION          : gParameters.SetTimeAggregationLength((long)ReadInt(sParameter, eParameterType)); break;
      case PURESPATIAL               : gParameters.SetIncludePurelySpatialClusters(ReadBoolean(sParameter, eParameterType)); break;
      case TIMESIZE                  : gParameters.SetMaximumTemporalClusterSize(ReadFloat(sParameter, eParameterType)); break;
      case REPLICAS                  : gParameters.SetNumberMonteCarloReplications(ReadUnsignedInt(sParameter, eParameterType)); break;
      case MODEL                     : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, POISSON, RANK);
                                       gParameters.SetProbabilityModelType((ProbabilityModelType)iValue); break;
      case RISKFUNCTION              : gParameters.SetRiskType((RiskType)ReadInt(sParameter, eParameterType)); break;
      case POWERCALC                 : gParameters.SetPowerCalculation(ReadBoolean(sParameter, eParameterType)); break;
      case POWERX                    : gParameters.SetPowerCalculationX(ReadDouble(sParameter, eParameterType)); break;
      case POWERY                    : gParameters.SetPowerCalculationY(ReadDouble(sParameter, eParameterType)); break;
      case TIMETREND                 : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, NOTADJUSTED, STRATIFIED_RANDOMIZATION);
                                       gParameters.SetTimeTrendAdjustmentType((TimeTrendAdjustmentType)iValue); break;
      case TIMETRENDPERC             : gParameters.SetTimeTrendAdjustmentPercentage(ReadDouble(sParameter, eParameterType)); break;
      case PURETEMPORAL              : gParameters.SetIncludePurelyTemporalClusters(ReadBoolean(sParameter, eParameterType)); break;
      case CONTROLFILE               : gParameters.SetControlFileName(sParameter.GetCString(), true); break;
      case COORDTYPE                 : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, CARTESIAN, LATLON);
                                       gParameters.SetCoordinatesType((CoordinatesType)iValue); break;
      case OUTPUT_SIM_LLR_ASCII      : gParameters.SetOutputSimLogLikeliRatiosAscii(ReadBoolean(sParameter, eParameterType)); break;
      case SEQUENTIAL                : gParameters.SetSequentialScanning(ReadBoolean(sParameter, eParameterType)); break;
      case SEQNUM                    : gParameters.SetNumSequentialScans(ReadUnsignedInt(sParameter, eParameterType)); break;
      case SEQPVAL                   : gParameters.SetSequentialCutOffPValue(ReadDouble(sParameter, eParameterType)); break;
      case VALIDATE                  : gParameters.SetValidatePriorToCalculation(ReadBoolean(sParameter, eParameterType)); break;
      case OUTPUT_RR_ASCII           : gParameters.SetOutputRelativeRisksAscii(ReadBoolean(sParameter, eParameterType)); break;
      case ELLIPSES                  : gParameters.SetNumberEllipses(ReadInt(sParameter, eParameterType)); break;
      case ESHAPES                   : ReadEllipseShapes(sParameter); break;
      case ENUMBERS                  : ReadEllipseRotations(sParameter); break;
      case START_PROSP_SURV          : ReadDate(sParameter, eParameterType); break;
      case OUTPUT_AREAS_ASCII        : gParameters.SetOutputAreaSpecificAscii(ReadBoolean(sParameter, eParameterType)); break;
      case OUTPUT_MLC_ASCII          : gParameters.SetOutputClusterLevelAscii(ReadBoolean(sParameter, eParameterType)); break;
      case CRITERIA_SECOND_CLUSTERS  : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, NOGEOOVERLAP, NORESTRICTIONS);
                                       gParameters.SetCriteriaForReportingSecondaryClusters((CriteriaSecondaryClustersType)iValue); break;
      case MAX_TEMPORAL_TYPE         : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, PERCENTAGETYPE, TIMETYPE);
                                       gParameters.SetMaximumTemporalClusterSizeType((TemporalSizeType)ReadInt(sParameter, eParameterType)); break;
      case MAX_SPATIAL_TYPE          : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, PERCENTOFPOPULATIONTYPE, PERCENTOFPOPULATIONFILETYPE);
                                       gParameters.SetMaximumSpacialClusterSizeType((SpatialSizeType)iValue); break;
      case RUN_HISTORY_FILENAME      : //Run History no longer scanned from parameters file. Set through setters/getters and copy() only.
                                       break;
      case OUTPUT_MLC_DBASE          : gParameters.SetOutputClusterLevelDBase(ReadBoolean(sParameter, eParameterType)); break;
      case OUTPUT_AREAS_DBASE        : gParameters.SetOutputAreaSpecificDBase(ReadBoolean(sParameter, eParameterType)); break;
      case OUTPUT_RR_DBASE           : gParameters.SetOutputRelativeRisksDBase(ReadBoolean(sParameter, eParameterType)); break;
      case OUTPUT_SIM_LLR_DBASE      : gParameters.SetOutputSimLogLikeliRatiosDBase(ReadBoolean(sParameter, eParameterType)); break;
      case NON_COMPACTNESS_PENALTY   : gParameters.SetNonCompactnessPenalty(ReadBoolean(sParameter, eParameterType)); break;
      case INTERVAL_STARTRANGE       : ReadDateRange(sParameter, eParameterType, Range);
                                       gParameters.SetStartRangeStartDate(Range.first.c_str());
                                       gParameters.SetStartRangeEndDate(Range.second.c_str()); break;
      case INTERVAL_ENDRANGE         : ReadDateRange(sParameter, eParameterType, Range);
                                       gParameters.SetEndRangeStartDate(Range.first.c_str());
                                       gParameters.SetEndRangeEndDate(Range.second.c_str()); break;
      case TIMETRENDCONVRG           : /*gParameters.SetTimeTrendConvergence(ReadDouble(sParameter, eParameterType));*/ break;
      case MAXCIRCLEPOPFILE          : gParameters.SetMaxCirclePopulationFileName(sParameter.GetCString(), true); break;
      case EARLY_SIM_TERMINATION     : gParameters.SetTerminateSimulationsEarly(ReadBoolean(sParameter, eParameterType)); break;
      case REPORTED_GEOSIZE          : gParameters.SetMaximumReportedGeographicalClusterSize(ReadFloat(sParameter, eParameterType)); break;
      case USE_REPORTED_GEOSIZE      : gParameters.SetRestrictReportedClusters(ReadBoolean(sParameter, eParameterType)); break;
      case SIMULATION_TYPE           : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, STANDARD, FILESOURCE);
                                       gParameters.SetSimulationType((SimulationType)iValue); break;
      case SIMULATION_SOURCEFILE     : gParameters.SetSimulationDataSourceFileName(sParameter.GetCString(), true); break;
      case ADJ_BY_RR_FILE            : gParameters.SetAdjustmentsByRelativeRisksFilename(sParameter.GetCString(), true); break;
      case OUTPUT_SIMULATION_DATA    : gParameters.SetOutputSimulationData(ReadBoolean(sParameter, eParameterType)); break;
      case SIMULATION_DATA_OUTFILE   : gParameters.SetSimulationDataOutputFileName(sParameter.GetCString(), true); break;
      case ADJ_FOR_EALIER_ANALYSES   : gParameters.SetAdjustForEarlierAnalyses(ReadBoolean(sParameter, eParameterType)); break;
      case USE_ADJ_BY_RR_FILE        : gParameters.SetUseAdjustmentForRelativeRisksFile(ReadBoolean(sParameter, eParameterType)); break;
      case SPATIAL_ADJ_TYPE          : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, NO_SPATIAL_ADJUSTMENT, SPATIALLY_STRATIFIED_RANDOMIZATION);
                                       gParameters.SetSpatialAdjustmentType((SpatialAdjustmentType)ReadInt(sParameter, eParameterType)); break;
      case MULTI_DATASET_PURPOSE_TYPE: iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, MULTIVARIATE, ADJUSTMENT);
                                       gParameters.SetMultipleDataSetPurposeType((MultipleDataSetPurposeType)iValue); break;
      case CREATION_VERSION          : ReadVersion(sParameter); break;
      case RANDOMIZATION_SEED        : gParameters.SetRandomizationSeed(ReadInt(sParameter, eParameterType)); break;
      case REPORT_CRITICAL_VALUES    : gParameters.SetReportCriticalValues(ReadBoolean(sParameter, eParameterType)); break;
      case EXECUTION_TYPE            : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, AUTOMATIC, CENTRICALLY);
                                       gParameters.SetExecutionType((ExecutionType)iValue); break;
      case NUM_PROCESSES             : gParameters.SetNumParallelProcessesToExecute(ReadUnsignedInt(sParameter, eParameterType)); break;
      case LOG_HISTORY               : gParameters.SetIsLoggingHistory(ReadBoolean(sParameter, eParameterType)); break;
      default : InvalidParameterException::Generate("Unknown parameter enumeration %d.","SetParameter()", eParameterType);
    };
  }
  catch (InvalidParameterException &x) {
    gbReadStatusError = true;
    PrintDirection.SatScanPrintWarning(x.GetErrorMessage());
  }
  catch (ZdException &x) {
    x.AddCallpath("SetParameter()","AbtractParameterFileAccess");
    throw;
  }
}

//var_arg constructor
InvalidParameterException::InvalidParameterException(va_list varArgs, const char *sMessage, const char *sSourceModule, ZdException::Level iLevel)
                          :ResolvableException(varArgs, sMessage, sSourceModule, iLevel){}

//static generation function:
void InvalidParameterException::Generate(const char *sMessage, const char *sSourceModule, ...) {
   va_list varArgs;
   va_start(varArgs, sSourceModule);

   InvalidParameterException theException(varArgs, sMessage, sSourceModule, Normal);
   va_end(varArgs);
   throw theException;
}


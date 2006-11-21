//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "ParameterFileAccess.h"
#include "ScanLineParameterFileAccess.h"
#include "IniParameterFileAccess.h"
#include<boost/tokenizer.hpp>

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
      throw resolvable_error("Unable to open parameter file '%s'.\n", sFilename);

    IniFile ini; ini.Read(sFilename);
    if (ini.GetNumSections())
      bSuccess = IniParameterFileAccess(gParameters, PrintDirection).Read(sFilename);
    else
      bSuccess = ScanLineParameterFileAccess(gParameters, PrintDirection).Read(sFilename);
  }
  catch (prg_exception &x) {
    throw resolvable_error("Unable to read parameters from file '%s'.\n", sFilename);
  }
  return bSuccess;
}

/** Writes parameters to ini file in most recent format. */
void ParameterAccessCoordinator::Write(const char * sFilename, BasePrint& PrintDirection /* ability to specify a version to write as ?*/) {
  try {
   IniParameterFileAccess(gParameters, PrintDirection).Write(sFilename);
   //ScanLineParameterFileAccess(gParameters, PrintDirection).Write(sFilename);
  }
  catch (prg_exception& x) {
    x.addTrace("Write()","ParameterAccessCoordinator");
    throw;
  }
}


/** constructor */
AbtractParameterFileAccess::AbtractParameterFileAccess(CParameters& Parameters, BasePrint& PrintDirection, bool bWriteBooleanAsDigit)
                           :gParameters(Parameters), gPrintDirection(PrintDirection), gbWriteBooleanAsDigit(bWriteBooleanAsDigit),
                            gdMaxSpatialClusterSize(50.0), gdMaxReportedSpatialClusterSize(50.0) {}

/** destructor */
AbtractParameterFileAccess::~AbtractParameterFileAccess() {}

/** Returns constant char pointer to parameters comment string. */
const char * AbtractParameterFileAccess::GetParameterComment(ParameterType eParameterType) const {
  try {
    switch (eParameterType) {
      case ANALYSISTYPE             : return " analysis type (1=Purely Spatial, 2=Purely Temporal, 3=Retrospective Space-Time, 4=Prospective Space-Time, 5=N/A, 6=Prospective Purely Temporal)";
      case SCANAREAS                : return " scan areas (1=High Rates(Poison,Bernoulli,STP); High Values(Ordinal,Normal); Short Survival(Exponential), 2=Low Rates(Poison,Bernoulli,STP); Low Values(Ordinal,Normal); Long Survival(Exponential), 3=Both Areas)";
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
      case TIME_AGGREGATION         : return " time aggregation length (Positive Integer)";
      case PURESPATIAL              : return " include purely spatial clusters? (y/n)";
      case TIMESIZE                 : return " maximum temporal cluster size (<=90%)";
      case REPLICAS                 : return " Monte Carlo replications (0, 9, 999, n999)";
      case MODEL                    : return " model type (0=Poisson, 1=Bernoulli, 2=Space-Time Permutation, 3=Ordinal, 4=Exponential, 5=Normal)";
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
      case ITERATIVE                : return " perform iterative scans? (y/n)";
      case ITERATIVE_NUM            : return " maximum iterations for iterative scan (0-32000)";
      case ITERATIVE_PVAL           : return " max p-value for iterative scan before cutoff (0.000-1.000)";
      case VALIDATE                 : return " validate parameters prior to analysis execution? (y/n)";
      case OUTPUT_RR_ASCII          : return " output relative risks in ASCII format? (y/n)";
      case WINDOW_SHAPE             : return " window shape (0=Circular, 1=Elliptic)";
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
      case NON_COMPACTNESS_PENALTY  : return " elliptic non-compactness penalty (0=NoPenalty, 1=MediumPenalty, 2=StrongPenalty)";
      case INTERVAL_STARTRANGE      : return " flexible temporal window start range (YYYY/MM/DD,YYYY/MM/DD)";
      case INTERVAL_ENDRANGE        : return " flexible temporal window end range (YYYY/MM/DD,YYYY/MM/DD)";
      case TIMETRENDCONVRG	    : return " time trend convergence for SVTT analysis (> 0)";
      case MAXCIRCLEPOPFILE         : return " maximum circle size filename";
      case EARLY_SIM_TERMINATION    : return " terminate simulations early for large p-values? (y/n)";
      case REPORTED_GEOSIZE         : return " max reported geographic size (< max geographical cluster size%)";
      case USE_REPORTED_GEOSIZE     : return " restrict reported clusters to maximum geographical cluster size? (y/n)";
      case SIMULATION_TYPE          : return " simulation methods (0=Null Randomization, 1=HA Randomization, 2=File Import)";
      case SIMULATION_SOURCEFILE    : return " simulation data input file name (with File Import=2)";
      case ADJ_BY_RR_FILE           : return " adjustments by known relative risks file name (with HA Randomization=1)";
      case OUTPUT_SIMULATION_DATA   : return " print simulation data to file? (y/n)";
      case SIMULATION_DATA_OUTFILE  : return " simulation data output filename";
      case ADJ_FOR_EALIER_ANALYSES  : return " adjust for earlier analyses(prospective analyses only)? (y/n)";
      case USE_ADJ_BY_RR_FILE       : return " use adjustments by known relative risks file? (y/n)";
      case SPATIAL_ADJ_TYPE         : return " spatial adjustments type (0=No Spatial Adjustment, 1=Spatially Stratified Randomization)";
      case MULTI_DATASET_PURPOSE_TYPE : return " multiple data sets purpose type (0=Multivariate, 1=Adjustment)";
      case CREATION_VERSION         : return " system setting - do not modify";
      case RANDOMIZATION_SEED       : return " randomization seed (0 < Seed < 2147483647)";
      case REPORT_CRITICAL_VALUES   : return " report critical values for .01 and .05? (y/n)";
      case EXECUTION_TYPE           : return " analysis execution method  (0=Automatic, 1=Successively, 2=Centrically)";
      case NUM_PROCESSES            : return " number of parallel processes to execute (0=All Processors, x=At Most X Processors)";
      case LOG_HISTORY              : return " log analysis run to history file? (y/n)";
      case SUPPRESS_WARNINGS        : return " suppressing warnings? (y/n)";
      case MAX_REPORTED_SPATIAL_TYPE: return " how max spatial size should be interpretted for reported clusters (0=Percentage, 1=Distance, 2=Percentage of max circle population file)";
      case OUTPUT_MLC_CASE_ASCII    : return " output cluster case information in ASCII format? (y/n)";
      case OUTPUT_MLC_CASE_DBASE    : return " output cluster case information in dBase format? (y/n)";
      case STUDYPERIOD_DATACHECK    : return " study period data check (0=Strict Bounds, 1=Relaxed Bounds)";
      case COORDINATES_DATACHECK    : return " geographical coordinates data check (0=Strict Coordinates, 1=Relaxed Coordinates)";
      case MAXGEOPOPATRISK          : return " maximum spatial size in population at risk (<=50%)";
      case MAXGEOPOPFILE            : return " maximum spatial size in max circle population file (<=50%)";
      case MAXGEODISTANCE           : return " maximum spatial size in distance from center (positive integer)";
      case USE_MAXGEOPOPFILE        : return " restrict maximum spatial size - max circle file? (y/n)";
      case USE_MAXGEODISTANCE       : return " restrict maximum spatial size - distance? (y/n)";
      case MAXGEOPOPATRISK_REPORTED : return " maximum reported spatial size in population at risk (<=50%)";
      case MAXGEOPOPFILE_REPORTED   : return " maximum reported spatial size in max circle population file (<=50%)";
      case MAXGEODISTANCE_REPORTED  : return " maximum reported spatial size in distance from center {positive integer)";
      case USE_MAXGEOPOPFILE_REPORTED: return " restrict maximum reported spatial size - max circle file? (y/n)";
      case USE_MAXGEODISTANCE_REPORTED: return " restrict maximum reported spatial size - distance? (y/n)";
      case LOCATION_NEIGHBORS_FILE  : return " neighbors file";
      case USE_LOCATION_NEIGHBORS_FILE : return " use neighbors file (y/n)";
      case RANDOMLY_GENERATE_SEED   : return " randomly generate seed (y/n)";
      case MULTIPLE_COORDINATES_TYPE: return " multiple coordinates type (0=OnePerLocation, 1=AtLeastOneLocation, 2=AllLocations)";
      case META_LOCATIONS_FILE      : return " meta locations file";
      case USE_META_LOCATIONS_FILE  : return " use meta locations file (y/n)";
      default : throw prg_error("Unknown parameter enumeration %d.","GetParameterComment()", eParameterType);
    };
  }
  catch (prg_exception& x) {
    x.addTrace("GetParameterComment()","AbtractParameterFileAccess");
    throw;
 }
}

/** Assigns string representation to passed string class for parameter. */
std::string & AbtractParameterFileAccess::GetParameterString(ParameterType eParameterType, std::string& s) const {
  std::string worker;

  try {
    switch (eParameterType) {
      case ANALYSISTYPE             : return AsString(s, gParameters.GetAnalysisType());
      case SCANAREAS                : return AsString(s, gParameters.GetAreaScanRateType());
      case CASEFILE                 : s = gParameters.GetCaseFileName().c_str(); return s;
      case POPFILE                  : s = gParameters.GetPopulationFileName().c_str(); return s;
      case COORDFILE                : s = gParameters.GetCoordinatesFileName().c_str(); return s;
      case OUTPUTFILE               : s = gParameters.GetOutputFileName().c_str(); return s;
      case PRECISION                : return AsString(s, gParameters.GetPrecisionOfTimesType());
      case DIMENSION                : s = "0"; return s;
      case SPECIALGRID              : return AsString(s, gParameters.UseSpecialGrid());
      case GRIDFILE                 : s = gParameters.GetSpecialGridFileName().c_str(); return s;
      case GEOSIZE                  : s = "0"; return s;
      case STARTDATE                : s = gParameters.GetStudyPeriodStartDate().c_str(); return s;
      case ENDDATE                  : s = gParameters.GetStudyPeriodEndDate().c_str(); return s;
      case CLUSTERS                 : return AsString(s, gParameters.GetIncludeClustersType());
      case EXACTTIMES               : s = "0"; return s;
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
      case ITERATIVE                : return AsString(s, gParameters.GetIsIterativeScanning());
      case ITERATIVE_NUM            : return AsString(s, gParameters.GetNumIterativeScansRequested());
      case ITERATIVE_PVAL           : return AsString(s, gParameters.GetIterativeCutOffPValue());
      case VALIDATE                 : s = "0"; return s;
      case OUTPUT_RR_ASCII          : return AsString(s, gParameters.GetOutputRelativeRisksAscii());
      case WINDOW_SHAPE             : return AsString(s, gParameters.GetSpatialWindowType());
      case ESHAPES                  : s = "";
                                      for (size_t i=0; i < gParameters.GetEllipseShapes().size(); ++i) {
                                         printString(worker, "%g", gParameters.GetEllipseShapes()[i]);
                                         s += (i == 0 ? "" : ","); s += worker;
                                      }
                                      return s;
      case ENUMBERS                 :  s = "";
                                      for (size_t i=0; i < gParameters.GetEllipseRotations().size(); ++i) {
                                         printString(worker, "%d", gParameters.GetEllipseRotations()[i]);
                                         s += (i == 0 ? "" : ","); s += worker;
                                      }
                                      return s;
      case START_PROSP_SURV         : s = gParameters.GetProspectiveStartDate(); return s;
      case OUTPUT_AREAS_ASCII       : return AsString(s, gParameters.GetOutputAreaSpecificAscii());
      case OUTPUT_MLC_ASCII         : return AsString(s, gParameters.GetOutputClusterLevelAscii());
      case CRITERIA_SECOND_CLUSTERS : return AsString(s, gParameters.GetCriteriaSecondClustersType());
      case MAX_TEMPORAL_TYPE        : return AsString(s, gParameters.GetMaximumTemporalClusterSizeType());
      case MAX_SPATIAL_TYPE         : s = "0"; return s;
      case RUN_HISTORY_FILENAME     : s = "0"; return s;
      case OUTPUT_MLC_DBASE         : return AsString(s, gParameters.GetOutputClusterLevelDBase());
      case OUTPUT_AREAS_DBASE       : return AsString(s, gParameters.GetOutputAreaSpecificDBase());
      case OUTPUT_RR_DBASE          : return AsString(s, gParameters.GetOutputRelativeRisksDBase());
      case OUTPUT_SIM_LLR_DBASE     : return AsString(s, gParameters.GetOutputSimLoglikeliRatiosDBase());
      case NON_COMPACTNESS_PENALTY  : return AsString(s, gParameters.GetNonCompactnessPenaltyType());
      case INTERVAL_STARTRANGE      : printString(s, "%s,%s", gParameters.GetStartRangeStartDate().c_str(), gParameters.GetStartRangeEndDate().c_str());
                                      return s;
      case INTERVAL_ENDRANGE        : printString(s, "%s,%s", gParameters.GetEndRangeStartDate().c_str(), gParameters.GetEndRangeEndDate().c_str());
                                      return s;
      case TIMETRENDCONVRG	    : return AsString(s, gParameters.GetTimeTrendConvergence());
      case MAXCIRCLEPOPFILE         : s = gParameters.GetMaxCirclePopulationFileName(); return s;
      case EARLY_SIM_TERMINATION    : return AsString(s, gParameters.GetTerminateSimulationsEarly());
      case REPORTED_GEOSIZE         : s = "0"; return s;
      case USE_REPORTED_GEOSIZE     : return AsString(s, gParameters.GetRestrictingMaximumReportedGeoClusterSize());
      case SIMULATION_TYPE          : return AsString(s, gParameters.GetSimulationType());
      case SIMULATION_SOURCEFILE    : s = gParameters.GetSimulationDataSourceFilename(); return s;
      case ADJ_BY_RR_FILE           : s = gParameters.GetAdjustmentsByRelativeRisksFilename(); return s;
      case OUTPUT_SIMULATION_DATA   : return AsString(s, gParameters.GetOutputSimulationData());
      case SIMULATION_DATA_OUTFILE  : s = gParameters.GetSimulationDataOutputFilename(); return s;
      case ADJ_FOR_EALIER_ANALYSES  : return AsString(s, gParameters.GetAdjustForEarlierAnalyses());
      case USE_ADJ_BY_RR_FILE       : return AsString(s, gParameters.UseAdjustmentForRelativeRisksFile());
      case SPATIAL_ADJ_TYPE         : return AsString(s, gParameters.GetSpatialAdjustmentType());
      case MULTI_DATASET_PURPOSE_TYPE : return AsString(s, gParameters.GetMultipleDataSetPurposeType());
      case CREATION_VERSION         : printString(s, "%s.%s.%s", VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE); return s;
      case RANDOMIZATION_SEED       : return AsString(s, (int)gParameters.GetRandomizationSeed());
      case REPORT_CRITICAL_VALUES   : return AsString(s, gParameters.GetReportCriticalValues());
      case EXECUTION_TYPE           : return AsString(s, gParameters.GetExecutionType());
      case NUM_PROCESSES            : return AsString(s, gParameters.GetNumRequestedParallelProcesses());
      case LOG_HISTORY              : return AsString(s, gParameters.GetIsLoggingHistory());
      case SUPPRESS_WARNINGS        : return AsString(s, gParameters.GetSuppressingWarnings());
      case MAX_REPORTED_SPATIAL_TYPE: s = "0"; return s;
      case OUTPUT_MLC_CASE_ASCII    : return AsString(s, gParameters.GetOutputClusterCaseAscii());
      case OUTPUT_MLC_CASE_DBASE    : return AsString(s, gParameters.GetOutputClusterCaseDBase());
      case STUDYPERIOD_DATACHECK    : return AsString(s, gParameters.GetStudyPeriodDataCheckingType());
      case COORDINATES_DATACHECK    : return AsString(s, gParameters.GetCoordinatesDataCheckingType());
      case MAXGEOPOPATRISK          : return AsString(s, gParameters.GetMaxSpatialSizeForType(PERCENTOFPOPULATION, false));
      case MAXGEOPOPFILE            : return AsString(s, gParameters.GetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false));
      case MAXGEODISTANCE           : return AsString(s, gParameters.GetMaxSpatialSizeForType(MAXDISTANCE, false));
      case USE_MAXGEOPOPFILE        : return AsString(s, gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false));
      case USE_MAXGEODISTANCE       : return AsString(s, gParameters.GetRestrictMaxSpatialSizeForType(MAXDISTANCE, false));
      case MAXGEOPOPATRISK_REPORTED : return AsString(s, gParameters.GetMaxSpatialSizeForType(PERCENTOFPOPULATION, true));
      case MAXGEOPOPFILE_REPORTED   : return AsString(s, gParameters.GetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, true));
      case MAXGEODISTANCE_REPORTED  : return AsString(s, gParameters.GetMaxSpatialSizeForType(MAXDISTANCE, true));
      case USE_MAXGEOPOPFILE_REPORTED: return AsString(s, gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, true));
      case USE_MAXGEODISTANCE_REPORTED: return AsString(s, gParameters.GetRestrictMaxSpatialSizeForType(MAXDISTANCE, true));
      case LOCATION_NEIGHBORS_FILE  : s = gParameters.GetLocationNeighborsFileName(); return s;
      case USE_LOCATION_NEIGHBORS_FILE : return AsString(s, gParameters.UseLocationNeighborsFile());
      case RANDOMLY_GENERATE_SEED   : return AsString(s, gParameters.GetIsRandomlyGeneratingSeed());
      case MULTIPLE_COORDINATES_TYPE: return AsString(s, gParameters.GetMultipleCoordinatesType());
      case META_LOCATIONS_FILE      : s = gParameters.getMetaLocationsFilename(); return s;
      case USE_META_LOCATIONS_FILE  : return AsString(s, gParameters.UseMetaLocationsFile());
      default : throw prg_error("Unknown parameter enumeration %d.","GetParameterComment()", eParameterType);
    };
  }
  catch (prg_exception& x) {
    x.addTrace("GetParameterComment()","AbtractParameterFileAccess");
    throw;
 }
}

/** Prints message to print direction that parameter was missing when read from
    parameter file and that a default value as assigned. */
void AbtractParameterFileAccess::MarkAsMissingDefaulted(ParameterType eParameterType, BasePrint& PrintDirection) {
  std::string default_value;

  try {
    switch (eParameterType) {
      case ANALYSISTYPE             : AsString(default_value, gParameters.GetAnalysisType()); break;
      case SCANAREAS                : AsString(default_value, gParameters.GetAreaScanRateType()); break;
      case CASEFILE                 : default_value = "<blank>"; break;
      case POPFILE                  : default_value = "<blank>"; break;
      case COORDFILE                : default_value = "<blank>"; break;
      case OUTPUTFILE               : default_value = "<blank>"; break;
      case PRECISION                : AsString(default_value, gParameters.GetPrecisionOfTimesType()); break;
      case DIMENSION                : /*  */ break;
      case SPECIALGRID              : default_value = (gParameters.UseSpecialGrid() ? "y" : "n"); break;
      case GRIDFILE                 : default_value = "<blank>"; break;
      case GEOSIZE                  : /* no longer used */ break;
      case STARTDATE                : default_value = gParameters.GetStudyPeriodStartDate(); break;
      case ENDDATE                  : default_value = gParameters.GetStudyPeriodEndDate(); break;
      case CLUSTERS                 : AsString(default_value, gParameters.GetIncludeClustersType()); break;
      case EXACTTIMES               : /* no longer used */ break;
      case TIME_AGGREGATION_UNITS   : AsString(default_value, gParameters.GetTimeAggregationUnitsType()); break;
      case TIME_AGGREGATION         : AsString(default_value, (unsigned int)gParameters.GetTimeAggregationLength()); break;
      case PURESPATIAL              : default_value = (gParameters.GetIncludePurelySpatialClusters() ? "y" : "n"); break;
      case TIMESIZE                 : AsString(default_value, gParameters.GetMaximumTemporalClusterSize()); break;
      case REPLICAS                 : AsString(default_value, gParameters.GetNumReplicationsRequested()); break;
      case MODEL                    : AsString(default_value, gParameters.GetProbabilityModelType()); break;
      case RISKFUNCTION             : AsString(default_value, gParameters.GetRiskType()); break;
      case POWERCALC                : default_value = (gParameters.GetIsPowerCalculated() ? "y" : "n"); break;
      case POWERX                   : AsString(default_value, gParameters.GetPowerCalculationX()); break;
      case POWERY                   : AsString(default_value, gParameters.GetPowerCalculationY()); break;
      case TIMETREND                : AsString(default_value, gParameters.GetTimeTrendAdjustmentType()); break;
      case TIMETRENDPERC            : AsString(default_value, gParameters.GetTimeTrendAdjustmentPercentage()); break;
      case PURETEMPORAL             : default_value = (gParameters.GetIncludePurelyTemporalClusters() ? "y" : "n"); break;
      case CONTROLFILE              : default_value = "<blank>"; break;
      case COORDTYPE                : AsString(default_value, gParameters.GetCoordinatesType()); break;
      case OUTPUT_SIM_LLR_ASCII     : default_value = (gParameters.GetOutputSimLoglikeliRatiosAscii() ? "y" : "n"); break;
      case ITERATIVE                : default_value = (gParameters.GetIsIterativeScanning() ? "y" : "n"); break;
      case ITERATIVE_NUM            : AsString(default_value, gParameters.GetNumIterativeScansRequested()); break;
      case ITERATIVE_PVAL           : AsString(default_value, gParameters.GetIterativeCutOffPValue()); break;
      case VALIDATE                 : /* no longer used */ break;
      case OUTPUT_RR_ASCII          : default_value = (gParameters.GetOutputRelativeRisksAscii() ? "y" : "n"); break;
      case WINDOW_SHAPE             : AsString(default_value, gParameters.GetSpatialWindowType()); break;
      case ESHAPES                  : default_value = "<blank>"; break;
      case ENUMBERS                 : default_value = "<blank>"; break;
      case START_PROSP_SURV         : default_value = gParameters.GetProspectiveStartDate(); break;
      case OUTPUT_AREAS_ASCII       : default_value = (gParameters.GetOutputAreaSpecificAscii() ? "y" : "n"); break;
      case OUTPUT_MLC_ASCII         : default_value = (gParameters.GetOutputClusterLevelAscii() ? "y" : "n"); break;
      case CRITERIA_SECOND_CLUSTERS : AsString(default_value, gParameters.GetCriteriaSecondClustersType()); break;
      case MAX_TEMPORAL_TYPE        : AsString(default_value, gParameters.GetMaximumTemporalClusterSizeType()); break;
      case MAX_SPATIAL_TYPE         : /* no longer used */ break;
      case RUN_HISTORY_FILENAME     : /* no longer read in from parameter file */ break;
      case OUTPUT_MLC_DBASE         : default_value = (gParameters.GetOutputClusterLevelDBase() ? "y" : "n"); break;
      case OUTPUT_AREAS_DBASE       : default_value = (gParameters.GetOutputAreaSpecificDBase() ? "y" : "n"); break;
      case OUTPUT_RR_DBASE          : default_value = (gParameters.GetOutputRelativeRisksDBase() ? "y" : "n"); break;
      case OUTPUT_SIM_LLR_DBASE     : default_value = (gParameters.GetOutputSimLoglikeliRatiosDBase() ? "y" : "n"); break;
      case NON_COMPACTNESS_PENALTY  : AsString(default_value, gParameters.GetNonCompactnessPenaltyType()); break;
      case INTERVAL_STARTRANGE      : printString(default_value, "%s,%s", gParameters.GetStartRangeStartDate().c_str(), gParameters.GetStartRangeEndDate().c_str());
                                      break;
      case INTERVAL_ENDRANGE        : printString(default_value, "%s,%s", gParameters.GetEndRangeStartDate().c_str(), gParameters.GetEndRangeEndDate().c_str());
                                      break;
      case TIMETRENDCONVRG	    : AsString(default_value, gParameters.GetTimeTrendConvergence()); break;
      case MAXCIRCLEPOPFILE         : default_value = "<blank>"; break;
      case EARLY_SIM_TERMINATION    : default_value = (gParameters.GetTerminateSimulationsEarly() ? "y" : "n"); break;
      case REPORTED_GEOSIZE         : /* no longer used */ break;
      case USE_REPORTED_GEOSIZE     : default_value = (gParameters.GetRestrictingMaximumReportedGeoClusterSize() ? "y" : "n"); break;
      case SIMULATION_TYPE          : AsString(default_value, gParameters.GetSimulationType()); break;
      case SIMULATION_SOURCEFILE    : default_value = "<blank>"; break;
      case ADJ_BY_RR_FILE           : default_value = "<blank>"; break;
      case OUTPUT_SIMULATION_DATA   : default_value = (gParameters.GetOutputSimulationData() ? "y" : "n"); break;
      case SIMULATION_DATA_OUTFILE  : default_value = "<blank>"; break;
      case ADJ_FOR_EALIER_ANALYSES  : default_value = (gParameters.GetAdjustForEarlierAnalyses() ? "y" : "n"); break;
      case USE_ADJ_BY_RR_FILE       : default_value = (gParameters.UseAdjustmentForRelativeRisksFile() ? "y" : "n"); break;
      case SPATIAL_ADJ_TYPE         : AsString(default_value, gParameters.GetSpatialAdjustmentType()); break;
      case MULTI_DATASET_PURPOSE_TYPE : AsString(default_value, gParameters.GetMultipleDataSetPurposeType()); break;
      case CREATION_VERSION         : printString(default_value, "%u.%u.%u", gParameters.GetCreationVersion().iMajor,
                                                                 gParameters.GetCreationVersion().iMinor, gParameters.GetCreationVersion().iRelease); break;
      case RANDOMIZATION_SEED       : break; //this parameter is not advertised
      case REPORT_CRITICAL_VALUES   : default_value = (gParameters.GetReportCriticalValues() ? "y" : "n"); break;
      case EXECUTION_TYPE           : AsString(default_value, gParameters.GetExecutionType()); break;
      case NUM_PROCESSES            : printString(default_value, "%u", gParameters.GetNumRequestedParallelProcesses()); break;
      case LOG_HISTORY              : default_value = (gParameters.GetIsLoggingHistory() ? "y" : "n"); break;
      case SUPPRESS_WARNINGS        : default_value = (gParameters.GetSuppressingWarnings() ? "y" : "n"); break;
      case MAX_REPORTED_SPATIAL_TYPE: /* no longer used */ break;
      case OUTPUT_MLC_CASE_ASCII    : default_value = (gParameters.GetOutputClusterCaseAscii() ? "y" : "n"); break;
      case OUTPUT_MLC_CASE_DBASE    : default_value = (gParameters.GetOutputClusterCaseDBase() ? "y" : "n"); break;
      case STUDYPERIOD_DATACHECK    : AsString(default_value, gParameters.GetStudyPeriodDataCheckingType()); break;
      case COORDINATES_DATACHECK    : AsString(default_value, gParameters.GetCoordinatesDataCheckingType()); break;
      case MAXGEOPOPATRISK          : AsString(default_value, gParameters.GetMaxSpatialSizeForType(PERCENTOFPOPULATION, false));
      case MAXGEOPOPFILE            : AsString(default_value, gParameters.GetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false));
      case MAXGEODISTANCE           : AsString(default_value, gParameters.GetMaxSpatialSizeForType(MAXDISTANCE, false));
      case USE_MAXGEOPOPFILE        : default_value = (gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false) ? "y" : "n"); break;
      case USE_MAXGEODISTANCE       : default_value = (gParameters.GetRestrictMaxSpatialSizeForType(MAXDISTANCE, false) ? "y" : "n"); break;
      case MAXGEOPOPATRISK_REPORTED : AsString(default_value, gParameters.GetMaxSpatialSizeForType(PERCENTOFPOPULATION, true));
      case MAXGEOPOPFILE_REPORTED   : AsString(default_value, gParameters.GetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, true));
      case MAXGEODISTANCE_REPORTED  : AsString(default_value, gParameters.GetMaxSpatialSizeForType(MAXDISTANCE, true));
      case USE_MAXGEOPOPFILE_REPORTED: default_value = (gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFPOPULATION, true) ? "y" : "n"); break;
      case USE_MAXGEODISTANCE_REPORTED: default_value = (gParameters.GetRestrictMaxSpatialSizeForType(MAXDISTANCE, true) ? "y" : "n"); break;
      case LOCATION_NEIGHBORS_FILE  : default_value = "<blank>"; break;
      case USE_LOCATION_NEIGHBORS_FILE : default_value = (gParameters.UseLocationNeighborsFile() ? "y" : "n"); break;
      case RANDOMLY_GENERATE_SEED   : default_value = (gParameters.GetIsRandomlyGeneratingSeed() ? "y" : "n"); break;
      case MULTIPLE_COORDINATES_TYPE: AsString(default_value, gParameters.GetMultipleCoordinatesType()); break;
      case META_LOCATIONS_FILE      : default_value = "<blank>"; break;
      case USE_META_LOCATIONS_FILE  : default_value = (gParameters.UseMetaLocationsFile() ? "y" : "n"); break;
      default : throw parameter_error("Unknown parameter enumeration %d.", eParameterType);
    };

    if (default_value.size()) {
      gvParametersMissingDefaulted.push_back(static_cast<int>(eParameterType)); //and default retained.
      PrintDirection.Printf("Notice:\nThe parameter '%s' is missing from the parameter file, "
                            "defaulted value '%s' assigned.\n", BasePrint::P_NOTICE,
                            GetParameterLabel(eParameterType), default_value.c_str());
    }
  }
  catch (prg_exception & x) {
    x.addTrace("MarkAsMissingDefaulted()","AbtractParameterFileAccess");
    throw;
 }
}

/** Attempts to interpret passed string as a boolean value. Throws parameter_error. */
bool AbtractParameterFileAccess::ReadBoolean(const std::string& sValue, ParameterType eParameterType) const {
  bool          bReadResult;

  if (sValue.size() == 0) {
    throw parameter_error("Invalid Parameter Setting:\nParameter '%s' is not set.\n", GetParameterLabel(eParameterType));
  }
  else if (!(!stricmp(sValue.c_str(),"y")   || !stricmp(sValue.c_str(),"n") ||
             !strcmp(sValue.c_str(),"1")    || !strcmp(sValue.c_str(),"0")   ||
             !stricmp(sValue.c_str(),"yes")  || !stricmp(sValue.c_str(),"no"))) {
    throw parameter_error("Invalid Parameter Setting:\nFor parameter '%s', setting '%s' is invalid. Valid values are 'y' or 'n'.\n",
                          GetParameterLabel(eParameterType), sValue.c_str());
  }
  else
    bReadResult = (!stricmp(sValue.c_str(),"y") || !stricmp(sValue.c_str(),"yes") || !strcmp(sValue.c_str(),"1"));
  return bReadResult;
}


/** Set date parameter with passed string using appropriate set function. */
void AbtractParameterFileAccess::ReadDate(const std::string& sValue, ParameterType eParameterType) const {
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
                                   gParameters.SetProspectiveStartDate(sValue.c_str());
                                 break;
    case STARTDATE             : gParameters.SetStudyPeriodStartDate(sValue.c_str()); break;
    case ENDDATE               : gParameters.SetStudyPeriodEndDate(sValue.c_str()); break;
    default : throw prg_error("Parameter enumeration '%d' is not listed for date read.\n","ReadDate()", eParameterType);
  };
}

/** Attempts to interpret passed string as comma separated string of dates. Throws parameter_error. */
void AbtractParameterFileAccess::ReadDateRange(const std::string& sValue, ParameterType eParameterType, DateRange_t& Range) const {
  int   iNumTokens=0;

  if (sValue.size()) {
    boost::tokenizer<boost::escaped_list_separator<char> > identifiers(sValue);
    for (boost::tokenizer<boost::escaped_list_separator<char> >::const_iterator itr=identifiers.begin(); itr != identifiers.end(); ++itr) {
      if (iNumTokens == 0) Range.first = *itr;
      else if (iNumTokens == 1) Range.second = *itr;
      ++iNumTokens;
    }
     if (iNumTokens != 2)
       throw parameter_error("Invalid Parameter Setting:\nFor parameter '%s', %d values specified but should have 2.\n",
                             GetParameterLabel(eParameterType), iNumTokens);
  }
}

/** Attempts to interpret passed string as a double value. Throws parameter_error. */
double AbtractParameterFileAccess::ReadDouble(const std::string & sValue, ParameterType eParameterType) const {
  double        dReadResult;

  if (sValue.size() == 0) {
    throw parameter_error("Invalid Parameter Setting:\nParameter '%s' is not set.\n", GetParameterLabel(eParameterType));
  }
  if (sscanf(sValue.c_str(), "%lf", &dReadResult) != 1) {
    throw parameter_error("Invalid Parameter Setting:\nFor parameter '%s', setting '%s' is not a valid real number.\n",
                          GetParameterLabel(eParameterType), sValue.c_str());
  }
  return dReadResult;
}

/** Attempts to interpret passed string as a space/comma delimited string of integers that represent
    the number of rotations ellipse will make. No attempt to convert is made if no
    ellipses defined.  Throws parameter_error. */
void AbtractParameterFileAccess::ReadEllipseRotations(const std::string& sParameter) const {
  int   iNumTokens=0, iReadRotations;

  if (sParameter.size()) {
    boost::escaped_list_separator<char> separator('\\', (sParameter.find(',') == sParameter.npos ? ' ' : ','), '\"');
    boost::tokenizer<boost::escaped_list_separator<char> > identifiers(sParameter, separator);
    for (boost::tokenizer<boost::escaped_list_separator<char> >::const_iterator itr=identifiers.begin(); itr != identifiers.end(); ++itr) {
       if (sscanf((*itr).c_str(), "%i", &iReadRotations)) {
         gParameters.AddEllipsoidRotations(iReadRotations, (iNumTokens == 0));
         ++iNumTokens;
       }
       else
         throw parameter_error("Invalid Parameter Setting:\nFor parameter '%s', setting '%s' is not an integer.\n", "ReadEllipseRotations()",
                               GetParameterLabel(ENUMBERS), (*itr).c_str());
    }
  }
}

/** Attempts to interpret passed string as a space delimited string of integers that represent
    the shape of each ellipsoid. No attempt to convert is made if there are no
    ellipses defined.  Throws InvalidParameterException. */
void AbtractParameterFileAccess::ReadEllipseShapes(const std::string& sParameter) const {
  int           iNumTokens=0;
  double        dReadShape;

  if (sParameter.size()) {
    boost::escaped_list_separator<char> separator('\\', (sParameter.find(',') == sParameter.npos ? ' ' : ','), '\"');
    boost::tokenizer<boost::escaped_list_separator<char> > identifiers(sParameter, separator);
    for (boost::tokenizer<boost::escaped_list_separator<char> >::const_iterator itr=identifiers.begin(); itr != identifiers.end(); ++itr) {
       if (sscanf((*itr).c_str(), "%lf", &dReadShape)) {
         gParameters.AddEllipsoidShape(dReadShape, (iNumTokens == 0));
         ++iNumTokens;
       }
       else
         throw parameter_error("Invalid Parameter Setting:\nFor parameter '%s', setting '%s' is not an decimal number.\n",
                               GetParameterLabel(ESHAPES), (*itr).c_str());
    }
  }
}

/** Attempts to validate integer as enumeration within specified range. Throws InvalidParameterException. */
int AbtractParameterFileAccess::ReadEnumeration(int iValue, ParameterType eParameterType, int iLow, int iHigh) const {
  if (iValue < iLow || iValue > iHigh)
    throw parameter_error("Invalid Parameter Setting:\nFor parameter '%s', setting '%d' is out of range [%d,%d].\n",
                          GetParameterLabel(eParameterType), iValue, iLow, iHigh);
  return iValue;
}

/** Attempts to interpret passed string as an integer value. Throws InvalidParameterException. */
int AbtractParameterFileAccess::ReadInt(const std::string& sValue, ParameterType eParameterType) const {
  int           iReadResult;

  if (sValue.size() == 0) {
    throw parameter_error("Invalid Parameter Setting:\nParameter '%s' is not set.\n", GetParameterLabel(eParameterType));
  }
  else if (sscanf(sValue.c_str(), "%i", &iReadResult) != 1) {
    throw parameter_error("Invalid Parameter Setting:\nFor parameter '%s', setting '%s' is not a valid integer.\n",
                          GetParameterLabel(eParameterType), sValue.c_str());
  }
  return iReadResult;
}

/** Attempts to interpret passed string as an integer value. Throws InvalidParameterException. */
int AbtractParameterFileAccess::ReadUnsignedInt(const std::string& sValue, ParameterType eParameterType) const {
  int           iReadResult;

  if (sValue.size() == 0) {
    throw parameter_error("Invalid Parameter Setting:\nParameter '%s' is not set.\n", GetParameterLabel(eParameterType));
  }
  else if (sscanf(sValue.c_str(), "%u", &iReadResult) != 1) {
    throw parameter_error("Invalid Parameter Setting:\nFor parameter '%s', setting '%s' is not a valid integer.\n",
                          GetParameterLabel(eParameterType), sValue.c_str());
  }
  else if (iReadResult < 0) {
    throw parameter_error("Invalid Parameter Setting:\nFor parameter '%s', setting '%s' is not a positive integer.\n",
                          GetParameterLabel(eParameterType), sValue.c_str());
  }
  return iReadResult;
}

/** Attempts to interpret passed string as version number of format '#.#.#'. Throws InvalidParameterException. */
void AbtractParameterFileAccess::ReadVersion(const std::string& sValue) const {
  CParameters::CreationVersion       vVersion;

   if (sValue.size() == 0)
     throw parameter_error("Invalid Parameter Setting:\nParameter '%s' is not set.\n", GetParameterLabel(CREATION_VERSION));
   else if (sscanf(sValue.c_str(), "%u.%u.%u", &vVersion.iMajor, &vVersion.iMinor, &vVersion.iRelease) < 3)
     throw parameter_error("Invalid Parameter Setting:\nParameter '%s' is not set.\n", GetParameterLabel(CREATION_VERSION));
  gParameters.SetVersion(vVersion);
}

/** Calls appropriate read and set function for parameter type. */
void AbtractParameterFileAccess::SetParameter(ParameterType eParameterType, const std::string& sParameter, BasePrint& PrintDirection) {
  int           iValue;
  DateRange_t   Range;

  try {
    switch (eParameterType) {
      case ANALYSISTYPE              : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, PURELYSPATIAL, PROSPECTIVEPURELYTEMPORAL);
                                       gParameters.SetAnalysisType((AnalysisType)iValue); break;
      case SCANAREAS                 : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, HIGH, HIGHANDLOW);
                                       gParameters.SetAreaRateType((AreaRateType)iValue); break;
      case CASEFILE                  : gParameters.SetCaseFileName(sParameter.c_str(), true); break;
      case POPFILE                   : gParameters.SetPopulationFileName(sParameter.c_str(), true); break;
      case COORDFILE                 : gParameters.SetCoordinatesFileName(sParameter.c_str(), true); break;
      case OUTPUTFILE                : gParameters.SetOutputFileName(sParameter.c_str(), true); break;
      case PRECISION                 : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, NONE, DAY);
                                       gParameters.SetPrecisionOfTimesType((DatePrecisionType)iValue); break;
      case DIMENSION                 : //Dimensions no longer read from file.
                                       break;
      case SPECIALGRID               : gParameters.SetUseSpecialGrid(ReadBoolean(sParameter, eParameterType)); break;
      case GRIDFILE                  : gParameters.SetSpecialGridFileName(sParameter.c_str(), true); break;
      case GEOSIZE                   : gdMaxSpatialClusterSize = ReadDouble(sParameter, eParameterType); break;
      case STARTDATE                 : ReadDate(sParameter, eParameterType); break;
      case ENDDATE                   : ReadDate(sParameter, eParameterType); break;
      case CLUSTERS                  : gParameters.SetIncludeClustersType((IncludeClustersType)ReadInt(sParameter, eParameterType)); break;
      case EXACTTIMES                : //No longer used. No documentation as to previous usage.
                                       break;
      case TIME_AGGREGATION_UNITS    : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, NONE, DAY);
                                       gParameters.SetTimeAggregationUnitsType((DatePrecisionType)iValue); break;
      case TIME_AGGREGATION          : gParameters.SetTimeAggregationLength((long)ReadInt(sParameter, eParameterType)); break;
      case PURESPATIAL               : gParameters.SetIncludePurelySpatialClusters(ReadBoolean(sParameter, eParameterType)); break;
      case TIMESIZE                  : gParameters.SetMaximumTemporalClusterSize(ReadDouble(sParameter, eParameterType)); break;
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
      case CONTROLFILE               : gParameters.SetControlFileName(sParameter.c_str(), true); break;
      case COORDTYPE                 : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, CARTESIAN, LATLON);
                                       gParameters.SetCoordinatesType((CoordinatesType)iValue); break;
      case OUTPUT_SIM_LLR_ASCII      : gParameters.SetOutputSimLogLikeliRatiosAscii(ReadBoolean(sParameter, eParameterType)); break;
      case ITERATIVE                 : gParameters.SetIterativeScanning(ReadBoolean(sParameter, eParameterType)); break;
      case ITERATIVE_NUM             : gParameters.SetNumIterativeScans(ReadUnsignedInt(sParameter, eParameterType)); break;
      case ITERATIVE_PVAL            : gParameters.SetIterativeCutOffPValue(ReadDouble(sParameter, eParameterType)); break;
      case VALIDATE                  : /* no longer used */ break;
      case OUTPUT_RR_ASCII           : gParameters.SetOutputRelativeRisksAscii(ReadBoolean(sParameter, eParameterType)); break;
      case WINDOW_SHAPE              : iValue = ReadInt(sParameter, eParameterType);
                                       //This parameter used to be 'number of ellipses' before v6.1, so set window shape to elliptic
                                       //if it is not zero and version warrants.
                                       if ((gParameters.GetCreationVersion().iMajor < 6 ||
                                           (gParameters.GetCreationVersion().iMajor == 6 && gParameters.GetCreationVersion().iMinor == 0)) && iValue > 1)
                                         iValue = 1;
                                       gParameters.SetSpatialWindowType((SpatialWindowType)ReadEnumeration(iValue, eParameterType, CIRCULAR, ELLIPTIC));
                                       break;
      case ESHAPES                   : ReadEllipseShapes(sParameter); break;
      case ENUMBERS                  : ReadEllipseRotations(sParameter); break;
      case START_PROSP_SURV          : ReadDate(sParameter, eParameterType); break;
      case OUTPUT_AREAS_ASCII        : gParameters.SetOutputAreaSpecificAscii(ReadBoolean(sParameter, eParameterType)); break;
      case OUTPUT_MLC_ASCII          : gParameters.SetOutputClusterLevelAscii(ReadBoolean(sParameter, eParameterType)); break;
      case CRITERIA_SECOND_CLUSTERS  : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, NOGEOOVERLAP, NORESTRICTIONS);
                                       gParameters.SetCriteriaForReportingSecondaryClusters((CriteriaSecondaryClustersType)iValue); break;
      case MAX_TEMPORAL_TYPE         : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, PERCENTAGETYPE, TIMETYPE);
                                       gParameters.SetMaximumTemporalClusterSizeType((TemporalSizeType)ReadInt(sParameter, eParameterType)); break;
      case MAX_SPATIAL_TYPE          : // The maximum spatial cluster size used to a choice between multiple options:
                                       //   enum SpatialSizeType {PERCENTOFPOPULATION=0, MAXDISTANCE, PERCENTOFMAXCIRCLEFILE};
                                       // but was updated in version 7.0 to permit selection of all simultaneously. To permit reading of
                                       // older parameter files, we need to mimic previous behavior.
                                       iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, PERCENTOFPOPULATION, PERCENTOFMAXCIRCLEFILE);
                                       gParameters.SetRestrictMaxSpatialSizeForType((SpatialSizeType)iValue, true, false);
                                       gParameters.SetMaxSpatialSizeForType((SpatialSizeType)iValue, gdMaxSpatialClusterSize, false); break;
      case RUN_HISTORY_FILENAME      : //Run History no longer scanned from parameters file. Set through setters/getters and copy() only.
                                       break;
      case OUTPUT_MLC_DBASE          : gParameters.SetOutputClusterLevelDBase(ReadBoolean(sParameter, eParameterType)); break;
      case OUTPUT_AREAS_DBASE        : gParameters.SetOutputAreaSpecificDBase(ReadBoolean(sParameter, eParameterType)); break;
      case OUTPUT_RR_DBASE           : gParameters.SetOutputRelativeRisksDBase(ReadBoolean(sParameter, eParameterType)); break;
      case OUTPUT_SIM_LLR_DBASE      : gParameters.SetOutputSimLogLikeliRatiosDBase(ReadBoolean(sParameter, eParameterType)); break;
      case NON_COMPACTNESS_PENALTY   : //This parameter used to be a boolean type, updated to be an enumeration in version 6.1.
                                       if (gParameters.GetCreationVersion().iMajor < 6 || (gParameters.GetCreationVersion().iMajor == 6 && gParameters.GetCreationVersion().iMinor == 0)) {
                                         iValue = ReadBoolean(sParameter, eParameterType);
                                         //if boolean true, set to full penalty - which is equivalent to previous versions
                                         if (iValue) iValue = STRONGPENALTY;
                                       }
                                       else
                                         iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, NOPENALTY, STRONGPENALTY);
                                       gParameters.SetNonCompactnessPenalty((NonCompactnessPenaltyType)iValue); break;
      case INTERVAL_STARTRANGE       : ReadDateRange(sParameter, eParameterType, Range);
                                       gParameters.SetStartRangeStartDate(Range.first.c_str());
                                       gParameters.SetStartRangeEndDate(Range.second.c_str()); break;
      case INTERVAL_ENDRANGE         : ReadDateRange(sParameter, eParameterType, Range);
                                       gParameters.SetEndRangeStartDate(Range.first.c_str());
                                       gParameters.SetEndRangeEndDate(Range.second.c_str()); break;
      case TIMETRENDCONVRG           : /*gParameters.SetTimeTrendConvergence(ReadDouble(sParameter, eParameterType));*/ break;
      case MAXCIRCLEPOPFILE          : gParameters.SetMaxCirclePopulationFileName(sParameter.c_str(), true); break;
      case EARLY_SIM_TERMINATION     : gParameters.SetTerminateSimulationsEarly(ReadBoolean(sParameter, eParameterType)); break;
      case REPORTED_GEOSIZE          : gdMaxReportedSpatialClusterSize = ReadDouble(sParameter, eParameterType); break;
      case USE_REPORTED_GEOSIZE      : gParameters.SetRestrictReportedClusters(ReadBoolean(sParameter, eParameterType)); break;
      case SIMULATION_TYPE           : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, STANDARD, FILESOURCE);
                                       gParameters.SetSimulationType((SimulationType)iValue); break;
      case SIMULATION_SOURCEFILE     : gParameters.SetSimulationDataSourceFileName(sParameter.c_str(), true); break;
      case ADJ_BY_RR_FILE            : gParameters.SetAdjustmentsByRelativeRisksFilename(sParameter.c_str(), true); break;
      case OUTPUT_SIMULATION_DATA    : gParameters.SetOutputSimulationData(ReadBoolean(sParameter, eParameterType)); break;
      case SIMULATION_DATA_OUTFILE   : gParameters.SetSimulationDataOutputFileName(sParameter.c_str(), true); break;
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
      case SUPPRESS_WARNINGS         : gParameters.SetSuppressingWarnings(ReadBoolean(sParameter, eParameterType)); break;
      case MAX_REPORTED_SPATIAL_TYPE : // The maximum spatial cluster size used to a choice between multiple options:
                                       //   enum SpatialSizeType {PERCENTOFPOPULATION=0, MAXDISTANCE, PERCENTOFMAXCIRCLEFILE};
                                       // but was updated in version 7.0 to permit selection of all simultaneously. To permit reading of
                                       // older parameter files, we need to mimic previous behavior.
                                       iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, PERCENTOFPOPULATION, PERCENTOFMAXCIRCLEFILE);
                                       gParameters.SetRestrictMaxSpatialSizeForType((SpatialSizeType)iValue, true, true);
                                       gParameters.SetMaxSpatialSizeForType((SpatialSizeType)iValue, gdMaxReportedSpatialClusterSize, true); break;
      case OUTPUT_MLC_CASE_ASCII     : gParameters.SetOutputClusterCaseAscii(ReadBoolean(sParameter, eParameterType)); break;
      case OUTPUT_MLC_CASE_DBASE     : gParameters.SetOutputClusterCaseDBase(ReadBoolean(sParameter, eParameterType)); break;
      case STUDYPERIOD_DATACHECK     : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, STRICTBOUNDS, RELAXEDBOUNDS);
                                       gParameters.SetStudyPeriodDataCheckingType((StudyPeriodDataCheckingType)iValue); break;
      case COORDINATES_DATACHECK     : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, STRICTCOORDINATES, RELAXEDCOORDINATES);
                                       gParameters.SetCoordinatesDataCheckingType((CoordinatesDataCheckingType)iValue); break;
      case MAXGEOPOPATRISK           : gParameters.SetMaxSpatialSizeForType(PERCENTOFPOPULATION, ReadDouble(sParameter, eParameterType), false); break;
      case MAXGEOPOPFILE             : gParameters.SetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, ReadDouble(sParameter, eParameterType), false); break;
      case MAXGEODISTANCE            : gParameters.SetMaxSpatialSizeForType(MAXDISTANCE, ReadDouble(sParameter, eParameterType), false); break;
      case USE_MAXGEOPOPFILE         : gParameters.SetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, ReadBoolean(sParameter, eParameterType), false); break;
      case USE_MAXGEODISTANCE        : gParameters.SetRestrictMaxSpatialSizeForType(MAXDISTANCE, ReadBoolean(sParameter, eParameterType), false); break;
      case MAXGEOPOPATRISK_REPORTED  : gParameters.SetMaxSpatialSizeForType(PERCENTOFPOPULATION, ReadDouble(sParameter, eParameterType), true); break;
      case MAXGEOPOPFILE_REPORTED    : gParameters.SetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, ReadDouble(sParameter, eParameterType), true); break;
      case MAXGEODISTANCE_REPORTED   : gParameters.SetMaxSpatialSizeForType(MAXDISTANCE, ReadDouble(sParameter, eParameterType), true); break;
      case USE_MAXGEOPOPFILE_REPORTED: gParameters.SetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, ReadBoolean(sParameter, eParameterType), true); break;
      case USE_MAXGEODISTANCE_REPORTED: gParameters.SetRestrictMaxSpatialSizeForType(MAXDISTANCE, ReadBoolean(sParameter, eParameterType), true); break;
      case LOCATION_NEIGHBORS_FILE   : gParameters.SetLocationNeighborsFileName(sParameter.c_str(), true); break;
      case USE_LOCATION_NEIGHBORS_FILE : gParameters.UseLocationNeighborsFile(ReadBoolean(sParameter, eParameterType)); break;
      case RANDOMLY_GENERATE_SEED    : gParameters.SetIsRandomlyGeneratingSeed(ReadBoolean(sParameter, eParameterType)); break;
      case MULTIPLE_COORDINATES_TYPE : iValue = ReadEnumeration(ReadInt(sParameter, eParameterType), eParameterType, ONEPERLOCATION, ALLLOCATIONS);
                                       gParameters.SetMultipleCoordinatesType((MultipleCoordinatesType)ReadInt(sParameter, eParameterType)); break;
      case META_LOCATIONS_FILE       : gParameters.setMetaLocationsFilename(sParameter.c_str(), true); break;
      case USE_META_LOCATIONS_FILE   : gParameters.UseMetaLocationsFile(ReadBoolean(sParameter, eParameterType)); break;
      default : throw parameter_error("Unknown parameter enumeration %d.", eParameterType);
    };
  }
  catch (parameter_error &x) {
    gbReadStatusError = true;
    PrintDirection.Printf(x.what(), BasePrint::P_PARAMERROR);
  }
  catch (prg_exception &x) {
    x.addTrace("SetParameter()","AbtractParameterFileAccess");
    throw;
  }
}

parameter_error::parameter_error(const char * format, ...) : resolvable_error() {
  va_list varArgs;
  va_start (varArgs, format);
  printStringArgs(__what, varArgs, format);
  va_end(varArgs);
}


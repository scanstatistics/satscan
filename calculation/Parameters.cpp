//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "Parameters.h"
#include "DataStreamHandler.h"

#define INCLUDE_RUN_HISTORY

const char*      YES                            	= "y";
const char*      NO                             	= "n";

const char*      INPUT_FILES_SECTION            	= "[Input Files]";
const char*      CASE_FILE_LINE                 	= "CaseFile";
const char*      CONTROL_FILE_LINE              	= "ControlFile";
const char*      POP_FILE_LINE                  	= "PopulationFile";
const char*      COORD_FILE_LINE                	= "CoordinatesFile";
const char*      GRID_FILE_LINE                 	= "GridFile";
const char*      USE_GRID_FILE_LINE             	= "UseGridFile";
const char*      PRECISION_TIMES_LINE           	= "PrecisionCaseTimes";
const char*      COORD_TYPE_LINE                	= "CoordinatesType";

const char*      ANALYSIS_SECTION               	= "[Analysis]";
const char*      ANALYSIS_TYPE_LINE             	= "AnalysisType";
const char*      MODEL_TYPE_LINE                	= "ModelType";
const char*      SCAN_AREAS_LINE                	= "ScanAreas";
const char*      START_DATE_LINE                	= "StartDate";
const char*      END_DATE_LINE                  	= "EndDate";
const char*      MONTE_CARLO_REPS_LINE          	= "MonteCarloReps";

const char*      TIME_PARAMS_SECTION            	= "[Time Parameters]";
const char*      TIME_AGGREGATION_UNITS_LINE          	= "TimeAggregationUnits";
const char*      TIME_AGGREGATION_LENGTH_LINE        	= "TimeAggregationLength";
const char*      PROSPECT_START_LINE            	= "ProspectiveStartDate";
const char*      TIME_TREND_ADJ_LINE            	= "TimeTrendAdjustmentType";
const char*      TIME_TREND_PERCENT_LINE        	= "TimeTrendPercentage";
const char*      TIME_TREND_CONVERGENCE_LINE            = "TimeTrendConvergence";
const char*      ADJUST_EALIER_ANALYSES_LINE            = "AdjustForEarlierAnalyses";

const char*      SCANNING_WINDOW_SECTION        	= "[Scanning Window]";
const char*      MAX_GEO_SIZE_LINE              	= "MaxGeographicSize";
const char*      MAX_GEO_INTERPRET_LINE         	= "MaxSpatialSizeInterpretation";
const char*      INCLUDE_PURE_TEMP_LINE         	= "IncludePurelyTemporal";
const char*      MAX_TEMP_SIZE_LINE             	= "MaxTemporalSize";
const char*      MAX_TEMP_INTERPRET_LINE       		= "MaxTemporalSizeInterpretation";
const char*      INCLUDE_PURELY_SPATIAL_LINE    	= "IncludePurelySpatial";
const char*      INLCUDE_CLUSTERS_LINE            	= "IncludeClusters";
const char*      STARTRANGE_LINE                        = "IntervalStartRange";
const char*      ENDRANGE_LINE                          = "IntervalEndRange";

const char*      OUTPUT_FILES_SECTION           	= "[Output Files]";
const char*      RESULTS_FILE_LINE              	= "ResultsFile";
const char*      OUTPUT_MLC_ASCII_LINE          	= "MostLikelyClusterEachCentroidASCII";
const char*      OUTPUT_MLC_DBASE_LINE          	= "MostLikelyClusterEachCentroidDBase";
const char*      OUTPUT_AREAS_ASCII_LINE        	= "CensusAreasReportedClustersASCII";
const char*      OUTPUT_AREAS_DBASE_LINE        	= "CensusAreasReportedClustersDBase";
const char*      OUTPUT_SIM_LLR_ASCII_LINE      	= "SaveSimLLRsASCII";
const char*      OUTPUT_SIM_LLR_DBASE_LINE      	= "SaveSimLLRsDBase";
const char*      OUTPUT_REL_RISKS_ASCII_LINE    	= "IncludeRelativeRisksCensusAreasASCII";
const char*      OUTPUT_REL_RISKS_DBASE_LINE    	= "IncludeRelativeRisksCensusAreasDBase";
const char*      CRIT_REPORT_SEC_CLUSTERS_LINE  	= "CriteriaForReportingSecondaryClusters";
const char*      REPORTED_GEOSIZE_LINE                  = "MaxReportedGeoClusterSize";
const char*      USE_REPORTED_GEOSIZE_LINE              = "UseReportOnlySmallerClusters";

const char*      SEQUENTIAL_SCAN_SECTION        	= "[Sequential Scan]";
const char*      SEQUENTIAL_SCAN_LINE           	= "SequentialScan";
const char*      SEQUENTIAL_MAX_ITERS_LINE      	= "SequentialScanMaxIterations";
const char*      SEQUENTIAL_MAX_PVALUE_LINE     	= "SequentialScanMaxPValue";

const char*      ELLIPSES_SECTION               	= "[Elliptic Scan]";
const char*      NUMBER_ELLIPSES_LINE           	= "NumberOfEllipses";
const char*      ELLIPSE_SHAPES_LINE            	= "EllipseShapes";
const char*      ELLIPSE_ANGLES_LINE            	= "EllipseAngles";
const char*      ELLIPSE_NON_COMPACTNESS_LINE   	= "NonCompactnessPenalty";

const char*      ADVANCED_FEATURES_SECTION      	= "[Advanced Features]";
const char*      VALID_PARAMS_LINE              	= "ValidateParameters";
const char*      ISOTONIC_SCAN_LINE             	= "IsotonicScan";
const char*      PVALUE_PROSPECT_LLR_LINE       	= "PValues2PrespecifiedLLRs";
const char*      LLR_1_LINE                     	= "LLR1";
const char*      LLR_2_LINE                    		= "LLR2";
const char*      EARLY_SIM_TERMINATION_LINE             = "EarlySimulationTermination";
const char*      SIMULATION_TYPE_LINE                   = "SimulatedDataMethodType";
const char*      SIMULATION_FILESOURCE_LINE             = "SimulatedDataInputFilename";
const char*      OUTPUT_SIMULATION_DATA_LINE            = "PrintSimulatedDataToFile";
const char*      SIMULATION_DATA_OUTFILE_LINE           = "SimulatedDataOutputFilename";
const char*      ADJUSTMENTS_BY_RR_FILE_LINE            = "AdjustmentsByKnownRelativeRisksFilename";
const char*      USE_ADJUSTMENTS_BY_RR_FILE_LINE        = "UseAdjustmentsByRRFile";
const char*      MAX_CIRCLE_POP_FILE_LINE               = "MaxCirclePopulationFile";
const char*      SPATIAL_ADJ_TYPE_LINE                  = "SpatialAdjustmentType";
const char*      MULTI_STREAM_PURPOSE_TYPE_LINE         = "MultipleDataStreamPurposeType";

/** system ini section */
const char*      SYSTEM_SECTION                         = "[System]";
const char*      CREATION_VERSION_LINE                  = "Version";

const int        MAXIMUM_SEQUENTIAL_ANALYSES    	= 32000;
const int        MAXIMUM_ELLIPSOIDS             	= 10;

const char*      PURELY_SPATIAL_ANALYSIS        	= "Purely Spatial";
const char*      RETROSPECTIVE_PURELY_TEMPORAL_ANALYSIS = "Retrospective Purely Temporal";
const char*      PROSPECTIVE_PURELY_TEMPORAL_ANALYSIS   = "Prospective Purely Temporal";
const char*      RETROSPECTIVE_SPACETIME_ANALYSIS 	= "Retrospective Space-Time";
const char*      PROSPECTIVE_SPACETIME_ANALYSIS 	= "Prospective Space-Time";
const char*      SPATIALVARIATION_TEMPORALTREND         = "Spatial Variation and Temporal Trends";

const char*      POISSON_MODEL                 		= "Poisson";
const char*      BERNOULLI_MODEL                	= "Bernoulli";
const char*      SPACETIME_PERMUTATION_MODEL    	= "Space-Time Permutation";
const char*      NORMAL_MODEL                           = "Normal";
const char*      SURVIVAL_MODEL                         = "Survival";
const char*      RANK_MODEL                             = "Rank";

int CParameters::giNumParameters 			= 68;

char mgsVariableLabels[69][100] = {
   "Analysis Type", "Scan Areas", "Case File", "Population File",
   "Coordinates File", "Results File", "Precision of Case Times",
   "Not applicable", "Grid File Use", "Grid File",
   "Maximum Geographic size", "Study Period Start Date",
   "Study Period End Date", "Include Clusters Type", "Exact Times",
   "Interval Units", "Interval Length", "Include Purely Spatial Cluster",
   "Maximum Temporal Size", "Replications", "Model Type", "Isotonic Scan",
   "p-Values for 2 Prospective LLR's", "LLR #1", "LLR #2",
   "Time Trend Adjustment Type", "Time Trend Percentage",
   "Include Purely Temporal Cluster", "Control File", "Coordinates Type",
   "Output Simulated Loglikelihood Ratios Ascii Format", "Sequential Scan",
   "Sequential Scan Max Iterations", "Sequential Scan Max p-Value",
   "Validate Parameters", "Output Relative Risks Ascii Format",
   "Number of Ellipses", "Ellipse Shapes", "Ellipse Angles",
   "Prospective Start Date", "Output Location Information Ascii Format",                        
   "Output Cluster Infomration Ascii Format", "Criteria for Reporting Secondary Clusters",
   "Maximum Temporal Cluster Size Type", "Maximum Geographic Cluster Size Type",
   "Analysis History File", "Output Cluster Information DBase Format",
   "Output Location Information DBase Format", "Output Relative Risks DBase Format",
   "Output Simulated Loglikelihood Ratios DBase Format",
   "Ellipsoid Non-Compactness Penalty", "Interval Start Range",
   "Interval End Range", "Time Trend Convergence", "Special Population File",
   "Early Termination of Simulations", "Maximum Reported Geographical Cluster Size",
   "Restrict Reported Max Geographical Cluster Size", "Simulation Method Type",
   "Simulated Data Import File", "Adjustments By Known Relative Risks File", "Printing Simulated Data",
   "Simulated Data Output File", "Adjust for Earlier Analyses", "Use Adjustments By Known Relative Risks File",
   "Spatial Adjustments Type", "Multiple Data Stream Purpose", "Version Number"
};

/** Constructor */
CParameters::CParameters() {
  SetDefaults();
}

/** Copy constructor */
CParameters::CParameters(const CParameters &other) {
   SetDefaults();
   Copy(other);
}

/** Destructor */
CParameters::~CParameters() {}

/** Overload assignment operator */
CParameters &CParameters::operator=(const CParameters &rhs) {
  try {
    if (this != &rhs) Copy(rhs);
  }
  catch (ZdException & x) {
    x.AddCallpath("operator=()","CParameters");
    throw;
  }
  return (*this);
}

/** If passed filename contains a slash, then assumes that path is complete and
    sInputFilename is not modified. If filename does not contain a slash, it is
    assumed that filename is located in same directory of parameter file.
    sInputFilename is reset to this location. Note that the primary reason for
    implementing this feature was to permit the program to be installed in any
    location and sample parameter files run immediately without having to edit
    input file paths. */
void CParameters::ConvertRelativePath(std::string & sInputFilename) {
  ZdFileName    fParameterFilename;                              
  ZdFileName    fFilename;
  std::string   sFile;

  try {
    if (! sInputFilename.empty()) {
      //Assume that if slashes exist, then this is a complete file path, so
      //we'll make no attempts to determine what path might be otherwise.
      if (sInputFilename.find(ZDFILENAME_SLASH) == sInputFilename.npos) {
        //If no slashes, then this file is assumed to be in same directory as parameters file.
        fParameterFilename.SetFullPath(GetSourceFileName().c_str());
        fFilename.SetFullPath(sInputFilename.c_str());
        fFilename.SetLocation(fParameterFilename.GetLocation());
        sInputFilename = fFilename.GetFullPath();
      }
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ConvertRelativePath()", "CParameters");
    throw;
  }
}

/** Copies all class variables from the given CParameters object (rhs) into this one */
void CParameters::Copy(const CParameters &rhs) {
  try {
    giNumberEllipses                    = rhs.giNumberEllipses;
    gvEllipseShapes                     = rhs.gvEllipseShapes;
    gvEllipseRotations                  = rhs.gvEllipseRotations;
    gbNonCompactnessPenalty             = rhs.gbNonCompactnessPenalty;
    glTotalNumEllipses                  = rhs.glTotalNumEllipses;
    geAnalysisType                      = rhs.geAnalysisType;
    geAreaScanRate                      = rhs.geAreaScanRate;
    geProbabiltyModelType               = rhs.geProbabiltyModelType;
    geRiskFunctionType                  = rhs.geRiskFunctionType;
    giReplications                      = rhs.giReplications;
    gbPowerCalculation                  = rhs.gbPowerCalculation;
    gdPower_X                           = rhs.gdPower_X;
    gdPower_Y                           = rhs.gdPower_Y;
    gsStudyPeriodStartDate              = rhs.gsStudyPeriodStartDate;
    gsStudyPeriodEndDate                = rhs.gsStudyPeriodEndDate;
    gfMaxGeographicClusterSize          = rhs.gfMaxGeographicClusterSize;
    gfMaxTemporalClusterSize            = rhs.gfMaxTemporalClusterSize;
    geIncludeClustersType               = rhs.geIncludeClustersType;
    geTimeAggregationUnitsType          = rhs.geTimeAggregationUnitsType;
    glTimeAggregationLength             = rhs.glTimeAggregationLength;
    geTimeTrendAdjustType               = rhs.geTimeTrendAdjustType;
    gdTimeTrendAdjustPercentage         = rhs.gdTimeTrendAdjustPercentage;
    gbIncludePurelySpatialClusters      = rhs.gbIncludePurelySpatialClusters;
    gbIncludePurelyTemporalClusters     = rhs.gbIncludePurelyTemporalClusters;
    gvCaseFilenames                     = rhs.gvCaseFilenames;
    gvControlFilenames                  = rhs.gvControlFilenames;
    gvPopulationFilenames               = rhs.gvPopulationFilenames;
    gsCoordinatesFileName               = rhs.gsCoordinatesFileName;
    gsSpecialGridFileName               = rhs.gsSpecialGridFileName;
    gbUseSpecialGridFile                = rhs.gbUseSpecialGridFile;
    gsMaxCirclePopulationFileName       = rhs.gsMaxCirclePopulationFileName;
    gePrecisionOfTimesType              = rhs.gePrecisionOfTimesType;
    giDimensionsOfData                  = rhs.giDimensionsOfData;
    geCoordinatesType                   = rhs.geCoordinatesType;
    gsOutputFileName                    = rhs.gsOutputFileName;
    gbOutputSimLogLikeliRatiosAscii     = rhs.gbOutputSimLogLikeliRatiosAscii;
    gbOutputRelativeRisksAscii          = rhs.gbOutputRelativeRisksAscii;
    gbSequentialRuns                    = rhs.gbSequentialRuns;
    giNumSequentialRuns                 = rhs.giNumSequentialRuns;
    gbSequentialCutOffPValue            = rhs.gbSequentialCutOffPValue;
    gbValidatePriorToCalc               = rhs.gbValidatePriorToCalc;
    gsProspectiveStartDate              = rhs.gsProspectiveStartDate;
    gbOutputAreaSpecificAscii           = rhs.gbOutputAreaSpecificAscii;
    gbOutputClusterLevelAscii           = rhs.gbOutputClusterLevelAscii;
    geCriteriaSecondClustersType        = rhs.geCriteriaSecondClustersType;
    geMaxTemporalClusterSizeType        = rhs.geMaxTemporalClusterSizeType;
    geMaxGeographicClusterSizeType      = rhs.geMaxGeographicClusterSizeType;
    gbOutputClusterLevelDBase           = rhs.gbOutputClusterLevelDBase;
    gbOutputAreaSpecificDBase           = rhs.gbOutputAreaSpecificDBase;
    gbOutputRelativeRisksDBase          = rhs.gbOutputRelativeRisksDBase;
    gbOutputSimLogLikeliRatiosDBase     = rhs.gbOutputSimLogLikeliRatiosDBase;
    gsRunHistoryFilename                = rhs.gsRunHistoryFilename;
    gbLogRunHistory                     = rhs.gbLogRunHistory;
    gsParametersSourceFileName          = rhs.gsParametersSourceFileName;
    gsEndRangeStartDate                 = rhs.gsEndRangeStartDate;
    gsEndRangeEndDate                   = rhs.gsEndRangeEndDate;
    gsStartRangeStartDate               = rhs.gsStartRangeStartDate;
    gsStartRangeEndDate                 = rhs.gsStartRangeEndDate;
    gdTimeTrendConverge			= rhs.gdTimeTrendConverge;
    gbEarlyTerminationSimulations       = rhs.gbEarlyTerminationSimulations;
    gbRestrictReportedClusters          = rhs.gbRestrictReportedClusters;
    gfMaxReportedGeographicClusterSize  = rhs.gfMaxReportedGeographicClusterSize;
    geSimulationType                    = rhs.geSimulationType;
    gsSimulationDataSourceFileName      = rhs.gsSimulationDataSourceFileName;
    gsAdjustmentsByRelativeRisksFileName= rhs.gsAdjustmentsByRelativeRisksFileName;
    gbOutputSimulationData              = rhs.gbOutputSimulationData;
    gsSimulationDataOutputFilename      = rhs.gsSimulationDataOutputFilename;
    gbAdjustForEarlierAnalyses          = rhs.gbAdjustForEarlierAnalyses;
    gbUseAdjustmentsForRRFile           = rhs.gbUseAdjustmentsForRRFile;
    geSpatialAdjustmentType             = rhs.geSpatialAdjustmentType;
    geMultipleStreamPurposeType         = rhs.geMultipleStreamPurposeType;
    gCreationVersion                    = rhs.gCreationVersion;
  }
  catch (ZdException & x) {
    x.AddCallpath("Copy()", "CParameters");
    throw;
  }
}

/** Prints time trend adjustment parameters, in a particular format, to passed ascii file. */
void CParameters::DisplayAdjustments(FILE* fp, const DataStreamHandler& StreamHandler) const {
  ZdString              sBuffer;
  AsciiPrintFormat      PrintFormat;

  try {
    //display temporal adjustments
    switch (geTimeTrendAdjustType) {
      case NOTADJUSTED :
        break;
      case NONPARAMETRIC :
        sBuffer = "Adjusted for time nonparametrically."; break;
      case LOGLINEAR_PERC :
        sBuffer.printf("of %0.2f%% per year.", fabs(gdTimeTrendAdjustPercentage));
        if (gdTimeTrendAdjustPercentage < 0)
          sBuffer.Insert("Adjusted for time with a decrease ", 0);
        else
          sBuffer.Insert("Adjusted for time with an increase ", 0);
        break;
      case CALCULATED_LOGLINEAR_PERC :
        DisplayCalculatedTimeTrend(fp, StreamHandler); break;
      case STRATIFIED_RANDOMIZATION  :
        sBuffer = "Adjusted for time by stratified randomization."; break;
      default :
        ZdException::Generate("Unknown time trend adjustment type '%d'\n.", "DisplayTimeAdjustments()", geTimeTrendAdjustType);
    }
    if (sBuffer.GetLength())
      PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
    //display spatial adjustments
    switch (geSpatialAdjustmentType) {
      case NO_SPATIAL_ADJUSTMENT :
        break;
      case SPATIALLY_STRATIFIED_RANDOMIZATION :
        sBuffer = "Adjusted for spatial orientation by stratified randomization.";
        PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer); break;
      default :
        ZdException::Generate("Unknown time trend adjustment type '%d'\n.", "DisplayTimeAdjustments()", geTimeTrendAdjustType);
    }
    //display space-time adjustments
    if (gbUseAdjustmentsForRRFile) {
        sBuffer = "Adjusted for known relative risks.";
        PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
    }    
  }
  catch (ZdException &x) {
    x.AddCallpath("DisplayAdjustments(FILE *)","CParameters");
    throw;
  }
}

/** Prints analysis type related information, in a particular format, to passed ascii file. */
void CParameters::DisplayAnalysisType(FILE* fp) const {
  try {
    switch (geAnalysisType) {
      case PURELYSPATIAL             : fprintf(fp, "Purely Spatial analysis\n"); break;
      case PURELYTEMPORAL            : fprintf(fp, "Retrospective Purely Temporal analysis\n"); break;
      case SPACETIME                 : fprintf(fp, "Retrospective Space-Time analysis\n"); break;
      case PROSPECTIVESPACETIME      : fprintf(fp, "Prospective Space-Time analysis\n"); break;
      case SPATIALVARTEMPTREND       : fprintf(fp, "Spatial Variation of Temporal Trends analysis\n"); break;
      case PROSPECTIVEPURELYTEMPORAL : fprintf(fp, "Prospective Purely Temporal analysis\n"); break;
      default : ZdException::Generate("Unknown analysis type '%d'.\n", "DisplayAnalysisType()", geAnalysisType);
    }

    fprintf(fp, "scanning for ");

    if (geRiskFunctionType == MONOTONERISK)
      fprintf(fp, "monotone ");

    fprintf(fp, "clusters with \n");

    switch (geAreaScanRate) {
      case HIGH       : fprintf(fp, "high rates"); break;
      case LOW        : fprintf(fp, "low rates"); break;
      case HIGHANDLOW : fprintf(fp, "high or low rates"); break;
      default : ZdException::Generate("Unknown area scan rate type '%d'.\n", "DisplayAnalysisType()", geAreaScanRate);
    }

    switch (geProbabiltyModelType) {
      case POISSON              : fprintf(fp, " using the Poisson model.\n"); break;
      case BERNOULLI            : fprintf(fp, " using the Bernoulli model.\n"); break;
      case SPACETIMEPERMUTATION : fprintf(fp, " using the Space-Time Permutation model.\n"); break;
      case NORMAL               : fprintf(fp, " using the Normal model.\n"); break;
      case SURVIVAL             : fprintf(fp, " using the Survival model.\n"); break;
      case RANK                 : fprintf(fp, " using the Rank model.\n"); break;
      default : ZdException::Generate("Unknown probabilty model type '%d'.\n", "DisplayAnalysisType()", geProbabiltyModelType);
    }

    if (geAnalysisType == SPACETIME || geAnalysisType == PROSPECTIVESPACETIME) {
      if (gbIncludePurelySpatialClusters && gbIncludePurelyTemporalClusters)
        fprintf(fp, "Analysis includes purely spatial and purely temporal clusters.\n");
      else if (gbIncludePurelySpatialClusters)
        fprintf(fp, "Analysis includes purely spatial clusters.\n");
      else if (gbIncludePurelyTemporalClusters)
        fprintf(fp, "Analysis includes purely temporal clusters.\n");
    }

    if (gbSequentialRuns)
      fprintf(fp, "Sequential analysis performed.\n");
  }
  catch (ZdException & x) {
    x.AddCallpath("DisplayAnalysisType()", "CParameters");
    throw;
  }
}

/** Prints parameters, in a particular format, to passed ascii file. */
void CParameters::DisplayParameters(FILE* fp, unsigned int iNumSimulationsCompleted, const DataStreamHandler& StreamHandler) const {
  int           i;
  size_t        t;
  ZdString      sBuffer;
  ZdFileName    AdditionalOutputFile(gsOutputFileName.c_str());

  try {
    AsciiPrintFormat::PrintSectionSeparatorString(fp, 1, 2);
    fprintf(fp, "PARAMETER SETTINGS\n\n");

    fprintf(fp, "Input Files\n");
    fprintf(fp, "-----------\n");
    fprintf(fp, "  Case File                  : %s\n", gvCaseFilenames[0].c_str());
    for (t=1; t < gvCaseFilenames.size(); ++t)
       fprintf(fp, "  Case File (stream %i)       : %s\n", t + 1, gvCaseFilenames[t].c_str());

    switch (geProbabiltyModelType) {
      case POISSON              : fprintf(fp, "  Population File            : %s\n", gvPopulationFilenames[0].c_str());
                                  for (t=1; t < gvPopulationFilenames.size(); ++t)
                                     fprintf(fp, "  Population File (stream %i) : %s\n", t + 1, gvPopulationFilenames[t].c_str());
                                  break;
      case BERNOULLI            : fprintf(fp, "  Control File               : %s\n", gvControlFilenames[0].c_str());
                                  for (t=1; t < gvControlFilenames.size(); ++t)
                                     fprintf(fp, "  Control File (stream %i)    : %s\n", t + 1, gvControlFilenames[t].c_str());
                                  break;
      case SPACETIMEPERMUTATION :
      case NORMAL               :
      case SURVIVAL             :
      case RANK                 : break;
      default : ZdException::Generate("Unknown probabilty model type '%d'.\n", "DisplayParameters()", geProbabiltyModelType);
    }

    fprintf(fp, "  Coordinates File           : %s\n", gsCoordinatesFileName.c_str());
    if (gbUseSpecialGridFile)
      fprintf(fp, "  Grid File                  : %s\n", gsSpecialGridFileName.c_str());
    if (geMaxGeographicClusterSizeType == PERCENTOFPOPULATIONFILETYPE)
      fprintf(fp, "  Max Circle Size File       : %s\n", gsMaxCirclePopulationFileName.c_str());
    if (geSimulationType == FILESOURCE)
      fprintf(fp, "  Simulated Data Import File : %s\n", gsSimulationDataSourceFileName.c_str());
    if(geSimulationType == HA_RANDOMIZATION || gbUseAdjustmentsForRRFile)
      fprintf(fp, "  Adjustments File           : %s\n", gsAdjustmentsByRelativeRisksFileName.c_str());

    fprintf(fp, "\n  Time Precision     : ");
    if (gePrecisionOfTimesType == NONE)
      fprintf(fp, "None\n");
    else {
      DatePrecisionType eType = (gCreationVersion.iMajor == 4 ? geTimeAggregationUnitsType : gePrecisionOfTimesType);
      switch (eType) {
        case YEAR  : fprintf(fp, "Year\n"); break;
        case MONTH : fprintf(fp, "Month\n"); break;
        case DAY   : fprintf(fp, "Day\n"); break;
        default : ZdException::Generate("Unknown date precision type '%d'.\n", "DisplayParameters()", eType);
      }
    }  

    fprintf(fp, "  Coordinates        : ");
    switch (geCoordinatesType) {
      case CARTESIAN : fprintf(fp, "Cartesian\n"); break;
      case LATLON    : fprintf(fp, "Latitude/Longitude\n"); break;
      default : ZdException::Generate("Unknown coordinated type '%d'.\n", "DisplayParameters()", geCoordinatesType);
    }
    if (GetNumDataStreams() > 1) {
      fprintf(fp, "  Purpose of Multiple Data Streams : ");
      switch (geMultipleStreamPurposeType) {
        case MULTIVARIATE : fprintf(fp, "Multivariate Analysis\n"); break;
        case ADJUSTMENT    : fprintf(fp, "Adjustment\n"); break;
        default : ZdException::Generate("Unknown purpose for multiple streams type '%d'.\n",
                                        "DisplayParameters()", geMultipleStreamPurposeType);
      }
    }
    
    fprintf(fp, "\nAnalysis\n");
    fprintf(fp, "--------\n");

    fprintf(fp, "  Type of Analysis    : %s\n", GetAnalysisTypeAsString());
    fprintf(fp, "  Probability Model   : %s\n", GetProbabiltyModelTypeAsString(geProbabiltyModelType));

    if (geAnalysisType != SPATIALVARTEMPTREND) {
      fprintf(fp, "  Scan for Areas with : ");
      switch (geAreaScanRate) {
        case HIGH       : fprintf(fp, "High Rates\n"); break;
        case LOW        : fprintf(fp, "Low Rates\n"); break;
        case HIGHANDLOW : fprintf(fp, "High or Low Rates\n"); break;
        default : ZdException::Generate("Unknown area scan rate type '%d'.\n", "DisplayParameters()", geAreaScanRate);
      }
    }

    fprintf(fp, "\n  Start Date : %s\n", gsStudyPeriodStartDate.c_str());
    fprintf(fp, "  End Date   : %s\n\n", gsStudyPeriodEndDate.c_str());

    fprintf(fp, "  Number of Replications : %u\n", giReplications);

    if (giNumberEllipses > 0) {
      fprintf(fp, "\nEllipses\n");
      fprintf(fp, "----------\n");
      fprintf(fp, "  Number of Ellipse Shapes Requested       : %i\n", giNumberEllipses);
      fprintf(fp, "  Shape for Each Ellipse                   : ");
      for (i=0; i < giNumberEllipses; ++i)
         fprintf(fp, "%g ", gvEllipseShapes[i]);
      fprintf(fp, "\n  Number of Angles for Each Ellipse Shape  : ");
      for (i=0; i < giNumberEllipses; ++i)
         fprintf(fp, "%i ", gvEllipseRotations[i]);
      fprintf(fp, "\n  Non-Compactness Penalty           : ");
      fprintf(fp, (gbNonCompactnessPenalty ? "Yes" : "No"));
    }
    fprintf(fp, "\n\nScanning Window\n");
    fprintf(fp, "---------------\n");

    if (geAnalysisType == PURELYSPATIAL || geAnalysisType == SPACETIME ||
        geAnalysisType == PROSPECTIVESPACETIME || geAnalysisType == SPATIALVARTEMPTREND) {
      fprintf(fp, "  Maximum Spatial Cluster Size          : %.2f", gfMaxGeographicClusterSize);
      switch (geMaxGeographicClusterSizeType) {
        case PERCENTOFPOPULATIONFILETYPE :
        case PERCENTOFPOPULATIONTYPE     : fprintf(fp, " %%\n"); break;
        case DISTANCETYPE            : fprintf(fp, (geCoordinatesType == CARTESIAN ? " Cartesian units\n" : " km\n")); break;
        default : ZdException::Generate("Unknown maximum spatial cluster size type '%d'.\n", "DisplayParameters()", geMaxGeographicClusterSizeType);
      }
    }

    if (geProbabiltyModelType != SPACETIMEPERMUTATION && (geAnalysisType == SPACETIME || geAnalysisType == PROSPECTIVESPACETIME)) {
       fprintf(fp, "  Also Include Purely Temporal Clusters : ");
       fprintf(fp, (gbIncludePurelyTemporalClusters ? "Yes\n" : "No\n"));
    }

    if (geAnalysisType == PURELYTEMPORAL || geAnalysisType == PROSPECTIVEPURELYTEMPORAL ||
        geAnalysisType == SPACETIME || geAnalysisType == PROSPECTIVESPACETIME) {
      fprintf(fp, "  Maximum Temporal Cluster Size         : %.2f", gfMaxTemporalClusterSize);
      switch (geMaxTemporalClusterSizeType) {
        case PERCENTAGETYPE :
          fprintf(fp, " %%\n"); break;
        case TIMETYPE       :
          fprintf(fp, " %s\n", GetDatePrecisionAsString(geTimeAggregationUnitsType, sBuffer, gfMaxTemporalClusterSize != 1, true)); break;
        default :
          ZdException::Generate("Unknown maximum temporal cluster size type '%d'.\n", "DisplayParameters()", geMaxTemporalClusterSizeType);
      }
    }

    if (geProbabiltyModelType != SPACETIMEPERMUTATION && (geAnalysisType == SPACETIME || geAnalysisType == PROSPECTIVESPACETIME)) {
      fprintf(fp, "  Also Include Purely Spatial Clusters  : ");
      fprintf(fp, (gbIncludePurelySpatialClusters ? "Yes\n" : "No\n"));
    }

    //The "Clusters to Include" do not apply to PROSPECTIVESPACETIME
    if (geAnalysisType == PURELYTEMPORAL || geAnalysisType == SPACETIME)  {
      switch (geIncludeClustersType) {
         case ALIVECLUSTERS   : fprintf(fp, "  Clusters to Include                   : ");
                                fprintf(fp, "Only those including the study end date\n"); break;
         case ALLCLUSTERS     : /*fprintf(fp, "All\n");
                                  -- geIncludeClustersType parameter no longer visible in GUI,
                                     defaulted to ALLCLUSTERS, so don't print setting */ break;
         case CLUSTERSINRANGE : fprintf(fp, "  Flexible Temporal Window Definition   : start time in %s through %s\n",
                                        gsStartRangeStartDate.c_str(), gsStartRangeEndDate.c_str());
                                fprintf(fp, "                                          end time in %s through %s\n",
                                        gsEndRangeStartDate.c_str(), gsEndRangeEndDate.c_str()); break;
         default : ZdException::Generate("Unknown inclusion cluster type '%d'.\n", "DisplayParameters()", geIncludeClustersType);
      };
    }

    if (geAnalysisType == PURELYTEMPORAL || geAnalysisType == SPACETIME || geAnalysisType == PROSPECTIVEPURELYTEMPORAL||
        geAnalysisType == PROSPECTIVESPACETIME || geAnalysisType == SPATIALVARTEMPTREND) {
      fprintf(fp, "\nTime Parameters\n");
      fprintf(fp, "---------------\n");

      fprintf(fp, "  Time Aggregation Units  : ");
      switch (geTimeAggregationUnitsType) {
        case YEAR  : fprintf(fp, "Year\n"); break;
        case MONTH : fprintf(fp, "Month\n"); break;
        case DAY   : fprintf(fp, "Day\n"); break;
        default : ZdException::Generate("Unknown date precision type '%d'.\n", "DisplayParameters()", geTimeAggregationUnitsType);
      }

      fprintf(fp, "  Time Aggregation Length : %i\n\n", glTimeAggregationLength);

      fprintf(fp, "  Temporal Adjustment : ");
      switch (geTimeTrendAdjustType) {
         case NOTADJUSTED :
           fprintf(fp, "None\n"); break;
         case NONPARAMETRIC :
           fprintf(fp, "Nonparametric\n"); break;
         case LOGLINEAR_PERC :
           fprintf(fp, "Log linear with %g%% per year\n", gdTimeTrendAdjustPercentage);
           break;
         case CALCULATED_LOGLINEAR_PERC :
           fprintf(fp, "Log linear with automatically calculated trend\n");
           break;
         case STRATIFIED_RANDOMIZATION :
           fprintf(fp, "Nonparametric, with time stratified randomization\n");
           break;
         default :
           ZdException::Generate("Unknown time trend adjustment type '%d'.\n", "DisplayParameters()", geTimeTrendAdjustType);
      }
    }

    if (geAnalysisType == PROSPECTIVESPACETIME || geAnalysisType == PROSPECTIVEPURELYTEMPORAL) {
      fprintf(fp, "  Adjusted for Earlier Analyses : %s\n", (gbAdjustForEarlierAnalyses ? "Yes" : "No"));
      if (gbAdjustForEarlierAnalyses)
        fprintf(fp, "  Prospective Start Date : %s\n", gsProspectiveStartDate.c_str());
    }
    fprintf(fp, "  Spatial Adjustment : ");
    switch (geSpatialAdjustmentType) {
      case NO_SPATIAL_ADJUSTMENT :
        fprintf(fp, "None\n"); break;
      case SPATIALLY_STRATIFIED_RANDOMIZATION :
        fprintf(fp, "Spatial adjustment by stratified randomization\n"); break;
      default :
        ZdException::Generate("Unknown spatial adjustment type '%d'.\n", "DisplayParameters()", geSpatialAdjustmentType);
    }

    fprintf(fp, "\nOutput\n");
    fprintf(fp, "------\n");
    fprintf(fp,   "  Results File          : %s\n", gsOutputFileName.c_str());
    // cluster information files
    if (gbOutputClusterLevelAscii) {
      AdditionalOutputFile.SetExtension(".col.txt");
      fprintf(fp, "  Cluster File          : %s\n", AdditionalOutputFile.GetFullPath());
    }
    if (gbOutputClusterLevelDBase) {
      AdditionalOutputFile.SetExtension(".col.dbf");
      fprintf(fp, "  Cluster File          : %s\n", AdditionalOutputFile.GetFullPath());
    }
    // area specific files
    if (gbOutputAreaSpecificAscii) {
      AdditionalOutputFile.SetExtension(".gis.txt");
      fprintf(fp, "  Location File         : %s\n", AdditionalOutputFile.GetFullPath());
    }
    if (gbOutputAreaSpecificDBase) {
      AdditionalOutputFile.SetExtension(".gis.dbf");
      fprintf(fp, "  Location File         : %s\n", AdditionalOutputFile.GetFullPath());
    }
    // relative risk files
    if (gbOutputRelativeRisksAscii) {
      AdditionalOutputFile.SetExtension(".rr.txt");
      fprintf(fp, "  Relative Risks File   : %s\n", AdditionalOutputFile.GetFullPath());
    }
    if (gbOutputRelativeRisksDBase) {
      AdditionalOutputFile.SetExtension(".rr.dbf");
      fprintf(fp, "  Relative Risks File   : %s\n", AdditionalOutputFile.GetFullPath());
    }
    // loglikelihood ratio files
    if (gbOutputSimLogLikeliRatiosAscii) {
      AdditionalOutputFile.SetExtension(".llr.txt");
      fprintf(fp, "  Simulated LLRs File   : %s\n", AdditionalOutputFile.GetFullPath());
    }
    if (gbOutputSimLogLikeliRatiosDBase) {
      AdditionalOutputFile.SetExtension(".llr.dbf");
      fprintf(fp, "  Simulated LLRs File   : %s\n", AdditionalOutputFile.GetFullPath());
    }
    if (gbOutputSimulationData)
      fprintf(fp, "  Simulated Data Output File : %s\n", gsSimulationDataOutputFilename.c_str());

    if (!(geAnalysisType == PURELYTEMPORAL || geAnalysisType == PROSPECTIVEPURELYTEMPORAL)) {
      fprintf(fp, "\n  Criteria for Reporting Secondary Clusters : ");
      switch (geCriteriaSecondClustersType) {
         case NOGEOOVERLAP          : fprintf(fp, "No Geographical Overlap\n"); break;
         case NOCENTROIDSINOTHER    : fprintf(fp, "No Cluster Centroids in Other Clusters\n"); break;
         case NOCENTROIDSINMORELIKE : fprintf(fp, "No Cluster Centroids in More Likely Clusters\n"); break;
         case NOCENTROIDSINLESSLIKE : fprintf(fp, "No Cluster Centroids in Less Likely Clusters\n"); break;
         case NOPAIRSINEACHOTHERS   : fprintf(fp, "No Pairs of Centroids Both in Each Others Clusters\n"); break;
         case NORESTRICTIONS        : fprintf(fp, "No Restrictions = Most Likely Cluster for Each Centroid\n"); break;
         default : ZdException::Generate("Unknown secondary clusters type '%d'.\n", "DisplayParameters()", geCriteriaSecondClustersType);
      }
    }

    if (gbRestrictReportedClusters)
      fprintf(fp, "  Only clusters smaller than %g %s reported.\n", gfMaxReportedGeographicClusterSize,
                  (geMaxGeographicClusterSizeType == DISTANCETYPE ?
                    (geCoordinatesType == CARTESIAN ? "Cartesian units" : "km") : "percent of population at risk"));

    AsciiPrintFormat::PrintSectionSeparatorString(fp, 1, 1);
  }
  catch (ZdException &x) {
    x.AddCallpath("DisplayParameters(FILE *)","CParameters");
    throw;
  }
}

/** Prints calculated time trend adjustment parameters, in a particular format, to passed ascii file. */
void CParameters::DisplayCalculatedTimeTrend(FILE* fp, const DataStreamHandler& StreamHandler) const {
  unsigned int                  t, iStart, iScan;
  ZdString                      sPrintString, sWorkString;
  std::deque<unsigned int>      TrendIncrease, TrendDecrease;

  if (geTimeTrendAdjustType != CALCULATED_LOGLINEAR_PERC)
    return;

  //NOTE: Each data stream has own calculated time trend.

  if (StreamHandler.GetNumStreams() == 1) {
    if (StreamHandler.GetStream(0).GetCalculatedTimeTrendPercentage() < 0)
      sPrintString.printf("Adjusted for time trend with an annual decrease ");
    else
      sPrintString.printf("Adjusted for time trend with an annual increase ");
    sWorkString.printf("of %0.2f%%.", fabs(StreamHandler.GetStream(0).GetCalculatedTimeTrendPercentage()));
    sPrintString << sWorkString;
  }
  else {//multiple streams print
    //count number of increasing and decreasing trends
    for (t=0; t < StreamHandler.GetNumStreams(); ++t) {
       if (StreamHandler.GetStream(t).GetCalculatedTimeTrendPercentage() < 0)
         TrendDecrease.push_back(t);
       else
         TrendIncrease.push_back(t);
    }
    //now print
    sPrintString.printf("Adjusted for time trend with an annual ");
    //print increasing trends first
    if (TrendIncrease.size()) {
       sWorkString.printf("increase of %0.2f%%",
                          fabs(StreamHandler.GetStream(TrendIncrease.front()).GetCalculatedTimeTrendPercentage()));
       sPrintString << sWorkString;
       for (t=1; t < TrendIncrease.size(); ++t) {
          sWorkString.printf((t < TrendIncrease.size() - 1) ? ", %0.2f%%" : " and %0.2f%%",
                             fabs(StreamHandler.GetStream(TrendIncrease[t]).GetCalculatedTimeTrendPercentage()));
          sPrintString << sWorkString;
       }
       sWorkString.printf(" for data stream%s %u", (TrendIncrease.size() == 1 ? "" : "s"), TrendIncrease.front() + 1);
       sPrintString << sWorkString;
       for (t=1; t < TrendIncrease.size(); ++t) {
          sWorkString.printf((t < TrendIncrease.size() - 1 ? ", %u" : " and %u"), TrendIncrease[t] + 1);
          sPrintString << sWorkString;
       }
       sWorkString.printf((TrendIncrease.size() > 1 ? " respectively" : ""));
       sPrintString << sWorkString;
       sWorkString.printf((TrendDecrease.size() > 0 ? " and an annual " : "."));
       sPrintString << sWorkString;
    }
    //print decreasing trends
    if (TrendDecrease.size()) {
      sWorkString.printf("decrease of %0.2f%%",
                         fabs(StreamHandler.GetStream(TrendDecrease.front()).GetCalculatedTimeTrendPercentage()));
      sPrintString << sWorkString;
      for (t=1; t < TrendDecrease.size(); ++t) {
         sWorkString.printf((t < TrendDecrease.size() - 1) ? ", %0.2f%%" : " and %0.2f%%",
                            fabs(StreamHandler.GetStream(TrendDecrease[t]).GetCalculatedTimeTrendPercentage()));
         sPrintString << sWorkString;
      }
      sWorkString.printf(" for data stream%s %u", (TrendDecrease.size() == 1 ? "" : "s"), TrendDecrease.front() + 1);
      sPrintString << sWorkString;
      for (t=1; t < TrendDecrease.size(); ++t) {
         sWorkString.printf((t < TrendDecrease.size() - 1 ? ", %u" : " and %u"), TrendDecrease[t] + 1);
         sPrintString << sWorkString;
      }
      sWorkString.printf((TrendDecrease.size() > 1 ? " respectively." : "."));
      sPrintString << sWorkString;
    }
  }
  AsciiPrintFormat PrintFormat;
  PrintFormat.SetMarginsAsOverviewSection();
  PrintFormat.PrintAlignedMarginsDataString(fp, sPrintString);
}

/** Returns analysis type as string. */
const char * CParameters::GetAnalysisTypeAsString() const {
  const char * sAnalysisType;

  try {
    switch (geAnalysisType) {
      case PURELYSPATIAL             : sAnalysisType = PURELY_SPATIAL_ANALYSIS; break;
      case PURELYTEMPORAL            : sAnalysisType = RETROSPECTIVE_PURELY_TEMPORAL_ANALYSIS; break;
      case SPACETIME                 : sAnalysisType = RETROSPECTIVE_SPACETIME_ANALYSIS; break;
      case PROSPECTIVESPACETIME      : sAnalysisType = PROSPECTIVE_SPACETIME_ANALYSIS; break;
      case SPATIALVARTEMPTREND       : sAnalysisType = SPATIALVARIATION_TEMPORALTREND; break;
      case PROSPECTIVEPURELYTEMPORAL : sAnalysisType = PROSPECTIVE_PURELY_TEMPORAL_ANALYSIS; break;
      default : ZdException::Generate("Unknown analysis type '%d'.\n", "GetAnalysisTypeAsString()", geAnalysisType);
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("GetAnalysisTypeAsString()","CParameters");
    throw;
  }
  return sAnalysisType;
}

const std::string & CParameters::GetCaseFileName(unsigned int iStream) const {
  try {
    if (!iStream || iStream > gvCaseFilenames.size())
      ZdGenerateException("Index out of range.","GetCaseFileName()");
  }
  catch (ZdException & x) {
    x.AddCallpath("GetCaseFileName()","CParameters");
    throw;
  }
  return gvCaseFilenames[iStream - 1];
}

const std::string & CParameters::GetControlFileName(unsigned int iStream) const {
  try {
    if (!iStream || iStream > gvControlFilenames.size())
      ZdGenerateException("Index out of range.","GetControlFileName()");
  }
  catch (ZdException & x) {
    x.AddCallpath("GetControlFileName()","CParameters");
    throw;
  }
  return gvControlFilenames[iStream - 1];
}

/** Returns whether analysis is a prospective analysis. */
bool CParameters::GetIsProspectiveAnalysis() const {
  return (geAnalysisType == PROSPECTIVESPACETIME || geAnalysisType == PROSPECTIVEPURELYTEMPORAL);
}

/** Returns whether analysis is purely temporal. */
bool CParameters::GetIsPurelyTemporalAnalysis() const {
  return (geAnalysisType == PURELYTEMPORAL || geAnalysisType == PROSPECTIVEPURELYTEMPORAL);
}

/** Returns whether analysis is space-time. */
bool CParameters::GetIsSpaceTimeAnalysis() const {
  return (geAnalysisType == SPACETIME || geAnalysisType == PROSPECTIVESPACETIME);
}

/** Returns description for LLR. */
bool CParameters::GetLogLikelihoodRatioIsTestStatistic() const {
  return (geProbabiltyModelType == SPACETIMEPERMUTATION || (giNumberEllipses && gbNonCompactnessPenalty));
}

/** Returns whether any area specific files are outputed. */
bool CParameters::GetOutputAreaSpecificFiles() const  {
  return gbOutputAreaSpecificAscii || gbOutputAreaSpecificDBase;
}

/** Returns whether any cluster level files are outputed. */
bool CParameters::GetOutputClusterLevelFiles() const {
  return gbOutputClusterLevelAscii || gbOutputClusterLevelDBase;
}

/** Returns whether any relative risk files are outputed. */
bool CParameters::GetOutputRelativeRisksFiles() const {
  return gbOutputRelativeRisksAscii || gbOutputRelativeRisksDBase;
}

/** Returns whether any simulated loglikelihood ratio files are outputed. */
bool CParameters::GetOutputSimLoglikeliRatiosFiles() const {
  return gbOutputSimLogLikeliRatiosAscii || gbOutputSimLogLikeliRatiosDBase;
}

/** Return label for parameter line.*/
const char * CParameters::GetParameterLineLabel(ParameterType eParameterType, ZdString & sParameterLineLabel, bool bIniSectionName) const {
  try {
    if (bIniSectionName) {
      switch (eParameterType) {
        case ANALYSISTYPE              : sParameterLineLabel = ANALYSIS_TYPE_LINE; break;
        case SCANAREAS                 : sParameterLineLabel = SCAN_AREAS_LINE; break;
        case CASEFILE                  : sParameterLineLabel = CASE_FILE_LINE; break;
        case POPFILE                   : sParameterLineLabel = POP_FILE_LINE; break;
        case COORDFILE                 : sParameterLineLabel = COORD_FILE_LINE; break;
        case OUTPUTFILE                : sParameterLineLabel = RESULTS_FILE_LINE; break;
        case PRECISION                 : sParameterLineLabel = PRECISION_TIMES_LINE; break;
        case DIMENSION                 : sParameterLineLabel = "no label"; break;
        case SPECIALGRID               : sParameterLineLabel = USE_GRID_FILE_LINE; break;
        case GRIDFILE                  : sParameterLineLabel = GRID_FILE_LINE; break;
        case GEOSIZE                   : sParameterLineLabel = MAX_GEO_SIZE_LINE; break;
        case STARTDATE                 : sParameterLineLabel = START_DATE_LINE; break;
        case ENDDATE                   : sParameterLineLabel = END_DATE_LINE; break;
        case CLUSTERS                  : sParameterLineLabel = INLCUDE_CLUSTERS_LINE; break;
        case EXACTTIMES                : sParameterLineLabel = "no label"; break;
        case TIME_AGGREGATION_UNITS    : sParameterLineLabel = TIME_AGGREGATION_UNITS_LINE; break;
        case TIME_AGGREGATION          : sParameterLineLabel = TIME_AGGREGATION_LENGTH_LINE; break;
        case PURESPATIAL               : sParameterLineLabel = INCLUDE_PURELY_SPATIAL_LINE; break;
        case TIMESIZE                  : sParameterLineLabel = MAX_TEMP_SIZE_LINE; break;
        case REPLICAS                  : sParameterLineLabel = MONTE_CARLO_REPS_LINE; break;
        case MODEL                     : sParameterLineLabel = MODEL_TYPE_LINE; break;
        case RISKFUNCTION              : sParameterLineLabel = ISOTONIC_SCAN_LINE; break;
        case POWERCALC                 : sParameterLineLabel = PVALUE_PROSPECT_LLR_LINE; break;
        case POWERX                    : sParameterLineLabel = LLR_1_LINE; break;
        case POWERY                    : sParameterLineLabel = LLR_2_LINE; break;
        case TIMETREND                 : sParameterLineLabel = TIME_TREND_ADJ_LINE; break;
        case TIMETRENDPERC             : sParameterLineLabel = TIME_TREND_PERCENT_LINE; break;
        case PURETEMPORAL              : sParameterLineLabel = INCLUDE_PURE_TEMP_LINE; break;
        case CONTROLFILE               : sParameterLineLabel = CONTROL_FILE_LINE; break;
        case COORDTYPE                 : sParameterLineLabel = COORD_TYPE_LINE; break;
        case OUTPUT_SIM_LLR_ASCII      : sParameterLineLabel = OUTPUT_SIM_LLR_ASCII_LINE; break;
        case SEQUENTIAL                : sParameterLineLabel = SEQUENTIAL_SCAN_LINE; break;
        case SEQNUM                    : sParameterLineLabel = SEQUENTIAL_MAX_ITERS_LINE; break;
        case SEQPVAL                   : sParameterLineLabel = SEQUENTIAL_MAX_PVALUE_LINE; break;
        case VALIDATE                  : sParameterLineLabel = VALID_PARAMS_LINE; break;
        case OUTPUT_RR_ASCII           : sParameterLineLabel = OUTPUT_REL_RISKS_ASCII_LINE; break;
        case ELLIPSES                  : sParameterLineLabel = NUMBER_ELLIPSES_LINE; break;
        case ESHAPES                   : sParameterLineLabel = ELLIPSE_SHAPES_LINE; break;
        case ENUMBERS                  : sParameterLineLabel = ELLIPSE_ANGLES_LINE; break;
        case START_PROSP_SURV          : sParameterLineLabel = PROSPECT_START_LINE; break;
        case OUTPUT_AREAS_ASCII        : sParameterLineLabel = OUTPUT_AREAS_ASCII_LINE; break;
        case OUTPUT_MLC_ASCII          : sParameterLineLabel = OUTPUT_MLC_ASCII_LINE; break;
        case CRITERIA_SECOND_CLUSTERS  : sParameterLineLabel = CRIT_REPORT_SEC_CLUSTERS_LINE; break;
        case MAX_TEMPORAL_TYPE         : sParameterLineLabel = MAX_TEMP_INTERPRET_LINE; break;
        case MAX_SPATIAL_TYPE          : sParameterLineLabel = MAX_GEO_INTERPRET_LINE; break;
        case RUN_HISTORY_FILENAME      : sParameterLineLabel = "no label"; break;
        case OUTPUT_MLC_DBASE          : sParameterLineLabel = OUTPUT_MLC_DBASE_LINE; break;
        case OUTPUT_AREAS_DBASE        : sParameterLineLabel = OUTPUT_AREAS_DBASE_LINE; break;
        case OUTPUT_RR_DBASE           : sParameterLineLabel = OUTPUT_REL_RISKS_DBASE_LINE; break;
        case OUTPUT_SIM_LLR_DBASE      : sParameterLineLabel = OUTPUT_SIM_LLR_DBASE_LINE; break;
        case NON_COMPACTNESS_PENALTY   : sParameterLineLabel = ELLIPSE_NON_COMPACTNESS_LINE; break;
	case INTERVAL_STARTRANGE       : sParameterLineLabel = STARTRANGE_LINE; break;			
	case INTERVAL_ENDRANGE         : sParameterLineLabel = ENDRANGE_LINE; break;			
        case TIMETRENDCONVRG           : sParameterLineLabel = TIME_TREND_CONVERGENCE_LINE; break;
        case MAXCIRCLEPOPFILE          : sParameterLineLabel = MAX_CIRCLE_POP_FILE_LINE; break;
        case EARLY_SIM_TERMINATION     : sParameterLineLabel = EARLY_SIM_TERMINATION_LINE; break;
        case REPORTED_GEOSIZE          : sParameterLineLabel = REPORTED_GEOSIZE_LINE; break;
        case USE_REPORTED_GEOSIZE      : sParameterLineLabel = USE_REPORTED_GEOSIZE_LINE; break;
        case SIMULATION_TYPE           : sParameterLineLabel = SIMULATION_TYPE_LINE; break;
        case SIMULATION_SOURCEFILE     : sParameterLineLabel = SIMULATION_FILESOURCE_LINE; break;
        case ADJ_BY_RR_FILE            : sParameterLineLabel = ADJUSTMENTS_BY_RR_FILE_LINE; break;
        case OUTPUT_SIMULATION_DATA    : sParameterLineLabel = OUTPUT_SIMULATION_DATA_LINE; break;
        case SIMULATION_DATA_OUTFILE   : sParameterLineLabel = SIMULATION_DATA_OUTFILE_LINE; break;
        case ADJ_FOR_EALIER_ANALYSES   : sParameterLineLabel = ADJUST_EALIER_ANALYSES_LINE; break;
        case USE_ADJ_BY_RR_FILE        : sParameterLineLabel = USE_ADJUSTMENTS_BY_RR_FILE_LINE; break;
        case SPATIAL_ADJ_TYPE          : sParameterLineLabel = SPATIAL_ADJ_TYPE_LINE; break;
        case MULTI_STREAM_PURPOSE_TYPE : sParameterLineLabel = MULTI_STREAM_PURPOSE_TYPE_LINE; break;
        case CREATION_VERSION          : sParameterLineLabel = CREATION_VERSION_LINE; break;
        default : ZdException::Generate("Unknown parameter enumeration %d.\n", "GetParameterLineLabel()", eParameterType);
      };
    }
    else
      sParameterLineLabel.printf("%s (line %d)", mgsVariableLabels[eParameterType - 1], eParameterType);
  }
  catch (ZdException & x) {
    x.AddCallpath("GetParameterLineLabel()","CParameters");
    throw;
  }
  return sParameterLineLabel.GetCString();
}

/** returns whether analysis type permits inclusion of purely spatial cluster */
bool CParameters::GetPermitsPurelySpatialCluster(AnalysisType eAnalysisType) const {
  return geAnalysisType == PURELYSPATIAL || geAnalysisType == SPACETIME || GetIsProspectiveAnalysis();
}

/** returns whether probability model type permits inclusion of purely spatial cluster */
bool CParameters::GetPermitsPurelySpatialCluster(ProbabiltyModelType eModelType) const {
  return eModelType == POISSON || eModelType == BERNOULLI || eModelType == NORMAL
         || eModelType == SURVIVAL || eModelType == RANK;
}

/** returns whether analysis type permits inclusion of purely temporal cluster */
bool CParameters::GetPermitsPurelyTemporalCluster(AnalysisType eAnalysisType) const {
  return geAnalysisType == PURELYTEMPORAL || geAnalysisType == SPACETIME || GetIsProspectiveAnalysis();
}

/** returns whether probability model type permits inclusion of purely temporal cluster */
bool CParameters::GetPermitsPurelyTemporalCluster(ProbabiltyModelType eModelType) const {
  return eModelType == POISSON || eModelType == BERNOULLI || eModelType == NORMAL
         || eModelType == SURVIVAL || eModelType == RANK;
}

const std::string & CParameters::GetPopulationFileName(unsigned int iStream) const {
  try {
    if (!iStream || iStream > gvPopulationFilenames.size())
      ZdGenerateException("Index out of range.","GetPopulationFileName()");
  }
  catch (ZdException & x) {
    x.AddCallpath("GetPopulationFileName()","CParameters");
    throw;
  }
  return gvPopulationFilenames[iStream - 1];
}

/** Returns probabilty model type as a character array. */
const char * CParameters::GetProbabiltyModelTypeAsString(ProbabiltyModelType eProbabiltyModelType) const {
  const char * sProbabilityModel;

  try {
    switch (eProbabiltyModelType) {
      case POISSON              : sProbabilityModel = POISSON_MODEL; break;
      case BERNOULLI            : sProbabilityModel = BERNOULLI_MODEL; break;
      case SPACETIMEPERMUTATION : sProbabilityModel = SPACETIME_PERMUTATION_MODEL; break;
      case NORMAL               : sProbabilityModel = NORMAL_MODEL; break;
      case SURVIVAL             : sProbabilityModel = SURVIVAL_MODEL; break;
      case RANK                 : sProbabilityModel = RANK_MODEL; break;
      default : ZdException::Generate("Unknown probabilty model type '%d'.\n", "GetProbabiltyModelTypeAsString()", geProbabiltyModelType);
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("GetProbabiltyModelTypeAsString()","CParameters");
    throw;
  }
  return sProbabilityModel;
}

/** If passed filename has same path as passed parameter filename, returns
    'name.extension' else returns filename. */
const char * CParameters::GetRelativeToParameterName(const ZdFileName& fParameterName,
                                                     const std::string& sFilename,
                                                     ZdString& sValue) const {
  ZdFileName fInputFilename(sFilename.c_str());

  if (!stricmp(fInputFilename.GetLocation(), fParameterName.GetLocation()))
    sValue = fInputFilename.GetCompleteFileName();
  else
    sValue = sFilename.c_str();
  return sValue;
}

/** Prints message to print direction that parameter was missing when read from
    parameter file and that a default value as assigned. */
void CParameters::MarkAsMissingDefaulted(ParameterType eParameterType, BasePrint & PrintDirection) {
  ZdString      sDefaultValue, sParameterLineLabel;

  try {
    switch (eParameterType) {
      case ANALYSISTYPE             : sDefaultValue = geAnalysisType; break;
      case SCANAREAS                : sDefaultValue = geAreaScanRate; break;
      case CASEFILE                 : sDefaultValue = "<blank>"; break;
      case POPFILE                  : sDefaultValue = "<blank>"; break;
      case COORDFILE                : sDefaultValue = "<blank>"; break;
      case OUTPUTFILE               : sDefaultValue = "<blank>"; break;
      case PRECISION                : sDefaultValue = gePrecisionOfTimesType; break;
      case DIMENSION                : /*  */ break;
      case SPECIALGRID              : sDefaultValue = (gbUseSpecialGridFile ? YES : NO); break;
      case GRIDFILE                 : sDefaultValue = "<blank>"; break;
      case GEOSIZE                  : sDefaultValue = gfMaxGeographicClusterSize; break;
      case STARTDATE                : sDefaultValue = gsStudyPeriodStartDate.c_str(); break;
      case ENDDATE                  : sDefaultValue = gsStudyPeriodEndDate.c_str(); break;
      case CLUSTERS                 : sDefaultValue = geIncludeClustersType; break;
      case EXACTTIMES               : /* no longer used */ break;
      case TIME_AGGREGATION_UNITS   : sDefaultValue = geTimeAggregationUnitsType; break;
      case TIME_AGGREGATION         : sDefaultValue = glTimeAggregationLength; break;
      case PURESPATIAL              : sDefaultValue = (gbIncludePurelySpatialClusters ? YES : NO); break;
      case TIMESIZE                 : sDefaultValue = gfMaxTemporalClusterSize; break;
      case REPLICAS                 : sDefaultValue << giReplications; break;
      case MODEL                    : sDefaultValue = geProbabiltyModelType; break;
      case RISKFUNCTION             : sDefaultValue = geRiskFunctionType; break;
      case POWERCALC                : sDefaultValue = (gbPowerCalculation ? YES : NO); break;
      case POWERX                   : sDefaultValue = gdPower_X; break;
      case POWERY                   : sDefaultValue = gdPower_Y; break;
      case TIMETREND                : sDefaultValue = geTimeTrendAdjustType; break;
      case TIMETRENDPERC            : sDefaultValue = GetTimeTrendAdjustmentPercentage(); break;
      case PURETEMPORAL             : sDefaultValue = (gbIncludePurelyTemporalClusters ? YES : NO); break;
      case CONTROLFILE              : sDefaultValue = "<blank>"; break;
      case COORDTYPE                : sDefaultValue = geCoordinatesType; break;
      case OUTPUT_SIM_LLR_ASCII     : sDefaultValue = (gbOutputSimLogLikeliRatiosAscii ? YES : NO); break;
      case SEQUENTIAL               : sDefaultValue = (gbSequentialRuns ? YES : NO); break;
      case SEQNUM                   : sDefaultValue << giNumSequentialRuns; break;
      case SEQPVAL                  : sDefaultValue = gbSequentialCutOffPValue; break;
      case VALIDATE                 : sDefaultValue = (gbValidatePriorToCalc ? YES : NO); break;
      case OUTPUT_RR_ASCII          : sDefaultValue = (gbOutputRelativeRisksAscii ? YES : NO); break;
      case ELLIPSES                 : sDefaultValue = giNumberEllipses; break;
      case ESHAPES                  : sDefaultValue = "<blank>"; break;
      case ENUMBERS                 : sDefaultValue = "<blank>"; break;
      case START_PROSP_SURV         : sDefaultValue = gsProspectiveStartDate.c_str(); break;
      case OUTPUT_AREAS_ASCII       : sDefaultValue = (gbOutputAreaSpecificAscii ? YES : NO); break;
      case OUTPUT_MLC_ASCII         : sDefaultValue = (gbOutputClusterLevelAscii ? YES : NO); break;
      case CRITERIA_SECOND_CLUSTERS : sDefaultValue = geCriteriaSecondClustersType; break;
      case MAX_TEMPORAL_TYPE        : sDefaultValue = geMaxTemporalClusterSizeType; break;
      case MAX_SPATIAL_TYPE         : sDefaultValue = geMaxGeographicClusterSizeType; break;
      case RUN_HISTORY_FILENAME     : /* no longer read in from parameter file */ break;
      case OUTPUT_MLC_DBASE         : sDefaultValue = (gbOutputClusterLevelDBase ? YES : NO); break;
      case OUTPUT_AREAS_DBASE       : sDefaultValue = (gbOutputAreaSpecificDBase ? YES : NO); break;
      case OUTPUT_RR_DBASE          : sDefaultValue = (gbOutputRelativeRisksDBase ? YES : NO); break;
      case OUTPUT_SIM_LLR_DBASE     : sDefaultValue = (gbOutputSimLogLikeliRatiosDBase ? YES : NO); break;
      case NON_COMPACTNESS_PENALTY  : sDefaultValue = (gbNonCompactnessPenalty ? YES : NO); break;
      case INTERVAL_STARTRANGE      : sDefaultValue.printf("%s,%s", gsStartRangeStartDate.c_str(), gsStartRangeEndDate.c_str());
                                      break;
      case INTERVAL_ENDRANGE        : sDefaultValue.printf("%s,%s", gsEndRangeStartDate.c_str(), gsEndRangeEndDate.c_str());
                                      break;
      case TIMETRENDCONVRG	    : sDefaultValue = gdTimeTrendConverge; break;
      case MAXCIRCLEPOPFILE         : sDefaultValue = "<blank>"; break;
      case EARLY_SIM_TERMINATION    : sDefaultValue = (gbEarlyTerminationSimulations ? YES : NO); break;
      case REPORTED_GEOSIZE         : sDefaultValue = gfMaxReportedGeographicClusterSize; break;
      case USE_REPORTED_GEOSIZE     : sDefaultValue = (gbRestrictReportedClusters ? YES : NO); break;
      case SIMULATION_TYPE          : sDefaultValue = geSimulationType; break;
      case SIMULATION_SOURCEFILE    : sDefaultValue = "<blank>"; break;
      case ADJ_BY_RR_FILE           : sDefaultValue = "<blank>"; break;
      case OUTPUT_SIMULATION_DATA   : sDefaultValue = (gbOutputSimulationData ? YES : NO); break;
      case SIMULATION_DATA_OUTFILE  : sDefaultValue = "<blank>"; break;
      case ADJ_FOR_EALIER_ANALYSES  : sDefaultValue = (gbAdjustForEarlierAnalyses ? YES : NO); break;
      case USE_ADJ_BY_RR_FILE       : sDefaultValue = (gbUseAdjustmentsForRRFile ? YES : NO); break;
      case SPATIAL_ADJ_TYPE         : sDefaultValue = geSpatialAdjustmentType; break;
      case MULTI_STREAM_PURPOSE_TYPE: sDefaultValue = geMultipleStreamPurposeType; break;
      case CREATION_VERSION         : sDefaultValue.printf("%u.%u.%u", gCreationVersion.iMajor, gCreationVersion.iMinor, gCreationVersion.iRelease); break;
      default : InvalidParameterException::Generate("Unknown parameter enumeration %d.","MarkAsMissingDefaulted()", eParameterType);
    };

//    if (sDefaultValue.GetLength()) {
//      gvParametersMissingDefaulted.push_back(static_cast<int>(eParameterType)); //and default retained.
//      PrintDirection.SatScanPrintWarning("Warning: The parameter '%s' is missing from the parameter file,\n",
//                                         GetParameterLineLabel(eParameterType, sParameterLineLabel, geReadType == INI));
//      PrintDirection.SatScanPrintWarning("         defaulted value '%s' assigned.\n", sDefaultValue.GetCString());
//    }
  }
  catch (ZdException & x) {
    x.AddCallpath("MarkAsMissingDefaulted()","CParameters");
    throw;
  }
}

/** Reads parameters from the file, supporting both new and old style parameter files. */
void CParameters::Read(const char* szFilename, BasePrint & PrintDirection) {
  try {
    if (access(szFilename, 04) == -1)
      GenerateResolvableException("Unable to open parameter file '%s'.\n", "Read()", szFilename);

    ZdIniFile file(szFilename, true, false);
    if (file.GetNumSections())
      ReadIniParameterFile(szFilename, PrintDirection);
    else
      ReadScanningLineParameterFile(szFilename, PrintDirection);
  }
  catch (ZdException &x) {
    GenerateResolvableException("Unable to read parameters from file '%s'.\n", "Read()", szFilename);
  }
}

// sets the global advanced features variables read in from the ini file
// pre: file is an open ini parameter file
// post: will set the global variables from the ini file
void CParameters::ReadAdvancedFeatures(ZdIniFile& file, BasePrint & PrintDirection) {
  const ZdIniSection  * pSection;
  long                  lIndex;

  try {
    //Get advanced section, add if non-existant.
    pSection = file.GetSection(ADVANCED_FEATURES_SECTION);
    
    ReadIniParameter(*pSection, VALID_PARAMS_LINE, VALIDATE, PrintDirection);
    ReadIniParameter(*pSection, ISOTONIC_SCAN_LINE, RISKFUNCTION, PrintDirection);
    ReadIniParameter(*pSection, PVALUE_PROSPECT_LLR_LINE, POWERCALC, PrintDirection);
    ReadIniParameter(*pSection, LLR_1_LINE, POWERX, PrintDirection);
    ReadIniParameter(*pSection, LLR_2_LINE, POWERY, PrintDirection);
    ReadIniParameter(*pSection, EARLY_SIM_TERMINATION_LINE, EARLY_SIM_TERMINATION, PrintDirection);
    ReadIniParameter(*pSection, SIMULATION_TYPE_LINE, SIMULATION_TYPE, PrintDirection);
    ReadIniParameter(*pSection, SIMULATION_FILESOURCE_LINE, SIMULATION_SOURCEFILE, PrintDirection);
    ReadIniParameter(*pSection, USE_ADJUSTMENTS_BY_RR_FILE_LINE, USE_ADJ_BY_RR_FILE, PrintDirection);
    ReadIniParameter(*pSection, ADJUSTMENTS_BY_RR_FILE_LINE, ADJ_BY_RR_FILE, PrintDirection);
    ReadIniParameter(*pSection, OUTPUT_SIMULATION_DATA_LINE, OUTPUT_SIMULATION_DATA, PrintDirection);
    ReadIniParameter(*pSection, SIMULATION_DATA_OUTFILE_LINE, SIMULATION_DATA_OUTFILE, PrintDirection);
    ReadIniParameter(*pSection, MAX_CIRCLE_POP_FILE_LINE, MAXCIRCLEPOPFILE, PrintDirection);
    ReadIniParameter(*pSection, SPATIAL_ADJ_TYPE_LINE, SPATIAL_ADJ_TYPE, PrintDirection);
    ReadIniParameter(*pSection, MULTI_STREAM_PURPOSE_TYPE_LINE, MULTI_STREAM_PURPOSE_TYPE, PrintDirection);
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadAdvancedFeatures()", "CParameters");
    throw;
  }
}

// sets the global analysis section variables read in from the ini file
// pre: file is an open ini parameter file
// post: will set the global variables from the ini file
void CParameters::ReadAnalysisSection(ZdIniFile& file, BasePrint & PrintDirection) {
  const ZdIniSection  * pSection;
  long                  lIndex;

  try {
    //Get analysis section, add if non-existant.
    pSection = file.GetSection(ANALYSIS_SECTION);

    ReadIniParameter(*pSection, ANALYSIS_TYPE_LINE, ANALYSISTYPE, PrintDirection);
    ReadIniParameter(*pSection, SCAN_AREAS_LINE, SCANAREAS, PrintDirection);
    ReadIniParameter(*pSection, MODEL_TYPE_LINE, MODEL, PrintDirection);
    ReadIniParameter(*pSection, START_DATE_LINE, STARTDATE, PrintDirection);
    ReadIniParameter(*pSection, END_DATE_LINE, ENDDATE, PrintDirection);
    ReadIniParameter(*pSection, MONTE_CARLO_REPS_LINE, REPLICAS, PrintDirection);
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadAnalysisSection()", "CParameters");
    throw;
  }
}

/** Attempts to interpret passed string as a boolean value. Throws exception. */
bool CParameters::ReadBoolean(const ZdString & sValue, ParameterType eParameterType) {
  bool          bReadResult;
  ZdString      sLabel;

  try {
    if (sValue.GetIsEmpty()) {
      gbReadStatusError = true;
      InvalidParameterException::Generate("Error: Parameter '%s' is not set.\n", "ReadBoolean()",
                                          GetParameterLineLabel(eParameterType, sLabel, geReadType == INI));
    }
    else if (!(!stricmp(sValue.GetCString(),"y")   || !stricmp(sValue.GetCString(),"n") ||
               !strcmp(sValue.GetCString(),"1")    || !strcmp(sValue.GetCString(),"0")   ||
               !stricmp(sValue.GetCString(),"yes")  || !stricmp(sValue.GetCString(),"no"))) {
      gbReadStatusError = true;
      InvalidParameterException::Generate("Error: For parameter '%s', setting '%s' is invalid. Valid values are 'y' or 'n'.\n",
                                          "ReadBoolean()",
                                          GetParameterLineLabel(eParameterType, sLabel, geReadType == INI), sValue.GetCString());
    }
    else
      bReadResult = (!stricmp(sValue.GetCString(),"y") || !stricmp(sValue.GetCString(),"yes") || !strcmp(sValue.GetCString(),"1"));
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadBoolean()","CParameters");
    throw;
  }
  return bReadResult;
}

/** Set date parameter with passed string using appropriate set function. */
void CParameters::ReadDate(const ZdString & sValue, ParameterType eParameterType) {
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
                                    SetProspectiveStartDate("");
                                  else
                                    SetProspectiveStartDate(sValue);
                                  break;
     case STARTDATE             : SetStudyPeriodStartDate(sValue); break;
     case ENDDATE               : SetStudyPeriodEndDate(sValue); break;
     default : ZdException::Generate("Parameter enumeration '%d' is not listed for date read.\n","ReadDate()", eParameterType);
   };
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadDate()","CParameters");
    throw;
  }
}

/** Attempts to interpret passed string as a double value. Throws exception. */
double CParameters::ReadDouble(const ZdString & sValue, ParameterType eParameterType) {
  double        dReadResult;
  ZdString      sLabel;

  try {
    if (sValue.GetIsEmpty()) {
      gbReadStatusError = true;
      InvalidParameterException::Generate("Error: Parameter '%s' is not set.\n", "ReadDouble()",
                                          GetParameterLineLabel(eParameterType, sLabel, geReadType == INI));
    }
    else if (sscanf(sValue.GetCString(), "%lf", &dReadResult) != 1) {
      gbReadStatusError = true;
      InvalidParameterException::Generate("Error: For parameter '%s', setting '%s' is not a valid real number.\n", "ReadDouble()",
                                          GetParameterLineLabel(eParameterType, sLabel, geReadType == INI),
                                          sValue.GetCString());
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadDouble()","CParameters");
    throw;
  }
  return dReadResult;
}

// sets the global ellipse variables read in from the ini file
// pre: file is an open ini parameter file
// post: will set the global variables from the ini file
void CParameters::ReadEllipseSection(ZdIniFile& file, BasePrint & PrintDirection) {
  const ZdIniSection  * pSection;
  long                  lIndex;
  ZdString              sValue;
  ZdStringTokenizer     Tokenizer("", ",");

  try {
    //Get ellipse section, add if non-existant.
    pSection = file.GetSection(ELLIPSES_SECTION);

    ReadIniParameter(*pSection, NUMBER_ELLIPSES_LINE, ELLIPSES, PrintDirection);

    if ((lIndex = pSection->FindKey(ELLIPSE_SHAPES_LINE)) > -1) {
      //Preprocess shapes string so that commas are spaces, like in scanning version.
      Tokenizer.SetString(pSection->GetLine(lIndex)->GetValue());
      while (Tokenizer.HasMoreTokens())
           sValue << Tokenizer.GetNextToken() << " ";
      sValue.Deblank();
      ReadParameter(ESHAPES, sValue, PrintDirection);
    }
    else
      MarkAsMissingDefaulted(ESHAPES, PrintDirection);

    if ((lIndex = pSection->FindKey(ELLIPSE_ANGLES_LINE)) > -1) {
      //Preprocess angles string so that commas are spaces, like in scanning version.
      Tokenizer.SetString(pSection->GetLine(lIndex)->GetValue());
      sValue << ZdString::reset;
      while (Tokenizer.HasMoreTokens())
           sValue << Tokenizer.GetNextToken() << " ";
      sValue.Deblank();
      ReadParameter(ENUMBERS, sValue, PrintDirection);
    }
    else
      MarkAsMissingDefaulted(ENUMBERS, PrintDirection);
                        
    ReadIniParameter(*pSection, ELLIPSE_NON_COMPACTNESS_LINE, NON_COMPACTNESS_PENALTY, PrintDirection, "DuczmalCompactnessCorrection");
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadEllipseSection()","CParameters");
    throw;
  }
}

/** Attempts to interpret passed string as a space delimited string of integers that represent
    the number of rotations ellipse will make. No attempt to convert is made if there are no
    ellipses defined.  Throws exception. */
void CParameters::ReadEllipseRotations(const ZdString & sParameter) {
  int                   i, iNumTokens, iReadNumberRotations;
  ZdString              sLabel;
  ZdStringTokenizer     Tokenizer("", " ");

  try {
    if (sParameter.GetLength() && giNumberEllipses) {
      Tokenizer.SetString(sParameter);
      iNumTokens = Tokenizer.GetNumTokens();
      //if (iNumTokens != giNumberEllipses)
      //  InvalidParameterException::Generate("Error: For parameter '%s', %d ellipses requested but there are %d rotation numbers defined.\n",
      //                                      "ReadEllipseRotations()", GetParameterLineLabel(ENUMBERS, sLabel, geReadType == INI).GetCString(),
      //                                      giNumberEllipses, iNumTokens);
      for (i=0; i < iNumTokens; i++) {
         if (sscanf(Tokenizer.GetToken(i).GetCString(), "%i", &iReadNumberRotations))
           SetNumberEllipsoidRotations(iReadNumberRotations);
         else
           InvalidParameterException::Generate("Error: For parameter '%s', setting '%s' is not an integer.\n", "ReadEllipseRotations()",
                                               GetParameterLineLabel(ENUMBERS, sLabel, geReadType == INI),
                                               Tokenizer.GetToken(i).GetCString());
      }
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadEllipseRotations()","CParameters");
    throw;
  }
}

/** Attempts to interpret passed string as a space delimited string of integers that represent
    the shape of each ellipsoid. No attempt to convert is made if there are no
    ellipses defined.  Throws exception. */
void CParameters::ReadEllipseShapes(const ZdString & sParameter) {
  int                   i, iNumTokens;
  double                dReadShape;
  ZdString              sLabel;  
  ZdStringTokenizer     Tokenizer("", " ");

  try {
    if (sParameter.GetLength() && giNumberEllipses) {
      Tokenizer.SetString(sParameter);
      iNumTokens = Tokenizer.GetNumTokens();
      //if (iNumTokens != giNumberEllipses)
      //  InvalidParameterException::Generate("Error: For parameter '%s', %d ellipses requested but there are %d shapes defined.\n",
      //                                      "ReadEllipseShapes()",
      //                                      GetParameterLineLabel(ESHAPES, sLabel, geReadType == INI).GetCString(),
      //                                      giNumberEllipses, iNumTokens);
      for (i=0; i < iNumTokens; i++) {
         if (sscanf(Tokenizer.GetToken(i).GetCString(), "%lf", &dReadShape))
           SetEllipsoidShape(dReadShape);
         else
           InvalidParameterException::Generate("Error: For parameter '%s', setting '%s' is not an decimal number.\n",
                                               "ReadEllipseShapes()",
                                               GetParameterLineLabel(ESHAPES, sLabel, geReadType == INI),
                                               Tokenizer.GetToken(i).GetCString());
      }
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadEllipseShapes()","CParameters");
    throw;
  }
}

/**  Reads date range for the end range of cluster time interval. */
void CParameters::ReadEndIntervalRange(const ZdString & sParameter) {
  int                   i, iNumTokens;
  ZdString              sLabel;
  ZdStringTokenizer     Tokenizer("", ",");

  try {
    if (sParameter.GetLength()) {
      Tokenizer.SetString(sParameter);
      iNumTokens = Tokenizer.GetNumTokens();
      if (iNumTokens != 2)
        InvalidParameterException::Generate("Error: For parameter '%s', start interval range contains '%d' values but should have 2.\n",
                                            "ReadEndIntervalRange()",
                                            GetParameterLineLabel(INTERVAL_STARTRANGE, sLabel, geReadType == INI),
                                           iNumTokens);
      SetEndRangeStartDate(Tokenizer.GetNextToken().GetCString());
      SetEndRangeEndDate(Tokenizer.GetNextToken().GetCString());
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadEndIntervalRange()","CParameters");
    throw;
  }
}

/** Attempts to interpret passed string as a float value. Throws exception. */
float CParameters::ReadFloat(const ZdString & sValue, ParameterType eParameterType) {
  float         fReadResult;
  ZdString      sLabel;

  try {
    if (sValue.GetIsEmpty()) {
      gbReadStatusError = true;
      InvalidParameterException::Generate("Error: Parameter '%s' is not set.\n",
                                          "ReadFloat()",
                                          GetParameterLineLabel(eParameterType, sLabel, geReadType == INI));
    }
    else if (sscanf(sValue.GetCString(), "%f", &fReadResult) != 1) {
      gbReadStatusError = true;
      InvalidParameterException::Generate("Error: For parameter '%s', setting '%s' is not a valid real number.\n",
                                          "ReadFloat()",
                                          GetParameterLineLabel(eParameterType, sLabel, geReadType == INI),
                                          sValue.GetCString());
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadFloat()","CParameters");
    throw;
  }
  return fReadResult;
}

/** Read parameter from ini section. */
void CParameters::ReadIniParameter(const ZdIniSection & IniSection, const char * sSectionName,
                                   ParameterType eParameterType, BasePrint & PrintDirection,
                                   const char * sDeprecatedSectionName) {
  long  lIndex;
  
  try {
    if ((lIndex = IniSection.FindKey(sSectionName)) > -1)
      ReadParameter(eParameterType, ZdString(IniSection.GetLine(lIndex)->GetValue()), PrintDirection);
    else if (sDeprecatedSectionName && (lIndex = IniSection.FindKey(sDeprecatedSectionName)) > -1)
      ReadParameter(eParameterType, ZdString(IniSection.GetLine(lIndex)->GetValue()), PrintDirection);
    else
      MarkAsMissingDefaulted(eParameterType, PrintDirection);
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadIniParameter()","CParameters");
    throw;
  }
}

// reads the parameters from the ini file
// pre: sFileName exists
// post: reads the parameters from the .ini file
void CParameters::ReadIniParameterFile(ZdString sFileName, BasePrint & PrintDirection) {
   try {
    gvParametersMissingDefaulted.clear();
    gbReadStatusError = false;

    ZdIniFile     IniFile(sFileName.GetCString(), true, false, ZDIO_OPEN_READ|ZDIO_SREAD);
    SetDefaults();
    geReadType = INI;
    SetSourceFileName(sFileName.GetCString());
    ReadInputFilesSection(IniFile, PrintDirection);
    ReadAnalysisSection(IniFile, PrintDirection);
    ReadTimeParametersSection(IniFile, PrintDirection);
    ReadScanningWindowSection(IniFile, PrintDirection);
    ReadOutputFileSection(IniFile, PrintDirection);
    ReadEllipseSection(IniFile, PrintDirection);
    ReadSequentialScanSection(IniFile, PrintDirection);
    ReadAdvancedFeatures(IniFile, PrintDirection);
    ReadSystemSection(IniFile, PrintDirection);
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadIniParameterFile()","CParameters");
    throw;
  }
}

/** Attempts to interpret passed string as an integer value. Throws exception. */
int CParameters::ReadInt(const ZdString & sValue, ParameterType eParameterType) {
  int           iReadResult;
  ZdString      sLabel;

  try {
   if (sValue.GetIsEmpty()) {
     gbReadStatusError = true;
     InvalidParameterException::Generate("Error: Parameter '%s' is not set.\n",
                                         "ReadInt()",
                                         GetParameterLineLabel(eParameterType, sLabel, geReadType == INI));
   }
   else if (sscanf(sValue.GetCString(), "%i", &iReadResult) != 1) {
     gbReadStatusError = true;
     InvalidParameterException::Generate("Error: For parameter '%s', setting '%s' is not a valid integer.\n",
                                         "ReadInt()",
                                         GetParameterLineLabel(eParameterType, sLabel, geReadType == INI),
                                         sValue.GetCString());
   }
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadInt()","CParameters");
    throw;
  }
  return iReadResult;
}

// sets the global input file variables read in from the ini file
// pre: file is an open ini parameter file
// post: will set the global variables from the ini file
void CParameters::ReadInputFilesSection(ZdIniFile& file, BasePrint & PrintDirection){
  const ZdIniSection  * pSection;
  long                  lIndex;
  unsigned int          iStream, iMostStreams=1;
  ZdString              sSectionName;

  try {
    //Get input section, add if non-existant.
    pSection  = file.GetSection(INPUT_FILES_SECTION);

    ReadIniParameter(*pSection, CASE_FILE_LINE, CASEFILE, PrintDirection);
    //read possibly other data stream case source
    iStream = 2;
    sSectionName.printf("%s%i", CASE_FILE_LINE, iStream);
    while ((lIndex = pSection->FindKey(sSectionName)) > -1) {
         SetCaseFileName(ZdString(pSection->GetLine(lIndex)->GetValue()), true, iStream);
         sSectionName.printf("%s%i", CASE_FILE_LINE, ++iStream);
    }
    iMostStreams = std::max(iMostStreams, gvCaseFilenames.size());
    ReadIniParameter(*pSection, CONTROL_FILE_LINE, CONTROLFILE, PrintDirection);
    //read possibly other data stream control source
    iStream = 2;
    sSectionName.printf("%s%i", CONTROL_FILE_LINE, iStream);
    while ((lIndex = pSection->FindKey(sSectionName)) > -1) {
         SetControlFileName(ZdString(pSection->GetLine(lIndex)->GetValue()), true, iStream);
         sSectionName.printf("%s%i", CONTROL_FILE_LINE, ++iStream);
    }
    iMostStreams = std::max(iMostStreams, gvControlFilenames.size());
    ReadIniParameter(*pSection, POP_FILE_LINE, POPFILE, PrintDirection);
    //read possibly other data stream control source
    iStream = 2;
    sSectionName.printf("%s%i", POP_FILE_LINE, iStream);
    while ((lIndex = pSection->FindKey(sSectionName)) > -1) {
         SetPopulationFileName(ZdString(pSection->GetLine(lIndex)->GetValue()), true, iStream);
         sSectionName.printf("%s%i", POP_FILE_LINE, ++iStream);
    }
    iMostStreams = std::max(iMostStreams, gvPopulationFilenames.size());
    //Synchronize collections of data stream filesnames so that we can ask for
    //any file of a particular stream, even if blank. This keeps the same behavior
    //as when there was only one data stream.
    SetNumDataStreams(iMostStreams);
    ReadIniParameter(*pSection, COORD_FILE_LINE, COORDFILE, PrintDirection);
    ReadIniParameter(*pSection, USE_GRID_FILE_LINE, SPECIALGRID, PrintDirection);
    ReadIniParameter(*pSection, GRID_FILE_LINE, GRIDFILE, PrintDirection);
    ReadIniParameter(*pSection, PRECISION_TIMES_LINE, PRECISION, PrintDirection);
    ReadIniParameter(*pSection, COORD_TYPE_LINE, COORDTYPE, PrintDirection);
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadInputFilesSection()", "CParameters");
    throw;
  }
}

/** . */
void CParameters::ReadStartIntervalRange(const ZdString & sParameter) {
  int                   i, iNumTokens;
  ZdString              sLabel;
  ZdStringTokenizer     Tokenizer("", ",");

  try {
    if (sParameter.GetLength()) {
      Tokenizer.SetString(sParameter);
      iNumTokens = Tokenizer.GetNumTokens();
      if (iNumTokens != 2)
        InvalidParameterException::Generate("Error: For parameter '%s', end interval range contains '%d' values but should have 2.\n",
                                            "ReadStartIntervalRange()",
                                            GetParameterLineLabel(INTERVAL_ENDRANGE, sLabel, geReadType == INI),
                                            iNumTokens);
      SetStartRangeStartDate(Tokenizer.GetNextToken().GetCString());
      SetStartRangeEndDate(Tokenizer.GetNextToken().GetCString());
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadStartIntervalRange()","CParameters");
    throw;
  }
}

/** Sets start range start date. Throws exception. */
void CParameters::SetStartRangeStartDate(const char * sStartRangeStartDate) {
  ZdString      sLabel;

  try {
    if (!sStartRangeStartDate)
      ZdException::Generate("Null pointer.","SetStartRangeStartDate()");

    if (strspn(sStartRangeStartDate,"0123456789/") < strlen(sStartRangeStartDate))
      InvalidParameterException::Generate("Error: For parameter %s, setting '%s' does not appear to be a date.\n",
                                          "SetStartRangeStartDate()",
                                          GetParameterLineLabel(INTERVAL_STARTRANGE, sLabel, geReadType == INI),
                                          sStartRangeStartDate);
    gsStartRangeStartDate = sStartRangeStartDate;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetStartRangeStartDate()","CParameters");
    throw;
  }
}

/** Sets start range start date. Throws exception. */
void CParameters::SetStartRangeEndDate(const char * sStartRangeEndDate) {
  ZdString      sLabel;

  try {
    if (!sStartRangeEndDate)
      ZdException::Generate("Null pointer.","SetStartRangeEndDate()");

    if (strspn(sStartRangeEndDate,"0123456789/") < strlen(sStartRangeEndDate))
      InvalidParameterException::Generate("Error: For parameter %s, setting '%s' does not appear to be a date.\n",
                                          "SetStartRangeEndDate()",
                                          GetParameterLineLabel(INTERVAL_STARTRANGE, sLabel, geReadType == INI),
                                          sStartRangeEndDate);
    gsStartRangeEndDate = sStartRangeEndDate;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetStartRangeEndDate()","CParameters");
    throw;
  }
}

/** Calls appropriate read/or set function for parameter type to set parameter from a string. */
void CParameters::ReadParameter(ParameterType eParameterType, const ZdString & sParameter, BasePrint & PrintDirection) {
  try {
    switch (eParameterType) {
      case ANALYSISTYPE              : SetAnalysisType((AnalysisType)ReadInt(sParameter, eParameterType)); break;
      case SCANAREAS                 : SetAreaRateType((AreaRateType)ReadInt(sParameter, eParameterType)); break;
      case CASEFILE                  : SetCaseFileName(sParameter.GetCString(), true); break;
      case POPFILE                   : SetPopulationFileName(sParameter.GetCString(), true); break;
      case COORDFILE                 : SetCoordinatesFileName(sParameter.GetCString(), true); break;
      case OUTPUTFILE                : SetOutputFileName(sParameter.GetCString(), true); break;
      case PRECISION                 : SetPrecisionOfTimesType((DatePrecisionType)ReadInt(sParameter, eParameterType)); break;
      case DIMENSION                 : //Dimensions no longer read from file.
                                       break;
      case SPECIALGRID               : SetUseSpecialGrid(ReadBoolean(sParameter, eParameterType)); break;
      case GRIDFILE                  : SetSpecialGridFileName(sParameter.GetCString(), true); break;
      case GEOSIZE                   : SetMaximumGeographicClusterSize(ReadFloat(sParameter, eParameterType)); break;
      case STARTDATE                 : ReadDate(sParameter, eParameterType); break;
      case ENDDATE                   : ReadDate(sParameter, eParameterType); break;
      case CLUSTERS                  : SetIncludeClustersType((IncludeClustersType)ReadInt(sParameter, eParameterType)); break;
      case EXACTTIMES                : //No longer used. No documentation as to previous usage.
                                       break;
      case TIME_AGGREGATION_UNITS    : SetTimeAggregationUnitsType((DatePrecisionType)ReadInt(sParameter, eParameterType)); break;
      case TIME_AGGREGATION          : SetTimeAggregationLength((long)ReadInt(sParameter, eParameterType)); break;
      case PURESPATIAL               : SetIncludePurelySpatialClusters(ReadBoolean(sParameter, eParameterType)); break;
      case TIMESIZE                  : SetMaximumTemporalClusterSize(ReadFloat(sParameter, eParameterType)); break;
      case REPLICAS                  : SetNumberMonteCarloReplications(ReadUnsignedInt(sParameter, eParameterType)); break;
      case MODEL                     : SetProbabilityModelType((ProbabiltyModelType)ReadInt(sParameter, eParameterType)); break;
      case RISKFUNCTION              : SetRiskType((RiskType)ReadInt(sParameter, eParameterType)); break;
      case POWERCALC                 : SetPowerCalculation(ReadBoolean(sParameter, eParameterType)); break;
      case POWERX                    : SetPowerCalculationX(ReadDouble(sParameter, eParameterType)); break;
      case POWERY                    : SetPowerCalculationY(ReadDouble(sParameter, eParameterType)); break;
      case TIMETREND                 : SetTimeTrendAdjustmentType((TimeTrendAdjustmentType)ReadInt(sParameter, eParameterType)); break;
      case TIMETRENDPERC             : SetTimeTrendAdjustmentPercentage(ReadDouble(sParameter, eParameterType)); break;
      case PURETEMPORAL              : SetIncludePurelyTemporalClusters(ReadBoolean(sParameter, eParameterType)); break;
      case CONTROLFILE               : SetControlFileName(sParameter.GetCString(), true); break;
      case COORDTYPE                 : SetCoordinatesType((CoordinatesType)ReadInt(sParameter, eParameterType)); break;
      case OUTPUT_SIM_LLR_ASCII      : SetOutputSimLogLikeliRatiosAscii(ReadBoolean(sParameter, eParameterType)); break;
      case SEQUENTIAL                : SetSequentialScanning(ReadBoolean(sParameter, eParameterType)); break;
      case SEQNUM                    : SetNumSequentialScans(ReadUnsignedInt(sParameter, eParameterType)); break;
      case SEQPVAL                   : SetSequentialCutOffPValue(ReadDouble(sParameter, eParameterType)); break;
      case VALIDATE                  : SetValidatePriorToCalculation(ReadBoolean(sParameter, eParameterType)); break;
      case OUTPUT_RR_ASCII           : SetOutputRelativeRisksAscii(ReadBoolean(sParameter, eParameterType)); break;
      case ELLIPSES                  : SetNumberEllipses(ReadInt(sParameter, eParameterType)); break;
      case ESHAPES                   : ReadEllipseShapes(sParameter); break;
      case ENUMBERS                  : ReadEllipseRotations(sParameter); break;
      case START_PROSP_SURV          : ReadDate(sParameter, eParameterType); break;
      case OUTPUT_AREAS_ASCII        : SetOutputAreaSpecificAscii(ReadBoolean(sParameter, eParameterType)); break;
      case OUTPUT_MLC_ASCII          : SetOutputClusterLevelAscii(ReadBoolean(sParameter, eParameterType)); break;
      case CRITERIA_SECOND_CLUSTERS  : SetCriteriaForReportingSecondaryClusters((CriteriaSecondaryClustersType)ReadInt(sParameter, eParameterType)); break;
      case MAX_TEMPORAL_TYPE         : SetMaximumTemporalClusterSizeType((TemporalSizeType)ReadInt(sParameter, eParameterType)); break;
      case MAX_SPATIAL_TYPE          : SetMaximumSpacialClusterSizeType((SpatialSizeType)ReadInt(sParameter, eParameterType)); break;
      case RUN_HISTORY_FILENAME      : //Run History no longer scanned from parameters file. Set through
                                       //setters/getters and copy() only.
                                       break;
      case OUTPUT_MLC_DBASE          : SetOutputClusterLevelDBase(ReadBoolean(sParameter, eParameterType)); break;
      case OUTPUT_AREAS_DBASE        : SetOutputAreaSpecificDBase(ReadBoolean(sParameter, eParameterType)); break;
      case OUTPUT_RR_DBASE           : SetOutputRelativeRisksDBase(ReadBoolean(sParameter, eParameterType)); break;
      case OUTPUT_SIM_LLR_DBASE      : SetOutputSimLogLikeliRatiosDBase(ReadBoolean(sParameter, eParameterType)); break;
      case NON_COMPACTNESS_PENALTY   : SetNonCompactnessPenalty(ReadBoolean(sParameter, eParameterType)); break;
      case INTERVAL_STARTRANGE       : ReadStartIntervalRange(sParameter); break;
      case INTERVAL_ENDRANGE         : ReadEndIntervalRange(sParameter); break;
      case TIMETRENDCONVRG           : SetTimeTrendConvergence(ReadDouble(sParameter, eParameterType)); break;
      case MAXCIRCLEPOPFILE          : SetMaxCirclePopulationFileName(sParameter.GetCString(), true); break;
      case EARLY_SIM_TERMINATION     : SetTerminateSimulationsEarly(ReadBoolean(sParameter, eParameterType)); break;
      case REPORTED_GEOSIZE          : SetMaximumReportedGeographicalClusterSize(ReadFloat(sParameter, eParameterType)); break;
      case USE_REPORTED_GEOSIZE      : SetRestrictReportedClusters(ReadBoolean(sParameter, eParameterType)); break;
      case SIMULATION_TYPE           : SetSimulationType((SimulationType)ReadInt(sParameter, eParameterType)); break;
      case SIMULATION_SOURCEFILE     : SetSimulationDataSourceFileName(sParameter.GetCString(), true); break;
      case ADJ_BY_RR_FILE            : SetAdjustmentsByRelativeRisksFilename(sParameter.GetCString(), true); break;
      case OUTPUT_SIMULATION_DATA    : SetOutputSimulationData(ReadBoolean(sParameter, eParameterType)); break;
      case SIMULATION_DATA_OUTFILE   : SetSimulationDataOutputFileName(sParameter.GetCString(), true); break;
      case ADJ_FOR_EALIER_ANALYSES   : SetAdjustForEarlierAnalyses(ReadBoolean(sParameter, eParameterType)); break;
      case USE_ADJ_BY_RR_FILE        : SetUseAdjustmentForRelativeRisksFile(ReadBoolean(sParameter, eParameterType)); break;
      case SPATIAL_ADJ_TYPE          : SetSpatialAdjustmentType((SpatialAdjustmentType)ReadInt(sParameter, eParameterType)); break;
      case MULTI_STREAM_PURPOSE_TYPE : SetMultipleDataStreamPurposeType((MultipleStreamPurposeType)ReadInt(sParameter, eParameterType)); break;
      case CREATION_VERSION          : SetVersion(sParameter); break;
      default : InvalidParameterException::Generate("Unknown parameter enumeration %d.","ReadParameter()", eParameterType);
    };
  }
  catch (InvalidParameterException &x) {
    gbReadStatusError = true;
    PrintDirection.SatScanPrintWarning(x.GetErrorMessage());
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadParameter()","CParameters");
    throw;
  }
}

// sets the global output file variables read in from the ini file
// pre: file is an open ini parameter file
// post: will set the global variables from the ini file
void CParameters::ReadOutputFileSection(ZdIniFile& file, BasePrint & PrintDirection) {
  const ZdIniSection  * pSection;
  long                  lIndex;

  try {
    //Get output section, add if non-existant.
    pSection = file.GetSection(OUTPUT_FILES_SECTION);

    ReadIniParameter(*pSection, RESULTS_FILE_LINE, OUTPUTFILE, PrintDirection);
    ReadIniParameter(*pSection, OUTPUT_SIM_LLR_ASCII_LINE, OUTPUT_SIM_LLR_ASCII, PrintDirection);
    ReadIniParameter(*pSection, OUTPUT_AREAS_ASCII_LINE, OUTPUT_AREAS_ASCII, PrintDirection);
    ReadIniParameter(*pSection, OUTPUT_MLC_ASCII_LINE, OUTPUT_MLC_ASCII, PrintDirection);
    ReadIniParameter(*pSection, OUTPUT_MLC_DBASE_LINE, OUTPUT_MLC_DBASE, PrintDirection);
    ReadIniParameter(*pSection, OUTPUT_AREAS_DBASE_LINE, OUTPUT_AREAS_DBASE, PrintDirection);
    ReadIniParameter(*pSection, OUTPUT_REL_RISKS_ASCII_LINE, OUTPUT_RR_ASCII, PrintDirection);
    ReadIniParameter(*pSection, OUTPUT_REL_RISKS_DBASE_LINE, OUTPUT_RR_DBASE, PrintDirection);
    ReadIniParameter(*pSection, OUTPUT_SIM_LLR_DBASE_LINE, OUTPUT_SIM_LLR_DBASE, PrintDirection);
    ReadIniParameter(*pSection, CRIT_REPORT_SEC_CLUSTERS_LINE, CRITERIA_SECOND_CLUSTERS, PrintDirection);
    ReadIniParameter(*pSection, REPORTED_GEOSIZE_LINE, REPORTED_GEOSIZE, PrintDirection);
    ReadIniParameter(*pSection, USE_REPORTED_GEOSIZE_LINE, USE_REPORTED_GEOSIZE, PrintDirection);
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadOutputFileSection()", "CParameters");
    throw;
  }
}

/** Read scanning line version of parameter file. */
void CParameters::ReadScanningLineParameterFile(const char * sParameterFileName, BasePrint & PrintDirection) {
  bool          bEOF=false;
  int           iPos, iLinesRead=0;
  ZdIO          ParametersFile;
  ZdString      sLineBuffer;

  try {
    gvParametersMissingDefaulted.clear();
    gbReadStatusError = false;
  
    ParametersFile.Open(sParameterFileName, ZDIO_OPEN_READ);
    SetSourceFileName(sParameterFileName);
    SetDefaults();
    geReadType = SCAN;

    while (iLinesRead <= giNumParameters && !bEOF) {
         bEOF = !ParametersFile.ReadLine(sLineBuffer);
         if (! bEOF) {
           ++iLinesRead;
           //Pre-process parameters that have descriptions, strip decription off.
           if (!((ParameterType)iLinesRead == CASEFILE || (ParameterType)iLinesRead == POPFILE ||
                 (ParameterType)iLinesRead == COORDFILE || (ParameterType)iLinesRead == OUTPUTFILE ||
                 (ParameterType)iLinesRead == GRIDFILE || (ParameterType)iLinesRead == CONTROLFILE ||
                 (ParameterType)iLinesRead == MAXCIRCLEPOPFILE)) {
              if ((iPos = sLineBuffer.Find("//")) > -1)
                sLineBuffer.Truncate(iPos);
           }
           sLineBuffer.Deblank();
           ReadParameter((ParameterType)iLinesRead, sLineBuffer, PrintDirection);
         }
    }

    if (iLinesRead <= START_PROSP_SURV) {
      //Version 2.1.3 had 40 parameters where the 40th item in enumeration
      //as called EXTRA4, now called START_PROSP_SURV.
      //In the switch to version 3.x somewhere the enumerations for analysis type
      //was modified from:
      //enum {PURELYSPATIAL=1, SPACETIME, PURELYTEMPORAL, PURELYSPATIALMONOTONE};
      //to
      //enum {PURELYSPATIAL=1, PURELYTEMPORAL, SPACETIME,  PROSPECTIVESPACETIME, PURELYSPATIALMONOTONE};
      //probably to accommodate the user interface? So we need to correct this
      //change in v2.1.3 and prior parameter files. Note that PURELYSPATIALMONOTONE
      //also become 5 instead of 4. It appears that this analysis type was never
      //made available so that it is not a concern.
      if (geAnalysisType == SPACETIME)
        geAnalysisType = PURELYTEMPORAL;
      else if (geAnalysisType == PURELYTEMPORAL)
        geAnalysisType = SPACETIME;

      if (giNumberEllipses > 0) {
        //version 2.1.3
        //... VALIDATE, OUTPUTRR, EXTRA1, EXTRA2, EXTRA3, EXTRA4 };
        //There is no documentation as to what these extra parameter
        //items were meant for. But they were displayed in parameters file.
        //As seen in parameter file:
        //0                     // Extra Parameter #1
        //0                     // Extra Parameter #2
        //0                     // Extra Parameter #3
        //0                     // Extra Parameter #4
        //pre-version 3.0
        //... VALIDATE, OUTPUTRR, ELLIPSES, ESHAPES, ENUMBERS, START_PROSP_SURV,
        //    OUTPUT_CENSUS_AREAS, OUTPUT_MOST_LIKE_CLUSTERS, CRITERIA_SECOND_CLUSTERS};
        //Only Dr. Kulldorph and small others should have gotten this version
        //as it was not offically released. If number of ellipse is not zero then someone
        //has set to run with ellipses...
        //The ability to restrict secondary clusters wasn't present at that time.
        //So SaTScan wasn't restricting anything. Now that the default restriction
        //is no overlapping, a person running SaTScan in batch mode wouldn't know
        //what what line to modify as it doesn't exist yet in file.
        //We'll keep up the hidden attribute for this situation.
        geCriteriaSecondClustersType = NORESTRICTIONS; // no restrictions like pre v3.0
      }
    }

    //Mark defaulted values.
   if (iLinesRead != giNumParameters)
      while (++iLinesRead <= giNumParameters)
          MarkAsMissingDefaulted((ParameterType)iLinesRead, PrintDirection);
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadScanningLineParameterFile()", "CParameters");
    throw;
  }
}

// sets the global scanning window variables read in from the ini file
// pre: file is an open ini parameter file
// post: will set the global variables from the ini file
void CParameters::ReadScanningWindowSection(ZdIniFile& file, BasePrint & PrintDirection) {
  const ZdIniSection  * pSection;

  try {
    //Get scanning window section, add if non-existant.
    pSection = file.GetSection(SCANNING_WINDOW_SECTION);
    
    ReadIniParameter(*pSection, INCLUDE_PURELY_SPATIAL_LINE, PURESPATIAL, PrintDirection);
    ReadIniParameter(*pSection, MAX_TEMP_SIZE_LINE, TIMESIZE, PrintDirection);
    ReadIniParameter(*pSection, MAX_GEO_SIZE_LINE, GEOSIZE, PrintDirection);
    ReadIniParameter(*pSection, INLCUDE_CLUSTERS_LINE, CLUSTERS, PrintDirection);
    ReadIniParameter(*pSection, INCLUDE_PURE_TEMP_LINE, PURETEMPORAL, PrintDirection);
    ReadIniParameter(*pSection, MAX_TEMP_INTERPRET_LINE, MAX_TEMPORAL_TYPE, PrintDirection);
    ReadIniParameter(*pSection, MAX_GEO_INTERPRET_LINE, MAX_SPATIAL_TYPE, PrintDirection);
    ReadIniParameter(*pSection, STARTRANGE_LINE, INTERVAL_STARTRANGE, PrintDirection);
    ReadIniParameter(*pSection, ENDRANGE_LINE, INTERVAL_ENDRANGE, PrintDirection);
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadScanningWindowSection()", "CParameters");
    throw;
  }
}

// sets the global sequential scan variables read in from the ini file
// pre: file is an open ini parameter file
// post: will set the global variables from the ini file
void CParameters::ReadSequentialScanSection(ZdIniFile& file, BasePrint & PrintDirection) {
  const ZdIniSection  * pSection;

  try {
    //Get sequential scan section, add if non-existant.
    pSection = file.GetSection(SEQUENTIAL_SCAN_SECTION);

    ReadIniParameter(*pSection, SEQUENTIAL_SCAN_LINE, SEQUENTIAL, PrintDirection);
    ReadIniParameter(*pSection, SEQUENTIAL_MAX_ITERS_LINE, SEQNUM, PrintDirection);
    ReadIniParameter(*pSection, SEQUENTIAL_MAX_PVALUE_LINE, SEQPVAL, PrintDirection);
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadSequentialScanSection()", "CParameters");
    throw;
  }
}

/** Reads 'system' ini section */
void CParameters::ReadSystemSection(ZdIniFile& file, BasePrint & PrintDirection) {
  const ZdIniSection  * pSection;

  try {
    //Get sequential scan section, add if non-existant.
    pSection = file.GetSection(SYSTEM_SECTION);

    ReadIniParameter(*pSection, CREATION_VERSION_LINE, CREATION_VERSION, PrintDirection);
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadSystemSection()", "CParameters");
    throw;
  }
}

// sets the global TimeParameters variables read in from the ini file
// pre: file is an open ini parameter file
// post: will set the global variables from the ini file
void CParameters::ReadTimeParametersSection(ZdIniFile& file, BasePrint & PrintDirection) {
  const ZdIniSection  * pSection;

  try {
    //Get time parameters section, add if non-existant.
    pSection = file.GetSection(TIME_PARAMS_SECTION);

    ReadIniParameter(*pSection, TIME_AGGREGATION_UNITS_LINE, TIME_AGGREGATION_UNITS, PrintDirection, "IntervalUnits");
    ReadIniParameter(*pSection, TIME_AGGREGATION_LENGTH_LINE, TIME_AGGREGATION, PrintDirection, "IntervalLength");
    ReadIniParameter(*pSection, TIME_TREND_ADJ_LINE, TIMETREND, PrintDirection);
    ReadIniParameter(*pSection, TIME_TREND_PERCENT_LINE, TIMETRENDPERC, PrintDirection);
    ReadIniParameter(*pSection, PROSPECT_START_LINE, START_PROSP_SURV, PrintDirection);
    ReadIniParameter(*pSection, TIME_TREND_CONVERGENCE_LINE, TIMETRENDCONVRG, PrintDirection);
    ReadIniParameter(*pSection, ADJUST_EALIER_ANALYSES_LINE, ADJ_FOR_EALIER_ANALYSES, PrintDirection);
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadTimeParametersSection()", "CParameters");
    throw;
  }
}

/** Attempts to interpret passed string as an integer value. Throws exception. */
int CParameters::ReadUnsignedInt(const ZdString & sValue, ParameterType eParameterType) {
  int           iReadResult;
  ZdString      sLabel;

  try {
   if (sValue.GetIsEmpty()) {
     gbReadStatusError = true;
     InvalidParameterException::Generate("Error: Parameter '%s' is not set.\n",
                                         "ReadUnsignedInt()",
                                         GetParameterLineLabel(eParameterType, sLabel, geReadType == INI));
   }
   else if (sscanf(sValue.GetCString(), "%u", &iReadResult) != 1) {
     gbReadStatusError = true;
     InvalidParameterException::Generate("Error: For parameter '%s', setting '%s' is not a valid integer.\n",
                                         "ReadUnsignedInt()",
                                         GetParameterLineLabel(eParameterType, sLabel, geReadType == INI),
                                         sValue.GetCString());
   }
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadUnsignedInt()","CParameters");
    throw;
  }
  return iReadResult;
}

// saves the model info section to the ini file
// pre: file is an open ZdIniFile
// post: write the appropraite global data to the file to the appropraite keys
void CParameters::SaveAdvancedFeaturesSection(ZdIniFile& file) {
  ZdString               sValue;
  ZdIniSection         * pSection;

  try {
    pSection = file.GetSection(ADVANCED_FEATURES_SECTION);
    pSection->AddComment(" validate parameters (y/n)");
    pSection->AddLine(VALID_PARAMS_LINE, gbValidatePriorToCalc ? YES : NO);
    pSection->AddComment(" Isotonic Scan (0=Standard, 1=Monotone)");
    pSection->AddLine(ISOTONIC_SCAN_LINE, AsString(sValue, geRiskFunctionType));
    pSection->AddComment(" p-Values for 2 Prespecified LLR's (y/n)");
    pSection->AddLine(PVALUE_PROSPECT_LLR_LINE, gbPowerCalculation ? YES : NO);
    pSection->AddLine(LLR_1_LINE, AsString(sValue, gdPower_X));
    pSection->AddLine(LLR_2_LINE, AsString(sValue, gdPower_Y));
    pSection->AddLine(EARLY_SIM_TERMINATION_LINE, gbEarlyTerminationSimulations ? YES : NO);
    pSection->AddComment(" Simulated data methods (Null Randomization=0, HA Randomization=1, File Import=2)");
    pSection->AddLine(SIMULATION_TYPE_LINE, AsString(sValue, geSimulationType));
    pSection->AddComment(" Simulated date input file name (with File Import=2)");
    pSection->AddLine(SIMULATION_FILESOURCE_LINE, gsSimulationDataSourceFileName.c_str());
    pSection->AddComment(" use adjustments by known relative risks file? (y/n)");
    pSection->AddLine(USE_ADJUSTMENTS_BY_RR_FILE_LINE, gbUseAdjustmentsForRRFile ? YES : NO);
    pSection->AddComment(" Adjustments by known relative risks file name (with HA Randomization=1 or ...)");
    pSection->AddLine(ADJUSTMENTS_BY_RR_FILE_LINE, gsAdjustmentsByRelativeRisksFileName.c_str());
    pSection->AddComment(" Print simulated data to file (y/n)");
    pSection->AddLine(OUTPUT_SIMULATION_DATA_LINE, gbOutputSimulationData ? YES : NO);
    pSection->AddComment(" Simulated data output file name");
    pSection->AddLine(SIMULATION_DATA_OUTFILE_LINE, gsSimulationDataOutputFilename.c_str());
    pSection->AddLine(MAX_CIRCLE_POP_FILE_LINE, gsMaxCirclePopulationFileName.c_str());
    pSection->AddComment(" Spatial Adjustments Type (no spatial adjustment=0, spatially stratified randomization=1)");
    pSection->AddLine(SPATIAL_ADJ_TYPE_LINE, AsString(sValue, geSpatialAdjustmentType));
    pSection->AddComment(" Multiple Data Streams Purpose Type (multivariate=0, adjustment=1)");
    pSection->AddLine(MULTI_STREAM_PURPOSE_TYPE_LINE, AsString(sValue, geMultipleStreamPurposeType));
  }
  catch (ZdException &x) {
    x.AddCallpath("SaveAdvancedFeaturesSection()","CParameters");
    throw;
  }
}

// saves the Analysis section to the ini file
// pre: file is an open ZdIniFile
// post: write the appropraite global data to the file to the appropraite keys
void CParameters::SaveAnalysisSection(ZdIniFile& file) {
  ZdIniSection        * pSection;
  ZdString              sValue;

  try {
    pSection = file.GetSection(ANALYSIS_SECTION);
    pSection->AddComment(" analysis type (1=Purely Spatial, 2=Purely Temporal, 3=Retrospective Space-Time, 4=Prospective Space-Time, 5=Spatial Variation/Temporal Trends, 6=Prospective Purely Temporal)");
    pSection->AddLine(ANALYSIS_TYPE_LINE, AsString(sValue, geAnalysisType));
    pSection->AddComment(" model type (0=Poisson, 1=Bernoulli, 2=Space-Time Permutation, 3=Normal, 4=Survival, 5=Rank)");
    pSection->AddLine(MODEL_TYPE_LINE, AsString(sValue, geProbabiltyModelType));
    pSection->AddComment(" scan areas (1=High, 2=Low, 3=High or Low)");
    pSection->AddLine(SCAN_AREAS_LINE, AsString(sValue, geAreaScanRate));
    pSection->AddComment(" study period start date (YYYY/MM/DD)");
    pSection->AddLine(START_DATE_LINE, gsStudyPeriodStartDate.c_str());
    pSection->AddComment(" study period end date (YYYY/MM/DD)");
    pSection->AddLine(END_DATE_LINE, gsStudyPeriodEndDate.c_str());
    pSection->AddComment(" Monte Carlo reps (0, 9, 999, n999)");
    pSection->AddLine(MONTE_CARLO_REPS_LINE, AsString(sValue, giReplications));
  }
  catch (ZdException &x) {
    x.AddCallpath("SaveAnalysisSection()","CParameters");
    throw;
  }
}

// saves the ellipse section to the ini file
// pre: file is an open ZdIniFile
// post: write the appropraite global data to the file to the appropraite keys
void CParameters::SaveEllipseSection(ZdIniFile& file) {
  int                   i;
  ZdString              sValue, sShapes, sAngles;
  ZdIniSection        * pSection;

  try {
    pSection = file.GetSection(ELLIPSES_SECTION);
    pSection->AddComment(" number of ellipses (0-10)");
    pSection->AddLine(NUMBER_ELLIPSES_LINE, AsString(sValue, giNumberEllipses));
    sValue << ZdString::reset;
    pSection->AddComment(" ellipse shapes");
    for (i=0; i < giNumberEllipses; ++i)
       sValue << (i == 0 ? "" : ",") << gvEllipseShapes[i];
    pSection->AddLine(ELLIPSE_SHAPES_LINE, sValue.GetCString());
    sValue << ZdString::reset;
    pSection->AddComment(" ellipse angles");
    for (i=0; i < giNumberEllipses; ++i)
        sValue << (i == 0 ? "" : ",") << gvEllipseRotations[i];
    pSection->AddLine(ELLIPSE_ANGLES_LINE, sValue.GetCString());
    pSection->AddComment(" Non-Compactness Penalty (y/n)");
    pSection->AddLine(ELLIPSE_NON_COMPACTNESS_LINE, gbNonCompactnessPenalty ? YES : NO);
  }
  catch (ZdException &x) {
    x.AddCallpath("SaveEllipseSection()", "CParameters");
    throw;
  }
}

// saves the input file section to the ini file
// pre: file is an open ZdIniFile
// post: write the appropraite global data to the file to the appropraite keys
void CParameters::SaveInputFileSection(ZdIniFile& file) {
  size_t                t;
  ZdIniSection        * pSection;
  ZdString              sValue, sSectionName;

  try {
    pSection = file.GetSection(INPUT_FILES_SECTION);
    if (gvCaseFilenames.size()) {
      pSection->AddLine(CASE_FILE_LINE, gvCaseFilenames[0].c_str());
      for (t=1; t < gvCaseFilenames.size(); ++t) {
         sSectionName.printf("%s%d", CASE_FILE_LINE, t + 1);
         pSection->AddLine(sSectionName.GetCString(), gvCaseFilenames[t].c_str());
      }
    }
    if (gvControlFilenames.size()) {
      pSection->AddLine(CONTROL_FILE_LINE, gvControlFilenames[0].c_str());
      for (t=1; t < gvControlFilenames.size(); ++t) {
         sSectionName.printf("%s%d", CONTROL_FILE_LINE, t + 1);
         pSection->AddLine(sSectionName.GetCString(), gvControlFilenames[t].c_str());
      }
    }
    if (gvPopulationFilenames.size()) {
      pSection->AddLine(POP_FILE_LINE, gvPopulationFilenames[0].c_str());
      for (t=1; t < gvPopulationFilenames.size(); ++t) {
         sSectionName.printf("%s%d", POP_FILE_LINE, t + 1);
         pSection->AddLine(sSectionName.GetCString(), gvPopulationFilenames[t].c_str());
      }
    }
    pSection->AddLine(COORD_FILE_LINE, gsCoordinatesFileName.c_str());
    pSection->AddLine(GRID_FILE_LINE, gsSpecialGridFileName.c_str());
    pSection->AddComment(" use grid file? (y/n)");
    pSection->AddLine(USE_GRID_FILE_LINE, gbUseSpecialGridFile ? YES : NO);
    pSection->AddComment(" time precision (0=None, 1=Year, 2=Month, 3=Day)");
    pSection->AddLine(PRECISION_TIMES_LINE, AsString(sValue, gePrecisionOfTimesType));
    pSection->AddComment(" coordinate type (0=Cartesian, 1=Lat/Long)");
    pSection->AddLine(COORD_TYPE_LINE, AsString(sValue, geCoordinatesType));
  }
  catch (ZdException &x) {
    x.AddCallpath("SaveInputFileSection()","CParameters");
    throw;
  }
}

// saves the output file section to the ini file
// pre: file is an open ZdIniFile
// post: write the appropraite global data to the file to the appropraite keys
void CParameters::SaveOutputFileSection(ZdIniFile& file) {
  ZdIniSection        * pSection;
  ZdString              sValue;

  try {
    pSection = file.GetSection(OUTPUT_FILES_SECTION);
    pSection->AddLine(RESULTS_FILE_LINE, gsOutputFileName.c_str());
    pSection->AddComment(" output most likely clusters in ASCII format (y/n)");
    pSection->AddLine(OUTPUT_MLC_ASCII_LINE, gbOutputClusterLevelAscii ? YES : NO);
    pSection->AddComment(" output most likely clusters in dBase format (y/n)");
    pSection->AddLine(OUTPUT_MLC_DBASE_LINE, gbOutputClusterLevelDBase ? YES : NO);
    pSection->AddComment(" report census areas in ASCII format (y/n)");
    pSection->AddLine(OUTPUT_AREAS_ASCII_LINE, gbOutputAreaSpecificAscii ? YES : NO);
    pSection->AddComment(" report census areas in dBase format (y/n)");
    pSection->AddLine(OUTPUT_AREAS_DBASE_LINE, gbOutputAreaSpecificDBase ? YES : NO);
    pSection->AddComment(" report Simulated Log Likelihoods Ratios in ASCII format (y/n)");
    pSection->AddLine(OUTPUT_SIM_LLR_ASCII_LINE, gbOutputSimLogLikeliRatiosAscii ? YES : NO);
    pSection->AddComment(" report Simulated Log Likelihoods Ratios in dBase format (y/n)");
    pSection->AddLine(OUTPUT_SIM_LLR_DBASE_LINE, gbOutputSimLogLikeliRatiosDBase ? YES : NO);
    pSection->AddComment(" report relative risks in ASCII format (y/n)");
    pSection->AddLine(OUTPUT_REL_RISKS_ASCII_LINE, gbOutputRelativeRisksAscii ? YES : NO);
    pSection->AddComment(" report relative risks in dBase format (y/n)");
    pSection->AddLine(OUTPUT_REL_RISKS_DBASE_LINE, gbOutputRelativeRisksDBase ? YES : NO);
    pSection->AddComment(" criteria for reporting secondary clusters(0=NoGeoOverlap, 1=NoCentersInOther, 2=NoCentersInMostLikely, 3=NoCentersInLessLikely, 4=NoPairsCentersEachOther, 5=NoRestrictions)");
    pSection->AddLine(CRIT_REPORT_SEC_CLUSTERS_LINE, AsString(sValue, geCriteriaSecondClustersType));
    pSection->AddComment(" max reported geographic size (< max geographical cluster size%)");
    pSection->AddLine(REPORTED_GEOSIZE_LINE, AsString(sValue, gfMaxReportedGeographicClusterSize));
    pSection->AddComment(" use reported maximum geographical cluster size (y/n)");
    pSection->AddLine(USE_REPORTED_GEOSIZE_LINE, gbRestrictReportedClusters ? YES : NO);
  }
  catch (ZdException &x) {
    x.AddCallpath("SaveOutputFileSection()","CParameters");
    throw;
  }
}

// saves the scanning window section to the ini file
// pre: file is an open ZdIniFile
// post: write the appropraite global data to the file to the appropraite keys
void CParameters::SaveScanningWindowSection(ZdIniFile& file) {
  ZdString             sValue;
  ZdIniSection       * pSection;

  try {
    pSection = file.GetSection(SCANNING_WINDOW_SECTION);
    pSection->AddComment(" max geographic size (<=50%)");
    pSection->AddLine(MAX_GEO_SIZE_LINE, AsString(sValue, gfMaxGeographicClusterSize));
    pSection->AddComment(" how max spatial size should be interpretted (0=Percentage, 1=Distance, 2=Percentage of max circle population file)");
    pSection->AddLine(MAX_GEO_INTERPRET_LINE, AsString(sValue, geMaxGeographicClusterSizeType));
    pSection->AddComment(" include purely temporal clusters (y/n)");
    pSection->AddLine(INCLUDE_PURE_TEMP_LINE, gbIncludePurelyTemporalClusters ? YES : NO);
    pSection->AddComment(" max temporal size (<=90%)");
    pSection->AddLine(MAX_TEMP_SIZE_LINE, AsString(sValue, gfMaxTemporalClusterSize));
    pSection->AddComment(" how max temporal size should be interpretted (0=Percentage, 1=Time)");
    pSection->AddLine(MAX_TEMP_INTERPRET_LINE, AsString(sValue, geMaxTemporalClusterSizeType));
    pSection->AddComment(" include purely spatial clusters (y/n)");
    pSection->AddLine(INCLUDE_PURELY_SPATIAL_LINE, gbIncludePurelySpatialClusters ? YES : NO);
    pSection->AddComment(" clusters to include (0=All, 1=Alive, 2=Range)");
    pSection->AddLine(INLCUDE_CLUSTERS_LINE, AsString(sValue, geIncludeClustersType));
    pSection->AddComment(" start range interval of window (YYYY/MM/DD,YYYY/MM/DD)");
    sValue.printf("%s,%s", gsStartRangeStartDate.c_str(), gsStartRangeEndDate.c_str());  
    pSection->AddLine(STARTRANGE_LINE, sValue.GetCString());
    pSection->AddComment(" end range interval of window (YYYY/MM/DD,YYYY/MM/DD)");
    sValue.printf("%s,%s", gsEndRangeStartDate.c_str(), gsEndRangeEndDate.c_str());
    pSection->AddLine(ENDRANGE_LINE, sValue.GetCString());
  }
  catch (ZdException &x) {
    x.AddCallpath("SaveScanningWindowSection()","CParameters");
    throw;
  }
}

// saves the output file section to the ini file
// pre: file is an open ZdIniFile
// post: write the appropraite global data to the file to the appropraite keys
void CParameters::SaveSequentialScanSection(ZdIniFile& file) {
  ZdString              sValue;
  ZdIniSection        * pSection;

  try {
    pSection = file.GetSection(SEQUENTIAL_SCAN_SECTION);
    pSection->AddComment(" sequential scan (y/n)");
    pSection->AddLine(SEQUENTIAL_SCAN_LINE, gbSequentialRuns ? YES : NO);
    pSection->AddComment(" max iterations for sequential scan (0-32000)");
    pSection->AddLine(SEQUENTIAL_MAX_ITERS_LINE, AsString(sValue, giNumSequentialRuns));
    pSection->AddComment(" max p-Value for sequential scan (0.000-1.000)");
    pSection->AddLine(SEQUENTIAL_MAX_PVALUE_LINE, AsString(sValue, gbSequentialCutOffPValue));
  }
  catch (ZdException &x) {
    x.AddCallpath("SaveSequentialScanSection()","CParameters");
    throw;
  }
}

/** saves the system parameters section to the ini file */
void CParameters::SaveSystemSection(ZdIniFile& file) {
  ZdString              sValue;
  ZdIniSection        * pSection;

  try {
    pSection = file.GetSection(SYSTEM_SECTION);
    sValue.printf("%s.%s.%s", VERSION_MAJOR, VERSION_MINOR, VERSION_RELEASE);
    pSection->AddLine(CREATION_VERSION_LINE, sValue.GetCString());
  }
  catch (ZdException &x) {
    x.AddCallpath("SaveSystemSection()","CParameters");
    throw;
  }
}

// saves the time parameters section to the ini file
// pre: file is an open ZdIniFile
// post: write the appropraite global data to the file to the appropraite keys
void CParameters::SaveTimeParametersSection(ZdIniFile& file) {
  ZdString              sValue;
  ZdIniSection        * pSection;

  try {
    pSection = file.GetSection(TIME_PARAMS_SECTION);
    pSection->AddComment(" time aggregation units (0=None, 1=Year, 2=Month, 3=Day)");
    pSection->AddLine(TIME_AGGREGATION_UNITS_LINE, AsString(sValue, geTimeAggregationUnitsType));
    pSection->AddComment(" time aggregation length (positive integer)");
    pSection->AddLine(TIME_AGGREGATION_LENGTH_LINE, AsString(sValue, (int)glTimeAggregationLength));
    pSection->AddComment(" prospective surveillance start date (YYYY/MM/DD)");
    pSection->AddLine(PROSPECT_START_LINE, gsProspectiveStartDate.c_str());
    pSection->AddComment(" Time trend adjustment type (0=None, 1=Nonparametric, 2=LogLinearPercentage, 3=CalculatedLogLinearPercentage, 4=TimeStratifiedRandomization)");
    pSection->AddLine(TIME_TREND_ADJ_LINE, AsString(sValue, geTimeTrendAdjustType));
    pSection->AddComment(" time trend adjustment percentage (>-100)");
    pSection->AddLine(TIME_TREND_PERCENT_LINE, AsString(sValue, GetTimeTrendAdjustmentPercentage()));
    pSection->AddComment(" time trend convergence (> 0)");
    pSection->AddLine(TIME_TREND_CONVERGENCE_LINE, AsString(sValue, gdTimeTrendConverge));
    pSection->AddComment(" adjust for earlier analyses -- prospective only (y/n)");
    pSection->AddLine(ADJUST_EALIER_ANALYSES_LINE, gbAdjustForEarlierAnalyses ? YES : NO);
  }
  catch (ZdException &x) {
    x.AddCallpath("SaveTimeParametersSection()","CParameters");
    throw;
  }
}

/** Sets analysis type. Throws exception if out of range. */
void CParameters::SetAnalysisType(AnalysisType eAnalysisType) {
  ZdString      sLabel;

  try {
    if (eAnalysisType < PURELYSPATIAL || eAnalysisType > PROSPECTIVEPURELYTEMPORAL)
      InvalidParameterException::Generate("Error: For parameter '%s', setting '%d' is out of range(%d - %d).\n", "SetAnalysisType()",
                                          GetParameterLineLabel(ANALYSISTYPE, sLabel, geReadType == INI),
                                          eAnalysisType, PURELYSPATIAL, PROSPECTIVEPURELYTEMPORAL);
    geAnalysisType = eAnalysisType;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetAnalysisType()","CParameters");
    throw;
  }
}

/** Sets area rate for areas scanned type. Throws exception if out of range. */
void CParameters::SetAreaRateType(AreaRateType eAreaRateType) {
  ZdString      sLabel;

  try {
    if (eAreaRateType < HIGH || eAreaRateType > HIGHANDLOW)
      InvalidParameterException::Generate("Error: For parameter '%s', setting '%d' is out of range(%d - %d).\n", "SetAreaRateType()",
                                          GetParameterLineLabel(SCANAREAS, sLabel, geReadType == INI),
                                          eAreaRateType, HIGH, HIGHANDLOW);
    geAreaScanRate = eAreaRateType;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetAreaRateType()","CParameters");
    throw;
  }
}

/** Sets case data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void CParameters::SetCaseFileName(const char * sCaseFileName, bool bCorrectForRelativePath, unsigned int iStream) {
  try {
    if (! sCaseFileName)
      ZdGenerateException("Null pointer.", "SetCaseFileName()");

    if (!iStream)
      ZdGenerateException("Index out of range.", "SetCaseFileName()");

    if (iStream > gvCaseFilenames.size())
      gvCaseFilenames.resize(iStream);

    gvCaseFilenames[iStream - 1] = sCaseFileName;
    if (bCorrectForRelativePath)
      ConvertRelativePath(gvCaseFilenames[iStream - 1]);
  }
  catch (ZdException &x) {
    x.AddCallpath("SetCaseFileName()", "CParameters");
    throw;
  }
}

/** Sets control data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void CParameters::SetControlFileName(const char * sControlFileName, bool bCorrectForRelativePath, unsigned int iStream) {
  try {
    if (! sControlFileName)
      ZdGenerateException("Null pointer.", "SetControlFileName()");

    if (!iStream)
      ZdGenerateException("Index out of range.", "SetControlFileName()");

    if (iStream > gvControlFilenames.size())
      gvControlFilenames.resize(iStream);

    gvControlFilenames[iStream - 1] = sControlFileName;
    if (bCorrectForRelativePath)
      ConvertRelativePath(gvControlFilenames[iStream - 1]);
  }
  catch (ZdException &x) {
    x.AddCallpath("SetControlFileName()", "CParameters");
    throw;
  }
}

/** Sets coordinates data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void CParameters::SetCoordinatesFileName(const char * sCoordinatesFileName, bool bCorrectForRelativePath) {
  try {
    if (! sCoordinatesFileName)
      ZdGenerateException("Null pointer.", "SetCoordinatesFileName()");

    gsCoordinatesFileName = sCoordinatesFileName;
    if (bCorrectForRelativePath)
      ConvertRelativePath(gsCoordinatesFileName);
  }
  catch (ZdException &x) {
    x.AddCallpath("SetCoordinatesFileName()", "CParameters");
    throw;
  }
}

/** Sets precision of input file dates type. Throws exception if out of range. */
void CParameters::SetCoordinatesType(CoordinatesType eCoordinatesType) {
  ZdString      sLabel;

  try {
    if (eCoordinatesType < CARTESIAN || eCoordinatesType > LATLON)
      InvalidParameterException::Generate("Error: For parameter '%s', setting '%d' is out of range(%d - %d).\n", "SetCoordinatesType()",
                                          GetParameterLineLabel(SCANAREAS, sLabel, geReadType == INI),
                                          eCoordinatesType, CARTESIAN, LATLON);
    geCoordinatesType = eCoordinatesType;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetCoordinatesType()","CParameters");
    throw;
  }
}

/** Sets criteria for reporting secondary clusters. Throws exception if out of range. */
void CParameters::SetCriteriaForReportingSecondaryClusters(CriteriaSecondaryClustersType eCriteriaSecondaryClustersType) {
  ZdString      sLabel;

  try {
    if (eCriteriaSecondaryClustersType < NOGEOOVERLAP || eCriteriaSecondaryClustersType > NORESTRICTIONS)
      InvalidParameterException::Generate("Error: For parameter %s, setting '%d' is out of range(%d - %d).\n",
                                          "SetCriteriaForReportingSecondaryClusters()",
                                          GetParameterLineLabel(CRITERIA_SECOND_CLUSTERS, sLabel, geReadType == INI),
                                          eCriteriaSecondaryClustersType, NOGEOOVERLAP, NORESTRICTIONS);
    geCriteriaSecondClustersType = eCriteriaSecondaryClustersType;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetCriteriaForReportingSecondaryClusters()","CParameters");
    throw;
  }
}

/** initializes global variables to default values */
void CParameters::SetDefaults() {
  geAnalysisType                        = PURELYSPATIAL;
  geAreaScanRate                        = HIGH;
  gvCaseFilenames.resize(1);
  gvPopulationFilenames.resize(1);
  gsCoordinatesFileName                 = "";
  gsOutputFileName                      = "";
  gsMaxCirclePopulationFileName         = "";
  gePrecisionOfTimesType                = YEAR;
  giDimensionsOfData                    = 0;
  gbUseSpecialGridFile                  = false;
  gsSpecialGridFileName                 = "";
  gfMaxGeographicClusterSize            = 50.0; //GG980716
  geMaxGeographicClusterSizeType        = PERCENTOFPOPULATIONTYPE;
  gsStudyPeriodStartDate                = "2000/01/01";
  gsStudyPeriodEndDate                  = "2000/12/31";
  geIncludeClustersType                 = ALLCLUSTERS;
  geTimeAggregationUnitsType            = NONE;
  glTimeAggregationLength               = 0;
  gbIncludePurelySpatialClusters        = false;
  gfMaxTemporalClusterSize              = 50.0;//KR980707 0 GG980716;
  geMaxTemporalClusterSizeType          = PERCENTAGETYPE;
  giReplications                        = 999;
  gbOutputClusterLevelDBase             = false;
  gbOutputAreaSpecificDBase             = false;
  gbOutputRelativeRisksDBase            = false;
  gbOutputSimLogLikeliRatiosDBase       = false;
  gsRunHistoryFilename                  = "";
  gbLogRunHistory                       = true;
  geProbabiltyModelType                 = POISSON;
  geRiskFunctionType                    = STANDARDRISK;
  gbPowerCalculation                    = false;
  gdPower_X                             = 0.0;
  gdPower_Y                             = 0.0;
  geTimeTrendAdjustType                 = NOTADJUSTED;
  gdTimeTrendAdjustPercentage           = 0;
  gbIncludePurelyTemporalClusters       = false;
  gvControlFilenames.resize(1);
  geCoordinatesType                     = LATLON;
  gbOutputSimLogLikeliRatiosAscii       = false;
  gbSequentialRuns                      = false;
  giNumSequentialRuns                   = 0;
  gbSequentialCutOffPValue              = 0.0;
  gbValidatePriorToCalc                 = true;
  gbOutputRelativeRisksAscii            = false;
  giNumberEllipses                      = 0;
  gvEllipseShapes.clear();
  gvEllipseRotations.clear();
  gsProspectiveStartDate                = "2000/12/31";
  gbOutputAreaSpecificAscii             = false;
  gbOutputClusterLevelAscii             = false;
  geCriteriaSecondClustersType          = NOGEOOVERLAP;
  glTotalNumEllipses                    = 0;
  gbNonCompactnessPenalty               = false;
  gbReadStatusError                     = false;
  geReadType                            = SCAN;
  gsEndRangeStartDate                   = gsStudyPeriodStartDate;
  gsEndRangeEndDate                     = gsStudyPeriodEndDate;
  gsStartRangeStartDate                 = gsStudyPeriodStartDate;
  gsStartRangeEndDate                   = gsStudyPeriodEndDate;
  gdTimeTrendConverge			= 0.0000001;
  gbEarlyTerminationSimulations         = false;
  gbRestrictReportedClusters            = false;
  gfMaxReportedGeographicClusterSize    = gfMaxGeographicClusterSize;
  geSimulationType                      = STANDARD;
  gsSimulationDataSourceFileName        = "";
  gsAdjustmentsByRelativeRisksFileName  = "";
  gbOutputSimulationData                = false;
  gsSimulationDataOutputFilename        = "";
  gbAdjustForEarlierAnalyses            = false;
  gbUseAdjustmentsForRRFile             = false;
  geSpatialAdjustmentType               = NO_SPATIAL_ADJUSTMENT;
  geMultipleStreamPurposeType           = MULTIVARIATE;
  //default to 4.0.3, the last version prior to 'version' parameter
  gCreationVersion.iMajor               = 4;
  gCreationVersion.iMinor               = 0;
  gCreationVersion.iRelease             = 3;
}

/** Sets dimensions of input data. */
void CParameters::SetDimensionsOfData(int iDimensions) {
  try {
    if (iDimensions < 0)
      InvalidParameterException::Generate("Error: Dimensions can not be less than 0.\n", "SetDimensionsOfData()");
    giDimensionsOfData = iDimensions;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetDimensionsOfData()","CParameters");
    throw;
  }
}

/** Sets ellipsoid shape for ellipse at index. If index = -1, value is pushed back on list. */
void CParameters::SetEllipsoidShape(double dShape, int iEllipsoidIndex) {
  try {
    if (iEllipsoidIndex < -1 || iEllipsoidIndex > giNumberEllipses - 1)
      ZdException::Generate("Index '%d' out of range(0 - %d).\n", "SetEllipsoidShape()", iEllipsoidIndex, giNumberEllipses - 1);

    if (iEllipsoidIndex >= 0)
      gvEllipseShapes[iEllipsoidIndex] = dShape;
    else
      gvEllipseShapes.push_back(dShape);
  }
  catch (ZdException & x) {
    x.AddCallpath("SetEllipsoidShape()","CParameters");
    throw;
  }
}

/** Sets start range start date. Throws exception. */
void CParameters::SetEndRangeEndDate(const char * sEndRangeEndDate) {
  ZdString      sLabel;

  try {
    if (!sEndRangeEndDate)
      ZdException::Generate("Null pointer.","SetEndRangeEndDate()");

    if (strspn(sEndRangeEndDate,"0123456789/") < strlen(sEndRangeEndDate))
      InvalidParameterException::Generate("Error: For parameter %s, setting '%s' does not appear to be a date.\n",
                                          "SetEndRangeEndDate()",
                                          GetParameterLineLabel(INTERVAL_ENDRANGE, sLabel, geReadType == INI),
                                          sEndRangeEndDate);
    gsEndRangeEndDate = sEndRangeEndDate;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetEndRangeEndDate()","CParameters");
    throw;
  }
}

/** Sets end range start date. Throws exception. */
void CParameters::SetEndRangeStartDate(const char * sEndRangeStartDate) {
  ZdString      sLabel;

  try {
    if (!sEndRangeStartDate)
      ZdException::Generate("Null pointer.","SetEndRangeStartDate()");

    if (strspn(sEndRangeStartDate,"0123456789/") < strlen(sEndRangeStartDate))
      InvalidParameterException::Generate("Error: For parameter %s, setting '%s' does not appear to be a date.\n",
                                          "SetEndRangeStartDate()",
                                          GetParameterLineLabel(INTERVAL_ENDRANGE, sLabel, geReadType == INI),
                                          sEndRangeStartDate);
    gsEndRangeStartDate = sEndRangeStartDate;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetEndRangeStartDate()","CParameters");
    throw;
  }
}

/** Sets clusters to include type. Throws exception if out of range. */
void CParameters::SetIncludeClustersType(IncludeClustersType eIncludeClustersType) {
  ZdString      sLabel;

  try {
    if (ALLCLUSTERS > eIncludeClustersType || CLUSTERSINRANGE < eIncludeClustersType)
      InvalidParameterException::Generate("Error: For parameter '%s', setting '%d' is out of range(%d - %d).\n",
                                          "SetIncludeClustersType()",
                                          GetParameterLineLabel(CLUSTERS, sLabel, geReadType == INI),
                                          eIncludeClustersType, ALLCLUSTERS, CLUSTERSINRANGE);
    geIncludeClustersType = eIncludeClustersType;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetIncludeClustersType()","CParameters");
    throw;
  }
}

/** Sets maximum geographic cluster size. */
void CParameters::SetMaximumGeographicClusterSize(float fMaxGeographicClusterSize) {
  //Validity of setting is checked in ValidateParameters() since this setting
  //might not be pertinent in calculation.
  gfMaxGeographicClusterSize = fMaxGeographicClusterSize;
}

/** Sets maximum reported geographic cluster size. */
void CParameters::SetMaximumReportedGeographicalClusterSize(float fMaxReportedGeographicClusterSize) {
  //Validity of setting is checked in ValidateParameters() since this setting
  //might not be pertinent in calculation.
  gfMaxReportedGeographicClusterSize = fMaxReportedGeographicClusterSize;
}

/** Sets maximum spacial cluster size type. Throws exception if out of range. */
void CParameters::SetMaximumSpacialClusterSizeType(SpatialSizeType eSpatialSizeType) {
  ZdString      sLabel;

  try {
    if (PERCENTOFPOPULATIONTYPE > eSpatialSizeType || PERCENTOFPOPULATIONFILETYPE < eSpatialSizeType)
      InvalidParameterException::Generate("Error: For parameter '%s', setting '%d' is out of range(%d - %d).\n",
                                          "SetMaximumSpacialClusterSizeType()",
                                          GetParameterLineLabel(MAX_SPATIAL_TYPE, sLabel, geReadType == INI),
                                          eSpatialSizeType, PERCENTOFPOPULATIONTYPE, PERCENTOFPOPULATIONFILETYPE);
    geMaxGeographicClusterSizeType = eSpatialSizeType;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetMaximumSpacialClusterSizeType()","CParameters");
    throw;
  }
}

/** Sets maximum temporal cluster size. */
void CParameters::SetMaximumTemporalClusterSize(float fMaxTemporalClusterSize) {
  //Validity of setting is checked in ValidateParameters() since this setting
  //might not be pertinent in calculation.
  gfMaxTemporalClusterSize = fMaxTemporalClusterSize;
}

/** Sets maximum temporal cluster size type. Throws exception if out of range. */
void CParameters::SetMaximumTemporalClusterSizeType(TemporalSizeType eTemporalSizeType) {
  ZdString      sLabel;

  try {
    if (PERCENTAGETYPE > eTemporalSizeType || TIMETYPE < eTemporalSizeType)
      InvalidParameterException::Generate("Error: For parameter '%s', setting '%d' is out of range(%d - %d).\n",
                                          "SetMaximumTemporalClusterSizeType()",
                                          GetParameterLineLabel(MAX_TEMPORAL_TYPE, sLabel, geReadType == INI),
                                          eTemporalSizeType, PERCENTAGETYPE, TIMETYPE);
    geMaxTemporalClusterSizeType = eTemporalSizeType;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetMaximumTemporalClusterSizeType()","CParameters");
    throw;
  }
}

/** Adjusts the number of data streams. */
void CParameters::SetNumDataStreams(unsigned int iNumStreams) {
  size_t        t;

  try {
    if (iNumStreams == 0)
      ZdException::Generate("Number of data streams can not be zero.\n", "SetNumDataStreams()");

    //adjust the number of filenames for case, control, and population
    gvCaseFilenames.resize(iNumStreams);
    gvControlFilenames.resize(iNumStreams);
    gvPopulationFilenames.resize(iNumStreams);
  }
  catch (ZdException & x) {
    x.AddCallpath("SetNumDataStreams()","CParameters");
    throw;
  }
}

/** Sets number of ellipses requested. */
void CParameters::SetNumberEllipses(int iNumEllipses) {
  ZdString      sLabel;

  try {
    if (iNumEllipses < 0)
      InvalidParameterException::Generate("Error: For parameter '%s', value '%d' is below minimum value of zero.\n",
                                          "SetNumberEllipses()",
                                          GetParameterLineLabel(ELLIPSES, sLabel, geReadType == INI),
                                          iNumEllipses);
    giNumberEllipses = iNumEllipses;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetNumberEllipses()","CParameters");
    throw;
  }
}

/** Sets ellipsoid rotations for ellipse at index. */
void CParameters::SetNumberEllipsoidRotations(int iNumberRotations, int iEllipsoidIndex) {
  size_t        t;

  try {
    if (iEllipsoidIndex < -1 || iEllipsoidIndex > giNumberEllipses - 1)
      ZdException::Generate("Index '%d' out of range(0 - %d).\n", "SetNumberEllipsoidRotations()", -1, giNumberEllipses - 1);

    if (iEllipsoidIndex >= 0)
      gvEllipseRotations[iEllipsoidIndex] = iNumberRotations;
    else
      gvEllipseRotations.push_back(iNumberRotations);

    //re-calculate number of total ellispes
    glTotalNumEllipses = 0;
    for (t=0; t < gvEllipseRotations.size(); t++)
       glTotalNumEllipses += gvEllipseRotations[t];
  }
  catch (ZdException & x) {
    x.AddCallpath("SetNumberEllipsoidRotations()","CParameters");
    throw;
  }
}

/** Sets number of Monte Carlo replications to run. */
void CParameters::SetNumberMonteCarloReplications(int iReplications) {
  //Validity of setting is checked in ValidateParameters().
  giReplications = iReplications;
}

/** Sets number of seqential scans to run. */
void CParameters::SetNumSequentialScans(int iNumSequentialScans) {
  //Validity of setting is checked in ValidateParameters() since this setting
  //might not be pertinent in calculation.
  giNumSequentialRuns = iNumSequentialScans;
}

/** Sets output data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file
    does not exist. */
void CParameters::SetOutputFileName(const char * sOutPutFileName, bool bCorrectForRelativePath) {
  try {
    if (! sOutPutFileName)
      ZdGenerateException("Null pointer.", "SetOutputFileName()");

    gsOutputFileName = sOutPutFileName;
    if (bCorrectForRelativePath)
      ConvertRelativePath(gsOutputFileName);
  }
  catch (ZdException &x) {
    x.AddCallpath("SetOutputFileName()", "CParameters");
    throw;
  }
}

/** Sets population data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void CParameters::SetPopulationFileName(const char * sPopulationFileName, bool bCorrectForRelativePath, unsigned int iStream) {
  try {
    if (! sPopulationFileName)
      ZdGenerateException("Null pointer.", "SetPopulationFileName()");

    if (!iStream)
      ZdGenerateException("Index out of range.", "SetPopulationFileName()");

    if (iStream > gvPopulationFilenames.size())
      gvPopulationFilenames.resize(iStream);

    gvPopulationFilenames[iStream - 1] = sPopulationFileName;
    if (bCorrectForRelativePath)
      ConvertRelativePath(gvPopulationFilenames[iStream - 1]);
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPopulationFileName()", "CParameters");
    throw;
  }
}

/** Sets X variable for power calculation. Throws exception if out of range. */
void CParameters::SetPowerCalculationX(double dPowerX) {
  //Validity of setting is checked in ValidateParameters() since this setting
  //might not be pertinent in calculation.
  gdPower_X = dPowerX;
}

/** Sets Y variable for power calculation. Throws exception if out of range. */
void CParameters::SetPowerCalculationY(double dPowerY) {
  //Validity of setting is checked in ValidateParameters() since this setting
  //might not be pertinent in calculation.
  gdPower_Y = dPowerY;
}

/** Sets relative risks adjustments file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void CParameters::SetAdjustmentsByRelativeRisksFilename(const char * sFileName, bool bCorrectForRelativePath) {
  try {
    if (! sFileName)
      ZdGenerateException("Null pointer.", "SetAdjustmentsByRelativeRisksFilename()");

    gsAdjustmentsByRelativeRisksFileName = sFileName;
    if (bCorrectForRelativePath)
      ConvertRelativePath(gsAdjustmentsByRelativeRisksFileName);
  }
  catch (ZdException &x) {
    x.AddCallpath("SetAdjustmentsByRelativeRisksFilename()", "CParameters");
    throw;
  }
}

/** Sets precision of input file dates type. Throws exception if out of range. */
void CParameters::SetPrecisionOfTimesType(DatePrecisionType eDatePrecisionType) {
  ZdString      sLabel;

  try {
    if (eDatePrecisionType < NONE || eDatePrecisionType > DAY)
      InvalidParameterException::Generate("Error: For parameter %s, setting '%d' is out of range(%d - %d).\n",
                                          "SetPrecisionOfTimesType()",
                                          GetParameterLineLabel(PRECISION, sLabel, geReadType == INI),
                                          eDatePrecisionType, NONE, DAY);
    gePrecisionOfTimesType = eDatePrecisionType;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPrecisionOfTimesType()","CParameters");
    throw;
  }
}

/** Sets probabilty model type. Throws exception if out of range. */
void CParameters::SetProbabilityModelType(ProbabiltyModelType eProbabiltyModelType) {
  ZdString      sLabel;

  try {
    if (eProbabiltyModelType < POISSON || eProbabiltyModelType > RANK)
      InvalidParameterException::Generate("Error: For parameter '%s', setting '%d' is out of range(%d - %d).\n",
                                          "SetAnalysisType()",
                                          GetParameterLineLabel(MODEL, sLabel, geReadType == INI),
                                          eProbabiltyModelType, POISSON, RANK);

    geProbabiltyModelType = eProbabiltyModelType;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetProbabilityModelType()","CParameters");
    throw;
  }
}

/** Sets prospective start date. Throws exception if out of range. */
void CParameters::SetProspectiveStartDate(const char * sProspectiveStartDate) {
  ZdString      sLabel;

  try {
    if (!sProspectiveStartDate)
      ZdException::Generate("Null pointer.","SetProspectiveStartDate()");

    if (strspn(sProspectiveStartDate,"0123456789/") < strlen(sProspectiveStartDate))
      InvalidParameterException::Generate("Error: For parameter %s, setting '%s' does not appear to be a date.\n",
                                          "SetProspectiveStartDate()",
                                          GetParameterLineLabel(START_PROSP_SURV, sLabel, geReadType == INI),
                                          sProspectiveStartDate);
    gsProspectiveStartDate = sProspectiveStartDate;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetProspectiveStartDate()","CParameters");
    throw;
  }
}

/** Sets risk type. Throws exception if out of range. */
void CParameters::SetRiskType(RiskType eRiskType) {
  ZdString      sLabel;

  try {
    if (eRiskType < STANDARDRISK || eRiskType > MONOTONERISK)
      InvalidParameterException::Generate("Error: For parameter %s, setting '%d' is out of range(%d - %d).\n",
                                          "SetRiskType()",
                                          GetParameterLineLabel(RISKFUNCTION, sLabel, geReadType == INI),
                                          eRiskType, STANDARDRISK, MONOTONERISK);
    geRiskFunctionType = eRiskType;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetRiskType()","CParameters");
    throw;
  }
}

/** sets simulation procedure type */
void CParameters::SetSimulationType(SimulationType eSimulationType) {
  ZdString      sLabel;

  try {
    if (eSimulationType < STANDARD || eSimulationType > FILESOURCE)
      InvalidParameterException::Generate("Error: For parameter %s, setting '%d' is out of range(%d - %d).\n",
                                          "SetSimulationType()",
                                          GetParameterLineLabel(SIMULATION_TYPE, sLabel, geReadType == INI),
                                          eSimulationType, STANDARD, FILESOURCE);
    geSimulationType = eSimulationType;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetSimulationType()","CParameters");
    throw;
  }
}

/** Sets simulation data output file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void CParameters::SetSimulationDataOutputFileName(const char * sSourceFileName, bool bCorrectForRelativePath) {
  try {
    if (! sSourceFileName)
      ZdGenerateException("Null pointer.", "SetSimulationDataOutputFileName()");

    gsSimulationDataOutputFilename = sSourceFileName;
    if (bCorrectForRelativePath)
      ConvertRelativePath(gsSimulationDataOutputFilename);
  }
  catch (ZdException &x) {
    x.AddCallpath("SetSimulationDataOutputFileName()", "CParameters");
    throw;
  }
}

/** Sets simulation data source file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void CParameters::SetSimulationDataSourceFileName(const char * sSourceFileName, bool bCorrectForRelativePath) {
  try {
    if (! sSourceFileName)
      ZdGenerateException("Null pointer.", "SetSimulationDataSourceFileName()");

    gsSimulationDataSourceFileName = sSourceFileName;
    if (bCorrectForRelativePath)
      ConvertRelativePath(gsSimulationDataSourceFileName);
  }
  catch (ZdException &x) {
    x.AddCallpath("SetSimulationDataSourceFileName()", "CParameters");
    throw;
  }
}

/** Set spatial adjustment type. Throws exception if out of range. */
void CParameters::SetSpatialAdjustmentType(SpatialAdjustmentType eSpatialAdjustmentType) {
  ZdString      sLabel;

  try {
    if (eSpatialAdjustmentType < NO_SPATIAL_ADJUSTMENT || eSpatialAdjustmentType > SPATIALLY_STRATIFIED_RANDOMIZATION)
      InvalidParameterException::Generate("Error: For parameter %s, setting '%d' is out of range(%d - %d).\n",
                                          "SetSpatialAdjustmentType()",
                                          GetParameterLineLabel(SPATIAL_ADJ_TYPE, sLabel, geReadType == INI),
                                          eSpatialAdjustmentType, NO_SPATIAL_ADJUSTMENT, SPATIALLY_STRATIFIED_RANDOMIZATION);
    geSpatialAdjustmentType = eSpatialAdjustmentType;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetSpatialAdjustmentType()","CParameters");
    throw;
  }
}

/** Set p-value that is cut-off for sequentail scans. */
void CParameters::SetSequentialCutOffPValue(double dPValue) {
  //Validity of setting is checked in ValidateParameters() since this setting
  //might not be pertinent in calculation.
  gbSequentialCutOffPValue = dPValue;
}

/** Sets filename of file used to load parameters. */
void CParameters::SetSourceFileName(const char * sParametersSourceFileName) {
  try {
    if (! sParametersSourceFileName)
      ZdGenerateException("Null pointer.", "SetSourceFileName()");
    //Use ZdFileName class to ensure that a relative path is expanded to absolute path.  
    gsParametersSourceFileName = ZdFileName(sParametersSourceFileName).GetFullPath();
  }
  catch (ZdException &x) {
    x.AddCallpath("SetSourceFileName()", "CParameters");
    throw;
  }
}

/** Sets special grid data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void CParameters::SetSpecialGridFileName(const char * sSpecialGridFileName, bool bCorrectForRelativePath, bool bSetUsingFlag) {
  try {
    if (! sSpecialGridFileName)
      ZdGenerateException("Null pointer.", "SetSpecialGridFileName()");

    gsSpecialGridFileName = sSpecialGridFileName;
    if (bCorrectForRelativePath)
      ConvertRelativePath(gsSpecialGridFileName);

    if (gsSpecialGridFileName.empty())
      gbUseSpecialGridFile = false; //If empty, then definately not using special grid.
    else if (bSetUsingFlag)
      gbUseSpecialGridFile = true;  //Permits setting special grid filename in GUI interface
                                  //where obviously the use of special grid file is the desire.
    //else gbUseSpecialGridFile is as set from parameters read. This permits the situation
    //where user has modified the paramters file manually so that there is a named
    //special grid file but they turned off option to use it. 
  }
  catch (ZdException &x) {
    x.AddCallpath("SetSpecialGridFileName()", "CParameters");
    throw;
  }
}

/** Sets maximum circle population data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void CParameters::SetMaxCirclePopulationFileName(const char * sMaxCirclePopulationFileName, bool bCorrectForRelativePath, bool bSetUsingFlag) {
  try {
    if (! sMaxCirclePopulationFileName)
      ZdGenerateException("Null pointer.", "SetMaxCirclePopulationFileName()");

    gsMaxCirclePopulationFileName = sMaxCirclePopulationFileName;
    if (bCorrectForRelativePath)
      ConvertRelativePath(gsMaxCirclePopulationFileName);
  }
  catch (ZdException &x) {
    x.AddCallpath("SetMaxCirclePopulationFileName()", "CParameters");
    throw;
  }
}

/** Set multiple data stream purpose type. Throws exception if out of range. */
void CParameters::SetMultipleDataStreamPurposeType(MultipleStreamPurposeType eType) {
  ZdString      sLabel;

  try {
    if (eType < MULTIVARIATE || eType > ADJUSTMENT)
      InvalidParameterException::Generate("Error: For parameter %s, setting '%d' is out of range(%d - %d).\n",
                                          "SetMultipleDataStreamPurposeType()",
                                          GetParameterLineLabel(MULTI_STREAM_PURPOSE_TYPE, sLabel, geReadType == INI),
                                          eType, MULTIVARIATE, ADJUSTMENT);
    geMultipleStreamPurposeType = eType;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetMultipleDataStreamPurposeType()","CParameters");
    throw;
  }
}

/** Sets study period start date. Throws exception if out of range. */
void CParameters::SetStudyPeriodEndDate(const char * sStudyPeriodEndDate) {
  try {
    if (!sStudyPeriodEndDate)
      ZdException::Generate("Null pointer.","SetStudyPeriodEndDate()");

    gsStudyPeriodEndDate = sStudyPeriodEndDate;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetStudyPeriodEndDate()","CParameters");
    throw;
  }
}

/** Sets study period start date. Throws exception if out of range. */
void CParameters::SetStudyPeriodStartDate(const char * sStudyPeriodStartDate) {
  try {
    if (!sStudyPeriodStartDate)
      ZdException::Generate("Null pointer.","SetStudyPeriodStartDate()");

    gsStudyPeriodStartDate = sStudyPeriodStartDate;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetStudyPeriodStartDate()","CParameters");
    throw;
  }
}

/** Sets time aggregation length. Throws exception if out of range. */
void CParameters::SetTimeAggregationLength(long lTimeAggregationLength) {
  //Validity of setting is checked in ValidateParameters() since this setting
  //might not be pertinent in calculation.
  glTimeAggregationLength = lTimeAggregationLength;
}

/** Sets precision of time interval units type. Throws exception if out of range. */
void CParameters::SetTimeAggregationUnitsType(DatePrecisionType eTimeAggregationUnits) {
  ZdString      sLabel;

  try {
    if (eTimeAggregationUnits < NONE || eTimeAggregationUnits > DAY)
      InvalidParameterException::Generate("Error: For parameter %s, setting '%d' is out of range(%d - %d).\n",
                                          "SetTimeAggregationUnitsType()",
                                          GetParameterLineLabel(TIME_AGGREGATION_UNITS, sLabel, geReadType == INI),
                                          eTimeAggregationUnits, NONE, DAY);
    geTimeAggregationUnitsType = eTimeAggregationUnits;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetTimeAggregationUnitsType()","CParameters");
    throw;
  }
}

/** Sets time trend adjustment percentage. Throws exception if out of range. */
void CParameters::SetTimeTrendAdjustmentPercentage(double dPercentage) {
  //Validity of setting is checked in ValidateParameters() since this setting
  //might not be pertinent in calculation.
   gdTimeTrendAdjustPercentage = dPercentage;
}

/** Sets time rend adjustment type. Throws exception if out of range. */
void CParameters::SetTimeTrendAdjustmentType(TimeTrendAdjustmentType eTimeTrendAdjustmentType) {
  ZdString      sLabel;

  try {
    if (eTimeTrendAdjustmentType < NOTADJUSTED || eTimeTrendAdjustmentType > STRATIFIED_RANDOMIZATION)
      InvalidParameterException::Generate("Error: For parameter %s, setting '%d' is out of range(%d - %d).\n",
                                          "SetTimeTrendAdjustmentType()",
                                          GetParameterLineLabel(TIMETREND, sLabel, geReadType == INI),
                                          eTimeTrendAdjustmentType, NOTADJUSTED, STRATIFIED_RANDOMIZATION);
    geTimeTrendAdjustType = eTimeTrendAdjustmentType;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetTimeTrendAdjustmentType()","CParameters");
    throw;
  }
}

/** Sets time trend convergence variable. */
void CParameters::SetTimeTrendConvergence(double dTimeTrendConvergence) {
  //Validity of setting is checked in ValidateParameters() since this setting
  //might not be pertinent in calculation.
   gdTimeTrendConverge = dTimeTrendConvergence;
}

/** Set version number that indicates what version of SaTScan created these parameters. */
void CParameters::SetVersion(const ZdString& sValue) {
  sscanf(sValue.GetCString(), "%u.%u.%u", &gCreationVersion.iMajor, &gCreationVersion.iMinor, &gCreationVersion.iRelease);
//  if (gCreationVersion.iMajor > (unsigned int)atoi(VERSION_MAJOR)) {
//    gCreationVersion.iMajor = atoi(VERSION_MAJOR);
//    gCreationVersion.iMinor = atoi(VERSION_MINOR);
//    gCreationVersion.iRelease = atoi(VERSION_RELEASE);
//  }
}

bool CParameters::UseMaxCirclePopulationFile() const {
  bool  bRequiredForProspective, bAskForByUser;

  bAskForByUser = GetMaxGeographicClusterSizeType() == PERCENTOFPOPULATIONFILETYPE;
  bAskForByUser &= GetAnalysisType() != PURELYTEMPORAL && GetAnalysisType() != PROSPECTIVEPURELYTEMPORAL;
  bRequiredForProspective = GetAnalysisType() == PROSPECTIVESPACETIME;
  bRequiredForProspective &= GetMaxGeographicClusterSizeType() == PERCENTOFPOPULATIONFILETYPE;
  bRequiredForProspective &= GetAdjustForEarlierAnalyses();

  return bAskForByUser || bRequiredForProspective;
}

/** Validates date parameters based upon current settings. Error messages
    sent to print direction object and indication of valid settings returned. */
bool CParameters::ValidateDateParameters(BasePrint& PrintDirection) const {
  bool          bValid=true, bStartDateValid=true, bEndDateValid=true, bProspectiveDateValid=true;
  Julian        StudyPeriodStartDate, StudyPeriodEndDate, ProspectiveStartDate;

  try {
    //validate study period start date based upon 'precision of times' parameter setting
    if (!ValidateStudyPeriodStartDate(PrintDirection)) {
      bValid = false;
      bStartDateValid = false;
    }
    //validate study period end date based upon precision of times parameter setting
    if (!ValidateStudyPeriodEndDate(PrintDirection)) {
      bValid = false;
      bEndDateValid = false;
    }
    //validate prospective start date based upon precision of times parameter setting
    if (GetIsProspectiveAnalysis() && !ValidateProspectiveDate(PrintDirection)) {
      bValid = false;
      bProspectiveDateValid = false;
    }

    if (bStartDateValid && bEndDateValid) {
      //check that study period start and end dates are chronologically correct
      StudyPeriodStartDate = CharToJulian(gsStudyPeriodStartDate.c_str());
      StudyPeriodEndDate = CharToJulian(gsStudyPeriodEndDate.c_str());
      if (StudyPeriodStartDate > StudyPeriodEndDate) {
        bValid = false;
        PrintDirection.SatScanPrintWarning("Error: The study period start date occurs after the end date.\n");
      }
      if (bValid && GetIsProspectiveAnalysis() && bProspectiveDateValid) {
        //validate prospective start date
        ProspectiveStartDate = CharToJulian(gsProspectiveStartDate.c_str());
        if (ProspectiveStartDate < StudyPeriodStartDate || ProspectiveStartDate > StudyPeriodEndDate) {
          bValid = false;
          PrintDirection.SatScanPrintWarning("Error: The start date of prospective surveillance does not occur within\n"
                                             "       the study period.\n");
        }
      }
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateDateParameters()","CParameters");
    throw;
  }                                                         
  return bValid;
}

/** Validates end date for the following attributes:
    - date contains year, month and day.
    - date is a real date.
    - checks that date agrees with precision of times settings
      ;precision year - month = 12 and day = 31
      ;precision month - day = last day of month
   Returns boolean indication of whether date is valid, printing relevant
   messages to BasePrint object.                                              */
bool CParameters::ValidateStudyPeriodEndDate(BasePrint& PrintDirection) const {
  UInt                  nYear, nMonth, nDay;
  DatePrecisionType     ePrecision = (gCreationVersion.iMajor == 4 ? geTimeAggregationUnitsType : gePrecisionOfTimesType);

  try {
    if (gCreationVersion.iMajor == 4 && gePrecisionOfTimesType != NONE)
      ePrecision = geTimeAggregationUnitsType;
    else
      ePrecision = gePrecisionOfTimesType;

    //parse date in parts
    if (CharToMDY(&nMonth, &nDay, &nYear, gsStudyPeriodEndDate.c_str()) != 3) {
      PrintDirection.SatScanPrintWarning("Error: The study period end date, '%s', is not valid.\n"
                                         "       Please specify as YYYY/MM/DD.\n", gsStudyPeriodEndDate.c_str());
      return false;
    }
    //validate date
    if (!IsDateValid(nMonth, nDay, nYear)) {
      PrintDirection.SatScanPrintWarning("Error: The study period end date, '%s', is not a valid date.\n", gsStudyPeriodEndDate.c_str());
      return false;
    }
    //validate against precision of times
    switch (ePrecision) {
      case YEAR  :
        if (nMonth != 12 || nDay != 31) {
          PrintDirection.SatScanPrintWarning("Error: The study period end date, '%s', is not valid.\n"
                                             "       With the setting for %s as years, the date\n"
                                             "       must be the last day of respective year.\n",
                                             gsStudyPeriodEndDate.c_str(),
                                             (gCreationVersion.iMajor == 4 ? "time aggregation" : "time precision"));
          return false;
        }
        break;
      case MONTH :
        if (nDay != DaysThisMonth(nYear, nMonth)) {
          PrintDirection.SatScanPrintWarning("Error: The study period end date, '%s', is not valid.\n"
                                             "       With the setting for %s as months, the date\n"
                                             "       must be the last day of respective month.\n",
                                             gsStudyPeriodEndDate.c_str(),
                                             (gCreationVersion.iMajor == 4 ? "time aggregation" : "time precision"));
          return false;
        }
      case DAY   :
      case NONE  : break;
    };
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateEndDate()", "CParameters");
    throw;
  }
  return true;
}

/** Validates ellipse parameters if number of ellipses greater than zero.
    Errors printed to print direction and return whether parameters are valid. */
bool CParameters::ValidateEllipseParameters(BasePrint & PrintDirection) {
  bool          bValid=true;
  size_t        t;

  try {
    if (giNumberEllipses < 0 || giNumberEllipses > MAXIMUM_ELLIPSOIDS)
      InvalidParameterException::Generate("Error: The number of requested ellipses '%d' is not within allowable range of 0 - %d.\n",
                                          "ValidateEllipseParameters()", giNumberEllipses, MAXIMUM_ELLIPSOIDS);

    if (giNumberEllipses) {
      //analyses with ellipses can not be performed with coordinates defiend in latitude/longitude system (currently)
      if (geCoordinatesType == LATLON) {
        bValid = false;
        PrintDirection.SatScanPrintWarning("Error: Invalid parameter setting for ellipses.\n");
        PrintDirection.SatScanPrintWarning("       SaTScan does not support lat/long coordinates when ellipses are used.\n"
                                           "       Please use the Cartesian coordinate system.\n");
      }
      if (giNumberEllipses != (int)gvEllipseShapes.size()) {
        bValid = false;
        PrintDirection.SatScanPrintWarning("Error: Invalid parameter setting, %d ellipses requested but %d shapes were specified.\n",
                                           giNumberEllipses, (int)gvEllipseShapes.size());
      }
      for (t=0; t < gvEllipseShapes.size(); t++)
         if (gvEllipseShapes[t] < 1) {
           bValid = false;
           PrintDirection.SatScanPrintWarning("Error: Invalid parameter setting, ellipse shape '%g' is invalid.\n", gvEllipseShapes[t]);
           PrintDirection.SatScanPrintWarning("       The shape can not be less than one.\n");

         }
      if (giNumberEllipses != (int)gvEllipseRotations.size()) {
        bValid = false;
        PrintDirection.SatScanPrintWarning("Error: Invalid parameter setting, %d ellipses requested but %d angle numbers were specified.\n",
                                           giNumberEllipses, (int)gvEllipseRotations.size());
      }
      for (t=0; t < gvEllipseRotations.size(); t++)
         if (gvEllipseRotations[t] < 1) {
           bValid = false;
           PrintDirection.SatScanPrintWarning("Error: Invalid parameter setting. The number of ellipse angles '%d' that were requested is invalid.\n", gvEllipseRotations[t]);
           PrintDirection.SatScanPrintWarning("       The number of angles can not be less than one.\n");
         }
    }
    else {
      //If there are no ellipses, then these variables must be reset to ensure that no code that
      //accesses them will wrongly think there are elipses.
      gbNonCompactnessPenalty = false;
      glTotalNumEllipses = 0;
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateEllipseParameters()","CParameters");
    throw;
  }
  return bValid;
}

/** Validates input/output file parameters. */
bool CParameters::ValidateFileParameters(BasePrint & PrintDirection) {
  bool          bValid=true;
  size_t        t;

  try {
    //validate number of data stream files match

    //validate case file
    if (!gvCaseFilenames.size()) {
      bValid = false;
      PrintDirection.SatScanPrintWarning("Error: No case file was specified.\n");
    }
    for (t=0; t < gvCaseFilenames.size(); ++t) {
       if (access(gvCaseFilenames[t].c_str(), 00)) {
         bValid = false;
         PrintDirection.SatScanPrintWarning("Error: The case file '%s' does not exist.\n", gvCaseFilenames[t].c_str());
         PrintDirection.SatScanPrintWarning("       Please check to make sure the path is correct.\n");
       }
    }
    //validate population file for a poisson model.
    if (geProbabiltyModelType == POISSON) {
      if (!gvPopulationFilenames.size()) {
        bValid = false;
        PrintDirection.SatScanPrintWarning("Error: For the Poisson model, a population file must be specified.\n");
      }
      for (t=0; t < gvPopulationFilenames.size(); ++t) {
        if (access(gvPopulationFilenames[t].c_str(), 00)) {
          bValid = false;
          PrintDirection.SatScanPrintWarning("Error: The population file '%s' does not exist.\n", gvPopulationFilenames[t].c_str());
          PrintDirection.SatScanPrintWarning("       Please check to make sure the path is correct.\n");
        }
      }
    }
    //validate control file for a bernoulli model.
    if (geProbabiltyModelType == BERNOULLI) {
      if (!gvControlFilenames.size()) {
        bValid = false;
        PrintDirection.SatScanPrintWarning("Error: For the Bernoulli model, a Control file must be specified.\n");
      }
      for (t=0; t < gvControlFilenames.size(); ++t) {
        if (access(gvControlFilenames[t].c_str(), 00)) {
          bValid = false;
          PrintDirection.SatScanPrintWarning("Error: The control file '%s' does not exist.\n", gvControlFilenames[t].c_str());
          PrintDirection.SatScanPrintWarning("       Please check to make sure the path is correct.\n");
        }
      }
    }
    //validate coordinates file
    if (gsCoordinatesFileName.empty()) {
      bValid = false;
      PrintDirection.SatScanPrintWarning("Error: No coordinates file specified.\n");
    }
    else if (access(gsCoordinatesFileName.c_str(), 00)) {
      bValid = false;
      PrintDirection.SatScanPrintWarning("Error: The coordinates file '%s' does not exist.\n", gsCoordinatesFileName.c_str());
      PrintDirection.SatScanPrintWarning("       Please check to make sure the path is correct.\n");
    }
    //validate special grid file
    if (gbUseSpecialGridFile && gsSpecialGridFileName.empty()) {
      bValid = false;
      PrintDirection.SatScanPrintWarning("Error: The settings indicate to the use a grid file, but a grid file name is not specified.\n");
    }
    else if (gbUseSpecialGridFile && access(gsSpecialGridFileName.c_str(), 00)) {
      bValid = false;
      PrintDirection.SatScanPrintWarning("Error: The grid file '%s' does not exist.\n", gsSpecialGridFileName.c_str());
      PrintDirection.SatScanPrintWarning("       Please check to make sure the path is correct.\n");
    }
    //validate adjustment for known relative risks file
    if (geProbabiltyModelType == POISSON) {
      if (gbUseAdjustmentsForRRFile && gsAdjustmentsByRelativeRisksFileName.empty()) {
        bValid = false;
        PrintDirection.SatScanPrintWarning("Error: The settings indicate to the use the adjustments file, but a file name not specified.\n");
      }
      else if (gbUseAdjustmentsForRRFile && access(gsAdjustmentsByRelativeRisksFileName.c_str(), 00)) {
        bValid = false;
        PrintDirection.SatScanPrintWarning("Error: The adjustments file '%s' does not exist.\n", gsAdjustmentsByRelativeRisksFileName.c_str());
        PrintDirection.SatScanPrintWarning("       Please check to make sure the path is correct.\n");
      }
    }
    else
      gbUseAdjustmentsForRRFile = false;

    //validate maximum circle population file for a prospective space-time analysis w/ maximum geographical cluster size
    //defined as a percentage of the population and adjusting for earlier analyses.
    if (geAnalysisType == PROSPECTIVESPACETIME && gbAdjustForEarlierAnalyses && geMaxGeographicClusterSizeType == PERCENTOFPOPULATIONTYPE) {
      bValid = false;
      PrintDirection.SatScanPrintWarning("Error: For a prospective space-time analysis adjusting for ealier analyses, the maximum spatial\n");
      PrintDirection.SatScanPrintWarning("       cluster size must be defined as a percentage of the population as defined in a max\n");
      PrintDirection.SatScanPrintWarning("       circle size file.\n");
      PrintDirection.SatScanPrintWarning("       Alternatively you may choose to specify the maximum as a fixed radius, in which case a\n");
      PrintDirection.SatScanPrintWarning("       max circle size file is not required.\n");
    }
    if (geMaxGeographicClusterSizeType == PERCENTOFPOPULATIONFILETYPE) {
      if (gsMaxCirclePopulationFileName.empty()) {
        bValid = false;
        PrintDirection.SatScanPrintWarning("Error: For a prospective space-time analysis adjusting for ealier analyses, the maximum spatial\n");
        PrintDirection.SatScanPrintWarning("       cluster size must be defined as a percentage of the population as defined in a max\n");
        PrintDirection.SatScanPrintWarning("       circle size file.\n");
        PrintDirection.SatScanPrintWarning("       Alternatively you may choose to specify the maximum as a fixed radius, in which case a\n");
        PrintDirection.SatScanPrintWarning("       max circle size file is not required.\n");
      }
      else if (access(gsMaxCirclePopulationFileName.c_str(), 00)) {
        bValid = false;
        PrintDirection.SatScanPrintWarning("Error: The max circle size file '%s' does not exist.\n", gsMaxCirclePopulationFileName.c_str());
        PrintDirection.SatScanPrintWarning("       Please check to make sure the path is correct.\n");
      }
    }
    //validate output file
    if (gsOutputFileName.empty()) {
      bValid = false;
      PrintDirection.SatScanPrintWarning("Error: No results file specified.\n");
    }
    else if (access(ZdFileName(gsOutputFileName.c_str()).GetLocation(), 00)) {
      bValid = false;
      PrintDirection.SatScanPrintWarning("Error: Results file '%s' have an invalid path.\n", gsOutputFileName.c_str());
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateFileParameters()","CParameters");
    throw;
  }
  return bValid;
}

/** Validates the maximum temporal cluster size parameters. */
bool CParameters::ValidateMaximumTemporalClusterSize(BasePrint& PrintDirection) const {
  ZdString      sPrecisionString;
  double        dStudyPeriodLengthInUnits, dMaxTemporalLengthInUnits;

  try {
    //Maximum temporal cluster size is parameters not used for these analyses.
    if (geAnalysisType == PURELYSPATIAL || geAnalysisType == SPATIALVARTEMPTREND)
      return true;

    if (geMaxTemporalClusterSizeType == PERCENTAGETYPE) {
      //validate for maximum specified as percentage of study period
      if (gfMaxTemporalClusterSize <= 0) {
        PrintDirection.SatScanPrintWarning("Error: The maximum temporal cluster size of '%g' is invalid.\n"
                                           "       Specifying the maximum as a percentage of the study period\n"
                                           "       requires the value to be a decimal number that is greater than zero.\n",
                                           gfMaxTemporalClusterSize);
        return false;
      }
      //check maximum temporal cluster size(as percentage of population) is less than maximum for given probabilty model
      if (gfMaxTemporalClusterSize > (geProbabiltyModelType == SPACETIMEPERMUTATION ? 50 : 90)) {
        PrintDirection.SatScanPrintWarning("Error: For the %s model, the maximum temporal cluster size as a percent\n"
                                           "       of the study period is %d percent.\n",
                                           GetProbabiltyModelTypeAsString(geProbabiltyModelType),
                                           (geProbabiltyModelType == SPACETIMEPERMUTATION ? 50 : 90));
        return false;
      }
      //validate that the time aggregation length agrees with the study period and maximum temporal cluster size
      dStudyPeriodLengthInUnits = ceil(CalculateNumberOfTimeIntervals(CharToJulian(gsStudyPeriodStartDate.c_str()),
                                                                      CharToJulian(gsStudyPeriodEndDate.c_str()),
                                                                      geTimeAggregationUnitsType, 1));
      dMaxTemporalLengthInUnits = floor(dStudyPeriodLengthInUnits * gfMaxTemporalClusterSize/100.0);
      if (dMaxTemporalLengthInUnits < 1) {
        GetDatePrecisionAsString(geTimeAggregationUnitsType, sPrecisionString, false, false);
        PrintDirection.SatScanPrintWarning("Error: A maximum temporal cluster size as %g percent of a %d %s study period\n"
                                           "       results in a maximum temporal cluster size that is less than one time\n"
                                           "       aggregation %s.\n",
                                           gfMaxTemporalClusterSize,
                                           static_cast<int>(dStudyPeriodLengthInUnits),
                                           sPrecisionString.GetCString(), sPrecisionString.GetCString());
        return false;
      }
    }
    else if (geMaxTemporalClusterSizeType == TIMETYPE) {
      //validate for maximum specified as time aggregation unit 
      if (gfMaxTemporalClusterSize < 1) {
        PrintDirection.SatScanPrintWarning("Error: The maximum temporal cluster size of '%2g' is invalid.\n"
                                           "       Specifying the maximum in time aggregation units requires\n"
                                           "       the value to be a whole number that is greater than zero.\n",
                                           gfMaxTemporalClusterSize);
        return false;
      }
      GetDatePrecisionAsString(geTimeAggregationUnitsType, sPrecisionString, false, false);
      dStudyPeriodLengthInUnits = ceil(CalculateNumberOfTimeIntervals(CharToJulian(gsStudyPeriodStartDate.c_str()),
                                                                      CharToJulian(gsStudyPeriodEndDate.c_str()),
                                                                      geTimeAggregationUnitsType, 1));
      dMaxTemporalLengthInUnits = floor(dStudyPeriodLengthInUnits * (geProbabiltyModelType == SPACETIMEPERMUTATION ? 50 : 90)/100.0);
      if (gfMaxTemporalClusterSize > dMaxTemporalLengthInUnits) {
        PrintDirection.SatScanPrintWarning("Error: A maximum temporal cluster size of %d %s%s exceeds\n"
                                           "       %d percent of a %d %s study period.\n"
                                           "       Note that current settings limit the maximum to %d %s%s.\n",
                                           static_cast<int>(gfMaxTemporalClusterSize), sPrecisionString.GetCString(),
                                           (gfMaxTemporalClusterSize == 1 ? "" : "s"),
                                           (geProbabiltyModelType == SPACETIMEPERMUTATION ? 50 : 90),
                                           static_cast<int>(dStudyPeriodLengthInUnits), sPrecisionString.GetCString(),
                                           static_cast<int>(dMaxTemporalLengthInUnits), sPrecisionString.GetCString(),
                                           (dMaxTemporalLengthInUnits == 1 ? "" : "s"));
        return false;
      }
    }
    else
      ZdException::GenerateNotification("Unknown temporal percentage type: %d.",
                                        "ValidateMaximumTemporalClusterSize()", geMaxTemporalClusterSizeType);
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateMaximumTemporalClusterSize()","CParameters");
    throw;
  }
  return true;
}

/** Validates that given current state of settings, parameters and their relationships
    with other parameters are correct. Errors are sent to print direction and*/
bool CParameters::ValidateParameters(BasePrint & PrintDirection) {
  bool         bValid=true;

  try {
    if (gbValidatePriorToCalc) {
      //prevent access to Spatial Variation and Temporal Trends analysis -- still in development
      if (geAnalysisType == SPATIALVARTEMPTREND) {
        bValid = false;
        PrintDirection.SatScanPrintWarning("Error: Please note that spatial variation in temporal trends analysis is not implemented\n");
        PrintDirection.SatScanPrintWarning("       in this version of SaTScan.\n");
      }
      if (geAnalysisType == PURELYSPATIAL && geRiskFunctionType == MONOTONERISK && GetNumDataStreams() > 1) {
        bValid = false;
        PrintDirection.SatScanPrintWarning("Error: Multiple data streams are not permitted with isotonic purely spatial analyses.\n");
      }
      if (geProbabiltyModelType == NORMAL && GetNumDataStreams() > 1) {
        bValid = false;
        PrintDirection.SatScanPrintWarning("Error: Multiple data streams are not permitted with the normal probablility model\n");
        PrintDirection.SatScanPrintWarning("       in this version of SaTScan.\n");
      }

      //validate dates
      if (! ValidateDateParameters(PrintDirection))
        bValid = false;
      else {
        //Validate temporal options only if date parameters are valid. Some
        //temporal parameters can not be correctly validated if dates are not valid.
        if (! ValidateTemporalParameters(PrintDirection))
           bValid = false;
      }

      //validate spatial options
      if (! ValidateSpatialParameters(PrintDirection))
        bValid = false;

      //validate number of replications requested
      if (!(giReplications == 0 || giReplications == 9 || giReplications == 19 || fmod(giReplications+1, 1000) == 0.0)) {
        bValid = false;
        PrintDirection.SatScanPrintWarning("Error: Invalid number of replications '%u'. The value must be 0, 9, 999, or n999.\n", giReplications);
      }

      //validate input/oupt files
      if (! ValidateFileParameters(PrintDirection))
        bValid = false;

      //validate model parameters
      if (geProbabiltyModelType == SPACETIMEPERMUTATION) {
        if (!(geAnalysisType == SPACETIME || geAnalysisType == PROSPECTIVESPACETIME)) {
          bValid = false;
          PrintDirection.SatScanPrintWarning("Error: For the %s model, the analysis type must be either %s or %s.\n",
                                             GetProbabiltyModelTypeAsString(geProbabiltyModelType), RETROSPECTIVE_SPACETIME_ANALYSIS, PROSPECTIVE_SPACETIME_ANALYSIS);
        }
        if (gbOutputRelativeRisksAscii || gbOutputRelativeRisksDBase) {
          bValid = false;
          PrintDirection.SatScanPrintWarning("Error: The relative risks output files can not be produced for the %s model.\n", GetProbabiltyModelTypeAsString(geProbabiltyModelType));
        }
      }
      //validate range parameters
      if (! ValidateRangeParameters(PrintDirection))
        bValid = false;

      //validate sequential scan parameters
      if (! ValidateSequentialScanParameters(PrintDirection))
        bValid = false;

      //validate power calculation parameters
      if (! ValidatePowerCalculationParameters(PrintDirection))
        bValid = false;

      //validate ellipse parameters
      if (! ValidateEllipseParameters(PrintDirection))
        bValid = false;

      //validate simulation options
      if (! ValidateSimulationDataParameters(PrintDirection))
        bValid = false;
    }
    else {
      PrintDirection.SatScanPrintWarning("Warning: Parameters will not be validated, in accordance with the setting of the validation\n"
                                         "         parameter in the parameters file.\n");
      PrintDirection.SatScanPrintWarning("         This may have adverse effects on analysis results and/or program operation.\n\n");
    }

  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateParameters()","CParameters");
    throw;
  }
  return bValid;
}

/** Validates power calculation parameters.
   Prints errors to print direction and returns validity. */
bool CParameters::ValidatePowerCalculationParameters(BasePrint & PrintDirection) const {
  bool  bValid=true;

  try {
    if (gbPowerCalculation) {
      if (0.0 > gdPower_X || gdPower_X > DBL_MAX) {
        bValid = false;
        PrintDirection.SatScanPrintWarning("Error: Invalid parameter setting for teh power calculation value X.\n");
        PrintDirection.SatScanPrintWarning("       Please use a value between 0 and %12.4f\n", DBL_MAX);
      }
      if (0.0 > gdPower_Y || gdPower_Y > DBL_MAX) {
        bValid = false;
        PrintDirection.SatScanPrintWarning("Error: Invalid parameter setting for the power calculation value Y.\n");
        PrintDirection.SatScanPrintWarning("       Please use a value between 0 and %12.4f\n", DBL_MAX);
      }
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidatePowerCalculationParameters()","CParameters");
    throw;
  }
  return bValid;
}

/** Validates prospective surveillance start date by:
    - if not adjusting for earlier analyses, ensures that prospective start
      date equals the study period end date.
    - calls method ValidateEndDate(...)                                       */
bool CParameters::ValidateProspectiveDate(BasePrint& PrintDirection) const {
  UInt          uiYear, uiMonth, uiDay;
  bool          bReturnValue=true;
  ZdString      sDate;

  try {
    if (!gbAdjustForEarlierAnalyses && gsProspectiveStartDate != gsStudyPeriodEndDate) {
      //when not adjusting for earlier analyses,
      PrintDirection.SatScanPrintWarning("Error: A prospective analysis that does not adjust for earlier\n"
                                         "       analyses must have the start of prospective surveillance\n"
                                         "       set to the same date as the study period end date.\n");
      bReturnValue = false;
    }
    //validate study period end date based upon precision of times parameter setting
    //parse date in parts
    if (CharToMDY(&uiMonth, &uiDay, &uiYear, gsProspectiveStartDate.c_str()) != 3) {
      PrintDirection.SatScanPrintWarning("Error: The specified prospective surveillance start date, '%s', is not valid.\n"
                                         "       Please specify as YYYY/MM/DD.\n", gsProspectiveStartDate.c_str());
      return false;
    }
    //validate date
    if (!IsDateValid(uiMonth, uiDay, uiYear)) {
      PrintDirection.SatScanPrintWarning("Error: The specified prospective surveillance start date, %s, is not a valid date.\n",
                                         gsProspectiveStartDate.c_str());
      bReturnValue = false;
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateProspectiveDate()","CParameters");
    throw;
  }
  return bReturnValue;
}

/** Validates parameters used in optional start and end ranges for time windows.
    Prints errors to print direction and returns whether values are vaild. */
bool CParameters::ValidateRangeParameters(BasePrint& PrintDirection) const {
  bool          bValid=true;
  UInt          uiYear, uiMonth, uiDay;
  Julian        StudyPeriodStartDate, StudyPeriodEndDate,
                StartRangeStartDate, StartRangeEndDate,
                EndRangeStartDate, EndRangeEndDate;

  try {
    if (geIncludeClustersType == CLUSTERSINRANGE && (geAnalysisType == PURELYTEMPORAL || geAnalysisType == SPACETIME)) {
      //validate start range start date
      if (CharToMDY(&uiMonth, &uiDay, &uiYear, gsStartRangeStartDate.c_str()) != 3) {
        PrintDirection.SatScanPrintWarning("Error: The start date of start range in flexible temporal window definition,\n"
                                           "       '%s', is not valid. Please specify as YYYY/MM/DD.\n", gsStartRangeStartDate.c_str());
        bValid = false;
      }
      if (!IsDateValid(uiMonth, uiDay, uiYear)) {
        PrintDirection.SatScanPrintWarning("Error: The start date of start range in flexible temporal window definition,\n"
                                           "       %s, is not a valid date.\n", gsStartRangeStartDate.c_str());
        bValid = false;
      }
      //validate start range end date
      if (CharToMDY(&uiMonth, &uiDay, &uiYear, gsStartRangeEndDate.c_str()) != 3) {
        PrintDirection.SatScanPrintWarning("Error: The end date of start range in flexible temporal window definition,\n"
                                           "       '%s', is not valid. Please specify as YYYY/MM/DD.\n", gsStartRangeEndDate.c_str());
        bValid = false;
      }
      if (!IsDateValid(uiMonth, uiDay, uiYear)) {
        PrintDirection.SatScanPrintWarning("Error: The end date of start range in flexible temporal window definition,\n"
                                           "       %s, is not a valid date.\n", gsStartRangeEndDate.c_str());
        bValid = false;
      }
      //validate end range start date
      if (CharToMDY(&uiMonth, &uiDay, &uiYear, gsEndRangeStartDate.c_str()) != 3) {
        PrintDirection.SatScanPrintWarning("Error: The start date of end range in flexible temporal window definition,\n"
                                           "       '%s', is not valid. Please specify as YYYY/MM/DD.\n", gsEndRangeStartDate.c_str());
        bValid = false;
      }
      if (!IsDateValid(uiMonth, uiDay, uiYear)) {
        PrintDirection.SatScanPrintWarning("Error: The start date of end range in flexible temporal window definition,\n"
                                           "       %s, is not a valid date.\n", gsEndRangeStartDate.c_str());
        bValid = false;
      }
      //validate end range end date
      if (CharToMDY(&uiMonth, &uiDay, &uiYear, gsEndRangeStartDate.c_str()) != 3) {
        PrintDirection.SatScanPrintWarning("Error: The end date of end range in flexible temporal window definition,\n"
                                           "       '%s', is not valid. Please specify as YYYY/MM/DD.\n", gsEndRangeEndDate.c_str());
        bValid = false;
      }
      if (!IsDateValid(uiMonth, uiDay, uiYear)) {
        PrintDirection.SatScanPrintWarning("Error: The end date of end range in flexible temporal window definition,\n"
                                           "       %s, is not a valid date.\n", gsEndRangeEndDate.c_str());
        bValid = false;
      }
      //now valid that range dates are within study period start and end dates
      if (bValid) {
        StudyPeriodStartDate = CharToJulian(gsStudyPeriodStartDate.c_str());
        StudyPeriodEndDate = CharToJulian(gsStudyPeriodEndDate.c_str());

        EndRangeStartDate = CharToJulian(gsEndRangeStartDate.c_str());
        EndRangeEndDate = CharToJulian(gsEndRangeEndDate.c_str());
        if (EndRangeStartDate > EndRangeEndDate) {
          bValid = false;
          PrintDirection.SatScanPrintWarning("Error: Invalid scanning window end range.\n");
          PrintDirection.SatScanPrintWarning("       Range date '%s' occurs after date '%s'.\n",
                                             gsEndRangeStartDate.c_str(), gsEndRangeEndDate.c_str());
        }
        else {
          if (EndRangeStartDate < StudyPeriodStartDate || EndRangeStartDate > StudyPeriodEndDate) {
            bValid = false;
            PrintDirection.SatScanPrintWarning("Error: The scanning window end range date '%s',\n",  gsEndRangeStartDate.c_str());
            PrintDirection.SatScanPrintWarning("       is not within the study period (%s - %s).\n", gsStudyPeriodStartDate.c_str(), gsStudyPeriodEndDate.c_str());
          }
          if (EndRangeEndDate < StudyPeriodStartDate || EndRangeEndDate > StudyPeriodEndDate) {
            bValid = false;
            PrintDirection.SatScanPrintWarning("Error: The scanning window end range date '%s',\n",  gsEndRangeEndDate.c_str());
            PrintDirection.SatScanPrintWarning("       is not within the study period (%s - %s) \n", gsStudyPeriodStartDate.c_str(), gsStudyPeriodEndDate.c_str());
          }
        }

        StartRangeStartDate = CharToJulian(gsStartRangeStartDate.c_str());
        StartRangeEndDate = CharToJulian(gsStartRangeEndDate.c_str());
        if (StartRangeStartDate > StartRangeEndDate) {
          bValid = false;
          PrintDirection.SatScanPrintWarning("Error: Invalid scanning window start range.\n");
          PrintDirection.SatScanPrintWarning("       The range date '%s' occurs after date '%s'.\n",
                                             gsStartRangeStartDate.c_str(), gsStartRangeEndDate.c_str());
        }
        else {
          if (StartRangeStartDate < StudyPeriodStartDate || StartRangeStartDate > StudyPeriodEndDate) {
            bValid = false;
            PrintDirection.SatScanPrintWarning("Error: The scanning window start range date '%s',\n",  gsStartRangeStartDate.c_str());
            PrintDirection.SatScanPrintWarning("       is not within the study period (%s - %s).\n", gsStudyPeriodStartDate.c_str(), gsStudyPeriodEndDate.c_str());
          }
          if (StartRangeEndDate < StudyPeriodStartDate || StartRangeEndDate > StudyPeriodEndDate) {
            bValid = false;
            PrintDirection.SatScanPrintWarning("Error: The scanning window start range date '%s',\n",  gsStartRangeEndDate.c_str());
            PrintDirection.SatScanPrintWarning("       is not within the study period (%s - %s) \n", gsStudyPeriodStartDate.c_str(), gsStudyPeriodEndDate.c_str());
          }
        }
        if (StartRangeStartDate >= EndRangeEndDate) {
          bValid = false;
          PrintDirection.SatScanPrintWarning("Error: The scanning window start range does not occur before the end range.\n");
        }
      }
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateRangeParameters()","CParameters");
    throw;
  }
  return bValid;
}

/** Validates parameters used in making simulation data. */
bool CParameters::ValidateSimulationDataParameters(BasePrint & PrintDirection) {
  bool  bValid=true;

  try {
    if (geProbabiltyModelType == POISSON) {
      switch (geSimulationType) {
        case STANDARD           : break;
        case HA_RANDOMIZATION   : if (gsAdjustmentsByRelativeRisksFileName.empty()) {
                                    bValid = false;
                                    PrintDirection.SatScanPrintWarning("Error: No adjustments file specified.\n");
                                  }
                                  else if (access(gsAdjustmentsByRelativeRisksFileName.c_str(), 00)) {
                                    bValid = false;
                                    PrintDirection.SatScanPrintWarning("Error: The adjustments file '%s' does not exist.\n",
                                                                       gsAdjustmentsByRelativeRisksFileName.c_str());
                                    PrintDirection.SatScanPrintWarning("       Please check to make sure the path is correct.\n");
                                  }
                                  break;
        case FILESOURCE         : if (!(geAnalysisType == PROSPECTIVESPACETIME || geAnalysisType == SPACETIME || geAnalysisType == PURELYTEMPORAL ||
                                        geAnalysisType == PROSPECTIVEPURELYTEMPORAL || geAnalysisType == PURELYSPATIAL)) {
                                     bValid = false;
                                     PrintDirection.SatScanPrintWarning("Error: The feature to read simulated data from a file is not implemented for a %s analyses.\n",
                                                                        GetAnalysisTypeAsString());
                                     break;
                                  }
                                  if (GetNumDataStreams() > 1){
                                    bValid = false;
                                    PrintDirection.SatScanPrintWarning("Error: The feature to read simulated data from a file is not implemented for analyses\n"
                                                                       "       that read data from multiple data streams.\n");
                                  }
                                  if (gsSimulationDataSourceFileName.empty()) {
                                    bValid = false;
                                    PrintDirection.SatScanPrintWarning("Error: The simulated data input file was not specified.\n");
                                  }
                                  else if (access(gsSimulationDataSourceFileName.c_str(), 00)) {
                                    bValid = false;
                                    PrintDirection.SatScanPrintWarning("Error: The simulated data input file '%s' does not exist.\n",
                                                                       gsSimulationDataSourceFileName.c_str());
                                    PrintDirection.SatScanPrintWarning("       Please check to make sure the path is correct.\n");
                                  }
                                  if (gbOutputSimulationData && gsSimulationDataSourceFileName == gsSimulationDataOutputFilename) {
                                    bValid = false;
                                    PrintDirection.SatScanPrintWarning("Error: : The file '%s' is specified as both\n",
                                                                       gsSimulationDataSourceFileName.c_str());
                                    PrintDirection.SatScanPrintWarning("       the input and the output file for simulated data.\n");                                  }
                                  break;
        default : ZdGenerateException("Unknown simulation type '%d'.","ValidateSimulationDataParameters()", geSimulationType);
      };
      if (giReplications == 0)
        gbOutputSimulationData = false;
      if (!(geAnalysisType == PROSPECTIVESPACETIME || geAnalysisType == SPACETIME || geAnalysisType == PURELYTEMPORAL ||
            geAnalysisType == PROSPECTIVEPURELYTEMPORAL || geAnalysisType == PURELYSPATIAL) && gbOutputSimulationData) {
         bValid = false;
         PrintDirection.SatScanPrintWarning("Error: Printing simulation data to file not implemented for %s analysis.\n",
                                            GetAnalysisTypeAsString());
      }
      else if (gbOutputSimulationData && gsSimulationDataOutputFilename.empty()) {
        bValid = false;
        PrintDirection.SatScanPrintWarning("Error: Simulation data output file not specified.\n");
      }
    }
    else {
      //only standard simulation type permitted for other model types, report errors accordingly
      switch (geSimulationType) {
        case STANDARD         : break;
        case HA_RANDOMIZATION : bValid = false;
                                PrintDirection.SatScanPrintWarning("Error: The alternative hypothesis method of creating simulated data\n");
                                PrintDirection.SatScanPrintWarning("       is only implemented for the Poisson model.\n");
                                break;
        case FILESOURCE       : bValid = false;
                                PrintDirection.SatScanPrintWarning("Error: Reading simulate data from a file is only\n");
                                PrintDirection.SatScanPrintWarning("       implemented for the Poisson model.\n");
                                break;
      }
      //The code for printing simulation data was modified by none programmer and it's
      //correctness has not been validated, but writing routine(and rountine for reading)
      //appears be ok for all probability model types. -- check this later
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateSimulationDataParameters()","CParameters");
    throw;
  }
  return bValid;
}

/** Validates parameters used in optional sequenatial scan feature.
    Prints errors to print direction and returns whether values are vaild.*/
bool CParameters::ValidateSequentialScanParameters(BasePrint & PrintDirection) {
  bool  bValid=true;

  try {
    if (gbSequentialRuns && giNumSequentialRuns > 0) {
      //if (geAnalysisType == PURELYTEMPORAL || geAnalysisType == PROSPECTIVEPURELYTEMPORAL) {
      if (geAnalysisType != PURELYSPATIAL) {
        //code only implemented for purley spatial analyses
        giNumSequentialRuns = 0;
        PrintDirection.SatScanPrintWarning("Error: The sequential scan feature is only implememted for purely spatial analyses only.\n");
        return false;
      }
      if (giNumSequentialRuns > MAXIMUM_SEQUENTIAL_ANALYSES) {
        bValid = false;
        PrintDirection.SatScanPrintWarning("Error: %d exceeds the maximum number of sequential analyses allowed (%d).\n",
                                           giNumSequentialRuns, MAXIMUM_SEQUENTIAL_ANALYSES);
      }
      if (gbSequentialCutOffPValue < 0 || gbSequentialCutOffPValue > 1) {
        bValid = false;
        PrintDirection.SatScanPrintWarning("Error: : The sequential scan analysis cutoff p-value of '%2g' is not a decimal value between 0 and 1.\n",
                                           gbSequentialCutOffPValue);
      }
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateSequentialScanParameters()","CParameters");
    throw;
  }
  return bValid;
}

/** Validates optional parameters particular to spatial analyses
    (i.e. purely spatial, retrospective space-time and prospective space-time).
    Prints errors to print direction and returns whether values are vaild. */
bool CParameters::ValidateSpatialParameters(BasePrint & PrintDirection) {
  bool  bValid=true;

  try {
    //validate spatial options
    if (geAnalysisType == PURELYSPATIAL || geAnalysisType == SPACETIME ||
        geAnalysisType == PROSPECTIVESPACETIME || geAnalysisType == SPATIALVARTEMPTREND) {
      if (gfMaxGeographicClusterSize <= 0) {
        bValid = false;
        PrintDirection.SatScanPrintWarning("Error: The maximum spatial cluster size of '%2g%%' is invalid. The value must be greater than zero.\n", gfMaxGeographicClusterSize);
      }
      if (GetMaxGeoClusterSizeTypeIsPopulationBased()  && gfMaxGeographicClusterSize > 50.0) {
        bValid = false;
        PrintDirection.SatScanPrintWarning("Error: Invalid parameter setting of '%2g%%' for the maximum spatial cluster size.\n", gfMaxGeographicClusterSize);
        PrintDirection.SatScanPrintWarning("       When defined as a percentage of the population at risk, the maximum spatial cluster size is 50%%.\n");
      }
      if (gbRestrictReportedClusters && gfMaxReportedGeographicClusterSize <= 0) {
        bValid = false;
        PrintDirection.SatScanPrintWarning("Error: The maximum spatial cluster size of '%2g%%' for reported clusters is invalid. It must be greater than zero.\n", gfMaxGeographicClusterSize);
      }
      if (gbRestrictReportedClusters && gfMaxReportedGeographicClusterSize > gfMaxGeographicClusterSize) {
        bValid = false;
        PrintDirection.SatScanPrintWarning("Error: Invalid parameter setting of '%2g' for maximum reported spatial cluster size.\n", gfMaxReportedGeographicClusterSize);
        PrintDirection.SatScanPrintWarning("       The settings can not be greater than the maximum spatial cluster size.\n");
      }
      if (gbRestrictReportedClusters && gfMaxReportedGeographicClusterSize == gfMaxGeographicClusterSize)
        gbRestrictReportedClusters = false;
    }
    else {
      //Purely temporal clusters should default maximum geographical clusters size to 50 of population.
      //This actually has no bearing on analysis results. These variables are used primarly for
      //finding neighbors which purely temporal analyses don't utilize. The finding neighbors
      //routine should really be skipped for this analysis type.
      gfMaxGeographicClusterSize = 50.0; //KR980707 0 GG980716;
      geMaxGeographicClusterSizeType = PERCENTOFPOPULATIONTYPE;
      gbRestrictReportedClusters = false;
    }

    if (gbIncludePurelySpatialClusters) {
      if (!GetPermitsPurelySpatialCluster(geProbabiltyModelType)) {
          bValid = false;
          PrintDirection.SatScanPrintWarning("Error: A purely spatial cluster cannot be included for a %s model.\n",
                                             GetProbabiltyModelTypeAsString(geProbabiltyModelType));
      }
      else if (!GetPermitsPurelySpatialCluster(geAnalysisType)) {
        bValid = false;
        PrintDirection.SatScanPrintWarning("Error: A purely spatial cluster can only be included for spatial based analyses.\n");
      }
    }
    if (geSpatialAdjustmentType == SPATIALLY_STRATIFIED_RANDOMIZATION) {
      if (!(geAnalysisType == SPACETIME || geAnalysisType == PROSPECTIVESPACETIME)) {
        bValid = false;
        PrintDirection.SatScanPrintWarning("Error: Spatial adjustment by stratified randomization is valid for\n"
                                           "       either retrospective or prospective space-time analyses only.\n");
      }
      if (geTimeTrendAdjustType == STRATIFIED_RANDOMIZATION) {
        bValid = false;
        PrintDirection.SatScanPrintWarning("Error: Spatial adjustment by stratified randomization can not be performed\n"
                                           "       in conjunction with the temporal adjustment by stratified randomization.\n");
      }
      if (gbIncludePurelySpatialClusters) {
        bValid = false;
        PrintDirection.SatScanPrintWarning("Error: Spatial adjustment by stratified randomization does not permit\n"
                                           "       the inclusion of a purely spatial cluster.\n");
      }
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateSpatialParameters()","CParameters");
    throw;
  }
  return bValid;
}

/** Validates start date for the following attributes:
    - date contains year, month and day.
    - date is a real date.
    - checks that date agrees with precision of times settings
      ;precision year - month = 12 and day = 31
      ;precision month - day = last day of month
   Returns boolean indication of whether date is valid, printing relevant
   messages to BasePrint object.                                              */
bool CParameters::ValidateStudyPeriodStartDate(BasePrint& PrintDirection) const {
  UInt                  nYear, nMonth, nDay;
  DatePrecisionType     ePrecision = (gCreationVersion.iMajor == 4 ? geTimeAggregationUnitsType : gePrecisionOfTimesType);

  try {
    if (gCreationVersion.iMajor == 4 && gePrecisionOfTimesType != NONE)
      ePrecision = geTimeAggregationUnitsType;
    else
      ePrecision = gePrecisionOfTimesType;

    //parse date in parts
    if (CharToMDY(&nMonth, &nDay, &nYear, gsStudyPeriodStartDate.c_str()) != 3) {
      PrintDirection.SatScanPrintWarning("Error: The study period start date, '%s', is not valid.\n"
                                         "       Please specify as YYYY/MM/DD.\n", gsStudyPeriodStartDate.c_str());
      return false;
    }
    //validate date
    if (!IsDateValid(nMonth, nDay, nYear)) {
      PrintDirection.SatScanPrintWarning("Error: The study period start date, '%s', is not valid date.\n", gsStudyPeriodStartDate.c_str());
      return false;
    }
    //validate against precision of times
    switch (ePrecision) {
      case YEAR  :
        if (nMonth != 1 || nDay != 1) {
          PrintDirection.SatScanPrintWarning("Error: The study period start date, '%s', is not valid.\n"
                                             "       With the setting for %s as years, the date\n"
                                             "       must be the first day of respective year.\n",
                                             gsStudyPeriodStartDate.c_str(),
                                             (gCreationVersion.iMajor == 4 ? "time aggregation" : "time precision"));
          return false;
        }
        break;
      case MONTH :
        if (nDay != 1) {
          PrintDirection.SatScanPrintWarning("Error: The study period start date, '%s', is not valid.\n"
                                             "       With the setting for %s as months, the date\n"
                                             "       must be the first day of respective month.\n",
                                             gsStudyPeriodStartDate.c_str(),
                                             (gCreationVersion.iMajor == 4 ? "time aggregation" : "time precision"));
          return false;
        }
      case DAY   :
      case NONE  : break;
    };
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateStartDate()", "CParameters");
    throw;
  }
  return true;
}

/** Validates optional parameters particular to temporal analyses
    (i.e. purely temporal, retrospective space-time and prospective space-time).
    Prints errors to print direction and returns whether values are vaild.*/
bool CParameters::ValidateTemporalParameters(BasePrint & PrintDirection) {
  bool          bValid=true;

  try {
    //validate temporal options only for analyses that are temporal
    if (geAnalysisType == PURELYSPATIAL) {
      //default these options - Not sure why orignal programmers did this. When
      //there is more time, we want to examine code so that we don't need to.
      //Instead, code related to these variables just shouldn't be executed.
      gfMaxTemporalClusterSize           = 50.0; 
      geMaxTemporalClusterSizeType       = PERCENTAGETYPE;
      geIncludeClustersType              = ALLCLUSTERS;
      geTimeAggregationUnitsType         = NONE;
      glTimeAggregationLength            = 0;
      geTimeTrendAdjustType              = NOTADJUSTED;
      gdTimeTrendAdjustPercentage        = 0;
      return true;
    }
    //validate maximum temporal cluster size
    if (!ValidateMaximumTemporalClusterSize(PrintDirection))
      bValid = false;
    //Prospective analyses include only alive clusters - reset this parameter
    //instead of reporting error since this parameter has under gone changes
    //that would make any error message confusing to user. Prospective analyses
    //should have reported this error when they were introduced to SaTScan, but
    //I think it's too late to start enforcing this.
    if (GetIsProspectiveAnalysis() && geIncludeClustersType != ALIVECLUSTERS)
      geIncludeClustersType = ALIVECLUSTERS;
    //validate time aggregation units  
    if (!ValidateTimeAggregationUnits(PrintDirection))
      bValid = false;
    //validate time trend adjustment
    switch (geProbabiltyModelType) {
      case BERNOULLI            :
        //The SVTT analysis has hooks for temporal adjustments, but that code needs
        //much closer examination before it can be used, even experimentally.
        if (geTimeTrendAdjustType != NOTADJUSTED) {
          PrintDirection.SatScanPrintWarning("Warning: For the Bernoulli model, adjusting for temporal trends is not permitted.\n");
                                             geTimeTrendAdjustType = NOTADJUSTED;
                                             gdTimeTrendAdjustPercentage = 0.0;
        }
        break;
      case NORMAL               :
      case SURVIVAL             :
      case RANK                 :
        if (geTimeTrendAdjustType != NOTADJUSTED) {
          PrintDirection.SatScanPrintWarning("Warning: For the %s model, adjusting for temporal trends is not permitted.\n",
                                             GetProbabiltyModelTypeAsString(geProbabiltyModelType));
                                             geTimeTrendAdjustType = NOTADJUSTED;
                                             gdTimeTrendAdjustPercentage = 0.0;
        }
        break;
      case SPACETIMEPERMUTATION :
        if (geTimeTrendAdjustType != NOTADJUSTED) {
          PrintDirection.SatScanPrintWarning("Warning: For the space-time permutation model, adjusting for temporal trends\n"
                                             "         is not permitted nor needed, as this model automatically adjusts for\n"
                                             "         any temporal variation.\n");
                                             geTimeTrendAdjustType = NOTADJUSTED;
                                             gdTimeTrendAdjustPercentage = 0.0;
        }
        break;
      case POISSON             :
        if (geTimeTrendAdjustType == NONPARAMETRIC && (geAnalysisType == PURELYTEMPORAL ||geAnalysisType == PROSPECTIVEPURELYTEMPORAL)) {
          bValid = false;
          PrintDirection.SatScanPrintWarning("Error: Invalid parameter setting for time trend adjustment.\n");
          PrintDirection.SatScanPrintWarning("       You may not use non-parametric time in a purely temporal analysis.\n");
        }
        if (geTimeTrendAdjustType == STRATIFIED_RANDOMIZATION && (geAnalysisType == PURELYTEMPORAL ||geAnalysisType == PROSPECTIVEPURELYTEMPORAL)) {
          bValid = false;
          PrintDirection.SatScanPrintWarning("Error: Temporal adjustment by stratified randomization is not valid\n");
          PrintDirection.SatScanPrintWarning("       for purely temporal analyses.\n");
        }
        if (geTimeTrendAdjustType == LOGLINEAR_PERC && -100.0 >= gdTimeTrendAdjustPercentage) {
          bValid = false;
          PrintDirection.SatScanPrintWarning("Error: The time adjustment percentage is '%2g', but must greater than -100.\n",
                                             gdTimeTrendAdjustPercentage);
        }
        if (geTimeTrendAdjustType == NOTADJUSTED) {
          gdTimeTrendAdjustPercentage = 0;
          if (geAnalysisType != SPATIALVARTEMPTREND)
            gdTimeTrendConverge = 0.0;
        }
        if (geTimeTrendAdjustType == CALCULATED_LOGLINEAR_PERC || geAnalysisType == SPATIALVARTEMPTREND) {
          if (gdTimeTrendConverge < 0.0) {
            bValid = false;
            PrintDirection.SatScanPrintWarning("Error: Time trend convergence value of '%2g' is less than zero.\n", gdTimeTrendConverge);
          }
        }
        break;
      default : ZdException::Generate("Unknown model type '%d'.","ValidateTemporalParameters()", geProbabiltyModelType);
    }
    //validate including purely temporal clusters
    if (gbIncludePurelyTemporalClusters) {
      if (!GetPermitsPurelyTemporalCluster(geProbabiltyModelType)) {
          bValid = false;
          PrintDirection.SatScanPrintWarning("Error: Looking for purely temporal clusters can not be included when the %s model is used.\n",
                                             GetProbabiltyModelTypeAsString(geProbabiltyModelType));
      }
      else if (!GetPermitsPurelyTemporalCluster(geAnalysisType)) {
        bValid = false;
        PrintDirection.SatScanPrintWarning("Error: A purely temporal cluster can only be included for time based analyses.\n");
      }
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateTemporalParameters()","CParameters");
    throw;
  }
  return bValid;
}

/** Validates the time aggregation units. */
bool CParameters::ValidateTimeAggregationUnits(BasePrint& PrintDirection) const {
  bool          bValid=true;
  ZdString      sPrecisionString;
  double        dStudyPeriodLengthInUnits, dMaxTemporalLengthInUnits;

  if (geAnalysisType == PURELYSPATIAL) //validate settings for temporal analyses
    return true;
  //get date precision string for error reporting
  GetDatePrecisionAsString(geTimeAggregationUnitsType, sPrecisionString, false, false);
  if (geTimeAggregationUnitsType == NONE) { //validate time aggregation units
    PrintDirection.SatScanPrintWarning("Error: Time aggregation units can not be 'none' for a temporal analysis.\n");
    return false;
  }
  if (glTimeAggregationLength <= 0) {
    PrintDirection.SatScanPrintWarning("Error: The time aggregation length of '%d' is invalid. Length must be greater than zero.\n"
                                       "       Note that current settings permit a maximum time aggregation of %g %s%s.\n",
                                       glTimeAggregationLength, dMaxTemporalLengthInUnits, sPrecisionString.GetCString(),
                                       (dMaxTemporalLengthInUnits == 1 ? "" : "s"));
    return false;
  }
  //validate that the time aggregation length agrees with the study period and maximum temporal cluster size
  dStudyPeriodLengthInUnits = ceil(CalculateNumberOfTimeIntervals(CharToJulian(gsStudyPeriodStartDate.c_str()),
                                                                  CharToJulian(gsStudyPeriodEndDate.c_str()),
                                                                  geTimeAggregationUnitsType, 1));
  if (dStudyPeriodLengthInUnits < static_cast<double>(glTimeAggregationLength))  {
    PrintDirection.SatScanPrintWarning("Error: A time aggregation of %d %s%s is greater than the %d %s study period.\n",
                                       glTimeAggregationLength, sPrecisionString.GetCString(), (glTimeAggregationLength == 1 ? "" : "s"),
                                       static_cast<int>(dStudyPeriodLengthInUnits), sPrecisionString.GetCString());
    return false;
  }
  if (ceil(dStudyPeriodLengthInUnits/static_cast<double>(glTimeAggregationLength)) <= 1) {
    PrintDirection.SatScanPrintWarning("Error: A time aggregation of %d %s%s with a %d %s study period results in only\n"
                                       "       one time period to analyze. Temporal and space-time analyses can not be performed\n"
                                       "       on less than two time periods.\n",
                                       glTimeAggregationLength, sPrecisionString.GetCString(), (glTimeAggregationLength == 1 ? "" : "s"),
                                       static_cast<int>(dStudyPeriodLengthInUnits), sPrecisionString.GetCString());
    return false;
  }

  if (geAnalysisType == SPATIALVARTEMPTREND) //svtt does not have a maximum temporal cluster size
    return true;
      
  if (geMaxTemporalClusterSizeType == PERCENTAGETYPE)
    dMaxTemporalLengthInUnits = floor(dStudyPeriodLengthInUnits * (double)gfMaxTemporalClusterSize/100.0);
  else if (geMaxTemporalClusterSizeType == TIMETYPE)
    dMaxTemporalLengthInUnits = (double)gfMaxTemporalClusterSize;

  //validate the time aggregation agrees with maximum temporal cluster size
  if (static_cast<int>(floor(dMaxTemporalLengthInUnits /static_cast<double>(glTimeAggregationLength))) == 0) {
    if (geMaxTemporalClusterSizeType == TIMETYPE)
      PrintDirection.SatScanPrintWarning("Error: The time aggregation of %d %s%s is greater than the maximum temporal\n"
                                         "       cluster size of %g %s%s.\n", glTimeAggregationLength,
                                         sPrecisionString.GetCString(), (glTimeAggregationLength == 1 ? "" : "s"),
                                         gfMaxTemporalClusterSize, sPrecisionString.GetCString(),
                                         (gfMaxTemporalClusterSize == 1 ? "" : "s"));
    else if (geMaxTemporalClusterSizeType == PERCENTAGETYPE)
      PrintDirection.SatScanPrintWarning("Error: With the maximum temporal cluster size as %g percent of a %d %s study period,\n"
                                         "       the time aggregation as %d %s%s is greater than the resulting maximum\n"
                                         "       temporal cluster size of %g %s%s.\n",
                                         gfMaxTemporalClusterSize, static_cast<int>(dStudyPeriodLengthInUnits),
                                         sPrecisionString.GetCString(), glTimeAggregationLength,
                                         sPrecisionString.GetCString(), (glTimeAggregationLength == 1 ? "" : "s"),
                                         dMaxTemporalLengthInUnits, sPrecisionString.GetCString(),
                                         (dMaxTemporalLengthInUnits == 1 ? "" : "s"));
    return false;
  }
    
  return true;
}

// saves the parameters to an .ini file and replaces the existing ini if necessary
// pre: sFileName is the name of the .prm parameter file
// post: saves the parameters to an .ini file
void CParameters::Write(const char * sParameterFileName) {
   try {
      ZdIniFile file(sParameterFileName);
      file.Clear();

      SetSourceFileName(sParameterFileName);
      SaveInputFileSection(file);
      SaveAnalysisSection(file);
      SaveTimeParametersSection(file);
      SaveScanningWindowSection(file);
      SaveOutputFileSection(file);
      SaveEllipseSection(file);
      SaveSequentialScanSection(file);
      SaveAdvancedFeaturesSection(file);
      SaveSystemSection(file);
      file.Write();
   }
   catch (ZdException &x) {
      x.AddCallpath("Write()", "CParameters");
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








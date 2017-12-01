//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "ScanLineParameterFileAccess.h"
#include "SSException.h" 
#include <iostream>
#include <fstream>

/** constructor */
ScanLineParameterFileAccess::ScanLineParameterFileAccess(CParameters& Parameters, BasePrint& PrintDirection)
                                :AbtractParameterFileAccess(Parameters, PrintDirection, true) {}

/** destructor */
ScanLineParameterFileAccess::~ScanLineParameterFileAccess() {}

/** Returns a string which describes the parameter (for error reporting on read). */
const char * ScanLineParameterFileAccess::GetParameterLabel(ParameterType eParameterType) const {
  switch (eParameterType) {
    case ANALYSISTYPE                       : return "Analysis Type (line 1)";
    case SCANAREAS                          : return "Scan Areas Type (line 2)";  
    case CASEFILE                           : return "Case Filename (line 3)";  
    case POPFILE                            : return "Population Filename (line 4)";  
    case COORDFILE                          : return "Coordinate Filename (line 5)";  
    case OUTPUTFILE                         : return "Results Filename (line 6)";  
    case PRECISION                          : return "Time Precision of Case File Dates (line 7)";  
    case DIMENSION                          : return "*Not Used* (line 8)";  
    case SPECIALGRID                        : return "Use Grid File? (line 9)";  
    case GRIDFILE                           : return "Grid Filename (line 10)";  
    case GEOSIZE                            : return "Max Spatial Cluster Size (line 11)";  
    case STARTDATE                          : return "Study Period Start Date (line 12)";  
    case ENDDATE                            : return "Study Period End Date (line 13)";  
    case CLUSTERS                           : return "Include Clusters Type (line 14)";  
    case EXACTTIMES                         : return "*Not Used* (line 15)";  
    case TIME_AGGREGATION_UNITS             : return "Time Aggregation Units (line 16)";  
    case TIME_AGGREGATION                   : return "Time Aggregation Length (line 17)";  
    case PURESPATIAL                        : return "Include Purely Spatial Cluster (line 18)";  
    case TIMESIZE                           : return "Max Temporal Cluster Size (line 19)";  
    case REPLICAS                           : return "Num Monte Carlo Replications (line 20)";  
    case MODEL                              : return "Probability Model Type (line 21)";  
    case RISKFUNCTION                       : return "Isotonic Scan (line 22)";  
    case POWER_EVALUATION                   : return " perform power evaluation - Poisson only (line 23)";
    case POWER_05                           : return "Power Evaluation Critical Value #1 (line 24)";  
    case POWER_01                           : return "Power Evaluation Critical Value #2 (line 25)";  
    case TIMETREND                          : return "Temporal Trend Adjustment Type (line 26)";  
    case TIMETRENDPERC                      : return "Temporal Trend Percentage (line 27)";  
    case PURETEMPORAL                       : return "Include Purely Temporal Cluster (line 28)";  
    case CONTROLFILE                        : return "Control Filename (line 29)";  
    case COORDTYPE                          : return "Coordinates Type (line 30)";  
    case OUTPUT_SIM_LLR_ASCII               : return "Output Simulation LLR - ASCII? (line 31)";  
    case ITERATIVE                          : return "Perform Iterative Scan? (line 32)";
    case ITERATIVE_NUM                      : return "Num Iterative Scan Iterations (line 33)";
    case ITERATIVE_PVAL                     : return "Iterative Scan P-Value Cutoff (line 34)";
    case VALIDATE                           : return "Validate Settings (line 35)";  
    case OUTPUT_RR_ASCII                    : return "Output Relative Risks - ASCII? (line 36)";  
    case WINDOW_SHAPE                       : return "Spatial Window Shape (line 37)";  
    case ESHAPES                            : return "Ellipse Shapes (line 38)";  
    case ENUMBERS                           : return "Ellipse Angles (line 39)";  
    case START_PROSP_SURV                   : return "Prospective Surveillance Start Date (line 40)";  
    case OUTPUT_AREAS_ASCII                 : return "Output Location Information - ASCII? (line 41)";  
    case OUTPUT_MLC_ASCII                   : return "Output Cluster Information - ASCII? (line 42)";  
    case CRITERIA_SECOND_CLUSTERS           : return "Criteria Secondary Clusters Type (line 43)";  
    case MAX_TEMPORAL_TYPE                  : return "Max Temporal Cluster Size Type (line 44)";  
    case MAX_SPATIAL_TYPE                   : return "Max Spatial Cluster Size Type (line 45)";
    case RUN_HISTORY_FILENAME               : return "*Not Used* (line 46)";  
    case OUTPUT_MLC_DBASE                   : return "Output Cluster Information - dBase? (line 47)";  
    case OUTPUT_AREAS_DBASE                 : return "Output Location Information - dBase? (line 48)";  
    case OUTPUT_RR_DBASE                    : return "Output Relative Risks - dBase? (line 49)";  
    case OUTPUT_SIM_LLR_DBASE               : return "Output Simulation LLR - dBase? (line 50)";  
    case NON_COMPACTNESS_PENALTY            : return "Ellipsoid Non-Compactness Penalty? (line 51)";  
    case INTERVAL_STARTRANGE                : return "Flexiable Window Start Range (line 52)";  
    case INTERVAL_ENDRANGE                  : return "Flexiable Window End Range (line 53)";  
    case TIMETRENDCONVRG                    : return "Time Trend Convergence Value (line 54)";  
    case MAXCIRCLEPOPFILE                   : return "Max Circle Size Filename (line 55)";;  
    case EARLY_SIM_TERMINATION              : return "Early Termination of Simulations? (line 56)";  
    case REPORTED_GEOSIZE                   : return "Max Size of Reported Spatial Cluster (line 57)";  
    case USE_REPORTED_GEOSIZE               : return "Use Max Reported Spatial Cluster? (line 58)";  
    case SIMULATION_TYPE                    : return "Simulation Method Type (line 59)";  
    case SIMULATION_SOURCEFILE              : return "Simulation Data Import Filename (line 60)";  
    case ADJ_BY_RR_FILE                     : return "Known Relative Risks Filename (line 61)";  
    case OUTPUT_SIMULATION_DATA             : return "Print Simulation Data - ASCII? (line 62)";  
    case SIMULATION_DATA_OUTFILE            : return "Simulation Data Output Filename (line 63)";  
    case ADJ_FOR_EALIER_ANALYSES            : return "Adjust for Earlier Analyses (line 64)";  
    case USE_ADJ_BY_RR_FILE                 : return "Use Known Relative Risks? (line 65)";  
    case SPATIAL_ADJ_TYPE                   : return "Spatial Adjustments Type (line 66)";  
    //NOTE: The next parameter involves multiple datasets - this inidicates where the line based version
    //      of the parameter file can no longer be maintained like the ini version unless the settings
    //      only specify one data set.
    case MULTI_DATASET_PURPOSE_TYPE         : return "Multiple Data Set Purpose Type (line 67)";  
    case CREATION_VERSION                   : return "Sotware Version Created (line 68)";  
    case RANDOMIZATION_SEED                 : return "Randomization Seed (line 69)";
    case REPORT_CRITICAL_VALUES             : return "Report Critical Value (line 70)";
    case EXECUTION_TYPE                     : return "Analysis Execution Type (line 71)";
    case NUM_PROCESSES                      : return "Number Parallel Processes (line 72)";
    case LOG_HISTORY                        : return "Log Run to History File (line 73)";
    case SUPPRESS_WARNINGS                  : return "Suppress Warnings (line 74)";
    case MAX_REPORTED_SPATIAL_TYPE          : return "Max Spatial Cluster Size Type - Reported Clusters (line 75)";
    case OUTPUT_MLC_CASE_ASCII              : return "Output Cluster Case Information - ASCII? (line 76)";
    case OUTPUT_MLC_CASE_DBASE              : return "Output Cluster Case Information - dBase? (line 77)";
    case STUDYPERIOD_DATACHECK              : return "Study Period Data Checking type (line 78)";
    case COORDINATES_DATACHECK              : return "Geographical Coordinates Data Checking type (line 79)";
    case MAXGEOPOPATRISK                    : return "Maximum spatial size in population at risk (line 80)";
    case MAXGEOPOPFILE                      : return "Maximum spatial size in max circle population file (line 81)";
    case MAXGEODISTANCE                     : return "Maximum spatial size in distance from center (line 82)";
    case USE_MAXGEOPOPFILE                  : return "Restrict maximum spatial size - max circle file (line 83)";
    case USE_MAXGEODISTANCE                 : return "Restrict maximum spatial size - distance (line 84)";
    case MAXGEOPOPATRISK_REPORTED           : return "Maximum reported spatial size in population at risk (line 85)";
    case MAXGEOPOPFILE_REPORTED             : return "Maximum reported spatial size in max circle population file (line 86)";
    case MAXGEODISTANCE_REPORTED            : return "Maximum reported spatial size in distance from center (line 87)";
    case USE_MAXGEOPOPFILE_REPORTED         : return "Restrict maximum reported spatial size - max circle file (line 88)";
    case USE_MAXGEODISTANCE_REPORTED        : return "Restrict maximum reported spatial size - distance (line 89)";
    case LOCATION_NEIGHBORS_FILE            : return "Location neighbors filename (line 90)";
    case USE_LOCATION_NEIGHBORS_FILE        : return "Use location neighbors file (line 91)";
    case RANDOMLY_GENERATE_SEED             : return "Randomly generate seed (line 92)";
    case MULTIPLE_COORDINATES_TYPE          : return "Multiple Coordinates Type (line 93)";
    case META_LOCATIONS_FILE                : return "Meta locations filename (line 94)";
    case USE_META_LOCATIONS_FILE            : return "Use meta locations file (line 95)";
    case OBSERVABLE_REGIONS                 : return "Inequalities (line 96)";
    case EARLY_TERM_THRESHOLD               : return "Early Termination Threshold (line 97)";
    case PVALUE_REPORT_TYPE                 : return "p-value reporting type (line 98)";
    case REPORT_GUMBEL                      : return "report Gumbel p-values (line 99)";
    case TIME_TREND_TYPE                    : return "time trend type (line 100)";
    case REPORT_RANK                        : return "report cluster rank (line 101)";
    case PRINT_ASCII_HEADERS                : return "print ascii headers in output files (line 102)";
    case REPORT_HIERARCHICAL_CLUSTERS       : return "report hierarachical clusters (line 103)";
    case REPORT_GINI_CLUSTERS               : return "report gini clusters (line 104)";
    case SPATIAL_MAXIMA                     : return "spatial window maximum window stops (line 105)";
    case GINI_INDEX_REPORT_TYPE             : return "gini index cluster report type (line 106)";
    case GINI_INDEX_PVALUE_CUTOFF           : return "gini index cluster p-value cut off (line 107)";
    case REPORT_GINI_COEFFICENTS            : return "report gini index coefficents (line 108)";
    case PE_COUNT                           : return "total cases in power evaluation (line 109)";
    case PE_CRITICAL_TYPE                   : return "critical value type (line 110)";
    case PE_ESTIMATION_TYPE                 : return "power estimation type (line 111)";
    case PE_ALT_HYPOTHESIS_FILE             : return "power evaluation alternative hypothesis filename (line 112)";
    case PE_POWER_REPLICAS                  : return "number of replications in power step (line 113)";
    case PE_SIMULATION_TYPE                 : return "power evaluation simulation method for power step (line 114)";
    case PE_SIMULATION_SOURCEFILE           : return "power evaluation simulation data source filename (line 115)";
    case PE_METHOD_TYPE                     : return "power evaluation method (line 116)";
    case POWER_001                          : return "power evaluation critical value .001 (line 117)";
    case PE_OUTPUT_SIMUALTION_DATA          : return "report power evaluation randomization data from power step (line 118)";
    case PE_SIMUALTION_OUTPUTFILE           : return "power evaluation simulation data output filename (line 119)";
    case OUTPUT_KML                         : return "output Google Earth KML file (line 120)";
    case OUTPUT_TEMPORAL_GRAPH              : return "output temporal graph HTML file (line 121)";
    case TEMPORAL_GRAPH_REPORT_TYPE         : return "temporal graph report type (line 122)";
    case TEMPORAL_GRAPH_MLC_COUNT           : return "temporal graph MLC count (line 123)";
    case TEMPORAL_GRAPH_CUTOFF              : return "temporal graph significant p-value (line 124)";
    case OUTPUT_SHAPEFILES                  : return "output shapefiles (line 125)";
    case INCLUDE_LOCATIONS_KML              : return "whether to include cluster locations kml output (line 126)";
    case LOCATIONS_THRESHOLD_KML            : return "threshold for generating separate kml files for cluster locations (line 127)";
    case COMPRESS_KML_OUTPUT                : return "compress the kml output into a kmz file (line 128)";
    case LAUNCH_KML_VIEWER                  : return "whether to launch kml/kmz viewer - gui only (line 129)";
    case ADJUST_WEEKLY_TRENDS               : return "adjust for weekly trends, nonparametric (line 130)";
    case MIN_TEMPORAL_CLUSTER               : return "minimum temporal cluster size in time aggregation units (line 131)";
    case USER_DEFINED_TITLE                 : return "user defined title (line 132)";
    case CALCULATE_OLIVEIRA                 : return "whether to calculate Oliveira's F (line 133)";
    case NUM_OLIVEIRA_SETS                  : return "number of data sets with Oliveira's F calculation (line 134)";
    case OLIVEIRA_CUTOFF                    : return "p-value cutoff with Oliveira's F calculation (line 135)";
    case OUTPUT_CARTESIAN_GRAPH             : return "output cartesian graph file (line 136)";
    case RISK_LIMIT_HIGH_CLUSTERS           : return "restrict high clusters by risk level (line 137)";
    case RISK_THESHOLD_HIGH_CLUSTERS        : return "threshold for high clusters by risk level (line 137)";
    case RISK_LIMIT_LOW_CLUSTERS            : return "restrict low clusters by risk level (line 139)";
    case RISK_THESHOLD_LOW_CLUSTERS         : return "threshold for low clusters by risk level (line 140)";
    case MIN_CASES_LOWRATE_CLUSTERS         : return "minimum cases in low rate cluster (line 141)";
    case MIN_CASES_HIGHRATE_CLUSTERS        : return "minimum cases in high rate cluster (line 142)";
    case LAUNCH_CARTESIAN_MAP               : return "whether to launch cartesian graph - gui only (line 129)";
    default : throw prg_error("Unknown parameter enumeration %d.\n", "GetParameterLabel()", eParameterType);
  };
}

/** Read scanning line version of parameter file. */
bool ScanLineParameterFileAccess::Read(const char* sFileName) {
  bool          bEOF=false;
  size_t        iPos, iLinesRead=0;
  std::ifstream SourceFile;
  std::string   sLineBuffer;

  try {
    gvParametersMissingDefaulted.clear();
    gbReadStatusError = false;

    SourceFile.open(sFileName);
    if (!SourceFile)
      throw resolvable_error("Error: Could not open file:\n'%s'.\n", sFileName);

    gParameters.SetSourceFileName(sFileName);
    gParameters.SetAsDefaulted();

    //initialize as 2.1.3, the last version with documentation or with available executable to produce param file
    CParameters::CreationVersion Version = {2, 1, 3};
    gParameters.SetVersion(Version);

    while (iLinesRead < (size_t)gParameters.GetNumReadParameters() && !bEOF) {
         bEOF = !getlinePortable(SourceFile, sLineBuffer);
         if (!bEOF) {
           ++iLinesRead;
           //Pre-process parameters that have descriptions, strip decription off.
           if (!((ParameterType)iLinesRead == CASEFILE || (ParameterType)iLinesRead == POPFILE ||
                (ParameterType)iLinesRead == COORDFILE || (ParameterType)iLinesRead == OUTPUTFILE ||
                (ParameterType)iLinesRead == GRIDFILE || (ParameterType)iLinesRead == CONTROLFILE ||
                (ParameterType)iLinesRead == MAXCIRCLEPOPFILE || (ParameterType)iLinesRead == SIMULATION_SOURCEFILE ||
                (ParameterType)iLinesRead == SIMULATION_DATA_OUTFILE || (ParameterType)iLinesRead == ADJ_BY_RR_FILE ||
                (ParameterType)iLinesRead == LOCATION_NEIGHBORS_FILE || (ParameterType)iLinesRead == META_LOCATIONS_FILE)) {
              if ((iPos = sLineBuffer.find("//")) != sLineBuffer.npos)
                sLineBuffer.resize(iPos);
           }
           trimString(sLineBuffer);
           SetParameter((ParameterType)iLinesRead, sLineBuffer, gPrintDirection);
         }
    }

    if (iLinesRead <= START_PROSP_SURV) {
      //Version 2.1.3 had 40 parameters where the 40th item in enumeration
      //as called EXTRA4, now called START_PROSP_SURV.
      //In the switch to version 3.x somewhere the enumerations for analysis type
      //was modified from:
      //enum {PURELYSPATIAL=1, SPACETIME, PURELYTEMPORAL, PURELYSPATIALMONOTONE};
      //to
      //enum {PURELYSPATIAL=1, PURELYTEMPORAL, SPACETIME, PROSPECTIVESPACETIME, PURELYSPATIALMONOTONE};
      //probably to accommodate the user interface? So we need to correct this
      //change in v2.1.3 and prior parameter files. Note that PURELYSPATIALMONOTONE
      //also become 5 instead of 4. It appears that this analysis type was never
      //made available so that it is not a concern.
      if (gParameters.GetAnalysisType() == SPACETIME)
        gParameters.SetAnalysisType(PURELYTEMPORAL);
      else if (gParameters.GetAnalysisType() == PURELYTEMPORAL)
        gParameters.SetAnalysisType(SPACETIME);
    }

    if (iLinesRead > START_PROSP_SURV) {
      //versions 3.0.0 - 3.0.5 all had same number of parameters; this also was the last version
      //which saved parameters in line based format; subsequent versions saved as ini file
      Version.iMajor=3; Version.iMinor=0; Version.iRelease=5;
      gParameters.SetVersion(Version);
    }

    //In version 7.0, the maximum spatial cluster size feature was modified to 'always' impose maximum on spatial size
    //and other maximums were made to be additional simultaneous restrictions.
    //Before version 3.0, there was only this one spatial option available -- so we need to assign read maximum value
    //here for parameter versions before 3.0.
    if (gParameters.GetCreationVersion().iMajor < 3)
      gParameters.SetMaxSpatialSizeForType(PERCENTOFPOPULATION, gdMaxSpatialClusterSize, false);
      
   ////Mark defaulted values.
   //if (iLinesRead != gParameters.GetNumReadParameters())
   //   while (++iLinesRead <= gParameters.GetNumReadParameters())
   //       MarkAsMissingDefaulted((ParameterType)iLinesRead, gPrintDirection);
  }
  catch (prg_exception& x) {
    x.addTrace("Read()", "ScanLineParameterFileAccess");
    throw;
  }
  return !gbReadStatusError;
}

/** Write parameters to file - not implemented for multiple data sets. */
void ScanLineParameterFileAccess::Write(const char * sFilename) {
  std::ofstream parameters;
  std::string   s, c;
  unsigned int  iLen;

  try {
    if (gParameters.GetNumDataSets() > 1)
      throw resolvable_error("Error: Lined based parameter file can not write with multiple data sets.\n");

    //open output file
    parameters.open(sFilename, std::ios::trunc);
    if (!parameters)
      throw resolvable_error("Error: Could not open parameter file '%s' for write.\n", sFilename);

    for (int eParameterType=ANALYSISTYPE; eParameterType <= gParameters.giNumParameters; ++eParameterType) {
       parameters << GetParameterString((ParameterType)eParameterType, s).c_str();
       //Don't write comment string for parameters which specify filenames -- problem for read
       if (!((ParameterType)eParameterType == CASEFILE || (ParameterType)eParameterType == POPFILE ||
             (ParameterType)eParameterType == COORDFILE || (ParameterType)eParameterType == OUTPUTFILE ||
             (ParameterType)eParameterType == GRIDFILE || (ParameterType)eParameterType == CONTROLFILE ||
             (ParameterType)eParameterType == MAXCIRCLEPOPFILE || (ParameterType)eParameterType == SIMULATION_SOURCEFILE ||
             (ParameterType)eParameterType == SIMULATION_DATA_OUTFILE || (ParameterType)eParameterType == ADJ_BY_RR_FILE ||
             (ParameterType)eParameterType == LOCATION_NEIGHBORS_FILE || (ParameterType)eParameterType == META_LOCATIONS_FILE)) {
          iLen = s.size();
          while (++iLen < 30) parameters << ' ';
          //older versions of SaTScan limited each line to 150 characters - so print just enough of comment
          //to get idea what is it.
          c = GetParameterComment((ParameterType)eParameterType);
          c.resize(std::min(c.size(), (size_t)50));
          parameters << "   // " << c.c_str();
        }
       parameters << std::endl;
    }
    parameters.close();
  }
  catch (prg_exception& x) {
    x.addTrace("Write()","ScanLineParameterFileAccess");
    throw;
  }
}



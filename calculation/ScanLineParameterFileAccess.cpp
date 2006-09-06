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
  try {
    switch (eParameterType) {
      case ANALYSISTYPE              : return "Analysis Type (line 1)";
      case SCANAREAS                 : return "Scan Areas Type (line 2)";  
      case CASEFILE                  : return "Case Filename (line 3)";  
      case POPFILE                   : return "Population Filename (line 4)";  
      case COORDFILE                 : return "Coordinate Filename (line 5)";  
      case OUTPUTFILE                : return "Results Filename (line 6)";  
      case PRECISION                 : return "Time Precision of Case File Dates (line 7)";  
      case DIMENSION                 : return "*Not Used* (line 8)";  
      case SPECIALGRID               : return "Use Grid File? (line 9)";  
      case GRIDFILE                  : return "Grid Filename (line 10)";  
      case GEOSIZE                   : return "Max Spatial Cluster Size (line 11)";  
      case STARTDATE                 : return "Study Period Start Date (line 12)";  
      case ENDDATE                   : return "Study Period End Date (line 13)";  
      case CLUSTERS                  : return "Include Clusters Type (line 14)";  
      case EXACTTIMES                : return "*Not Used* (line 15)";  
      case TIME_AGGREGATION_UNITS    : return "Time Aggregation Units (line 16)";  
      case TIME_AGGREGATION          : return "Time Aggregation Length (line 17)";  
      case PURESPATIAL               : return "Include Purely Spatial Cluster (line 18)";  
      case TIMESIZE                  : return "Max Temporal Cluster Size (line 19)";  
      case REPLICAS                  : return "Num Monte Carlo Replications (line 20)";  
      case MODEL                     : return "Probability Model Type (line 21)";  
      case RISKFUNCTION              : return "Isotonic Scan (line 22)";  
      case POWERCALC                 : return "Perform Power Calculations? (line 23)";  
      case POWERX                    : return "Power Calculation LLR #1 (line 24)";  
      case POWERY                    : return "Power Calculation LLR #2 (line 25)";  
      case TIMETREND                 : return "Temporal Trend Adjustment Type (line 26)";  
      case TIMETRENDPERC             : return "Temporal Trend Percentage (line 27)";  
      case PURETEMPORAL              : return "Include Purely Temporal Cluster (line 28)";  
      case CONTROLFILE               : return "Control Filename (line 29)";  
      case COORDTYPE                 : return "Coordinates Type (line 30)";  
      case OUTPUT_SIM_LLR_ASCII      : return "Output Simulation LLR - ASCII? (line 31)";  
      case ITERATIVE                 : return "Perform Iterative Scan? (line 32)";
      case ITERATIVE_NUM             : return "Num Iterative Scan Iterations (line 33)";
      case ITERATIVE_PVAL            : return "Iterative Scan P-Value Cutoff (line 34)";
      case VALIDATE                  : return "Validate Settings (line 35)";  
      case OUTPUT_RR_ASCII           : return "Output Relative Risks - ASCII? (line 36)";  
      case WINDOW_SHAPE              : return "Spatial Window Shape (line 37)";  
      case ESHAPES                   : return "Ellipse Shapes (line 38)";  
      case ENUMBERS                  : return "Ellipse Angles (line 39)";  
      case START_PROSP_SURV          : return "Prospective Surveillance Start Date (line 40)";  
      case OUTPUT_AREAS_ASCII        : return "Output Location Information - ASCII? (line 41)";  
      case OUTPUT_MLC_ASCII          : return "Output Cluster Information - ASCII? (line 42)";  
      case CRITERIA_SECOND_CLUSTERS  : return "Criteria Secondary Clusters Type (line 43)";  
      case MAX_TEMPORAL_TYPE         : return "Max Temporal Cluster Size Type (line 44)";  
      case MAX_SPATIAL_TYPE          : return "Max Spatial Cluster Size Type (line 45)";
      case RUN_HISTORY_FILENAME      : return "*Not Used* (line 46)";  
      case OUTPUT_MLC_DBASE          : return "Output Cluster Information - dBase? (line 47)";  
      case OUTPUT_AREAS_DBASE        : return "Output Location Information - dBase? (line 48)";  
      case OUTPUT_RR_DBASE           : return "Output Relative Risks - dBase? (line 49)";  
      case OUTPUT_SIM_LLR_DBASE      : return "Output Simulation LLR - dBase? (line 50)";  
      case NON_COMPACTNESS_PENALTY   : return "Ellipsoid Non-Compactness Penalty? (line 51)";  
      case INTERVAL_STARTRANGE       : return "Flexiable Window Start Range (line 52)";  
      case INTERVAL_ENDRANGE         : return "Flexiable Window End Range (line 53)";  
      case TIMETRENDCONVRG           : return "Time Trend Convergence Value (line 54)";  
      case MAXCIRCLEPOPFILE          : return "Max Circle Size Filename (line 55)";;  
      case EARLY_SIM_TERMINATION     : return "Early Termination of Simulations? (line 56)";  
      case REPORTED_GEOSIZE          : return "Max Size of Reported Spatial Cluster (line 57)";  
      case USE_REPORTED_GEOSIZE      : return "Use Max Reported Spatial Cluster? (line 58)";  
      case SIMULATION_TYPE           : return "Simulation Method Type (line 59)";  
      case SIMULATION_SOURCEFILE     : return "Simulation Data Import Filename (line 60)";  
      case ADJ_BY_RR_FILE            : return "Known Relative Risks Filename (line 61)";  
      case OUTPUT_SIMULATION_DATA    : return "Print Simulation Data - ASCII? (line 62)";  
      case SIMULATION_DATA_OUTFILE   : return "Simulation Data Output Filename (line 63)";  
      case ADJ_FOR_EALIER_ANALYSES   : return "Adjust for Earlier Analyses (line 64)";  
      case USE_ADJ_BY_RR_FILE        : return "Use Known Relative Risks? (line 65)";  
      case SPATIAL_ADJ_TYPE          : return "Spatial Adjustments Type (line 66)";  
      //NOTE: The next parameter involves multiple datasets - this inidicates where the line based version
      //      of the parameter file can no longer be maintained like the ini version unless the settings
      //      only specify one data set.
      case MULTI_DATASET_PURPOSE_TYPE: return "Multiple Data Set Purpose Type (line 67)";  
      case CREATION_VERSION          : return "Sotware Version Created (line 68)";  
      case RANDOMIZATION_SEED        : return "Randomization Seed (line 69)";
      case REPORT_CRITICAL_VALUES    : return "Report Critical Value (line 70)";
      case EXECUTION_TYPE            : return "Analysis Execution Type (line 71)";
      case NUM_PROCESSES             : return "Number Parallel Processes (line 72)";
      case LOG_HISTORY               : return "Log Run to History File (line 73)";
      case SUPPRESS_WARNINGS         : return "Suppress Warnings (line 74)";
      case MAX_REPORTED_SPATIAL_TYPE : return "Max Spatial Cluster Size Type - Reported Clusters (line 75)";
      case OUTPUT_MLC_CASE_ASCII     : return "Output Cluster Case Information - ASCII? (line 76)";
      case OUTPUT_MLC_CASE_DBASE     : return "Output Cluster Case Information - dBase? (line 77)";
      case STUDYPERIOD_DATACHECK     : return "Study Period Data Checking type (line 78)";
      case COORDINATES_DATACHECK     : return "Geographical Coordinates Data Checking type (line 79)";
      case MAXGEOPOPATRISK           : return "Maximum spatial size in population at risk (line 80)";
      case MAXGEOPOPFILE             : return "Maximum spatial size in max circle population file (line 81)";
      case MAXGEODISTANCE            : return "Maximum spatial size in distance from center (line 82)";
      case USE_MAXGEOPOPFILE         : return "Restrict maximum spatial size - max circle file (line 83)";
      case USE_MAXGEODISTANCE        : return "Restrict maximum spatial size - distance (line 84)";
      case MAXGEOPOPATRISK_REPORTED  : return "Maximum reported spatial size in population at risk (line 85)";
      case MAXGEOPOPFILE_REPORTED    : return "Maximum reported spatial size in max circle population file (line 86)";
      case MAXGEODISTANCE_REPORTED   : return "Maximum reported spatial size in distance from center (line 87)";
      case USE_MAXGEOPOPFILE_REPORTED: return "Restrict maximum reported spatial size - max circle file (line 88)";
      case USE_MAXGEODISTANCE_REPORTED : return "Restrict maximum reported spatial size - distance (line 89)";
      case LOCATION_NEIGHBORS_FILE   : return "Location neighbors filename (line 90)";
      case USE_LOCATION_NEIGHBORS_FILE  : return "Use location neighbors file (line 91)";
      case MULTIPLE_COORDINATES_TYPE : return "Multiple Coordinates Type (line 92)";
      default : ZdException::Generate("Unknown parameter enumeration %d.\n", "GetParameterLabel()", eParameterType);
    };
  }
  catch (ZdException& x) {
    x.AddCallpath("GetParameterLabel()","ScanLineParameterFileAccess");
    throw;
  }
  return 0;
}

/** Read scanning line version of parameter file. */
bool ScanLineParameterFileAccess::Read(const char* sFileName) {
  bool          bEOF=false;
  int           iPos, iLinesRead=0;
  ZdIO          ParametersFile;
  ZdString      sLineBuffer;

  try {
    gvParametersMissingDefaulted.clear();
    gbReadStatusError = false;
  
    ParametersFile.Open(sFileName, ZDIO_OPEN_READ);
    gParameters.SetSourceFileName(sFileName);
    gParameters.SetAsDefaulted();

    //initialize as 2.1.3, the last version with documentation or with available executable to produce param file
    CParameters::CreationVersion Version = {2, 1, 3};
    gParameters.SetVersion(Version);

    while (iLinesRead < gParameters.GetNumReadParameters() && !bEOF) {
         bEOF = !ParametersFile.ReadLine(sLineBuffer);
         if (!bEOF) {
           ++iLinesRead;
           //Pre-process parameters that have descriptions, strip decription off.
           if (!((ParameterType)iLinesRead == CASEFILE || (ParameterType)iLinesRead == POPFILE ||
                (ParameterType)iLinesRead == COORDFILE || (ParameterType)iLinesRead == OUTPUTFILE ||
                (ParameterType)iLinesRead == GRIDFILE || (ParameterType)iLinesRead == CONTROLFILE ||
                (ParameterType)iLinesRead == MAXCIRCLEPOPFILE || (ParameterType)iLinesRead == SIMULATION_SOURCEFILE ||
                (ParameterType)iLinesRead == SIMULATION_DATA_OUTFILE || (ParameterType)iLinesRead == ADJ_BY_RR_FILE ||
                (ParameterType)iLinesRead == LOCATION_NEIGHBORS_FILE)) {
              if ((iPos = sLineBuffer.Find("//")) > -1)
                sLineBuffer.Truncate(iPos);
           }
           sLineBuffer.Deblank();
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
  catch (ZdException & x) {
    x.AddCallpath("Read()", "ScanLineParameterFileAccess");
    throw;
  }
  return !gbReadStatusError;
}

/** Write parameters to file - not implemented for multiple data sets. */
void ScanLineParameterFileAccess::Write(const char * sFilename) {
  std::ofstream parameters;
  ZdString      s, c;
  unsigned int  iLen;

  try {
    if (gParameters.GetNumDataSets() > 1)
      GenerateResolvableException("Error: Lined based parameter file can not write with multiple data sets.\n", "Write()");

    //open output file
    parameters.open(sFilename, std::ios::trunc);
    if (!parameters)
      GenerateResolvableException("Error: Could not open parameter file '%s' for write.\n", "Write()", sFilename);

    for (int eParameterType=ANALYSISTYPE; eParameterType <= gParameters.giNumParameters; ++eParameterType) {
       parameters << GetParameterString((ParameterType)eParameterType, s).GetCString();
       //Don't write comment string for parameters which specify filenames -- problem for read
       if (!((ParameterType)eParameterType == CASEFILE || (ParameterType)eParameterType == POPFILE ||
             (ParameterType)eParameterType == COORDFILE || (ParameterType)eParameterType == OUTPUTFILE ||
             (ParameterType)eParameterType == GRIDFILE || (ParameterType)eParameterType == CONTROLFILE ||
             (ParameterType)eParameterType == MAXCIRCLEPOPFILE || (ParameterType)eParameterType == SIMULATION_SOURCEFILE ||
             (ParameterType)eParameterType == SIMULATION_DATA_OUTFILE || (ParameterType)eParameterType == ADJ_BY_RR_FILE ||
             (ParameterType)eParameterType == LOCATION_NEIGHBORS_FILE)) {
          iLen = s.GetLength();
          while (++iLen < 30) parameters << ' ';
          //older versions of SaTScan limited each line to 150 characters - so print just enough of comment
          //to get idea what is it.
          c = GetParameterComment((ParameterType)eParameterType);
          c.Truncate(std::min(c.GetLength(), (unsigned long)50));
          parameters << "   // " << c.GetCString();
        }
       parameters << std::endl;
    }
    parameters.close();
  }
  catch (ZdException& x) {
    x.AddCallpath("Write()","ScanLineParameterFileAccess");
    throw;
  }
}



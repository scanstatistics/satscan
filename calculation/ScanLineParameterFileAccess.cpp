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
    case TIMETRENDLENGTH                    : return "Time Stratified Adjustment Length (line 28)";
    case PURETEMPORAL                       : return "Include Purely Temporal Cluster (line 29)";  
    case CONTROLFILE                        : return "Control Filename (line 30)";  
    case COORDTYPE                          : return "Coordinates Type (line 31)";  
    case OUTPUT_SIM_LLR_ASCII               : return "Output Simulation LLR - ASCII? (line 31)";
    case ITERATIVE                          : return "Perform Iterative Scan? (line 33)";
    case ITERATIVE_NUM                      : return "Num Iterative Scan Iterations (line 34)";
    case ITERATIVE_PVAL                     : return "Iterative Scan P-Value Cutoff (line 35)";
    case VALIDATE                           : return "Validate Settings (line 36)";
    case OUTPUT_RR_ASCII                    : return "Output Relative Risks - ASCII? (line 37)";
    case WINDOW_SHAPE                       : return "Spatial Window Shape (line 38)";
    case ESHAPES                            : return "Ellipse Shapes (line 39)";
    case ENUMBERS                           : return "Ellipse Angles (line 40)";
    case START_PROSP_SURV                   : return "Prospective Surveillance Start Date (line 41)";
    case OUTPUT_AREAS_ASCII                 : return "Output Location Information - ASCII? (line 42)";  
    case OUTPUT_MLC_ASCII                   : return "Output Cluster Information - ASCII? (line 43)";  
    case CRITERIA_SECOND_CLUSTERS           : return "Criteria Secondary Clusters Type (line 44)";  
    case MAX_TEMPORAL_TYPE                  : return "Max Temporal Cluster Size Type (line 45)";  
    case MAX_SPATIAL_TYPE                   : return "Max Spatial Cluster Size Type (line 46)";
    case RUN_HISTORY_FILENAME               : return "*Not Used* (line 47)";
    case OUTPUT_MLC_DBASE                   : return "Output Cluster Information - dBase? (line 48)";  
    case OUTPUT_AREAS_DBASE                 : return "Output Location Information - dBase? (line 49)";
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
    CParameters::CreationVersion Version(2, 1, 3);
    gParameters.SetVersion(Version);

    while (iLinesRead < (size_t)gParameters.GetNumReadParameters() && !bEOF) {
         bEOF = !getlinePortable(SourceFile, sLineBuffer);
         if (!bEOF) {
           ++iLinesRead;
           if (iLinesRead > OUTPUT_AREAS_DBASE)
               throw resolvable_error("Error: Maximum number of parameters exceeded for line based parameter file.\n"
                                      "Use .ini parameter file if additional parameters are needed.\n");
           //Pre-process parameters that have descriptions, strip decription off.
           if (!((ParameterType)iLinesRead == CASEFILE || (ParameterType)iLinesRead == POPFILE ||
                (ParameterType)iLinesRead == COORDFILE || (ParameterType)iLinesRead == OUTPUTFILE ||
                (ParameterType)iLinesRead == GRIDFILE || (ParameterType)iLinesRead == CONTROLFILE)) {
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
    if (gParameters.getNumFileSets() > 1)
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
             (ParameterType)eParameterType == GRIDFILE || (ParameterType)eParameterType == CONTROLFILE)) {
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



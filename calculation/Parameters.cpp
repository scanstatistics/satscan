#include "SaTScan.h"
#pragma hdrstop
#include "Parameters.h"
#include <io.h>

const char*      ANALYSIS_HISTORY_FILE  = "AnalysisHistory.dbf";
const char*      YES                    = "y";
const char*      NO                     = "n";

const char*      INPUT_FILES_SECTION            = "[InputFiles]";
const char*      CASE_FILE_LINE                 = "CaseFile";
const char*      CONTROL_FILE_LINE              = "ControlFile";
const char*      POP_FILE_LINE                  = "PopulationFile";
const char*      COORD_FILE_LINE                = "CoordinatesFile";
const char*      GRID_FILE_LINE                 = "GridFile";

const char*      MODEL_INFO_SECTION             = "[ModelInfo]";
const char*      ANALYSIS_TYPE_LINE             = "AnalysisType";
const char*      SCAN_AREAS_LINE                = "ScanAreas";
const char*      PRECISION_TIMES_LINE           = "PrecisionCaseTimes";
const char*      MAX_GEO_SIZE_LINE              = "MaxGeographicSize";
const char*      START_DATE_LINE                = "StartDate";
const char*      END_DATE_LINE                  = "EndDate";
const char*      ALIVE_CLUSTERS_LINE            = "AliveClustersOnly";
const char*      INTERVAL_UNITS_LINE            = "IntervalUnits";
const char*      INTERVAL_LENGTH_LINE           = "IntervalLength";
const char*      INCLUDE_PURELY_SPATIAL_LINE    = "IncludePurelySpatial";
const char*      MAX_TEMP_SIZE_LINE             = "MaxTemporalSize";
const char*      MONTE_CARLO_REPS_LINE          = "MonteCarloReps";
const char*      MODEL_TYPE_LINE                = "ModelType";
const char*      ISOTONIC_SCAN_LINE             = "IsotonicScan";
const char*      PVALUE_PROSPECT_LLR_LINE       = "PValues2ProspectiveLLRs";
const char*      LLR_1_LINE                     = "LLR1";
const char*      LLR_2_LINE                     = "LLR2";
const char*      TIME_TREND_ADJ_LINE            = "TimeTrendAdjustmentType";
const char*      TIME_TREND_PERCENT_LINE        = "TimeTrendPercentage";
const char*      INCLUDE_PURE_TEMP_LINE         = "IncludePurelyTemporal";
const char*      COORD_TYPE_LINE                = "CoordinatesType";
const char*      VALID_PARAMS_LINE              = "ValidateParameters";
const char*      PROSPECT_START_LINE            = "ProspectiveStartDate";
const char*      CRIT_REPORT_SEC_CLUSTERS_LINE  = "CriteriaForReportingSecondaryClusters";
const char*      MAX_TEMP_INTERPRET_LINE        = "MaxTemporalSizeInterpretation";
const char*      MAX_SPATIAL_SIZE_LINE          = "MaxSpatialSizeInterpretation";

const char*      SEQUENTIAL_SCAN_SECTION        = "[SequentialScan]";
const char*      SEQUENTIAL_SCAN_LINE           = "SequentialScan";
const char*      SEQUENTIAL_MAX_ITERS_LINE      = "SequentialScanMaxIterations";
const char*      SEQUENTIAL_MAX_PVALUE_LINE     = "SequentialScanMaxPValue";

const char*      ELLIPSES_SECTION               = "[Ellipses]";
const char*      NUMBER_ELLIPSES_LINE           = "NumberOfEllipses";
const char*      ELLIPSE_SHAPES_LINE            = "EllipseShapes";
const char*      ELLIPSE_ANGLES_LINE            = "EllipseAngles";

const char*      OUTPUT_FILES_SECTION           = "[OutputFiles]";
const char*      RESULTS_FILE_LINE              = "ResultsFile";
const char*      ANALYSIS_HISTORY_LINE          = "AnalysisRunHistoryFile";
const char*      MOST_LIKELY_CLUSTER_LINE       = "MostLikelyClusterEachCentroidASCII";
const char*      DBASE_CLUSTER_LINE             = "MostLikelyClusterEachCentroidDBase";
const char*      CENSUS_REPORT_CLUSTERS_LINE    = "CensusAreasReportedClustersASCII";
const char*      DBASE_AREA_LINE                = "CensusAreasReportedClustersDBase";
const char*      SAVE_SIM_LLRS                  = "SaveSimLLRsASCII";
const char*      DBASE_LOG_LIKELI               = "SaveSimLLRsDBase";
const char*      INCLUDE_REL_RISKS_LINE         = "IncludeRelativeRisksCensusAreasASCII";
const char*      DBASE_RELATIVE_RISKS           = "IncludeRelativeRisksCensusAreasDBase";


char mgsVariableLabels[50][100] = {
   "Analysis Type",
   "Scan Areas",
   "Case File",
   "Population File",
   "Coordinates File",
   "Results File",
   "Precision of Case Times",
   "Not applicable",
   "Special Grid File Use",
   "Grid File",
   "Max Geographic size",
   "Start Date",
   "End Date",
   "Alive Clusters Only",
   "Exact Times",
   "Interval Units",
   "Interval Length",
   "Include Purely Spatial",
   "Max Temporal Size",
   "Replications",
   "Model Type (Poisson, Bernoulli, or Space-Time Permutation)",
   "Isotonic Scan",
   "p-Values for 2 Prospective LLR's",
   "LLR #1",
   "LLR #2",
   "Time Trend Adjustment Type",
   "Time Trend Percentage",
   "Include Purely Temporal",
   "Control File",
   "Coordinates Type",
   "Save Sim LLRs",
   "Sequential Scan",
   "Sequential Scan Max Iterations",
   "Sequential Scan Max p-Value",
   "Validate Parameters",
   "Include Relative Risks for Census Areas",
   "Number of Ellipses",
   "Ellipse Shapes",
   "Ellipse Angles",
   "Prospective Start Date",
   "Output Census Areas in Reported Clusters",
   "Output Most Likely Cluster for each Centroid",
   "Criteria for Reporting Secondary Clusters",
   "How Max Temporal Size Should Be Interpreted",
   "How Max Spatial Size Should Be Interpreted",
   "Analysis Run History File",
   "Output for cluster information",
   "Output for area information",
   "dBase output for Relative Risks",
   "dBase output for Log Likelihoods"
   };

CParameters::CParameters(bool bDisplayErrors) {
   SetDefaults();
   m_bDisplayErrors = bDisplayErrors;
}

//------------------------------------------------------------------------------
//Copy constructor
//------------------------------------------------------------------------------
CParameters::CParameters(const CParameters &other) {
   SetDefaults();
   copy(other);
}

CParameters::~CParameters() {
   Free();
}

//------------------------------------------------------------------------------
//Overload assignment operator
//------------------------------------------------------------------------------
CParameters &CParameters::operator= (const CParameters &rhs) {
   if (this != &rhs)
      copy(rhs);

   return (*this);
}

// checks to make sure all of the required keys exist in the ellipse section of the ini
// pre: file is opened ini parameters file
// post: throws an exception if one of the keys required is missing
void CParameters::CheckEllipseIniSection(ZdIniFile& file, bool bCreateIfMissing) {
   try {
      ZdIniSection *pSection = file.GetSection(ELLIPSES_SECTION);
      if ( pSection->FindKey(NUMBER_ELLIPSES_LINE) == -1 ) {
 //        if (bCreateIfMissing)
            pSection->AddComment(" number of ellipses (0-10)");
            pSection->AddLine(NUMBER_ELLIPSES_LINE, "");
  //       else
  //          ZdException::GenerateNotification("Error reading parameter file. Number of ellipses line is missing!", "Error!");
      }
      if ( pSection->FindKey(ELLIPSE_SHAPES_LINE) == -1 ) {
  //       if (bCreateIfMissing)
            pSection->AddComment(" ellipse shapes");
            pSection->AddLine(ELLIPSE_SHAPES_LINE, "");
  //       else
  //          ZdException::GenerateNotification("Error reading parameter file. Ellipse shapes line is missing!", "Error!");
      }
      if ( pSection->FindKey(ELLIPSE_ANGLES_LINE) == -1 ) {
  //       if (bCreateIfMissing)
            pSection->AddComment(" ellipse angles");
            pSection->AddLine(ELLIPSE_ANGLES_LINE, "");
  //       else
  //          ZdException::GenerateNotification("Error reading parameter file. Ellipse angles line is missing!", "Error!");
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("CheckEllipseIniSection()", "CParameters");
      throw;
   }
}

// checks to make sure all of the required ini sections are in the file
// pre: file is opened ini parameter file
// post: throws an exception if a required section is missing
void CParameters::CheckIniSectionsExist(ZdIniFile& file, bool bCreateIfMissing) {
   try {
      if (file.GetSectionIndex(INPUT_FILES_SECTION) == -1) {
   //      if (bCreateIfMissing)
            file.AddSection(INPUT_FILES_SECTION);
   //      else
   //         ZdException::GenerateNotification("Error reading parameter file. Input files section is missing!", "Error!");
      }
      if (file.GetSectionIndex(MODEL_INFO_SECTION) == -1) {
   //      if (bCreateIfMissing)
            file.AddSection(MODEL_INFO_SECTION);
   //      else
   //         ZdException::GenerateNotification("Error reading parameter file. Model info section is missing!", "Error!");
      }
      if (file.GetSectionIndex(SEQUENTIAL_SCAN_SECTION) == -1) {
   //      if (bCreateIfMissing)
            file.AddSection(SEQUENTIAL_SCAN_SECTION);
   //      else
   //         ZdException::GenerateNotification("Error reading parameter file. Sequential scan section is missing!", "Error!");
      }
      if (file.GetSectionIndex(ELLIPSES_SECTION) == -1) {
  //       if (bCreateIfMissing)
            file.AddSection(ELLIPSES_SECTION);
  //       else
  //          ZdException::GenerateNotification("Error reading parameter file. Ellipses section is missing!", "Error!");
      }
      if (file.GetSectionIndex(OUTPUT_FILES_SECTION) == -1) {
  //       if (bCreateIfMissing)
            file.AddSection(OUTPUT_FILES_SECTION);
  //       else
  //          ZdException::GenerateNotification("Error reading parameter file. Output files section is missing!", "Error!");
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("CheckIniSectionsExist()", "CParameters");
      throw;
   }
}

// checks to make sure all of the required keys exist in the input file section of the ini
// pre: file is opened ini parameters file
// post: throws an exception if one of the keys required is missing
void CParameters::CheckInputFileSection(ZdIniFile& file, bool bCreateIfMissing) {
   try {
      ZdIniSection* pSection = file.GetSection(INPUT_FILES_SECTION);
      if ( pSection->FindKey(CASE_FILE_LINE) == -1 ) {
  //       if (bCreateIfMissing)
            pSection->AddLine(CASE_FILE_LINE, "");
  //       else
  //          ZdException::GenerateNotification("Error reading parameter file. Case file line is missing!", "Error!");
      }
      if ( pSection->FindKey(POP_FILE_LINE) == -1 ) {
  //       if (bCreateIfMissing)
            pSection->AddLine(POP_FILE_LINE, "");
  //       else
  //          ZdException::GenerateNotification("Error reading parameter file. Population file line is missing!", "Error!");
      }
      if ( pSection->FindKey(COORD_FILE_LINE) == -1 ) {
  //       if (bCreateIfMissing)
            pSection->AddLine(COORD_FILE_LINE, "");
 //        else
 //           ZdException::GenerateNotification("Error reading parameter file. Coordinates file line is missing!", "Error!");
      }
      if ( pSection->FindKey(GRID_FILE_LINE) == -1 ) {
 //        if (bCreateIfMissing)
            pSection->AddLine(GRID_FILE_LINE, "");
 //        else
 //           ZdException::GenerateNotification("Error reading parameter file. Grid file line is missing!", "Error!");
      }
      if ( pSection->FindKey(CONTROL_FILE_LINE) == -1 ) {
 //        if (bCreateIfMissing)
            pSection->AddLine(CONTROL_FILE_LINE, "");
  //       else
  //          ZdException::GenerateNotification("Error reading parameter file. Control file line is missing!", "Error!");
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("CheckInputFileSection()", "CParameters");
      throw;
   }
}

// checks to make sure all of the required keys exist in the model info section of the ini
// pre: file is opened ini parameters file
// post: throws an exception if one of the keys required is missing
void CParameters::CheckModelInfoIniSection(ZdIniFile& file, bool bCreateIfMissing) {
   try {
      ZdIniSection* pSection = file.GetSection(MODEL_INFO_SECTION);
      if ( pSection->FindKey(ANALYSIS_TYPE_LINE) == -1 ) {
 //        if (bCreateIfMissing)
            pSection->AddComment(" analysis type (1=S, 2=T, 3=RST, 4=PST)");
            pSection->AddLine(ANALYSIS_TYPE_LINE, "");
 //        else
 //           ZdException::GenerateNotification("Error reading parameter file. Analysis type line is missing!", "Error!");
      }
      if ( pSection->FindKey(SCAN_AREAS_LINE) == -1 ) {
 //        if (bCreateIfMissing)
            pSection->AddComment(" scan areas (1=High, 2=Low, 3=HighLow)");
            pSection->AddLine(SCAN_AREAS_LINE, "");
 //        else
 //           ZdException::GenerateNotification("Error reading parameter file. Scan Areas line is missing!", "Error!");
      }
      if ( pSection->FindKey(PRECISION_TIMES_LINE) == -1 ) {
  //       if (bCreateIfMissing)
            pSection->AddComment(" precision of case times (0=None, 1=Year, 2=Month, 3=day)");
            pSection->AddLine(PRECISION_TIMES_LINE, "");
  //       else
  //          ZdException::GenerateNotification("Error reading parameter file. Precision of case times line is missing!", "Error!");
      }
      if ( pSection->FindKey(MAX_GEO_SIZE_LINE) == -1 ) {
  //       if (bCreateIfMissing)
            pSection->AddComment(" max geographic size (<=50%)");
            pSection->AddLine(MAX_GEO_SIZE_LINE, "");
  //       else
  //          ZdException::GenerateNotification("Error reading parameter file. Max geographic size line is missing!", "Error!");
      }
      if ( pSection->FindKey(START_DATE_LINE) == -1 ) {
  //       if (bCreateIfMissing)
            pSection->AddComment(" start date (YYYY/MM/DD)");
            pSection->AddLine(START_DATE_LINE, "");
  //       else
  //          ZdException::GenerateNotification("Error reading parameter file. Start date line is missing!", "Error!");
      }
      if ( pSection->FindKey(END_DATE_LINE) == -1 ) {
  //       if (bCreateIfMissing)
            pSection->AddComment(" end date (YYYY/MM/DD)");
            pSection->AddLine(END_DATE_LINE, "");
  //       else
  //          ZdException::GenerateNotification("Error reading parameter file. End date line is missing!", "Error!");
      }
      if ( pSection->FindKey(ALIVE_CLUSTERS_LINE) == -1 ) {
  //       if (bCreateIfMissing)
            pSection->AddComment(" alive clusters only? (y/n)");
            pSection->AddLine(ALIVE_CLUSTERS_LINE, "");
  //       else
  //          ZdException::GenerateNotification("Error reading parameter file. Alive clusters only line is missing!", "Error!");
      }
      if ( pSection->FindKey(INTERVAL_UNITS_LINE) == -1 ) {
  //       if (bCreateIfMissing)
            pSection->AddComment(" interval units (0=None, 1=Year, 2=Month, 3=Day)");
            pSection->AddLine(INTERVAL_UNITS_LINE, "");
  //       else
  //          ZdException::GenerateNotification("Error reading parameter file. Interval units line is missing!", "Error!");
      }
      if ( pSection->FindKey(INTERVAL_LENGTH_LINE) == -1 ) {
  //       if (bCreateIfMissing)
            pSection->AddComment(" inteval length (positive integer)");
            pSection->AddLine(INTERVAL_LENGTH_LINE, "");
  //       else
  //          ZdException::GenerateNotification("Error reading parameter file. Interval length line is missing!", "Error!");
      }
      if ( pSection->FindKey(INCLUDE_PURELY_SPATIAL_LINE) == -1 ) {
  //       if (bCreateIfMissing)
            pSection->AddComment(" include purely spatial clusters (y/n)");
            pSection->AddLine(INCLUDE_PURELY_SPATIAL_LINE, "");
  //       else
  //          ZdException::GenerateNotification("Error reading parameter file. Include purely spatial clusters line is missing!", "Error!");
      }
      if ( pSection->FindKey(MAX_TEMP_SIZE_LINE) == -1 ) {
 //        if (bCreateIfMissing)
            pSection->AddComment(" max temporal size (<=90%)");
            pSection->AddLine(MAX_TEMP_SIZE_LINE, "");
 //        else
 //           ZdException::GenerateNotification("Error reading parameter file. Max temporal cluseter size line is missing!", "Error!");
      }
      if ( pSection->FindKey(MONTE_CARLO_REPS_LINE) == -1 ) {
  //       if (bCreateIfMissing)
            pSection->AddComment(" Monte Carlo reps (0, 9, 999, n999)");
            pSection->AddLine(MONTE_CARLO_REPS_LINE, "");
  //       else
  //          ZdException::GenerateNotification("Error reading parameter file. Monte Carlo reps line is missing!", "Error!");
      }
      if ( pSection->FindKey(MODEL_TYPE_LINE) == -1 ) {
 //        if (bCreateIfMissing)
            pSection->AddComment(" model type (0=Poisson, 1=Bernoulli, 2=Space-Time)");
            pSection->AddLine(MODEL_TYPE_LINE, "");
  //       else
  //          ZdException::GenerateNotification("Error reading parameter file. Model type line is missing!", "Error!");
      }
      if ( pSection->FindKey(ISOTONIC_SCAN_LINE) == -1 ) {
 //        if (bCreateIfMissing)
            pSection->AddComment(" Isotonic Scan (y/n)");
            pSection->AddLine(ISOTONIC_SCAN_LINE, "");
 //        else
 //           ZdException::GenerateNotification("Error reading parameter file. Isotonic scan line is missing!", "Error!");
      }
      if ( pSection->FindKey(PVALUE_PROSPECT_LLR_LINE) == -1 ) {
 //        if (bCreateIfMissing)
            pSection->AddComment(" p-Values for 2 Prespecified LLR's (y/n)");
            pSection->AddLine(PVALUE_PROSPECT_LLR_LINE, "");
 //        else
 //           ZdException::GenerateNotification("Error reading parameter file. p-Value for 2 prospective LLRs line is missing!", "Error!");
      }
      if ( pSection->FindKey(LLR_1_LINE) == -1 ) {
 //        if (bCreateIfMissing)
            pSection->AddLine(LLR_1_LINE, "");
 //        else
 //           ZdException::GenerateNotification("Error reading parameter file. LLR1 line is missing!", "Error!");
      }
      if ( pSection->FindKey(LLR_2_LINE) == -1 ) {
 //        if (bCreateIfMissing)
            pSection->AddLine(LLR_2_LINE, "");
 //        else
 //           ZdException::GenerateNotification("Error reading parameter file. LLR2 line is missing!", "Error!");
      }
      if ( pSection->FindKey(TIME_TREND_ADJ_LINE) == -1 ) {
 //        if (bCreateIfMissing)
            pSection->AddComment(" Time trend adjustment type (0=None, 1=Nonparametric, 2=LogLinear)");
            pSection->AddLine(TIME_TREND_ADJ_LINE, "");
 //        else
 //           ZdException::GenerateNotification("Error reading parameter file. Time trend adjustment type line is missing!", "Error!");
      }
      if ( pSection->FindKey(TIME_TREND_PERCENT_LINE) == -1 ) {
 //        if (bCreateIfMissing)
            pSection->AddComment(" time trend adjustment percentage (>-100)");
            pSection->AddLine(TIME_TREND_PERCENT_LINE, "");
 //        else
 //           ZdException::GenerateNotification("Error reading parameter file. Time trend adjustment percentage line is missing!", "Error!");
      }
      if ( pSection->FindKey(INCLUDE_PURE_TEMP_LINE) == -1 ) {
 //        if (bCreateIfMissing)
            pSection->AddComment(" include purely temporal clusters (y/n)");
            pSection->AddLine(INCLUDE_PURE_TEMP_LINE, "");
 //        else
 //           ZdException::GenerateNotification("Error reading parameter file. Include purely temporal clusters line is missing!", "Error!");
      }
      if ( pSection->FindKey(COORD_TYPE_LINE) == -1 ) {
 //        if (bCreateIfMissing)
            pSection->AddComment(" coordinate type (0=Cartesian, 1=Lat/Long)");
            pSection->AddLine(COORD_TYPE_LINE, "");
 //        else
 //           ZdException::GenerateNotification("Error reading parameter file. Coordinate type line is missing!", "Error!");
      }
      if ( pSection->FindKey(VALID_PARAMS_LINE) == -1 ) {
 //        if (bCreateIfMissing)
            pSection->AddComment(" validate parameters (y/n)");
            pSection->AddLine(VALID_PARAMS_LINE, "");
 //        else
 //           ZdException::GenerateNotification("Error reading parameter file. Validate parameters line is missing!", "Error!");
      }
      if ( pSection->FindKey(PROSPECT_START_LINE) == -1 ) {
  //       if (bCreateIfMissing)
            pSection->AddComment(" prospective surveillance start date (YYYY/MM/DD)");
            pSection->AddLine(PROSPECT_START_LINE, "");
 //        else
 //           ZdException::GenerateNotification("Error reading parameter file. Prospective start date line is missing!", "Error!");
      }
      if ( pSection->FindKey(CRIT_REPORT_SEC_CLUSTERS_LINE) == -1 ) {
 //        if (bCreateIfMissing)
            pSection->AddComment(" criteria for reporting secondary clusters(0=NoGeoOverlap, ..., 5=NoRestrictions)");
            pSection->AddLine(CRIT_REPORT_SEC_CLUSTERS_LINE, "");
 //        else
 //           ZdException::GenerateNotification("Error reading parameter file. Report secondary clusters line is missing!", "Error!");
      }
      if ( pSection->FindKey(MAX_TEMP_INTERPRET_LINE) == -1 ) {
 //        if (bCreateIfMissing)
            pSection->AddComment(" how max temporal size should be interpretted (0=Percentage, 1=Time)");
            pSection->AddLine(MAX_TEMP_INTERPRET_LINE, "");
 //        else
 //           ZdException::GenerateNotification("Error reading parameter file. Max temporal interpretation line is missing!", "Error!");
      }
      if ( pSection->FindKey(MAX_SPATIAL_SIZE_LINE) == -1 ) {
 //        if (bCreateIfMissing)
            pSection->AddComment(" how max spatial size should be interpretted (0=Percentage, 1=Distance)");
            pSection->AddLine(MAX_SPATIAL_SIZE_LINE, "");
 //        else
 //           ZdException::GenerateNotification("Error reading parameter file. Max spatial interpretation line is missing!", "Error!");
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("CheckModelInfoIniSection()", "CParameters");
      throw;
   }
}

// checks to make sure all of the required keys exist in the output file section of the ini
// pre: file is opened ini parameters file
// post: throws an exception if one of the keys required is missing
void CParameters::CheckOutputFileIniSection(ZdIniFile& file, bool bCreateIfMissing) {
   try {
      ZdIniSection* pSection = file.GetSection(OUTPUT_FILES_SECTION);
       if ( pSection->FindKey(RESULTS_FILE_LINE) == -1 ) {
  //       if (bCreateIfMissing)
            pSection->AddLine(RESULTS_FILE_LINE, "");
  //       else
  //          ZdException::GenerateNotification("Error reading parameter file. Results file line is missing!", "Error!");
      }
      if ( pSection->FindKey(ANALYSIS_HISTORY_LINE) == -1 ) {
  //       if (bCreateIfMissing)
            pSection->AddLine(ANALYSIS_HISTORY_LINE, "");
  //       else
  //          ZdException::GenerateNotification("Error reading parameter file. Analysis history file line is missing!", "Error!");
      }
      if ( pSection->FindKey(MOST_LIKELY_CLUSTER_LINE) == -1 ) {
  //       if (bCreateIfMissing)
            pSection->AddComment(" output most likely clusters in ASCII format (y/n)");
            pSection->AddLine(MOST_LIKELY_CLUSTER_LINE, "");
  //       else
  //          ZdException::GenerateNotification("Error reading parameter file. Report most likely clusters line is missing!", "Error!");
      }
      if ( pSection->FindKey(DBASE_CLUSTER_LINE) == -1 ) {
  //       if (bCreateIfMissing)
            pSection->AddComment(" output most likely clusters in dBase format (y/n)");
            pSection->AddLine(DBASE_CLUSTER_LINE, "");
  //       else
  //          ZdException::GenerateNotification("Error reading parameter file. dBase cluster level report line is missing!", "Error!");
      }
      if ( pSection->FindKey(CENSUS_REPORT_CLUSTERS_LINE) == -1 ) {
  //       if (bCreateIfMissing)
            pSection->AddComment(" report census areas in ASCII format (y/n)");
            pSection->AddLine(CENSUS_REPORT_CLUSTERS_LINE, "");
  //       else
  //          ZdException::GenerateNotification("Error reading parameter file. Report census clusters line is missing!", "Error!");
      }
      if ( pSection->FindKey(DBASE_AREA_LINE) == -1 ) {
  //       if (bCreateIfMissing)
            pSection->AddComment(" report census areas in dBase format (y/n)");
            pSection->AddLine(DBASE_AREA_LINE, "");
  //       else
  //          ZdException::GenerateNotification("Error reading parameter file. dBase area specific report line is missing!", "Error!");
      }
      if ( pSection->FindKey(SAVE_SIM_LLRS) == -1 ) {
  //       if (bCreateIfMissing)
            pSection->AddComment(" report Simulated Log Likelihoods Ratios in ASCII format (y/n)");
            pSection->AddLine(SAVE_SIM_LLRS, "");
  //       else
  //          ZdException::GenerateNotification("Error reading parameter file. Save simulated LLRs line is missing!", "Error!");
      }
      if ( pSection->FindKey(DBASE_LOG_LIKELI) == -1 ) {
 //        if (bCreateIfMissing)
            pSection->AddComment(" report Simulated Log Likelihoods Ratios in dBase format (y/n)");
            pSection->AddLine(DBASE_LOG_LIKELI, "");
//         else
 //           ZdException::GenerateNotification("Error reading parameter file. Include dBase log likelihood line is missing!", "Error!");
      }
      if ( pSection->FindKey(INCLUDE_REL_RISKS_LINE) == -1 ) {
  //       if (bCreateIfMissing)
            pSection->AddComment(" report relative risks in ASCII format (y/n)");
            pSection->AddLine(INCLUDE_REL_RISKS_LINE, "");
  //       else
  //          ZdException::GenerateNotification("Error reading parameter file. Include relative risks line is missing!", "Error!");
      }
      if ( pSection->FindKey(DBASE_RELATIVE_RISKS) == -1 ) {
   //      if (bCreateIfMissing)
            pSection->AddComment(" report relative risks in dBase format (y/n)");
            pSection->AddLine(DBASE_RELATIVE_RISKS, "");
   //      else
   //         ZdException::GenerateNotification("Error reading parameter file. Include dBase relative risks line is missing!", "Error!");
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("CheckOutputFileIniSection()", "CParameters");
      throw;
   }
}

bool CParameters::CheckProspDateRange(int iStartYear, int iStartMonth, int iStartDay,
                                      int iEndYear, int iEndMonth, int iEndDay,
                                      int iProspYear, int iProspMonth, int iProspDay)
{
   Julian Start, End, Prosp;

   Start = MDYToJulian(iStartMonth, iStartDay, iStartYear);
   End   = MDYToJulian(iEndMonth, iEndDay, iEndYear);
   Prosp = MDYToJulian(iProspMonth, iProspDay, iProspYear);
   if ((Prosp < Start) || (Prosp > End))
      return false;
   else
      return true;
}

// checks to make sure all of the required keys exist in the sequential scan section of the ini
// pre: file is opened ini parameters file
// post: throws an exception if one of the keys required is missing
void CParameters::CheckSequentialScanIniSection(ZdIniFile& file, bool bCreateIfMissing) {
   try {
      ZdIniSection *pSection = file.GetSection(SEQUENTIAL_SCAN_SECTION);
      if ( pSection->FindKey(SEQUENTIAL_SCAN_LINE) == -1 ) {
 //        if (bCreateIfMissing)
            pSection->AddComment(" sequential scan (y/n)");
            pSection->AddLine(SEQUENTIAL_SCAN_LINE, "");
  //       else
  //          ZdException::GenerateNotification("Error reading parameter file. Sequential scan line is missing!", "Error!");
      }
      if ( pSection->FindKey(SEQUENTIAL_MAX_ITERS_LINE) == -1 ) {
  //       if (bCreateIfMissing)
            pSection->AddComment(" max iterations for sequential scan (0-32000)");
            pSection->AddLine(SEQUENTIAL_MAX_ITERS_LINE, "");
  //       else
  //          ZdException::GenerateNotification("Error reading parameter file. Sequential scan max iterations line is missing!", "Error!");
      }
      if ( pSection->FindKey(SEQUENTIAL_MAX_PVALUE_LINE) == -1 ) {
  //       if (bCreateIfMissing)
            pSection->AddComment(" max p-Value for sequential scan (0.000-1.000)");
            pSection->AddLine(SEQUENTIAL_MAX_PVALUE_LINE, "");
   //      else
   //         ZdException::GenerateNotification("Error reading parameter file. Sequential scan max p-Value line is missing!", "Error!");
      }

  //    if (bCreateIfMissing)
  //       file.Write();
   }
   catch (ZdException &x) {
      x.AddCallpath("CheckSequentialScanIniSection()", "CParameters");
      throw;
   }
}

//** Converts m_nMaxClusterSizeType to passed type. */
void CParameters::ConvertMaxTemporalClusterSizeToType(TemporalSizeType eTemporalSizeType) {
  double dTemp, dPrecision = 10000, dTimeBetween;

  try {
    dTimeBetween = TimeBetween(CharToJulian(m_szStartDate),CharToJulian(m_szEndDate), m_nIntervalUnits);
    if (dTimeBetween <= 0)
      SSException::Generate("Invalid study period with start date \"%s\" and end date \"%s\".",
                            "ConvertMaxTemporalClusterSizeToType()", m_szStartDate, m_szEndDate);

    // store intial type and size for parameter output display
    m_nInitialMaxTemporalClusterSize = m_nMaxTemporalClusterSize;
    m_nInitialMaxClusterSizeType = (TemporalSizeType)m_nMaxClusterSizeType;

    switch (eTemporalSizeType) {
       case PERCENTAGETYPE     : if (m_nMaxClusterSizeType == PERCENTAGETYPE)
                                   break;
                                 // convert from TIMETYPE to PERCENTAGETYPE
                                 // Since some variables are hard to accurately represent
                                 // as a double, we will cause variable to round up at some
                                 // fixed precision.
                                 dTemp = static_cast<double>(m_nMaxTemporalClusterSize)/dTimeBetween*100 * dPrecision;
                                 dTemp = ceil(dTemp);
                                 m_nMaxTemporalClusterSize = static_cast<float>(dTemp / dPrecision);
                                 break;
       case TIMETYPE           : if (m_nMaxClusterSizeType == TIMETYPE)
                                   break;
                                 // convert from PERCENTAGETYPE to TIMETYPE
                                 //m_nMaxTemporalClusterSize should be an integer from 1-90
                                 m_nMaxTemporalClusterSize = dTimeBetween * m_nMaxTemporalClusterSize/100;
                                 break;
       default                 : SSException::Generate("Unknown TemporalSizeType type %d", "ConvertMaxTemporalClusterSizeToType()", eTemporalSizeType);
    };
    m_nMaxClusterSizeType = eTemporalSizeType;
  }
  catch (ZdException & x) {
    x.AddCallpath("ConvertMaxTemporalClusterSizeToType()", "CParameters");
    throw;
  }
}

//------------------------------------------------------------------------------
// copies all class variables from the given CParameters object (rhs) into this one
//------------------------------------------------------------------------------
void CParameters::copy(const CParameters &rhs) {
    m_nNumEllipses = rhs.m_nNumEllipses;
    if (m_nNumEllipses > 0) {
       if (mp_dEShapes)
          delete [] mp_dEShapes;
       if (mp_nENumbers)
          delete [] mp_nENumbers;
       mp_dEShapes = new double[ m_nNumEllipses ];
       mp_nENumbers = new int[ m_nNumEllipses ];
       memcpy(mp_dEShapes, rhs.mp_dEShapes, (sizeof(double)*m_nNumEllipses));
       memcpy(mp_nENumbers, rhs.mp_nENumbers, (sizeof(int)*m_nNumEllipses));
    }
    else {
       mp_dEShapes = 0;
       mp_nENumbers = 0;
    }
    m_lTotalNumEllipses = rhs.m_lTotalNumEllipses;
    m_nAnalysisType     = rhs.m_nAnalysisType;
    m_nAreas            = rhs.m_nAreas;
    m_nModel            = rhs.m_nModel;
    m_nRiskFunctionType = rhs.m_nRiskFunctionType;
    m_nReplicas         = rhs.m_nReplicas;
    m_bPowerCalc        = rhs.m_bPowerCalc;
    m_nPower_X          = rhs.m_nPower_X;
    m_nPower_Y          = rhs.m_nPower_Y;
    strcpy(m_szStartDate, rhs.m_szStartDate);
    strcpy(m_szEndDate, rhs.m_szEndDate);
    m_nMaxGeographicClusterSize = rhs.m_nMaxGeographicClusterSize;
    m_nMaxTemporalClusterSize   = rhs.m_nMaxTemporalClusterSize;
    m_bAliveClustersOnly        = rhs.m_bAliveClustersOnly;
    m_nIntervalUnits            = rhs.m_nIntervalUnits;
    m_nIntervalLength           = rhs.m_nIntervalLength;
    m_nTimeAdjustType           = rhs.m_nTimeAdjustType;
    m_nTimeAdjPercent           = rhs.m_nTimeAdjPercent;
    m_bIncludePurelySpatial     = rhs.m_bIncludePurelySpatial;
    m_bIncludePurelyTemporal    = rhs.m_bIncludePurelyTemporal;
    strcpy(m_szCaseFilename,rhs.m_szCaseFilename);
    strcpy(m_szControlFilename,rhs.m_szControlFilename);
    strcpy(m_szPopFilename,rhs.m_szPopFilename);
    strcpy(m_szCoordFilename,rhs.m_szCoordFilename);
    strcpy(m_szGridFilename,rhs.m_szGridFilename);
    m_bSpecialGridFile = rhs.m_bSpecialGridFile;
    m_nPrecision                = rhs.m_nPrecision;
    m_nDimension                = rhs.m_nDimension;
    m_nCoordType                = rhs.m_nCoordType;
    strcpy(m_szOutputFilename,rhs.m_szOutputFilename);
    strcpy(m_szGISFilename,rhs.m_szGISFilename);
    strcpy(m_szLLRFilename,rhs.m_szLLRFilename);
    strcpy(m_szMLClusterFilename,rhs.m_szMLClusterFilename);
    strcpy(m_szRelRiskFilename, rhs.m_szRelRiskFilename);
    m_bSaveSimLogLikelihoods    = rhs.m_bSaveSimLogLikelihoods;
    m_bOutputRelRisks           = rhs.m_bOutputRelRisks;
    m_bSequential               = rhs.m_bSequential;
    m_nAnalysisTimes            = rhs.m_nAnalysisTimes;
    m_nCutOffPVal               = rhs.m_nCutOffPVal;
    m_bExactTimes               = rhs.m_bExactTimes;
    m_nClusterType              = rhs.m_nClusterType;
    m_bValidatePriorToCalc      = rhs.m_bValidatePriorToCalc;
    m_bDisplayErrors            = rhs.m_bDisplayErrors;

    strcpy(m_szProspStartDate, rhs.m_szProspStartDate);
    m_bOutputCensusAreas        = rhs.m_bOutputCensusAreas;
    m_bMostLikelyClusters       = rhs.m_bMostLikelyClusters;
    m_iCriteriaSecondClusters   = rhs.m_iCriteriaSecondClusters;

    m_nMaxClusterSizeType       = rhs.m_nMaxClusterSizeType;
    m_nMaxSpatialClusterSizeType = rhs.m_nMaxSpatialClusterSizeType;
    gbOutputClusterLevelDBF     = rhs.gbOutputClusterLevelDBF;
    gbOutputAreaSpecificDBF     = rhs.gbOutputAreaSpecificDBF;
    gbRelativeRiskDBF           = rhs.gbRelativeRiskDBF; 
    gbLogLikelihoodDBF          = rhs.gbLogLikelihoodDBF;
    gsRunHistoryFilename        = rhs.gsRunHistoryFilename;
}

void CParameters::DisplayAnalysisType(FILE* fp) {
   switch (m_nAnalysisType) {
      case PURELYSPATIAL  : fprintf(fp, "Purely Spatial analysis\n"); break;
      case PURELYTEMPORAL : fprintf(fp, "Purely Temporal analysis\n"); break;
      case SPACETIME      : fprintf(fp, "Retrospective Space-Time analysis\n"); break;
      case PROSPECTIVESPACETIME: fprintf(fp, "Prospective Space-Time analysis\n"); break;
   }

   fprintf(fp, "scanning for ");

   if (m_nRiskFunctionType==MONOTONERISK)
     fprintf(fp, "monotone ");

   fprintf(fp, "clusters with \n");

   switch (m_nAreas) {
     case (HIGH)      : fprintf(fp, "high rates"); break;
     case (LOW)       : fprintf(fp, "low rates"); break;
     case (HIGHANDLOW): fprintf(fp, "high or low rates"); break;
   }

   switch (m_nModel) {
      case POISSON :  fprintf(fp, " using the Poisson model.\n"); break;
      case BERNOULLI :  fprintf(fp, " using the Bernoulli model.\n"); break;
      case SPACETIMEPERMUTATION : fprintf(fp, " using the Space-Time Permutation model.\n"); break;
      default : fprintf(fp, " using unspecified model.\n"); break;
   }

   if ((m_nAnalysisType == SPACETIME) || (m_nAnalysisType == PROSPECTIVESPACETIME)) {
     if (m_bIncludePurelySpatial && m_bIncludePurelyTemporal)
       fprintf(fp, "Analysis includes purely spatial and purely temporal clusters.\n");
     else if (m_bIncludePurelySpatial)
       fprintf(fp, "Analysis includes purely spatial clusters.\n");
     else if (m_bIncludePurelyTemporal)
       fprintf(fp, "Analysis includes purely temporal clusters.\n");
   }

   if (m_bSequential)
     fprintf(fp, "Sequential analysis performed.\n");
}

bool CParameters::DisplayParamError(int nLine) {
  // do not throw an exception here...
  // want the entire parameter file verified...
  // do not want "the first" bad parameter thrown...
  //
  // app will throw an exception up return from the main verifyparameter function.
  if (m_bDisplayErrors) {
     gpPrintDirection->SatScanPrintWarning("  Error: Invalid value in parameter file, line %i.\n", nLine);
     gpPrintDirection->SatScanPrintWarning("    Variable:  %s", mgsVariableLabels[nLine - 1]);
  }
  return false;
}

void CParameters::DisplayParameters(FILE* fp) {
   ZdFileName   fileName(m_szOutputFilename);
   ZdString     sName;

   try {
     fprintf(fp, "\n________________________________________________________________\n\n");
     fprintf(fp, "PARAMETER SETTINGS\n\n");

     fprintf(fp, "Input Files\n");
     fprintf(fp, "-----------\n");

     fprintf(fp, "  Case File        : %s\n", m_szCaseFilename);
   
     if (m_nModel == POISSON || (m_nModel == SPACETIMEPERMUTATION && m_nMaxSpatialClusterSizeType == PERCENTAGEOFMEASURETYPE))
       fprintf(fp, "  Population File  : %s\n", m_szPopFilename);
     else if (m_nModel == BERNOULLI)
       fprintf(fp, "  Control File     : %s\n", m_szControlFilename);
   
     fprintf(fp, "  Coordinates File : %s\n", m_szCoordFilename);
     if (m_bSpecialGridFile)
       fprintf(fp, "  Special Grid File: %s\n", m_szGridFilename);

     fprintf(fp, "\n  Precision of Times : ");
     switch (m_nPrecision) {
       case NONE  : fprintf(fp, "None\n"); break;
       case YEAR  : fprintf(fp, "Years\n"); break;
       case MONTH : fprintf(fp, "Months\n"); break;
       case DAY   : fprintf(fp, "Days\n"); break;
     }
   
     fprintf(fp, "  Coordinates        : ");
     switch (m_nCoordType) {
       case CARTESIAN : fprintf(fp, "Cartesian\n"); break;
       case LATLON    : fprintf(fp, "Latitude/Longitude\n"); break;
     }
   
     fprintf(fp, "\nAnalysis\n");
     fprintf(fp, "--------\n");
   
     fprintf(fp, "  Type of Analysis    : ");
     switch (m_nAnalysisType) {
       case PURELYSPATIAL  : fprintf(fp, "Purely Spatial\n"); break;
       case PURELYTEMPORAL : fprintf(fp, "Purely Temporal\n"); break;
       case SPACETIME      : fprintf(fp, "Retrospective Space-Time\n"); break;
       case PROSPECTIVESPACETIME: fprintf(fp, "Prospective Space-Time\n"); break;
     }

     fprintf(fp, "  Probability Model   : ");
     switch (m_nModel) {
       case POISSON   : fprintf(fp, "Poisson\n"); break;
       case BERNOULLI : fprintf(fp, "Bernoulli\n"); break;
       case SPACETIMEPERMUTATION : fprintf(fp, "Space-Time Permutation\n"); break;
     }
   
     fprintf(fp, "  Scan for Areas with : ");
     switch (m_nAreas) {
       case HIGH       : fprintf(fp, "High Rates\n"); break;
       case LOW        : fprintf(fp, "Low Rates\n"); break;
       case HIGHANDLOW : fprintf(fp, "High or Low Rates\n"); break;
     }

     fprintf(fp, "\n  Start Date : %s\n", m_szStartDate);
     fprintf(fp, "  End Date   : %s\n\n", m_szEndDate);

     fprintf(fp, "  Number of Replications : %i\n", m_nReplicas);
   
     if (m_nNumEllipses > 0) {
        fprintf(fp, "\nEllipses\n");
        fprintf(fp, "----------\n");
        fprintf(fp, "Number of Ellipse Shapes Requested:  %i\n", m_nNumEllipses);
        fprintf(fp, "Shape for Each Ellipse:  ");
        for (int i = 0; i < m_nNumEllipses; ++i)
           fprintf(fp, "%7.3f ", mp_dEShapes[i]);
        fprintf(fp, "\nNumber of Angles for Each Ellipse Shape:  ");
        for (int i = 0; i < m_nNumEllipses; ++i)
           fprintf(fp, "%i ", mp_nENumbers[i]);
     }
     fprintf(fp, "\n\nScanning Window\n");
     fprintf(fp, "---------------\n");

     if (m_nAnalysisType == PURELYSPATIAL || m_nAnalysisType == SPACETIME || m_nAnalysisType == PROSPECTIVESPACETIME) {
       fprintf(fp, "  Maximum Spatial Cluster Size : %.2f", m_nMaxGeographicClusterSize);
       switch (m_nMaxSpatialClusterSizeType) {
          case    PERCENTAGEOFMEASURETYPE    : fprintf(fp, " %%\n"); break;
          case    DISTANCETYPE               : if (m_nCoordType == CARTESIAN)
                                                 fprintf(fp, " Cartesian Units\n");
                                               else
                                                 fprintf(fp, " km\n");
                                               break;
          default                            : fprintf(fp, "\n"); break;
       }
     }
     if ((m_nAnalysisType == SPACETIME) || (m_nAnalysisType == PROSPECTIVESPACETIME)) {
       fprintf(fp, "  Also Include Purely Temporal Clusters : ");
       switch (m_bIncludePurelyTemporal) {
         case true  : fprintf(fp, "Yes\n"); break;
         case false : fprintf(fp, "No\n");  break;
       }
     }

     if (m_nAnalysisType == PURELYTEMPORAL || m_nAnalysisType == SPACETIME || (m_nAnalysisType == PROSPECTIVESPACETIME)) {
       fprintf(fp, "  Maximum Temporal Cluster Size : %.2f", m_nInitialMaxTemporalClusterSize);
       switch (m_nInitialMaxClusterSizeType) {
         case    PERCENTAGETYPE : fprintf(fp, " %%\n"); break;
         case    TIMETYPE       : if (m_nIntervalUnits == YEAR)
                                    fprintf(fp, " Years\n");
                                  else if (m_nIntervalUnits == MONTH)
                                    fprintf(fp, " Months\n");
                                  else if (m_nIntervalUnits == DAY)
                                    fprintf(fp, " Days\n");
                                  else
                                    fprintf(fp, " None\n");
                                  break;
         default                : fprintf(fp, "\n"); break;
       }
     }

     if ((m_nAnalysisType == SPACETIME) || (m_nAnalysisType == PROSPECTIVESPACETIME)) {
       fprintf(fp, "  Also Include Purely Spatial Clusters : ");
       switch (m_bIncludePurelySpatial) {
         case true  : fprintf(fp, "Yes\n"); break;
         case false : fprintf(fp, "No\n");  break;
       }
     }

     //The "Clusters to Include" do not apply to PROSPECTIVESPACETIME
     if (m_nAnalysisType == PURELYTEMPORAL || m_nAnalysisType == SPACETIME)  {
       fprintf(fp, "  Clusters to Include : ");
       switch (m_bAliveClustersOnly) {
         case ALLCLUSTERS   : fprintf(fp, "All\n"); break;
         case ALIVECLUSTERS : fprintf(fp, "Only those including the study end date\n"); break;
       }
     }

     if (m_nAnalysisType == PURELYTEMPORAL || m_nAnalysisType == SPACETIME || (m_nAnalysisType == PROSPECTIVESPACETIME)) {
       fprintf(fp, "\nTime Parameters\n");
       fprintf(fp, "---------------\n");
   
       fprintf(fp, "\n  Time Interval Units  : ");
       switch (m_nIntervalUnits) {
         case 1 : fprintf(fp, "Years\n"); break;
         case 2 : fprintf(fp, "Months\n"); break;
         case 3 : fprintf(fp, "Days\n"); break;
       }
       fprintf(fp, "  Time Interval Length : %i\n", m_nIntervalLength);
   
       fprintf(fp, "\n  Adjustment for Time Trend : ");
       switch (m_nTimeAdjustType) {
         case NOTADJUSTED : fprintf(fp, "None\n"); break;
         case NONPARAMETRIC    : fprintf(fp, "Nonparametric\n"); break;
         case LINEAR  : fprintf(fp, "Linear with %0.2f%% per year\n", m_nTimeAdjPercent); break;
       }
     }
     if (m_nAnalysisType == PROSPECTIVESPACETIME)
        fprintf(fp, "  Prospective Start Date : %s\n", m_szProspStartDate);

     fprintf(fp, "\nOutput\n");
     fprintf(fp, "------\n");
#ifdef INCLUDE_RUN_HISTORY
     fprintf(fp, "  Run History File  : %s\n", gsRunHistoryFilename.GetCString());
#endif
     fprintf(fp, "  Results File      : %s\n", m_szOutputFilename);

     // gis files
     if (gbOutputAreaSpecificDBF) {
        sName = fileName.GetFullPath();
        if(strlen(fileName.GetExtension()) != 0)
           sName.Replace(fileName.GetExtension(), ".gis.dbf");
        else
           sName <<  ".gis.dbf";
        if (m_bOutputCensusAreas) {
           fprintf(fp, "  GIS File(s)       : %s\n", m_szGISFilename);
           fprintf(fp, "                    : %s\n", sName.GetCString());
        }
        else
           fprintf(fp, "  GIS File          : %s\n", sName.GetCString());
     }
     if (m_bOutputCensusAreas && !gbOutputAreaSpecificDBF)  // Output Census areas in Reported Clusters
        fprintf(fp, "  GIS File          : %s\n", m_szGISFilename);

     // mlc files
     if (gbOutputClusterLevelDBF) {
        sName = fileName.GetFullPath();
        if(strlen(fileName.GetExtension()) != 0)
           sName.Replace(fileName.GetExtension(), ".col.dbf");
        else
           sName << ".col.dbf";
        if (m_bMostLikelyClusters)  {  // Output Most Likely Cluster for each Centroid
           fprintf(fp, "  MLC File(s)       : %s\n", m_szMLClusterFilename);
           fprintf(fp, "                    : %s\n", sName.GetCString());
        }
        else
           fprintf(fp, "  MLC File          : %s\n", sName.GetCString());
     }
     if (m_bMostLikelyClusters && !gbOutputClusterLevelDBF)   // Output Most Likely Cluster for each Centroid
        fprintf(fp, "  MLC File          : %s\n", m_szMLClusterFilename);

        // RRE files
     if (gbRelativeRiskDBF) {
        sName = fileName.GetFullPath();
        if(strlen(fileName.GetExtension()) != 0)
           sName.Replace(fileName.GetExtension(), ".rr.dbf");
        else
           sName << ".rr.dbf";
        if (m_bOutputRelRisks){
           fprintf(fp, "  RRE File(s)       : %s\n", m_szRelRiskFilename);
           fprintf(fp, "                    : %s\n", sName.GetCString());
        }
        else
           fprintf(fp, "  RRE File          : %s\n", sName.GetCString());
     }
     if (m_bOutputRelRisks && !gbRelativeRiskDBF ) {
        fprintf(fp, "  RRE File          : %s\n", m_szRelRiskFilename);
     }

     // LLR Files
     if (gbLogLikelihoodDBF) {
        sName = fileName.GetFullPath();
        if(strlen(fileName.GetExtension()) != 0)
           sName.Replace(fileName.GetExtension(), ".llr.dbf");
        else
           sName << ".llr.dbf";
        if(m_bSaveSimLogLikelihoods) {
           fprintf(fp,  "  LLR File(s)       : %s\n", m_szLLRFilename);
           fprintf(fp,  "                    : %s\n", sName.GetCString());
        }
        else
           fprintf(fp,  "  LLR File          : %s\n", sName.GetCString());
     }
     if (m_bSaveSimLogLikelihoods && !gbLogLikelihoodDBF) {
       fprintf(fp,  "  LLR File          : %s\n", m_szLLRFilename);
     }

     fprintf(fp, "\n  Criteria for Reporting Secondary Clusters : ");
     switch (m_iCriteriaSecondClusters) {
        case NOGEOOVERLAP          : fprintf(fp, "No Geographical Overlap\n"); break;
        case NOCENTROIDSINOTHER    : fprintf(fp, "No Cluster Centroids in Other Clusters\n"); break;
        case NOCENTROIDSINMORELIKE : fprintf(fp, "No Cluster Centroids in More Likely Clusters\n"); break;
        case NOCENTROIDSINLESSLIKE : fprintf(fp, "No Cluster Centroids in Less Likely Clusters\n"); break;
        case NOPAIRSINEACHOTHERS   : fprintf(fp, "No Pairs of Centroids Both in Each Others Clusters\n"); break;
        case NORESTRICTIONS        : fprintf(fp, "No Restrictions = Most Likely Cluster for Each Centroid\n"); break;
     }
     fprintf(fp, "\n________________________________________________________________\n");
   }
   catch (ZdException & x) {
      x.AddCallpath("DisplayParameters(FILE *)", "CParameters");
      throw;
   }
}

void CParameters::DisplayTimeAdjustments(FILE* fp) {
   try {
      if (m_nTimeAdjustType == NOTADJUSTED)
        return;
    
      fprintf(fp, "Adjusted for time ");

      if (m_nTimeAdjustType == NONPARAMETRIC)
        fprintf(fp, "nonparametrically.\n");
      else if (m_nTimeAdjustType == LINEAR) {
        if (m_nTimeAdjPercent < 0)
          fprintf(fp, "with a decrease ");
        else
          fprintf(fp, "with an increase ");
        fprintf(fp, "of %0.2f%% per year.\n", fabs(m_nTimeAdjPercent));
      }
   }
   catch (ZdException & x) {
      x.AddCallpath("DisplayTimeAdjustments(FILE *)", "CParameters");
      throw;
   }
}

void CParameters::FindDelimiter(char *sString, char cDelimiter) {
   char  * psString;

   psString = sString;
   while (*psString != cDelimiter && *psString)
      ++psString;
   if (psString != sString)
      strcpy(sString, psString);
}

void CParameters::Free() {
   if (mp_dEShapes)
      delete [] mp_dEShapes;
   if (mp_nENumbers)
      delete [] mp_nENumbers;
}

// accessor function for the private variable gbOutputClusterLevelDBF
const bool CParameters::GetOutputClusterLevelDBF() const {
   return gbOutputClusterLevelDBF;
}

// accessor function for the private variable gbOutputAreaSpecificDBF
const bool CParameters::GetOutputAreaSpecificDBF() const {
   return gbOutputAreaSpecificDBF;
}

int CParameters::LoadEShapes(const char* szParam) {
   int          nScanCount, iLineLength;
   bool         bOk(true);
   char *       sTempLine=0;

   try {
      //Example is   "1.5 2"      DOUBLES
      if (m_nNumEllipses > 0) {
         iLineLength = strlen(szParam);
         if (iLineLength == 0)
            SSGenerateException("Parameter line for Ellipsoid shapes is empty","LoadEShapes");
         else {
            sTempLine = new char [ iLineLength + 2 ];
            strcpy(sTempLine, szParam);
         }
         if (mp_dEShapes)
            delete[] mp_dEShapes;
         mp_dEShapes = new double[ m_nNumEllipses ];
         for (int i = 0; (i < m_nNumEllipses) && bOk; ++i) {
            TrimLeft(sTempLine);
            nScanCount=sscanf(sTempLine, "%lf", &mp_dEShapes[i]);
            if (nScanCount == 1)
               FindDelimiter(sTempLine, ' ');
            else
               bOk = false;
         }
      }
      if ( ! bOk )
          SSGenerateException("Not enough ellipsoid shape specifications in parameter file","LoadEShapes");

      delete [] sTempLine;
   }
   catch (ZdException & x) {
      delete [] sTempLine;
      x.AddCallpath("LoadEShapes()", "CParameters");
      throw;
   }
   return 1;
}

int CParameters::LoadEAngles(const char* szParam) {
   int  nScanCount, iLineLength;
   bool bOk(true);
   char *sTempLine=0;

   try {
      //Example is   "1.5 2"      DOUBLES
      if (m_nNumEllipses > 0) {
         iLineLength = strlen(szParam);
         if (iLineLength == 0)
            SSGenerateException("Parameter line for Ellipsoid shapes is empty","LoadEAngles()");
         else {
            sTempLine = new char [ iLineLength + 2 ];
            strcpy(sTempLine, szParam);
         }
         if (mp_nENumbers)
            delete[] mp_nENumbers;
         mp_nENumbers = new int[ m_nNumEllipses ];
         for (int i = 0; (i < m_nNumEllipses) && bOk; ++i) {
            TrimLeft(sTempLine);
            nScanCount=sscanf(sTempLine, "%i", &mp_nENumbers[i]);
            if (nScanCount == 1)
               FindDelimiter(sTempLine, ' ');
            else
               bOk = false;
         }
         for (int i = 0; i < m_nNumEllipses; ++i)
           m_lTotalNumEllipses +=  mp_nENumbers[i];
      }
      if ( ! bOk )
          SSGenerateException("Not enough ellipsoid angle specifications in parameter file.","LoadEAngles()");

      delete [] sTempLine;
   }
   catch (ZdException & x) {
      delete [] sTempLine;
      x.AddCallpath("LoadEAngles()", "CParameters");
      throw;
   }
   return 1;
}

// sets the global ellipse variables read in from the ini file
// pre: file is an open ini parameter file
// post: will set the global variables from the ini file
void CParameters::ReadEllipseSectionFromIni(ZdIniFile& file) {
   ZdString  sShapes, sAngles;

   try {
      ZdIniSection* pSection = file.GetSection(ELLIPSES_SECTION);
      m_nNumEllipses = atoi(pSection->GetLine(pSection->FindKey(NUMBER_ELLIPSES_LINE))->GetValue());
      sShapes = pSection->GetLine(pSection->FindKey(ELLIPSE_SHAPES_LINE))->GetValue();
      SetEShapesFromIniFile(sShapes);
      sAngles = pSection->GetLine(pSection->FindKey(ELLIPSE_ANGLES_LINE))->GetValue();
      SetEAnglesFromIniFile(sAngles);
   }
   catch (ZdException &x) {
      x.AddCallpath("ReadEllipseSectionFromIni()", "CParameters");
      throw;
   }
}

// reads the parameters from the ini file
// pre: sFileName exists
// post: reads the parameters from the .ini file
void CParameters::ReadFromIniFile(ZdString sFileName) {
   try {
      // verify all the keys exist in the file so we can safely call the findkey() here without worrying
      // about it returning a -1  -- AJV 10/24/2002
      VerifyIniFileSetup(sFileName, true);
      ZdIniFile file(sFileName.GetCString());
      ReadInputFilesSectionFromIni(file);
      ReadModelInfoSectionFromIni(file);
      ReadSequentialScanSectionFromIni(file);
      ReadEllipseSectionFromIni(file);
      ReadOutputFileSectionFromIni(file);
   }
   catch (ZdException &x) {
      x.AddCallpath("ReadFromIniFile()", "CParameters");
      throw;
   }
}

// sets the global input file variables read in from the ini file
// pre: file is an open ini parameter file
// post: will set the global variables from the ini file
void CParameters::ReadInputFilesSectionFromIni(ZdIniFile& file){
   try {
      ZdIniSection* pSection = file.GetSection(INPUT_FILES_SECTION);
      strcpy(m_szCaseFilename, pSection->GetLine(pSection->FindKey(CASE_FILE_LINE))->GetValue());
      strcpy(m_szControlFilename, pSection->GetLine(pSection->FindKey(CONTROL_FILE_LINE))->GetValue());
      strcpy(m_szPopFilename, pSection->GetLine(pSection->FindKey(POP_FILE_LINE))->GetValue());
      strcpy(m_szCoordFilename, pSection->GetLine(pSection->FindKey(COORD_FILE_LINE))->GetValue());
      strcpy(m_szGridFilename, pSection->GetLine(pSection->FindKey(GRID_FILE_LINE))->GetValue());
      m_bSpecialGridFile = strlen(m_szGridFilename);
   }
   catch (ZdException &x) {
      x.AddCallpath("ReadInputFilesSectionFromIni", "CParameters");
      throw;
   }
}

// sets the global model info variables read in from the ini file
// pre: file is an open ini parameter file
// post: will set the global variables from the ini file
void CParameters::ReadModelInfoSectionFromIni(ZdIniFile& file) {
   try {
      ZdIniSection* pSection = file.GetSection(MODEL_INFO_SECTION);
      m_nAnalysisType = atoi(pSection->GetLine(pSection->FindKey(ANALYSIS_TYPE_LINE))->GetValue());
      m_nAreas = atoi(pSection->GetLine(pSection->FindKey(SCAN_AREAS_LINE))->GetValue());
      m_nPrecision = atoi(pSection->GetLine(pSection->FindKey(PRECISION_TIMES_LINE))->GetValue());
      m_nMaxGeographicClusterSize = atof(pSection->GetLine(pSection->FindKey(MAX_GEO_SIZE_LINE))->GetValue());
      strcpy(m_szStartDate, pSection->GetLine(pSection->FindKey(START_DATE_LINE))->GetValue());
      strcpy(m_szEndDate, pSection->GetLine(pSection->FindKey(END_DATE_LINE))->GetValue());
      m_bAliveClustersOnly  = ValueIsYes(pSection->GetLine(pSection->FindKey(ALIVE_CLUSTERS_LINE))->GetValue());
      m_nIntervalUnits = atoi(pSection->GetLine(pSection->FindKey(INTERVAL_UNITS_LINE))->GetValue());
      m_nIntervalLength = atol(pSection->GetLine(pSection->FindKey(INTERVAL_LENGTH_LINE))->GetValue());
      m_bIncludePurelySpatial = ValueIsYes(pSection->GetLine(pSection->FindKey(INCLUDE_PURELY_SPATIAL_LINE))->GetValue());
      m_nMaxTemporalClusterSize = atof(pSection->GetLine(pSection->FindKey(MAX_TEMP_SIZE_LINE))->GetValue());
      m_nReplicas = atoi(pSection->GetLine(pSection->FindKey(MONTE_CARLO_REPS_LINE))->GetValue());
      m_nModel = atoi(pSection->GetLine(pSection->FindKey(MODEL_TYPE_LINE))->GetValue());
      m_nRiskFunctionType = atoi(pSection->GetLine(pSection->FindKey(ISOTONIC_SCAN_LINE))->GetValue());
      m_bPowerCalc = ValueIsYes(pSection->GetLine(pSection->FindKey(PVALUE_PROSPECT_LLR_LINE))->GetValue());
      m_nPower_X = atof(pSection->GetLine(pSection->FindKey(LLR_1_LINE))->GetValue());
      m_nPower_Y = atof(pSection->GetLine(pSection->FindKey(LLR_2_LINE))->GetValue());
      m_nTimeAdjustType = atoi(pSection->GetLine(pSection->FindKey(TIME_TREND_ADJ_LINE))->GetValue());
      m_nTimeAdjPercent = atof(pSection->GetLine(pSection->FindKey(TIME_TREND_PERCENT_LINE))->GetValue());
      m_bIncludePurelyTemporal = ValueIsYes(pSection->GetLine(pSection->FindKey(INCLUDE_PURE_TEMP_LINE))->GetValue());
      m_nCoordType = atoi(pSection->GetLine(pSection->FindKey(COORD_TYPE_LINE))->GetValue());
      m_bValidatePriorToCalc = ValueIsYes(pSection->GetLine(pSection->FindKey(VALID_PARAMS_LINE))->GetValue());
      strcpy(m_szProspStartDate, pSection->GetLine(pSection->FindKey(PROSPECT_START_LINE))->GetValue());
      m_iCriteriaSecondClusters = atoi(pSection->GetLine(pSection->FindKey(CRIT_REPORT_SEC_CLUSTERS_LINE))->GetValue());
      m_nMaxClusterSizeType = atoi(pSection->GetLine(pSection->FindKey(MAX_TEMP_INTERPRET_LINE))->GetValue());
      m_nMaxSpatialClusterSizeType = atoi(pSection->GetLine(pSection->FindKey(MAX_SPATIAL_SIZE_LINE))->GetValue());
   }
   catch (ZdException &x) {
      x.AddCallpath("ReadModelInfoSectionFromIni()", "CParameters");
      throw;
   }
}

// sets the global output file variables read in from the ini file
// pre: file is an open ini parameter file
// post: will set the global variables from the ini file
void CParameters::ReadOutputFileSectionFromIni(ZdIniFile& file) {
   try {
      ZdIniSection* pSection = file.GetSection(OUTPUT_FILES_SECTION);
      strcpy(m_szOutputFilename, pSection->GetLine(pSection->FindKey(RESULTS_FILE_LINE))->GetValue());
      m_bSaveSimLogLikelihoods = ValueIsYes(pSection->GetLine(pSection->FindKey(SAVE_SIM_LLRS))->GetValue());
      m_bOutputCensusAreas = ValueIsYes(pSection->GetLine(pSection->FindKey(CENSUS_REPORT_CLUSTERS_LINE))->GetValue());
      m_bMostLikelyClusters = ValueIsYes(pSection->GetLine(pSection->FindKey(MOST_LIKELY_CLUSTER_LINE))->GetValue());
      gsRunHistoryFilename = pSection->GetLine(pSection->FindKey(ANALYSIS_HISTORY_LINE))->GetValue();
      gbOutputClusterLevelDBF = ValueIsYes(pSection->GetLine(pSection->FindKey(DBASE_CLUSTER_LINE))->GetValue());
      gbOutputAreaSpecificDBF = ValueIsYes(pSection->GetLine(pSection->FindKey(DBASE_AREA_LINE))->GetValue());
      m_bOutputRelRisks = ValueIsYes(pSection->GetLine(pSection->FindKey(INCLUDE_REL_RISKS_LINE))->GetValue());
      gbRelativeRiskDBF = ValueIsYes(pSection->GetLine(pSection->FindKey(DBASE_RELATIVE_RISKS))->GetValue());
      gbLogLikelihoodDBF = ValueIsYes(pSection->GetLine(pSection->FindKey(DBASE_LOG_LIKELI))->GetValue());
   }
   catch (ZdException &x) {
      x.AddCallpath("ReadOutputFileSectionFromIni()", "CParameters");
      throw;
   }
}

// sets the global sequential scan variables read in from the ini file
// pre: file is an open ini parameter file
// post: will set the global variables from the ini file
void CParameters::ReadSequentialScanSectionFromIni(ZdIniFile& file) {
   try {
      ZdIniSection* pSection = file.GetSection(SEQUENTIAL_SCAN_SECTION);
      m_bSequential = ValueIsYes(pSection->GetLine(pSection->FindKey(SEQUENTIAL_SCAN_LINE))->GetValue());
      m_nAnalysisTimes = atoi(pSection->GetLine(pSection->FindKey(SEQUENTIAL_MAX_ITERS_LINE))->GetValue());
      m_nCutOffPVal = atof(pSection->GetLine(pSection->FindKey(SEQUENTIAL_MAX_PVALUE_LINE))->GetValue());
   }
   catch (ZdException &x) {
      x.AddCallpath("ReadSequentialScanSectionFromIni()", "CParameters");
      throw;
   }
}


// saves the ellipse section to the ini file
// pre: file is an open ZdIniFile
// post: write the appropraite global data to the file to the appropraite keys
void CParameters::SaveEllipseSection(ZdIniFile& file) {
   ZdString  sShapes, sAngles;

   try {
      ZdIniSection* pSection = file.GetSection(ELLIPSES_SECTION);
      pSection->SetInt(NUMBER_ELLIPSES_LINE, m_nNumEllipses);
      for (int i = 0; i < m_nNumEllipses; ++i) {
         if(i == 0)
            sShapes << mp_dEShapes[i];
         else
            sShapes << "," << mp_dEShapes[i];
      }
      pSection->GetLine(pSection->FindKey(ELLIPSE_SHAPES_LINE))->SetValue(sShapes.GetCString());
      for (int i = 0; i < m_nNumEllipses; ++i) {
          if(i == 0)
             sAngles << mp_nENumbers[i];
          else
             sAngles << "," << mp_nENumbers[i];
      }
      pSection->GetLine(pSection->FindKey(ELLIPSE_ANGLES_LINE))->SetValue(sAngles.GetCString());
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
   try {
      ZdIniSection* pSection = file.GetSection(INPUT_FILES_SECTION);
      pSection->GetLine(pSection->FindKey(CASE_FILE_LINE))->SetValue(m_szCaseFilename);
      pSection->GetLine(pSection->FindKey(CONTROL_FILE_LINE))->SetValue(m_szControlFilename);
      pSection->GetLine(pSection->FindKey(POP_FILE_LINE))->SetValue(m_szPopFilename);
      pSection->GetLine(pSection->FindKey(COORD_FILE_LINE))->SetValue(m_szCoordFilename);
      pSection->GetLine(pSection->FindKey(GRID_FILE_LINE))->SetValue(m_szGridFilename);
   }
   catch (ZdException &x) {
      x.AddCallpath("SaveInputFileSection()", "CParameters");
      throw;
   }
}

// saves the model info section to the ini file
// pre: file is an open ZdIniFile
// post: write the appropraite global data to the file to the appropraite keys
void CParameters::SaveModelInfoSection(ZdIniFile& file) {
   ZdString     sFloatValue;

   try {
      ZdIniSection* pSection = file.GetSection(MODEL_INFO_SECTION);
      pSection->SetInt(ANALYSIS_TYPE_LINE, m_nAnalysisType);
      pSection->SetInt(MODEL_TYPE_LINE, m_nModel);
      pSection->SetInt(SCAN_AREAS_LINE, m_nAreas);
      pSection->GetLine(pSection->FindKey(START_DATE_LINE))->SetValue(m_szStartDate);
      pSection->GetLine(pSection->FindKey(END_DATE_LINE))->SetValue(m_szEndDate);
      pSection->SetInt(PRECISION_TIMES_LINE, m_nPrecision);
      pSection->SetInt(MONTE_CARLO_REPS_LINE, m_nReplicas);
      sFloatValue = m_nMaxGeographicClusterSize;
      pSection->SetString(MAX_GEO_SIZE_LINE, sFloatValue.GetCString());
      sFloatValue = m_nMaxTemporalClusterSize;
      pSection->SetString(MAX_TEMP_SIZE_LINE, sFloatValue.GetCString());
      pSection->SetInt(INTERVAL_LENGTH_LINE, m_nIntervalLength);
      pSection->SetInt(INTERVAL_UNITS_LINE, m_nIntervalUnits);
      pSection->SetInt(COORD_TYPE_LINE, m_nCoordType);
      pSection->SetString(ALIVE_CLUSTERS_LINE, m_bAliveClustersOnly ? YES : NO );
      pSection->SetString(INCLUDE_PURELY_SPATIAL_LINE, m_bIncludePurelySpatial ? YES : NO);
      pSection->SetString(INCLUDE_PURE_TEMP_LINE, m_bIncludePurelyTemporal ? YES : NO);
      pSection->SetString(ISOTONIC_SCAN_LINE, (m_nRiskFunctionType == MONOTONERISK) ? YES : NO);
      pSection->SetString(PVALUE_PROSPECT_LLR_LINE, m_bPowerCalc ? YES : NO);
      sFloatValue = m_nPower_X;
      pSection->SetString(LLR_1_LINE, sFloatValue.GetCString());
      sFloatValue = m_nPower_Y;
      pSection->SetString(LLR_2_LINE, sFloatValue.GetCString());
      pSection->SetInt(TIME_TREND_ADJ_LINE, m_nTimeAdjustType);
      sFloatValue = m_nTimeAdjPercent;
      pSection->SetString(TIME_TREND_PERCENT_LINE, sFloatValue.GetCString());
      pSection->SetString(VALID_PARAMS_LINE, m_bValidatePriorToCalc ? YES : NO);
      pSection->GetLine(pSection->FindKey(PROSPECT_START_LINE))->SetValue(m_szProspStartDate);
      pSection->SetInt(CRIT_REPORT_SEC_CLUSTERS_LINE, m_iCriteriaSecondClusters);
      pSection->SetInt(MAX_TEMP_INTERPRET_LINE, m_nMaxClusterSizeType);
      pSection->SetInt(MAX_SPATIAL_SIZE_LINE, m_nMaxSpatialClusterSizeType);
   }
   catch (ZdException &x) {
      x.AddCallpath("SaveModelInfoSection()", "CParameters");
      throw;
   }
}

// saves the output file section to the ini file
// pre: file is an open ZdIniFile
// post: write the appropraite global data to the file to the appropraite keys
void CParameters::SaveOutputFileSection(ZdIniFile& file) {
   try {
      ZdIniSection* pSection = file.GetSection(OUTPUT_FILES_SECTION);
      pSection->GetLine(pSection->FindKey(RESULTS_FILE_LINE))->SetValue(m_szOutputFilename);
      pSection->GetLine(pSection->FindKey(ANALYSIS_HISTORY_LINE))->SetValue(gsRunHistoryFilename.GetCString());
      pSection->SetString(CENSUS_REPORT_CLUSTERS_LINE, m_bOutputCensusAreas ? YES : NO);
      pSection->SetString(DBASE_CLUSTER_LINE, gbOutputClusterLevelDBF ? YES : NO);
      pSection->SetString(MOST_LIKELY_CLUSTER_LINE, m_bMostLikelyClusters ? YES : NO);
      pSection->SetString(DBASE_AREA_LINE, gbOutputAreaSpecificDBF ? YES : NO);
      pSection->SetString(INCLUDE_REL_RISKS_LINE, m_bOutputRelRisks ? YES : NO);
      pSection->SetString(DBASE_RELATIVE_RISKS, gbRelativeRiskDBF ? YES : NO);
      pSection->SetString(SAVE_SIM_LLRS, m_bSaveSimLogLikelihoods ? YES : NO);
      pSection->SetString(DBASE_LOG_LIKELI, gbLogLikelihoodDBF ? YES : NO);
   }
   catch (ZdException &x) {
      x.AddCallpath("SaveOutputFileSection()", "CParameters");
      throw;
   }
}

// old version of saving parameters to the file, deprecated and replaced by SaveToIniFile()
// pre: szFilename is name of parameter file
// post: prints out the parameters in unique lines in a specified order to a file
bool CParameters::SaveParameters(char* szFilename) {
   try {
      SaveToIniFile(szFilename);
   }
   catch (ZdException & x) {
      x.AddCallpath("SaveParameters(char *)", "CParameters");
      throw;
   }
   return true;
}

// saves the output file section to the ini file
// pre: file is an open ZdIniFile
// post: write the appropraite global data to the file to the appropraite keys
void CParameters::SaveSequentialScanSection(ZdIniFile& file) {
   ZdString  sFloatValue;

   try {
      ZdIniSection* pSection = file.GetSection(SEQUENTIAL_SCAN_SECTION);
      pSection->SetString(SEQUENTIAL_SCAN_LINE, m_bSequential ? YES : NO);
      pSection->SetInt(SEQUENTIAL_MAX_ITERS_LINE, m_nAnalysisTimes);
      sFloatValue = m_nCutOffPVal;
      pSection->SetString(SEQUENTIAL_MAX_PVALUE_LINE, sFloatValue.GetCString());
   }
   catch (ZdException &x) {
      x.AddCallpath("SaveSequentialScanSection()", "CParameters");
      throw;
   }
}

// saves the parameters to an .ini file and replaces the existing ini if necessary
// pre: sFileName is the name of the .prm parameter file
// post: saves the parameters to an .ini file
void CParameters::SaveToIniFile(ZdString sFileName) {
   try {
      VerifyIniFileSetup(sFileName, true);

      ZdIniFile file(sFileName.GetCString());
      SaveInputFileSection(file);
      SaveModelInfoSection(file);
      SaveEllipseSection(file);
      SaveSequentialScanSection(file);
      SaveOutputFileSection(file);

      file.Write();
   }
   catch (ZdException &x) {
      x.AddCallpath("SaveToIniFile()", "CParameters");
      throw;
   }
}

// initializes global variables to default values
void CParameters::SetDefaults() {
  m_nAnalysisType             = PURELYSPATIAL;
  m_nAreas                    = HIGH;
  strcpy(m_szCaseFilename, "");
  strcpy(m_szPopFilename, "");
  strcpy(m_szCoordFilename, "");
  strcpy(m_szOutputFilename, "");
  m_nPrecision                = YEAR;
  m_nDimension                = 0;
  m_bSpecialGridFile          = false;
  strcpy(m_szGridFilename, "");
  m_nMaxGeographicClusterSize = 50.0; //GG980716
  m_nMaxSpatialClusterSizeType = PERCENTAGEOFMEASURETYPE;
  strcpy(m_szStartDate, "1900/01/01");
  strcpy(m_szEndDate, "1900/12/31");
  m_bAliveClustersOnly        = false;
  m_bExactTimes               = false;
  m_nIntervalUnits            = 0;
  m_nIntervalLength           = 0;
  m_bIncludePurelySpatial     = false;
  m_nMaxTemporalClusterSize   = 50.0;//KR980707 0 GG980716;
  m_nMaxClusterSizeType = PERCENTAGETYPE;
  m_nReplicas                 = 999;

  gbOutputClusterLevelDBF     = false;
  gbOutputAreaSpecificDBF     = false;
  gbRelativeRiskDBF           = false;
  gbLogLikelihoodDBF          = false;
  gsRunHistoryFilename        = "";

  SetDefaultsV2();
}

void CParameters::SetDefaultsV2() {
  m_nModel                 = POISSON;
  m_nRiskFunctionType      = STANDARDRISK;
  m_bPowerCalc             = false;
  m_nPower_X               = 0.0;
  m_nPower_Y               = 0.0;
  m_nTimeAdjustType        = NOTADJUSTED;
  m_nTimeAdjPercent        = 0;
  m_bIncludePurelyTemporal = false;
  strcpy(m_szControlFilename, "");
  m_nCoordType             = CARTESIAN;
  m_bSaveSimLogLikelihoods = false;
  m_bSequential            = false;
  m_nAnalysisTimes         = 0;
  m_nCutOffPVal            = 0.0;
  m_bValidatePriorToCalc   = true;
  m_bOutputRelRisks        = false;
  m_nNumEllipses           = 0;
  mp_dEShapes              = 0;
  mp_nENumbers             = 0;
  //m_nExtraParam4           = 0;
  strcpy(m_szProspStartDate, "");
  m_bOutputCensusAreas     = false;
  m_bMostLikelyClusters    = false;
  m_iCriteriaSecondClusters = 0;
  m_lTotalNumEllipses      = 0;

   //need to convert old parameter analysis settings to new ones..
  if (m_nAnalysisType == 2) m_nAnalysisType = 3;
  else if (m_nAnalysisType == 3) m_nAnalysisType = 2;
}

//**************************************************************************
// THIS ASSUMES THAT IT IS AN OLD SESSION TYPE...
// IF NUMBER OF ELLIPSES IS GREATER THAN ZERO, THEN SOMEONE HAS ALTERED
// AN OLD SESSION TO RUN WITH ELLIPSES AND IT SHOULD WORK WITH THEM...
//
//  SO IF NUMELLIPSES > 0, THEN LEAVE SHAPES, NUMBERS, TOTAL ALONE
//**************************************************************************
void CParameters::SetDefaultsV3() {
  if (m_nNumEllipses == 0) {
     //m_nNumEllipses           = 0;
     mp_dEShapes              = 0;
     mp_nENumbers             = 0;
     m_lTotalNumEllipses      = 0;
     m_iCriteriaSecondClusters = 0;
  }
  // someone has set up some old session to run with ellipses...
  else {
     // must set criteria for ellipses to no restrictions....
     m_iCriteriaSecondClusters = 5;
  }
  strcpy(m_szProspStartDate, "");
  m_bOutputCensusAreas     = false;
  m_bMostLikelyClusters    = false;

  //need to convert old parameter analysis settings to new ones..
  if (m_nAnalysisType == 2) m_nAnalysisType = 3;
  else if (m_nAnalysisType == 3) m_nAnalysisType = 2;
}

void CParameters::SetDisplayParameters(bool bValue) {
   m_bDisplayErrors = bValue;
}

// sets up the array of ellipse angles from the comma delimited string stored in the ini file
// pre : sAngles is the comma delimited line from the ini file
// post: allocates and assigns to the global array mp_dNumbers
void CParameters::SetEAnglesFromIniFile(const ZdString& sAngles) {
   ZdString     sTempAngles;

   try {
      for (int i = 0; i < m_nNumEllipses; ++i) {
          ZdStringTokenizer angleTokenizer(sAngles, ",");
          while(angleTokenizer.HasMoreTokens()) {
             sTempAngles << angleTokenizer.GetNextToken() << " ";
          }
          LoadEAngles(sTempAngles.GetCString());
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("SetEAnglesFromIniFile()", "CParameters");
      throw;
   }
}

// sets up the array of ellipse shapes from the comma delimited string stored in the ini file
// pre : sShapes is the comma delimited line from the ini file
// post: allocates and assigns to the global array mp_dEShapes
void CParameters::SetEShapesFromIniFile(const ZdString& sShapes) {
   ZdString     sTempShapes;

   try {
      for (int i = 0; i < m_nNumEllipses; ++i) {
         ZdStringTokenizer shapeTokenizer(sShapes, ",");
         while(shapeTokenizer.HasMoreTokens()) {
            sTempShapes << shapeTokenizer.GetNextToken() << " ";
         }
         LoadEShapes(sTempShapes.GetCString());
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("SetEShapesFromIniFile()", "CParameters");
      throw;
   }
}

bool CParameters::SetGISFilename() {
   int          nReportNameLen = strlen(m_szOutputFilename);
   int          nIndex = nReportNameLen-1;
   bool         bDone = false, bExtFound = false, bReturnValue = true;

   try {
      while (!bDone && !bExtFound && nIndex>=0) {
        if (m_szOutputFilename[nIndex]=='/' || m_szOutputFilename[nIndex]=='\\')
          bDone=true;
        else if (m_szOutputFilename[nIndex]=='.')
          bExtFound=true;
        else
          --nIndex;
      }

      if (!bExtFound)
        nIndex = nReportNameLen;

      strncpy(m_szGISFilename, m_szOutputFilename, nIndex);
      strcpy(m_szGISFilename+nIndex, ".gis.txt");

      if (strcmp(m_szGISFilename, m_szOutputFilename)==0)
         bReturnValue = false;
   }
   catch (ZdException & x) {
      x.AddCallpath("SetGISFileName()", "CParameters");
      throw;
   }
   return bReturnValue;
}

bool CParameters::SetLLRFilename() {
   int          nReportNameLen = strlen(m_szOutputFilename);
   int          nIndex = nReportNameLen-1;
   bool         bDone = false, bExtFound = false, bReturnValue = true;

   try {
      while (!bDone && !bExtFound && nIndex>=0) {
         if (m_szOutputFilename[nIndex] == '/' || m_szOutputFilename[nIndex] == '\\')
           bDone = true;
         else if (m_szOutputFilename[nIndex] == '.')
           bExtFound = true;
         else
           --nIndex;
       }

      if (!bExtFound)
        nIndex = nReportNameLen;

      strncpy(m_szLLRFilename, m_szOutputFilename, nIndex);
      strcpy(m_szLLRFilename + nIndex, ".llr.txt");


      if (strcmp(m_szLLRFilename, m_szOutputFilename) == 0)
         bReturnValue = false;
   }
   catch (ZdException & x)  {
      x.AddCallpath("SetLLRFileName()", "CParameters");
      throw;
   }
   return bReturnValue;
}

//most likely cluster for each centroid - file name
bool CParameters::SetMLCFilename() {
   int          nReportNameLen = strlen(m_szOutputFilename);
   int          nIndex         = nReportNameLen-1;
   bool         bDone = false, bExtFound = false, bReturnValue = true;

   try {
      while (!bDone && !bExtFound && nIndex>=0) {
        if (m_szOutputFilename[nIndex]=='/' || m_szOutputFilename[nIndex]=='\\')
          bDone=true;
        else if (m_szOutputFilename[nIndex]=='.')
          bExtFound=true;
        else
          --nIndex;
      }

      if (!bExtFound)
        nIndex = nReportNameLen;

      strncpy(m_szMLClusterFilename, m_szOutputFilename, nIndex);
      strcpy(m_szMLClusterFilename+nIndex, ".col.txt");

      if (strcmp(m_szMLClusterFilename, m_szOutputFilename)==0)
         bReturnValue = false;
   }
   catch (ZdException & x) {
      x.AddCallpath("SetMLCFilename()", "CParameters");
      throw;
   }
   return bReturnValue;
}

// old method of setting the parameters from the old parameter file
bool CParameters::SetParameter(int nParam, const char* szParam) {
   bool bValid = false;
   int  nScanCount, nTemp;
   char sTemp[MAX_STR_LEN];

   try {
      switch (nParam) {
        case ANALYSISTYPE : nScanCount=sscanf(szParam, "%i", &m_nAnalysisType); break;
        case SCANAREAS    : nScanCount=sscanf(szParam, "%i", &m_nAreas); break;
        case CASEFILE     : strncpy(m_szCaseFilename, szParam, strlen(szParam)-1);
                            m_szCaseFilename[strlen(szParam)-1]='\0'; nScanCount=1; break;
        case POPFILE      : strncpy(m_szPopFilename, szParam, strlen(szParam)-1);
                            m_szPopFilename[strlen(szParam)-1]='\0'; nScanCount=1; break;
        case COORDFILE    : strncpy(m_szCoordFilename, szParam, strlen(szParam)-1);
                            m_szCoordFilename[strlen(szParam)-1]='\0'; nScanCount=1; break;
        case OUTPUTFILE   : strncpy(m_szOutputFilename, szParam, strlen(szParam)-1);
                            m_szOutputFilename[strlen(szParam)-1]='\0'; nScanCount=1; break;
        case PRECISION    : nScanCount=sscanf(szParam, "%i", &m_nPrecision); break;
        case DIMENSION    : nScanCount=sscanf(szParam, "%i", &m_nDimension); break;
        case SPECIALGRID  : nScanCount=sscanf(szParam, "%i", &nTemp);
                            m_bSpecialGridFile = (nTemp ? true : false);
                            break;
        case GRIDFILE     : strncpy(m_szGridFilename, szParam, strlen(szParam)-1);
                            m_szGridFilename[strlen(szParam)-1]='\0'; nScanCount=1; break;
        case GEOSIZE      : nScanCount=sscanf(szParam, "%f", &m_nMaxGeographicClusterSize); break;
        case STARTDATE    : nScanCount=sscanf(szParam, "%s", &m_szStartDate); break;
        case ENDDATE      : nScanCount=sscanf(szParam, "%s", &m_szEndDate); break;
        case CLUSTERS     : nScanCount=sscanf(szParam, "%i", &nTemp);
                            m_bAliveClustersOnly = (nTemp ? true : false);
                            break;
        case EXACTTIMES   : nScanCount=sscanf(szParam, "%i", &nTemp);
                            m_bExactTimes = (nTemp ? true : false);
                            break;
        case INTERVALUNITS: nScanCount=sscanf(szParam, "%i", &m_nIntervalUnits); break;
        case TIMEINTLEN   : nScanCount=sscanf(szParam, "%i", &m_nIntervalLength); break;
        case PURESPATIAL  : nScanCount=sscanf(szParam, "%i", &nTemp);
                            m_bIncludePurelySpatial = (nTemp ? true : false);
                            break;
        case TIMESIZE     : nScanCount=sscanf(szParam, "%f", &m_nMaxTemporalClusterSize); break;
        case REPLICAS     : nScanCount=sscanf(szParam, "%i", &m_nReplicas); break;
        case MODEL        : nScanCount=sscanf(szParam, "%i", &m_nModel); break;
        case RISKFUNCTION : nScanCount=sscanf(szParam, "%i", &m_nRiskFunctionType); break;
        case POWERCALC    : nScanCount=sscanf(szParam, "%i", &nTemp);
                            m_bPowerCalc = ( nTemp ? true : false);
                            break;
        case POWERX       : nScanCount=sscanf(szParam, "%lf", &m_nPower_X); break;
        case POWERY       : nScanCount=sscanf(szParam, "%lf", &m_nPower_Y); break;
        case TIMETREND    : nScanCount=sscanf(szParam, "%i", &m_nTimeAdjustType); break;
        case TIMETRENDPERC: nScanCount=sscanf(szParam, "%lf", &m_nTimeAdjPercent); break;
        case PURETEMPORAL : nScanCount=sscanf(szParam, "%i", &nTemp);
                            m_bIncludePurelyTemporal = (nTemp ? true : false);
                            break;
        case CONTROLFILE  : strncpy(m_szControlFilename, szParam, strlen(szParam)-1);
                            m_szControlFilename[strlen(szParam)-1]='\0'; nScanCount=1; break;
        case COORDTYPE    : nScanCount=sscanf(szParam, "%i", &m_nCoordType); break;
        case SAVESIMLL    : nScanCount=sscanf(szParam, "%i", &nTemp);
                            m_bSaveSimLogLikelihoods = (nTemp ? true : false);
                            break;
        case SEQUENTIAL   : nScanCount=sscanf(szParam, "%i", &nTemp);
                            m_bSequential = (nTemp ? true : false);
                            break;
        case SEQNUM       : nScanCount=sscanf(szParam, "%i", &m_nAnalysisTimes); break;
        case SEQPVAL      : nScanCount=sscanf(szParam, "%lf", &m_nCutOffPVal); break;
        case VALIDATE     : nScanCount=sscanf(szParam, "%i", &nTemp);
                            m_bValidatePriorToCalc = (nTemp ? true : false);
                            break;
        case OUTPUTRR     : nScanCount=sscanf(szParam, "%i", &nTemp);
                            m_bOutputRelRisks = (nTemp ? true : false);
                            break;
        case ELLIPSES     : nScanCount=sscanf(szParam, "%i", &m_nNumEllipses); break;
        case ESHAPES      : nScanCount = LoadEShapes(szParam); break;
        case ENUMBERS     : nScanCount = LoadEAngles(szParam); break;
        case START_PROSP_SURV:
              nScanCount=sscanf(szParam, "%s", &m_szProspStartDate);
              if ((nScanCount == 0) || (strlen(m_szProspStartDate)==1))  strcpy(m_szProspStartDate, "");
              break;
        case OUTPUT_CENSUS_AREAS:  nScanCount=sscanf(szParam, "%i", &nTemp);
                                   m_bOutputCensusAreas = (nTemp ? true : false);
                                   break;
        case OUTPUT_MOST_LIKE_CLUSTERS: nScanCount=sscanf(szParam, "%i", &nTemp);
                                        m_bMostLikelyClusters = (nTemp ? true : false);
                                        break;
        case CRITERIA_SECOND_CLUSTERS: nScanCount=sscanf(szParam, "%i", &m_iCriteriaSecondClusters); break;
        case MAX_TEMPORAL_TYPE: nScanCount=sscanf(szParam, "%i", &m_nMaxClusterSizeType); break;
        case MAX_SPATIAL_TYPE: nScanCount=sscanf(szParam, "%i", &m_nMaxSpatialClusterSizeType); break;
        case RUN_HISTORY_FILENAME: strncpy(sTemp, szParam, strlen(szParam)-1);
                                   sTemp[strlen(szParam)-1]='\0';
                                   nScanCount=1;
                                   gsRunHistoryFilename = sTemp;
                                   break;
        case OUTPUTCLUSTERDBF: nScanCount=sscanf(szParam, "%i", &nTemp);
                               gbOutputClusterLevelDBF = (nTemp ? true : false);
                               break;
        case OUTPUTAREADBF: nScanCount=sscanf(szParam, "%i", &nTemp);
                            gbOutputAreaSpecificDBF = (nTemp ? true : false);
                            break;
        case RELATIVE_RISK_DBF: nScanCount=sscanf(szParam, "%i", &nTemp);
                                gbRelativeRiskDBF = (nTemp ? true : false);
                                break;
        case LOG_LIKELI_DBF:  nScanCount=sscanf(szParam, "%i", &nTemp);
                              gbLogLikelihoodDBF = (nTemp ? true : false);
                              break;
      }

      if (nParam==POPFILE || nParam==GRIDFILE || nParam==CONTROLFILE)
        bValid = true;
      else {
        // Need to be able to handle old parameter files  !!!!!!
        bValid = (nScanCount == 1);
        if (!bValid)
          DisplayParamError(nParam);
      }
   }
   catch (ZdException & x) {
      x.AddCallpath("SetParameter()", "CParameters");
      throw;
   }
   return bValid;
}

// function to set the parameters from the file, uses both new and old style parameter files
// pre: szFilename is name of paramter file, validate indicates whether or not parameters should be validated after read in
// post: reads in parameters from the file and validates if requested
bool CParameters::SetParameters(const char* szFilename, bool bValidate) {
   FILE* pFile;
   char  szTemp [MAX_STR_LEN];
   bool  bValid = true, bEOF = false;
   int  i    = 1;

   try {
      if ((pFile = fopen(szFilename, "r")) == NULL)
         SSGenerateException("  Error: Unable to open parameter file.", "SetParameters()");

      // set defaults for newer parameters -- this section of code, in fact this whole unit,
      // needs to be revised. Backward compatibility may be a thorn in our side.
      m_nMaxClusterSizeType = PERCENTAGETYPE;
      m_nMaxSpatialClusterSizeType = PERCENTAGEOFMEASURETYPE;
      gsRunHistoryFilename << ZdString::reset;
      gbOutputClusterLevelDBF   = false;
      gbOutputAreaSpecificDBF   = false;
      gbRelativeRiskDBF         = false;
      gbLogLikelihoodDBF        = false;

      ZdIniFile file(szFilename);
      if (file.GetNumSections()) {
         fclose(pFile);
         ReadFromIniFile(szFilename);
      }
      else {
         while (i<=PARAMETERS && !bEOF) {
            if (fgets(szTemp, MAX_STR_LEN, pFile) == NULL)
               bEOF   = true;
            else if (!SetParameter(i, szTemp))
               bValid = false;
            ++i;
         }
         fclose(pFile);
      }

      if (bEOF && ((i-1)==MODEL))              // Accept V.1 parameter files
        SetDefaultsV2();
      else if (bEOF && ((i-2) < CRITERIA_SECOND_CLUSTERS))      // Accept V.1.3 parameter files
         SetDefaultsV3();
      //else if ( (bEOF && (i-2) < MAX_TEMPORAL_TYPE) || (!bEOF && (i-1) < MAX_TEMPORAL_TYPE ) )
      //   m_nMaxClusterSizeType = PERCENTAGETYPE;

      if (!SetGISFilename() || !SetLLRFilename() || !SetMLCFilename() || !SetRelRiskFilename())
        bValid = false;

      if (bValid && bValidate)
        bValid = ValidateParameters();
   }
   catch (ZdException & x) {         
      fclose(pFile);
      x.AddCallpath("SetParameters()", "CParameters");
      throw;
   }
   return bValid;
}

// sets the global print direction pointer
void CParameters::SetPrintDirection(BasePrint *pPrintDirection) {
   gpPrintDirection = pPrintDirection;
}

//set rel risk estimate file name
bool CParameters::SetRelRiskFilename() {
   int          nReportNameLen = strlen(m_szOutputFilename);
   int          nIndex = nReportNameLen-1;
   bool         bDone = false, bExtFound = false, bReturnValue = true;

   try {
      while (!bDone && !bExtFound && nIndex>=0) {
        if (m_szOutputFilename[nIndex]=='/' || m_szOutputFilename[nIndex]=='\\')
          bDone=true;
        else if (m_szOutputFilename[nIndex]=='.')
          bExtFound=true;
        else
          --nIndex;
      }

      if (!bExtFound)
        nIndex = nReportNameLen;

      strncpy(m_szRelRiskFilename, m_szOutputFilename, nIndex);
      strcpy(m_szRelRiskFilename+nIndex, ".rr.txt");

      if (strcmp(m_szRelRiskFilename, m_szOutputFilename)==0)
         bReturnValue = false;
   }
   catch (ZdException & x) {
      x.AddCallpath("SetRelRiskFilename()", "CParameters");
      throw;
   }
   return bReturnValue;
}
// trims the left whitespace from the char string
void CParameters::TrimLeft(char *sString) {
   char  * psString;

   psString = sString;
   while (*psString <= ' ' && *psString)
      ++psString;
   if (psString != sString)
      strcpy(sString, psString);
}

bool CParameters::ValidateParameters() {
   bool         bValid = true, bValidDate = true;
   FILE*        pFile;

   try {
      if (m_bValidatePriorToCalc) {
        if (!(PURELYSPATIAL <= m_nAnalysisType && m_nAnalysisType <= PROSPECTIVESPACETIME))   // use to be <= PURELYTEMPORAL
          bValid = DisplayParamError(ANALYSISTYPE);

        if (!(HIGH <= m_nAreas && m_nAreas<= HIGHANDLOW))
          bValid = DisplayParamError(SCANAREAS);

        if (!(POISSON == m_nModel || m_nModel == BERNOULLI || m_nModel == SPACETIMEPERMUTATION))
          bValid = DisplayParamError(MODEL);

        if (!(STANDARDRISK == m_nRiskFunctionType || m_nRiskFunctionType == MONOTONERISK))
          bValid = DisplayParamError(RISKFUNCTION);

        if (m_bSequential) {
          //if (!((1 <= m_nAnalysisTimes) && (m_nAnalysisTimes <= INT_MAX)))
          if (!(1 <= m_nAnalysisTimes))
            bValid = DisplayParamError(SEQNUM);
          if (!(0.0 <= m_nCutOffPVal && m_nCutOffPVal <= 1.0))
            bValid = DisplayParamError(SEQPVAL);
        }
        else {
          m_nAnalysisTimes = 0;
          m_nCutOffPVal    = 0.0;
        }

        if (m_bPowerCalc) {
          if (!(0.0 <= m_nPower_X && m_nPower_X<= DBL_MAX))
            bValid = DisplayParamError(POWERX);
          if (!(0.0 <= m_nPower_Y && m_nPower_Y <= DBL_MAX))
            bValid = DisplayParamError(POWERY);
        }
        else {
          m_nPower_X = 0.0;
          m_nPower_Y = 0.0;
        }

        if (!(CARTESIAN == m_nCoordType || m_nCoordType == LATLON))
          bValid = DisplayParamError(COORDTYPE);
        else if ((m_nCoordType == LATLON) && (m_nNumEllipses > 0))
          bValid = DisplayParamError(ELLIPSES);                                  // Could do a better job of displaying messages here...  just shows line number and thats all...

        // If number of ellipsoids > 0, then criteria for reporting secondary clusters must be "No Restrictions"
        if ((m_nNumEllipses > 0) && (m_iCriteriaSecondClusters != 5))
           SSGenerateException("  Error: Number of Ellipsiods is greater than zero and Criteria for Secondary Clusters is NOT set to No Restrictions.", "ValidateParameters()");

        if (!ValidateReplications(m_nReplicas))
          bValid = DisplayParamError(REPLICAS);

        if (m_nAnalysisType == PURELYSPATIAL) {
          if (!(NONE <= m_nPrecision && m_nPrecision <= DAY))
            bValid = DisplayParamError(PRECISION);
        }
        else
          if (!(YEAR <= m_nPrecision && m_nPrecision <= DAY))  // Change to DAYS, YEARS
            bValid = DisplayParamError(PRECISION);

        if (!ValidateDateString(m_szStartDate, STARTDATE)) {
           bValid = DisplayParamError(STARTDATE);
           bValidDate = false;
        }
        else if (!ValidateDateString(m_szEndDate, ENDDATE)) {
          bValid = DisplayParamError(ENDDATE);
          bValidDate = false;
        }
        else if (CharToJulian(m_szStartDate) >= CharToJulian(m_szEndDate)) {
          bValid = DisplayParamError(ENDDATE);
          bValidDate = false;
        }

        // Spatial Options
        if ((m_nAnalysisType == PURELYSPATIAL) || (m_nAnalysisType == SPACETIME) || (m_nAnalysisType == PROSPECTIVESPACETIME)) {
          if (m_nMaxSpatialClusterSizeType == PERCENTAGEOFMEASURETYPE && !(0.0 < m_nMaxGeographicClusterSize && m_nMaxGeographicClusterSize <= 50.0))
            bValid = DisplayParamError(GEOSIZE);
          if (0.0 > m_nMaxGeographicClusterSize)
            bValid = DisplayParamError(GEOSIZE);
        }
        else
          m_nMaxGeographicClusterSize = 50.0; //KR980707 0 GG980716;

        // Temporal Options
        if ((m_nAnalysisType == PURELYTEMPORAL) || (m_nAnalysisType == SPACETIME) || (m_nAnalysisType == PROSPECTIVESPACETIME)) {
          if (m_nMaxClusterSizeType == PERCENTAGETYPE &&
              !(0.0 < m_nMaxTemporalClusterSize &&
                m_nMaxTemporalClusterSize <= (m_nModel == SPACETIMEPERMUTATION ? 50 : 90)))
            bValid = DisplayParamError(TIMESIZE);
          if ((!PROSPECTIVESPACETIME) && (!(m_bAliveClustersOnly==0 || m_bAliveClustersOnly==1)))
            bValid = DisplayParamError(CLUSTERS);
          if (!(YEAR <= m_nIntervalUnits && m_nIntervalUnits <= m_nPrecision))
            bValid = DisplayParamError(INTERVALUNITS);
          if (!(1 <= m_nIntervalLength && m_nIntervalLength <= TimeBetween(CharToJulian(m_szStartDate), CharToJulian(m_szEndDate), m_nIntervalUnits) ))// Change to Max Interval
            bValid = DisplayParamError(TIMEINTLEN);

          if (m_nModel == BERNOULLI || m_nModel == SPACETIMEPERMUTATION) {
            m_nTimeAdjustType = NOTADJUSTED;
            m_nTimeAdjPercent = 0.0;
          }
          else {
            if (!(NOTADJUSTED <= m_nTimeAdjustType && m_nTimeAdjustType <= LINEAR))
              bValid = DisplayParamError(TIMETREND);
            if (m_nTimeAdjustType == NONPARAMETRIC && m_nAnalysisType == PURELYTEMPORAL)
              bValid = DisplayParamError(TIMETREND);
            if (m_nTimeAdjustType == LINEAR)
              if (!(-100.0 < m_nTimeAdjPercent))
                bValid = DisplayParamError(TIMETRENDPERC);
          }
        }
        else {
          m_nMaxTemporalClusterSize = 50.0; // KR980707 0 GG980716;
          m_bAliveClustersOnly      = false;
          m_nIntervalUnits          = NONE;
          m_nIntervalLength         = 0;
          m_nTimeAdjustType         = NOTADJUSTED;
          m_nTimeAdjPercent         = 0;
        }
      }

      if (strlen(m_szCaseFilename)==0 || (pFile = fopen(m_szCaseFilename, "r")) == NULL)
        bValid = DisplayParamError(CASEFILE);
      else
        fclose(pFile);
    
      if (m_nModel == POISSON) {
        if (strlen(m_szPopFilename)==0 || (pFile = fopen(m_szPopFilename, "r")) == NULL)
          bValid = DisplayParamError(POPFILE);
        else 
          fclose(pFile);
        strcpy(m_szControlFilename, "");
      }
      else if (m_nModel == BERNOULLI) {
        if (strlen(m_szControlFilename)==0 || (pFile = fopen(m_szControlFilename, "r")) == NULL)
          bValid = DisplayParamError(CONTROLFILE);
        else
          fclose(pFile);
        strcpy(m_szPopFilename, "");
      }
      else if (m_nModel == SPACETIMEPERMUTATION) {
        if (m_nMaxSpatialClusterSizeType == PERCENTAGEOFMEASURETYPE)
          if (strlen(m_szPopFilename)==0 || (pFile = fopen(m_szPopFilename, "r")) == NULL)
            bValid = DisplayParamError(POPFILE);
          else
            fclose(pFile);
        if (!(m_nAnalysisType == SPACETIME || m_nAnalysisType == PROSPECTIVESPACETIME))
          bValid = DisplayParamError(ANALYSISTYPE);
        if (m_bIncludePurelySpatial)
          bValid = DisplayParamError(PURESPATIAL);
        if (m_bIncludePurelyTemporal)
          bValid = DisplayParamError(PURETEMPORAL);
        if (m_bOutputRelRisks)
          bValid = DisplayParamError(OUTPUTRR);
      }

      if (strlen(m_szCoordFilename)==0 || (pFile = fopen(m_szCoordFilename, "r")) == NULL)
        bValid = DisplayParamError(COORDFILE);
      else
        fclose(pFile);
    
      if (m_bSpecialGridFile) {
        if (strlen(m_szGridFilename)==0 || (pFile = fopen(m_szGridFilename, "r")) == NULL)
          bValid = DisplayParamError(GRIDFILE);
        else
          fclose(pFile);
      }
      else
        strcpy(m_szGridFilename, "");

      if (strlen(m_szOutputFilename)==0 || (pFile = fopen(m_szOutputFilename, "r")) == NULL) {
        if ((pFile = fopen(m_szOutputFilename, "w")) == NULL)
          bValid = DisplayParamError(OUTPUTFILE);
        else
          fclose(pFile);
      }
      else
        fclose(pFile);

      if (!(m_iCriteriaSecondClusters>=0 && m_iCriteriaSecondClusters<=5))
          bValid = DisplayParamError(CRITERIA_SECOND_CLUSTERS);

      // Verify Character Prospective start date (YYYY/MM/DD).
      // THIS IS RUN AFTER STUDY DATES HAVE BEEN CHECKED !!!
      if (bValid && (m_nAnalysisType == PROSPECTIVESPACETIME) && bValidDate)
         if (! ValidateProspectiveStartDate(m_szProspStartDate, m_szStartDate, m_szEndDate))
            bValid = DisplayParamError(START_PROSP_SURV);

      m_bExactTimes = 0;
      m_nDimension  = 0;

      // if no filename defined in the parameters file then we'll use the current working directory
      if( !ValidHistoryFileName(gsRunHistoryFilename) )
         gsRunHistoryFilename << ZdString::reset << ZdFileName(_argv[0]).GetLocation() << ANALYSIS_HISTORY_FILE;
   }
   catch (ZdException & x) {
      fclose(pFile);
      x.AddCallpath("ValidateParameters()", "CParameters");
      throw;
   }
   return bValid;
}

bool CParameters::ValidateDateString(char* szDate, int nDateType) {
   UInt nYear, nMonth, nDay;
   int  nScanCount;
   bool bReturnValue = false;

   try {
      nScanCount = CharToMDY(&nMonth, &nDay, &nYear, szDate);

      if (nScanCount < 1)
        return false;

      if (m_nPrecision == YEAR || nScanCount == 1) {
        switch(nDateType) {
          case STARTDATE: nMonth = 1; break;
          case ENDDATE  : nMonth = 12; break;
        }
      }

      if (m_nPrecision== YEAR || m_nPrecision == MONTH || nScanCount  == 1 || nScanCount   == 2) {
        switch(nDateType) {
          case STARTDATE: nDay = 1; break;
          case ENDDATE  : nDay = DaysThisMonth(nYear, nMonth); break;
        }
      }

      if (IsDateValid(nMonth, nDay, nYear))
        bReturnValue = true;
   }
   catch (ZdException & x) {
      x.AddCallpath("ValidateDateString()", "CParameters");
      throw;
   }
   return bReturnValue;
}

bool CParameters::ValidateProspectiveStartDate(char* szProspDate, char *szStartDate, char *szEndDate) {
   UInt nProspYear, nProspMonth, nProspDay;
   UInt nStartYear, nStartMonth, nStartDay;
   UInt nEndYear, nEndMonth, nEndDay;
   int  nStartScanCount, nEndScanCount, nProspScanCount;
   bool bReturnValue = false;

   try {
      nStartScanCount = CharToMDY(&nStartMonth, &nStartDay, &nStartYear, szStartDate);
      nEndScanCount = CharToMDY(&nEndMonth, &nEndDay, &nEndYear, szEndDate);
      nProspScanCount = CharToMDY(&nProspMonth, &nProspDay, &nProspYear, szProspDate);

      if ((nStartScanCount < 1) || (nEndScanCount < 1) || (nProspScanCount < 1))
        return false;

      if (m_nPrecision == YEAR || nStartScanCount == 1)
         nStartMonth = 1;
      if (m_nPrecision == YEAR || nEndScanCount == 1)
         nEndMonth = 1;
      if (m_nPrecision == YEAR || nProspScanCount == 1)
         nProspMonth = 1;

      if (m_nPrecision == YEAR || m_nPrecision == MONTH) {
         if (nStartScanCount == 1 || nStartScanCount == 2)
           nStartDay = 1;
         if (nEndScanCount == 1 || nEndScanCount == 2)
           nEndDay = DaysThisMonth(nEndYear, nEndMonth);
         if (nProspScanCount == 1 || nProspScanCount == 2)
           nProspDay = 1;
      }

      //Prospective surveillance start date must be between study dates.
         // can be equal to one of them...
         //compute Julian date and check ranges...
      if ((IsDateValid(nStartMonth, nStartDay, nStartYear) && IsDateValid(nEndMonth, nEndDay, nEndYear) && IsDateValid(nProspMonth, nProspDay, nProspYear)))
         bReturnValue = CheckProspDateRange(nStartYear, nStartMonth, nStartDay, nEndYear, nEndMonth, nEndDay,
                                            nProspYear, nProspMonth, nProspDay);
   }
   catch (ZdException & x) {
      x.AddCallpath("ValidateProspectiveStartDate()", "CParameters");
      throw;
   }
   return bReturnValue;
}

// checks to see that the reps are a valid number (i.e. either 9, 19, or n999)
// pre: nReps number of Monte Carlo reps
// post: returns true if number is valid, false otherwise
bool CParameters::ValidateReplications(int nReps) {
  return (nReps >= 0) && (nReps == 0 || nReps == 9 || nReps == 19 || fmod(nReps+1, 1000) == 0.0);
}

// determines whether or not the history file name from the parameters file is a valid one
// if it is not, the controlling branch which calls this function must set the history file
// name to a more appropraite path - AJV 9/16/2002
bool CParameters::ValidHistoryFileName(const ZdString& sRunHistoryFilename) {
   bool         bValid = false;

   try {
      if(!sRunHistoryFilename.GetIsEmpty())
         // in the old parameter file if the name was empty it would interpret the // as the filename which
         // cause an exception and error later on - AJV 9/16/2002
         if(!(sRunHistoryFilename == "\/\/"))
            if(!access(sRunHistoryFilename.GetCString(), 00))    // check for existence of file
               if(!access(sRunHistoryFilename.GetCString(), 06)) // check for read and write access
                  bValid = true;
   }
   catch (ZdException &x) {
      x.AddCallpath("ValidHistoryFileName()", "CParameters");
      throw;
   }
   return bValid;
}

// helper function to translate the y/n stored in the ini file to a bool
// pre : sTestValue not empty
// post : returns true if sTestValue begins with a 'y' or 'Y'
bool CParameters::ValueIsYes(const ZdString& sTestValue) {
   return (sTestValue.BeginsWith("Y") || sTestValue.BeginsWith("y"));
}

// checks the format of the ini file to be read to ensure that the correct lines exist
// pre: sFileName is the name of the ini file
// post: will throw an exception if one of the required ini lines is missing
void CParameters::VerifyIniFileSetup(const ZdString& sFileName, bool bCreateIfMissing) {
   try {
      ZdIniFile file(sFileName.GetCString());

      CheckIniSectionsExist(file, bCreateIfMissing);
      CheckInputFileSection(file, bCreateIfMissing);
      CheckModelInfoIniSection(file, bCreateIfMissing);
      CheckSequentialScanIniSection(file, bCreateIfMissing);
      CheckEllipseIniSection(file, bCreateIfMissing);
      CheckOutputFileIniSection(file, bCreateIfMissing);

  //    if(bCreateIfMissing)
         file.Write();
   }
   catch (ZdException &x) {
      x.AddCallpath("VerifyIniFileSetup()", "CParameters");
      throw;
   }
}



#include "SaTScan.h"
#pragma hdrstop
#include "Parameters.h"
#include <io.h>

#define INCLUDE_RUN_HISTORY

const char*      YES                    = "y";
const char*      NO                     = "n";

const char*      INPUT_FILES_SECTION            = "[Input Files]";
const char*      CASE_FILE_LINE                 = "CaseFile";
const char*      CONTROL_FILE_LINE              = "ControlFile";
const char*      POP_FILE_LINE                  = "PopulationFile";
const char*      COORD_FILE_LINE                = "CoordinatesFile";
const char*      GRID_FILE_LINE                 = "GridFile";
const char*      USE_GRID_FILE_LINE             = "UseGridFile";
const char*      PRECISION_TIMES_LINE           = "PrecisionCaseTimes";
const char*      COORD_TYPE_LINE                = "CoordinatesType";

const char*      ANALYSIS_SECTION               = "[Analysis]";
const char*      ANALYSIS_TYPE_LINE             = "AnalysisType";
const char*      MODEL_TYPE_LINE                = "ModelType";
const char*      SCAN_AREAS_LINE                = "ScanAreas";
const char*      START_DATE_LINE                = "StartDate";
const char*      END_DATE_LINE                  = "EndDate";
const char*      MONTE_CARLO_REPS_LINE          = "MonteCarloReps";

const char*      TIME_PARAMS_SECTION            = "[Time Parameters]";
const char*      INTERVAL_UNITS_LINE            = "IntervalUnits";
const char*      INTERVAL_LENGTH_LINE           = "IntervalLength";
const char*      PROSPECT_START_LINE            = "ProspectiveStartDate";
const char*      TIME_TREND_ADJ_LINE            = "TimeTrendAdjustmentType";
const char*      TIME_TREND_PERCENT_LINE        = "TimeTrendPercentage";

const char*      SCANNING_WINDOW_SECTION        = "[Scanning Window]";
const char*      MAX_GEO_SIZE_LINE              = "MaxGeographicSize";
const char*      MAX_SPATIAL_SIZE_LINE          = "MaxSpatialSizeInterpretation";
const char*      INCLUDE_PURE_TEMP_LINE         = "IncludePurelyTemporal";
const char*      MAX_TEMP_SIZE_LINE             = "MaxTemporalSize";
const char*      MAX_TEMP_INTERPRET_LINE        = "MaxTemporalSizeInterpretation";
const char*      INCLUDE_PURELY_SPATIAL_LINE    = "IncludePurelySpatial";
const char*      ALIVE_CLUSTERS_LINE            = "AliveClustersOnly";

const char*      OUTPUT_FILES_SECTION           = "[Output Files]";
const char*      RESULTS_FILE_LINE              = "ResultsFile";
const char*      MOST_LIKELY_CLUSTER_LINE       = "MostLikelyClusterEachCentroidASCII";
const char*      DBASE_CLUSTER_LINE             = "MostLikelyClusterEachCentroidDBase";
const char*      CENSUS_REPORT_CLUSTERS_LINE    = "CensusAreasReportedClustersASCII";
const char*      DBASE_AREA_LINE                = "CensusAreasReportedClustersDBase";
const char*      SAVE_SIM_LLRS                  = "SaveSimLLRsASCII";
const char*      DBASE_LOG_LIKELI               = "SaveSimLLRsDBase";
const char*      INCLUDE_REL_RISKS_LINE         = "IncludeRelativeRisksCensusAreasASCII";
const char*      DBASE_RELATIVE_RISKS           = "IncludeRelativeRisksCensusAreasDBase";
const char*      CRIT_REPORT_SEC_CLUSTERS_LINE  = "CriteriaForReportingSecondaryClusters";

const char*      SEQUENTIAL_SCAN_SECTION        = "[Sequential Scan]";
const char*      SEQUENTIAL_SCAN_LINE           = "SequentialScan";
const char*      SEQUENTIAL_MAX_ITERS_LINE      = "SequentialScanMaxIterations";
const char*      SEQUENTIAL_MAX_PVALUE_LINE     = "SequentialScanMaxPValue";

const char*      ELLIPSES_SECTION               = "[Elliptic Scan]";
const char*      NUMBER_ELLIPSES_LINE           = "NumberOfEllipses";
const char*      ELLIPSE_SHAPES_LINE            = "EllipseShapes";
const char*      ELLIPSE_ANGLES_LINE            = "EllipseAngles";
const char*      ELLIPSE_DUCZMAL_COMPACT_LINE   = "DuczmalCompactnessCorrection";

const char*      ADVANCED_FEATURES_SECTION      = "[Advanced Features]";
const char*      VALID_PARAMS_LINE              = "ValidateParameters";
const char*      ISOTONIC_SCAN_LINE             = "IsotonicScan";
const char*      PVALUE_PROSPECT_LLR_LINE       = "PValues2PrespecifiedLLRs";
const char*      LLR_1_LINE                     = "LLR1";
const char*      LLR_2_LINE                     = "LLR2";


char mgsVariableLabels[51][100] = {
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
   "dBase output for Log Likelihoods",
   "Duczmal compactness correction for ellipses"
   };

/** Constructor */   
CParameters::CParameters() {
  SetDefaults();
}

/** Copy constructor */
CParameters::CParameters(const CParameters &other) {
   SetDefaults();
   copy(other);
}

/** Destructor */
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

// checks to make sure all of the required keys exist in the model info section of the ini
// pre: file is opened ini parameters file
// post: adds the lines missing along with the corresponding comments
void CParameters::CheckAdvancedFeaturesIniSection(ZdIniFile& file) {
   try {
      ZdIniSection* pSection = file.GetSection(ADVANCED_FEATURES_SECTION);

      if ( pSection->FindKey(VALID_PARAMS_LINE) == -1 ) {
            pSection->AddComment(" validate parameters (y/n)");
            pSection->AddLine(VALID_PARAMS_LINE, "");
      }
      if ( pSection->FindKey(ISOTONIC_SCAN_LINE) == -1 ) {
            pSection->AddComment(" Isotonic Scan (y/n)");
            pSection->AddLine(ISOTONIC_SCAN_LINE, "");
      }
      if ( pSection->FindKey(PVALUE_PROSPECT_LLR_LINE) == -1 ) {
            pSection->AddComment(" p-Values for 2 Prespecified LLR's (y/n)");
            pSection->AddLine(PVALUE_PROSPECT_LLR_LINE, "");
      }
      if ( pSection->FindKey(LLR_1_LINE) == -1 )
            pSection->AddLine(LLR_1_LINE, "");
      if ( pSection->FindKey(LLR_2_LINE) == -1 )
            pSection->AddLine(LLR_2_LINE, "");
   }
   catch (ZdException &x) {
      x.AddCallpath("CheckAdvancedFeaturesIniSection()", "CParameters");
      throw;
   }
}

// checks to make sure all of the required keys exist in the analysis section of the ini
// pre: file is opened ini parameters file
// post: adds the lines missing along with the corresponding comments
void CParameters::CheckAnalysisIniSection(ZdIniFile& file) {
   try {
      ZdIniSection* pSection = file.GetSection(ANALYSIS_SECTION);
      if ( pSection->FindKey(ANALYSIS_TYPE_LINE) == -1 ) {
            pSection->AddComment(" analysis type (1=Purely Spatial, 2=Purely Temporal, 3=Retrospective Space-Time, 4=Prospective Space-Time)");
            pSection->AddLine(ANALYSIS_TYPE_LINE, "");
      }
      if ( pSection->FindKey(MODEL_TYPE_LINE) == -1 ) {
            pSection->AddComment(" model type (0=Poisson, 1=Bernoulli, 2=Space-Time Permutation)");
            pSection->AddLine(MODEL_TYPE_LINE, "");
      }
      if ( pSection->FindKey(SCAN_AREAS_LINE) == -1 ) {
            pSection->AddComment(" scan areas (1=High, 2=Low, 3=High or Low)");
            pSection->AddLine(SCAN_AREAS_LINE, "");
      }
      if ( pSection->FindKey(START_DATE_LINE) == -1 ) {
            pSection->AddComment(" start date (YYYY/MM/DD)");
            pSection->AddLine(START_DATE_LINE, "");
      }
      if ( pSection->FindKey(END_DATE_LINE) == -1 ) {
            pSection->AddComment(" end date (YYYY/MM/DD)");
            pSection->AddLine(END_DATE_LINE, "");
      }
      if ( pSection->FindKey(MONTE_CARLO_REPS_LINE) == -1 ) {
            pSection->AddComment(" Monte Carlo reps (0, 9, 999, n999)");
            pSection->AddLine(MONTE_CARLO_REPS_LINE, "");
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("CheckAnalysisIniSection()", "CParameters");
      throw;
   }
}

// checks to make sure all of the required keys exist in the ellipse section of the ini
// pre: file is opened ini parameters file
// post: adds the lines missing along with the corresponding comments
void CParameters::CheckEllipseIniSection(ZdIniFile& file) {
   try {
      ZdIniSection *pSection = file.GetSection(ELLIPSES_SECTION);
      if ( pSection->FindKey(NUMBER_ELLIPSES_LINE) == -1 ) {
            pSection->AddComment(" number of ellipses (0-10)");
            pSection->AddLine(NUMBER_ELLIPSES_LINE, "");
      }
      if ( pSection->FindKey(ELLIPSE_SHAPES_LINE) == -1 ) {
            pSection->AddComment(" ellipse shapes");
            pSection->AddLine(ELLIPSE_SHAPES_LINE, "");
      }
      if ( pSection->FindKey(ELLIPSE_ANGLES_LINE) == -1 ) {
            pSection->AddComment(" ellipse angles");
            pSection->AddLine(ELLIPSE_ANGLES_LINE, "");
      }
      if ( pSection->FindKey(ELLIPSE_DUCZMAL_COMPACT_LINE) == -1 ) {
            pSection->AddComment(" Duczmal Compactness Correction (y/n)");
            pSection->AddLine(ELLIPSE_DUCZMAL_COMPACT_LINE, "");
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("CheckEllipseIniSection()", "CParameters");
      throw;
   }
}

// checks to make sure all of the required ini sections are in the file
// pre: file is opened ini parameter file
// post: adds the lines missing along with the corresponding comments
void CParameters::CheckIniSectionsExist(ZdIniFile& file) {
   try {
      if (file.GetSectionIndex(INPUT_FILES_SECTION) == -1)
         file.AddSection(INPUT_FILES_SECTION);
      if (file.GetSectionIndex(ANALYSIS_SECTION) == -1)
         file.AddSection(ANALYSIS_SECTION);
      if (file.GetSectionIndex(TIME_PARAMS_SECTION) == -1)
         file.AddSection(TIME_PARAMS_SECTION);
      if (file.GetSectionIndex(SCANNING_WINDOW_SECTION) == -1)
         file.AddSection(SCANNING_WINDOW_SECTION);
      if (file.GetSectionIndex(OUTPUT_FILES_SECTION) == -1)
         file.AddSection(OUTPUT_FILES_SECTION);
      if (file.GetSectionIndex(SEQUENTIAL_SCAN_SECTION) == -1)
         file.AddSection(SEQUENTIAL_SCAN_SECTION);
      if (file.GetSectionIndex(ELLIPSES_SECTION) == -1)
         file.AddSection(ELLIPSES_SECTION);
      if (file.GetSectionIndex(ADVANCED_FEATURES_SECTION) == -1)
         file.AddSection(ADVANCED_FEATURES_SECTION);
   }
   catch (ZdException &x) {
      x.AddCallpath("CheckIniSectionsExist()", "CParameters");
      throw;
   }
}

// checks to make sure all of the required keys exist in the input file section of the ini
// pre: file is opened ini parameters file
// post: adds the lines missing along with the corresponding comments
void CParameters::CheckInputFileSection(ZdIniFile& file) {
   try {
      ZdIniSection* pSection = file.GetSection(INPUT_FILES_SECTION);
      if ( pSection->FindKey(CASE_FILE_LINE) == -1 )
         pSection->AddLine(CASE_FILE_LINE, "");

      if ( pSection->FindKey(CONTROL_FILE_LINE) == -1 )
         pSection->AddLine(CONTROL_FILE_LINE, "");

      if ( pSection->FindKey(POP_FILE_LINE) == -1 )
         pSection->AddLine(POP_FILE_LINE, "");

      if ( pSection->FindKey(COORD_FILE_LINE) == -1 )
         pSection->AddLine(COORD_FILE_LINE, "");

      if ( pSection->FindKey(USE_GRID_FILE_LINE) == -1 ) {
         pSection->AddComment(" use special grid file? (y/n)");
         pSection->AddLine(USE_GRID_FILE_LINE, "");
      }

      if ( pSection->FindKey(GRID_FILE_LINE) == -1 )
         pSection->AddLine(GRID_FILE_LINE, "");

      if ( pSection->FindKey(PRECISION_TIMES_LINE) == -1 ) {
         pSection->AddComment(" precision of case times (0=None, 1=Year, 2=Month, 3=day)");
         pSection->AddLine(PRECISION_TIMES_LINE, "");
      }

      if ( pSection->FindKey(COORD_TYPE_LINE) == -1 ) {
         pSection->AddComment(" coordinate type (0=Cartesian, 1=Lat/Long)");
         pSection->AddLine(COORD_TYPE_LINE, "");
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("CheckInputFileSection()", "CParameters");
      throw;
   }
}


// checks to make sure all of the required keys exist in the output file section of the ini
// pre: file is opened ini parameters file
// post: adds the lines missing along with the corresponding comments
void CParameters::CheckOutputFileIniSection(ZdIniFile& file) {
   try {
      ZdIniSection* pSection = file.GetSection(OUTPUT_FILES_SECTION);
       if ( pSection->FindKey(RESULTS_FILE_LINE) == -1 ) 
            pSection->AddLine(RESULTS_FILE_LINE, "");
      if ( pSection->FindKey(MOST_LIKELY_CLUSTER_LINE) == -1 ) {
         pSection->AddComment(" output most likely clusters in ASCII format (y/n)");
         pSection->AddLine(MOST_LIKELY_CLUSTER_LINE, "");
      }
      if ( pSection->FindKey(DBASE_CLUSTER_LINE) == -1 ) {
         pSection->AddComment(" output most likely clusters in dBase format (y/n)");
         pSection->AddLine(DBASE_CLUSTER_LINE, "");
      }
      if ( pSection->FindKey(CENSUS_REPORT_CLUSTERS_LINE) == -1 ) {
         pSection->AddComment(" report census areas in ASCII format (y/n)");
         pSection->AddLine(CENSUS_REPORT_CLUSTERS_LINE, "");
      }
      if ( pSection->FindKey(DBASE_AREA_LINE) == -1 ) {
         pSection->AddComment(" report census areas in dBase format (y/n)");
         pSection->AddLine(DBASE_AREA_LINE, "");
      }
      if ( pSection->FindKey(SAVE_SIM_LLRS) == -1 ) {
         pSection->AddComment(" report Simulated Log Likelihoods Ratios in ASCII format (y/n)");
         pSection->AddLine(SAVE_SIM_LLRS, "");
      }
      if ( pSection->FindKey(DBASE_LOG_LIKELI) == -1 ) {
         pSection->AddComment(" report Simulated Log Likelihoods Ratios in dBase format (y/n)");
         pSection->AddLine(DBASE_LOG_LIKELI, "");
      }
      if ( pSection->FindKey(INCLUDE_REL_RISKS_LINE) == -1 ) {
         pSection->AddComment(" report relative risks in ASCII format (y/n)");
         pSection->AddLine(INCLUDE_REL_RISKS_LINE, "");
      }
      if ( pSection->FindKey(DBASE_RELATIVE_RISKS) == -1 ) {
         pSection->AddComment(" report relative risks in dBase format (y/n)");
         pSection->AddLine(DBASE_RELATIVE_RISKS, "");
      }
      if ( pSection->FindKey(CRIT_REPORT_SEC_CLUSTERS_LINE) == -1 ) {
         pSection->AddComment(" criteria for reporting secondary clusters(0=NoGeoOverlap, 1=NoCentersInOther, 2=NoCentersInMostLikely, 3=NoCentersInLessLikely, 4=NoPairsCentersEachOther, 5=NoRestrictions)");
         pSection->AddLine(CRIT_REPORT_SEC_CLUSTERS_LINE, "");
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

// checks to make sure all of the required keys exist in the scanning window section of the ini
// pre: file is opened ini parameters file
// post: adds the lines missing along with the corresponding comments
void CParameters::CheckScanningWindowIniSection(ZdIniFile& file) {
   try {
      ZdIniSection* pSection = file.GetSection(SCANNING_WINDOW_SECTION);
      if ( pSection->FindKey(MAX_GEO_SIZE_LINE) == -1 ) {
            pSection->AddComment(" max geographic size (<=50%)");
            pSection->AddLine(MAX_GEO_SIZE_LINE, "");
      }
      if ( pSection->FindKey(MAX_SPATIAL_SIZE_LINE) == -1 ) {
            pSection->AddComment(" how max spatial size should be interpretted (0=Percentage, 1=Distance)");
            pSection->AddLine(MAX_SPATIAL_SIZE_LINE, "");
      }
      if ( pSection->FindKey(INCLUDE_PURE_TEMP_LINE) == -1 ) {
            pSection->AddComment(" include purely temporal clusters (y/n)");
            pSection->AddLine(INCLUDE_PURE_TEMP_LINE, "");
      }
      if ( pSection->FindKey(MAX_TEMP_SIZE_LINE) == -1 ) {
            pSection->AddComment(" max temporal size (<=90%)");
            pSection->AddLine(MAX_TEMP_SIZE_LINE, "");
      }
      if ( pSection->FindKey(MAX_TEMP_INTERPRET_LINE) == -1 ) {
            pSection->AddComment(" how max temporal size should be interpretted (0=Percentage, 1=Time)");
            pSection->AddLine(MAX_TEMP_INTERPRET_LINE, "");
      }
      if ( pSection->FindKey(INCLUDE_PURELY_SPATIAL_LINE) == -1 ) {
            pSection->AddComment(" include purely spatial clusters (y/n)");
            pSection->AddLine(INCLUDE_PURELY_SPATIAL_LINE, "");
      }
      if ( pSection->FindKey(ALIVE_CLUSTERS_LINE) == -1 ) {
            pSection->AddComment(" alive clusters only? (y/n)");
            pSection->AddLine(ALIVE_CLUSTERS_LINE, "");
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("CheckScanningWindowIniSection()", "CParameters");
      throw;
   }
}

// checks to make sure all of the required keys exist in the sequential scan section of the ini
// pre: file is opened ini parameters file
// post: adds the lines missing along with the corresponding comments
void CParameters::CheckSequentialScanIniSection(ZdIniFile& file) {
   try {
      ZdIniSection *pSection = file.GetSection(SEQUENTIAL_SCAN_SECTION);
      if ( pSection->FindKey(SEQUENTIAL_SCAN_LINE) == -1 ) {
            pSection->AddComment(" sequential scan (y/n)");
            pSection->AddLine(SEQUENTIAL_SCAN_LINE, "");
      }
      if ( pSection->FindKey(SEQUENTIAL_MAX_ITERS_LINE) == -1 ) {
            pSection->AddComment(" max iterations for sequential scan (0-32000)");
            pSection->AddLine(SEQUENTIAL_MAX_ITERS_LINE, "");
      }
      if ( pSection->FindKey(SEQUENTIAL_MAX_PVALUE_LINE) == -1 ) {
            pSection->AddComment(" max p-Value for sequential scan (0.000-1.000)");
            pSection->AddLine(SEQUENTIAL_MAX_PVALUE_LINE, "");
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("CheckSequentialScanIniSection()", "CParameters");
      throw;
   }
}

// checks to make sure all of the required keys exist in the time parameters section of the ini
// pre: file is opened ini parameters file
// post: adds the lines missing along with the corresponding comments
void CParameters::CheckTimeParametersIniSection(ZdIniFile& file) {
   try {
      ZdIniSection* pSection = file.GetSection(TIME_PARAMS_SECTION);
      if ( pSection->FindKey(INTERVAL_UNITS_LINE) == -1 ) {
         pSection->AddComment(" interval units (0=None, 1=Year, 2=Month, 3=Day)");
         pSection->AddLine(INTERVAL_UNITS_LINE, "");
      }
      if ( pSection->FindKey(INTERVAL_LENGTH_LINE) == -1 ) {
         pSection->AddComment(" inteval length (positive integer)");
         pSection->AddLine(INTERVAL_LENGTH_LINE, "");
      }
      if ( pSection->FindKey(PROSPECT_START_LINE) == -1 ) {
         pSection->AddComment(" prospective surveillance start date (YYYY/MM/DD)");
         pSection->AddLine(PROSPECT_START_LINE, "");
      }
      if ( pSection->FindKey(TIME_TREND_ADJ_LINE) == -1 ) {
         pSection->AddComment(" Time trend adjustment type (0=None, 1=Nonparametric, 2=LogLinear)");
         pSection->AddLine(TIME_TREND_ADJ_LINE, "");
      }
      if ( pSection->FindKey(TIME_TREND_PERCENT_LINE) == -1 ) {
         pSection->AddComment(" time trend adjustment percentage (>-100)");
         pSection->AddLine(TIME_TREND_PERCENT_LINE, "");
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("CheckTimeParametersIniSection()", "CParameters");
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

/** If passed filename contains a slash, then assumes that path is complete and
    sInputFilename is not modified.
    If filename does not contain a slash, it is assumed that filename is located
    in same directory of parameter file. sInputFilename is reset to this location.
    Note that the primary reason for implementing this feature was to permit
    the program to be installed in any location and sample parameter files
    run immediately with no modifications to settings. */
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
       m_bDuczmalCorrectEllipses = rhs.m_bDuczmalCorrectEllipses;
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
    m_sCaseFileName = rhs.m_sCaseFileName;
    m_sControlFileName = rhs.m_sControlFileName;
    m_sPopulationFileName = rhs.m_sPopulationFileName;
    m_sPopulationFileName = rhs.m_sPopulationFileName;
    m_sCoordinatesFileName = rhs.m_sCoordinatesFileName;
    m_sCoordinatesFileName = rhs.m_sCoordinatesFileName;
    m_sSpecialGridFileName = rhs.m_sSpecialGridFileName;
    m_sSpecialGridFileName = rhs.m_sSpecialGridFileName;
    m_bSpecialGridFile = rhs.m_bSpecialGridFile;
    m_nPrecision                = rhs.m_nPrecision;
    m_nDimension                = rhs.m_nDimension;
    m_nCoordType                = rhs.m_nCoordType;
    m_sOutputFileName = rhs.m_sOutputFileName;
    m_bSaveSimLogLikelihoods    = rhs.m_bSaveSimLogLikelihoods;
    m_bOutputRelRisks           = rhs.m_bOutputRelRisks;
    m_bSequential               = rhs.m_bSequential;
    m_nAnalysisTimes            = rhs.m_nAnalysisTimes;
    m_nCutOffPVal               = rhs.m_nCutOffPVal;
    m_bExactTimes               = rhs.m_bExactTimes;
    m_nClusterType              = rhs.m_nClusterType;
    m_bValidatePriorToCalc      = rhs.m_bValidatePriorToCalc;

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
    gbLogRunHistory             = rhs.gbLogRunHistory;

    m_nInitialMaxClusterSizeType = rhs.m_nInitialMaxClusterSizeType;
    m_nInitialMaxTemporalClusterSize = rhs.m_nInitialMaxTemporalClusterSize;
}

void CParameters::DisplayAnalysisType(FILE* fp) const {
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
 
void CParameters::DisplayParameters(FILE* fp) const {
   ZdFileName   fileName(m_sOutputFileName.c_str());
   ZdString     sASCIIName, sDBaseName;

   try {
     fprintf(fp, "\n________________________________________________________________\n\n");
     fprintf(fp, "PARAMETER SETTINGS\n\n");

     fprintf(fp, "Input Files\n");
     fprintf(fp, "-----------\n");

     fprintf(fp, "  Case File        : %s\n", m_sCaseFileName.c_str());
   
     if (m_nModel == POISSON || (m_nModel == SPACETIMEPERMUTATION && m_nMaxSpatialClusterSizeType == PERCENTAGEOFMEASURETYPE))
       fprintf(fp, "  Population File  : %s\n", m_sPopulationFileName.c_str());
     else if (m_nModel == BERNOULLI)
       fprintf(fp, "  Control File     : %s\n", m_sControlFileName.c_str());

     fprintf(fp, "  Coordinates File : %s\n", m_sCoordinatesFileName.c_str());
     if (m_bSpecialGridFile)
       fprintf(fp, "  Special Grid File: %s\n", m_sSpecialGridFileName.c_str());

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
        fprintf(fp, "  Number of Ellipse Shapes Requested       : %i\n", m_nNumEllipses);
        fprintf(fp, "  Shape for Each Ellipse                   : ");
        for (int i = 0; i < m_nNumEllipses; ++i)
           fprintf(fp, "%.3f ", mp_dEShapes[i]);
        fprintf(fp, "\n  Number of Angles for Each Ellipse Shape  : ");
        for (int i = 0; i < m_nNumEllipses; ++i)
           fprintf(fp, "%i ", mp_nENumbers[i]);
        fprintf(fp, "\n  Duczmal Compactness Correction           : ");
        switch (m_bDuczmalCorrectEllipses) {
          case true  : fprintf(fp, "Yes"); break;
          case false : fprintf(fp, "No");  break;
        }
     }
     fprintf(fp, "\n\nScanning Window\n");
     fprintf(fp, "---------------\n");

     if (m_nAnalysisType == PURELYSPATIAL || m_nAnalysisType == SPACETIME || m_nAnalysisType == PROSPECTIVESPACETIME) {
       fprintf(fp, "  Maximum Spatial Cluster Size          : %.2f", m_nMaxGeographicClusterSize);
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
       fprintf(fp, "  Maximum Temporal Cluster Size         : %.2f", m_nInitialMaxTemporalClusterSize);
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
       fprintf(fp, "  Also Include Purely Spatial Clusters  : ");
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
   
       fprintf(fp, "  Time Interval Units  : ");
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
     fprintf(fp, "  Results File      : %s\n", m_sOutputFileName.c_str());

     // gis files
     if (gbOutputAreaSpecificDBF || m_bOutputCensusAreas) {
        sASCIIName = fileName.GetFullPath();
        if(strlen(fileName.GetExtension()) != 0)
           sASCIIName.Replace(fileName.GetExtension(), ".gis");
        else
           sASCIIName <<  ".gis";
        sDBaseName = sASCIIName;
        sASCIIName << ".txt";
        sDBaseName << ".dbf";
     }
     if (gbOutputAreaSpecificDBF) {
        if (m_bOutputCensusAreas) {
           fprintf(fp, "  GIS File(s)       : %s\n", sASCIIName.GetCString());
           fprintf(fp, "                    : %s\n", sDBaseName.GetCString());
        }
        else
           fprintf(fp, "  GIS File          : %s\n", sDBaseName.GetCString());
     }
     if (m_bOutputCensusAreas && !gbOutputAreaSpecificDBF)  // Output Census areas in Reported Clusters
        fprintf(fp, "  GIS File          : %s\n", sASCIIName.GetCString());

     // mlc files
     if (gbOutputClusterLevelDBF || m_bMostLikelyClusters) {
        sASCIIName = fileName.GetFullPath();
        if(strlen(fileName.GetExtension()) != 0)
           sASCIIName.Replace(fileName.GetExtension(), ".col");
        else
           sASCIIName << ".col";
        sDBaseName = sASCIIName;
        sASCIIName << ".txt";
        sDBaseName << ".dbf";
     }
     if (gbOutputClusterLevelDBF) {
        if (m_bMostLikelyClusters)  {  // Output Most Likely Cluster for each Centroid
           fprintf(fp, "  MLC File(s)       : %s\n", sASCIIName.GetCString());
           fprintf(fp, "                    : %s\n", sDBaseName.GetCString());
        }
        else
           fprintf(fp, "  MLC File          : %s\n", sDBaseName.GetCString());
     }
     if (m_bMostLikelyClusters && !gbOutputClusterLevelDBF)   // Output Most Likely Cluster for each Centroid
        fprintf(fp, "  MLC File          : %s\n", sASCIIName.GetCString());

     // RRE files
     if (gbRelativeRiskDBF || m_bOutputRelRisks) {
        sASCIIName = fileName.GetFullPath();
        if(strlen(fileName.GetExtension()) != 0)
           sASCIIName.Replace(fileName.GetExtension(), ".rr");
        else
           sASCIIName << ".rr.dbf";
        sDBaseName = sASCIIName;
        sASCIIName << ".txt";
        sDBaseName << ".dbf";
     }
     if (gbRelativeRiskDBF) {
        if (m_bOutputRelRisks){
           fprintf(fp, "  RRE File(s)       : %s\n", sASCIIName.GetCString());
           fprintf(fp, "                    : %s\n", sDBaseName.GetCString());
        }
        else
           fprintf(fp, "  RRE File          : %s\n", sDBaseName.GetCString());
     }
     if (m_bOutputRelRisks && !gbRelativeRiskDBF ) {
        fprintf(fp, "  RRE File          : %s\n", sASCIIName.GetCString());
     }

     // LLR Files
     if (gbLogLikelihoodDBF || m_bSaveSimLogLikelihoods) {
        sASCIIName = fileName.GetFullPath();
        if(strlen(fileName.GetExtension()) != 0)
           sASCIIName.Replace(fileName.GetExtension(), ".llr");
        else
           sASCIIName << ".llr";
        sDBaseName = sASCIIName;
        sASCIIName << ".txt";
        sDBaseName << ".dbf";
     }
     if (gbLogLikelihoodDBF) {
        if(m_bSaveSimLogLikelihoods) {
           fprintf(fp,  "  LLR File(s)       : %s\n", sASCIIName.GetCString());
           fprintf(fp,  "                    : %s\n", sDBaseName.GetCString());
        }
        else
           fprintf(fp,  "  LLR File          : %s\n", sDBaseName.GetCString());
     }
     if (m_bSaveSimLogLikelihoods && !gbLogLikelihoodDBF)
       fprintf(fp,  "  LLR File          : %s\n", sASCIIName.GetCString());

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

void CParameters::DisplayTimeAdjustments(FILE* fp) const {
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

void CParameters::FindDelimiter(char *sString, char cDelimiter) {
   char  *psString = sString;
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

// function to set the parameters from the file, uses both new and old style parameter files
// pre: szFilename is name of paramter file, validate indicates whether or not parameters should be validated after read in
// post: reads in parameters from the file and validates if requested
void CParameters::Read(const char* szFilename) {
  FILE        * pFile;

  try {
    if ((pFile = fopen(szFilename, "r")) == NULL)
      ZdGenerateException("  Error: Unable to open parameter file.", "Read()");
    fclose(pFile);

    ZdIniFile file(szFilename);
    if (file.GetNumSections())
      ReadFromIniFile(szFilename);
    else
      ReadScanningLineParameterFile(szFilename);
  }
  catch (ZdException & x) {
    x.AddCallpath("Read()", "CParameters");
    throw;
  }
}

// sets the global advanced features variables read in from the ini file
// pre: file is an open ini parameter file
// post: will set the global variables from the ini file
void CParameters::ReadAdvancedFeaturesFromIni(ZdIniFile& file) {
   try {
      ZdIniSection* pSection = file.GetSection(ADVANCED_FEATURES_SECTION);
      m_bValidatePriorToCalc = ValueIsYes(pSection->GetLine(pSection->FindKey(VALID_PARAMS_LINE))->GetValue());
      SetIntValue(m_nRiskFunctionType, pSection->GetLine(pSection->FindKey(ISOTONIC_SCAN_LINE))->GetValue(), RISKFUNCTION, STANDARDRISK);
      m_bPowerCalc = ValueIsYes(pSection->GetLine(pSection->FindKey(PVALUE_PROSPECT_LLR_LINE))->GetValue());
      SetDoubleValue(m_nPower_X, pSection->GetLine(pSection->FindKey(LLR_1_LINE))->GetValue(), POWERX, 0.0);
      SetDoubleValue(m_nPower_Y, pSection->GetLine(pSection->FindKey(LLR_2_LINE))->GetValue(), POWERY, 0.0);
   }
   catch (ZdException &x) {
      x.AddCallpath("ReadAdvancedFeaturesFromIni", "CParameters");
      throw;
   }
}

// sets the global analysis section variables read in from the ini file
// pre: file is an open ini parameter file
// post: will set the global variables from the ini file
void CParameters::ReadAnalysisSectionFromIni(ZdIniFile& file) {
   try {
      ZdIniSection* pSection = file.GetSection(ANALYSIS_SECTION);
      SetIntValue(m_nAnalysisType, pSection->GetLine(pSection->FindKey(ANALYSIS_TYPE_LINE))->GetValue(), ANALYSISTYPE, PURELYSPATIAL);
      SetIntValue(m_nAreas, pSection->GetLine(pSection->FindKey(SCAN_AREAS_LINE))->GetValue(), SCANAREAS, HIGH);
      SetIntValue(m_nModel, pSection->GetLine(pSection->FindKey(MODEL_TYPE_LINE))->GetValue(), MODEL, POISSON);
      strcpy(m_szStartDate, pSection->GetLine(pSection->FindKey(START_DATE_LINE))->GetValue());
      strcpy(m_szEndDate, pSection->GetLine(pSection->FindKey(END_DATE_LINE))->GetValue());
      SetIntValue(m_nReplicas, pSection->GetLine(pSection->FindKey(MONTE_CARLO_REPS_LINE))->GetValue(), REPLICAS, 0);
   }
   catch (ZdException &x) {
      x.AddCallpath("ReadAnalysisSectionFromIni()", "CParameters");
      throw;
   }
}

// sets the global ellipse variables read in from the ini file
// pre: file is an open ini parameter file
// post: will set the global variables from the ini file
void CParameters::ReadEllipseSectionFromIni(ZdIniFile& file) {
   try {
      ZdIniSection* pSection = file.GetSection(ELLIPSES_SECTION);
      m_nNumEllipses = atoi(pSection->GetLine(pSection->FindKey(NUMBER_ELLIPSES_LINE))->GetValue());
      SetEShapesFromIniFile(pSection->GetLine(pSection->FindKey(ELLIPSE_SHAPES_LINE))->GetValue());
      SetEAnglesFromIniFile(pSection->GetLine(pSection->FindKey(ELLIPSE_ANGLES_LINE))->GetValue());
      m_bDuczmalCorrectEllipses = ValueIsYes(pSection->GetLine(pSection->FindKey(ELLIPSE_DUCZMAL_COMPACT_LINE))->GetValue());
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
      ZdIniFile file(sFileName.GetCString());
      VerifyIniFileSetup(file);
      SetSourceFileName(sFileName.GetCString());
      ReadInputFilesSectionFromIni(file);
      ReadAnalysisSectionFromIni(file);
      ReadTimeParametersSectionFromIni(file);
      ReadScanningWindowSectionFromIni(file);
      ReadOutputFileSectionFromIni(file);
      ReadEllipseSectionFromIni(file);
      ReadSequentialScanSectionFromIni(file);
      ReadAdvancedFeaturesFromIni(file);
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
      SetCaseFileName(pSection->GetLine(pSection->FindKey(CASE_FILE_LINE))->GetValue(), true);
      SetControlFileName(pSection->GetLine(pSection->FindKey(CONTROL_FILE_LINE))->GetValue(), true);
      SetPopulationFileName(pSection->GetLine(pSection->FindKey(POP_FILE_LINE))->GetValue(), true);
      SetCoordinatesFileName(pSection->GetLine(pSection->FindKey(COORD_FILE_LINE))->GetValue(), true);
      SetSpecialGridFileName(pSection->GetLine(pSection->FindKey(GRID_FILE_LINE))->GetValue(), true);
      m_bSpecialGridFile = ValueIsYes(pSection->GetLine(pSection->FindKey(USE_GRID_FILE_LINE))->GetValue());
      SetIntValue(m_nPrecision, pSection->GetLine(pSection->FindKey(PRECISION_TIMES_LINE))->GetValue(), PRECISION, 1);
      SetIntValue(m_nCoordType, pSection->GetLine(pSection->FindKey(COORD_TYPE_LINE))->GetValue(), COORDTYPE, CARTESIAN);
   }
   catch (ZdException &x) {
      x.AddCallpath("ReadInputFilesSectionFromIni", "CParameters");
      throw;
   }
}

// sets the global output file variables read in from the ini file
// pre: file is an open ini parameter file
// post: will set the global variables from the ini file
void CParameters::ReadOutputFileSectionFromIni(ZdIniFile& file) {
   try {
      ZdIniSection* pSection = file.GetSection(OUTPUT_FILES_SECTION);
      SetOutputFileName(pSection->GetLine(pSection->FindKey(RESULTS_FILE_LINE))->GetValue(), true);
      m_bSaveSimLogLikelihoods = ValueIsYes(pSection->GetLine(pSection->FindKey(SAVE_SIM_LLRS))->GetValue());
      m_bOutputCensusAreas = ValueIsYes(pSection->GetLine(pSection->FindKey(CENSUS_REPORT_CLUSTERS_LINE))->GetValue());
      m_bMostLikelyClusters = ValueIsYes(pSection->GetLine(pSection->FindKey(MOST_LIKELY_CLUSTER_LINE))->GetValue());
      gbOutputClusterLevelDBF = ValueIsYes(pSection->GetLine(pSection->FindKey(DBASE_CLUSTER_LINE))->GetValue());
      gbOutputAreaSpecificDBF = ValueIsYes(pSection->GetLine(pSection->FindKey(DBASE_AREA_LINE))->GetValue());
      m_bOutputRelRisks = ValueIsYes(pSection->GetLine(pSection->FindKey(INCLUDE_REL_RISKS_LINE))->GetValue());
      gbRelativeRiskDBF = ValueIsYes(pSection->GetLine(pSection->FindKey(DBASE_RELATIVE_RISKS))->GetValue());
      gbLogLikelihoodDBF = ValueIsYes(pSection->GetLine(pSection->FindKey(DBASE_LOG_LIKELI))->GetValue());
      SetIntValue(m_iCriteriaSecondClusters, pSection->GetLine(pSection->FindKey(CRIT_REPORT_SEC_CLUSTERS_LINE))->GetValue(), CRITERIA_SECOND_CLUSTERS, NOGEOOVERLAP);
   }
   catch (ZdException &x) {
      x.AddCallpath("ReadOutputFileSectionFromIni()", "CParameters");
      throw;
   }
}

/** Read scanning line version of parameter file. */
void CParameters::ReadScanningLineParameterFile(const char * sParameterFileName) {
  char          szTemp [MAX_STR_LEN];
  bool          bEOF=false;
  int           iLinesRead=0;
  ZdIO          ParametersFile;
  ZdString      sLineBuffer;

  try {
    ParametersFile.Open(sParameterFileName, ZDIO_OPEN_READ);
    SetSourceFileName(sParameterFileName);
    SetDefaults();

    while (iLinesRead <= PARAMETERS && !bEOF) {
         bEOF = !ParametersFile.ReadLine(sLineBuffer);
         if (! bEOF) {
           ++iLinesRead;
           SetParameter(iLinesRead, sLineBuffer);
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
      if (m_nAnalysisType == SPACETIME)
        m_nAnalysisType = PURELYTEMPORAL;
      else if (m_nAnalysisType == PURELYTEMPORAL)
        m_nAnalysisType = SPACETIME;

      if (m_nNumEllipses > 0) {
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
        m_iCriteriaSecondClusters = 5; // no restrictions like pre v3.0
      }
    }

    //Mark defaulted values.
   if (iLinesRead != PARAMETERS) {
      while (iLinesRead <= PARAMETERS) {
          gvDefaultedValues.push_back(iLinesRead);
          ++iLinesRead;
      }
      //run history no longer stored in parameter file, need to insure that
      //it is not set as defaulted.
   }
   //Write as ini format.
   ParametersFile.Close();
   try {
     Write(sParameterFileName);
   }
   catch (ZdFileOpenFailedException & x ){/*Can write. File could be readonly, write locked, ...*/}
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadScanningLineParameterFile()", "CParameters");
    throw;
  }
}

// sets the global scanning window variables read in from the ini file
// pre: file is an open ini parameter file
// post: will set the global variables from the ini file
void CParameters::ReadScanningWindowSectionFromIni(ZdIniFile& file) {
   try {
      ZdIniSection* pSection = file.GetSection(SCANNING_WINDOW_SECTION);
      m_bIncludePurelySpatial = ValueIsYes(pSection->GetLine(pSection->FindKey(INCLUDE_PURELY_SPATIAL_LINE))->GetValue());
      SetFloatValue(m_nMaxTemporalClusterSize, pSection->GetLine(pSection->FindKey(MAX_TEMP_SIZE_LINE))->GetValue(), TIMESIZE, 0.0);
      SetFloatValue(m_nMaxGeographicClusterSize, pSection->GetLine(pSection->FindKey(MAX_GEO_SIZE_LINE))->GetValue(), GEOSIZE, 0.0);
      m_bAliveClustersOnly  = ValueIsYes(pSection->GetLine(pSection->FindKey(ALIVE_CLUSTERS_LINE))->GetValue());
      m_bIncludePurelyTemporal = ValueIsYes(pSection->GetLine(pSection->FindKey(INCLUDE_PURE_TEMP_LINE))->GetValue());
      SetIntValue(m_nMaxClusterSizeType, pSection->GetLine(pSection->FindKey(MAX_TEMP_INTERPRET_LINE))->GetValue(), MAX_TEMPORAL_TYPE, PERCENTAGETYPE);
      SetIntValue(m_nMaxSpatialClusterSizeType, pSection->GetLine(pSection->FindKey(MAX_SPATIAL_SIZE_LINE))->GetValue(), MAX_SPATIAL_TYPE, PERCENTAGEOFMEASURETYPE);
   }
   catch (ZdException &x) {
      x.AddCallpath("ReadScanningWindowSectionFromIni()", "CParameters");
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

// sets the global TimeParameters variables read in from the ini file
// pre: file is an open ini parameter file
// post: will set the global variables from the ini file
void CParameters::ReadTimeParametersSectionFromIni(ZdIniFile& file) {
   try {
      ZdIniSection* pSection = file.GetSection(TIME_PARAMS_SECTION);
      SetIntValue(m_nIntervalUnits, pSection->GetLine(pSection->FindKey(INTERVAL_UNITS_LINE))->GetValue(), TIMEINTLEN, 1);
      m_nIntervalLength = atol(pSection->GetLine(pSection->FindKey(INTERVAL_LENGTH_LINE))->GetValue());
      SetIntValue(m_nTimeAdjustType, pSection->GetLine(pSection->FindKey(TIME_TREND_ADJ_LINE))->GetValue(), TIMETREND, NOTADJUSTED);
      SetDoubleValue(m_nTimeAdjPercent, pSection->GetLine(pSection->FindKey(TIME_TREND_PERCENT_LINE))->GetValue(), TIMETRENDPERC, 0.0);
      strcpy(m_szProspStartDate, pSection->GetLine(pSection->FindKey(PROSPECT_START_LINE))->GetValue());
   }
   catch (ZdException &x) {
      x.AddCallpath("ReadTimeParametersSectionFromIni()", "CParameters");
      throw;
   }
}

// reports default value warnings to the user
// pre : !gvDefaultedValues.empty()
// post : writes out the variables that were defaulted
void CParameters::ReportDefaultValueWarnings() {
   try {
     unique(gvDefaultedValues.begin(), gvDefaultedValues.end());
     ZdString sWarning("Warning - default values were used for the following value(s) : \n");
     for(size_t i = 0; i < gvDefaultedValues.size(); ++i) {
        sWarning << (i == 0 ? "" : ", ") << mgsVariableLabels[gvDefaultedValues[i]-1];
     }
     //gpPrintDirection->SatScanPrintWarning(sWarning.GetCString());
   }
   catch (ZdException &x) {
      x.AddCallpath("ReportDefaultValueWarnings()", "CParameters");
      throw;
   }
}

// saves the model info section to the ini file
// pre: file is an open ZdIniFile
// post: write the appropraite global data to the file to the appropraite keys
void CParameters::SaveAdvancedFeaturesSection(ZdIniFile& file) {
   ZdString     sFloatValue;

   try {
      ZdIniSection* pSection = file.GetSection(ADVANCED_FEATURES_SECTION);
      pSection->SetString(ISOTONIC_SCAN_LINE, (m_nRiskFunctionType == MONOTONERISK) ? YES : NO);
      pSection->SetString(PVALUE_PROSPECT_LLR_LINE, m_bPowerCalc ? YES : NO);
      sFloatValue = m_nPower_X;
      pSection->SetString(LLR_1_LINE, sFloatValue.GetCString());
      sFloatValue = m_nPower_Y;
      pSection->SetString(LLR_2_LINE, sFloatValue.GetCString());
      pSection->SetString(VALID_PARAMS_LINE, m_bValidatePriorToCalc ? YES : NO);
   }
   catch (ZdException &x) {
      x.AddCallpath("SaveAdvancedFeaturesSection()", "CParameters");
      throw;
   }
}

// saves the Analysis section to the ini file
// pre: file is an open ZdIniFile
// post: write the appropraite global data to the file to the appropraite keys
void CParameters::SaveAnalysisSection(ZdIniFile& file) {
   try {
      ZdIniSection* pSection = file.GetSection(ANALYSIS_SECTION);
      pSection->SetInt(ANALYSIS_TYPE_LINE, m_nAnalysisType);
      pSection->SetInt(MODEL_TYPE_LINE, m_nModel);
      pSection->SetInt(SCAN_AREAS_LINE, m_nAreas);
      pSection->GetLine(pSection->FindKey(START_DATE_LINE))->SetValue(m_szStartDate);
      pSection->GetLine(pSection->FindKey(END_DATE_LINE))->SetValue(m_szEndDate);
      pSection->SetInt(MONTE_CARLO_REPS_LINE, m_nReplicas);
   }
   catch (ZdException &x) {
      x.AddCallpath("SaveAnalysisSection()", "CParameters");
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

      for (int i = 0; i < m_nNumEllipses; ++i)
         sShapes << (i == 0 ? "" : ",") << mp_dEShapes[i];
      pSection->GetLine(pSection->FindKey(ELLIPSE_SHAPES_LINE))->SetValue(sShapes.GetCString());

      for (int i = 0; i < m_nNumEllipses; ++i)
         sAngles << (i == 0 ? "" : ",") << mp_nENumbers[i];
      pSection->GetLine(pSection->FindKey(ELLIPSE_ANGLES_LINE))->SetValue(sAngles.GetCString());
      pSection->SetString(ELLIPSE_DUCZMAL_COMPACT_LINE, m_bDuczmalCorrectEllipses ? YES : NO);
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
      pSection->GetLine(pSection->FindKey(CASE_FILE_LINE))->SetValue(m_sCaseFileName.c_str());
      pSection->GetLine(pSection->FindKey(CONTROL_FILE_LINE))->SetValue(m_sControlFileName.c_str());
      pSection->GetLine(pSection->FindKey(POP_FILE_LINE))->SetValue(m_sPopulationFileName.c_str());
      pSection->GetLine(pSection->FindKey(COORD_FILE_LINE))->SetValue(m_sCoordinatesFileName.c_str());
      pSection->GetLine(pSection->FindKey(GRID_FILE_LINE))->SetValue(m_sSpecialGridFileName.c_str());
      pSection->SetString(USE_GRID_FILE_LINE, m_bSpecialGridFile ? YES : NO );
      pSection->SetInt(PRECISION_TIMES_LINE, m_nPrecision);
      pSection->SetInt(COORD_TYPE_LINE, m_nCoordType);
   }
   catch (ZdException &x) {
      x.AddCallpath("SaveInputFileSection()", "CParameters");
      throw;
   }
}

// saves the output file section to the ini file
// pre: file is an open ZdIniFile
// post: write the appropraite global data to the file to the appropraite keys
void CParameters::SaveOutputFileSection(ZdIniFile& file) {
   try {
      ZdIniSection* pSection = file.GetSection(OUTPUT_FILES_SECTION);
      pSection->GetLine(pSection->FindKey(RESULTS_FILE_LINE))->SetValue(m_sOutputFileName.c_str());
      pSection->SetString(CENSUS_REPORT_CLUSTERS_LINE, m_bOutputCensusAreas ? YES : NO);
      pSection->SetString(DBASE_CLUSTER_LINE, gbOutputClusterLevelDBF ? YES : NO);
      pSection->SetString(MOST_LIKELY_CLUSTER_LINE, m_bMostLikelyClusters ? YES : NO);
      pSection->SetString(DBASE_AREA_LINE, gbOutputAreaSpecificDBF ? YES : NO);
      pSection->SetString(INCLUDE_REL_RISKS_LINE, m_bOutputRelRisks ? YES : NO);
      pSection->SetString(DBASE_RELATIVE_RISKS, gbRelativeRiskDBF ? YES : NO);
      pSection->SetString(SAVE_SIM_LLRS, m_bSaveSimLogLikelihoods ? YES : NO);
      pSection->SetString(DBASE_LOG_LIKELI, gbLogLikelihoodDBF ? YES : NO);
      pSection->SetInt(CRIT_REPORT_SEC_CLUSTERS_LINE, m_iCriteriaSecondClusters);
   }
   catch (ZdException &x) {
      x.AddCallpath("SaveOutputFileSection()", "CParameters");
      throw;
   }
}

// saves the scanning window section to the ini file
// pre: file is an open ZdIniFile
// post: write the appropraite global data to the file to the appropraite keys
void CParameters::SaveScanningWindowSection(ZdIniFile& file) {
   ZdString     sFloatValue;

   try {
      ZdIniSection* pSection = file.GetSection(SCANNING_WINDOW_SECTION);
      sFloatValue = m_nMaxGeographicClusterSize;
      pSection->SetString(MAX_GEO_SIZE_LINE, sFloatValue.GetCString());
      sFloatValue = m_nMaxTemporalClusterSize;
      pSection->SetString(MAX_TEMP_SIZE_LINE, sFloatValue.GetCString());
      pSection->SetString(ALIVE_CLUSTERS_LINE, m_bAliveClustersOnly ? YES : NO );
      pSection->SetString(INCLUDE_PURELY_SPATIAL_LINE, m_bIncludePurelySpatial ? YES : NO);
      pSection->SetString(INCLUDE_PURE_TEMP_LINE, m_bIncludePurelyTemporal ? YES : NO);
      pSection->SetInt(MAX_TEMP_INTERPRET_LINE, m_nMaxClusterSizeType);
      pSection->SetInt(MAX_SPATIAL_SIZE_LINE, m_nMaxSpatialClusterSizeType);
   }
   catch (ZdException &x) {
      x.AddCallpath("SaveScanningWindowSection()", "CParameters");
      throw;
   }
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

// saves the time parameters section to the ini file
// pre: file is an open ZdIniFile
// post: write the appropraite global data to the file to the appropraite keys
void CParameters::SaveTimeParametersSection(ZdIniFile& file) {
   ZdString sFloat;

   try {
      ZdIniSection* pSection = file.GetSection(TIME_PARAMS_SECTION);
      pSection->SetInt(INTERVAL_LENGTH_LINE, m_nIntervalLength);
      pSection->SetInt(INTERVAL_UNITS_LINE, m_nIntervalUnits);
      pSection->GetLine(pSection->FindKey(PROSPECT_START_LINE))->SetValue(m_szProspStartDate);
      pSection->SetInt(TIME_TREND_ADJ_LINE, m_nTimeAdjustType);
      sFloat = m_nTimeAdjPercent;
      pSection->SetString(TIME_TREND_PERCENT_LINE, sFloat.GetCString());
   }
   catch (ZdException &x) {
      x.AddCallpath("SaveTimeParametersSection()", "CParameters");
      throw;
   }
}

/** Sets case data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file
    does not exist. */
void CParameters::SetCaseFileName(const char * sCaseFileName, bool bCorrectForRelativePath) {
  try {
    if (! sCaseFileName)
      ZdGenerateException("Null pointer.", "SetCaseFileName()");

    m_sCaseFileName = sCaseFileName;
    if (bCorrectForRelativePath)
      ConvertRelativePath(m_sCaseFileName);
  }
  catch (ZdException &x) {
    x.AddCallpath("SetCaseFileName()", "CParameters");
    throw;
  }
}

/** Sets control data file name. 
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file
    does not exist. */
void CParameters::SetControlFileName(const char * sControlFileName, bool bCorrectForRelativePath) {
  try {
    if (! sControlFileName)
      ZdGenerateException("Null pointer.", "SetControlFileName()");

    m_sControlFileName = sControlFileName;
    if (bCorrectForRelativePath)
      ConvertRelativePath(m_sControlFileName);
  }
  catch (ZdException &x) {
    x.AddCallpath("SetControlFileName()", "CParameters");
    throw;
  }
}

/** Sets coordinates data file name. 
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file
    does not exist. */
void CParameters::SetCoordinatesFileName(const char * sCoordinatesFileName, bool bCorrectForRelativePath) {
  try {
    if (! sCoordinatesFileName)
      ZdGenerateException("Null pointer.", "SetCoordinatesFileName()");

    m_sCoordinatesFileName = sCoordinatesFileName;
    if (bCorrectForRelativePath)
      ConvertRelativePath(m_sCoordinatesFileName);
  }
  catch (ZdException &x) {
    x.AddCallpath("SetCoordinatesFileName()", "CParameters");
    throw;
  }
}

// initializes global variables to default values
void CParameters::SetDefaults() {
  m_nAnalysisType             = PURELYSPATIAL;
  m_nAreas                    = HIGH;
  m_sCaseFileName = "";
  m_sPopulationFileName = "";
  m_sCoordinatesFileName = "";
  m_sOutputFileName = "";
  m_nPrecision                = YEAR;
  m_nDimension                = 0;
  m_bSpecialGridFile          = false;
  m_sSpecialGridFileName = "";
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
  gbLogRunHistory             = true;  

  m_nModel                 = POISSON;
  m_nRiskFunctionType      = STANDARDRISK;
  m_bPowerCalc             = false;
  m_nPower_X               = 0.0;
  m_nPower_Y               = 0.0;
  m_nTimeAdjustType        = NOTADJUSTED;
  m_nTimeAdjPercent        = 0;
  m_bIncludePurelyTemporal = false;
  m_sControlFileName = "";
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
  strcpy(m_szProspStartDate, "");
  m_bOutputCensusAreas     = false;
  m_bMostLikelyClusters    = false;
  m_iCriteriaSecondClusters = 0;
  m_lTotalNumEllipses      = 0;
  m_bDuczmalCorrectEllipses = false;
}

// helper function which will check the string to see if it is empty and if it is will add
// the line number (corresponds to line description) to defaulted value vector and set the Value to the default
// pre : none
// post : sets the Value to either default or the value read in from the file
void CParameters::SetDoubleValue(double &dValue, const std::string& sValueFromFile, int iLineNumberFromFile, double dDefaultValue) {
   if(sValueFromFile.empty()) {
      gvDefaultedValues.push_back(iLineNumberFromFile);
      dValue = dDefaultValue;
   }
   else
      dValue = atof(sValueFromFile.c_str());
}

// sets up the array of ellipse angles from the comma delimited string stored in the ini file
// pre : sAngles is the comma delimited line from the ini file
// post: allocates and assigns to the global array mp_dNumbers
void CParameters::SetEAnglesFromIniFile(const ZdString& sAngles) {
   ZdString     sTempAngles;

   try {
      ZdStringTokenizer angleTokenizer(sAngles, ",");
      while(angleTokenizer.HasMoreTokens()) {
         sTempAngles << angleTokenizer.GetNextToken() << " ";
      }
      LoadEAngles(sTempAngles.GetCString());
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
      ZdStringTokenizer shapeTokenizer(sShapes, ",");
      while(shapeTokenizer.HasMoreTokens()) {
         sTempShapes << shapeTokenizer.GetNextToken() << " ";
      }
      LoadEShapes(sTempShapes.GetCString());
   }
   catch (ZdException &x) {
      x.AddCallpath("SetEShapesFromIniFile()", "CParameters");
      throw;
   }
}

// helper function which will check the string to see if it is empty and if it is will add
// the line number (corresponds to line description) to defaulted value vector and set the Value to the default
// pre : none
// post : sets the Value to either default or the value read in from the file
void CParameters::SetFloatValue(float &fValue, const std::string& sValueFromFile, int iLineNumberFromFile, float fDefaultValue) {
   if(sValueFromFile.empty()) {
      gvDefaultedValues.push_back(iLineNumberFromFile);
      fValue = fDefaultValue;
   }
   else
      fValue = atof(sValueFromFile.c_str());
}

// helper function which will check the string to see if it is empty and if it is will add
// the line number (corresponds to line description) to defaulted value vector and set the Value to the default
// pre : none
// post : sets the Value to either default or the value read in from the file
void CParameters::SetIntValue(int &iValue, const std::string& sValueFromFile, int iLineNumberFromFile, int iDefaultValue) {
   if(sValueFromFile.empty()) {
      gvDefaultedValues.push_back(iLineNumberFromFile);
      iValue = iDefaultValue;
   }
   else
      iValue = atoi(sValueFromFile.c_str());
}

/** Sets output data file name. 
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file
    does not exist. */
void CParameters::SetOutputFileName(const char * sOutPutFileName, bool bCorrectForRelativePath) {
  try {
    if (! sOutPutFileName)
      ZdGenerateException("Null pointer.", "SetOutputFileName()");

    m_sOutputFileName = sOutPutFileName;
    if (bCorrectForRelativePath)
      ConvertRelativePath(m_sOutputFileName);
  }
  catch (ZdException &x) {
    x.AddCallpath("SetOutputFileName()", "CParameters");
    throw;
  }
}

// old method of setting the parameters from the old parameter file
void CParameters::SetParameter(int nParam, ZdString & szParam) {
  int           nScanCount, nTemp;

  try {
    switch (nParam) {
      case ANALYSISTYPE              : nScanCount=sscanf(szParam.GetCString(), "%i", &m_nAnalysisType); break;
      case SCANAREAS                 : nScanCount=sscanf(szParam.GetCString(), "%i", &m_nAreas); break;
      case CASEFILE                  : szParam.Deblank();
                                       SetCaseFileName(szParam.GetCString(), true);
                                       nScanCount=1; break;
      case POPFILE                   : szParam.Deblank();
                                       SetPopulationFileName(szParam.GetCString(), true);
                                       nScanCount=1; break;
      case COORDFILE                 : szParam.Deblank();
                                       SetCoordinatesFileName(szParam.GetCString(), true);
                                       nScanCount=1; break;
      case OUTPUTFILE                : szParam.Deblank();
                                       SetOutputFileName(szParam.GetCString(), true);
                                       nScanCount=1; break;
      case PRECISION                 : nScanCount=sscanf(szParam.GetCString(), "%i", &m_nPrecision); break;
      case DIMENSION                 : nScanCount=sscanf(szParam.GetCString(), "%i", &m_nDimension); break;
      case SPECIALGRID               : nScanCount=sscanf(szParam.GetCString(), "%i", &nTemp);
                                       m_bSpecialGridFile = (nTemp ? true : false);  break;
      case GRIDFILE                  : szParam.Deblank();
                                       SetSpecialGridFileName(szParam.GetCString(), true);
                                       nScanCount=1; break;
      case GEOSIZE                   : nScanCount=sscanf(szParam.GetCString(), "%f", &m_nMaxGeographicClusterSize); break;
      case STARTDATE                 : nScanCount=sscanf(szParam.GetCString(), "%s", &m_szStartDate); break;
      case ENDDATE                   : nScanCount=sscanf(szParam.GetCString(), "%s", &m_szEndDate); break;
      case CLUSTERS                  : nScanCount=sscanf(szParam.GetCString(), "%i", &nTemp);
                                       m_bAliveClustersOnly = (nTemp ? true : false); break;
      case EXACTTIMES                : nScanCount=sscanf(szParam.GetCString(), "%i", &nTemp);
                                       m_bExactTimes = (nTemp ? true : false); break;
      case INTERVALUNITS             : nScanCount=sscanf(szParam.GetCString(), "%i", &m_nIntervalUnits); break;
      case TIMEINTLEN                : nScanCount=sscanf(szParam.GetCString(), "%i", &m_nIntervalLength); break;
      case PURESPATIAL               : nScanCount=sscanf(szParam.GetCString(), "%i", &nTemp);
                                       m_bIncludePurelySpatial = (nTemp ? true : false); break;
      case TIMESIZE                  : nScanCount=sscanf(szParam.GetCString(), "%f", &m_nMaxTemporalClusterSize); break;
      case REPLICAS                  : nScanCount=sscanf(szParam.GetCString(), "%i", &m_nReplicas); break;
      case MODEL                     : nScanCount=sscanf(szParam.GetCString(), "%i", &m_nModel); break;
      case RISKFUNCTION              : nScanCount=sscanf(szParam.GetCString(), "%i", &m_nRiskFunctionType); break;
      case POWERCALC                 : nScanCount=sscanf(szParam.GetCString(), "%i", &nTemp);
                                       m_bPowerCalc = ( nTemp ? true : false); break;
      case POWERX                    : nScanCount=sscanf(szParam.GetCString(), "%lf", &m_nPower_X); break;
      case POWERY                    : nScanCount=sscanf(szParam.GetCString(), "%lf", &m_nPower_Y); break;
      case TIMETREND                 : nScanCount=sscanf(szParam.GetCString(), "%i", &m_nTimeAdjustType); break;
      case TIMETRENDPERC             : nScanCount=sscanf(szParam.GetCString(), "%lf", &m_nTimeAdjPercent); break;
      case PURETEMPORAL              : nScanCount=sscanf(szParam.GetCString(), "%i", &nTemp);
                                       m_bIncludePurelyTemporal = (nTemp ? true : false); break;
      case CONTROLFILE               : szParam.Deblank();
                                       SetControlFileName(szParam.GetCString(), true);
                                       nScanCount=1; break;
      case COORDTYPE                 : nScanCount=sscanf(szParam.GetCString(), "%i", &m_nCoordType); break;
      case SAVESIMLL                 : nScanCount=sscanf(szParam.GetCString(), "%i", &nTemp);
                                       m_bSaveSimLogLikelihoods = (nTemp ? true : false); break;
      case SEQUENTIAL                : nScanCount=sscanf(szParam.GetCString(), "%i", &nTemp);
                                       m_bSequential = (nTemp ? true : false); break;
      case SEQNUM                    : nScanCount=sscanf(szParam.GetCString(), "%i", &m_nAnalysisTimes); break;
      case SEQPVAL                   : nScanCount=sscanf(szParam.GetCString(), "%lf", &m_nCutOffPVal); break;
      case VALIDATE                  : nScanCount=sscanf(szParam.GetCString(), "%i", &nTemp);
                                       m_bValidatePriorToCalc = (nTemp ? true : false); break;
      case OUTPUTRR                  : nScanCount=sscanf(szParam.GetCString(), "%i", &nTemp);
                                       m_bOutputRelRisks = (nTemp ? true : false); break;
      case ELLIPSES                  : nScanCount=sscanf(szParam.GetCString(), "%i", &m_nNumEllipses); break;
      case ESHAPES                   : nScanCount = LoadEShapes(szParam.GetCString()); break;
      case ENUMBERS                  : nScanCount = LoadEAngles(szParam.GetCString()); break;
      case START_PROSP_SURV          : nScanCount=sscanf(szParam.GetCString(), "%s", &m_szProspStartDate);
                                       if ((nScanCount == 0) || (strlen(m_szProspStartDate)==1))
                                         strcpy(m_szProspStartDate, "");
                                       break;
      case OUTPUT_CENSUS_AREAS       : nScanCount=sscanf(szParam.GetCString(), "%i", &nTemp);
                                       m_bOutputCensusAreas = (nTemp ? true : false); break;
      case OUTPUT_MOST_LIKE_CLUSTERS : nScanCount=sscanf(szParam.GetCString(), "%i", &nTemp);
                                       m_bMostLikelyClusters = (nTemp ? true : false); break;
      case CRITERIA_SECOND_CLUSTERS  : nScanCount=sscanf(szParam.GetCString(), "%i", &m_iCriteriaSecondClusters); break;
      case MAX_TEMPORAL_TYPE         : nScanCount=sscanf(szParam.GetCString(), "%i", &m_nMaxClusterSizeType); break;
      case MAX_SPATIAL_TYPE          : nScanCount=sscanf(szParam.GetCString(), "%i", &m_nMaxSpatialClusterSizeType); break;
      case RUN_HISTORY_FILENAME      : //Run History no longer scanned from parameters file. Set through
                                       //setters/getters and copy() only.
                                       nScanCount=1; break;
      case OUTPUTCLUSTERDBF          : nScanCount=sscanf(szParam.GetCString(), "%i", &nTemp);
                                       gbOutputClusterLevelDBF = (nTemp ? true : false); break;
      case OUTPUTAREADBF             : nScanCount=sscanf(szParam.GetCString(), "%i", &nTemp);
                                       gbOutputAreaSpecificDBF = (nTemp ? true : false); break;
      case RELATIVE_RISK_DBF         : nScanCount=sscanf(szParam.GetCString(), "%i", &nTemp);
                                       gbRelativeRiskDBF = (nTemp ? true : false); break;
      case LOG_LIKELI_DBF            : nScanCount=sscanf(szParam.GetCString(), "%i", &nTemp);
                                       gbLogLikelihoodDBF = (nTemp ? true : false);  break;
    }

    //Mark parameter line as defaulted when scan fails.
    if (nScanCount != 1)
      gvDefaultedValues.push_back(nParam);
   }
   catch (ZdException & x) {
      x.AddCallpath("SetParameter()", "CParameters");
      throw;
   }
}

/** Sets coordinates data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file
    does not exist. */
void CParameters::SetPopulationFileName(const char * sPopulationFileName, bool bCorrectForRelativePath) {
  try {
    if (! sPopulationFileName)
      ZdGenerateException("Null pointer.", "SetPopulationFileName()");

    m_sPopulationFileName = sPopulationFileName;
    if (bCorrectForRelativePath)
      ConvertRelativePath(m_sPopulationFileName);
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPopulationFileName()", "CParameters");
    throw;
  }
}

// sets the global print direction pointer
void CParameters::SetPrintDirection(BasePrint *pPrintDirection) {
   gpPrintDirection = pPrintDirection;
}

/** Sets filename of file used to load parameters. */
void CParameters::SetSourceFileName(const char * sParametersSourceFileName) {
  try {
    if (! sParametersSourceFileName)
      ZdGenerateException("Null pointer.", "SetSourceFileName()");

    m_sParametersSourceFileName = sParametersSourceFileName;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetSourceFileName()", "CParameters");
    throw;
  }
}

/** Sets special grid data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file
    does not exist. */
void CParameters::SetSpecialGridFileName(const char * sSpecialGridFileName, bool bCorrectForRelativePath, bool bSetUsingFlag) {
  try {
    if (! sSpecialGridFileName)
      ZdGenerateException("Null pointer.", "SetSpecialGridFileName()");

    m_sSpecialGridFileName = sSpecialGridFileName;
    if (bCorrectForRelativePath)
      ConvertRelativePath(m_sSpecialGridFileName);

    if (m_sSpecialGridFileName.empty())
      m_bSpecialGridFile = false; //If empty, then definately not using special grid.
    else if (bSetUsingFlag)
      m_bSpecialGridFile = true;  //Permits setting special grid filename in GUI interface
                                  //where obviously the use of special grid file is the desire.
    //else m_bSpecialGridFile is as set from parameters read. This permits the situation
    //where user has modified the paramters file manually so that there is a named
    //special grid file but they turned off option to use it. 
  }
  catch (ZdException &x) {
    x.AddCallpath("SetSpecialGridFileName()", "CParameters");
    throw;
  }
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
        if (! gpPrintDirection)
          ZdGenerateException("No print direction set.", "ValidateParameters()");

        if (!(PURELYSPATIAL <= m_nAnalysisType && m_nAnalysisType <= PROSPECTIVESPACETIME)) {   
           bValid = false;
           gpPrintDirection->SatScanPrintWarning("Invalid parameter setting for analysis type. Please use a value between 1 and 3.\n");
        }

        if (!(HIGH <= m_nAreas && m_nAreas<= HIGHANDLOW)) {
           bValid = false;
           gpPrintDirection->SatScanPrintWarning("Invalid parameter setting for scan areas. Please use a value between 1 and 3.\n");
        }

        if (!(POISSON == m_nModel || m_nModel == BERNOULLI || m_nModel == SPACETIMEPERMUTATION)) {
           bValid = false;
           gpPrintDirection->SatScanPrintWarning("Invalid parameter setting for model type. Please use a value between 0 and 2.\n");
        }

        if (!(STANDARDRISK == m_nRiskFunctionType || m_nRiskFunctionType == MONOTONERISK)) {
           bValid = false;
           gpPrintDirection->SatScanPrintWarning("Invalid parameter setting for risk function type. Please use a value of 0 or 1.\n");
        }

        if (m_bSequential) {
          //if (!((1 <= m_nAnalysisTimes) && (m_nAnalysisTimes <= INT_MAX)))
          if (!(1 <= m_nAnalysisTimes)) {
             bValid = false;
             gpPrintDirection->SatScanPrintWarning("Invalid parameter setting for max sequential scan iterations. Please use a value between 1 and 32000.\n");
          }
          if (!(0.0 <= m_nCutOffPVal && m_nCutOffPVal <= 1.0)) {
             bValid = false;
             gpPrintDirection->SatScanPrintWarning("Invalid parameter setting for sequential p-value. Please use a value between 0 and 1.\n");
          }
        }
        else {
          m_nAnalysisTimes = 0;
          m_nCutOffPVal    = 0.0;
        }

        if (m_bPowerCalc) {
          if (!(0.0 <= m_nPower_X && m_nPower_X<= DBL_MAX)) {
             bValid = false;
             gpPrintDirection->SatScanPrintWarning("Invalid parameter setting for LLR#1. Please use a value between 0 and %12.4f\n", DBL_MAX);
          }
          if (!(0.0 <= m_nPower_Y && m_nPower_Y <= DBL_MAX)) {
             bValid = false;
             gpPrintDirection->SatScanPrintWarning("Invalid parameter setting for LLR#2. Please use a value between 0 and %12.4f\n", DBL_MAX);
          }
        }
        else {
          m_nPower_X = 0.0;
          m_nPower_Y = 0.0;
        }

        if (!(CARTESIAN == m_nCoordType || m_nCoordType == LATLON)) {
           bValid = false;
           gpPrintDirection->SatScanPrintWarning("Invalid parameter setting for coordinate type. Please use a value of 0 or 1.\n");
        }
        else if ((m_nCoordType == LATLON) && (m_nNumEllipses > 0)) {
           bValid = false;
           gpPrintDirection->SatScanPrintWarning("Invalid parameter setting for ellipses. Program currently does not support ellipses with lat/long coordinates.\n");
        }

        if (!ValidateReplications(m_nReplicas)) {
           bValid = false;
           gpPrintDirection->SatScanPrintWarning("Invalid parameter setting for Monte Carlo reps. Please use a value of 0, 9, 999, or n999.\n");
        }

        if (m_nAnalysisType == PURELYSPATIAL) {
          if (!(NONE <= m_nPrecision && m_nPrecision <= DAY)) {
            bValid = false;
            gpPrintDirection->SatScanPrintWarning("Invalid parameter setting for precision of case times. Please use a value between 0 and 3.\n");
          }
        }
        else {
          if (!(YEAR <= m_nPrecision && m_nPrecision <= DAY)) { // Change to DAYS, YEARS
             bValid = false;
             gpPrintDirection->SatScanPrintWarning("Invalid parameter setting for precision of case times. Please use a value between 1 and 3.\n");
          }
        }

        if (!ValidateDateString(m_szStartDate, STARTDATE)) {
           bValid = false;
           gpPrintDirection->SatScanPrintWarning("Invalid parameter setting for start date. Please use a valid date of the form YYYY/MM/DD.\n");
           bValidDate = false;
        }
        else if (!ValidateDateString(m_szEndDate, ENDDATE)) {
          bValid = false;
          gpPrintDirection->SatScanPrintWarning("Invalid parameter setting for end date. Please use a valid date of the form YYYY/MM/DD.\n");
          bValidDate = false;
        }
        else if (CharToJulian(m_szStartDate) >= CharToJulian(m_szEndDate)) {
          bValid = false;
          gpPrintDirection->SatScanPrintWarning("Invalid parameter setting for start date. Please use a valid date of the form YYYY/MM/DD.\n");
          bValidDate = false;
        }

        // Spatial Options
        if ((m_nAnalysisType == PURELYSPATIAL) || (m_nAnalysisType == SPACETIME) || (m_nAnalysisType == PROSPECTIVESPACETIME)) {
          if (m_nMaxSpatialClusterSizeType == PERCENTAGEOFMEASURETYPE && !(0.0 < m_nMaxGeographicClusterSize && m_nMaxGeographicClusterSize <= 50.0)) {
             bValid = false;
             gpPrintDirection->SatScanPrintWarning("Invalid parameter setting for max geographic size. Please use a value between 0 and 50.\n");
          }
          if (0.0 > m_nMaxGeographicClusterSize) {
            bValid = false;
            gpPrintDirection->SatScanPrintWarning("Invalid parameter setting for max geographic size. Please use a value between 0 and 50.\n");
          }
        }
        else
          m_nMaxGeographicClusterSize = 50.0; //KR980707 0 GG980716;

        // Temporal Options
        if ((m_nAnalysisType == PURELYTEMPORAL) || (m_nAnalysisType == SPACETIME) || (m_nAnalysisType == PROSPECTIVESPACETIME)) {
          if (m_nMaxClusterSizeType == PERCENTAGETYPE && !(0.0 < m_nMaxTemporalClusterSize && m_nMaxTemporalClusterSize <= (m_nModel == SPACETIMEPERMUTATION ? 50 : 90))) {
            bValid = false;
            gpPrintDirection->SatScanPrintWarning("Invalid parameter setting for max temporal size. Please use a value between 0 and %d.\n", (m_nModel == SPACETIMEPERMUTATION ? 50 : 90));
          }
          if (!(YEAR <= m_nIntervalUnits && m_nIntervalUnits <= m_nPrecision)) {
            gpPrintDirection->SatScanPrintWarning("Invalid parameter setting for interval units. Please use a value between 1 and 3.\n");
            bValid = false;
          }
          if (!(1 <= m_nIntervalLength && m_nIntervalLength <= TimeBetween(CharToJulian(m_szStartDate), CharToJulian(m_szEndDate), m_nIntervalUnits) )) {  // Change to Max Interval
            bValid = false;
            gpPrintDirection->SatScanPrintWarning("Invalid parameter setting for interval length. Please use a value between 1 and the length of the study period.\n");
          }
          if (m_nModel == BERNOULLI || m_nModel == SPACETIMEPERMUTATION) {
            m_nTimeAdjustType = NOTADJUSTED;
            m_nTimeAdjPercent = 0.0;
          }
          else {
            if (!(NOTADJUSTED <= m_nTimeAdjustType && m_nTimeAdjustType <= LINEAR)) {
               bValid = false;
               gpPrintDirection->SatScanPrintWarning("Invalid parameter setting for time adjustment type. Please use a value 0, 1 or 2.\n");
            }
            if (m_nTimeAdjustType == NONPARAMETRIC && m_nAnalysisType == PURELYTEMPORAL) {
              bValid = false;
              gpPrintDirection->SatScanPrintWarning("Invalid parameter setting for time adjustment type. You may not use non-parametric time in a Purely Temporal analysis.\n");
            }
            if (m_nTimeAdjustType == LINEAR)
              if (!(-100.0 < m_nTimeAdjPercent)) {
                bValid = false;
                gpPrintDirection->SatScanPrintWarning("Invalid parameter setting for time adjustment percentage. Please use a value greater than -100.\n");
              }
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

      if (m_sCaseFileName.empty() || (pFile = fopen(m_sCaseFileName.c_str(), "r")) == NULL) {
        bValid = false;
        gpPrintDirection->SatScanPrintWarning("Unable to open the case file %s. Please check to make sure the path is correct.\n",
                                              m_sCaseFileName.c_str());
      }
      else
        fclose(pFile);

      if (m_nModel == POISSON) {
        if (m_sPopulationFileName.empty() || (pFile = fopen(m_sPopulationFileName.c_str(), "r")) == NULL) {
           bValid = false;
           gpPrintDirection->SatScanPrintWarning("Unable to open the population file %s. Please check to make sure the path is correct.\n",
                                                  m_sPopulationFileName.c_str());
        }
        else
          fclose(pFile);
        m_sControlFileName = "";
      }
      else if (m_nModel == BERNOULLI) {
        if (m_sControlFileName.empty() || (pFile = fopen(m_sControlFileName.c_str(), "r")) == NULL) {
          bValid = false;
          gpPrintDirection->SatScanPrintWarning("Unable to open the population file %s. Please check to make sure the path is correct.\n",
                                                 m_sControlFileName.c_str());
        }
        else
          fclose(pFile);
        m_sPopulationFileName = "";
      }
      else if (m_nModel == SPACETIMEPERMUTATION) {
        if (m_nMaxSpatialClusterSizeType == PERCENTAGEOFMEASURETYPE)
          if (m_sPopulationFileName.empty() || (pFile = fopen(m_sPopulationFileName.c_str(), "r")) == NULL) {
            bValid = false;
            gpPrintDirection->SatScanPrintWarning("Unable to open the population file %s. Please check to make sure the path is correct.\n",
                                                  m_sPopulationFileName.c_str());
          }
          else
            fclose(pFile);
        if (!(m_nAnalysisType == SPACETIME || m_nAnalysisType == PROSPECTIVESPACETIME)) {
          bValid = false;
          gpPrintDirection->SatScanPrintWarning("Invalid parameter setting for analysis type. For Space-Time Permutation model,\n\t analysis type must be either Prospective or Retrospective Space-Time.\n");
        }
        if (m_bIncludePurelySpatial) {
          bValid = false;
          gpPrintDirection->SatScanPrintWarning("Invalid parameter setting for include purely spatial. Cannot include purely spatial clusters in the Space-Time model.\n");
        }
        if (m_bIncludePurelyTemporal) {
          bValid = false;
          gpPrintDirection->SatScanPrintWarning("Invalid parameter setting for include purely temporal. Cannot include purely temporal clusters in the Space-Time model.\n");
        }
        if (m_bOutputRelRisks || gbRelativeRiskDBF) {
          bValid = false;
          gpPrintDirection->SatScanPrintWarning("Invalid parameter setting for output relative risks. Cannot output relative risks for the Space-Time model.\n");
        }
      }

      if (m_sCoordinatesFileName.empty() || (pFile = fopen(m_sCoordinatesFileName.c_str(), "r")) == NULL) {
        bValid = false;
        gpPrintDirection->SatScanPrintWarning("Unable to open Coordinate file: %s. Please check to make sure the path is correct.\n",
                                              m_sCoordinatesFileName.c_str());
      }
      else
        fclose(pFile);

      if (m_bSpecialGridFile) {
        if (m_sSpecialGridFileName.empty() || (pFile = fopen(m_sSpecialGridFileName.c_str(), "r")) == NULL) {
          bValid = false;
          gpPrintDirection->SatScanPrintWarning("Unable to open Grid file: %s. Please check to make sure the path is correct.\n",
                                                m_sSpecialGridFileName.c_str());
        }
        else
          fclose(pFile);
      }
      else
        m_sSpecialGridFileName = "";

      if (m_sOutputFileName.empty() || (pFile = fopen(m_sOutputFileName.c_str(), "r")) == NULL) {
        if ((pFile = fopen(m_sOutputFileName.c_str(), "w")) == NULL) {
          bValid = false;
          gpPrintDirection->SatScanPrintWarning("Unable to open Results file: %s. Please check to make sure the path is correct.\n",
                                                m_sOutputFileName.c_str());
        }
        else
          fclose(pFile);
      }
      else
        fclose(pFile);

      if (!(m_iCriteriaSecondClusters>=0 && m_iCriteriaSecondClusters<=5)) {
          bValid = false;
          gpPrintDirection->SatScanPrintWarning("Invalid parameter setting for criteria for reporting secondary clusters.\n\tPlease use a value between 0 and 5.\n");
      }
      if (m_iCriteriaSecondClusters != 5 && m_nNumEllipses) {
          bValid = false;
          gpPrintDirection->SatScanPrintWarning("Invalid parameter setting for criteria for reporting secondary clusters.\n\tAnalyses with ellipses can have no restrictions.\n");
      }

      // Verify Character Prospective start date (YYYY/MM/DD).
      // THIS IS RUN AFTER STUDY DATES HAVE BEEN CHECKED !!!
      if (bValid && (m_nAnalysisType == PROSPECTIVESPACETIME) && bValidDate)
         if (! ValidateProspectiveStartDate(m_szProspStartDate, m_szStartDate, m_szEndDate)) {
            bValid = false;
            gpPrintDirection->SatScanPrintWarning("Invalid parameter setting for prospective start date. Please use a valid dat of the form YYYY/MM/DD.\n");
         }

      m_bExactTimes = 0;
      m_nDimension  = 0;

      // if no filename defined in the parameters file then we'll use the current working directory
      if (! ValidHistoryFileName(gsRunHistoryFilename)) {
        gpPrintDirection->SatScanPrintWarning("Run history file \"%s\" is not accessible.\nAnalysis history will not be recorded.\n\n",
                                              gsRunHistoryFilename.GetCString());
        gbLogRunHistory = false;
      }
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
bool CParameters::ValidHistoryFileName(ZdString& sRunHistoryFilename) {
   bool bValid(false);
   ZdString sExt(ZdFileName(sRunHistoryFilename.GetCString()).GetExtension());
   ZdString sDirectory(ZdFileName(sRunHistoryFilename.GetCString()).GetLocation());

   try {
      if(!sRunHistoryFilename.GetIsEmpty()) {
         if(sExt != ".dbf") {
            if(sExt.GetIsEmpty())
               sRunHistoryFilename << ".dbf";
            else
               sRunHistoryFilename.Replace(sExt.GetCString(), ".dbf");
         }

         // check to see if the directory exists and if it does assume we can write to it
         if(access(sDirectory.GetCString(), 00) == NULL)
            bValid = true;
      }
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
   return (sTestValue[0] == 'Y' || sTestValue[0] == 'y');
}

// checks the format of the ini file to be read to ensure that the correct lines exist
// pre: sFileName is the name of the ini file
// post: will throw an exception if one of the required ini lines is missing
void CParameters::VerifyIniFileSetup(ZdIniFile& file) {
   try {
      CheckIniSectionsExist(file);
      CheckInputFileSection(file);
      CheckAnalysisIniSection(file);
      CheckTimeParametersIniSection(file);
      CheckScanningWindowIniSection(file);
      CheckOutputFileIniSection(file);
      CheckSequentialScanIniSection(file);
      CheckEllipseIniSection(file);
      CheckAdvancedFeaturesIniSection(file);
   }
   catch (ZdException &x) {
      x.AddCallpath("VerifyIniFileSetup()", "CParameters");
      throw;
   }
}

// saves the parameters to an .ini file and replaces the existing ini if necessary
// pre: sFileName is the name of the .prm parameter file
// post: saves the parameters to an .ini file
void CParameters::Write(const char * sParameterFileName) {
   try {
      ZdIniFile file(sParameterFileName);
      VerifyIniFileSetup(file);

      SetSourceFileName(sParameterFileName);
      SaveInputFileSection(file);
      SaveAnalysisSection(file);
      SaveTimeParametersSection(file);
      SaveScanningWindowSection(file);
      SaveOutputFileSection(file);
      SaveEllipseSection(file);
      SaveSequentialScanSection(file);
      SaveAdvancedFeaturesSection(file);

      file.Write();
   }
   catch (ZdException &x) {
      x.AddCallpath("Write()", "CParameters");
      throw;
   }
}


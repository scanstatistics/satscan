//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "Parameters.h"
#include "DataSetHandler.h"
#include "ParameterFileAccess.h"
#include "RandomNumberGenerator.h"

#define INCLUDE_RUN_HISTORY

const char*      YES                            	= "y";
const char*      NO                             	= "n";
const int        MAXIMUM_SEQUENTIAL_ANALYSES    	= 32000;
const int        MAXIMUM_ELLIPSOIDS             	= 10;
int CParameters::giNumParameters 			= 70;

/** Constructor */
CParameters::CParameters() {
  SetAsDefaulted();
}

/** Copy constructor */
CParameters::CParameters(const CParameters &other) {
   SetAsDefaulted();
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

bool  CParameters::operator==(const CParameters& rhs) const {
  bool bEqual;

  if (giNumberEllipses                    != rhs.giNumberEllipses) return false;
  if (gvEllipseShapes                     != rhs.gvEllipseShapes) return false;
  if (gvEllipseRotations                  != rhs.gvEllipseRotations) return false;
  if (gbNonCompactnessPenalty             != rhs.gbNonCompactnessPenalty) return false;
  if (glTotalNumEllipses                  != rhs.glTotalNumEllipses) return false;
  if (geAnalysisType                      != rhs.geAnalysisType) return false;
  if (geAreaScanRate                      != rhs.geAreaScanRate) return false;
  if (geProbabilityModelType              != rhs.geProbabilityModelType) return false;
  if (geRiskFunctionType                  != rhs.geRiskFunctionType) return false;
  if (giReplications                      != rhs.giReplications) return false;
  if (gbPowerCalculation                  != rhs.gbPowerCalculation) return false;
  if (gdPower_X                           != rhs.gdPower_X) return false;
  if (gdPower_Y                           != rhs.gdPower_Y) return false;
  if (gsStudyPeriodStartDate              != rhs.gsStudyPeriodStartDate) return false;
  if (gsStudyPeriodEndDate                != rhs.gsStudyPeriodEndDate) return false;
  if (gfMaxGeographicClusterSize          != rhs.gfMaxGeographicClusterSize) return false;
  if (gfMaxTemporalClusterSize            != rhs.gfMaxTemporalClusterSize) return false;
  if (geIncludeClustersType               != rhs.geIncludeClustersType) return false;
  if (geTimeAggregationUnitsType          != rhs.geTimeAggregationUnitsType) return false;
  if (glTimeAggregationLength             != rhs.glTimeAggregationLength) return false;
  if (geTimeTrendAdjustType               != rhs.geTimeTrendAdjustType) return false;
  if (gdTimeTrendAdjustPercentage         != rhs.gdTimeTrendAdjustPercentage) return false;
  if (gbIncludePurelySpatialClusters      != rhs.gbIncludePurelySpatialClusters) return false;
  if (gbIncludePurelyTemporalClusters     != rhs.gbIncludePurelyTemporalClusters) return false;
  if (gvCaseFilenames                     != rhs.gvCaseFilenames) return false;
  if (gvControlFilenames                  != rhs.gvControlFilenames) return false;
  if (gvPopulationFilenames               != rhs.gvPopulationFilenames) return false;
  if (gsCoordinatesFileName               != rhs.gsCoordinatesFileName) return false;
  if (gsSpecialGridFileName               != rhs.gsSpecialGridFileName) return false;
  if (gbUseSpecialGridFile                != rhs.gbUseSpecialGridFile) return false;
  if (gsMaxCirclePopulationFileName       != rhs.gsMaxCirclePopulationFileName) return false;
  if (gePrecisionOfTimesType              != rhs.gePrecisionOfTimesType) return false;
  if (giDimensionsOfData                  != rhs.giDimensionsOfData) return false;
  if (geCoordinatesType                   != rhs.geCoordinatesType) return false;
  if (gsOutputFileName                    != rhs.gsOutputFileName) return false;
  if (gbOutputSimLogLikeliRatiosAscii     != rhs.gbOutputSimLogLikeliRatiosAscii) return false;
  if (gbOutputRelativeRisksAscii          != rhs.gbOutputRelativeRisksAscii) return false;
  if (gbSequentialRuns                    != rhs.gbSequentialRuns) return false;
  if (giNumSequentialRuns                 != rhs.giNumSequentialRuns) return false;
  if (gbSequentialCutOffPValue            != rhs.gbSequentialCutOffPValue) return false;
  if (gbValidatePriorToCalc               != rhs.gbValidatePriorToCalc) return false;
  if (gsProspectiveStartDate              != rhs.gsProspectiveStartDate) return false;
  if (gbOutputAreaSpecificAscii           != rhs.gbOutputAreaSpecificAscii) return false;
  if (gbOutputClusterLevelAscii           != rhs.gbOutputClusterLevelAscii) return false;
  if (geCriteriaSecondClustersType        != rhs.geCriteriaSecondClustersType) return false;
  if (geMaxTemporalClusterSizeType        != rhs.geMaxTemporalClusterSizeType) return false;
  if (geMaxGeographicClusterSizeType      != rhs.geMaxGeographicClusterSizeType) return false;
  if (gbOutputClusterLevelDBase           != rhs.gbOutputClusterLevelDBase) return false;
  if (gbOutputAreaSpecificDBase           != rhs.gbOutputAreaSpecificDBase) return false;
  if (gbOutputRelativeRisksDBase          != rhs.gbOutputRelativeRisksDBase) return false;
  if (gbOutputSimLogLikeliRatiosDBase     != rhs.gbOutputSimLogLikeliRatiosDBase) return false;
  if (gsRunHistoryFilename                != rhs.gsRunHistoryFilename) return false;
  if (gbLogRunHistory                     != rhs.gbLogRunHistory) return false;
  if (gsParametersSourceFileName          != rhs.gsParametersSourceFileName) return false;
  if (gsEndRangeStartDate                 != rhs.gsEndRangeStartDate) return false;
  if (gsEndRangeEndDate                   != rhs.gsEndRangeEndDate) return false;
  if (gsStartRangeStartDate               != rhs.gsStartRangeStartDate) return false;
  if (gsStartRangeEndDate                 != rhs.gsStartRangeEndDate) return false;
  if (gdTimeTrendConverge		  != rhs.gdTimeTrendConverge) return false;
  if (gbEarlyTerminationSimulations       != rhs.gbEarlyTerminationSimulations) return false;
  if (gbRestrictReportedClusters          != rhs.gbRestrictReportedClusters) return false;
  if (gfMaxReportedGeographicClusterSize  != rhs.gfMaxReportedGeographicClusterSize) return false;
  if (geSimulationType                    != rhs.geSimulationType) return false;
  if (gsSimulationDataSourceFileName      != rhs.gsSimulationDataSourceFileName) return false;
  if (gsAdjustmentsByRelativeRisksFileName!= rhs.gsAdjustmentsByRelativeRisksFileName) return false;
  if (gbOutputSimulationData              != rhs.gbOutputSimulationData) return false;
  if (gsSimulationDataOutputFilename      != rhs.gsSimulationDataOutputFilename) return false;
  if (gbAdjustForEarlierAnalyses          != rhs.gbAdjustForEarlierAnalyses) return false;
  if (gbUseAdjustmentsForRRFile           != rhs.gbUseAdjustmentsForRRFile) return false;
  if (geSpatialAdjustmentType             != rhs.geSpatialAdjustmentType) return false;
  if (geMultipleSetPurposeType            != rhs.geMultipleSetPurposeType) return false;
  //if (gCreationVersion                    != rhs.gCreationVersion) return false;
  if (gbUsePopulationFile                 != rhs.gbUsePopulationFile) return false;
  //if (glRandomizationSeed                 != rhs.glRandomizationSeed) return false;
  if (gbReportCriticalValues              != rhs.gbReportCriticalValues) return false;
  return true;
}

bool  CParameters::operator!=(const CParameters& rhs) const{
   return !(*this == rhs);
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
    geProbabilityModelType              = rhs.geProbabilityModelType;
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
    geMultipleSetPurposeType         = rhs.geMultipleSetPurposeType;
    gCreationVersion                    = rhs.gCreationVersion;
    gbUsePopulationFile                 = rhs.gbUsePopulationFile;
    glRandomizationSeed                 = rhs.glRandomizationSeed;
    gbReportCriticalValues              = rhs.gbReportCriticalValues;
  }
  catch (ZdException & x) {
    x.AddCallpath("Copy()", "CParameters");
    throw;
  }
}

/** Prints time trend adjustment parameters, in a particular format, to passed ascii file. */
void CParameters::DisplayAdjustments(FILE* fp, const DataSetHandler& SetHandler) const {
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
        DisplayCalculatedTimeTrend(fp, SetHandler); break;
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
        sBuffer = "Adjusted for purely spatial clusters by stratified randomization.";
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
void CParameters::DisplayAnalysisSummary(FILE* fp) const {
  try {
    switch (geAnalysisType) {
      case PURELYSPATIAL             : fprintf(fp, "Purely Spatial analysis\n"); break;
      case PURELYTEMPORAL            : fprintf(fp, "Retrospective Purely Temporal analysis\n"); break;
      case SPACETIME                 : fprintf(fp, "Retrospective Space-Time analysis\n"); break;
      case PROSPECTIVESPACETIME      : fprintf(fp, "Prospective Space-Time analysis\n"); break;
      case SPATIALVARTEMPTREND       : fprintf(fp, "Spatial Variation of Temporal Trends analysis\n"); break;
      case PROSPECTIVEPURELYTEMPORAL : fprintf(fp, "Prospective Purely Temporal analysis\n"); break;
      default : ZdException::Generate("Unknown analysis type '%d'.\n", "DisplayAnalysisSummary()", geAnalysisType);
    }

    fprintf(fp, "scanning for ");

    if (geAnalysisType == PURELYSPATIAL && geRiskFunctionType == MONOTONERISK)
      fprintf(fp, "monotone ");

    fprintf(fp, "clusters with ");

    switch (geAreaScanRate) {
      case HIGH       : fprintf(fp, "high rates\n"); break;
      case LOW        : fprintf(fp, "low rates\n"); break;
      case HIGHANDLOW : fprintf(fp, "high or low rates\n"); break;
      default : ZdException::Generate("Unknown area scan rate type '%d'.\n", "DisplayAnalysisSummary()", geAreaScanRate);
    }

    switch (geProbabilityModelType) {
      case POISSON              : fprintf(fp, "using the Poisson model.\n"); break;
      case BERNOULLI            : fprintf(fp, "using the Bernoulli model.\n"); break;
      case SPACETIMEPERMUTATION : fprintf(fp, "using the Space-Time Permutation model.\n"); break;
      case ORDINAL              : fprintf(fp, "using the Ordinal model.\n"); break;
      case EXPONENTIAL          : fprintf(fp, "using the Exponential model.\n"); break;
      case NORMAL               : fprintf(fp, "using the Normal model.\n"); break;
      case RANK                 : fprintf(fp, "using the Rank model.\n"); break;
      default : ZdException::Generate("Unknown probability model type '%d'.\n", "DisplayAnalysisSummary()", geProbabilityModelType);
    }

    if (geAnalysisType == SPACETIME || geAnalysisType == PROSPECTIVESPACETIME) {
      if (gbIncludePurelySpatialClusters && gbIncludePurelyTemporalClusters)
        fprintf(fp, "Analysis includes purely spatial and purely temporal clusters.\n");
      else if (gbIncludePurelySpatialClusters)
        fprintf(fp, "Analysis includes purely spatial clusters.\n");
      else if (gbIncludePurelyTemporalClusters)
        fprintf(fp, "Analysis includes purely temporal clusters.\n");
    }

    if (GetNumDataSets() > 1) {
      switch (geMultipleSetPurposeType) {
        case MULTIVARIATE : fprintf(fp, "Multivariate scan using %u data sets.\n", GetNumDataSets()); break;
        case ADJUSTMENT   : fprintf(fp, "Adjusted using %u data sets.\n", GetNumDataSets()); break;
        default : ZdException::Generate("Unknown purpose for multiple data sets type '%d'.\n", "DisplayAnalysisSummary()", geMultipleSetPurposeType);
      }
    }
    if (gbSequentialRuns)
      fprintf(fp, "Sequential analysis performed.\n");
  }
  catch (ZdException & x) {
    x.AddCallpath("DisplayAnalysisSummary()", "CParameters");
    throw;
  }
}

/** Prints parameters, in a particular format, to passed ascii file. */
void CParameters::DisplayParameters(FILE* fp, unsigned int iNumSimulationsCompleted, const DataSetHandler& SetHandler) const {
  int           i;
  size_t        t;
  ZdString      sBuffer;
  ZdFileName    AdditionalOutputFile(gsOutputFileName.c_str());

  try {
    AsciiPrintFormat::PrintSectionSeparatorString(fp, 1, 2);
    fprintf(fp, "PARAMETER SETTINGS\n\n");

    fprintf(fp, "Input Files\n");
    fprintf(fp, "-----------\n");
    if (gvCaseFilenames.size() == 1)
      fprintf(fp, "  Case File                    : %s\n", gvCaseFilenames[0].c_str());
    else {
      for (t=0; t < gvCaseFilenames.size(); ++t)
         fprintf(fp, "  Case File       (data set %i) : %s\n", t + 1, gvCaseFilenames[t].c_str());
    }

    switch (geProbabilityModelType) {
      case POISSON              : if (!UsePopulationFile()) break;
                                  if (gvPopulationFilenames.size() == 1)
                                    fprintf(fp, "  Population File              : %s\n", gvPopulationFilenames[0].c_str());
                                  else {
                                    for (t=0; t < gvPopulationFilenames.size(); ++t)
                                       fprintf(fp, "  Population File (data set %i) : %s\n", t + 1, gvPopulationFilenames[t].c_str());
                                  }
                                  break;
      case BERNOULLI            : if (gvControlFilenames.size() == 1)
                                    fprintf(fp, "  Control File                 : %s\n", gvControlFilenames[0].c_str());
                                  else {
                                    for (t=0; t < gvControlFilenames.size(); ++t)
                                       fprintf(fp, "  Control File    (data set %i) : %s\n", t + 1, gvControlFilenames[t].c_str());
                                  }
                                  break;
      case SPACETIMEPERMUTATION :
      case ORDINAL              :
      case EXPONENTIAL          :
      case NORMAL               :
      case RANK                 : break;
      default : ZdException::Generate("Unknown probability model type '%d'.\n", "DisplayParameters()", geProbabilityModelType);
    }

    fprintf(fp, "  Coordinates File             : %s\n", gsCoordinatesFileName.c_str());
    if (gbUseSpecialGridFile)
      fprintf(fp, "  Grid File                    : %s\n", gsSpecialGridFileName.c_str());
    if (geMaxGeographicClusterSizeType == PERCENTOFPOPULATIONFILETYPE)
      fprintf(fp, "  Max Circle Size File         : %s\n", gsMaxCirclePopulationFileName.c_str());
    if (geSimulationType == FILESOURCE)
      fprintf(fp, "  Simulated Data Import File   : %s\n", gsSimulationDataSourceFileName.c_str());
    if(geSimulationType == HA_RANDOMIZATION || gbUseAdjustmentsForRRFile)
      fprintf(fp, "  Adjustments File             : %s\n", gsAdjustmentsByRelativeRisksFileName.c_str());

    DatePrecisionType ePrecision;
    fprintf(fp, "\n  Time Precision     : ");
    //Display precision, keeping in mind the v4 behavior.
    if (gePrecisionOfTimesType == NONE)
      ePrecision = NONE;
    else if (gCreationVersion.iMajor == 4)
      ePrecision = (geAnalysisType == PURELYSPATIAL ? YEAR : geTimeAggregationUnitsType);
    else
      ePrecision =  gePrecisionOfTimesType;
    switch (ePrecision) {
      case YEAR  : fprintf(fp, "Year\n"); break;
      case MONTH : fprintf(fp, "Month\n"); break;
      case DAY   : fprintf(fp, "Day\n"); break;
      default    : fprintf(fp, "None\n"); break;;
    }

    fprintf(fp, "  Coordinates        : ");
    switch (geCoordinatesType) {
      case CARTESIAN : fprintf(fp, "Cartesian\n"); break;
      case LATLON    : fprintf(fp, "Latitude/Longitude\n"); break;
      default : ZdException::Generate("Unknown coordinated type '%d'.\n", "DisplayParameters()", geCoordinatesType);
    }
    if (GetNumDataSets() > 1) {
      fprintf(fp, "  Purpose of Multiple Data Sets : ");
      switch (geMultipleSetPurposeType) {
        case MULTIVARIATE : fprintf(fp, "Multivariate Analysis\n"); break;
        case ADJUSTMENT    : fprintf(fp, "Adjustment\n"); break;
        default : ZdException::Generate("Unknown purpose for multiple data sets type '%d'.\n",
                                        "DisplayParameters()", geMultipleSetPurposeType);
      }
    }
    
    fprintf(fp, "\nAnalysis\n");
    fprintf(fp, "--------\n");

    fprintf(fp, "  Type of Analysis    : %s\n", GetAnalysisTypeAsString());
    fprintf(fp, "  Probability Model   : %s\n", GetProbabilityModelTypeAsString(geProbabilityModelType));

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

    if (glRandomizationSeed != RandomNumberGenerator::glDefaultSeed)
       fprintf(fp, "  Randomization Seed     : %ld\n", glRandomizationSeed);
    fprintf(fp, "  Report Critical Values : %s\n", (gbReportCriticalValues ? "Yes" : "No"));

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

    if (geProbabilityModelType != SPACETIMEPERMUTATION && (geAnalysisType == SPACETIME || geAnalysisType == PROSPECTIVESPACETIME)) {
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

    if (geProbabilityModelType != SPACETIMEPERMUTATION && (geAnalysisType == SPACETIME || geAnalysisType == PROSPECTIVESPACETIME)) {
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

      if (geProbabilityModelType == POISSON) {
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
    }

    if (geProbabilityModelType == POISSON && (geAnalysisType == SPACETIME || geAnalysisType == PROSPECTIVESPACETIME)) {
      fprintf(fp, "  Spatial Adjustment  : ");
      switch (geSpatialAdjustmentType) {
        case NO_SPATIAL_ADJUSTMENT :
          fprintf(fp, "None\n"); break;
        case SPATIALLY_STRATIFIED_RANDOMIZATION :
          fprintf(fp, "Spatial adjustment by stratified randomization\n"); break;
        default :
          ZdException::Generate("Unknown spatial adjustment type '%d'.\n", "DisplayParameters()", geSpatialAdjustmentType);
      }
    }  

    if (geAnalysisType == PROSPECTIVESPACETIME || geAnalysisType == PROSPECTIVEPURELYTEMPORAL) {
      fprintf(fp, "  Adjusted for Earlier Analyses : %s\n", (gbAdjustForEarlierAnalyses ? "Yes" : "No"));
      if (gbAdjustForEarlierAnalyses)
        fprintf(fp, "  Prospective Start Date : %s\n", gsProspectiveStartDate.c_str());
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
void CParameters::DisplayCalculatedTimeTrend(FILE* fp, const DataSetHandler& SetHandler) const {
  unsigned int                  t, iStart, iScan;
  ZdString                      sPrintString, sWorkString;
  std::deque<unsigned int>      TrendIncrease, TrendDecrease;

  if (geTimeTrendAdjustType != CALCULATED_LOGLINEAR_PERC)
    return;

  //NOTE: Each dataset has own calculated time trend.

  if (SetHandler.GetNumDataSets() == 1) {
    if (SetHandler.GetDataSet(0).GetCalculatedTimeTrendPercentage() < 0)
      sPrintString.printf("Adjusted for time trend with an annual decrease ");
    else
      sPrintString.printf("Adjusted for time trend with an annual increase ");
    sWorkString.printf("of %0.2f%%.", fabs(SetHandler.GetDataSet(0).GetCalculatedTimeTrendPercentage()));
    sPrintString << sWorkString;
  }
  else {//multiple datasets print
    //count number of increasing and decreasing trends
    for (t=0; t < SetHandler.GetNumDataSets(); ++t) {
       if (SetHandler.GetDataSet(t).GetCalculatedTimeTrendPercentage() < 0)
         TrendDecrease.push_back(t);
       else
         TrendIncrease.push_back(t);
    }
    //now print
    sPrintString.printf("Adjusted for time trend with an annual ");
    //print increasing trends first
    if (TrendIncrease.size()) {
       sWorkString.printf("increase of %0.2f%%",
                          fabs(SetHandler.GetDataSet(TrendIncrease.front()).GetCalculatedTimeTrendPercentage()));
       sPrintString << sWorkString;
       for (t=1; t < TrendIncrease.size(); ++t) {
          sWorkString.printf((t < TrendIncrease.size() - 1) ? ", %0.2f%%" : " and %0.2f%%",
                             fabs(SetHandler.GetDataSet(TrendIncrease[t]).GetCalculatedTimeTrendPercentage()));
          sPrintString << sWorkString;
       }
       sWorkString.printf(" for data set%s %u", (TrendIncrease.size() == 1 ? "" : "s"), TrendIncrease.front() + 1);
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
                         fabs(SetHandler.GetDataSet(TrendDecrease.front()).GetCalculatedTimeTrendPercentage()));
      sPrintString << sWorkString;
      for (t=1; t < TrendDecrease.size(); ++t) {
         sWorkString.printf((t < TrendDecrease.size() - 1) ? ", %0.2f%%" : " and %0.2f%%",
                            fabs(SetHandler.GetDataSet(TrendDecrease[t]).GetCalculatedTimeTrendPercentage()));
         sPrintString << sWorkString;
      }
      sWorkString.printf(" for data set%s %u", (TrendDecrease.size() == 1 ? "" : "s"), TrendDecrease.front() + 1);
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
      case PURELYSPATIAL             : sAnalysisType = "Purely Spatial"; break;
      case PURELYTEMPORAL            : sAnalysisType = "Retrospective Purely Temporal"; break;
      case SPACETIME                 : sAnalysisType = "Retrospective Space-Time"; break;
      case PROSPECTIVESPACETIME      : sAnalysisType = "Prospective Space-Time"; break;
      case SPATIALVARTEMPTREND       : sAnalysisType = "Spatial Variation in Temporal Trends"; break;
      case PROSPECTIVEPURELYTEMPORAL : sAnalysisType = "Prospective Purely Temporal"; break;
      default : ZdException::Generate("Unknown analysis type '%d'.\n", "GetAnalysisTypeAsString()", geAnalysisType);
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("GetAnalysisTypeAsString()","CParameters");
    throw;
  }
  return sAnalysisType;
}

const std::string & CParameters::GetCaseFileName(size_t iSetIndex) const {
  try {
    if (!iSetIndex || iSetIndex > gvCaseFilenames.size())
      ZdGenerateException("Index out of range.","GetCaseFileName()");
  }
  catch (ZdException & x) {
    x.AddCallpath("GetCaseFileName()","CParameters");
    throw;
  }
  return gvCaseFilenames[iSetIndex - 1];
}

const std::string & CParameters::GetControlFileName(size_t iSetIndex) const {
  try {
    if (!iSetIndex || iSetIndex > gvControlFilenames.size())
      ZdGenerateException("Index out of range.","GetControlFileName()");
  }
  catch (ZdException & x) {
    x.AddCallpath("GetControlFileName()","CParameters");
    throw;
  }
  return gvControlFilenames[iSetIndex - 1];
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
  return (geProbabilityModelType == SPACETIMEPERMUTATION || (giNumberEllipses && gbNonCompactnessPenalty));
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

/** returns whether analysis type permits inclusion of purely spatial cluster */
bool CParameters::GetPermitsPurelySpatialCluster(AnalysisType eAnalysisType) const {
  return geAnalysisType == PURELYSPATIAL || geAnalysisType == SPACETIME || GetIsProspectiveAnalysis();
}

/** returns whether probability model type permits inclusion of purely spatial cluster */
bool CParameters::GetPermitsPurelySpatialCluster(ProbabilityModelType eModelType) const {
  return eModelType == POISSON || eModelType == BERNOULLI || eModelType == NORMAL
         || eModelType == EXPONENTIAL || eModelType == RANK || eModelType == ORDINAL;
}

/** returns whether analysis type permits inclusion of purely temporal cluster */
bool CParameters::GetPermitsPurelyTemporalCluster(AnalysisType eAnalysisType) const {
  return geAnalysisType == PURELYTEMPORAL || geAnalysisType == SPACETIME || GetIsProspectiveAnalysis();
}

/** returns whether probability model type permits inclusion of purely temporal cluster */
bool CParameters::GetPermitsPurelyTemporalCluster(ProbabilityModelType eModelType) const {
  return eModelType == POISSON || eModelType == BERNOULLI || eModelType == NORMAL
         || eModelType == EXPONENTIAL || eModelType == RANK || eModelType == ORDINAL;
}

const std::string & CParameters::GetPopulationFileName(size_t iSetIndex) const {
  try {
    if (!iSetIndex || iSetIndex > gvPopulationFilenames.size())
      ZdGenerateException("Index out of range.","GetPopulationFileName()");
  }
  catch (ZdException & x) {
    x.AddCallpath("GetPopulationFileName()","CParameters");
    throw;
  }
  return gvPopulationFilenames[iSetIndex - 1];
}

/** Returns probability model type as a character array. */
const char * CParameters::GetProbabilityModelTypeAsString(ProbabilityModelType eProbabilityModelType) const {
  const char * sProbabilityModel;

  try {
    switch (eProbabilityModelType) {
      case POISSON              : sProbabilityModel = "Poisson"; break;
      case BERNOULLI            : sProbabilityModel = "Bernoulli"; break;
      case SPACETIMEPERMUTATION : sProbabilityModel = "Space-Time Permutation"; break;
      case ORDINAL              : sProbabilityModel = "Ordinal"; break;
      case EXPONENTIAL          : sProbabilityModel = "Exponential"; break;
      case NORMAL               : sProbabilityModel = "Normal"; break;
      case RANK                 : sProbabilityModel = "Rank"; break;
      default : ZdException::Generate("Unknown probability model type '%d'.\n", "GetProbabilityModelTypeAsString()", geProbabilityModelType);
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("GetProbabilityModelTypeAsString()","CParameters");
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

/** Sets start range start date. Throws exception. */
void CParameters::SetStartRangeStartDate(const char * sStartRangeStartDate) {
  ZdString      sLabel;

  try {
    if (!sStartRangeStartDate)
      ZdException::Generate("Null pointer.","SetStartRangeStartDate()");

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

    gsStartRangeEndDate = sStartRangeEndDate;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetStartRangeEndDate()","CParameters");
    throw;
  }
}

/** Sets analysis type. Throws exception if out of range. */
void CParameters::SetAnalysisType(AnalysisType eAnalysisType) {
  ZdString      sLabel;

  try {
    if (eAnalysisType < PURELYSPATIAL || eAnalysisType > PROSPECTIVEPURELYTEMPORAL)
      ZdException::Generate("'%d' is out of range(%d - %d).", "SetAnalysisType()", eAnalysisType, PURELYSPATIAL, PROSPECTIVEPURELYTEMPORAL);
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
      ZdException::Generate("'%d' is out of range(%d - %d).", "SetAreaRateType()", eAreaRateType, HIGH, HIGHANDLOW);
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
void CParameters::SetCaseFileName(const char * sCaseFileName, bool bCorrectForRelativePath, size_t iSetIndex) {
  try {
    if (! sCaseFileName)
      ZdGenerateException("Null pointer.", "SetCaseFileName()");

    if (!iSetIndex)
      ZdGenerateException("Index out of range.", "SetCaseFileName()");

    if (iSetIndex > gvCaseFilenames.size())
      gvCaseFilenames.resize(iSetIndex);

    gvCaseFilenames[iSetIndex - 1] = sCaseFileName;
    if (bCorrectForRelativePath)
      ConvertRelativePath(gvCaseFilenames[iSetIndex - 1]);
  }
  catch (ZdException &x) {
    x.AddCallpath("SetCaseFileName()", "CParameters");
    throw;
  }
}

/** Sets control data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void CParameters::SetControlFileName(const char * sControlFileName, bool bCorrectForRelativePath, size_t iSetIndex) {
  try {
    if (! sControlFileName)
      ZdGenerateException("Null pointer.", "SetControlFileName()");

    if (!iSetIndex)
      ZdGenerateException("Index out of range.", "SetControlFileName()");

    if (iSetIndex > gvControlFilenames.size())
      gvControlFilenames.resize(iSetIndex);

    gvControlFilenames[iSetIndex - 1] = sControlFileName;
    if (bCorrectForRelativePath)
      ConvertRelativePath(gvControlFilenames[iSetIndex - 1]);
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
      ZdException::Generate("'%d' is out of range(%d - %d).", "SetCoordinatesType()", eCoordinatesType, CARTESIAN, LATLON);
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
      ZdException::Generate("'%d' is out of range(%d - %d).", "SetCriteriaForReportingSecondaryClusters()",
                            eCriteriaSecondaryClustersType, NOGEOOVERLAP, NORESTRICTIONS);
    geCriteriaSecondClustersType = eCriteriaSecondaryClustersType;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetCriteriaForReportingSecondaryClusters()","CParameters");
    throw;
  }
}

/** initializes global variables to default values */
void CParameters::SetAsDefaulted() {
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
  gsStudyPeriodStartDate                = "2000/1/1";
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
  geProbabilityModelType                 = POISSON;
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
  geMultipleSetPurposeType           = MULTIVARIATE;
  //default to 4.0.3, the last version prior to 'version' parameter
  gCreationVersion.iMajor               = 4;
  gCreationVersion.iMinor               = 0;
  gCreationVersion.iRelease             = 3;
  gbUsePopulationFile                   = false;
  glRandomizationSeed                   = RandomNumberGenerator::glDefaultSeed;
  gbReportCriticalValues                = false;
}

/** Sets dimensions of input data. */
void CParameters::SetDimensionsOfData(int iDimensions) {
  try {
    if (iDimensions < 0)
      ZdException::Generate("Dimensions can not be less than 0.\n", "SetDimensionsOfData()");
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
      ZdException::Generate("Index '%d' out of range(0 - %d).", "SetEllipsoidShape()", iEllipsoidIndex, giNumberEllipses - 1);

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
      ZdException::Generate("'%d' is out of range(%d - %d).", "SetIncludeClustersType()",
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
      ZdException::Generate("'%d' is out of range(%d - %d).", "SetMaximumSpacialClusterSizeType()",
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
      ZdException::Generate("'%d' is out of range(%d - %d).","SetMaximumTemporalClusterSizeType()",
                            eTemporalSizeType, PERCENTAGETYPE, TIMETYPE);
    geMaxTemporalClusterSizeType = eTemporalSizeType;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetMaximumTemporalClusterSizeType()","CParameters");
    throw;
  }
}

/** Adjusts the number of data sets. */
void CParameters::SetNumDataSets(size_t iNumDataSets) {
  size_t        t;

  try {
    if (iNumDataSets == 0)
      ZdException::Generate("Number of data sets can not be zero.\n", "SetNumDataSets()");

    //adjust the number of filenames for case, control, and population
    gvCaseFilenames.resize(iNumDataSets);
    gvControlFilenames.resize(iNumDataSets);
    gvPopulationFilenames.resize(iNumDataSets);
  }
  catch (ZdException & x) {
    x.AddCallpath("SetNumDataSets()","CParameters");
    throw;
  }
}

/** Sets number of ellipses requested. */
void CParameters::SetNumberEllipses(int iNumEllipses) {
  ZdString      sLabel;

  try {
    if (iNumEllipses < 0)
      ZdException::Generate("Number of ellipses can not be less than 0.", "SetNumberEllipses()", iNumEllipses);
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
void CParameters::SetPopulationFileName(const char * sPopulationFileName, bool bCorrectForRelativePath, size_t tSetIndex) {
  try {
    if (! sPopulationFileName)
      ZdGenerateException("Null pointer.", "SetPopulationFileName()");

    if (!tSetIndex)
      ZdGenerateException("Index out of range.", "SetPopulationFileName()");

    if (tSetIndex > gvPopulationFilenames.size())
      gvPopulationFilenames.resize(tSetIndex);

    gvPopulationFilenames[tSetIndex - 1] = sPopulationFileName;
    if (bCorrectForRelativePath)
      ConvertRelativePath(gvPopulationFilenames[tSetIndex - 1]);
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
      ZdException::Generate("'%d' is out of range(%d - %d).", "SetPrecisionOfTimesType()", eDatePrecisionType, NONE, DAY);
    gePrecisionOfTimesType = eDatePrecisionType;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPrecisionOfTimesType()","CParameters");
    throw;
  }
}

/** Sets probability model type. Throws exception if out of range. */
void CParameters::SetProbabilityModelType(ProbabilityModelType eProbabilityModelType) {
  ZdString      sLabel;

  try {
    if (eProbabilityModelType < POISSON || eProbabilityModelType > RANK)
      ZdException::Generate("'%d' is out of range(%d - %d).", "SetAnalysisType()", eProbabilityModelType, POISSON, RANK);

    geProbabilityModelType = eProbabilityModelType;
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

//    if (strspn(sProspectiveStartDate,"0123456789/") < strlen(sProspectiveStartDate))
//     ZdException::Generate("Error: For parameter %s, setting '%s' does not appear to be a date.\n",
//                                          "SetProspectiveStartDate()",
//                                          GetParameterLineLabel(START_PROSP_SURV, sLabel, geReadType == INI),
//                                          sProspectiveStartDate);
    gsProspectiveStartDate = sProspectiveStartDate;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetProspectiveStartDate()","CParameters");
    throw;
  }
}

/** Set seed used by randomization process. */
void CParameters::SetRandomizationSeed(long lSeed) {
  //Validity of setting is checked in ValidateParameters().
  glRandomizationSeed = lSeed;
}

/** Sets risk type. Throws exception if out of range. */
void CParameters::SetRiskType(RiskType eRiskType) {
  ZdString      sLabel;

  try {
    if (eRiskType < STANDARDRISK || eRiskType > MONOTONERISK)
      ZdException::Generate("'%d' is out of range(%d - %d).", "SetRiskType()", eRiskType, STANDARDRISK, MONOTONERISK);
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
      ZdException::Generate("'%d' is out of range(%d - %d).", "SetSimulationType()", eSimulationType, STANDARD, FILESOURCE);
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
      ZdException::Generate("'%d' is out of range(%d - %d).", "SetSpatialAdjustmentType()", eSpatialAdjustmentType, NO_SPATIAL_ADJUSTMENT, SPATIALLY_STRATIFIED_RANDOMIZATION);
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

/** Set multiple dataset purpose type. Throws exception if out of range. */
void CParameters::SetMultipleDataSetPurposeType(MultipleDataSetPurposeType eType) {
  ZdString      sLabel;

  try {
    if (eType < MULTIVARIATE || eType > ADJUSTMENT)
      ZdException::Generate("'%d' is out of range(%d - %d).", "SetMultipleDataSetPurposeType()", eType, MULTIVARIATE, ADJUSTMENT);
    geMultipleSetPurposeType = eType;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetMultipleDataSetPurposeType()","CParameters");
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
      ZdException::Generate("'%d' is out of range(%d - %d).","SetTimeAggregationUnitsType()", eTimeAggregationUnits, NONE, DAY);
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
      ZdException::Generate("'%d' is out of range(%d - %d).", "SetTimeTrendAdjustmentType()",
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
void CParameters::SetVersion(const CreationVersion& vVersion) {
  gCreationVersion = vVersion;
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
    if (GetIsProspectiveAnalysis() && gbAdjustForEarlierAnalyses && !ValidateProspectiveDate(PrintDirection)) {
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
      if (bValid && GetIsProspectiveAnalysis() && gbAdjustForEarlierAnalyses && bProspectiveDateValid) {
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
  DatePrecisionType     ePrecision;

  try {
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
    if (gCreationVersion.iMajor == 4)
      // no date precision validation needed for purely spatial
      ePrecision = (geAnalysisType == PURELYSPATIAL ? NONE : geTimeAggregationUnitsType);
    else
      ePrecision = gePrecisionOfTimesType;
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
    if (giNumberEllipses < 0 || giNumberEllipses > MAXIMUM_ELLIPSOIDS) {
      bValid = false;
      PrintDirection.SatScanPrintWarning("Error: The number of requested ellipses '%d' is not within allowable range of 0 - %d.\n",
                                          giNumberEllipses, MAXIMUM_ELLIPSOIDS);
    }
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
bool CParameters::ValidateFileParameters(BasePrint& PrintDirection) {
  bool          bValid=true;
  size_t        t;

  try {
    //validate number of datasets files match

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
    if (geProbabilityModelType == POISSON ) {
      //special processing for purely temporal analyses - population file is optional
      if (GetIsPurelyTemporalAnalysis()) {
         //either all datasets omit the population file or specify a filename
         unsigned int iNumDataSetsWithoutPopFile=0;
         for (t=0; t < gvPopulationFilenames.size(); ++t)
            if (gvPopulationFilenames[0].empty())
              ++iNumDataSetsWithoutPopFile;
         if (iNumDataSetsWithoutPopFile && iNumDataSetsWithoutPopFile != gvPopulationFilenames.size()) {
            bValid = false;
            PrintDirection.SatScanPrintWarning("Error: For the Poisson model with purely temporal analyses, the population file\n");
            PrintDirection.SatScanPrintWarning("       is optional but all data sets must either specify a population file or omit it.\n");
         }
         else if (!iNumDataSetsWithoutPopFile) {
           gbUsePopulationFile = true;
           for (t=0; t < gvPopulationFilenames.size(); ++t) {
              if (access(gvPopulationFilenames[t].c_str(), 00)) {
                bValid = false;
                PrintDirection.SatScanPrintWarning("Error: The population file '%s' does not exist.\n", gvPopulationFilenames[t].c_str());
                PrintDirection.SatScanPrintWarning("       Please check to make sure the path is correct.\n");
              }
           }
         }
      }
      else {
        gbUsePopulationFile = true;
        if (!gvPopulationFilenames.size()) {
          bValid = false;
          PrintDirection.SatScanPrintWarning("Error: For the Poisson model, a population file must be specified unless analysis\n");
          PrintDirection.SatScanPrintWarning("       is purely temporal. In which case the population file is optional.\n");
        }
        for (t=0; t < gvPopulationFilenames.size(); ++t) {
          if (access(gvPopulationFilenames[t].c_str(), 00)) {
            bValid = false;
            PrintDirection.SatScanPrintWarning("Error: The population file '%s' does not exist.\n", gvPopulationFilenames[t].c_str());
            PrintDirection.SatScanPrintWarning("       Please check to make sure the path is correct.\n");
          }
        }
      }
    }
    //validate control file for a bernoulli model.
    if (geProbabilityModelType == BERNOULLI) {
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
    if (geProbabilityModelType == POISSON) {
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
      //check maximum temporal cluster size(as percentage of population) is less than maximum for given probability model
      if (gfMaxTemporalClusterSize > (geProbabilityModelType == SPACETIMEPERMUTATION ? 50 : 90)) {
        PrintDirection.SatScanPrintWarning("Error: For the %s model, the maximum temporal cluster size as a percent\n"
                                           "       of the study period is %d percent.\n",
                                           GetProbabilityModelTypeAsString(geProbabilityModelType),
                                           (geProbabilityModelType == SPACETIMEPERMUTATION ? 50 : 90));
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
      dMaxTemporalLengthInUnits = floor(dStudyPeriodLengthInUnits * (geProbabilityModelType == SPACETIMEPERMUTATION ? 50 : 90)/100.0);
      if (gfMaxTemporalClusterSize > dMaxTemporalLengthInUnits) {
        PrintDirection.SatScanPrintWarning("Error: A maximum temporal cluster size of %d %s%s exceeds\n"
                                           "       %d percent of a %d %s study period.\n"
                                           "       Note that current settings limit the maximum to %d %s%s.\n",
                                           static_cast<int>(gfMaxTemporalClusterSize), sPrecisionString.GetCString(),
                                           (gfMaxTemporalClusterSize == 1 ? "" : "s"),
                                           (geProbabilityModelType == SPACETIMEPERMUTATION ? 50 : 90),
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
      if (geAnalysisType == PURELYSPATIAL && geRiskFunctionType == MONOTONERISK && GetNumDataSets() > 1) {
        bValid = false;
        PrintDirection.SatScanPrintWarning("Error: Multiple data sets are not permitted with isotonic purely spatial analyses.\n");
      }
      if (geProbabilityModelType == ORDINAL && geRiskFunctionType == MONOTONERISK) {
        bValid = false;
        PrintDirection.SatScanPrintWarning("Error: Ordinal probablility model does not permit isotonic purely spatial analyses.\n");
      }
      if (geProbabilityModelType == EXPONENTIAL && geRiskFunctionType == MONOTONERISK) {
        bValid = false;
        PrintDirection.SatScanPrintWarning("Error: Exponential probablility model does not permit isotonic purely spatial analyses.\n");
      }
      if (geProbabilityModelType == NORMAL && GetNumDataSets() > 1) {
        bValid = false;
        PrintDirection.SatScanPrintWarning("Error: Multiple data sets are not permitted with the normal probablility model\n");
        PrintDirection.SatScanPrintWarning("       in this version of SaTScan.\n");
      }
      if (geProbabilityModelType == ORDINAL && GetNumDataSets() > 1 && geMultipleSetPurposeType == ADJUSTMENT) {
        bValid = false;
        PrintDirection.SatScanPrintWarning("Error: Adjustment purpose for multiple data sets is not permitted\n"
                                           "       with ordinal probability model in this version of SaTScan.\n");
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
      if (geProbabilityModelType == SPACETIMEPERMUTATION) {
        if (!(geAnalysisType == SPACETIME || geAnalysisType == PROSPECTIVESPACETIME)) {
          bValid = false;
          PrintDirection.SatScanPrintWarning("Error: For the %s model, the analysis type must be either Retrospective or Prospective Space-Time.\n",
                                             GetProbabilityModelTypeAsString(geProbabilityModelType));
        }
        if (gbOutputRelativeRisksAscii || gbOutputRelativeRisksDBase) {
          bValid = false;
          PrintDirection.SatScanPrintWarning("Error: The relative risks output files can not be produced for the %s model.\n", GetProbabilityModelTypeAsString(geProbabilityModelType));
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

      //validate hidden parameter which specifies randomization seed
      if (!(0 < glRandomizationSeed && glRandomizationSeed < RandomNumberGenerator::glM)) {
         bValid = false;
         PrintDirection.SatScanPrintWarning("Error: Randomization seed out of range [1 - %ld].\n", RandomNumberGenerator::glM);
      }
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
    if (giReplications == 0)
      gbOutputSimulationData = false;
    if (gbOutputSimulationData && gsSimulationDataOutputFilename.empty()) {
      bValid = false;
      PrintDirection.SatScanPrintWarning("Error: Simulation data output file not specified.\n");
    }

    switch (geSimulationType) {
      case STANDARD         : break;
      case HA_RANDOMIZATION :
        if (geProbabilityModelType == POISSON) {
          if (gsAdjustmentsByRelativeRisksFileName.empty()) {
            bValid = false;
            PrintDirection.SatScanPrintWarning("Error: No adjustments file specified.\n");
          }
          else if (access(gsAdjustmentsByRelativeRisksFileName.c_str(), 00)) {
            bValid = false;
            PrintDirection.SatScanPrintWarning("Error: The adjustments file '%s' does not exist.\n", gsAdjustmentsByRelativeRisksFileName.c_str());
            PrintDirection.SatScanPrintWarning("       Please check to make sure the path is correct.\n");
          }
        }
        else {
          bValid = false;
          PrintDirection.SatScanPrintWarning("Error: The alternative hypothesis method of creating simulated data\n");
          PrintDirection.SatScanPrintWarning("       is only implemented for the Poisson model.\n");
        }
        break;
      case FILESOURCE       :
        if (geProbabilityModelType == EXPONENTIAL) {
          bValid = false;
          PrintDirection.SatScanPrintWarning("Error: The feature to read simulated data from a file is not implemented for\n"
                                             "       the exponential probability model.\n");
        }
        if (GetNumDataSets() > 1){
          bValid = false;
          PrintDirection.SatScanPrintWarning("Error: The feature to read simulated data from a file is not implemented for analyses\n"
                                             "       that read data from multiple data sets.\n");
        }
        if (gsSimulationDataSourceFileName.empty()) {
          bValid = false;
          PrintDirection.SatScanPrintWarning("Error: The simulated data input file was not specified.\n");
        }
        else if (access(gsSimulationDataSourceFileName.c_str(), 00)) {
          bValid = false;
          PrintDirection.SatScanPrintWarning("Error: The simulated data input file '%s' does not exist.\n", gsSimulationDataSourceFileName.c_str());
          PrintDirection.SatScanPrintWarning("       Please check to make sure the path is correct.\n");
        }
        if (gbOutputSimulationData && gsSimulationDataSourceFileName == gsSimulationDataOutputFilename) {
          bValid = false;
          PrintDirection.SatScanPrintWarning("Error: The file '%s' is specified as both\n", gsSimulationDataSourceFileName.c_str());
          PrintDirection.SatScanPrintWarning("       the input and the output file for simulated data.\n");
        }
        break;
      default : ZdGenerateException("Unknown simulation type '%d'.","ValidateSimulationDataParameters()", geSimulationType);
    };
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
      if (geAnalysisType != PURELYSPATIAL) {
        //code only implemented for purley spatial analyses
        PrintDirection.SatScanPrintWarning("Error: The sequential scan feature is only implemented for purely spatial analyses.\n");
        return false;
      }
      if (!(geProbabilityModelType == POISSON || geProbabilityModelType == BERNOULLI || geProbabilityModelType == ORDINAL)) {
        //code only implemented for Poisson or Bernoulli models
        PrintDirection.SatScanPrintWarning("Error: The sequential scan feature is implemented for Poisson, Bernoulli and Ordinal models only.\n");
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
      if (!GetPermitsPurelySpatialCluster(geProbabilityModelType)) {
          bValid = false;
          PrintDirection.SatScanPrintWarning("Error: A purely spatial cluster cannot be included for a %s model.\n",
                                             GetProbabilityModelTypeAsString(geProbabilityModelType));
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
  DatePrecisionType     ePrecision;

  try {
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
    if (gCreationVersion.iMajor == 4)
      // no date precision validation needed for purely spatial
      ePrecision = (geAnalysisType == PURELYSPATIAL ? NONE : geTimeAggregationUnitsType);
    else
      ePrecision = gePrecisionOfTimesType;
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
    switch (geProbabilityModelType) {
      case BERNOULLI            :
        //The SVTT analysis has hooks for temporal adjustments, but that code needs
        //much closer examination before it can be used, even experimentally.
        if (geTimeTrendAdjustType != NOTADJUSTED) {
          PrintDirection.SatScanPrintWarning("Warning: For the Bernoulli model, adjusting for temporal trends is not permitted.\n");
                                             geTimeTrendAdjustType = NOTADJUSTED;
                                             gdTimeTrendAdjustPercentage = 0.0;
        }
        break;
      case ORDINAL              :
      case EXPONENTIAL          :
      case NORMAL               :
      case RANK                 :
        if (geTimeTrendAdjustType != NOTADJUSTED) {
          PrintDirection.SatScanPrintWarning("Warning: For the %s model, adjusting for temporal trends is not permitted.\n",
                                             GetProbabilityModelTypeAsString(geProbabilityModelType));
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
        if (geTimeTrendAdjustType != NOTADJUSTED && (geAnalysisType == PURELYTEMPORAL || geAnalysisType == PROSPECTIVEPURELYTEMPORAL)
            && GetPopulationFileName().empty()) {
           bValid = false; 
          PrintDirection.SatScanPrintWarning("Error: Temporal adjustments can not be performed for a purely temporal analysis\n"
                                             "       using the Poisson model, when no population file has been specfied.\n");
    }

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
      default : ZdException::Generate("Unknown model type '%d'.","ValidateTemporalParameters()", geProbabilityModelType);
    }
    //validate including purely temporal clusters
    if (gbIncludePurelyTemporalClusters) {
      if (!GetPermitsPurelyTemporalCluster(geProbabilityModelType)) {
          bValid = false;
          PrintDirection.SatScanPrintWarning("Error: Looking for purely temporal clusters can not be included when the %s model is used.\n",
                                             GetProbabilityModelTypeAsString(geProbabilityModelType));
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


//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "Parameters.h"
#include "Randomizer.h"
#include "SSException.h"

const int CParameters::MAXIMUM_ITERATIVE_ANALYSES     = 32000;
const int CParameters::MAXIMUM_ELLIPSOIDS             = 10;
const int CParameters::giNumParameters 	              = 102;

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
  if (this != &rhs) Copy(rhs);
  return (*this);
}

bool  CParameters::operator==(const CParameters& rhs) const {
  if (geSpatialWindowType                    != rhs.geSpatialWindowType) return false;
  if (gvEllipseShapes                        != rhs.gvEllipseShapes) return false;
  if (gvEllipseRotations                     != rhs.gvEllipseRotations) return false;
  if (geNonCompactnessPenaltyType            != rhs.geNonCompactnessPenaltyType) return false;
  if (glTotalNumEllipses                     != rhs.glTotalNumEllipses) return false;
  if (geAnalysisType                         != rhs.geAnalysisType) return false;
  if (geAreaScanRate                         != rhs.geAreaScanRate) return false;
  if (geProbabilityModelType                 != rhs.geProbabilityModelType) return false;
  if (geRiskFunctionType                     != rhs.geRiskFunctionType) return false;
  if (giReplications                         != rhs.giReplications) return false;
  if (gbPowerCalculation                     != rhs.gbPowerCalculation) return false;
  if (gdPower_X                              != rhs.gdPower_X) return false;
  if (gdPower_Y                              != rhs.gdPower_Y) return false;
  if (gsStudyPeriodStartDate                 != rhs.gsStudyPeriodStartDate) return false;
  if (gsStudyPeriodEndDate                   != rhs.gsStudyPeriodEndDate) return false;
  if (gdMaxTemporalClusterSize               != rhs.gdMaxTemporalClusterSize) return false;
  if (geIncludeClustersType                  != rhs.geIncludeClustersType) return false;
  if (geTimeAggregationUnitsType             != rhs.geTimeAggregationUnitsType) return false;
  if (glTimeAggregationLength                != rhs.glTimeAggregationLength) return false;
  if (geTimeTrendAdjustType                  != rhs.geTimeTrendAdjustType) return false;
  if (gdTimeTrendAdjustPercentage            != rhs.gdTimeTrendAdjustPercentage) return false;
  if (gbIncludePurelySpatialClusters         != rhs.gbIncludePurelySpatialClusters) return false;
  if (gbIncludePurelyTemporalClusters        != rhs.gbIncludePurelyTemporalClusters) return false;
  if (gvCaseFilenames                        != rhs.gvCaseFilenames) return false;
  if (gvControlFilenames                     != rhs.gvControlFilenames) return false;
  if (gvPopulationFilenames                  != rhs.gvPopulationFilenames) return false;
  if (gsCoordinatesFileName                  != rhs.gsCoordinatesFileName) return false;
  if (gsSpecialGridFileName                  != rhs.gsSpecialGridFileName) return false;
  if (gbUseSpecialGridFile                   != rhs.gbUseSpecialGridFile) return false;
  if (gsMaxCirclePopulationFileName          != rhs.gsMaxCirclePopulationFileName) return false;
  if (gePrecisionOfTimesType                 != rhs.gePrecisionOfTimesType) return false;
  if (geCoordinatesType                      != rhs.geCoordinatesType) return false;
  if (gsOutputFileName                       != rhs.gsOutputFileName) return false;
  if (gbOutputSimLogLikeliRatiosAscii        != rhs.gbOutputSimLogLikeliRatiosAscii) return false;
  if (gbOutputRelativeRisksAscii             != rhs.gbOutputRelativeRisksAscii) return false;
  if (gbIterativeRuns                        != rhs.gbIterativeRuns) return false;
  if (giNumIterativeRuns                     != rhs.giNumIterativeRuns) return false;
  if (gbIterativeCutOffPValue                != rhs.gbIterativeCutOffPValue) return false;
  if (gsProspectiveStartDate                 != rhs.gsProspectiveStartDate) return false;
  if (gbOutputAreaSpecificAscii              != rhs.gbOutputAreaSpecificAscii) return false;
  if (gbOutputClusterLevelAscii              != rhs.gbOutputClusterLevelAscii) return false;
  if (geCriteriaSecondClustersType           != rhs.geCriteriaSecondClustersType) return false;
  if (geMaxTemporalClusterSizeType           != rhs.geMaxTemporalClusterSizeType) return false;
  if (gbOutputClusterLevelDBase              != rhs.gbOutputClusterLevelDBase) return false;
  if (gbOutputAreaSpecificDBase              != rhs.gbOutputAreaSpecificDBase) return false;
  if (gbOutputRelativeRisksDBase             != rhs.gbOutputRelativeRisksDBase) return false;
  if (gbOutputSimLogLikeliRatiosDBase        != rhs.gbOutputSimLogLikeliRatiosDBase) return false;
  if (gsRunHistoryFilename                   != rhs.gsRunHistoryFilename) return false;
  if (gbLogRunHistory                        != rhs.gbLogRunHistory) return false;
  if (gsParametersSourceFileName             != rhs.gsParametersSourceFileName) return false;
  if (gsEndRangeStartDate                    != rhs.gsEndRangeStartDate) return false;
  if (gsEndRangeEndDate                      != rhs.gsEndRangeEndDate) return false;
  if (gsStartRangeStartDate                  != rhs.gsStartRangeStartDate) return false;
  if (gsStartRangeEndDate                    != rhs.gsStartRangeEndDate) return false;
  if (gdTimeTrendConverge		             != rhs.gdTimeTrendConverge) return false;
  if (gbRestrictReportedClusters             != rhs.gbRestrictReportedClusters) return false;
  if (geSimulationType                       != rhs.geSimulationType) return false;
  if (gsSimulationDataSourceFileName         != rhs.gsSimulationDataSourceFileName) return false;
  if (gsAdjustmentsByRelativeRisksFileName   != rhs.gsAdjustmentsByRelativeRisksFileName) return false;
  if (gbOutputSimulationData                 != rhs.gbOutputSimulationData) return false;
  if (gsSimulationDataOutputFilename         != rhs.gsSimulationDataOutputFilename) return false;
  if (gbAdjustForEarlierAnalyses             != rhs.gbAdjustForEarlierAnalyses) return false;
  if (gbUseAdjustmentsForRRFile              != rhs.gbUseAdjustmentsForRRFile) return false;
  if (geSpatialAdjustmentType                != rhs.geSpatialAdjustmentType) return false;
  if (geMultipleSetPurposeType               != rhs.geMultipleSetPurposeType) return false;
  //if (gCreationVersion                       != rhs.gCreationVersion) return false;
  if (gbUsePopulationFile                    != rhs.gbUsePopulationFile) return false;
  //if (glRandomizationSeed                    != rhs.glRandomizationSeed) return false;
  if (gbReportCriticalValues                 != rhs.gbReportCriticalValues) return false;
  //if (geExecutionType                        != rhs.geExecutionType) return false;
  if (giNumRequestedParallelProcesses        != rhs.giNumRequestedParallelProcesses) return false;
  if (gbSuppressWarnings                     != rhs.gbSuppressWarnings) return false;
  if (gbOutputClusterCaseAscii               != rhs.gbOutputClusterCaseAscii) return false;
  if (gbOutputClusterCaseDBase               != rhs.gbOutputClusterCaseDBase) return false;
  if (geStudyPeriodDataCheckingType          != rhs.geStudyPeriodDataCheckingType) return false;
  if (geCoordinatesDataCheckingType          != rhs.geCoordinatesDataCheckingType) return false;
  if (gdMaxSpatialSizeInPopulationAtRisk     != rhs.gdMaxSpatialSizeInPopulationAtRisk) return false;
  if  (gbRestrictMaxSpatialSizeThroughMaxCirclePopulationFile != rhs.gbRestrictMaxSpatialSizeThroughMaxCirclePopulationFile) return false;
  if (gdMaxSpatialSizeInMaxCirclePopulationFile != rhs.gdMaxSpatialSizeInMaxCirclePopulationFile) return false;
  if (gbRestrictMaxSpatialSizeThroughDistanceFromCenter != rhs.gbRestrictMaxSpatialSizeThroughDistanceFromCenter) return false;
  if (gdMaxSpatialSizeInMaxDistanceFromCenter != rhs.gdMaxSpatialSizeInMaxDistanceFromCenter) return false;
  if (gdMaxSpatialSizeInPopulationAtRisk_Reported != rhs.gdMaxSpatialSizeInPopulationAtRisk_Reported) return false;
  if (gbRestrictMaxSpatialSizeThroughMaxCirclePopulationFile_Reported != rhs.gbRestrictMaxSpatialSizeThroughMaxCirclePopulationFile_Reported) return false;
  if (gdMaxSpatialSizeInMaxCirclePopulationFile_Reported != rhs.gdMaxSpatialSizeInMaxCirclePopulationFile_Reported) return false;
  if (gbRestrictMaxSpatialSizeThroughDistanceFromCenter_Reported != rhs.gbRestrictMaxSpatialSizeThroughDistanceFromCenter_Reported) return false;
  if (gdMaxSpatialSizeInMaxDistanceFromCenter_Reported != rhs.gdMaxSpatialSizeInMaxDistanceFromCenter_Reported) return false;
  if (gsLocationNeighborsFilename            != rhs.gsLocationNeighborsFilename) return false;
  if (gbUseLocationNeighborsFile             != rhs.gbUseLocationNeighborsFile) return false;
  if (gbRandomlyGenerateSeed != rhs.gbRandomlyGenerateSeed) return false;
  if (geMultipleCoordinatesType              != rhs.geMultipleCoordinatesType) return false;
  if (gsMetaLocationsFilename                != rhs.gsMetaLocationsFilename) return false;
  if (gbUseMetaLocationsFile                 != rhs.gbUseMetaLocationsFile) return false;
  if (gvObservableRegions                    != rhs.gvObservableRegions) return false;
  if (gbWeightedNormal                       != rhs.gbWeightedNormal) return false;
  if (gbWeightedNormalCovariates             != rhs.gbWeightedNormalCovariates) return false;  
  if (giEarlyTermThreshold                   != rhs.giEarlyTermThreshold) return false;
  if (gePValueReportingType                  != rhs.gePValueReportingType) return false;
  if (gbReportGumbelPValue                   != rhs.gbReportGumbelPValue) return false;
  if (geTimeTrendType                        != rhs.geTimeTrendType) return false;
  if (gbReportRank                           != rhs.gbReportRank) return false;
  if (gbPrintAsciiHeaders                    != rhs.gbPrintAsciiHeaders) return false;

  return true;
}

bool  CParameters::operator!=(const CParameters& rhs) const{
   return !(*this == rhs);
}

/** Add ellipsoid rotations to collection of spatial shapes evaluated. */
void CParameters::AddEllipsoidRotations(int iRotations, bool bEmptyFirst) {
  if (bEmptyFirst) gvEllipseRotations.clear();
  gvEllipseRotations.push_back(iRotations);

  //re-calculate number of total ellispes
  glTotalNumEllipses = 0;
  for (size_t t=0; t < gvEllipseRotations.size(); ++t)
     glTotalNumEllipses += gvEllipseRotations[t];
}

/** Add ellipsoid shape to collection of spatial shapes evaluated. */
void CParameters::AddEllipsoidShape(double dShape, bool bEmptyFirst) {
  if (bEmptyFirst) gvEllipseShapes.clear();
  gvEllipseShapes.push_back(dShape);
}

/** Adds string that defines observable region to internal collection. */
void CParameters::AddObservableRegion(const char * sRegions, size_t iIndex, bool bEmptyFirst) {
  if (!sRegions)
     throw prg_error("Null pointer.","AddObservableRegion()");

  if (bEmptyFirst) 
    gvObservableRegions.clear();
    
  if (iIndex + 1 > gvObservableRegions.size())
    gvObservableRegions.resize(iIndex + 1);

  gvObservableRegions[iIndex] = sRegions;
}

/** If passed filename contains a slash, then assumes that path is complete and
    sInputFilename is not modified. If filename does not contain a slash, it is
    assumed that filename is located in same directory of parameter file.
    sInputFilename is reset to this location. Note that the primary reason for
    implementing this feature was to permit the program to be installed in any
    location and sample parameter files run immediately without having to edit
    input file paths. */
void CParameters::AssignMissingPath(std::string & sInputFilename, bool bCheckWritable) {
  FileName      fParameterFilename, fFilename;
  std::string   buffer;

  if (! sInputFilename.empty()) {
    //Assume that if slashes exist, then this is a complete file path, so
    //we'll make no attempts to determine what path might be otherwise.
    if (sInputFilename.find(FileName::SLASH) == sInputFilename.npos) {
      //If no slashes, then this file is assumed to be in same directory as parameters file.
      fParameterFilename.setFullPath(GetSourceFileName().c_str());

      fFilename.setFullPath(sInputFilename.c_str());
      fFilename.setLocation(fParameterFilename.getLocation(buffer).c_str());

      if (bCheckWritable && !ValidateFileAccess(fFilename.getFullPath(buffer), true)) {
        // if writability fails, then try setting to user documents directory
        std::string temp;
        fFilename.setLocation(GetUserDocumentsDirectory(buffer, fParameterFilename.getLocation(temp)).c_str());
      }

      fFilename.getFullPath(sInputFilename);
    }
  }
}

/** Copies all class variables from the given CParameters object (rhs) into this one */
void CParameters::Copy(const CParameters &rhs) {
  geSpatialWindowType                    = rhs.geSpatialWindowType;
  gvEllipseShapes                        = rhs.gvEllipseShapes;
  gvEllipseRotations                     = rhs.gvEllipseRotations;
  geNonCompactnessPenaltyType            = rhs.geNonCompactnessPenaltyType;
  glTotalNumEllipses                     = rhs.glTotalNumEllipses;
  geAnalysisType                         = rhs.geAnalysisType;
  geAreaScanRate                         = rhs.geAreaScanRate;
  geProbabilityModelType                 = rhs.geProbabilityModelType;
  geRiskFunctionType                     = rhs.geRiskFunctionType;
  giReplications                         = rhs.giReplications;
  gbPowerCalculation                     = rhs.gbPowerCalculation;
  gdPower_X                              = rhs.gdPower_X;
  gdPower_Y                              = rhs.gdPower_Y;
  gsStudyPeriodStartDate                 = rhs.gsStudyPeriodStartDate;
  gsStudyPeriodEndDate                   = rhs.gsStudyPeriodEndDate;
  gdMaxTemporalClusterSize               = rhs.gdMaxTemporalClusterSize;
  geIncludeClustersType                  = rhs.geIncludeClustersType;
  geTimeAggregationUnitsType             = rhs.geTimeAggregationUnitsType;
  glTimeAggregationLength                = rhs.glTimeAggregationLength;
  geTimeTrendAdjustType                  = rhs.geTimeTrendAdjustType;
  gdTimeTrendAdjustPercentage            = rhs.gdTimeTrendAdjustPercentage;
  gbIncludePurelySpatialClusters         = rhs.gbIncludePurelySpatialClusters;
  gbIncludePurelyTemporalClusters        = rhs.gbIncludePurelyTemporalClusters;
  gvCaseFilenames                        = rhs.gvCaseFilenames;
  gvControlFilenames                     = rhs.gvControlFilenames;
  gvPopulationFilenames                  = rhs.gvPopulationFilenames;
  gsCoordinatesFileName                  = rhs.gsCoordinatesFileName;
  gsSpecialGridFileName                  = rhs.gsSpecialGridFileName;
  gbUseSpecialGridFile                   = rhs.gbUseSpecialGridFile;
  gsMaxCirclePopulationFileName          = rhs.gsMaxCirclePopulationFileName;
  gePrecisionOfTimesType                 = rhs.gePrecisionOfTimesType;
  geCoordinatesType                      = rhs.geCoordinatesType;
  gsOutputFileName                       = rhs.gsOutputFileName;
  gbOutputSimLogLikeliRatiosAscii        = rhs.gbOutputSimLogLikeliRatiosAscii;
  gbOutputRelativeRisksAscii             = rhs.gbOutputRelativeRisksAscii;
  gbIterativeRuns                        = rhs.gbIterativeRuns;
  giNumIterativeRuns                     = rhs.giNumIterativeRuns;
  gbIterativeCutOffPValue                = rhs.gbIterativeCutOffPValue;
  gsProspectiveStartDate                 = rhs.gsProspectiveStartDate;
  gbOutputAreaSpecificAscii              = rhs.gbOutputAreaSpecificAscii;
  gbOutputClusterLevelAscii              = rhs.gbOutputClusterLevelAscii;
  geCriteriaSecondClustersType           = rhs.geCriteriaSecondClustersType;
  geMaxTemporalClusterSizeType           = rhs.geMaxTemporalClusterSizeType;
  gbOutputClusterLevelDBase              = rhs.gbOutputClusterLevelDBase;
  gbOutputAreaSpecificDBase              = rhs.gbOutputAreaSpecificDBase;
  gbOutputRelativeRisksDBase             = rhs.gbOutputRelativeRisksDBase;
  gbOutputSimLogLikeliRatiosDBase        = rhs.gbOutputSimLogLikeliRatiosDBase;
  gsRunHistoryFilename                   = rhs.gsRunHistoryFilename;
  gbLogRunHistory                        = rhs.gbLogRunHistory;
  gsParametersSourceFileName             = rhs.gsParametersSourceFileName;
  gsEndRangeStartDate                    = rhs.gsEndRangeStartDate;
  gsEndRangeEndDate                      = rhs.gsEndRangeEndDate;
  gsStartRangeStartDate                  = rhs.gsStartRangeStartDate;
  gsStartRangeEndDate                    = rhs.gsStartRangeEndDate;
  gdTimeTrendConverge			         = rhs.gdTimeTrendConverge;
  gbRestrictReportedClusters             = rhs.gbRestrictReportedClusters;
  geSimulationType                       = rhs.geSimulationType;
  gsSimulationDataSourceFileName         = rhs.gsSimulationDataSourceFileName;
  gsAdjustmentsByRelativeRisksFileName   = rhs.gsAdjustmentsByRelativeRisksFileName;
  gbOutputSimulationData                 = rhs.gbOutputSimulationData;
  gsSimulationDataOutputFilename         = rhs.gsSimulationDataOutputFilename;
  gbAdjustForEarlierAnalyses             = rhs.gbAdjustForEarlierAnalyses;
  gbUseAdjustmentsForRRFile              = rhs.gbUseAdjustmentsForRRFile;
  geSpatialAdjustmentType                = rhs.geSpatialAdjustmentType;
  geMultipleSetPurposeType               = rhs.geMultipleSetPurposeType;
  gCreationVersion                       = rhs.gCreationVersion;
  gbUsePopulationFile                    = rhs.gbUsePopulationFile;
  glRandomizationSeed                    = rhs.glRandomizationSeed;
  gbReportCriticalValues                 = rhs.gbReportCriticalValues;
  geExecutionType                        = rhs.geExecutionType;
  giNumRequestedParallelProcesses        = rhs.giNumRequestedParallelProcesses;
  gbSuppressWarnings                     = rhs.gbSuppressWarnings;
  gbOutputClusterCaseAscii               = rhs.gbOutputClusterCaseAscii;
  gbOutputClusterCaseDBase               = rhs.gbOutputClusterCaseDBase;
  geStudyPeriodDataCheckingType          = rhs.geStudyPeriodDataCheckingType;
  geCoordinatesDataCheckingType          = rhs.geCoordinatesDataCheckingType;
  gdMaxSpatialSizeInPopulationAtRisk     = rhs.gdMaxSpatialSizeInPopulationAtRisk;
  gbRestrictMaxSpatialSizeThroughMaxCirclePopulationFile = rhs.gbRestrictMaxSpatialSizeThroughMaxCirclePopulationFile;
  gdMaxSpatialSizeInMaxCirclePopulationFile = rhs.gdMaxSpatialSizeInMaxCirclePopulationFile;
  gbRestrictMaxSpatialSizeThroughDistanceFromCenter = rhs.gbRestrictMaxSpatialSizeThroughDistanceFromCenter;
  gdMaxSpatialSizeInMaxDistanceFromCenter = rhs.gdMaxSpatialSizeInMaxDistanceFromCenter;
  gdMaxSpatialSizeInPopulationAtRisk_Reported = rhs.gdMaxSpatialSizeInPopulationAtRisk_Reported;
  gbRestrictMaxSpatialSizeThroughMaxCirclePopulationFile_Reported = rhs.gbRestrictMaxSpatialSizeThroughMaxCirclePopulationFile_Reported;
  gdMaxSpatialSizeInMaxCirclePopulationFile_Reported = rhs.gdMaxSpatialSizeInMaxCirclePopulationFile_Reported;
  gbRestrictMaxSpatialSizeThroughDistanceFromCenter_Reported = rhs.gbRestrictMaxSpatialSizeThroughDistanceFromCenter_Reported;
  gdMaxSpatialSizeInMaxDistanceFromCenter_Reported = rhs.gdMaxSpatialSizeInMaxDistanceFromCenter_Reported;
  gsLocationNeighborsFilename            = rhs.gsLocationNeighborsFilename;
  gbUseLocationNeighborsFile             = rhs.gbUseLocationNeighborsFile;
  gbRandomlyGenerateSeed                 = rhs.gbRandomlyGenerateSeed;
  geMultipleCoordinatesType              = rhs.geMultipleCoordinatesType;
  gsMetaLocationsFilename                = rhs.gsMetaLocationsFilename;
  gbUseMetaLocationsFile                 = rhs.gbUseMetaLocationsFile;
  gvObservableRegions                    = rhs.gvObservableRegions;
  gbWeightedNormal                       = rhs.gbWeightedNormal;
  gbWeightedNormalCovariates             = rhs.gbWeightedNormalCovariates;
  giEarlyTermThreshold                   = rhs.giEarlyTermThreshold;
  gePValueReportingType                  = rhs.gePValueReportingType;
  gbReportGumbelPValue                   = rhs.gbReportGumbelPValue;
  geTimeTrendType                        = rhs.geTimeTrendType;
  gbReportRank                           = rhs.gbReportRank;
  gbPrintAsciiHeaders                    = rhs.gbPrintAsciiHeaders;
}

const std::string & CParameters::GetCaseFileName(size_t iSetIndex) const {
  if (!iSetIndex || iSetIndex > gvCaseFilenames.size())
    throw prg_error("Index %d out of range [%d,%d].","GetCaseFileName()", iSetIndex,
                    (gvCaseFilenames.size() ? 1 : -1), (gvCaseFilenames.size() ? (int)gvCaseFilenames.size() : -1));
  return gvCaseFilenames[iSetIndex - 1];
}

const std::string & CParameters::GetControlFileName(size_t iSetIndex) const {
  if (!iSetIndex || iSetIndex > gvControlFilenames.size())
    throw prg_error("Index %d out of range [%d,%d].","GetControlFileName()", iSetIndex,
                    (gvControlFilenames.size() ? 1 : -1), (gvControlFilenames.size() ? (int)gvControlFilenames.size() : -1));
  return gvControlFilenames[iSetIndex - 1];
}

/** Returns threshold for early termination. If reporting default p-value, then
    threshold is determined by number of replications requested. */
unsigned int CParameters::GetExecuteEarlyTermThreshold() const {
  if (GetPValueReportingType() == DEFAULT_PVALUE) {
      return (GetNumReplicationsRequested() + 1)/20;
  }
  return giEarlyTermThreshold;
}

/** Returns the scanning area type used during execution. For the normal model,
    high and low are reversed. */
AreaRateType CParameters::GetExecuteScanRateType() const {
    if (geProbabilityModelType == NORMAL && !getIsWeightedNormal() && geAreaScanRate == HIGH)
    return LOW;
  if (geProbabilityModelType == NORMAL && !getIsWeightedNormal() && geAreaScanRate == LOW)
    return HIGH;

  return geAreaScanRate;
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
  return (geProbabilityModelType == SPACETIMEPERMUTATION ||
          (geSpatialWindowType == ELLIPTIC && geNonCompactnessPenaltyType != NOPENALTY));
}

/** Returns maximum spatial cluster size given type and whether value is for real or simulations. */
double CParameters::GetMaxSpatialSizeForType(SpatialSizeType eSpatialSizeType, bool bReported) const {
  switch (eSpatialSizeType) {
    case PERCENTOFPOPULATION    : return bReported ? gdMaxSpatialSizeInPopulationAtRisk_Reported : gdMaxSpatialSizeInPopulationAtRisk;
    case MAXDISTANCE            : return bReported ? gdMaxSpatialSizeInMaxDistanceFromCenter_Reported : gdMaxSpatialSizeInMaxDistanceFromCenter;
    case PERCENTOFMAXCIRCLEFILE : return bReported ? gdMaxSpatialSizeInMaxCirclePopulationFile_Reported : gdMaxSpatialSizeInMaxCirclePopulationFile;
    default : throw prg_error("Unknown type '%d'.\n", "GetMaxSpatialSizeForType()", eSpatialSizeType);
  };
}

/** Returns number of parallel processes to run. */
unsigned int CParameters::GetNumParallelProcessesToExecute() const {
#ifdef RPRTCMPT_RUNTIMES
  // reporting of run-time components is not thread safe at this time,
  // and has no useful purpose to be such - at this time
  return 1;
#else
  unsigned int  iNumProcessors;

  if (giNumRequestedParallelProcesses <= 0)
    //parameter of zero or less indicates that we want all available processors
    iNumProcessors = GetNumSystemProcessors();
  else
    //else parameter indicates the maximum number of processors to use
    iNumProcessors = std::min(giNumRequestedParallelProcesses, GetNumSystemProcessors());
    //iNumProcessors = giNumRequestedParallelProcesses;

  return iNumProcessors;
#endif  
}

/** Returns whether any area specific files are outputed. */
bool CParameters::GetOutputAreaSpecificFiles() const  {
  return gbOutputAreaSpecificAscii || gbOutputAreaSpecificDBase;
}

/** Returns whether any cluster case files are outputed. */
bool CParameters::GetOutputClusterCaseFiles() const {
  return gbOutputClusterCaseAscii || gbOutputClusterCaseDBase;
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

bool CParameters::GetPermitsCentricExecution(bool excludePValue) const {
 if (GetIsPurelyTemporalAnalysis()) return false;
 if (GetProbabilityModelType() == HOMOGENEOUSPOISSON) return false;
 if (GetAnalysisType() == PURELYSPATIAL && GetRiskType() == MONOTONERISK) return false;
 if (GetSpatialWindowType() == ELLIPTIC && GetNonCompactnessPenaltyType() > NOPENALTY) return false;
 if (!excludePValue && GetPValueReportingType() == TERMINATION_PVALUE && GetNumReplicationsRequested() >= MIN_SIMULATION_RPT_PVALUE) return false;
 if (UseLocationNeighborsFile()) return false;
 if (UsingMultipleCoordinatesMetaLocations()) return false;
 return true;
}

/** returns whether analysis type permits inclusion of purely spatial cluster */
bool CParameters::GetPermitsPurelySpatialCluster() const {
  return geAnalysisType == PURELYSPATIAL || geAnalysisType == SPACETIME || GetIsProspectiveAnalysis();
}

/** returns whether probability model type permits inclusion of purely spatial cluster */
bool CParameters::GetPermitsPurelySpatialCluster(ProbabilityModelType eModelType) const {
  return eModelType == POISSON || eModelType == BERNOULLI || eModelType == NORMAL
         || eModelType == EXPONENTIAL || eModelType == RANK || eModelType == ORDINAL || eModelType == CATEGORICAL;
}

/** returns whether analysis type permits inclusion of purely temporal cluster */
bool CParameters::GetPermitsPurelyTemporalCluster() const {
  return geAnalysisType == PURELYTEMPORAL || geAnalysisType == SPACETIME || GetIsProspectiveAnalysis();
}

/** returns whether probability model type permits inclusion of purely temporal cluster */
bool CParameters::GetPermitsPurelyTemporalCluster(ProbabilityModelType eModelType) const {
  return eModelType == POISSON || eModelType == BERNOULLI || eModelType == NORMAL
         || eModelType == EXPONENTIAL || eModelType == RANK || eModelType == ORDINAL || eModelType == CATEGORICAL;
}

const std::string & CParameters::GetPopulationFileName(size_t iSetIndex) const {
  if (!iSetIndex || iSetIndex > gvPopulationFilenames.size())
    throw prg_error("Index %d out of range [%d,%d].","GetPopulationFileName()", iSetIndex,
                    (gvPopulationFilenames.size() ? 1 : -1), (gvPopulationFilenames.size() ? (int)gvPopulationFilenames.size() : -1));
  return gvPopulationFilenames[iSetIndex - 1];
}

/** If passed filename has same path as passed parameter filename, returns
    'name.extension' else returns filename. */
const char * CParameters::GetRelativeToParameterName(const FileName& fParameterName,
                                                     const std::string& sFilename,
                                                     std::string& sValue) const {
  FileName      fInputFilename(sFilename.c_str());
  std::string   buffer, buffer2;

  if (!stricmp(fInputFilename.getLocation(buffer).c_str(), fParameterName.getLocation(buffer2).c_str()))
    sValue = fInputFilename.getFileName() + fInputFilename.getExtension();
  else
    sValue = sFilename.c_str();
  return sValue.c_str();
}

/** Returns indication of whether maximum spatial cluster size is restricted by given type and for real or simulations. */
bool CParameters::GetRestrictMaxSpatialSizeForType(SpatialSizeType eSpatialSizeType, bool bReported) const {
  switch (eSpatialSizeType) {
    case PERCENTOFPOPULATION    : return true;
    case MAXDISTANCE            : return bReported ? gbRestrictMaxSpatialSizeThroughDistanceFromCenter_Reported : gbRestrictMaxSpatialSizeThroughDistanceFromCenter;
    case PERCENTOFMAXCIRCLEFILE : return bReported ? gbRestrictMaxSpatialSizeThroughMaxCirclePopulationFile_Reported : gbRestrictMaxSpatialSizeThroughMaxCirclePopulationFile;
    default : throw prg_error("Unknown type '%d'.\n", "GetRestrictMaxSpatialSizeForType()", eSpatialSizeType);
  };
}

/** Sets maximum spatial cluster size given type and whether value is for real or simulations. */
void CParameters::SetMaxSpatialSizeForType(SpatialSizeType eSpatialSizeType, double d, bool bReported) {
  switch (eSpatialSizeType) {
    case PERCENTOFPOPULATION    : bReported ? gdMaxSpatialSizeInPopulationAtRisk_Reported = d : gdMaxSpatialSizeInPopulationAtRisk = d; break;
    case MAXDISTANCE            : bReported ? gdMaxSpatialSizeInMaxDistanceFromCenter_Reported = d : gdMaxSpatialSizeInMaxDistanceFromCenter = d; break;
    case PERCENTOFMAXCIRCLEFILE : bReported ? gdMaxSpatialSizeInMaxCirclePopulationFile_Reported = d : gdMaxSpatialSizeInMaxCirclePopulationFile = d; break;
    default : throw prg_error("Unknown type '%d'.\n", "GetMaxSpatialSizeForType()", eSpatialSizeType);
  };
}

/** Returns whether early termination option is set. */
bool CParameters::GetTerminateSimulationsEarly() const {
   return (GetPValueReportingType() == DEFAULT_PVALUE || GetPValueReportingType() == TERMINATION_PVALUE) && GetNumReplicationsRequested() >= MIN_SIMULATION_RPT_PVALUE;
}

/** Returns indication of Gumbel value is reported. */
bool CParameters::getIsReportingGumbelPValue() const {
    return GetPValueReportingType() == DEFAULT_PVALUE ||  // clusters with rank < 10 report Gumbel p-value
          (GetPValueReportingType() == STANDARD_PVALUE && GetReportGumbelPValue()) ||
          (GetPValueReportingType() == TERMINATION_PVALUE && GetReportGumbelPValue()) ||           
           GetPValueReportingType() == GUMBEL_PVALUE;
}

/** Returns indication of standard value is reported. */
bool CParameters::getIsReportingStandardPValue() const {
    return GetPValueReportingType() != GUMBEL_PVALUE;
}

/** Selects additional output file parameters - for current parameters state. */
void CParameters::RequestAllAdditionalOutputFiles() {
   SetOutputAreaSpecificAscii(true);
   SetOutputAreaSpecificDBase(true);
   SetOutputClusterCaseAscii(true);
   SetOutputClusterCaseDBase(true);
   SetOutputClusterLevelAscii(true);
   SetOutputClusterLevelDBase(true);
   if (!GetIsPurelyTemporalAnalysis() && 
        GetProbabilityModelType() != SPACETIMEPERMUTATION &&
        GetProbabilityModelType() != HOMOGENEOUSPOISSON &&
        GetProbabilityModelType() != ORDINAL &&
        GetProbabilityModelType() != CATEGORICAL) {
     SetOutputRelativeRisksAscii(true);
     SetOutputRelativeRisksDBase(true);
   } else {
     SetOutputRelativeRisksAscii(false);
     SetOutputRelativeRisksDBase(false);
   }
   SetOutputSimLogLikeliRatiosAscii(true);
   SetOutputSimLogLikeliRatiosDBase(true);
}

/** Sets whether maximum spatial cluster size is restricted by given type and whether retriction is for real or simulations. */
void CParameters::SetRestrictMaxSpatialSizeForType(SpatialSizeType eSpatialSizeType, bool b, bool bReported) {
  switch (eSpatialSizeType) {
    case PERCENTOFPOPULATION    : break;
    case MAXDISTANCE            : bReported ? gbRestrictMaxSpatialSizeThroughDistanceFromCenter_Reported = b : gbRestrictMaxSpatialSizeThroughDistanceFromCenter = b; break;
    case PERCENTOFMAXCIRCLEFILE : bReported ? gbRestrictMaxSpatialSizeThroughMaxCirclePopulationFile_Reported = b : gbRestrictMaxSpatialSizeThroughMaxCirclePopulationFile = b; break;
    default : throw prg_error("Unknown type '%d'.\n", "SetRestrictMaxSpatialSizeForType()", eSpatialSizeType);
  };
}

/** Sets start range start date. Throws exception. */
void CParameters::SetStartRangeStartDate(const char * sStartRangeStartDate) {
  gsStartRangeStartDate = sStartRangeStartDate;
}

/** Sets start range start date. Throws exception. */
void CParameters::SetStartRangeEndDate(const char * sStartRangeEndDate) {
  gsStartRangeEndDate = sStartRangeEndDate;
}

/** Sets analysis type. Throws exception if out of range. */
void CParameters::SetAnalysisType(AnalysisType eAnalysisType) {
  if (eAnalysisType < PURELYSPATIAL || eAnalysisType > PROSPECTIVEPURELYTEMPORAL)
    throw prg_error("Enumeration %d out of range [%d,%d].", "SetAnalysisType()", eAnalysisType, PURELYSPATIAL, PROSPECTIVEPURELYTEMPORAL);
  geAnalysisType = eAnalysisType;
}

/** Sets area rate for areas scanned type. Throws exception if out of range. */
void CParameters::SetAreaRateType(AreaRateType eAreaRateType) {
  if (eAreaRateType < HIGH || eAreaRateType > HIGHANDLOW)
    throw prg_error("Enumeration %d out of range [%d,%d].", "SetAreaRateType()", eAreaRateType, HIGH, HIGHANDLOW);
  geAreaScanRate = eAreaRateType;
}

/** Sets case data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void CParameters::SetCaseFileName(const char * sCaseFileName, bool bCorrectForRelativePath, size_t iSetIndex) {
  if (!iSetIndex)
    throw prg_error("Index %d out of range [1,].", "SetCaseFileName()", iSetIndex);

  if (iSetIndex > gvCaseFilenames.size())
    gvCaseFilenames.resize(iSetIndex);

  gvCaseFilenames[iSetIndex - 1] = sCaseFileName;
  if (bCorrectForRelativePath)
    AssignMissingPath(gvCaseFilenames[iSetIndex - 1]);
}

/** Sets control data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void CParameters::SetControlFileName(const char * sControlFileName, bool bCorrectForRelativePath, size_t iSetIndex) {
  if (!iSetIndex)
    throw prg_error("Index %d out of range [1,].", "SetControlFileName()", iSetIndex);

  if (iSetIndex > gvControlFilenames.size())
    gvControlFilenames.resize(iSetIndex);

  gvControlFilenames[iSetIndex - 1] = sControlFileName;
  if (bCorrectForRelativePath)
    AssignMissingPath(gvControlFilenames[iSetIndex - 1]);
}

/** Sets geographical coordinates data checking type. Throws exception if out of range. */
void CParameters::SetCoordinatesDataCheckingType(CoordinatesDataCheckingType eCoordinatesDataCheckingType) {
  if (eCoordinatesDataCheckingType < STRICTCOORDINATES || eCoordinatesDataCheckingType > RELAXEDCOORDINATES)
    throw prg_error("Enumeration %d out of range [%d,%d].", "SetCoordinatesDataCheckingType()",
                    eCoordinatesDataCheckingType, STRICTCOORDINATES, RELAXEDCOORDINATES);
  geCoordinatesDataCheckingType = eCoordinatesDataCheckingType;
}

/** Sets coordinates data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void CParameters::SetCoordinatesFileName(const char * sCoordinatesFileName, bool bCorrectForRelativePath) {
  gsCoordinatesFileName = sCoordinatesFileName;
  if (bCorrectForRelativePath)
    AssignMissingPath(gsCoordinatesFileName);
}

/** Sets precision of input file dates type. Throws exception if out of range. */
void CParameters::SetCoordinatesType(CoordinatesType eCoordinatesType) {
  if (eCoordinatesType < CARTESIAN || eCoordinatesType > LATLON)
    throw prg_error("Enumeration %d out of range [%d,%d].", "SetCoordinatesType()", eCoordinatesType, CARTESIAN, LATLON);
  geCoordinatesType = eCoordinatesType;
}

/** Sets criteria for reporting secondary clusters. Throws exception if out of range. */
void CParameters::SetCriteriaForReportingSecondaryClusters(CriteriaSecondaryClustersType eCriteriaSecondaryClustersType) {
  if (eCriteriaSecondaryClustersType < NOGEOOVERLAP || eCriteriaSecondaryClustersType > NORESTRICTIONS)
    throw prg_error("Enumeration %d out of range [%d,%d].", "SetCriteriaForReportingSecondaryClusters()",
                    eCriteriaSecondaryClustersType, NOGEOOVERLAP, NORESTRICTIONS);
  geCriteriaSecondClustersType = eCriteriaSecondaryClustersType;
}

/** initializes global variables to default values */
void CParameters::SetAsDefaulted() {
  geAnalysisType                           = PURELYSPATIAL;
  geAreaScanRate                           = HIGH;
  gvCaseFilenames.resize(1);
  gvPopulationFilenames.resize(1);
  gsCoordinatesFileName                    = "";
  gsOutputFileName                         = "";
  gsMaxCirclePopulationFileName            = "";
  gePrecisionOfTimesType                   = YEAR;
  gbUseSpecialGridFile                     = false;
  gsSpecialGridFileName                    = "";
  gsStudyPeriodStartDate                   = "2000/1/1";
  gsStudyPeriodEndDate                     = "2000/12/31";
  geIncludeClustersType                    = ALLCLUSTERS;
  geTimeAggregationUnitsType               = NONE;
  glTimeAggregationLength                  = 0;
  gbIncludePurelySpatialClusters           = false;
  gdMaxTemporalClusterSize                 = 50.0;
  geMaxTemporalClusterSizeType             = PERCENTAGETYPE;
  giReplications                           = 999;
  gbOutputClusterLevelDBase                = false;
  gbOutputAreaSpecificDBase                = false;
  gbOutputRelativeRisksDBase               = false;
  gbOutputSimLogLikeliRatiosDBase          = false;
  gsRunHistoryFilename                     = "";
  gbLogRunHistory                          = false;
  geProbabilityModelType                   = POISSON;
  geRiskFunctionType                       = STANDARDRISK;
  gbPowerCalculation                       = false;
  gdPower_X                                = 0.0;
  gdPower_Y                                = 0.0;
  geTimeTrendAdjustType                    = NOTADJUSTED;
  gdTimeTrendAdjustPercentage              = 0;
  gbIncludePurelyTemporalClusters          = false;
  gvControlFilenames.resize(1);
  geCoordinatesType                        = LATLON;
  gbOutputSimLogLikeliRatiosAscii          = false;
  gbIterativeRuns                          = false;
  giNumIterativeRuns                       = 0;
  gbIterativeCutOffPValue                  = 0.05;
  gbOutputRelativeRisksAscii               = false;
  geSpatialWindowType                      = CIRCULAR;
  gvEllipseShapes.clear();
  gvEllipseShapes.push_back(1.5);
  gvEllipseShapes.push_back(2);
  gvEllipseShapes.push_back(3);
  gvEllipseShapes.push_back(4);
  gvEllipseShapes.push_back(5);
  gvEllipseRotations.clear();
  gvEllipseRotations.push_back(4);
  gvEllipseRotations.push_back(6);
  gvEllipseRotations.push_back(9);
  gvEllipseRotations.push_back(12);
  gvEllipseRotations.push_back(15);
  glTotalNumEllipses = 0;
  for (size_t t=0; t < gvEllipseRotations.size(); ++t)
     glTotalNumEllipses += gvEllipseRotations[t];
  gsProspectiveStartDate                   = "2000/12/31";
  gbOutputAreaSpecificAscii                = false;
  gbOutputClusterLevelAscii                = false;
  geCriteriaSecondClustersType             = NOGEOOVERLAP;
  glTotalNumEllipses                       = 0;
  geNonCompactnessPenaltyType              = MEDIUMPENALTY;
  gsEndRangeStartDate                      = gsStudyPeriodStartDate;
  gsEndRangeEndDate                        = gsStudyPeriodEndDate;
  gsStartRangeStartDate                    = gsStudyPeriodStartDate;
  gsStartRangeEndDate                      = gsStudyPeriodEndDate;
  gdTimeTrendConverge			           = 0.00001; //0.0000001;
  gbRestrictReportedClusters               = false;
  geSimulationType                         = STANDARD;
  gsSimulationDataSourceFileName           = "";
  gsAdjustmentsByRelativeRisksFileName     = "";
  gbOutputSimulationData                   = false;
  gsSimulationDataOutputFilename           = "";
  gbAdjustForEarlierAnalyses               = false;
  gbUseAdjustmentsForRRFile                = false;
  geSpatialAdjustmentType                  = NO_SPATIAL_ADJUSTMENT;
  geMultipleSetPurposeType                 = MULTIVARIATE;
  gCreationVersion.iMajor                  = atoi(VERSION_MAJOR);
  gCreationVersion.iMinor                  = atoi(VERSION_MINOR);
  gCreationVersion.iRelease                = atoi(VERSION_RELEASE);
  gbUsePopulationFile                      = false;
  glRandomizationSeed                      = RandomNumberGenerator::glDefaultSeed;
  gbReportCriticalValues                   = false;
  geExecutionType                          = AUTOMATIC;
  giNumRequestedParallelProcesses          = 0;
  gbSuppressWarnings                       = false;
  gbOutputClusterCaseAscii                 = false;
  gbOutputClusterCaseDBase                 = false;
  geStudyPeriodDataCheckingType            = STRICTBOUNDS;
  geCoordinatesDataCheckingType            = STRICTCOORDINATES;
  gdMaxSpatialSizeInPopulationAtRisk       = 50.0;
  gbRestrictMaxSpatialSizeThroughMaxCirclePopulationFile = false;
  gdMaxSpatialSizeInMaxCirclePopulationFile = 50.0;
  gbRestrictMaxSpatialSizeThroughDistanceFromCenter = false;
  gdMaxSpatialSizeInMaxDistanceFromCenter = 1.0;
  gdMaxSpatialSizeInPopulationAtRisk_Reported = 50.0;
  gbRestrictMaxSpatialSizeThroughMaxCirclePopulationFile_Reported = false;
  gdMaxSpatialSizeInMaxCirclePopulationFile_Reported = 50.0;
  gbRestrictMaxSpatialSizeThroughDistanceFromCenter_Reported = false;
  gdMaxSpatialSizeInMaxDistanceFromCenter_Reported = 1.0;
  gsLocationNeighborsFilename = "";
  gbUseLocationNeighborsFile = false;
  gbRandomlyGenerateSeed = false;
  geMultipleCoordinatesType = ONEPERLOCATION;
  gsMetaLocationsFilename = "";
  gbUseMetaLocationsFile = false;
  gvObservableRegions.clear();
  gbWeightedNormal = false;
  gbWeightedNormalCovariates = false;
  giEarlyTermThreshold = 50;
  gePValueReportingType = DEFAULT_PVALUE;
  gbReportGumbelPValue = false;
  geTimeTrendType = LINEAR;
  gbReportRank = false;
  gbPrintAsciiHeaders = false;
}

/** Sets start range start date. Throws exception. */
void CParameters::SetEndRangeEndDate(const char * sEndRangeEndDate) {
  gsEndRangeEndDate = sEndRangeEndDate;
}

/** Sets end range start date. Throws exception. */
void CParameters::SetEndRangeStartDate(const char * sEndRangeStartDate) {
  gsEndRangeStartDate = sEndRangeStartDate;
}

/** Sets analysis execution type. Throws exception if out of range. */
void CParameters::SetExecutionType(ExecutionType eExecutionType) {
  if (AUTOMATIC > eExecutionType || CENTRICALLY < eExecutionType)
    throw prg_error("Enumeration %d out of range [%d,%d].", "SetExecutionType()", eExecutionType, AUTOMATIC, CENTRICALLY);
  geExecutionType = eExecutionType;
}

/** Sets clusters to include type. Throws exception if out of range. */
void CParameters::SetIncludeClustersType(IncludeClustersType eIncludeClustersType) {
  if (ALLCLUSTERS > eIncludeClustersType || CLUSTERSINRANGE < eIncludeClustersType)
    throw prg_error("Enumeration %d out of range [%d,%d].", "SetIncludeClustersType()", eIncludeClustersType, ALLCLUSTERS, CLUSTERSINRANGE);
  geIncludeClustersType = eIncludeClustersType;
}

/** Sets maximum temporal cluster size. */
void CParameters::SetMaximumTemporalClusterSize(double dMaxTemporalClusterSize) {
  gdMaxTemporalClusterSize = dMaxTemporalClusterSize;
}

/** Sets maximum temporal cluster size type. Throws exception if out of range. */
void CParameters::SetMaximumTemporalClusterSizeType(TemporalSizeType eTemporalSizeType) {
  if (PERCENTAGETYPE > eTemporalSizeType || TIMETYPE < eTemporalSizeType)
    throw prg_error("Enumeration %d out of range [%d,%d].","SetMaximumTemporalClusterSizeType()",
                    eTemporalSizeType, PERCENTAGETYPE, TIMETYPE);
  geMaxTemporalClusterSizeType = eTemporalSizeType;
}

/** Sets meta locations data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file
    does not exist. */
void CParameters::setMetaLocationsFilename(const char * sMetaLocationsFileName, bool bCorrectForRelativePath) {
  gsMetaLocationsFilename = sMetaLocationsFileName;
  if (bCorrectForRelativePath)
    AssignMissingPath(gsMetaLocationsFilename);
}

/** Set ellipse non-compactness penalty type. */
void CParameters::SetNonCompactnessPenalty(NonCompactnessPenaltyType eType) {
  if (eType < NOPENALTY || eType > STRONGPENALTY)
    throw prg_error("Enumeration %d out of range [%d,%d].", "SetNonCompactnessPenalty()", eType, NOPENALTY, STRONGPENALTY);
  geNonCompactnessPenaltyType = eType;
}

/** Adjusts the number of data sets. */
void CParameters::SetNumDataSets(size_t iNumDataSets) {
  if (iNumDataSets == 0)
    throw prg_error("Number of data sets can not be zero.\n", "SetNumDataSets()");

  //adjust the number of filenames for case, control, and population
  gvCaseFilenames.resize(iNumDataSets);
  gvControlFilenames.resize(iNumDataSets);
  gvPopulationFilenames.resize(iNumDataSets);
}

/** Sets number of Monte Carlo replications to run. */
void CParameters::SetNumberMonteCarloReplications(unsigned int iReplications) {
  giReplications = iReplications;
}

/** Sets number of iterative scans to run. */
void CParameters::SetNumIterativeScans(int iNumIterativeScans) {
  giNumIterativeRuns = iNumIterativeScans;
}

/** Sets output data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file
    does not exist. */
void CParameters::SetOutputFileName(const char * sOutPutFileName, bool bCorrectForRelativePath) {
  gsOutputFileName = sOutPutFileName;
  if (bCorrectForRelativePath)
    AssignMissingPath(gsOutputFileName, true);
}

/** Sets population data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void CParameters::SetPopulationFileName(const char * sPopulationFileName, bool bCorrectForRelativePath, size_t tSetIndex) {
  if (!tSetIndex)
    throw prg_error("Index %s out of range [1,].", "SetPopulationFileName()", tSetIndex);

  if (tSetIndex > gvPopulationFilenames.size())
    gvPopulationFilenames.resize(tSetIndex);

  gvPopulationFilenames[tSetIndex - 1] = sPopulationFileName;
  if (bCorrectForRelativePath)
    AssignMissingPath(gvPopulationFilenames[tSetIndex - 1]);
}

/** Sets X variable for power calculation. Throws exception if out of range. */
void CParameters::SetPowerCalculationX(double dPowerX) {
  gdPower_X = dPowerX;
}

/** Sets Y variable for power calculation. Throws exception if out of range. */
void CParameters::SetPowerCalculationY(double dPowerY) {
  gdPower_Y = dPowerY;
}

/** Sets relative risks adjustments file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void CParameters::SetAdjustmentsByRelativeRisksFilename(const char * sFileName, bool bCorrectForRelativePath) {
  gsAdjustmentsByRelativeRisksFileName = sFileName;
  if (bCorrectForRelativePath)
    AssignMissingPath(gsAdjustmentsByRelativeRisksFileName);
}

/** Sets precision of input file dates type. Throws exception if out of range. */
void CParameters::SetPrecisionOfTimesType(DatePrecisionType eDatePrecisionType) {
  if (eDatePrecisionType < NONE || eDatePrecisionType > GENERIC)
    throw prg_error("Enumeration %d out of range [%d,%d].", "SetPrecisionOfTimesType()", eDatePrecisionType, NONE, GENERIC);
  gePrecisionOfTimesType = eDatePrecisionType;
}

/** Sets probability model type. Throws exception if out of range. */
void CParameters::SetProbabilityModelType(ProbabilityModelType eProbabilityModelType) {
  if (eProbabilityModelType < POISSON || eProbabilityModelType > RANK)
    throw prg_error("Enumeration %d out of range [%d,%d].", "SetAnalysisType()", eProbabilityModelType, POISSON, RANK);
  geProbabilityModelType = eProbabilityModelType;
}

/** Sets prospective start date. Throws exception if out of range. */
void CParameters::SetProspectiveStartDate(const char * sProspectiveStartDate) {
  gsProspectiveStartDate = sProspectiveStartDate;
}

/** Sets p-value reporting type. Throws exception if out of range. */
void CParameters::SetPValueReportingType(PValueReportingType eType) {
  if (eType < DEFAULT_PVALUE || eType > GUMBEL_PVALUE)
    throw prg_error("Enumeration %d out of range [%d,%d].", "SetPValueReportingType()", eType, DEFAULT_PVALUE, GUMBEL_PVALUE);
  gePValueReportingType = eType;
}

/** Set seed used by randomization process. */
void CParameters::SetRandomizationSeed(long lSeed) {
  glRandomizationSeed = lSeed;
}

/** Sets risk type. Throws exception if out of range. */
void CParameters::SetRiskType(RiskType eRiskType) {
  if (eRiskType < STANDARDRISK || eRiskType > MONOTONERISK)
    throw prg_error("Enumeration %d out of range [%d,%d].", "SetRiskType()", eRiskType, STANDARDRISK, MONOTONERISK);
  geRiskFunctionType = eRiskType;
}

/** sets simulation procedure type */
void CParameters::SetSimulationType(SimulationType eSimulationType) {
  if (eSimulationType < STANDARD || eSimulationType > FILESOURCE)
    throw prg_error("Enumeration %d out of range [%d,%d].", "SetSimulationType()", eSimulationType, STANDARD, FILESOURCE);
  geSimulationType = eSimulationType;
}

/** Sets simulation data output file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void CParameters::SetSimulationDataOutputFileName(const char * sSourceFileName, bool bCorrectForRelativePath) {
  gsSimulationDataOutputFilename = sSourceFileName;
  if (bCorrectForRelativePath)
    AssignMissingPath(gsSimulationDataOutputFilename);
}

/** Sets simulation data source file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void CParameters::SetSimulationDataSourceFileName(const char * sSourceFileName, bool bCorrectForRelativePath) {
  gsSimulationDataSourceFileName = sSourceFileName;
  if (bCorrectForRelativePath)
    AssignMissingPath(gsSimulationDataSourceFileName);
}

/** Set spatial adjustment type. Throws exception if out of range. */
void CParameters::SetSpatialAdjustmentType(SpatialAdjustmentType eSpatialAdjustmentType) {
  if (eSpatialAdjustmentType < NO_SPATIAL_ADJUSTMENT || eSpatialAdjustmentType > SPATIALLY_STRATIFIED_RANDOMIZATION)
    throw prg_error("Enumeration %d out of range [%d,%d].", "SetSpatialAdjustmentType()", eSpatialAdjustmentType, NO_SPATIAL_ADJUSTMENT, SPATIALLY_STRATIFIED_RANDOMIZATION);
  geSpatialAdjustmentType = eSpatialAdjustmentType;
}

/** Set spatial window shape type. Throws exception if out of range. */
void  CParameters::SetSpatialWindowType(SpatialWindowType eSpatialWindowType) {
  if (eSpatialWindowType < CIRCULAR || eSpatialWindowType > ELLIPTIC)
    throw prg_error("Enumeration %d out of range [%d,%d].", "SetSpatialWindowType()", eSpatialWindowType, CIRCULAR, ELLIPTIC);
  geSpatialWindowType = eSpatialWindowType;
}

/** Set p-value that is cut-off for Iterative scans. */
void CParameters::SetIterativeCutOffPValue(double dPValue) {
  gbIterativeCutOffPValue = dPValue;
}

/** Sets filename of file used to load parameters. */
void CParameters::SetSourceFileName(const char * sParametersSourceFileName) {
  //Use FileName class to ensure that a relative path is expanded to absolute path.
  std::string buffer;
  gsParametersSourceFileName = FileName(sParametersSourceFileName).getFullPath(buffer);
}

/** Sets special grid data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void CParameters::SetSpecialGridFileName(const char * sSpecialGridFileName, bool bCorrectForRelativePath, bool bSetUsingFlag) {
  gsSpecialGridFileName = sSpecialGridFileName;
  if (bCorrectForRelativePath)
    AssignMissingPath(gsSpecialGridFileName);

  if (gsSpecialGridFileName.empty())
    gbUseSpecialGridFile = false; //If empty, then definately not using special grid.
  else if (bSetUsingFlag)
    gbUseSpecialGridFile = true;  //Permits setting special grid filename in GUI interface
                                  //where obviously the use of special grid file is the desire.
  //else gbUseSpecialGridFile is as set from parameters read. This permits the situation
  //where user has modified the paramters file manually so that there is a named
  //special grid file but they turned off option to use it.
}

/** Sets maximum circle population data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void CParameters::SetMaxCirclePopulationFileName(const char * sMaxCirclePopulationFileName, bool bCorrectForRelativePath) {
  gsMaxCirclePopulationFileName = sMaxCirclePopulationFileName;
  if (bCorrectForRelativePath)
    AssignMissingPath(gsMaxCirclePopulationFileName);
}

/** Set multiple dataset purpose type. Throws exception if out of range. */
void CParameters::SetMultipleDataSetPurposeType(MultipleDataSetPurposeType eType) {
  if (eType < MULTIVARIATE || eType > ADJUSTMENT)
    throw prg_error("Enumeration %d out of range [%d,%d].", "SetMultipleDataSetPurposeType()", eType, MULTIVARIATE, ADJUSTMENT);
  geMultipleSetPurposeType = eType;
}

/** Set multiple coordinates type. Throws exception if out of range. */
void CParameters::SetMultipleCoordinatesType(MultipleCoordinatesType eMultipleCoordinatesType) {
  if (eMultipleCoordinatesType < ONEPERLOCATION || eMultipleCoordinatesType > ALLLOCATIONS)
    throw prg_error("Enumeration %d out of range [%d,%d].", "SetMultipleCoordinatesType()", eMultipleCoordinatesType, ONEPERLOCATION, ALLLOCATIONS);
  geMultipleCoordinatesType = eMultipleCoordinatesType;
}

/** Sets neighbor array data file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void CParameters::SetLocationNeighborsFileName(const char * sLocationNeighborsFileName, bool bCorrectForRelativePath) {
  gsLocationNeighborsFilename = sLocationNeighborsFileName;
  if (bCorrectForRelativePath)
    AssignMissingPath(gsLocationNeighborsFilename);
}

/** Sets study period data checking type. Throws exception if out of range. */
void CParameters::SetStudyPeriodDataCheckingType(StudyPeriodDataCheckingType eStudyPeriodDataCheckingType) {
  if (eStudyPeriodDataCheckingType < STRICTBOUNDS || eStudyPeriodDataCheckingType > RELAXEDBOUNDS)
    throw prg_error("Enumeration %d out of range [%d,%d].","SetStudyPeriodDataCheckingType()", eStudyPeriodDataCheckingType, STRICTBOUNDS, RELAXEDBOUNDS);
  geStudyPeriodDataCheckingType = eStudyPeriodDataCheckingType;
}

/** Sets study period start date. Throws exception if out of range. */
void CParameters::SetStudyPeriodEndDate(const char * sStudyPeriodEndDate) {
  gsStudyPeriodEndDate = sStudyPeriodEndDate;
}

/** Sets study period start date. Throws exception if out of range. */
void CParameters::SetStudyPeriodStartDate(const char * sStudyPeriodStartDate) {
  gsStudyPeriodStartDate = sStudyPeriodStartDate;
}

/** Sets time aggregation length. Throws exception if out of range. */
void CParameters::SetTimeAggregationLength(long lTimeAggregationLength) {
  glTimeAggregationLength = lTimeAggregationLength;
}

/** Sets precision of time interval units type. Throws exception if out of range. */
void CParameters::SetTimeAggregationUnitsType(DatePrecisionType eTimeAggregationUnits) {
  if (eTimeAggregationUnits < NONE || eTimeAggregationUnits > GENERIC)
    throw prg_error("Enumeration %d out of range [%d,%d].","SetTimeAggregationUnitsType()", eTimeAggregationUnits, NONE, GENERIC);
  geTimeAggregationUnitsType = eTimeAggregationUnits;
}

/** Sets time trend adjustment percentage. Throws exception if out of range. */
void CParameters::SetTimeTrendAdjustmentPercentage(double dPercentage) {
   gdTimeTrendAdjustPercentage = dPercentage;
}

/** Sets time rend adjustment type. Throws exception if out of range. */
void CParameters::SetTimeTrendAdjustmentType(TimeTrendAdjustmentType eTimeTrendAdjustmentType) {
  if (eTimeTrendAdjustmentType < NOTADJUSTED || eTimeTrendAdjustmentType > CALCULATED_QUADRATIC_PERC)
    throw prg_error("Enumeration %d out of range [%d,%d].", "SetTimeTrendAdjustmentType()",
                    eTimeTrendAdjustmentType, NOTADJUSTED, CALCULATED_QUADRATIC_PERC);
  geTimeTrendAdjustType = eTimeTrendAdjustmentType;
}

/** Sets time trend convergence variable. */
void CParameters::SetTimeTrendConvergence(double dTimeTrendConvergence) {
  gdTimeTrendConverge = dTimeTrendConvergence;
}

/** Sets time trend calculation type. */
void CParameters::setTimeTrendType(TimeTrendType eTimeTrendType) {
  if (eTimeTrendType < LINEAR || eTimeTrendType > QUADRATIC)
    throw prg_error("Enumeration %d out of range [%d,%d].", "setTimeTrendType()",
                    eTimeTrendType, LINEAR, QUADRATIC);
  geTimeTrendType = eTimeTrendType;
}

/** Set version number that indicates what version of SaTScan created these parameters. */
void CParameters::SetVersion(const CreationVersion& vVersion) {
  gCreationVersion = vVersion;
}

/** Returns indication of whether to use coordinates file. */
bool CParameters::UseCoordinatesFile() const {
    if (GetIsPurelyTemporalAnalysis()) {
        //When executing a purely temporal analysis, the coordinates file is ignored unless
        //the user is adjusting for relative risks and has defined a coordinates file. This
        //allows the user to use an adjustments file that defines adjustments for all
        //locations or adjustments at the locations level.
        return UseAdjustmentForRelativeRisksFile() && GetCoordinatesFileName().size() != 0;
    } else {
        return !UseLocationNeighborsFile();
    }
}

/** Returns indication of whether current parameter settings indicate that the max circle file should be read. */
bool CParameters::UseMaxCirclePopulationFile() const {
  bool  bAskForByUser;

  bAskForByUser = (gbRestrictMaxSpatialSizeThroughMaxCirclePopulationFile || gbRestrictMaxSpatialSizeThroughMaxCirclePopulationFile_Reported) &&
                  !GetIsPurelyTemporalAnalysis();
  return bAskForByUser;
}

/** Returns indication of whether multiple coordinates per location parameter is used. */
bool CParameters::UsingMultipleCoordinatesMetaLocations() const {
    return !GetIsPurelyTemporalAnalysis() && 
           !UseLocationNeighborsFile() && 
           geMultipleCoordinatesType != ONEPERLOCATION &&
           GetProbabilityModelType() != HOMOGENEOUSPOISSON;
}

/** Returns indication of whether meta locations file parameter is used. */
bool CParameters::UseMetaLocationsFile() const {
    return !GetIsPurelyTemporalAnalysis() && 
           UseLocationNeighborsFile() && 
           gbUseMetaLocationsFile;
}

/** Returns indication of whether non-Euclidian neighbors file parameter is used. */
bool CParameters::UseLocationNeighborsFile() const {
    return !GetIsPurelyTemporalAnalysis() && 
            gbUseLocationNeighborsFile &&
            GetProbabilityModelType() != HOMOGENEOUSPOISSON;
}

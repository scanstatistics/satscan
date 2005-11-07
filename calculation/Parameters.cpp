//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "Parameters.h"
#include "Randomizer.h"

const int CParameters::MAXIMUM_SEQUENTIAL_ANALYSES    = 32000;
const int CParameters::MAXIMUM_ELLIPSOIDS             = 10;
const int CParameters::giNumParameters 	              = 74;

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
  if (geSpatialWindowType                 != rhs.geSpatialWindowType) return false;
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
  //if (geExecutionType                     != rhs.geExecutionType) return false;
  if (giNumRequestedParallelProcesses     != rhs.giNumRequestedParallelProcesses) return false;
  if (gbSuppressWarnings                  != rhs.gbSuppressWarnings) return false;
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
    geSpatialWindowType                 = rhs.geSpatialWindowType;
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
    geMultipleSetPurposeType            = rhs.geMultipleSetPurposeType;
    gCreationVersion                    = rhs.gCreationVersion;
    gbUsePopulationFile                 = rhs.gbUsePopulationFile;
    glRandomizationSeed                 = rhs.glRandomizationSeed;
    gbReportCriticalValues              = rhs.gbReportCriticalValues;
    geExecutionType                     = rhs.geExecutionType;
    giNumRequestedParallelProcesses     = rhs.giNumRequestedParallelProcesses;
    gbSuppressWarnings                  = rhs.gbSuppressWarnings;
  }
  catch (ZdException & x) {
    x.AddCallpath("Copy()", "CParameters");
    throw;
  }
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
  return (geProbabilityModelType == SPACETIMEPERMUTATION ||
          (geSpatialWindowType == ELLIPTIC && gbNonCompactnessPenalty));
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
bool CParameters::GetPermitsPurelySpatialCluster() const {
  return geAnalysisType == PURELYSPATIAL || geAnalysisType == SPACETIME || GetIsProspectiveAnalysis();
}

/** returns whether probability model type permits inclusion of purely spatial cluster */
bool CParameters::GetPermitsPurelySpatialCluster(ProbabilityModelType eModelType) const {
  return eModelType == POISSON || eModelType == BERNOULLI || eModelType == NORMAL
         || eModelType == EXPONENTIAL || eModelType == RANK || eModelType == ORDINAL;
}

/** returns whether analysis type permits inclusion of purely temporal cluster */
bool CParameters::GetPermitsPurelyTemporalCluster() const {
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
  geSpatialWindowType                   = CIRCULAR;
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
  geMultipleSetPurposeType              = MULTIVARIATE;
  gCreationVersion.iMajor               = atoi(VERSION_MAJOR);
  gCreationVersion.iMinor               = atoi(VERSION_MINOR);
  gCreationVersion.iRelease             = atoi(VERSION_RELEASE);
  gbUsePopulationFile                   = false;
  glRandomizationSeed                   = RandomNumberGenerator::glDefaultSeed;
  gbReportCriticalValues                = false;
  geExecutionType                       = AUTOMATIC;
  giNumRequestedParallelProcesses       = 0;
  gbSuppressWarnings                    = false;
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

/** Sets analysis execution type. Throws exception if out of range. */
void CParameters::SetExecutionType(ExecutionType eExecutionType) {
  try {
    if (AUTOMATIC > eExecutionType || CENTRICALLY < eExecutionType)
      ZdException::Generate("'%d' is out of range(%d - %d).", "SetExecutionType()",
                            eExecutionType, AUTOMATIC, CENTRICALLY);
    geExecutionType = eExecutionType;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetExecutionType()","CParameters");
    throw;
  }
}

/** Sets clusters to include type. Throws exception if out of range. */
void CParameters::SetIncludeClustersType(IncludeClustersType eIncludeClustersType) {
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

/** Sets number of Monte Carlo replications to run. */
void CParameters::SetNumberMonteCarloReplications(unsigned int iReplications) {
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

/** Set spatial window shape type. Throws exception if out of range. */
void  CParameters::SetSpatialWindowType(SpatialWindowType eSpatialWindowType) {
  ZdString      sLabel;

  try {
    if (eSpatialWindowType < CIRCULAR || eSpatialWindowType > ELLIPTIC)
      ZdException::Generate("'%d' is out of range(%d - %d).", "SetSpatialWindowType()", eSpatialWindowType, CIRCULAR, ELLIPTIC);
    geSpatialWindowType = eSpatialWindowType;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetSpatialWindowType()","CParameters");
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
void CParameters::SetMaxCirclePopulationFileName(const char * sMaxCirclePopulationFileName, bool bCorrectForRelativePath) {
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


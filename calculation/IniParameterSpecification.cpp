//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "IniParameterSpecification.h"

const char * IniParameterSpecification::NotUsed                 = "NotUsed";

const char * IniParameterSpecification::Input                   = "[Input]";
const char * IniParameterSpecification::MultipleDataSets        = "[Multiple Data Sets]";
const char * IniParameterSpecification::Analysis                = "[Analysis]";
const char * IniParameterSpecification::SpatialWindow           = "[Spatial Window]";
const char * IniParameterSpecification::TemporalWindow          = "[Temporal Window]";
const char * IniParameterSpecification::SpaceAndTimeAdjustments = "[Space and Time Adjustments]";
const char * IniParameterSpecification::Inference               = "[Inference]";
const char * IniParameterSpecification::Output                  = "[Output]";
const char * IniParameterSpecification::ClustersReported        = "[Clusters Reported]";

const char * IniParameterSpecification::EllipticScan            = "[Elliptic Scan]";
const char * IniParameterSpecification::SequentialScan          = "[Sequential Scan]";
const char * IniParameterSpecification::IsotonicScan            = "[Isotonic Scan]";
const char * IniParameterSpecification::PowerSimulations        = "[Power Simulations]";
const char * IniParameterSpecification::BatchModeFeatures       = "[BatchMode Features]";
const char * IniParameterSpecification::System                  = "[System]";

const char * IniParameterSpecification::InputFiles              = "[Input Files]";
const char * IniParameterSpecification::ScanningWindow          = "[Scanning Window]";
const char * IniParameterSpecification::TimeParameters          = "[Time Parameters]";
const char * IniParameterSpecification::OutputFiles             = "[Output Files]";
const char * IniParameterSpecification::AdvancedFeatures        = "[Advanced Features]";

/** constructor -- builds specification for write process */
IniParameterSpecification::IniParameterSpecification() {
  Build_5_1_x_ParameterList();
}

/** constructor -- builds specification for read process */
IniParameterSpecification::IniParameterSpecification(const ZdIniFile& SourceFile) {
  long                          lSectionIndex, lKeyIndex;
  CParameters::CreationVersion  Version = {4, 0, 3};

  if ((lSectionIndex = SourceFile.GetSectionIndex("[System]")) > -1) {
    const ZdIniSection * pSection = SourceFile.GetSection(lSectionIndex);
    if ((lKeyIndex = pSection->FindKey("Version")) > -1)
      sscanf(pSection->GetLine(lKeyIndex)->GetValue(), "%u.%u.%u", &Version.iMajor, &Version.iMinor, &Version.iRelease);
  }

  if (Version.iMajor == 4  && Version.iMinor == 0 && Version.iRelease == 3)
    Build_4_0_x_ParameterList();
  else if (Version.iMajor == 5  && Version.iMinor == 0)
    Build_5_0_x_ParameterList();
  else
    Build_5_1_x_ParameterList();
}

/** destructor */
IniParameterSpecification::~IniParameterSpecification() {}

void IniParameterSpecification::BuildPrimaryParameterList() {
  // Order in vector is essential - should identical to ParameterType enumeration.
  gvParameterInfo.push_back(std::make_pair(Analysis, "AnalysisType"));
  gvParameterInfo.push_back(std::make_pair(Analysis, "ScanAreas"));
  gvParameterInfo.push_back(std::make_pair(InputFiles, "CaseFile"));
  gvParameterInfo.push_back(std::make_pair(InputFiles, "PopulationFile"));
  gvParameterInfo.push_back(std::make_pair(InputFiles, "CoordinatesFile"));
  gvParameterInfo.push_back(std::make_pair(OutputFiles, "ResultsFile"));
  gvParameterInfo.push_back(std::make_pair(InputFiles, "PrecisionCaseTimes"));
  gvParameterInfo.push_back(std::make_pair(NotUsed, NotUsed));
  gvParameterInfo.push_back(std::make_pair(InputFiles, "UseGridFile"));
  gvParameterInfo.push_back(std::make_pair(InputFiles, "GridFile"));   
  gvParameterInfo.push_back(std::make_pair(ScanningWindow, "MaxGeographicSize"));
  gvParameterInfo.push_back(std::make_pair(Analysis, "StartDate"));
  gvParameterInfo.push_back(std::make_pair(Analysis, "EndDate"));
  gvParameterInfo.push_back(std::make_pair(ScanningWindow, "IncludeClusters"));
  gvParameterInfo.push_back(std::make_pair(NotUsed, NotUsed));
  gvParameterInfo.push_back(std::make_pair(TimeParameters, "IntervalUnits"));
  gvParameterInfo.push_back(std::make_pair(TimeParameters, "IntervalLength"));
  gvParameterInfo.push_back(std::make_pair(ScanningWindow, "IncludePurelySpatial"));
  gvParameterInfo.push_back(std::make_pair(ScanningWindow, "MaxTemporalSize"));
  gvParameterInfo.push_back(std::make_pair(Analysis, "MonteCarloReps"));
  gvParameterInfo.push_back(std::make_pair(Analysis, "ModelType"));
  gvParameterInfo.push_back(std::make_pair(AdvancedFeatures, "IsotonicScan"));
  gvParameterInfo.push_back(std::make_pair(AdvancedFeatures, "PValues2PrespecifiedLLRs"));
  gvParameterInfo.push_back(std::make_pair(AdvancedFeatures, "LLR1"));
  gvParameterInfo.push_back(std::make_pair(AdvancedFeatures, "LLR2"));
  gvParameterInfo.push_back(std::make_pair(TimeParameters, "TimeTrendAdjustmentType"));
  gvParameterInfo.push_back(std::make_pair(TimeParameters, "TimeTrendPercentage"));
  gvParameterInfo.push_back(std::make_pair(ScanningWindow, "IncludePurelyTemporal"));
  gvParameterInfo.push_back(std::make_pair(InputFiles, "ControlFile"));
  gvParameterInfo.push_back(std::make_pair(InputFiles, "CoordinatesType"));
  gvParameterInfo.push_back(std::make_pair(OutputFiles, "SaveSimLLRsASCII"));
  gvParameterInfo.push_back(std::make_pair(SequentialScan, "SequentialScan"));
  gvParameterInfo.push_back(std::make_pair(SequentialScan, "SequentialScanMaxIterations"));
  gvParameterInfo.push_back(std::make_pair(SequentialScan, "SequentialScanMaxPValue"));
  gvParameterInfo.push_back(std::make_pair(AdvancedFeatures, "ValidateParameters"));
  gvParameterInfo.push_back(std::make_pair(OutputFiles, "IncludeRelativeRisksCensusAreasASCII"));
  gvParameterInfo.push_back(std::make_pair(EllipticScan, "NumberOfEllipses"));
  gvParameterInfo.push_back(std::make_pair(EllipticScan, "EllipseShapes"));
  gvParameterInfo.push_back(std::make_pair(EllipticScan, "EllipseAngles"));
  gvParameterInfo.push_back(std::make_pair(TimeParameters, "ProspectiveStartDate"));
  gvParameterInfo.push_back(std::make_pair(OutputFiles, "CensusAreasReportedClustersASCII"));
  gvParameterInfo.push_back(std::make_pair(OutputFiles, "MostLikelyClusterEachCentroidASCII"));
  gvParameterInfo.push_back(std::make_pair(OutputFiles, "CriteriaForReportingSecondaryClusters"));
  gvParameterInfo.push_back(std::make_pair(ScanningWindow, "MaxTemporalSizeInterpretation"));
  gvParameterInfo.push_back(std::make_pair(ScanningWindow, "MaxSpatialSizeInterpretation"));
  gvParameterInfo.push_back(std::make_pair(NotUsed, NotUsed));
  gvParameterInfo.push_back(std::make_pair(OutputFiles, "MostLikelyClusterEachCentroidDBase"));
  gvParameterInfo.push_back(std::make_pair(OutputFiles, "CensusAreasReportedClustersDBase"));
  gvParameterInfo.push_back(std::make_pair(OutputFiles, "IncludeRelativeRisksCensusAreasDBase"));
  gvParameterInfo.push_back(std::make_pair(OutputFiles, "SaveSimLLRsDBase"));
  gvParameterInfo.push_back(std::make_pair(EllipticScan, "DuczmalCompactnessCorrection"));
}

void IniParameterSpecification::Build_4_0_x_ParameterList() {
  BuildPrimaryParameterList();
  //Versions 4.0.x made no name changes, only defined more parameters
  gvParameterInfo.push_back(std::make_pair(ScanningWindow, "IntervalStartRange"));
  gvParameterInfo.push_back(std::make_pair(ScanningWindow, "IntervalEndRange"));
  gvParameterInfo.push_back(std::make_pair(TimeParameters, "TimeTrendConvergence"));
  gvParameterInfo.push_back(std::make_pair(AdvancedFeatures, "MaxCirclePopulationFile"));
  gvParameterInfo.push_back(std::make_pair(AdvancedFeatures, "EarlySimulationTermination"));
  gvParameterInfo.push_back(std::make_pair(OutputFiles, "MaxReportedGeoClusterSize"));
  gvParameterInfo.push_back(std::make_pair(OutputFiles, "UseReportOnlySmallerClusters"));
  gvParameterInfo.push_back(std::make_pair(AdvancedFeatures, "SimulatedDataMethodType"));
  gvParameterInfo.push_back(std::make_pair(AdvancedFeatures, "SimulatedDataInputFilename"));
  gvParameterInfo.push_back(std::make_pair(AdvancedFeatures, "AdjustmentsByKnownRelativeRisksFilename"));
  gvParameterInfo.push_back(std::make_pair(AdvancedFeatures, "PrintSimulatedDataToFile"));
  gvParameterInfo.push_back(std::make_pair(AdvancedFeatures, "SimulatedDataOutputFilename"));
  gvParameterInfo.push_back(std::make_pair(TimeParameters, "AdjustForEarlierAnalyses"));
  gvParameterInfo.push_back(std::make_pair(AdvancedFeatures, "UseAdjustmentsByRRFile"));
}

void IniParameterSpecification::Build_5_0_x_ParameterList() {
  Build_4_0_x_ParameterList();
  //Version 5.0.0 updated these three ini key names and defined several more parameters
  gvParameterInfo[static_cast<int>(TIME_AGGREGATION_UNITS) - 1].second = "TimeAggregationUnits";
  gvParameterInfo[static_cast<int>(TIME_AGGREGATION) - 1].second = "TimeAggregationLength";
  gvParameterInfo[static_cast<int>(NON_COMPACTNESS_PENALTY) - 1].second = "NonCompactnessPenalty";
  gvParameterInfo.push_back(std::make_pair(AdvancedFeatures, "SpatialAdjustmentType"));
  gvParameterInfo.push_back(std::make_pair(AdvancedFeatures, "MultipleDataSetsPurposeType"));
  gvMultipleParameterInfo[CASEFILE] = std::make_pair(Input, "CaseFile");
  gvMultipleParameterInfo[CONTROLFILE] = std::make_pair(Input, "ControlFile");
  gvMultipleParameterInfo[POPFILE] = std::make_pair(Input, "PopulationFile");
  gvParameterInfo.push_back(std::make_pair(System, "Version"));
}

/** Version 5.1.x made a major revision to the structure of the ini file. This was partly due
   to a specifications miscommunication with Martin when parameter file was converted to
   ini format. Since the desire is to allows mimic the graphical interface within the
   paramter file, we need to move sections around accordingly. */
void IniParameterSpecification::Build_5_1_x_ParameterList() {

//  -------- not done -- only copied

  // Order in vector is essential - should identical to ParameterType enumeration.
  gvParameterInfo.push_back(std::make_pair(Analysis, "AnalysisType"));
  gvParameterInfo.push_back(std::make_pair(Analysis, "ScanAreas"));
  gvParameterInfo.push_back(std::make_pair(Input, "CaseFile"));
  gvParameterInfo.push_back(std::make_pair(Input, "PopulationFile"));
  gvParameterInfo.push_back(std::make_pair(Input, "CoordinatesFile"));
  gvParameterInfo.push_back(std::make_pair(Output, "ResultsFile"));
  gvParameterInfo.push_back(std::make_pair(Input, "PrecisionCaseTimes"));
  gvParameterInfo.push_back(std::make_pair(NotUsed, NotUsed));
  gvParameterInfo.push_back(std::make_pair(Input, "UseGridFile"));
  gvParameterInfo.push_back(std::make_pair(Input, "GridFile"));
  gvParameterInfo.push_back(std::make_pair(SpatialWindow, "MaxGeographicSize"));
  gvParameterInfo.push_back(std::make_pair(Input, "StartDate"));
  gvParameterInfo.push_back(std::make_pair(Input, "EndDate"));
  gvParameterInfo.push_back(std::make_pair(TemporalWindow, "IncludeClusters"));
  gvParameterInfo.push_back(std::make_pair(NotUsed, NotUsed));
  gvParameterInfo.push_back(std::make_pair(Analysis, "TimeAggregationUnits"));
  gvParameterInfo.push_back(std::make_pair(Analysis, "TimeAggregationLength"));
  gvParameterInfo.push_back(std::make_pair(TemporalWindow, "IncludePurelySpatial"));
  gvParameterInfo.push_back(std::make_pair(TemporalWindow, "MaxTemporalSize"));
  gvParameterInfo.push_back(std::make_pair(Analysis, "MonteCarloReps"));
  gvParameterInfo.push_back(std::make_pair(Analysis, "ModelType"));
  gvParameterInfo.push_back(std::make_pair(IsotonicScan, "IsotonicScan"));
  gvParameterInfo.push_back(std::make_pair(PowerSimulations, "PValues2PrespecifiedLLRs"));
  gvParameterInfo.push_back(std::make_pair(PowerSimulations, "LLR1"));
  gvParameterInfo.push_back(std::make_pair(PowerSimulations, "LLR2"));
  gvParameterInfo.push_back(std::make_pair(SpaceAndTimeAdjustments, "TimeTrendAdjustmentType"));
  gvParameterInfo.push_back(std::make_pair(SpaceAndTimeAdjustments, "TimeTrendPercentage"));
  gvParameterInfo.push_back(std::make_pair(SpatialWindow, "IncludePurelyTemporal"));
  gvParameterInfo.push_back(std::make_pair(Input, "ControlFile"));
  gvParameterInfo.push_back(std::make_pair(Input, "CoordinatesType"));
  gvParameterInfo.push_back(std::make_pair(Output, "SaveSimLLRsASCII"));
  gvParameterInfo.push_back(std::make_pair(SequentialScan, "SequentialScan"));
  gvParameterInfo.push_back(std::make_pair(SequentialScan, "SequentialScanMaxIterations"));
  gvParameterInfo.push_back(std::make_pair(SequentialScan, "SequentialScanMaxPValue"));
  gvParameterInfo.push_back(std::make_pair(BatchModeFeatures, "ValidateParameters"));
  gvParameterInfo.push_back(std::make_pair(Output, "IncludeRelativeRisksCensusAreasASCII"));
  gvParameterInfo.push_back(std::make_pair(EllipticScan, "NumberOfEllipses"));
  gvParameterInfo.push_back(std::make_pair(EllipticScan, "EllipseShapes"));
  gvParameterInfo.push_back(std::make_pair(EllipticScan, "EllipseAngles"));
  gvParameterInfo.push_back(std::make_pair(Inference, "ProspectiveStartDate"));
  gvParameterInfo.push_back(std::make_pair(Output, "CensusAreasReportedClustersASCII"));
  gvParameterInfo.push_back(std::make_pair(Output, "MostLikelyClusterEachCentroidASCII"));
  gvParameterInfo.push_back(std::make_pair(ClustersReported, "CriteriaForReportingSecondaryClusters"));
  gvParameterInfo.push_back(std::make_pair(TemporalWindow, "MaxTemporalSizeInterpretation"));
  gvParameterInfo.push_back(std::make_pair(SpatialWindow, "MaxSpatialSizeInterpretation"));
  gvParameterInfo.push_back(std::make_pair(NotUsed, NotUsed));
  gvParameterInfo.push_back(std::make_pair(Output, "MostLikelyClusterEachCentroidDBase"));
  gvParameterInfo.push_back(std::make_pair(Output, "CensusAreasReportedClustersDBase"));
  gvParameterInfo.push_back(std::make_pair(Output, "IncludeRelativeRisksCensusAreasDBase"));
  gvParameterInfo.push_back(std::make_pair(Output, "SaveSimLLRsDBase"));
  gvParameterInfo.push_back(std::make_pair(EllipticScan, "NonCompactnessPenalty"));
  gvParameterInfo.push_back(std::make_pair(SpaceAndTimeAdjustments, "IntervalStartRange"));
  gvParameterInfo.push_back(std::make_pair(SpaceAndTimeAdjustments, "IntervalEndRange"));
  gvParameterInfo.push_back(std::make_pair(BatchModeFeatures, "TimeTrendConvergence"));
  gvParameterInfo.push_back(std::make_pair(SpatialWindow, "MaxCirclePopulationFile"));
  gvParameterInfo.push_back(std::make_pair(Inference, "EarlySimulationTermination"));
  gvParameterInfo.push_back(std::make_pair(ClustersReported, "MaxReportedGeoClusterSize"));
  gvParameterInfo.push_back(std::make_pair(ClustersReported, "UseReportOnlySmallerClusters"));
  gvParameterInfo.push_back(std::make_pair(PowerSimulations, "SimulatedDataMethodType"));
  gvParameterInfo.push_back(std::make_pair(PowerSimulations, "SimulatedDataInputFilename"));
  gvParameterInfo.push_back(std::make_pair(SpaceAndTimeAdjustments, "AdjustmentsByKnownRelativeRisksFilename"));
  gvParameterInfo.push_back(std::make_pair(PowerSimulations, "PrintSimulatedDataToFile"));
  gvParameterInfo.push_back(std::make_pair(PowerSimulations, "SimulatedDataOutputFilename"));
  gvParameterInfo.push_back(std::make_pair(Inference, "AdjustForEarlierAnalyses"));
  gvParameterInfo.push_back(std::make_pair(SpaceAndTimeAdjustments, "UseAdjustmentsByRRFile"));
  gvParameterInfo.push_back(std::make_pair(SpaceAndTimeAdjustments, "SpatialAdjustmentType"));
  gvParameterInfo.push_back(std::make_pair(MultipleDataSets, "MultipleDataSetsPurposeType"));
  gvParameterInfo.push_back(std::make_pair(System, "Version"));

  gvMultipleParameterInfo[CASEFILE] = std::make_pair(MultipleDataSets, "CaseFile");
  gvMultipleParameterInfo[CONTROLFILE] = std::make_pair(MultipleDataSets, "ControlFile");
  gvMultipleParameterInfo[POPFILE] = std::make_pair(MultipleDataSets, "PopulationFile");
}

/** For sepcified ParameterType, attempts to retrieve ini section and key name if ini file.
    Returns true if parameter found else false. */
bool IniParameterSpecification::GetParameterIniInfo(ParameterType eParameterType,  const char ** sSectionName, const char ** sKey) const {
  size_t        tParamIndex = static_cast<size_t>(eParameterType); //remember that ParameterType enumeration starts at one
  bool          bReturn = false;

  if (tParamIndex > 0  && tParamIndex <= gvParameterInfo.size()) {
    *sSectionName = gvParameterInfo[tParamIndex - 1].first;
    *sKey = gvParameterInfo[tParamIndex - 1].second;
    bReturn = true;
  }

  return bReturn;
}

/** For sepcified ParameterType, attempts to retrieve ini section and key name if ini file.
    Returns true if parameter found else false. */
bool IniParameterSpecification::GetMultipleParameterIniInfo(ParameterType eParameterType,  const char ** sSectionName, const char ** sKey) const {
  bool          bReturn = false;
  MultipleParameterInfo_t::const_iterator itr;

  itr = gvMultipleParameterInfo.find(eParameterType);
  if (itr != gvMultipleParameterInfo.end()) {
    *sSectionName = itr->second.first;
    *sKey = itr->second.second;
    bReturn = true;
  }

  return bReturn;
}


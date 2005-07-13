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
const char * IniParameterSpecification::RunOptions              = "[Run Options]";
const char * IniParameterSpecification::BatchModeFeatures       = "[BatchMode Features]";
const char * IniParameterSpecification::System                  = "[System]";

const char * IniParameterSpecification::InputFiles              = "[Input Files]";
const char * IniParameterSpecification::ScanningWindow          = "[Scanning Window]";
const char * IniParameterSpecification::TimeParameters          = "[Time Parameters]";
const char * IniParameterSpecification::OutputFiles             = "[Output Files]";
const char * IniParameterSpecification::AdvancedFeatures        = "[Advanced Features]";

/** constructor -- builds specification for write process */
IniParameterSpecification::IniParameterSpecification() {
  Build_6_0_x_ParameterList();
}

/** constructor -- builds specification for read process */
IniParameterSpecification::IniParameterSpecification(const ZdIniFile& SourceFile) {
  long                          lSectionIndex, lKeyIndex;
  CParameters::CreationVersion  Version = {4, 0, 3};

  if ((lSectionIndex = SourceFile.GetSectionIndex(System)) > -1) {
    const ZdIniSection * pSection = SourceFile.GetSection(lSectionIndex);
    if ((lKeyIndex = pSection->FindKey("Version")) > -1)
      sscanf(pSection->GetLine(lKeyIndex)->GetValue(), "%u.%u.%u", &Version.iMajor, &Version.iMinor, &Version.iRelease);
  }

  if (Version.iMajor == 4  && Version.iMinor == 0 && Version.iRelease == 3)
    Build_4_0_x_ParameterList();
  else if (Version.iMajor == 5  && Version.iMinor == 0)
    Build_5_0_x_ParameterList();
  else
    Build_6_0_x_ParameterList();
}

/** destructor */
IniParameterSpecification::~IniParameterSpecification() {}

/** Version 3.0.5 and priot parameter section/keys.
    NOTE: The (const char *) cast is needed for gcc 2.96 or less, but otherwise is irrelevant. */
void IniParameterSpecification::BuildPrimaryParameterList() {
  // Order in vector is essential - should identical to ParameterType enumeration.
  gvParameterInfo.push_back(std::make_pair(Analysis, (const char*)"AnalysisType"));
  gvParameterInfo.push_back(std::make_pair(Analysis, (const char*)"ScanAreas"));
  gvParameterInfo.push_back(std::make_pair(InputFiles, (const char*)"CaseFile"));
  gvParameterInfo.push_back(std::make_pair(InputFiles, (const char*)"PopulationFile"));
  gvParameterInfo.push_back(std::make_pair(InputFiles, (const char*)"CoordinatesFile"));
  gvParameterInfo.push_back(std::make_pair(OutputFiles, (const char*)"ResultsFile"));
  gvParameterInfo.push_back(std::make_pair(InputFiles, (const char*)"PrecisionCaseTimes"));
  gvParameterInfo.push_back(std::make_pair(NotUsed, NotUsed));
  gvParameterInfo.push_back(std::make_pair(InputFiles, (const char*)"UseGridFile"));
  gvParameterInfo.push_back(std::make_pair(InputFiles, (const char*)"GridFile"));   
  gvParameterInfo.push_back(std::make_pair(ScanningWindow, (const char*)"MaxGeographicSize"));
  gvParameterInfo.push_back(std::make_pair(Analysis, (const char*)"StartDate"));
  gvParameterInfo.push_back(std::make_pair(Analysis, (const char*)"EndDate"));
  gvParameterInfo.push_back(std::make_pair(ScanningWindow, (const char*)"IncludeClusters"));
  gvParameterInfo.push_back(std::make_pair(NotUsed, NotUsed));
  gvParameterInfo.push_back(std::make_pair(TimeParameters, (const char*)"IntervalUnits"));
  gvParameterInfo.push_back(std::make_pair(TimeParameters, (const char*)"IntervalLength"));
  gvParameterInfo.push_back(std::make_pair(ScanningWindow, (const char*)"IncludePurelySpatial"));
  gvParameterInfo.push_back(std::make_pair(ScanningWindow, (const char*)"MaxTemporalSize"));
  gvParameterInfo.push_back(std::make_pair(Analysis, (const char*)"MonteCarloReps"));
  gvParameterInfo.push_back(std::make_pair(Analysis, (const char*)"ModelType"));
  gvParameterInfo.push_back(std::make_pair(AdvancedFeatures, (const char*)"IsotonicScan"));
  gvParameterInfo.push_back(std::make_pair(AdvancedFeatures, (const char*)"PValues2PrespecifiedLLRs"));
  gvParameterInfo.push_back(std::make_pair(AdvancedFeatures, (const char*)"LLR1"));
  gvParameterInfo.push_back(std::make_pair(AdvancedFeatures, (const char*)"LLR2"));
  gvParameterInfo.push_back(std::make_pair(TimeParameters, (const char*)"TimeTrendAdjustmentType"));
  gvParameterInfo.push_back(std::make_pair(TimeParameters, (const char*)"TimeTrendPercentage"));
  gvParameterInfo.push_back(std::make_pair(ScanningWindow, (const char*)"IncludePurelyTemporal"));
  gvParameterInfo.push_back(std::make_pair(InputFiles, (const char*)"ControlFile"));
  gvParameterInfo.push_back(std::make_pair(InputFiles, (const char*)"CoordinatesType"));
  gvParameterInfo.push_back(std::make_pair(OutputFiles, (const char*)"SaveSimLLRsASCII"));
  gvParameterInfo.push_back(std::make_pair(SequentialScan, (const char*)"SequentialScan"));
  gvParameterInfo.push_back(std::make_pair(SequentialScan, (const char*)"SequentialScanMaxIterations"));
  gvParameterInfo.push_back(std::make_pair(SequentialScan, (const char*)"SequentialScanMaxPValue"));
  gvParameterInfo.push_back(std::make_pair(AdvancedFeatures, (const char*)"ValidateParameters"));
  gvParameterInfo.push_back(std::make_pair(OutputFiles, (const char*)"IncludeRelativeRisksCensusAreasASCII"));
  gvParameterInfo.push_back(std::make_pair(EllipticScan, (const char*)"NumberOfEllipses"));
  gvParameterInfo.push_back(std::make_pair(EllipticScan, (const char*)"EllipseShapes"));
  gvParameterInfo.push_back(std::make_pair(EllipticScan, (const char*)"EllipseAngles"));
  gvParameterInfo.push_back(std::make_pair(TimeParameters, (const char*)"ProspectiveStartDate"));
  gvParameterInfo.push_back(std::make_pair(OutputFiles, (const char*)"CensusAreasReportedClustersASCII"));
  gvParameterInfo.push_back(std::make_pair(OutputFiles, (const char*)"MostLikelyClusterEachCentroidASCII"));
  gvParameterInfo.push_back(std::make_pair(OutputFiles, (const char*)"CriteriaForReportingSecondaryClusters"));
  gvParameterInfo.push_back(std::make_pair(ScanningWindow, (const char*)"MaxTemporalSizeInterpretation"));
  gvParameterInfo.push_back(std::make_pair(ScanningWindow, (const char*)"MaxSpatialSizeInterpretation"));
  gvParameterInfo.push_back(std::make_pair(NotUsed, NotUsed));
  gvParameterInfo.push_back(std::make_pair(OutputFiles, (const char*)"MostLikelyClusterEachCentroidDBase"));
  gvParameterInfo.push_back(std::make_pair(OutputFiles, (const char*)"CensusAreasReportedClustersDBase"));
  gvParameterInfo.push_back(std::make_pair(OutputFiles, (const char*)"IncludeRelativeRisksCensusAreasDBase"));
  gvParameterInfo.push_back(std::make_pair(OutputFiles, (const char*)"SaveSimLLRsDBase"));
  gvParameterInfo.push_back(std::make_pair(EllipticScan, (const char*)"DuczmalCompactnessCorrection"));
}

/** Version 4.0.x parameter section/keys. 
    NOTE: The (const char *) cast is needed for gcc 2.96 or less, but otherwise is irrelevant. */
void IniParameterSpecification::Build_4_0_x_ParameterList() {
  BuildPrimaryParameterList();
  //Versions 4.0.x made no name changes, only defined more parameters
  gvParameterInfo.push_back(std::make_pair(ScanningWindow, (const char*)"IntervalStartRange"));
  gvParameterInfo.push_back(std::make_pair(ScanningWindow, (const char*)"IntervalEndRange"));
  gvParameterInfo.push_back(std::make_pair(NotUsed, NotUsed));
  gvParameterInfo.push_back(std::make_pair(AdvancedFeatures, (const char*)"MaxCirclePopulationFile"));
  gvParameterInfo.push_back(std::make_pair(AdvancedFeatures, (const char*)"EarlySimulationTermination"));
  gvParameterInfo.push_back(std::make_pair(OutputFiles, (const char*)"MaxReportedGeoClusterSize"));
  gvParameterInfo.push_back(std::make_pair(OutputFiles, (const char*)"UseReportOnlySmallerClusters"));
  gvParameterInfo.push_back(std::make_pair(AdvancedFeatures, (const char*)"SimulatedDataMethodType"));
  gvParameterInfo.push_back(std::make_pair(AdvancedFeatures, (const char*)"SimulatedDataInputFilename"));
  gvParameterInfo.push_back(std::make_pair(AdvancedFeatures, (const char*)"AdjustmentsByKnownRelativeRisksFilename"));
  gvParameterInfo.push_back(std::make_pair(AdvancedFeatures, (const char*)"PrintSimulatedDataToFile"));
  gvParameterInfo.push_back(std::make_pair(AdvancedFeatures, (const char*)"SimulatedDataOutputFilename"));
  gvParameterInfo.push_back(std::make_pair(TimeParameters, (const char*)"AdjustForEarlierAnalyses"));
  gvParameterInfo.push_back(std::make_pair(AdvancedFeatures, (const char*)"UseAdjustmentsByRRFile"));
}

/** Version 5.0 parameter section/keys. 
    NOTE: The (const char *) cast is needed for gcc 2.96 or less, but otherwise is irrelevant. */
void IniParameterSpecification::Build_5_0_x_ParameterList() {
  Build_4_0_x_ParameterList();
  //Version 5.0.0 updated these three ini key names and defined several more parameters
  gvParameterInfo[static_cast<int>(TIME_AGGREGATION_UNITS) - 1].second = "TimeAggregationUnits";
  gvParameterInfo[static_cast<int>(TIME_AGGREGATION) - 1].second = "TimeAggregationLength";
  gvParameterInfo[static_cast<int>(NON_COMPACTNESS_PENALTY) - 1].second = "NonCompactnessPenalty";
  gvParameterInfo.push_back(std::make_pair(AdvancedFeatures, (const char*)"SpatialAdjustmentType"));
  gvParameterInfo.push_back(std::make_pair(AdvancedFeatures, (const char*)"MultipleDataSetsPurposeType"));
  gvMultipleParameterInfo[CASEFILE] = std::make_pair(Input, (const char*)"CaseFile");
  gvMultipleParameterInfo[CONTROLFILE] = std::make_pair(Input, (const char*)"ControlFile");
  gvMultipleParameterInfo[POPFILE] = std::make_pair(Input, (const char*)"PopulationFile");
  gvParameterInfo.push_back(std::make_pair(System, (const char*)"Version"));
}

/** Version 5.1.x made a major revision to the structure of the ini file. This was partly due
   to a specifications miscommunication with Martin when parameter file was converted to
   ini format. Since the desire is to allows mimic the graphical interface within the
   paramter file, we need to move sections around accordingly.
   NOTE: The (const char *) cast is needed for gcc 2.96 or less, but otherwise is irrelevant. */
void IniParameterSpecification::Build_5_1_x_ParameterList() {
  // Order in vector is essential - should identical to ParameterType enumeration.
  gvParameterInfo.push_back(std::make_pair(Analysis, (const char*)"AnalysisType"));
  gvParameterInfo.push_back(std::make_pair(Analysis, (const char*)"ScanAreas"));
  gvParameterInfo.push_back(std::make_pair(Input, (const char*)"CaseFile"));
  gvParameterInfo.push_back(std::make_pair(Input, (const char*)"PopulationFile"));
  gvParameterInfo.push_back(std::make_pair(Input, (const char*)"CoordinatesFile"));
  gvParameterInfo.push_back(std::make_pair(Output, (const char*)"ResultsFile"));
  gvParameterInfo.push_back(std::make_pair(Input, (const char*)"PrecisionCaseTimes"));
  gvParameterInfo.push_back(std::make_pair(NotUsed, NotUsed));
  gvParameterInfo.push_back(std::make_pair(Input, (const char*)"UseGridFile"));
  gvParameterInfo.push_back(std::make_pair(Input, (const char*)"GridFile"));
  gvParameterInfo.push_back(std::make_pair(SpatialWindow, (const char*)"MaxGeographicSize"));
  gvParameterInfo.push_back(std::make_pair(Input, (const char*)"StartDate"));
  gvParameterInfo.push_back(std::make_pair(Input, (const char*)"EndDate"));
  gvParameterInfo.push_back(std::make_pair(TemporalWindow, (const char*)"IncludeClusters"));
  gvParameterInfo.push_back(std::make_pair(NotUsed, NotUsed));
  gvParameterInfo.push_back(std::make_pair(Analysis, (const char*)"TimeAggregationUnits"));
  gvParameterInfo.push_back(std::make_pair(Analysis, (const char*)"TimeAggregationLength"));
  gvParameterInfo.push_back(std::make_pair(TemporalWindow, (const char*)"IncludePurelySpatial"));
  gvParameterInfo.push_back(std::make_pair(TemporalWindow, (const char*)"MaxTemporalSize"));
  gvParameterInfo.push_back(std::make_pair(Analysis, (const char*)"MonteCarloReps"));
  gvParameterInfo.push_back(std::make_pair(Analysis, (const char*)"ModelType"));
  gvParameterInfo.push_back(std::make_pair(IsotonicScan, (const char*)"IsotonicScan"));
  gvParameterInfo.push_back(std::make_pair(PowerSimulations, (const char*)"PValues2PrespecifiedLLRs"));
  gvParameterInfo.push_back(std::make_pair(PowerSimulations, (const char*)"LLR1"));
  gvParameterInfo.push_back(std::make_pair(PowerSimulations, (const char*)"LLR2"));
  gvParameterInfo.push_back(std::make_pair(SpaceAndTimeAdjustments, (const char*)"TimeTrendAdjustmentType"));
  gvParameterInfo.push_back(std::make_pair(SpaceAndTimeAdjustments, (const char*)"TimeTrendPercentage"));
  gvParameterInfo.push_back(std::make_pair(SpatialWindow, (const char*)"IncludePurelyTemporal"));
  gvParameterInfo.push_back(std::make_pair(Input, (const char*)"ControlFile"));
  gvParameterInfo.push_back(std::make_pair(Input, (const char*)"CoordinatesType"));
  gvParameterInfo.push_back(std::make_pair(Output, (const char*)"SaveSimLLRsASCII"));
  gvParameterInfo.push_back(std::make_pair(SequentialScan, (const char*)"SequentialScan"));
  gvParameterInfo.push_back(std::make_pair(SequentialScan, (const char*)"SequentialScanMaxIterations"));
  gvParameterInfo.push_back(std::make_pair(SequentialScan, (const char*)"SequentialScanMaxPValue"));
  gvParameterInfo.push_back(std::make_pair(BatchModeFeatures, (const char*)"ValidateParameters"));
  gvParameterInfo.push_back(std::make_pair(Output, (const char*)"IncludeRelativeRisksCensusAreasASCII"));
  gvParameterInfo.push_back(std::make_pair(EllipticScan, (const char*)"NumberOfEllipses"));
  gvParameterInfo.push_back(std::make_pair(EllipticScan, (const char*)"EllipseShapes"));
  gvParameterInfo.push_back(std::make_pair(EllipticScan, (const char*)"EllipseAngles"));
  gvParameterInfo.push_back(std::make_pair(Inference, (const char*)"ProspectiveStartDate"));
  gvParameterInfo.push_back(std::make_pair(Output, (const char*)"CensusAreasReportedClustersASCII"));
  gvParameterInfo.push_back(std::make_pair(Output, (const char*)"MostLikelyClusterEachCentroidASCII"));
  gvParameterInfo.push_back(std::make_pair(ClustersReported, (const char*)"CriteriaForReportingSecondaryClusters"));
  gvParameterInfo.push_back(std::make_pair(TemporalWindow, (const char*)"MaxTemporalSizeInterpretation"));
  gvParameterInfo.push_back(std::make_pair(SpatialWindow, (const char*)"MaxSpatialSizeInterpretation"));
  gvParameterInfo.push_back(std::make_pair(NotUsed, NotUsed));
  gvParameterInfo.push_back(std::make_pair(Output, (const char*)"MostLikelyClusterEachCentroidDBase"));
  gvParameterInfo.push_back(std::make_pair(Output, (const char*)"CensusAreasReportedClustersDBase"));
  gvParameterInfo.push_back(std::make_pair(Output, (const char*)"IncludeRelativeRisksCensusAreasDBase"));
  gvParameterInfo.push_back(std::make_pair(Output, (const char*)"SaveSimLLRsDBase"));
  gvParameterInfo.push_back(std::make_pair(EllipticScan, (const char*)"NonCompactnessPenalty"));
  gvParameterInfo.push_back(std::make_pair(SpaceAndTimeAdjustments, (const char*)"IntervalStartRange"));
  gvParameterInfo.push_back(std::make_pair(SpaceAndTimeAdjustments, (const char*)"IntervalEndRange"));
  gvParameterInfo.push_back(std::make_pair(NotUsed, NotUsed));
  gvParameterInfo.push_back(std::make_pair(SpatialWindow, (const char*)"MaxCirclePopulationFile"));
  gvParameterInfo.push_back(std::make_pair(Inference, (const char*)"EarlySimulationTermination"));
  gvParameterInfo.push_back(std::make_pair(ClustersReported, (const char*)"MaxReportedGeoClusterSize"));
  gvParameterInfo.push_back(std::make_pair(ClustersReported, (const char*)"UseReportOnlySmallerClusters"));
  gvParameterInfo.push_back(std::make_pair(PowerSimulations, (const char*)"SimulatedDataMethodType"));
  gvParameterInfo.push_back(std::make_pair(PowerSimulations, (const char*)"SimulatedDataInputFilename"));
  gvParameterInfo.push_back(std::make_pair(SpaceAndTimeAdjustments, (const char*)"AdjustmentsByKnownRelativeRisksFilename"));
  gvParameterInfo.push_back(std::make_pair(PowerSimulations, (const char*)"PrintSimulatedDataToFile"));
  gvParameterInfo.push_back(std::make_pair(PowerSimulations, (const char*)"SimulatedDataOutputFilename"));
  gvParameterInfo.push_back(std::make_pair(Inference, (const char*)"AdjustForEarlierAnalyses"));
  gvParameterInfo.push_back(std::make_pair(SpaceAndTimeAdjustments, (const char*)"UseAdjustmentsByRRFile"));
  gvParameterInfo.push_back(std::make_pair(SpaceAndTimeAdjustments, (const char*)"SpatialAdjustmentType"));
  gvParameterInfo.push_back(std::make_pair(MultipleDataSets, (const char*)"MultipleDataSetsPurposeType"));
  gvParameterInfo.push_back(std::make_pair(System, (const char*)"Version"));
  gvParameterInfo.push_back(std::make_pair(BatchModeFeatures, (const char*)"RandomSeed"));
  gvParameterInfo.push_back(std::make_pair(Inference, (const char*)"CriticalValue"));

  gvMultipleParameterInfo[CASEFILE] = std::make_pair(MultipleDataSets, (const char*)"CaseFile");
  gvMultipleParameterInfo[CONTROLFILE] = std::make_pair(MultipleDataSets, (const char*)"ControlFile");
  gvMultipleParameterInfo[POPFILE] = std::make_pair(MultipleDataSets, (const char*)"PopulationFile");
}

/** Version 6.0.x */
void IniParameterSpecification::Build_6_0_x_ParameterList() {
  Build_5_1_x_ParameterList();

  gvParameterInfo.push_back(std::make_pair(BatchModeFeatures, (const char*)"ExecutionType"));
  gvParameterInfo.push_back(std::make_pair(RunOptions, (const char*)"NumberParallelProcesses"));
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



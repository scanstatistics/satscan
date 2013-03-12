//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "IniParameterSpecification.h"

const char * IniParameterSpecification::NotUsed                 = "NotUsed";

const char * IniParameterSpecification::Input                   = "Input";
const char * IniParameterSpecification::MultipleDataSets        = "Multiple Data Sets";
const char * IniParameterSpecification::DataChecking            = "Data Checking";
const char * IniParameterSpecification::NeighborsFile           = "Non-Eucledian Neighbors";
const char * IniParameterSpecification::SpatialNeighbors        = "Spatial Neighbors";
const char * IniParameterSpecification::Analysis                = "Analysis";
const char * IniParameterSpecification::SpatialWindow           = "Spatial Window";
const char * IniParameterSpecification::TemporalWindow          = "Temporal Window";
const char * IniParameterSpecification::Polygons                = "Polygons";
const char * IniParameterSpecification::SpaceAndTimeAdjustments = "Space and Time Adjustments";
const char * IniParameterSpecification::Inference               = "Inference";
const char * IniParameterSpecification::Output                  = "Output";
const char * IniParameterSpecification::ClustersReported        = "Clusters Reported";
const char * IniParameterSpecification::AdditionalOutput        = "Additional Output";

const char * IniParameterSpecification::EllipticScan            = "Elliptic Scan";
const char * IniParameterSpecification::SequentialScan          = "Sequential Scan";
const char * IniParameterSpecification::IsotonicScan            = "Isotonic Scan";
const char * IniParameterSpecification::PowerSimulations        = "Power Simulations";
const char * IniParameterSpecification::PowerEvaluations        = "Power Evaluations";
const char * IniParameterSpecification::RunOptions              = "Run Options";
const char * IniParameterSpecification::BatchModeFeatures       = "BatchMode Features";
const char * IniParameterSpecification::System                  = "System";

const char * IniParameterSpecification::InputFiles              = "Input Files";
const char * IniParameterSpecification::ScanningWindow          = "Scanning Window";
const char * IniParameterSpecification::TimeParameters          = "Time Parameters";
const char * IniParameterSpecification::OutputFiles             = "Output Files";
const char * IniParameterSpecification::AdvancedFeatures        = "Advanced Features";

/** constructor -- builds specification for write process */
IniParameterSpecification::IniParameterSpecification() {
  Build_9_2_x_ParameterList();
}

/** constructor -- builds specification for read process */
IniParameterSpecification::IniParameterSpecification(const IniFile& SourceFile, CParameters& Parameters) {
  long                          lSectionIndex, lKeyIndex;
  CParameters::CreationVersion  Version = {3, 1, 2};
  bool                          bHasVersionKey=false;

  if ((lSectionIndex = SourceFile.GetSectionIndex(System)) > -1) {
    const IniSection * pSection = SourceFile.GetSection(lSectionIndex);
    if ((lKeyIndex = pSection->FindKey("Version")) > -1) {
      sscanf(pSection->GetLine(lKeyIndex)->GetValue(), "%u.%u.%u", &Version.iMajor, &Version.iMinor, &Version.iRelease);
      bHasVersionKey = true;
    }
  }

  if (!bHasVersionKey) {//version prior to 5.0 didn't have the system section
    //Attempt to determine which version, 3.1 was first version with ini structure:
    //  with 3.1.0 - 3.1.2 having the same number of parameters
    //  with 4.0.0 - 4.0.3 having the same number of parameters
    //So this parameter file will either be marked as 3.1.2 or 4.0.3 and since the
    //early termination of simulations feature was added in version 4.0.0; we'll
    //use this knowledge to help determine which.
    if ((lSectionIndex = SourceFile.GetSectionIndex(AdvancedFeatures)) > -1) {
      const IniSection * pSection = SourceFile.GetSection(lSectionIndex);
      if ((lKeyIndex = pSection->FindKey("EarlySimulationTermination")) > -1)
        {Version.iMajor = 4; Version.iMinor = 0; Version.iRelease = 3;}
    }
  }
  //Set creation version now, we'll need to know this for parameters which read differently
  //based upon version; such as WINDOW_SHAPE and NON_COMPACTNESS_PENALTY.
  Parameters.SetVersion(Version);

  if (Version.iMajor <= 4)
    Build_4_0_x_ParameterList();
  else if (Version.iMajor == 5  && Version.iMinor == 0)
    Build_5_0_x_ParameterList();
  else if (Version.iMajor == 5  && Version.iMinor == 1)
    Build_5_1_x_ParameterList();
  else if (Version.iMajor == 6  && Version.iMinor == 0)
    Build_6_0_x_ParameterList();
  else if (Version.iMajor == 6  && Version.iMinor == 1)
    Build_6_1_x_ParameterList();
  else if (Version.iMajor == 7 && Version.iMinor == 0)
    Build_7_0_x_ParameterList();
  else if (Version.iMajor == 8 && (Version.iMinor == 0 || Version.iMinor == 1))
    Build_8_0_x_ParameterList();
  else if (Version.iMajor == 8  && Version.iMinor == 2)
    Build_8_2_x_ParameterList();
  else if (Version.iMajor == 9  && Version.iMinor == 0)
    Build_9_0_x_ParameterList();
  else
    Build_9_2_x_ParameterList();
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
  // Order in vector is essential - should be identical to ParameterType enumeration.
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
  gvParameterInfo.push_back(std::make_pair(PowerEvaluations, (const char*)"PValues2PrespecifiedLLRs"));
  gvParameterInfo.push_back(std::make_pair(PowerEvaluations, (const char*)"LLR1"));
  gvParameterInfo.push_back(std::make_pair(PowerEvaluations, (const char*)"LLR2"));
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

  //Flexible start and end range were in wrong section
  gvParameterInfo[INTERVAL_STARTRANGE - 1] = std::make_pair(TemporalWindow, (const char*)"IntervalStartRange");
  gvParameterInfo[INTERVAL_ENDRANGE - 1] = std::make_pair(TemporalWindow, (const char*)"IntervalEndRange");

  //VALIDATE and RANDOMIZATION_SEED parameters moved from 'BatchModeFeatures' to 'RunOptions'
  gvParameterInfo[VALIDATE - 1] = std::make_pair(RunOptions, (const char*)"ValidateParameters");
  gvParameterInfo[RANDOMIZATION_SEED - 1] = std::make_pair(RunOptions, (const char*)"RandomSeed");
  gvParameterInfo.push_back(std::make_pair(RunOptions, (const char*)"ExecutionType"));
  gvParameterInfo.push_back(std::make_pair(RunOptions, (const char*)"NumberParallelProcesses"));
  gvParameterInfo.push_back(std::make_pair(RunOptions, (const char*)"LogRunToHistoryFile"));
}

/** Version 6.1.x */
void IniParameterSpecification::Build_6_1_x_ParameterList() {
  Build_6_0_x_ParameterList();

  //Elliptic parameter replaced by spatial window type 
  gvParameterInfo[WINDOW_SHAPE - 1] = std::make_pair(SpatialWindow, (const char*)"SpatialWindowShapeType");
  gvParameterInfo[NON_COMPACTNESS_PENALTY - 1] = std::make_pair(SpatialWindow, (const char*)"NonCompactnessPenalty");
  gvParameterInfo.push_back(std::make_pair(RunOptions, (const char*)"SuppressWarnings"));
  gvParameterInfo.push_back(std::make_pair(ClustersReported, (const char*)"MaxReportedSpatialSizeInterpretation"));
  gvParameterInfo.push_back(std::make_pair(Output, (const char*)"MostLikelyClusterCaseInfoEachCentroidASCII"));
  gvParameterInfo.push_back(std::make_pair(Output, (const char*)"MostLikelyClusterCaseInfoEachCentroidDBase"));
}

/** Version 7.0.x */
void IniParameterSpecification::Build_7_0_x_ParameterList() {
  Build_6_1_x_ParameterList();

  //Sequential Scan parameters moved to Inference section - removed SequentialScan section
  gvParameterInfo[ITERATIVE - 1] = std::make_pair(Inference, (const char*)"IterativeScan");
  gvParameterInfo[ITERATIVE_NUM - 1] = std::make_pair(Inference, (const char*)"IterativeScanMaxIterations");
  gvParameterInfo[ITERATIVE_PVAL - 1] = std::make_pair(Inference, (const char*)"IterativeScanMaxPValue");
  gvParameterInfo.push_back(std::make_pair(DataChecking, (const char*)"StudyPeriodCheckType"));
  gvParameterInfo.push_back(std::make_pair(DataChecking, (const char*)"GeographicalCoordinatesCheckType"));
  //Maximum spatial cluster size updated to use new parameters
  gvParameterInfo[GEOSIZE - 1] = std::make_pair(NotUsed, NotUsed);
  gvParameterInfo[MAX_SPATIAL_TYPE - 1] = std::make_pair(NotUsed, NotUsed);
  gvParameterInfo[REPORTED_GEOSIZE - 1] = std::make_pair(NotUsed, NotUsed);
  gvParameterInfo[MAX_REPORTED_SPATIAL_TYPE - 1] = std::make_pair(NotUsed, NotUsed);
  gvParameterInfo.push_back(std::make_pair(SpatialWindow, (const char*)"MaxSpatialSizeInPopulationAtRisk"));
  gvParameterInfo.push_back(std::make_pair(SpatialWindow, (const char*)"MaxSpatialSizeInMaxCirclePopulationFile"));
  gvParameterInfo.push_back(std::make_pair(SpatialWindow, (const char*)"MaxSpatialSizeInDistanceFromCenter"));
  gvParameterInfo.push_back(std::make_pair(SpatialWindow, (const char*)"UseMaxCirclePopulationFileOption"));
  gvParameterInfo.push_back(std::make_pair(SpatialWindow, (const char*)"UseDistanceFromCenterOption"));
  gvParameterInfo.push_back(std::make_pair(ClustersReported, (const char*)"MaxSpatialSizeInPopulationAtRisk_Reported"));
  gvParameterInfo.push_back(std::make_pair(ClustersReported, (const char*)"MaxSizeInMaxCirclePopulationFile_Reported"));
  gvParameterInfo.push_back(std::make_pair(ClustersReported, (const char*)"MaxSpatialSizeInDistanceFromCenter_Reported"));
  gvParameterInfo.push_back(std::make_pair(ClustersReported, (const char*)"UseMaxCirclePopulationFileOption_Reported"));
  gvParameterInfo.push_back(std::make_pair(ClustersReported, (const char*)"UseDistanceFromCenterOption_Reported"));
  //validate parameters no longer used
  gvParameterInfo[VALIDATE - 1] = std::make_pair(NotUsed, NotUsed);
  gvParameterInfo.push_back(std::make_pair(NeighborsFile, (const char*)"NeighborsFilename"));
  gvParameterInfo.push_back(std::make_pair(NeighborsFile, (const char*)"UseNeighborsFile"));
}

/** Version 8.0.x */
void IniParameterSpecification::Build_8_0_x_ParameterList() {
  Build_7_0_x_ParameterList();
  gvParameterInfo.push_back(std::make_pair(RunOptions, (const char*)"RandomlyGenerateSeed"));
  gvParameterInfo.push_back(std::make_pair(SpatialNeighbors, (const char*)"MultipleCoordinatesType"));
  gvParameterInfo.push_back(std::make_pair(SpatialNeighbors, (const char*)"MetaLocationsFilename"));
  gvParameterInfo.push_back(std::make_pair(SpatialNeighbors, (const char*)"UseMetaLocationsFile"));

  gvParameterInfo.push_back(std::make_pair(Polygons, (const char*)"Polygons"));
  gvMultipleParameterInfo[OBSERVABLE_REGIONS] = std::make_pair(Polygons, (const char*)"Polygon");

  //risk/isotonic scan moved to spatial window tab
  gvParameterInfo[RISKFUNCTION - 1] = std::make_pair(SpatialWindow, (const char*)"IsotonicScan");
  //non-Euclidian neighbors moved to new spatial neighbors tab
  gvParameterInfo[LOCATION_NEIGHBORS_FILE - 1] = std::make_pair(SpatialNeighbors, (const char*)"NeighborsFilename");
  gvParameterInfo[USE_LOCATION_NEIGHBORS_FILE - 1] = std::make_pair(SpatialNeighbors, (const char*)"UseNeighborsFile");
  //critical values parameter moved to new to additional output tab
  gvParameterInfo[REPORT_CRITICAL_VALUES - 1] = std::make_pair(AdditionalOutput, (const char*)"CriticalValue");
}

/** Version 8.2.x */
void IniParameterSpecification::Build_8_2_x_ParameterList() {
  Build_8_0_x_ParameterList();

  // number of replications moved to inference tab
  gvParameterInfo[REPLICAS - 1] = std::make_pair(Inference, (const char*)"MonteCarloReps");
}

/** Version 9.0.x */
void IniParameterSpecification::Build_9_0_x_ParameterList() {
  Build_8_2_x_ParameterList();

  gvParameterInfo.push_back(std::make_pair(Inference, (const char*)"EarlyTerminationThreshold"));
  gvParameterInfo.push_back(std::make_pair(Inference, (const char*)"PValueReportType"));
  gvParameterInfo.push_back(std::make_pair(Inference, (const char*)"ReportGumbel"));

  gvParameterInfo.push_back(std::make_pair(SpaceAndTimeAdjustments, (const char*)"TimeTrendType"));
  gvParameterInfo.push_back(std::make_pair(AdditionalOutput, (const char*)"ReportClusterRank"));  
  gvParameterInfo.push_back(std::make_pair(AdditionalOutput, (const char*)"PrintAsciiColumnHeaders"));  
}

/** Version 9.2.x */
void IniParameterSpecification::Build_9_2_x_ParameterList() {
  Build_9_0_x_ParameterList();

  gvParameterInfo.push_back(std::make_pair(ClustersReported, (const char*)"ReportHierarchicalClusters"));
  gvParameterInfo.push_back(std::make_pair(ClustersReported, (const char*)"ReportGiniClusters"));
  gvParameterInfo.push_back(std::make_pair(ClustersReported, (const char*)"SpatialMaxima"));
  gvParameterInfo.push_back(std::make_pair(ClustersReported, (const char*)"GiniIndexClusterReportingType"));
  gvParameterInfo.push_back(std::make_pair(ClustersReported, (const char*)"GiniIndexClustersPValueCutOff"));
  gvParameterInfo.push_back(std::make_pair(ClustersReported, (const char*)"ReportGiniIndexCoefficents"));

  gvParameterInfo.push_back(std::make_pair(PowerEvaluations, (const char*)"PowerEvaluationTotalCases"));
  gvParameterInfo.push_back(std::make_pair(PowerEvaluations, (const char*)"CriticalValueType"));
  gvParameterInfo.push_back(std::make_pair(PowerEvaluations, (const char*)"PowerEstimationType"));
  gvParameterInfo.push_back(std::make_pair(PowerEvaluations, (const char*)"AlternativeHypothesisFilename"));
  gvParameterInfo.push_back(std::make_pair(PowerEvaluations, (const char*)"NumberPowerReplications"));

  gvParameterInfo.push_back(std::make_pair(PowerEvaluations, (const char*)"PowerEvaluationsSimulationMethod"));
  gvParameterInfo.push_back(std::make_pair(PowerEvaluations, (const char*)"PowerEvaluationsSimulationSourceFilename"));
  gvParameterInfo.push_back(std::make_pair(PowerEvaluations, (const char*)"PowerEvaluationsMethod"));
  gvParameterInfo.push_back(std::make_pair(PowerEvaluations, (const char*)"CriticalValue001"));
  gvParameterInfo.push_back(std::make_pair(PowerEvaluations, (const char*)"ReportPowerEvaluationSimulationData"));
  gvParameterInfo.push_back(std::make_pair(PowerEvaluations, (const char*)"PowerEvaluationsSimulationOutputFilename"));

  gvParameterInfo.push_back(std::make_pair(Output, (const char*)"OutputGoogleEarthKML"));
  gvParameterInfo.push_back(std::make_pair(Output, (const char*)"OutputTemporalGraphHTML"));

  gvParameterInfo[POWER_EVALUATION - 1] = std::make_pair(PowerEvaluations, (const char*)"PerformPowerEvaluation");
  gvParameterInfo[POWER_05 - 1] = std::make_pair(PowerEvaluations, (const char*)"CriticalValue05");
  gvParameterInfo[POWER_01 - 1] = std::make_pair(PowerEvaluations, (const char*)"CriticalValue01");
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



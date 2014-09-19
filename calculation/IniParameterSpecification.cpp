//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "IniParameterSpecification.h"
#include "SSException.h"

const char * IniParameterSpecification::NotUsed                   = "NotUsed";

const char * IniParameterSpecification::Input                     = "Input";
const char * IniParameterSpecification::MultipleDataSets          = "Multiple Data Sets";
const char * IniParameterSpecification::DataChecking              = "Data Checking";
const char * IniParameterSpecification::NeighborsFile             = "Non-Eucledian Neighbors";
const char * IniParameterSpecification::SpatialNeighbors          = "Spatial Neighbors";
const char * IniParameterSpecification::Analysis                  = "Analysis";
const char * IniParameterSpecification::SpatialWindow             = "Spatial Window";
const char * IniParameterSpecification::TemporalWindow            = "Temporal Window";
const char * IniParameterSpecification::Polygons                  = "Polygons";
const char * IniParameterSpecification::SpaceAndTimeAdjustments   = "Space and Time Adjustments";
const char * IniParameterSpecification::Inference                 = "Inference";
const char * IniParameterSpecification::BorderAnalysis            = "Border Analysis";
const char * IniParameterSpecification::Output                    = "Output";
const char * IniParameterSpecification::ClustersReported          = "Clusters Reported";
const char * IniParameterSpecification::AdditionalOutput          = "Additional Output";
const char * IniParameterSpecification::TemporalOutput            = "Temporal Output";
const char * IniParameterSpecification::SpatialOutput             = "Spatial Output";

const char * IniParameterSpecification::EllipticScan              = "Elliptic Scan";
const char * IniParameterSpecification::SequentialScan            = "Sequential Scan";
const char * IniParameterSpecification::IsotonicScan              = "Isotonic Scan";
const char * IniParameterSpecification::PowerSimulations          = "Power Simulations";
const char * IniParameterSpecification::PowerEvaluations          = "Power Evaluation";
const char * IniParameterSpecification::RunOptions                = "Run Options";
const char * IniParameterSpecification::BatchModeFeatures         = "BatchMode Features";
const char * IniParameterSpecification::System                    = "System";

const char * IniParameterSpecification::InputFiles                = "Input Files";
const char * IniParameterSpecification::ScanningWindow            = "Scanning Window";
const char * IniParameterSpecification::TimeParameters            = "Time Parameters";
const char * IniParameterSpecification::OutputFiles               = "Output Files";
const char * IniParameterSpecification::AdvancedFeatures          = "Advanced Features";
const char * IniParameterSpecification::OtherOutput               = "Other Output";

const char * IniParameterSpecification::SourceType                = "SourceType";
const char * IniParameterSpecification::SourceDelimiter           = "SourceDelimiter";
const char * IniParameterSpecification::SourceGrouper             = "SourceGrouper";
const char * IniParameterSpecification::SourceSkip                = "SourceSkip";
const char * IniParameterSpecification::SourceFieldMap            = "SourceFieldMap";
const char * IniParameterSpecification::SourceFirstRowHeader      = "SourceFirstRowHeader";
const char * IniParameterSpecification::SourceFieldMapShapeX      = "shapeX";
const char * IniParameterSpecification::SourceFieldMapShapeY      = "shapeY";
const char * IniParameterSpecification::SourceFieldMapOneCount    = "oneCount";
const char * IniParameterSpecification::SourceFieldMapGeneratedId = "generatedId";
const char * IniParameterSpecification::SourceFieldMapUnspecifiedPopulationDate = "unspecifiedPopDate";

/** constructor - builds specification to current version */
IniParameterSpecification::IniParameterSpecification() {
    // default to current version
    CParameters::CreationVersion  version = {std::atoi(VERSION_MAJOR), std::atoi(VERSION_MINOR), std::atoi(VERSION_RELEASE)};
    setup(version);
}

/** constructor - builds specification to version of source ini file */
IniParameterSpecification::IniParameterSpecification(const IniFile& SourceFile, CParameters& Parameters) {
    CParameters::CreationVersion version = getIniVersion(SourceFile);
    // Set parameter classes creation version now, we'll need to know this for parameters which read
    // differently based upon version; such as WINDOW_SHAPE and NON_COMPACTNESS_PENALTY.
    Parameters.SetVersion(version);
    setup(version);
}

/** constructor - builds specification to version specified in argument */
IniParameterSpecification::IniParameterSpecification(CParameters::CreationVersion version, CParameters& Parameters) {
    // Set parameter classes creation version now, we'll need to know this for parameters which read
    // differently based upon version; such as WINDOW_SHAPE and NON_COMPACTNESS_PENALTY.
    Parameters.SetVersion(version);
    setup(version);
}

/** constructor - builds specification to version specified in argument
     - argument version must agree with version of ini file. */
IniParameterSpecification::IniParameterSpecification(const IniFile& SourceFile, CParameters::CreationVersion version, CParameters& Parameters) {
    // first get the Version setting from the source Ini file
    CParameters::CreationVersion ini_version = getIniVersion(SourceFile);
    // confirm that ini file and specified version are the same
    if (version != ini_version) {
        throw resolvable_error("Parameter file version (%u.%u.%u) does not match override version (%u.%u.%u).",
                               ini_version.iMajor, ini_version.iMinor, ini_version.iRelease,
                               version.iMajor, version.iMinor, version.iRelease);
    }
    setup(version);
}

/* Returns ini version setting or default. */
CParameters::CreationVersion IniParameterSpecification::getIniVersion(const IniFile& SourceFile) {
    long                          lSectionIndex, lKeyIndex;
    CParameters::CreationVersion  version = {std::atoi(VERSION_MAJOR), std::atoi(VERSION_MINOR), std::atoi(VERSION_RELEASE)};
    bool                          bHasVersionKey=false;

    // search ini for version setting
    if ((lSectionIndex = SourceFile.GetSectionIndex(System)) > -1) {
        const IniSection * pSection = SourceFile.GetSection(lSectionIndex);
        if ((lKeyIndex = pSection->FindKey("Version")) > -1) {
            sscanf(pSection->GetLine(lKeyIndex)->GetValue(), "%u.%u.%u", &version.iMajor, &version.iMinor, &version.iRelease);
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
                {version.iMajor = 4; version.iMinor = 0; version.iRelease = 3;}
            else
                {version.iMajor = 3; version.iMinor = 1; version.iRelease = 2;}
        }
    }
    return version;
}

/** define ini sections and section parameters base on passed version */
void IniParameterSpecification::setup(CParameters::CreationVersion version) {
    // define sections in which parameters belong
    _not_used_section = SectionInfo(NotUsed, 0);
    _input_files_section = SectionInfo(InputFiles, 100);
    _input_section = SectionInfo(Input, 110);
    _analysis_section = SectionInfo(Analysis, 200);
    _time_parameters_section  = SectionInfo(TimeParameters, 300);
    _scanning_window_section  = SectionInfo(ScanningWindow, 400);
    _output_files_section = SectionInfo(OutputFiles, 500);
    _output_section = SectionInfo(Output, 510);
    _polygons_section = SectionInfo(Polygons, 515);
    _multiple_data_section = SectionInfo(MultipleDataSets, 520);
    _data_checking_section = SectionInfo(DataChecking, 522);
    _non_eucledian_section = SectionInfo(NeighborsFile, 524);
    _spatial_neighbors_section = SectionInfo(SpatialNeighbors, 526);
    _spatial_window_section = SectionInfo(SpatialWindow, 530);
    _temporal_window_section = SectionInfo(TemporalWindow, 540);
    _space_time_adjustments_section = SectionInfo(SpaceAndTimeAdjustments, 542);
    _other_output_section = SectionInfo(OtherOutput, 545);
    _inference_section = SectionInfo(Inference, 560);
    _border_analysis_section = SectionInfo(BorderAnalysis, 561);
    _power_evaluation_section = SectionInfo(PowerEvaluations, 564);
    _clusters_reported_section = SectionInfo(ClustersReported, 570);
    _temporal_output_section = SectionInfo(TemporalOutput, 572);
    _spatial_output_section = SectionInfo(SpatialOutput, 574);
    _additional_output_section = SectionInfo(AdditionalOutput, 580);
    _elliptic_section = SectionInfo(EllipticScan, 600);
    _isotonic_scan_section = SectionInfo(IsotonicScan, 610);
    _sequential_section = SectionInfo(SequentialScan, 700);
    _power_simulations_section = SectionInfo(PowerSimulations, 710);
    _batch_features_section = SectionInfo(BatchModeFeatures, 720);
    _run_options_section = SectionInfo(RunOptions, 720);
    _advanced_section = SectionInfo(AdvancedFeatures, 800);
    _system_section = SectionInfo(System, 2000);

    if (version.iMajor <= 3)
        Build_3_0_5_ParameterList();
    else if (version.iMajor <= 4)
        Build_4_0_x_ParameterList();
    else if (version.iMajor == 5  && version.iMinor == 0)
        Build_5_0_x_ParameterList();
    else if (version.iMajor == 5  && version.iMinor == 1)
        Build_5_1_x_ParameterList();
    else if (version.iMajor == 6  && version.iMinor == 0)
        Build_6_0_x_ParameterList();
    else if (version.iMajor == 6  && version.iMinor == 1)
        Build_6_1_x_ParameterList();
    else if (version.iMajor == 7 && version.iMinor == 0)
        Build_7_0_x_ParameterList();
    else if (version.iMajor == 8 && (version.iMinor == 0 || version.iMinor == 1))
        Build_8_0_x_ParameterList();
    else if (version.iMajor == 8  && version.iMinor == 2)
        Build_8_2_x_ParameterList();
    else if (version.iMajor == 9  && version.iMinor == 0)
        Build_9_0_x_ParameterList();
    else if (version.iMajor == 9  && version.iMinor == 2 && version.iRelease < 1)
        Build_9_2_x_ParameterList();
    else if (version.iMajor == 9  && version.iMinor == 3)
        Build_9_3_x_ParameterList();
    else
        Build_9_4_x_ParameterList();
}

/** Version 3.0.5 and prior parameter section/keys. */
void IniParameterSpecification::Build_3_0_5_ParameterList() {
    _parameter_info[CASEFILE] = ParamInfo(CASEFILE, "CaseFile", 1, _input_files_section);
    _parameter_info[CONTROLFILE] = ParamInfo(CONTROLFILE, "ControlFile", 2, _input_files_section);
    _parameter_info[PRECISION] = ParamInfo(PRECISION, "PrecisionCaseTimes", 3, _input_files_section);
    _parameter_info[POPFILE] = ParamInfo(POPFILE, "PopulationFile", 4, _input_files_section);
    _parameter_info[COORDFILE] = ParamInfo(COORDFILE, "CoordinatesFile", 5, _input_files_section);
    _parameter_info[COORDTYPE] = ParamInfo(COORDTYPE, "CoordinatesType",6, _input_files_section);
    _parameter_info[SPECIALGRID] = ParamInfo(SPECIALGRID, "UseGridFile", 7, _input_files_section);
    _parameter_info[GRIDFILE] = ParamInfo(GRIDFILE, "GridFile", 8, _input_files_section);

    _parameter_info[ANALYSISTYPE] = ParamInfo(ANALYSISTYPE, "AnalysisType", 1, _analysis_section);
    _parameter_info[MODEL] = ParamInfo(MODEL, "ModelType", 2, _analysis_section);
    _parameter_info[SCANAREAS] = ParamInfo(SCANAREAS, "ScanAreas", 3, _analysis_section);
    _parameter_info[REPLICAS] = ParamInfo(REPLICAS, "MonteCarloReps", 5, _analysis_section);

    _parameter_info[TIME_AGGREGATION_UNITS] = ParamInfo(TIME_AGGREGATION_UNITS, "IntervalUnits", 1, _time_parameters_section);
    _parameter_info[TIME_AGGREGATION] = ParamInfo(TIME_AGGREGATION, "IntervalLength", 2, _time_parameters_section);
    _parameter_info[TIMETREND] = ParamInfo(TIMETREND, "TimeTrendAdjustmentType", 3, _time_parameters_section);
    _parameter_info[TIMETRENDPERC] = ParamInfo(TIMETRENDPERC, "TimeTrendPercentage", 4, _time_parameters_section);
    _parameter_info[START_PROSP_SURV] = ParamInfo(START_PROSP_SURV, "ProspectiveStartDate", 5, _time_parameters_section);
    _parameter_info[STARTDATE] = ParamInfo(STARTDATE, "StartDate", 6, _time_parameters_section);
    _parameter_info[ENDDATE] = ParamInfo(ENDDATE, "EndDate", 7, _time_parameters_section);

    _parameter_info[GEOSIZE] = ParamInfo(GEOSIZE, "MaxGeographicSize", 1, _scanning_window_section);
    _parameter_info[MAX_SPATIAL_TYPE] = ParamInfo(MAX_SPATIAL_TYPE, "MaxSpatialSizeInterpretation", 2, _scanning_window_section);
    _parameter_info[PURETEMPORAL] = ParamInfo(PURETEMPORAL, "IncludePurelyTemporal", 3, _scanning_window_section);
    _parameter_info[TIMESIZE] = ParamInfo(TIMESIZE, "MaxTemporalSize", 4, _scanning_window_section);
    _parameter_info[MAX_TEMPORAL_TYPE] = ParamInfo(MAX_TEMPORAL_TYPE, "MaxTemporalSizeInterpretation", 5, _scanning_window_section);
    _parameter_info[PURESPATIAL] = ParamInfo(PURESPATIAL, "IncludePurelySpatial", 6, _scanning_window_section);
    _parameter_info[CLUSTERS] = ParamInfo(CLUSTERS, "IncludeClusters", 7, _scanning_window_section);

    _parameter_info[OUTPUTFILE] = ParamInfo(OUTPUTFILE, "ResultsFile", 1, _output_files_section);
    _parameter_info[OUTPUT_SIM_LLR_ASCII] = ParamInfo(OUTPUT_SIM_LLR_ASCII, "SaveSimLLRsASCII", 2, _output_files_section);
    _parameter_info[OUTPUT_RR_ASCII] = ParamInfo(OUTPUT_RR_ASCII, "IncludeRelativeRisksCensusAreasASCII", 3, _output_files_section);
    _parameter_info[OUTPUT_AREAS_ASCII] = ParamInfo(OUTPUT_AREAS_ASCII, "CensusAreasReportedClustersASCII", 4, _output_files_section);
    _parameter_info[OUTPUT_MLC_ASCII] = ParamInfo(OUTPUT_MLC_ASCII, "MostLikelyClusterEachCentroidASCII", 5, _output_files_section);
    _parameter_info[OUTPUT_MLC_DBASE] = ParamInfo(OUTPUT_MLC_DBASE, "MostLikelyClusterEachCentroidDBase", 6, _output_files_section);
    _parameter_info[OUTPUT_AREAS_DBASE] = ParamInfo(OUTPUT_AREAS_DBASE, "CensusAreasReportedClustersDBase", 7, _output_files_section);
    _parameter_info[OUTPUT_RR_DBASE] = ParamInfo(OUTPUT_RR_DBASE, "IncludeRelativeRisksCensusAreasDBase", 8, _output_files_section);
    _parameter_info[OUTPUT_SIM_LLR_DBASE] = ParamInfo(OUTPUT_SIM_LLR_DBASE, "SaveSimLLRsDBase", 9, _output_files_section);
    _parameter_info[CRITERIA_SECOND_CLUSTERS] = ParamInfo(CRITERIA_SECOND_CLUSTERS, "CriteriaForReportingSecondaryClusters", 10, _output_files_section);

    _parameter_info[WINDOW_SHAPE] = ParamInfo(WINDOW_SHAPE, "NumberOfEllipses", 1, _elliptic_section);
    _parameter_info[ESHAPES] = ParamInfo(ESHAPES, "EllipseShapes", 2, _elliptic_section);
    _parameter_info[ENUMBERS] = ParamInfo(ENUMBERS, "EllipseAngles", 3, _elliptic_section);
    _parameter_info[NON_COMPACTNESS_PENALTY] = ParamInfo(NON_COMPACTNESS_PENALTY, "DuczmalCompactnessCorrection", 4, _elliptic_section);

    _parameter_info[ITERATIVE] = ParamInfo(ITERATIVE, "SequentialScan", 1, _sequential_section);
    _parameter_info[ITERATIVE_NUM] = ParamInfo(ITERATIVE_NUM, "SequentialScanMaxIterations", 2, _sequential_section);
    _parameter_info[ITERATIVE_PVAL] = ParamInfo(ITERATIVE_PVAL, "SequentialScanMaxPValue", 3, _sequential_section);

    _parameter_info[VALIDATE] = ParamInfo(VALIDATE, "ValidateParameters", 1, _advanced_section);
    _parameter_info[RISKFUNCTION] = ParamInfo(RISKFUNCTION, "IsotonicScan",2, _advanced_section);
    _parameter_info[POWER_EVALUATION] = ParamInfo(POWER_EVALUATION, "PValues2PrespecifiedLLRs", 3, _advanced_section);
    _parameter_info[POWER_05] = ParamInfo(POWER_05, "LLR1", 4, _advanced_section);
    _parameter_info[POWER_01] = ParamInfo(POWER_01, "LLR2", 5, _advanced_section);

    _parameter_info[DIMENSION] = ParamInfo(DIMENSION, NotUsed, 0, _not_used_section);
    _parameter_info[EXACTTIMES] = ParamInfo(EXACTTIMES, NotUsed, 0, _not_used_section);
    _parameter_info[RUN_HISTORY_FILENAME] = ParamInfo(RUN_HISTORY_FILENAME, NotUsed, 0, _not_used_section);

    assert(_parameter_info.size() == 51);
}

/** Version 4.0.x parameter section/keys.
    Versions 4.0.x made no name changes, only defined more parameters. */
void IniParameterSpecification::Build_4_0_x_ParameterList() {
    Build_3_0_5_ParameterList();

    _parameter_info[ADJ_FOR_EALIER_ANALYSES] = ParamInfo(ADJ_FOR_EALIER_ANALYSES, "AdjustForEarlierAnalyses", 6, _time_parameters_section);

    _parameter_info[INTERVAL_STARTRANGE] = ParamInfo(INTERVAL_STARTRANGE, "IntervalStartRange", 8, _scanning_window_section);
    _parameter_info[INTERVAL_ENDRANGE] = ParamInfo(INTERVAL_ENDRANGE, "IntervalEndRange", 9, _scanning_window_section);

    _parameter_info[USE_REPORTED_GEOSIZE] = ParamInfo(USE_REPORTED_GEOSIZE, "UseReportOnlySmallerClusters", 11, _output_files_section);
    _parameter_info[REPORTED_GEOSIZE] = ParamInfo(REPORTED_GEOSIZE, "MaxReportedGeoClusterSize", 12, _output_files_section);

    _parameter_info[MAXCIRCLEPOPFILE] = ParamInfo(MAXCIRCLEPOPFILE, "MaxCirclePopulationFile", 6, _advanced_section);
    _parameter_info[EARLY_SIM_TERMINATION] = ParamInfo(EARLY_SIM_TERMINATION, "EarlySimulationTermination", 7, _advanced_section);
    _parameter_info[SIMULATION_TYPE] = ParamInfo(SIMULATION_TYPE, "SimulatedDataMethodType", 8, _advanced_section);
    _parameter_info[SIMULATION_SOURCEFILE] = ParamInfo(SIMULATION_SOURCEFILE, "SimulatedDataInputFilename", 9, _advanced_section);
    _parameter_info[ADJ_BY_RR_FILE] = ParamInfo(ADJ_BY_RR_FILE, "AdjustmentsByKnownRelativeRisksFilename", 10, _advanced_section);
    _parameter_info[OUTPUT_SIMULATION_DATA] = ParamInfo(OUTPUT_SIMULATION_DATA, "PrintSimulatedDataToFile", 11, _advanced_section);
    _parameter_info[SIMULATION_DATA_OUTFILE] = ParamInfo(SIMULATION_DATA_OUTFILE, "SimulatedDataOutputFilename", 12, _advanced_section);
    _parameter_info[USE_ADJ_BY_RR_FILE] = ParamInfo(USE_ADJ_BY_RR_FILE, "UseAdjustmentsByRRFile", 13, _advanced_section);

    _parameter_info[TIMETRENDCONVRG] = ParamInfo(TIMETRENDCONVRG, NotUsed, 0, _not_used_section);

    assert(_parameter_info.size() == 65);
}

/** Version 5.0 parameter section/keys.
    Version 5.0.0 updated these three ini key names and defined several more parameters. */
void IniParameterSpecification::Build_5_0_x_ParameterList() {
    Build_4_0_x_ParameterList();

    _parameter_info[TIME_AGGREGATION_UNITS]._label = "TimeAggregationUnits";
    _parameter_info[TIME_AGGREGATION]._label = "TimeAggregationLength";
    _parameter_info[NON_COMPACTNESS_PENALTY]._label = "NonCompactnessPenalty";

    _parameter_info[SPATIAL_ADJ_TYPE] = ParamInfo(SPATIAL_ADJ_TYPE, "SpatialAdjustmentType", 14, _advanced_section);
    _parameter_info[MULTI_DATASET_PURPOSE_TYPE] = ParamInfo(MULTI_DATASET_PURPOSE_TYPE, "MultipleDataSetsPurposeType", 15, _advanced_section);

    _parameter_info[CREATION_VERSION] = ParamInfo(CREATION_VERSION, "Version", 1, _system_section);

    _multiple_parameter_info[CASEFILE] = ParamInfo(CASEFILE, "CaseFile", 1, _input_files_section);
    _multiple_parameter_info[CONTROLFILE] = ParamInfo(CONTROLFILE, "ControlFile", 2, _input_files_section);
    _multiple_parameter_info[POPFILE] = ParamInfo(POPFILE, "PopulationFile", 3, _input_files_section);

    assert(_parameter_info.size() == 68);
}

/** Version 5.1.x made a major revision to the structure of the ini file. This was partly due
   to a specifications miscommunication with Martin when parameter file was converted to
   ini format. Since the desire is to allows mimic the graphical interface within the
   paramter file, we need to move sections around accordingly. */
void IniParameterSpecification::Build_5_1_x_ParameterList() {
    _parameter_info[CASEFILE] = ParamInfo(CASEFILE, "CaseFile", 1, _input_section);
    _parameter_info[CONTROLFILE] = ParamInfo(CONTROLFILE, "ControlFile", 2, _input_section);
    _parameter_info[PRECISION] = ParamInfo(PRECISION, "PrecisionCaseTimes", 3, _input_section);
    _parameter_info[STARTDATE] = ParamInfo(STARTDATE, "StartDate", 4, _input_section);
    _parameter_info[ENDDATE] = ParamInfo(ENDDATE, "EndDate", 5, _input_section);
    _parameter_info[POPFILE] = ParamInfo(POPFILE, "PopulationFile", 6, _input_section);
    _parameter_info[COORDFILE] = ParamInfo(COORDFILE, "CoordinatesFile", 7, _input_section);
    _parameter_info[COORDTYPE] = ParamInfo(COORDTYPE, "CoordinatesType", 8, _input_section);
    _parameter_info[SPECIALGRID] = ParamInfo(SPECIALGRID, "UseGridFile", 9, _input_section);
    _parameter_info[GRIDFILE] = ParamInfo(GRIDFILE, "GridFile", 10, _input_section);

    _parameter_info[ANALYSISTYPE] = ParamInfo(ANALYSISTYPE, "AnalysisType", 1, _analysis_section);
    _parameter_info[MODEL] = ParamInfo(MODEL, "ModelType", 2, _analysis_section);
    _parameter_info[SCANAREAS] = ParamInfo(SCANAREAS, "ScanAreas", 3, _analysis_section);
    _parameter_info[TIME_AGGREGATION_UNITS] = ParamInfo(TIME_AGGREGATION_UNITS, "TimeAggregationUnits", 4, _analysis_section);
    _parameter_info[TIME_AGGREGATION] = ParamInfo(TIME_AGGREGATION, "TimeAggregationLength", 5, _analysis_section);
    _parameter_info[REPLICAS] = ParamInfo(REPLICAS, "MonteCarloReps", 6, _analysis_section);

    _parameter_info[OUTPUTFILE] = ParamInfo(OUTPUTFILE, "ResultsFile", 1, _output_section);
    _parameter_info[OUTPUT_SIM_LLR_ASCII] = ParamInfo(OUTPUT_SIM_LLR_ASCII, "SaveSimLLRsASCII", 2, _output_section);
    _parameter_info[OUTPUT_RR_ASCII] = ParamInfo(OUTPUT_RR_ASCII, "IncludeRelativeRisksCensusAreasASCII", 3, _output_section);
    _parameter_info[OUTPUT_AREAS_ASCII] = ParamInfo(OUTPUT_AREAS_ASCII, "CensusAreasReportedClustersASCII", 4, _output_section);
    _parameter_info[OUTPUT_MLC_ASCII] = ParamInfo(OUTPUT_MLC_ASCII, "MostLikelyClusterEachCentroidASCII", 5, _output_section);
    _parameter_info[OUTPUT_MLC_DBASE] = ParamInfo(OUTPUT_MLC_DBASE, "MostLikelyClusterEachCentroidDBase", 6, _output_section);
    _parameter_info[OUTPUT_AREAS_DBASE] = ParamInfo(OUTPUT_AREAS_DBASE, "CensusAreasReportedClustersDBase", 7, _output_section);
    _parameter_info[OUTPUT_RR_DBASE] = ParamInfo(OUTPUT_RR_DBASE, "IncludeRelativeRisksCensusAreasDBase", 8, _output_section);
    _parameter_info[OUTPUT_SIM_LLR_DBASE] = ParamInfo(OUTPUT_SIM_LLR_DBASE, "SaveSimLLRsDBase", 9, _output_section);

    _parameter_info[MULTI_DATASET_PURPOSE_TYPE] = ParamInfo(MULTI_DATASET_PURPOSE_TYPE, "MultipleDataSetsPurposeType", 1, _multiple_data_section);

    _parameter_info[GEOSIZE] = ParamInfo(GEOSIZE, "MaxGeographicSize", 1, _spatial_window_section);
    _parameter_info[MAX_SPATIAL_TYPE] = ParamInfo(MAX_SPATIAL_TYPE, "MaxSpatialSizeInterpretation", 2, _spatial_window_section);
    _parameter_info[MAXCIRCLEPOPFILE] = ParamInfo(MAXCIRCLEPOPFILE, "MaxCirclePopulationFile", 3, _spatial_window_section);
    _parameter_info[PURETEMPORAL] = ParamInfo(PURETEMPORAL, "IncludePurelyTemporal", 4, _spatial_window_section);

    _parameter_info[MAX_TEMPORAL_TYPE] = ParamInfo(MAX_TEMPORAL_TYPE, "MaxTemporalSizeInterpretation", 1, _temporal_window_section);
    _parameter_info[TIMESIZE] = ParamInfo(TIMESIZE, "MaxTemporalSize", 2, _temporal_window_section);
    _parameter_info[PURESPATIAL] = ParamInfo(PURESPATIAL, "IncludePurelySpatial", 3, _temporal_window_section);
    _parameter_info[CLUSTERS] = ParamInfo(CLUSTERS, "IncludeClusters", 4, _temporal_window_section);

    _parameter_info[TIMETREND] = ParamInfo(TIMETREND, "TimeTrendAdjustmentType", 1, _space_time_adjustments_section);
    _parameter_info[TIMETRENDPERC] = ParamInfo(TIMETRENDPERC, "TimeTrendPercentage", 2, _space_time_adjustments_section);
    _parameter_info[SPATIAL_ADJ_TYPE] = ParamInfo(SPATIAL_ADJ_TYPE, "SpatialAdjustmentType", 3, _space_time_adjustments_section);
    _parameter_info[USE_ADJ_BY_RR_FILE] = ParamInfo(USE_ADJ_BY_RR_FILE, "UseAdjustmentsByRRFile", 4, _space_time_adjustments_section);
    _parameter_info[ADJ_BY_RR_FILE] = ParamInfo(ADJ_BY_RR_FILE, "AdjustmentsByKnownRelativeRisksFilename", 5, _space_time_adjustments_section);
    _parameter_info[INTERVAL_STARTRANGE] = ParamInfo(INTERVAL_STARTRANGE, "IntervalStartRange", 6, _space_time_adjustments_section);
    _parameter_info[INTERVAL_ENDRANGE] = ParamInfo(INTERVAL_ENDRANGE, "IntervalEndRange", 7, _space_time_adjustments_section);

    _parameter_info[EARLY_SIM_TERMINATION] = ParamInfo(EARLY_SIM_TERMINATION, "EarlySimulationTermination", 1, _inference_section);
    _parameter_info[START_PROSP_SURV] = ParamInfo(START_PROSP_SURV, "ProspectiveStartDate", 2, _inference_section);
    _parameter_info[ADJ_FOR_EALIER_ANALYSES] = ParamInfo(ADJ_FOR_EALIER_ANALYSES, "AdjustForEarlierAnalyses", 3, _inference_section);
    _parameter_info[REPORT_CRITICAL_VALUES] = ParamInfo(REPORT_CRITICAL_VALUES, "CriticalValue", 4, _inference_section);

    _parameter_info[CRITERIA_SECOND_CLUSTERS] = ParamInfo(CRITERIA_SECOND_CLUSTERS, "CriteriaForReportingSecondaryClusters", 1, _clusters_reported_section);
    _parameter_info[REPORTED_GEOSIZE] = ParamInfo(REPORTED_GEOSIZE, "MaxReportedGeoClusterSize", 2, _clusters_reported_section);
    _parameter_info[USE_REPORTED_GEOSIZE] = ParamInfo(USE_REPORTED_GEOSIZE, "UseReportOnlySmallerClusters", 3, _clusters_reported_section);

    _parameter_info[WINDOW_SHAPE] = ParamInfo(WINDOW_SHAPE, "NumberOfEllipses", 1, _elliptic_section);
    _parameter_info[ESHAPES] = ParamInfo(ESHAPES, "EllipseShapes", 2, _elliptic_section);
    _parameter_info[ENUMBERS] = ParamInfo(ENUMBERS, "EllipseAngles", 3, _elliptic_section);
    _parameter_info[NON_COMPACTNESS_PENALTY] = ParamInfo(NON_COMPACTNESS_PENALTY, "NonCompactnessPenalty", 4, _elliptic_section);

    _parameter_info[RISKFUNCTION] = ParamInfo(RISKFUNCTION, "IsotonicScan", 1, _isotonic_scan_section);

    _parameter_info[ITERATIVE] = ParamInfo(ITERATIVE, "SequentialScan", 1, _sequential_section);
    _parameter_info[ITERATIVE_NUM] = ParamInfo(ITERATIVE_NUM, "SequentialScanMaxIterations", 2, _sequential_section);
    _parameter_info[ITERATIVE_PVAL] = ParamInfo(ITERATIVE_PVAL, "SequentialScanMaxPValue", 3, _sequential_section);

    _parameter_info[POWER_EVALUATION] = ParamInfo(POWER_EVALUATION, "PValues2PrespecifiedLLRs", 1, _power_simulations_section);
    _parameter_info[POWER_05] = ParamInfo(POWER_05, "LLR1", 2, _power_simulations_section);
    _parameter_info[POWER_01] = ParamInfo(POWER_01, "LLR2", 3, _power_simulations_section);
    _parameter_info[SIMULATION_TYPE] = ParamInfo(SIMULATION_TYPE, "SimulatedDataMethodType", 4, _power_simulations_section);
    _parameter_info[SIMULATION_SOURCEFILE] = ParamInfo(SIMULATION_SOURCEFILE, "SimulatedDataInputFilename", 5, _power_simulations_section);
    _parameter_info[OUTPUT_SIMULATION_DATA] = ParamInfo(OUTPUT_SIMULATION_DATA, "PrintSimulatedDataToFile", 6, _power_simulations_section);
    _parameter_info[SIMULATION_DATA_OUTFILE] = ParamInfo(SIMULATION_DATA_OUTFILE, "SimulatedDataOutputFilename", 7, _power_simulations_section);

    _parameter_info[VALIDATE] = ParamInfo(VALIDATE, "ValidateParameters", 1, _batch_features_section);
    _parameter_info[RANDOMIZATION_SEED] = ParamInfo(RANDOMIZATION_SEED, "RandomSeed", 2, _batch_features_section);

    _parameter_info[CREATION_VERSION] = ParamInfo(CREATION_VERSION, "Version", 1, _system_section);

    _parameter_info[DIMENSION] = ParamInfo(DIMENSION, NotUsed, 0, _not_used_section);
    _parameter_info[EXACTTIMES] = ParamInfo(EXACTTIMES, NotUsed, 0, _not_used_section);
    _parameter_info[RUN_HISTORY_FILENAME] = ParamInfo(RUN_HISTORY_FILENAME, NotUsed, 0, _not_used_section);
    _parameter_info[TIMETRENDCONVRG] = ParamInfo(TIMETRENDCONVRG, NotUsed, 0, _not_used_section);

    _multiple_parameter_info[CASEFILE] = ParamInfo(CASEFILE, "CaseFile", 2, _multiple_data_section);
    _multiple_parameter_info[CONTROLFILE] = ParamInfo(CONTROLFILE, "ControlFile", 3, _multiple_data_section);
    _multiple_parameter_info[POPFILE] = ParamInfo(POPFILE, "PopulationFile", 4, _multiple_data_section);

    assert(_parameter_info.size() == 70);
}

/** Version 6.0.x */
void IniParameterSpecification::Build_6_0_x_ParameterList() {
    Build_5_1_x_ParameterList();

    //Flexible start and end range were in wrong section
    _parameter_info[INTERVAL_STARTRANGE]._section = &_temporal_window_section;
    _parameter_info[INTERVAL_ENDRANGE]._section = &_temporal_window_section;

    //VALIDATE and RANDOMIZATION_SEED parameters moved from 'BatchModeFeatures' to 'RunOptions'
    _parameter_info[VALIDATE] = ParamInfo(VALIDATE, "ValidateParameters", 1, _run_options_section);
    _parameter_info[RANDOMIZATION_SEED] = ParamInfo(RANDOMIZATION_SEED, "RandomSeed", 2, _run_options_section);

    _parameter_info[EXECUTION_TYPE] = ParamInfo(EXECUTION_TYPE, "ExecutionType", 3, _run_options_section);
    _parameter_info[NUM_PROCESSES] = ParamInfo(NUM_PROCESSES, "NumberParallelProcesses", 4, _run_options_section);
    _parameter_info[LOG_HISTORY] = ParamInfo(LOG_HISTORY, "LogRunToHistoryFile", 5, _run_options_section);

    assert(_parameter_info.size() == 73);
}

/** Version 6.1.x */
void IniParameterSpecification::Build_6_1_x_ParameterList() {
    Build_6_0_x_ParameterList();

    //Elliptic parameter replaced by spatial window type
    _parameter_info[WINDOW_SHAPE] = ParamInfo(WINDOW_SHAPE, "SpatialWindowShapeType", 5, _spatial_window_section);
    _parameter_info[NON_COMPACTNESS_PENALTY] = ParamInfo(NON_COMPACTNESS_PENALTY, "NonCompactnessPenalty", 6, _spatial_window_section);

    _parameter_info[MAX_REPORTED_SPATIAL_TYPE] = ParamInfo(MAX_REPORTED_SPATIAL_TYPE, "MaxReportedSpatialSizeInterpretation", 4, _clusters_reported_section);

    _parameter_info[OUTPUT_MLC_CASE_ASCII] = ParamInfo(OUTPUT_MLC_CASE_ASCII, "MostLikelyClusterCaseInfoEachCentroidASCII", 15, _output_section);
    _parameter_info[OUTPUT_MLC_CASE_DBASE] = ParamInfo(OUTPUT_MLC_CASE_DBASE, "MostLikelyClusterCaseInfoEachCentroidDBase", 16, _output_section);

    _parameter_info[SUPPRESS_WARNINGS] = ParamInfo(SUPPRESS_WARNINGS, "SuppressWarnings", 6, _run_options_section);

    assert(_parameter_info.size() == 77);
}

/** Version 7.0.x */
void IniParameterSpecification::Build_7_0_x_ParameterList() {
    Build_6_1_x_ParameterList();

    //Sequential Scan parameters moved to Inference section - removed SequentialScan section
    _parameter_info[ITERATIVE] = ParamInfo(ITERATIVE, "IterativeScan", 5, _inference_section);
    _parameter_info[ITERATIVE_NUM] = ParamInfo(ITERATIVE_NUM, "IterativeScanMaxIterations", 6, _inference_section);
    _parameter_info[ITERATIVE_PVAL] = ParamInfo(ITERATIVE_PVAL, "IterativeScanMaxPValue", 7, _inference_section);

    _parameter_info[STUDYPERIOD_DATACHECK] = ParamInfo(STUDYPERIOD_DATACHECK, "StudyPeriodCheckType", 1, _data_checking_section);
    _parameter_info[COORDINATES_DATACHECK] = ParamInfo(COORDINATES_DATACHECK, "GeographicalCoordinatesCheckType", 2, _data_checking_section);

    _parameter_info[LOCATION_NEIGHBORS_FILE] = ParamInfo(LOCATION_NEIGHBORS_FILE, "NeighborsFilename", 1, _non_eucledian_section);
    _parameter_info[USE_LOCATION_NEIGHBORS_FILE] = ParamInfo(USE_LOCATION_NEIGHBORS_FILE, "UseNeighborsFile", 2, _non_eucledian_section);


    _parameter_info[MAXGEOPOPATRISK] = ParamInfo(MAXGEOPOPATRISK, "MaxSpatialSizeInPopulationAtRisk", 7, _spatial_window_section);
    _parameter_info[MAXGEOPOPFILE] = ParamInfo(MAXGEOPOPFILE, "MaxSpatialSizeInMaxCirclePopulationFile", 8, _spatial_window_section);
    _parameter_info[MAXGEODISTANCE] = ParamInfo(MAXGEODISTANCE, "MaxSpatialSizeInDistanceFromCenter", 9, _spatial_window_section);
    _parameter_info[USE_MAXGEOPOPFILE] = ParamInfo(USE_MAXGEOPOPFILE, "UseMaxCirclePopulationFileOption", 10, _spatial_window_section);
    _parameter_info[USE_MAXGEODISTANCE] = ParamInfo(USE_MAXGEODISTANCE, "UseDistanceFromCenterOption", 11, _spatial_window_section);

    _parameter_info[MAXGEOPOPATRISK_REPORTED] = ParamInfo(MAXGEOPOPATRISK_REPORTED, "MaxSpatialSizeInPopulationAtRisk_Reported", 5, _clusters_reported_section);
    _parameter_info[MAXGEOPOPFILE_REPORTED] = ParamInfo(MAXGEOPOPFILE_REPORTED, "MaxSizeInMaxCirclePopulationFile_Reported", 6, _clusters_reported_section);
    _parameter_info[MAXGEODISTANCE_REPORTED] = ParamInfo(MAXGEODISTANCE_REPORTED, "MaxSpatialSizeInDistanceFromCenter_Reported", 7, _clusters_reported_section);
    _parameter_info[USE_MAXGEOPOPFILE_REPORTED] = ParamInfo(USE_MAXGEOPOPFILE_REPORTED, "UseMaxCirclePopulationFileOption_Reported", 8, _clusters_reported_section);
    _parameter_info[USE_MAXGEODISTANCE_REPORTED] = ParamInfo(USE_MAXGEODISTANCE_REPORTED, "UseDistanceFromCenterOption_Reported", 9, _clusters_reported_section);

    //Maximum spatial cluster size updated to use new parameters
    _parameter_info[GEOSIZE] = ParamInfo(GEOSIZE, NotUsed, 0, _not_used_section);
    _parameter_info[MAX_SPATIAL_TYPE] = ParamInfo(MAX_SPATIAL_TYPE, NotUsed,  0, _not_used_section);
    _parameter_info[REPORTED_GEOSIZE] = ParamInfo(REPORTED_GEOSIZE, NotUsed, 0, _not_used_section);
    _parameter_info[MAX_REPORTED_SPATIAL_TYPE] = ParamInfo(MAX_REPORTED_SPATIAL_TYPE, NotUsed, 0, _not_used_section);

    //validate parameters no longer used
    _parameter_info[VALIDATE] = ParamInfo(VALIDATE, NotUsed, 0, _not_used_section);

    assert(_parameter_info.size() == 91);
}

/** Version 8.0.x */
void IniParameterSpecification::Build_8_0_x_ParameterList() {
    Build_7_0_x_ParameterList();

    _parameter_info[OBSERVABLE_REGIONS] = ParamInfo(OBSERVABLE_REGIONS, "Polygons", 1, _polygons_section);
    _multiple_parameter_info[OBSERVABLE_REGIONS] = ParamInfo(OBSERVABLE_REGIONS, "Polygon", 1, _polygons_section);

    //non-Euclidian neighbors moved to new spatial neighbors tab
    _parameter_info[USE_LOCATION_NEIGHBORS_FILE] = ParamInfo(USE_LOCATION_NEIGHBORS_FILE, "UseNeighborsFile", 1, _spatial_neighbors_section);
    _parameter_info[LOCATION_NEIGHBORS_FILE] = ParamInfo(LOCATION_NEIGHBORS_FILE, "NeighborsFilename", 2, _spatial_neighbors_section);
    _parameter_info[USE_META_LOCATIONS_FILE] = ParamInfo(USE_META_LOCATIONS_FILE, "UseMetaLocationsFile", 3, _spatial_neighbors_section);
    _parameter_info[META_LOCATIONS_FILE] = ParamInfo(META_LOCATIONS_FILE, "MetaLocationsFilename", 4, _spatial_neighbors_section);
    _parameter_info[MULTIPLE_COORDINATES_TYPE] = ParamInfo(MULTIPLE_COORDINATES_TYPE, "MultipleCoordinatesType", 5, _spatial_neighbors_section);

    //risk/isotonic scan moved to spatial window tab
    _parameter_info[RISKFUNCTION] = ParamInfo(RISKFUNCTION, "IsotonicScan", 12, _spatial_window_section);

    //critical values parameter moved to new to additional output tab
    _parameter_info[REPORT_CRITICAL_VALUES] = ParamInfo(REPORT_CRITICAL_VALUES, "CriticalValue", 1, _additional_output_section);

    _parameter_info[RANDOMLY_GENERATE_SEED] = ParamInfo(RANDOMLY_GENERATE_SEED, "RandomlyGenerateSeed", 7, _run_options_section);

    assert(_parameter_info.size() == 96);
}

/** Version 8.2.x */
void IniParameterSpecification::Build_8_2_x_ParameterList() {
    Build_8_0_x_ParameterList();

    // number of replications moved to inference tab
    _parameter_info[REPLICAS] = ParamInfo(REPLICAS, "MonteCarloReps", 8, _inference_section);

    assert(_parameter_info.size() == 96);
}

/** Version 9.0.x */
void IniParameterSpecification::Build_9_0_x_ParameterList() {
    Build_8_2_x_ParameterList();

    _parameter_info[TIME_TREND_TYPE] = ParamInfo(TIME_TREND_TYPE, "TimeTrendType", 8, _space_time_adjustments_section);

    _parameter_info[EARLY_TERM_THRESHOLD] = ParamInfo(EARLY_TERM_THRESHOLD, "EarlyTerminationThreshold", 15, _inference_section);
    _parameter_info[PVALUE_REPORT_TYPE] = ParamInfo(PVALUE_REPORT_TYPE, "PValueReportType", 16, _inference_section);
    _parameter_info[REPORT_GUMBEL] = ParamInfo(REPORT_GUMBEL, "ReportGumbel", 17, _inference_section);

    _parameter_info[REPORT_RANK] = ParamInfo(REPORT_RANK, "ReportClusterRank", 2, _additional_output_section);
    _parameter_info[PRINT_ASCII_HEADERS] = ParamInfo(PRINT_ASCII_HEADERS, "PrintAsciiColumnHeaders", 3, _additional_output_section);

    assert(_parameter_info.size() == 102);
}

/** Version 9.2.x */
void IniParameterSpecification::Build_9_2_x_ParameterList() {
    Build_9_0_x_ParameterList();

    _parameter_info[ADJUST_WEEKLY_TRENDS] = ParamInfo(ADJUST_WEEKLY_TRENDS, "AdjustForWeeklyTrends", 9, _space_time_adjustments_section);

    _parameter_info[POWER_EVALUATION] = ParamInfo(POWER_EVALUATION, "PerformPowerEvaluation", 1, _power_evaluation_section);
    _parameter_info[PE_COUNT] = ParamInfo(PE_COUNT, "PowerEvaluationTotalCases", 2, _power_evaluation_section);
    _parameter_info[PE_CRITICAL_TYPE] = ParamInfo(PE_CRITICAL_TYPE, "CriticalValueType", 3, _power_evaluation_section);
    _parameter_info[PE_ESTIMATION_TYPE] = ParamInfo(PE_ESTIMATION_TYPE, "PowerEstimationType", 4, _power_evaluation_section);
    _parameter_info[PE_ALT_HYPOTHESIS_FILE] = ParamInfo(PE_ALT_HYPOTHESIS_FILE, "AlternativeHypothesisFilename", 5, _power_evaluation_section);
    _parameter_info[PE_POWER_REPLICAS] = ParamInfo(PE_POWER_REPLICAS, "NumberPowerReplications", 6, _power_evaluation_section);
    _parameter_info[PE_SIMULATION_TYPE] = ParamInfo(PE_SIMULATION_TYPE, "PowerEvaluationsSimulationMethod", 7, _power_evaluation_section);
    _parameter_info[PE_SIMULATION_SOURCEFILE] = ParamInfo(PE_SIMULATION_SOURCEFILE, "PowerEvaluationsSimulationSourceFilename", 8, _power_evaluation_section);
    _parameter_info[PE_METHOD_TYPE] = ParamInfo(PE_METHOD_TYPE, "PowerEvaluationsMethod", 9, _power_evaluation_section);
    _parameter_info[POWER_05] = ParamInfo(POWER_05, "CriticalValue05", 10, _power_evaluation_section);
    _parameter_info[POWER_01] = ParamInfo(POWER_01, "CriticalValue01", 11, _power_evaluation_section);
    _parameter_info[POWER_001] = ParamInfo(POWER_001, "CriticalValue001", 12, _power_evaluation_section);
    _parameter_info[PE_OUTPUT_SIMUALTION_DATA] = ParamInfo(PE_OUTPUT_SIMUALTION_DATA, "ReportPowerEvaluationSimulationData", 13, _power_evaluation_section);
    _parameter_info[PE_SIMUALTION_OUTPUTFILE] = ParamInfo(PE_SIMUALTION_OUTPUTFILE, "PowerEvaluationsSimulationOutputFilename", 14, _power_evaluation_section);

    _parameter_info[OUTPUT_KML] = ParamInfo(OUTPUT_KML, "OutputGoogleEarthKML", 13, _output_section);
    _parameter_info[OUTPUT_SHAPEFILES] = ParamInfo(OUTPUT_SHAPEFILES, "OutputShapefiles", 14, _output_section);
    _parameter_info[OUTPUT_TEMPORAL_GRAPH] = ParamInfo(OUTPUT_TEMPORAL_GRAPH, "OutputTemporalGraphHTML", 1, _temporal_output_section);

    // 'Clusters Reported' was renamed to 'Spatial Output'
    _parameter_info[INCLUDE_LOCATIONS_KML] = ParamInfo(INCLUDE_LOCATIONS_KML, "IncludeClusterLocationsKML", 1, _spatial_output_section);
    _parameter_info[LOCATIONS_THRESHOLD_KML] = ParamInfo(LOCATIONS_THRESHOLD_KML, "ThresholdLocationsSeparateKML", 2, _spatial_output_section);
    _parameter_info[COMPRESS_KML_OUTPUT] = ParamInfo(COMPRESS_KML_OUTPUT, "CompressKMLtoKMZ", 3, _spatial_output_section);
    _parameter_info[LAUNCH_KML_VIEWER] = ParamInfo(LAUNCH_KML_VIEWER, "LaunchKMLViewer", 4, _spatial_output_section);

    _parameter_info[REPORT_HIERARCHICAL_CLUSTERS] = ParamInfo(REPORT_HIERARCHICAL_CLUSTERS, "ReportHierarchicalClusters", 5, _spatial_output_section);
    _parameter_info[REPORT_GINI_CLUSTERS] = ParamInfo(REPORT_GINI_CLUSTERS, "ReportGiniClusters", 6, _spatial_output_section);
    _parameter_info[SPATIAL_MAXIMA] = ParamInfo(SPATIAL_MAXIMA, "SpatialMaxima", 7, _spatial_output_section);
    _parameter_info[GINI_INDEX_REPORT_TYPE] = ParamInfo(GINI_INDEX_REPORT_TYPE, "GiniIndexClusterReportingType", 8, _spatial_output_section);
    _parameter_info[GINI_INDEX_PVALUE_CUTOFF] = ParamInfo(GINI_INDEX_PVALUE_CUTOFF, "GiniIndexClustersPValueCutOff", 9, _spatial_output_section);
    _parameter_info[REPORT_GINI_COEFFICENTS] = ParamInfo(REPORT_GINI_COEFFICENTS, "ReportGiniIndexCoefficents", 10, _spatial_output_section);
    _parameter_info[CRITERIA_SECOND_CLUSTERS] = ParamInfo(CRITERIA_SECOND_CLUSTERS, "CriteriaForReportingSecondaryClusters", 11, _spatial_output_section);
    _parameter_info[USE_REPORTED_GEOSIZE] = ParamInfo(USE_REPORTED_GEOSIZE, "UseReportOnlySmallerClusters", 12, _spatial_output_section);
    _parameter_info[MAXGEOPOPATRISK_REPORTED] = ParamInfo(MAXGEOPOPATRISK_REPORTED, "MaxSpatialSizeInPopulationAtRisk_Reported", 13, _spatial_output_section);
    _parameter_info[USE_MAXGEOPOPFILE_REPORTED] = ParamInfo(USE_MAXGEOPOPFILE_REPORTED, "UseMaxCirclePopulationFileOption_Reported", 14, _spatial_output_section);
    _parameter_info[MAXGEOPOPFILE_REPORTED] = ParamInfo(MAXGEOPOPFILE_REPORTED, "MaxSizeInMaxCirclePopulationFile_Reported", 15, _spatial_output_section);
    _parameter_info[USE_MAXGEODISTANCE_REPORTED] = ParamInfo(USE_MAXGEODISTANCE_REPORTED, "UseDistanceFromCenterOption_Reported", 16, _spatial_output_section);
    _parameter_info[MAXGEODISTANCE_REPORTED] = ParamInfo(MAXGEODISTANCE_REPORTED, "MaxSpatialSizeInDistanceFromCenter_Reported", 17, _spatial_output_section);

    // 'Additional Output' was renamed to 'Other Output'
    _parameter_info[REPORT_CRITICAL_VALUES] = ParamInfo(REPORT_CRITICAL_VALUES, "CriticalValue",1, _other_output_section);
    _parameter_info[REPORT_RANK] = ParamInfo(REPORT_RANK, "ReportClusterRank", 2, _other_output_section);
    _parameter_info[PRINT_ASCII_HEADERS] = ParamInfo(PRINT_ASCII_HEADERS, "PrintAsciiColumnHeaders", 3, _other_output_section);

    assert(_parameter_info.size() == 127);
}

/** Version 9.3.x */
void IniParameterSpecification::Build_9_3_x_ParameterList() {
    Build_9_2_x_ParameterList();

    _parameter_info[MIN_TEMPORAL_CLUSTER] = ParamInfo(MIN_TEMPORAL_CLUSTER, "MinimumTemporalClusterSize", 7, _temporal_window_section);

    assert(_parameter_info.size() == 128);
}

/** Version 9.4.x */
void IniParameterSpecification::Build_9_4_x_ParameterList() {
    Build_9_3_x_ParameterList();

    _parameter_info[TEMPORAL_GRAPH_REPORT_TYPE] = ParamInfo(TEMPORAL_GRAPH_REPORT_TYPE, "TemporalGraphReportType", 2, _temporal_output_section);
    _parameter_info[TEMPORAL_GRAPH_MLC_COUNT] = ParamInfo(TEMPORAL_GRAPH_MLC_COUNT, "TemporalGraphMostMLC", 3, _temporal_output_section);
    _parameter_info[TEMPORAL_GRAPH_CUTOFF] = ParamInfo(TEMPORAL_GRAPH_CUTOFF, "TemporalGraphSignificanceCutoff", 4, _temporal_output_section);

    _parameter_info[USER_DEFINED_TITLE] = ParamInfo(USER_DEFINED_TITLE, "ResultsTitle", 4, _other_output_section);
    _parameter_info[CALCULATE_OLIVIERA] = ParamInfo(CALCULATE_OLIVIERA, "CalculateOliviera", 1, _border_analysis_section);
    _parameter_info[NUM_OLIVIERA_SETS] = ParamInfo(NUM_OLIVIERA_SETS, "NumOlivieraSets", 2, _border_analysis_section);
    _parameter_info[OLIVIERA_CUTOFF] = ParamInfo(OLIVIERA_CUTOFF, "OlivieraPvalueCutoff", 3, _border_analysis_section);

    assert(_parameter_info.size() == 135);
}

/** For sepcified ParameterType, attempts to retrieve ini section and key name if ini file.
    Returns true if parameter found else false. */
bool IniParameterSpecification::GetParameterIniInfo(ParameterType eParameterType,  const char ** sSectionName, const char ** sKey) const {
    ParameterInfoMap_t::const_iterator itr = _parameter_info.find(eParameterType);
    if (itr != _parameter_info.end()) {
        *sSectionName = itr->second._section->_label;
        *sKey = itr->second._label;
        return true;
    }
    return false;
}

/** For sepcified ParameterType, attempts to retrieve ini section and key name if ini file.
    Returns true if parameter found else false. */
bool IniParameterSpecification::GetMultipleParameterIniInfo(ParameterType eParameterType,  const char ** sSectionName, const char ** sKey) const {
    MultipleParameterInfoMap_t::const_iterator itr = _multiple_parameter_info.find(eParameterType);
    if (itr != _multiple_parameter_info.end()) {
        *sSectionName = itr->second._section->_label;
        *sKey = itr->second._label;
        return true;
    }
    return false;
}

IniParameterSpecification::ParameterInfoCollection_t & IniParameterSpecification::getParameterInfoCollection(ParameterInfoCollection_t& collection) const {
    collection.clear();
    for (ParameterInfoMap_t::const_iterator itr=_parameter_info.begin(); itr != _parameter_info.end(); ++itr) {
        if (strcmp(itr->second._section->_label, _not_used_section._label))
            //collection.push_back(itr->second);
            collection.push_back(ParamInfo(itr->second));
    }
    std::sort(collection.begin(), collection.end());
    return collection;
}

//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "Parameters.h"
#include "Randomizer.h"
#include "SSException.h"
#include <boost/assign/std/vector.hpp>
using namespace boost::assign;

const int CParameters::MAXIMUM_ITERATIVE_ANALYSES     = 32000;
const int CParameters::MAXIMUM_ELLIPSOIDS             = 10;
const int CParameters::giNumParameters                = 171;

/** Constructor */
CParameters::CParameters() {
  SetAsDefaulted();
}

/** Copy constructor */
CParameters::CParameters(const CParameters &other) {
   SetAsDefaulted();
   Copy(other);
}

/** Overload assignment operator */
CParameters &CParameters::operator=(const CParameters &rhs) {
  if (this != &rhs) Copy(rhs);
  return (*this);
}

bool  CParameters::operator==(const CParameters& rhs) const {
  if (geSpatialWindowType != rhs.geSpatialWindowType) return false;
  if (gvEllipseShapes != rhs.gvEllipseShapes) return false;
  if (gvEllipseRotations != rhs.gvEllipseRotations) return false;
  if (geNonCompactnessPenaltyType != rhs.geNonCompactnessPenaltyType) return false;
  if (glTotalNumEllipses != rhs.glTotalNumEllipses) return false;
  if (geAnalysisType != rhs.geAnalysisType) return false;
  if (geAreaScanRate != rhs.geAreaScanRate) return false;
  if (geProbabilityModelType != rhs.geProbabilityModelType) return false;
  if (geRiskFunctionType != rhs.geRiskFunctionType) return false;
  if (giReplications != rhs.giReplications) return false;
  if (_performPowerEvaluation != rhs._performPowerEvaluation) return false;
  if (_critical_value_05 != rhs._critical_value_05) return false;
  if (_critical_value_01 != rhs._critical_value_01) return false;
  if (_critical_value_001 != rhs._critical_value_001) return false;
  if (gsStudyPeriodStartDate != rhs.gsStudyPeriodStartDate) return false;
  if (gsStudyPeriodEndDate != rhs.gsStudyPeriodEndDate) return false;
  if (gdMaxTemporalClusterSize != rhs.gdMaxTemporalClusterSize) return false;
  if (geIncludeClustersType != rhs.geIncludeClustersType) return false;
  if (geTimeAggregationUnitsType != rhs.geTimeAggregationUnitsType) return false;
  if (glTimeAggregationLength != rhs.glTimeAggregationLength) return false;
  if (geTimeTrendAdjustType != rhs.geTimeTrendAdjustType) return false;
  if (gdTimeTrendAdjustPercentage != rhs.gdTimeTrendAdjustPercentage) return false;
  if (gbIncludePurelySpatialClusters != rhs.gbIncludePurelySpatialClusters) return false;
  if (gbIncludePurelyTemporalClusters != rhs.gbIncludePurelyTemporalClusters) return false;
  if (gvCaseFilenames != rhs.gvCaseFilenames) return false;
  if (gvControlFilenames != rhs.gvControlFilenames) return false;
  if (gvPopulationFilenames != rhs.gvPopulationFilenames) return false;
  if (gsCoordinatesFileName != rhs.gsCoordinatesFileName) return false;
  if (gsSpecialGridFileName != rhs.gsSpecialGridFileName) return false;
  if (gbUseSpecialGridFile != rhs.gbUseSpecialGridFile) return false;
  if (gsMaxCirclePopulationFileName != rhs.gsMaxCirclePopulationFileName) return false;
  if (gePrecisionOfTimesType != rhs.gePrecisionOfTimesType) return false;
  if (geCoordinatesType != rhs.geCoordinatesType) return false;
  if (gsOutputFileNameSetting != rhs.gsOutputFileNameSetting) return false;
  if (gbOutputSimLogLikeliRatiosAscii != rhs.gbOutputSimLogLikeliRatiosAscii) return false;
  if (gbOutputRelativeRisksAscii != rhs.gbOutputRelativeRisksAscii) return false;
  if (gbIterativeRuns != rhs.gbIterativeRuns) return false;
  if (giNumIterativeRuns != rhs.giNumIterativeRuns) return false;
  if (gbIterativeCutOffPValue != rhs.gbIterativeCutOffPValue) return false;
  if (gsProspectiveStartDate                 != rhs.gsProspectiveStartDate) return false;
  if (gbOutputAreaSpecificAscii              != rhs.gbOutputAreaSpecificAscii) return false;
  if (gbOutputClusterLevelAscii              != rhs.gbOutputClusterLevelAscii) return false;
  if (geCriteriaSecondClustersType           != rhs.geCriteriaSecondClustersType) return false;
  if (geMaxTemporalClusterSizeType           != rhs.geMaxTemporalClusterSizeType) return false;
  if (gbOutputClusterLevelDBase              != rhs.gbOutputClusterLevelDBase) return false;
  if (gbOutputAreaSpecificDBase              != rhs.gbOutputAreaSpecificDBase) return false;
  if (gbOutputRelativeRisksDBase             != rhs.gbOutputRelativeRisksDBase) return false;
  if (gbOutputSimLogLikeliRatiosDBase        != rhs.gbOutputSimLogLikeliRatiosDBase) return false;
  if (gbLogRunHistory                        != rhs.gbLogRunHistory) return false;
  if (gsParametersSourceFileName             != rhs.gsParametersSourceFileName) return false;
  if (gsEndRangeStartDate                    != rhs.gsEndRangeStartDate) return false;
  if (gsEndRangeEndDate                      != rhs.gsEndRangeEndDate) return false;
  if (gsStartRangeStartDate                  != rhs.gsStartRangeStartDate) return false;
  if (gsStartRangeEndDate                    != rhs.gsStartRangeEndDate) return false;
  if (gdTimeTrendConverge                    != rhs.gdTimeTrendConverge) return false;
  if (gbRestrictReportedClusters             != rhs.gbRestrictReportedClusters) return false;
  if (_simulationType                        != rhs._simulationType) return false;
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
  if (gvSpatialWindowStops                   != rhs.gvSpatialWindowStops) return false;
  if (gsTitleName                            != rhs.gsTitleName) return false;
  if (_giniIndexPValueCutoff                 != rhs._giniIndexPValueCutoff) return false;
  if (_reportHierarchicalClusters            != rhs._reportHierarchicalClusters) return false;
  if (_reportGiniOptimizedClusters           != rhs._reportGiniOptimizedClusters) return false;
  if (_giniIndexReportType                   != rhs._giniIndexReportType) return false;
  if (_reportGiniIndexCoefficients           != rhs._reportGiniIndexCoefficients) return false;
  if (_powerEvaluationTotalCases             != rhs._powerEvaluationTotalCases) return false;
  if (_critical_value_type                   != rhs._critical_value_type) return false;
  if (_power_estimation_type                 != rhs._power_estimation_type) return false;
  if (_power_alt_hypothesis_filename         != rhs._power_alt_hypothesis_filename) return false;
  if (_power_replica != rhs._power_replica) return false;
  if (_power_simulation_type != rhs._power_simulation_type) return false;
  if (_power_simulation_source_filename != rhs._power_simulation_source_filename) return false;
  if (_report_power_simulation_data != rhs._report_power_simulation_data) return false;
  if (_power_simulation_output_filename != rhs._power_simulation_output_filename) return false;
  if (_power_evaluation_method != rhs._power_evaluation_method) return false;
  if (_output_kml != rhs._output_kml) return false;
  if (_output_temporal_graph != rhs._output_temporal_graph) return false;
  if (_temporal_graph_report_type != rhs._temporal_graph_report_type) return false;
  if (_temporal_graph_report_count != rhs._temporal_graph_report_count) return false;
  if (_temporal_graph_report_cutoff != rhs._temporal_graph_report_cutoff) return false;
  if (_output_shapefiles != rhs._output_shapefiles) return false;
  if (_include_locations_kml != rhs._include_locations_kml) return false;
  if (_locations_threshold_kml != rhs._locations_threshold_kml) return false;
  if (_compress_kml_output != rhs._compress_kml_output) return false;
  if (_launch_map_viewer != rhs._launch_map_viewer) return false;
  if (_adjustWeeklyTrends != rhs._adjustWeeklyTrends) return false;
  if (_minimum_temporal_cluster_size != rhs._minimum_temporal_cluster_size) return false;
  if (_temporal_graph_report_cutoff != rhs._temporal_graph_report_cutoff) return false;
  if (_temporal_graph_report_count != rhs._temporal_graph_report_count) return false;
  if (_temporal_graph_report_type != rhs._temporal_graph_report_type) return false;
  if (_calculate_oliveira_f != rhs._calculate_oliveira_f) return false;
  if (_num_oliveira_sets != rhs._num_oliveira_sets) return false;
  if (_oliveira_pvalue_cutoff != rhs._oliveira_pvalue_cutoff) return false;
  if (_output_cartesian_graph != rhs._output_cartesian_graph) return false; 
  if (_risk_limit_high_clusters != rhs._risk_limit_high_clusters) return false;
  if (_risk_threshold_high_clusters != rhs._risk_threshold_high_clusters) return false;
  if (_risk_limit_low_clusters != rhs._risk_limit_low_clusters) return false;
  if (_risk_threshold_low_clusters != rhs._risk_threshold_low_clusters) return false;
  if (_minimum_low_rate_cases != rhs._minimum_low_rate_cases) return false;
  if (_minimum_high_rate_cases != rhs._minimum_high_rate_cases) return false;
  if (_output_google_map != rhs._output_google_map) return false;
  if (_perform_standard_drilldown != rhs._perform_standard_drilldown) return false;
  if (_perform_bernoulli_drilldown != rhs._perform_bernoulli_drilldown) return false;
  if (_drilldown_minimum_locations != rhs._drilldown_minimum_locations) return false;
  if (_drilldown_minimum_cases != rhs._drilldown_minimum_cases) return false;
  if (_drilldown_pvalue_cutoff != rhs._drilldown_pvalue_cutoff) return false;
  if (_drilldown_adjust_weekly_trends != rhs._drilldown_adjust_weekly_trends) return false;
  if (_locations_network_filename != rhs._locations_network_filename) return false;
  if (_use_locations_network_file != rhs._use_locations_network_file) return false;
  if (_network_file_purpose != rhs._network_file_purpose) return false;
  if (_prospective_frequency_type != rhs._prospective_frequency_type) return false;
  if (_prospective_frequency != rhs._prospective_frequency) return false;
  if (_casefile_includes_linedata != rhs._casefile_includes_linedata) return false;
  if (_casefile_includes_header != rhs._casefile_includes_header) return false;
  if (_kml_event_group_attribute != rhs._kml_event_group_attribute) return false;
  if (_group_kml_linelist_attribute != rhs._group_kml_linelist_attribute) return false;
  if (_event_cache_filename != rhs._event_cache_filename) return false;
  if (_email_analysis_results != rhs._email_analysis_results) return false;
  if (_email_always_recipients != rhs._email_always_recipients) return false;
  if (_email_significant_recipients != rhs._email_significant_recipients) return false;
  if (_email_subject_no_significant != rhs._email_subject_no_significant) return false;
  if (_email_message_body_no_significant != rhs._email_message_body_no_significant) return false;
  if (_email_subject_significant != rhs._email_subject_significant) return false;
  if (_email_message_body_significant != rhs._email_message_body_significant) return false;
  if (_email_significant_ri_value != rhs._email_significant_ri_value) return false;
  if (_email_significant_ri_type != rhs._email_significant_ri_type) return false;
  if (_email_significant_pval_value != rhs._email_significant_pval_value) return false;
  if (_email_attach_results != rhs._email_attach_results) return false;
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

void CParameters::AddSpatialWindowStop(double windowStop, bool bEmptyFirst) {
  if (bEmptyFirst) gvSpatialWindowStops.clear();
  gvSpatialWindowStops.push_back(windowStop);
  // ensure that window stops are ascending
  std::sort(gvSpatialWindowStops.begin(), gvSpatialWindowStops.end());
  _executeSpatialWindowStops.clear();
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

      if (bCheckWritable && !ValidateFileAccess(fFilename.getFullPath(buffer), true, true)) {
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
  _performPowerEvaluation                = rhs._performPowerEvaluation;
  _critical_value_05                     = rhs._critical_value_05;
  _critical_value_01                     = rhs._critical_value_01;
  _critical_value_001                    = rhs._critical_value_001;
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
  gsOutputFileNameSetting                = rhs.gsOutputFileNameSetting;
  _results_filename                      = rhs._results_filename;
  _event_cache_filename                  = rhs._event_cache_filename;
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
  gbLogRunHistory                        = rhs.gbLogRunHistory;
  gsParametersSourceFileName             = rhs.gsParametersSourceFileName;
  gsEndRangeStartDate                    = rhs.gsEndRangeStartDate;
  gsEndRangeEndDate                      = rhs.gsEndRangeEndDate;
  gsStartRangeStartDate                  = rhs.gsStartRangeStartDate;
  gsStartRangeEndDate                    = rhs.gsStartRangeEndDate;
  gdTimeTrendConverge                    = rhs.gdTimeTrendConverge;
  gbRestrictReportedClusters             = rhs.gbRestrictReportedClusters;
  _simulationType                        = rhs._simulationType;
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
  gvSpatialWindowStops                   = rhs.gvSpatialWindowStops; _executeSpatialWindowStops.clear();
  gsTitleName                            = rhs.gsTitleName;
  _giniIndexPValueCutoff                 = rhs._giniIndexPValueCutoff;
  _reportHierarchicalClusters            = rhs._reportHierarchicalClusters;
  _reportGiniOptimizedClusters           = rhs._reportGiniOptimizedClusters;
  _giniIndexReportType                   = rhs._giniIndexReportType;
  _reportGiniIndexCoefficients           = rhs._reportGiniIndexCoefficients;
  _powerEvaluationTotalCases             = rhs._powerEvaluationTotalCases;
  _critical_value_type                   = rhs._critical_value_type;
  _power_estimation_type                 = rhs._power_estimation_type;
  _power_alt_hypothesis_filename         = rhs._power_alt_hypothesis_filename;
  _power_replica                         = rhs._power_replica;
  _power_simulation_type                 = rhs._power_simulation_type;
  _power_simulation_source_filename      = rhs._power_simulation_source_filename;
  _report_power_simulation_data          = rhs._report_power_simulation_data;
  _power_simulation_output_filename      = rhs._power_simulation_output_filename;
  _power_evaluation_method               = rhs._power_evaluation_method;
  _output_kml = rhs._output_kml;
  _output_temporal_graph = rhs._output_temporal_graph;
  _temporal_graph_report_type = rhs._temporal_graph_report_type;
  _temporal_graph_report_count = rhs._temporal_graph_report_count;
  _temporal_graph_report_cutoff = rhs._temporal_graph_report_cutoff;
  _output_shapefiles = rhs._output_shapefiles;
  _include_locations_kml = rhs._include_locations_kml;
  _compress_kml_output = rhs._compress_kml_output;
  _locations_threshold_kml = rhs._locations_threshold_kml;
  _launch_map_viewer = rhs._launch_map_viewer;
  _adjustWeeklyTrends = rhs._adjustWeeklyTrends;
  _minimum_temporal_cluster_size = rhs._minimum_temporal_cluster_size;
  _calculate_oliveira_f = rhs._calculate_oliveira_f;
  _num_oliveira_sets = rhs._num_oliveira_sets;
  _oliveira_pvalue_cutoff = rhs._oliveira_pvalue_cutoff;
  _output_cartesian_graph = rhs._output_cartesian_graph;
  _risk_limit_high_clusters = rhs._risk_limit_high_clusters;
  _risk_threshold_high_clusters = rhs._risk_threshold_high_clusters;
  _risk_limit_low_clusters = rhs._risk_limit_low_clusters;
  _risk_threshold_low_clusters = rhs._risk_threshold_low_clusters;
  _minimum_low_rate_cases = rhs._minimum_low_rate_cases;
  _minimum_high_rate_cases = rhs._minimum_high_rate_cases;
  _output_google_map = rhs._output_google_map;
  _input_sources = rhs._input_sources;
  _perform_standard_drilldown = rhs._perform_standard_drilldown;
  _perform_bernoulli_drilldown = rhs._perform_bernoulli_drilldown;
  _drilldown_minimum_locations = rhs._drilldown_minimum_locations;
  _drilldown_minimum_cases = rhs._drilldown_minimum_cases;
  _drilldown_pvalue_cutoff = rhs._drilldown_pvalue_cutoff;
  _drilldown_adjust_weekly_trends = rhs._drilldown_adjust_weekly_trends;
  _locations_network_filename = rhs._locations_network_filename;
  _use_locations_network_file = rhs._use_locations_network_file;
  _network_file_purpose = rhs._network_file_purpose;
  _cluster_moniker_prefix = rhs._cluster_moniker_prefix;
  _local_timestamp = rhs._local_timestamp;
  _prospective_frequency_type = rhs._prospective_frequency_type;
  _prospective_frequency = rhs._prospective_frequency;
  _casefile_includes_linedata = rhs._casefile_includes_linedata;
  _casefile_includes_header = rhs._casefile_includes_header;
  _kml_event_group_attribute = rhs._kml_event_group_attribute;
  _group_kml_linelist_attribute = rhs._group_kml_linelist_attribute;
  _email_analysis_results = rhs._email_analysis_results;
  _email_always_recipients = rhs._email_always_recipients;
  _email_significant_recipients = rhs._email_significant_recipients;
  _email_subject_no_significant = rhs._email_subject_no_significant;
  _email_message_body_no_significant = rhs._email_message_body_no_significant;
  _email_subject_significant = rhs._email_subject_significant;
  _email_message_body_significant = rhs._email_message_body_significant;
  _email_significant_ri_value = rhs._email_significant_ri_value;
  _email_significant_ri_value = rhs._email_significant_ri_value;
  _email_significant_pval_value = rhs._email_significant_pval_value;
  _email_attach_results = rhs._email_attach_results;
}

/* Returns whether line list data is read from case file - which is indicated in two exclusive ways:
   - user included line list meta row in case file
   - user used file wizard to define line list columns
*/
bool CParameters::getReadingLineDataFromCasefile() const {
    const InputSource * source = getInputSource(CASEFILE);
    return _casefile_includes_linedata || (source && source->getLinelistFieldsMap().size());
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

/* Returns email text with tags substituted. */
std::string CParameters::getEmailFormattedText(const std::string &messagebody, const std::string& newline) const {
    using boost::algorithm::replace_all;
    using boost::algorithm::ireplace_all;
    boost::posix_time::ptime localTime = boost::posix_time::second_clock::local_time();
    boost::posix_time::time_facet * facet = new boost::posix_time::time_facet();
    std::stringstream bufferStream, workStream;
    std::string buffer, message(messagebody);
    FileName fileName(_results_filename.c_str());

    // Replace <date> tag
    facet->format("%B"); // Full month name
    workStream.imbue(std::locale(std::locale::classic(), facet));
    workStream.str(""); workStream << localTime;
    bufferStream << workStream.str() << " " << localTime.date().day().as_number() << ", " << localTime.date().year();
    ireplace_all(message, "<date>", bufferStream.str());
    // Replace <output-directory> tag
    ireplace_all(message, "<output-directory>", fileName.getLocation(buffer).c_str());
    // Replace <output-filename> tag
    ireplace_all(message, "<output-filename>", fileName.getFullPath(buffer).c_str());
    // Replace newlines tag -- since we're using .ini file for parameters, newlines are a problem.
    workStream.str(""); workStream << std::endl;
    ireplace_all(message, "<linebreak>", newline.c_str());
    return message;
}

/** Returns threshold for early termination. If reporting default p-value, then
    threshold is determined by number of replications requested. */
unsigned int CParameters::GetExecuteEarlyTermThreshold() const {
  if (GetPValueReportingType() == DEFAULT_PVALUE) {
      return (GetNumReplicationsRequested() + 1)/20;
  }
  return giEarlyTermThreshold;
}

const std::vector<double> & CParameters::getExecuteSpatialWindowStops() const {
  if (_executeSpatialWindowStops.size()) return _executeSpatialWindowStops; // already calculated
  double spatialMaxInPopulationAtRisk = GetMaxSpatialSizeForType(PERCENTOFPOPULATION, GetRestrictingMaximumReportedGeoClusterSize());
  if (getReportGiniOptimizedClusters()) {
    for (std::vector<double>::const_iterator itr=gvSpatialWindowStops.begin(); itr != gvSpatialWindowStops.end(); ++itr) {
        if (*itr <= spatialMaxInPopulationAtRisk)
            _executeSpatialWindowStops.push_back(*itr);
    }  
    std::sort(_executeSpatialWindowStops.begin(), _executeSpatialWindowStops.end());
  } else
    _executeSpatialWindowStops.resize(1, spatialMaxInPopulationAtRisk);
  return _executeSpatialWindowStops;
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

/** Returns whether analysis is purely spatial. */
bool CParameters::GetIsPurelySpatialAnalysis() const {
	return geAnalysisType == PURELYSPATIAL;
}

/** Returns whether analysis is purely temporal. */
bool CParameters::GetIsPurelyTemporalAnalysis() const {
    return (geAnalysisType == PURELYTEMPORAL || geAnalysisType == PROSPECTIVEPURELYTEMPORAL || geAnalysisType == SEASONALTEMPORAL);
}

/** Returns whether analysis is space-time. */
bool CParameters::GetIsSpaceTimeAnalysis() const {
  return (geAnalysisType == SPACETIME || geAnalysisType == PROSPECTIVESPACETIME);
}

/** Returns description for LLR. */
bool CParameters::GetLogLikelihoodRatioIsTestStatistic() const {
  return (geProbabilityModelType == SPACETIMEPERMUTATION || 
          geProbabilityModelType == RANK || 
          geProbabilityModelType == UNIFORMTIME || 
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
 if (getCalculateOliveirasF()) return false;
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
         || eModelType == EXPONENTIAL || eModelType == RANK || eModelType == ORDINAL || eModelType == CATEGORICAL || eModelType == UNIFORMTIME;
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
    case PERCENTOFPOPULATION    : return bReported ? gbRestrictReportedClusters && gdMaxSpatialSizeInPopulationAtRisk_Reported : true;
    case MAXDISTANCE            : return bReported ? gbRestrictReportedClusters && gbRestrictMaxSpatialSizeThroughDistanceFromCenter_Reported : gbRestrictMaxSpatialSizeThroughDistanceFromCenter;
    case PERCENTOFMAXCIRCLEFILE : return bReported ? gbRestrictReportedClusters && gbRestrictMaxSpatialSizeThroughMaxCirclePopulationFile_Reported : gbRestrictMaxSpatialSizeThroughMaxCirclePopulationFile;
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
  _executeSpatialWindowStops.clear();
}

/** Returns whether early termination option is set. */
bool CParameters::GetTerminateSimulationsEarly() const {
   return (GetPValueReportingType() == DEFAULT_PVALUE || GetPValueReportingType() == TERMINATION_PVALUE) && // these 2 p-value options early terminate
          GetNumReplicationsRequested() >= MIN_SIMULATION_RPT_PVALUE && // number of replications is at least at threshold for reporting p-value
          getPerformPowerEvaluation() == false; // we're not performing the power evaluation
}

bool CParameters::getIsReportingIndexBasedCoefficents() const {
    return getReportGiniOptimizedClusters() && _reportGiniIndexCoefficients;
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
void CParameters::requestAllAdditionalOutputFiles() {
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
        GetProbabilityModelType() != CATEGORICAL &&
        !(getPerformPowerEvaluation() && getPowerEvaluationMethod() == PE_ONLY_SPECIFIED_CASES)) {
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
  if (eAnalysisType < PURELYSPATIAL || eAnalysisType > SEASONALTEMPORAL)
    throw prg_error("Enumeration %d out of range [%d,%d].", "SetAnalysisType()", eAnalysisType, PURELYSPATIAL, SEASONALTEMPORAL);
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

void CParameters::checkEnumeration(int e, int eLow, int eHigh) const {
  if (e < eLow || e > eHigh) throw prg_error("Enumeration %d out of range [%d,%d].", "checkEnumeration()", e, eLow, eHigh);
}

void CParameters::setGiniIndexReportType(GiniIndexReportType e) {
  checkEnumeration(e, OPTIMAL_ONLY, ALL_VALUES);
  _giniIndexReportType = e;
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

/** Sets locations network data file name.
If bCorrectForRelativePath is true, an attempt is made to modify filename
to path relative to executable. This is only attempted if current file does not exist. */
void CParameters::setLocationsNetworkFilename(const char * filename, bool bCorrectForRelativePath) {
    _locations_network_filename = filename;
    if (bCorrectForRelativePath)
        AssignMissingPath(_locations_network_filename);
}

/** Sets precision of input file dates type. Throws exception if out of range. */
void CParameters::SetCoordinatesType(CoordinatesType eCoordinatesType) {
  if (eCoordinatesType < CARTESIAN || eCoordinatesType > LATLON)
    throw prg_error("Enumeration %d out of range [%d,%d].", "SetCoordinatesType()", eCoordinatesType, CARTESIAN, LATLON);
  geCoordinatesType = eCoordinatesType;
}

/** Sets prospective frequency type. Throws exception if out of range. */
void CParameters::setProspectiveFrequencyType(ProspectiveFrequency e) {
    if (e < SAME_TIMEAGGREGATION || e > YEARLY)
        throw prg_error("Enumeration %d out of range [%d,%d].", "setProspectiveFrequencyType()", e, SAME_TIMEAGGREGATION, YEARLY);
    _prospective_frequency_type = e;
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
  gsOutputFileNameSetting                  = "";
  _results_filename                        = "";
  _event_cache_filename                    = "";
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
  gbLogRunHistory                          = false;
  geProbabilityModelType                   = POISSON;
  geRiskFunctionType                       = STANDARDRISK;
  _performPowerEvaluation                  = false;
  _critical_value_05                       = 0.0;
  _critical_value_01                       = 0.0;
  _critical_value_001                      = 0.0;
  geTimeTrendAdjustType                    = TEMPORAL_NOTADJUSTED;
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
  gvEllipseShapes += 1.5,2,3,4,5;
  gvEllipseRotations.clear();
  gvEllipseRotations += 4,6,9,12,15;
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
  _simulationType                          = STANDARD;
  gsSimulationDataSourceFileName           = "";
  gsAdjustmentsByRelativeRisksFileName     = "";
  gbOutputSimulationData                   = false;
  gsSimulationDataOutputFilename           = "";
  gbAdjustForEarlierAnalyses               = false;
  gbUseAdjustmentsForRRFile                = false;
  geSpatialAdjustmentType                  = SPATIAL_NOTADJUSTED;
  geMultipleSetPurposeType                 = MULTIVARIATE;
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
  gvSpatialWindowStops.clear();
  gvSpatialWindowStops += 1,2,3,4,5,6,8,10,12,15,20,25,30,40,50;
  gsTitleName = "";
  _giniIndexPValueCutoff = 0.05;
  _reportHierarchicalClusters = true;
  _reportGiniOptimizedClusters = true;
  _giniIndexReportType = OPTIMAL_ONLY;
  _reportGiniIndexCoefficients = false;
  _powerEvaluationTotalCases = 600;
  _critical_value_type = CV_MONTECARLO;
  _power_estimation_type = PE_MONTECARLO;
  _power_alt_hypothesis_filename = "";
  _power_replica = giReplications + 1;
  _power_simulation_type = STANDARD;
  _power_simulation_source_filename = "";
  _report_power_simulation_data = false;
  _power_simulation_output_filename = "";
  _power_evaluation_method = PE_WITH_ANALYSIS;
  _output_kml = false;
  _output_temporal_graph = false;
  _temporal_graph_report_cutoff = 0.05;
  _temporal_graph_report_count = 1;
  _temporal_graph_report_type = MLC_ONLY;
  _output_shapefiles = false;
  _include_locations_kml = true;
  _compress_kml_output = false;
  _launch_map_viewer = true;
  _locations_threshold_kml = 1000;
  _adjustWeeklyTrends = false;
  _minimum_temporal_cluster_size = 1;
  _calculate_oliveira_f = false;
  _num_oliveira_sets = 1000;
  _oliveira_pvalue_cutoff = 0.05;
  _output_cartesian_graph = false;
  _risk_limit_high_clusters = false;
  _risk_threshold_high_clusters = 1.0;
  _risk_limit_low_clusters = false;
  _risk_threshold_low_clusters = 1.0;
  _minimum_low_rate_cases = 0;
  _minimum_high_rate_cases = 2;
  _output_google_map = false;
  _perform_standard_drilldown = false;
  _perform_bernoulli_drilldown = false;
  _drilldown_minimum_locations = 2;
  _drilldown_minimum_cases = 10;
  _drilldown_pvalue_cutoff = 0.05;
  _drilldown_adjust_weekly_trends = false;
  _locations_network_filename = "";
  _use_locations_network_file = false;
  _network_file_purpose = NETWORK_DEFINITION;
  _cluster_moniker_prefix = "";
  _local_timestamp = boost::posix_time::second_clock::local_time();
  _prospective_frequency_type = SAME_TIMEAGGREGATION;
  _prospective_frequency = 1;
  _casefile_includes_linedata = false;
  _casefile_includes_header = false;
  _kml_event_group_attribute = "";
  _group_kml_linelist_attribute = false;
  _email_analysis_results = false;
  _email_always_recipients = "";
  _email_significant_recipients = "";
  _email_subject_no_significant = "";
  _email_message_body_no_significant = "";
  _email_subject_significant = "";
  _email_message_body_significant = "";
  _email_significant_ri_value = 100;
  _email_significant_ri_type = DAY;
  _email_significant_pval_value = 0.05;
  _email_attach_results = false;
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
void CParameters::setNumFileSets(size_t numSets) {
  if (numSets == 0)
    throw prg_error("Number of data sets can not be zero.\n", "setNumFileSets()");

  //adjust the number of filenames for case, control, and population
  gvCaseFilenames.resize(numSets);
  gvControlFilenames.resize(numSets);
  gvPopulationFilenames.resize(numSets);
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
void CParameters::SetOutputFileNameSetting(const char * sOutPutFileName, bool bCorrectForRelativePath) {
    gsOutputFileNameSetting = sOutPutFileName;
    if (bCorrectForRelativePath && !gsOutputFileNameSetting.empty() && gsOutputFileNameSetting.find(FileName::SLASH) == gsOutputFileNameSetting.npos) {
        // We're applying relative path to a filename which doesn't appear to have a path.
        gsOutputFileNameSetting = getFilenameFormatTime(gsOutputFileNameSetting, getTimestamp()); // apply any formatting so we can properly test write
        AssignMissingPath(gsOutputFileNameSetting, true); // Assign path to fully formatted filename;
        FileName resultant(gsOutputFileNameSetting.c_str());
        // replace original name passed.
        FileName original(sOutPutFileName);
        resultant.setFileName(original.getFileName().c_str());
        resultant.setExtension(original.getExtension().c_str());
        resultant.getFullPath(gsOutputFileNameSetting);
    }
    // gsOutputFileNameSetting could contain substitution variables - resolve now and store in _results_filename. 
    _results_filename = getFilenameFormatTime(gsOutputFileNameSetting, getTimestamp());
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

void CParameters::setPowerEvaluationAltHypothesisFilename(const char * f, bool bCorrectForRelativePath) {
  _power_alt_hypothesis_filename = f;
  if (bCorrectForRelativePath) AssignMissingPath(_power_alt_hypothesis_filename);
}

/** Sets relative risks adjustments file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void CParameters::SetAdjustmentsByRelativeRisksFilename(const char * sFileName, bool bCorrectForRelativePath) {
  gsAdjustmentsByRelativeRisksFileName = sFileName;
  if (bCorrectForRelativePath)
    AssignMissingPath(gsAdjustmentsByRelativeRisksFileName);
}

void CParameters::setPowerEvaluationCriticalValueType(CriticalValuesType e) {
  if (e < CV_MONTECARLO || e > CV_POWER_VALUES)
    throw prg_error("Enumeration %d out of range [%d,%d].", "setPowerEvaluationCriticalValueType()", e, PE_MONTECARLO, CV_POWER_VALUES);
  _critical_value_type = e;
}

/** Sets precision of input file dates type. Throws exception if out of range. */
void CParameters::SetPrecisionOfTimesType(DatePrecisionType eDatePrecisionType) {
  if (eDatePrecisionType < NONE || eDatePrecisionType > GENERIC)
    throw prg_error("Enumeration %d out of range [%d,%d].", "SetPrecisionOfTimesType()", eDatePrecisionType, NONE, GENERIC);
  gePrecisionOfTimesType = eDatePrecisionType;
}

/** Sets recurrence interval type. Throws exception if out of range. */
void CParameters::setEmailSignificantRecurrenceType(DatePrecisionType etype) {
    if (!(etype == DAY || etype == YEAR))
        throw prg_error("Invalid enumeration %d for settings.", "SetPrecisionOfTimesType()", etype);
    _email_significant_ri_type = etype;
}

/** Sets probability model type. Throws exception if out of range. */
void CParameters::SetProbabilityModelType(ProbabilityModelType eProbabilityModelType) {
  if (eProbabilityModelType < POISSON || eProbabilityModelType > UNIFORMTIME)
    throw prg_error("Enumeration %d out of range [%d,%d].", "SetAnalysisType()", eProbabilityModelType, POISSON, UNIFORMTIME);
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
void CParameters::SetSimulationType(SimulationType e) {
  if (e < STANDARD || e > FILESOURCE)
    throw prg_error("Enumeration %d out of range [%d,%d].", "SetSimulationType()", e, STANDARD, FILESOURCE);
  _simulationType = e;
}

/** sets power evalaution simulation procedure type for power step */
void CParameters::setPowerEvaluationSimulationType(SimulationType e) {
  if (e < STANDARD || e > FILESOURCE)
    throw prg_error("Enumeration %d out of range [%d,%d].", "SetPowerEvaluationSimulationType()", e, STANDARD, FILESOURCE);
  _power_simulation_type = e;
}

/** Sets method for performing the power evaluation. */
void CParameters::setPowerEvaluationMethod(PowerEvaluationMethodType e) {
  if (e < PE_WITH_ANALYSIS || e > PE_ONLY_SPECIFIED_CASES)
    throw prg_error("Enumeration %d out of range [%d,%d].", "setPowerEvaluationMethod()", e, PE_WITH_ANALYSIS, PE_ONLY_SPECIFIED_CASES);
  _power_evaluation_method = e;
}

/** Sets simulation data output filename.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void CParameters::SetSimulationDataOutputFileName(const char * sSourceFileName, bool bCorrectForRelativePath) {
  gsSimulationDataOutputFilename = sSourceFileName;
  if (bCorrectForRelativePath)
    AssignMissingPath(gsSimulationDataOutputFilename);
}

/** Sets power evaluation simulation data output filename.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void CParameters::setPowerEvaluationSimulationDataOutputFilename(const char * sSourceFileName, bool bCorrectForRelativePath) {
  _power_simulation_output_filename = sSourceFileName;
  if (bCorrectForRelativePath)
    AssignMissingPath(_power_simulation_output_filename);
}

/** Sets simulation data source file name.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void CParameters::SetSimulationDataSourceFileName(const char * sSourceFileName, bool bCorrectForRelativePath) {
  gsSimulationDataSourceFileName = sSourceFileName;
  if (bCorrectForRelativePath)
    AssignMissingPath(gsSimulationDataSourceFileName);
}

/** Sets power evaluation simulation data source filename for power step.
    If bCorrectForRelativePath is true, an attempt is made to modify filename
    to path relative to executable. This is only attempted if current file does not exist. */
void CParameters::setPowerEvaluationSimulationDataSourceFilename(const char * sSourceFileName, bool bCorrectForRelativePath) {
  _power_simulation_source_filename = sSourceFileName;
  if (bCorrectForRelativePath)
    AssignMissingPath(_power_simulation_source_filename);
}

void CParameters::setEventCacheFileName(const char * filename, bool bCorrectForRelativePath) {
    _event_cache_filename = filename;
    if (bCorrectForRelativePath) AssignMissingPath(_event_cache_filename);
}

/** Set spatial adjustment type. Throws exception if out of range. */
void CParameters::SetSpatialAdjustmentType(SpatialAdjustmentType eSpatialAdjustmentType) {
  if (eSpatialAdjustmentType < SPATIAL_NOTADJUSTED || eSpatialAdjustmentType > SPATIAL_NONPARAMETRIC)
    throw prg_error("Enumeration %d out of range [%d,%d].", "SetSpatialAdjustmentType()", eSpatialAdjustmentType, SPATIAL_NOTADJUSTED, SPATIAL_NONPARAMETRIC);
  geSpatialAdjustmentType = eSpatialAdjustmentType;
}

/** Set spatial window shape type. Throws exception if out of range. */
void  CParameters::SetSpatialWindowType(SpatialWindowType eSpatialWindowType) {
  if (eSpatialWindowType < CIRCULAR || eSpatialWindowType > ELLIPTIC)
    throw prg_error("Enumeration %d out of range [%d,%d].", "SetSpatialWindowType()", eSpatialWindowType, CIRCULAR, ELLIPTIC);
  geSpatialWindowType = eSpatialWindowType;
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

/** Sets temporal graph report type. Throws exception if out of range. */
void CParameters::setTemporalGraphReportType(TemporalGraphReportType e) {
  if (e < MLC_ONLY || e > SIGNIFICANT_ONLY)
    throw prg_error("Enumeration %d out of range [%d,%d].", "setTemporalGraphReportType()", e, MLC_ONLY, SIGNIFICANT_ONLY);
  _temporal_graph_report_type = e;
}

/** Sets network file purpose type. Throws exception if out of range. */
void CParameters::setNetworkFilePurpose(NetworkPurposeType e) {
	if (e < COORDINATES_OVERRIDE || e > NETWORK_DEFINITION)
		throw prg_error("Enumeration %d out of range [%d,%d].", "setTemporalGraphReportType()", e, COORDINATES_OVERRIDE, NETWORK_DEFINITION);
	_network_file_purpose = e;
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
  if (eTimeTrendAdjustmentType < TEMPORAL_NOTADJUSTED || eTimeTrendAdjustmentType > CALCULATED_QUADRATIC)
    throw prg_error("Enumeration %d out of range [%d,%d].", "SetTimeTrendAdjustmentType()",
                    eTimeTrendAdjustmentType, TEMPORAL_NOTADJUSTED, CALCULATED_QUADRATIC);
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

/** Sets user defined title. */
void CParameters::SetTitleName(const char * sTitleName) {
  gsTitleName = sTitleName;
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
        return !(UseLocationNeighborsFile() || (getUseLocationsNetworkFile() && GetCoordinatesFileName().size() == 0));
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

/** Returns indication of whether non-Euclidean neighbors file parameter is used. */
bool CParameters::UseLocationNeighborsFile() const {
    return !GetIsPurelyTemporalAnalysis() && 
            gbUseLocationNeighborsFile &&
            GetProbabilityModelType() != HOMOGENEOUSPOISSON;
}

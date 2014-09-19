//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ParametersPrint.h"
#include "DataSetHandler.h"
#include "SSException.h"
#include "ObservableRegion.h"
#include "ChartGenerator.h"

/** Returns analysis type as string. */
const char * ParametersPrint::GetAnalysisTypeAsString() const {
    const char * sAnalysisType;

    try {
        switch (gParameters.GetAnalysisType()) {
            case PURELYSPATIAL             : sAnalysisType = "Purely Spatial"; break;
            case PURELYTEMPORAL            : sAnalysisType = "Retrospective Purely Temporal"; break;
            case SPACETIME                 : sAnalysisType = "Retrospective Space-Time"; break;
            case PROSPECTIVESPACETIME      : sAnalysisType = "Prospective Space-Time"; break;
            case SPATIALVARTEMPTREND       : sAnalysisType = "Spatial Variation in Temporal Trends"; break;
            case PROSPECTIVEPURELYTEMPORAL : sAnalysisType = "Prospective Purely Temporal"; break;
            default : throw prg_error("Unknown analysis type '%d'.\n", "GetAnalysisTypeAsString()", gParameters.GetAnalysisType());
        }
    } catch (prg_exception& x) {
        x.addTrace("GetAnalysisTypeAsString()","ParametersPrint");
        throw;
    }
    return sAnalysisType;
}

/** Returns area scan type as string based upon probability model type. */
const char * ParametersPrint::GetAreaScanRateTypeAsString() const {
    try {
        switch (gParameters.GetProbabilityModelType()) {
            case POISSON :
                if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
                    switch (gParameters.GetAreaScanRateType()) {
                        case HIGH       : return "More Increasing or Less Decreasing Rates";
                        case LOW        : return "More Decreasing or Less Increasing Rates";
                        case HIGHANDLOW : return "Increasing or Decreasing Rates";
                        default : throw prg_error("Unknown area scan rate type '%d'.\n", "GetAreaScanRateTypeAsString()", gParameters.GetAreaScanRateType());
                    }
                }
            case HOMOGENEOUSPOISSON :
            case BERNOULLI :
            case SPACETIMEPERMUTATION :
                switch (gParameters.GetAreaScanRateType()) {
                    case HIGH       : return "High Rates";
                    case LOW        : return "Low Rates";
                    case HIGHANDLOW : return "High or Low Rates";
                    default : throw prg_error("Unknown area scan rate type '%d'.\n", "GetAreaScanRateTypeAsString()", gParameters.GetAreaScanRateType());
                }
            case CATEGORICAL : return "All Values";
            case ORDINAL :
            case NORMAL :
                switch (gParameters.GetAreaScanRateType()) {
                    case HIGH       : return "High Values";
                    case LOW        : return "Low Values";
                    case HIGHANDLOW : return "High or Low Values";
                    default : throw prg_error("Unknown area scan rate type '%d'.\n", "GetAreaScanRateTypeAsString()", gParameters.GetAreaScanRateType());
                }
            case EXPONENTIAL :
                switch (gParameters.GetAreaScanRateType()) {
                    case HIGH       : return "Short Survival";
                    case LOW        : return "Long Survival";
                    case HIGHANDLOW : return "Short or Long Survival";
                    default : throw prg_error("Unknown area scan rate type '%d'.\n", "GetAreaScanRateTypeAsString()", gParameters.GetAreaScanRateType());
                }
            default : throw prg_error("Unknown probability model '%d'.", "GetAreaScanRateTypeAsString()", gParameters.GetProbabilityModelType());
        }
    } catch (prg_exception& x) {
        x.addTrace("GetAreaScanRateTypeAsString()","ParametersPrint");
        throw;
    }
}

/** Returns probability model type as a character array. */
const char * ParametersPrint::GetProbabilityModelTypeAsString() const {
    const char * sProbabilityModel;

    try {
        switch (gParameters.GetProbabilityModelType()) {
            case POISSON              : sProbabilityModel = "Discrete Poisson"; break;
            case BERNOULLI            : sProbabilityModel = "Bernoulli"; break;
            case SPACETIMEPERMUTATION : sProbabilityModel = "Space-Time Permutation"; break;
            case CATEGORICAL          : sProbabilityModel = "Multinomial"; break;
            case ORDINAL              : sProbabilityModel = "Ordinal"; break;
            case EXPONENTIAL          : sProbabilityModel = "Exponential"; break;
            case NORMAL               : sProbabilityModel = "Normal"; break;
            case RANK                 : sProbabilityModel = "Rank"; break;
            case HOMOGENEOUSPOISSON   : sProbabilityModel = "Continuous Poisson"; break;
            default : throw prg_error("Unknown probability model type '%d'.\n", "GetProbabilityModelTypeAsString()", gParameters.GetProbabilityModelType());
        }
    } catch (prg_exception& x) {
        x.addTrace("GetProbabilityModelTypeAsString()","ParametersPrint");
        throw;
    }
    return sProbabilityModel;
}

const char * ParametersPrint::getPowerEvaluationMethodAsString() const {
    const char * sProbabilityModel;

    try {
        switch (gParameters.getPowerEvaluationMethod()) {
            case PE_WITH_ANALYSIS        : return "power evaluation with analysis";
            case PE_ONLY_CASEFILE        : return "only power evaluation using case file";
            case PE_ONLY_SPECIFIED_CASES : return "only power evaluation using specified total cases";
            default : throw prg_error("Unknown power evaluation method '%d'.\n", "getPowerEvaluationMethodAsString()", gParameters.getPowerEvaluationMethod());
        }
    } catch (prg_exception& x) {
        x.addTrace("getPowerEvaluationMethodAsString()","ParametersPrint");
        throw;
    }
    return sProbabilityModel;
}

/** Prints parameters, in a particular format, to passed ascii file. */
void ParametersPrint::Print(FILE* fp) const {
    try {
        AsciiPrintFormat::PrintSectionSeparatorString(fp, 0, 2);
        fprintf(fp, "PARAMETER SETTINGS\n");

        PrintInputParameters(fp);
        PrintAnalysisParameters(fp);
        PrintOutputParameters(fp);
        PrintPolygonParameters(fp);
        PrintMultipleDataSetParameters(fp);
        PrintDataCheckingParameters(fp);
        PrintSpatialNeighborsParameters(fp);
        PrintSpatialWindowParameters(fp);
        PrintTemporalWindowParameters(fp);
        PrintSpaceAndTimeAdjustmentsParameters(fp);
        PrintInferenceParameters(fp);
        PrintBorderAnalysisParameters(fp);
        PrintPowerEvaluationsParameters(fp);
        PrintSpatialOutputParameters(fp);
        PrintTemporalOutputParameters(fp);
        PrintOtherOutputParameters(fp);
        PrintEllipticScanParameters(fp);
        PrintPowerSimulationsParameters(fp);
        PrintRunOptionsParameters(fp);
        PrintSystemParameters(fp);
        AsciiPrintFormat::PrintSectionSeparatorString(fp, 0, 1);
    } catch (prg_exception& x) {
        x.addTrace("Print()","ParametersPrint");
        throw;
    }
}

/** Print parameters of 'Other Output' tab/section. */
void ParametersPrint::PrintOtherOutputParameters(FILE* fp) const {
    SettingContainer_t settings;

    try {
        if (!gParameters.getPerformPowerEvaluation() || (gParameters.getPerformPowerEvaluation() && gParameters.getPowerEvaluationMethod() == PE_WITH_ANALYSIS)) {
            settings.push_back(std::make_pair("Report Critical Values",(gParameters.GetReportCriticalValues() ? "Yes" : "No")));
            settings.push_back(std::make_pair("Report Monte Carlo Rank",(gParameters.getReportClusterRank() ? "Yes" : "No")));
        }
        if (gParameters.GetOutputAreaSpecificAscii() || gParameters.GetOutputClusterCaseAscii() ||
            gParameters.GetOutputClusterLevelAscii() || gParameters.GetOutputRelativeRisksAscii() || gParameters.GetOutputSimLoglikeliRatiosAscii())
            settings.push_back(std::make_pair("Print ASCII Column Headers",(gParameters.getPrintAsciiHeaders() ? "Yes" : "No")));
        if (gParameters.GetTitleName() != "")
            settings.push_back(std::make_pair("User Defined Title",gParameters.GetTitleName()));
        WriteSettingsContainer(settings, "Other Output", fp);
    } catch (prg_exception& x) {
        x.addTrace("PrintOtherOutputParameters()","ParametersPrint");
        throw;
    }
}

/** Print parameters of 'Border Analysis' tab/section. */
void ParametersPrint::PrintBorderAnalysisParameters(FILE* fp) const {
    SettingContainer_t settings;

    if (gParameters.GetProbabilityModelType() == POISSON && gParameters.GetAnalysisType() == PURELYSPATIAL) {
        settings.push_back(std::make_pair("Report Oliviera's F", (gParameters.getCalculateOlivierasF() ? "Yes" : "No")));
        if (gParameters.getCalculateOlivierasF()) {
            std::string buffer;
            settings.push_back(std::make_pair("Number of Oliviera Data Sets", printString(buffer, "%u", gParameters.getNumRequestedOlivieraSets())));
        }
        WriteSettingsContainer(settings, "Border Analysis", fp);
    }
}


/** Prints time trend adjustment parameters, in a particular format, to passed ascii file. */
void ParametersPrint::PrintAdjustments(FILE* fp, const DataSetHandler& SetHandler) const {
  std::string           buffer;
  AsciiPrintFormat      PrintFormat;

  try {
    //display temporal adjustments
    switch (gParameters.GetTimeTrendAdjustmentType()) {
      case NOTADJUSTED :
        break;
      case NONPARAMETRIC :
        buffer = "Adjusted for time nonparametrically."; break;
      case LOGLINEAR_PERC :
        printString(buffer, "of %g%% per year.", fabs(gParameters.GetTimeTrendAdjustmentPercentage()));
        if (gParameters.GetTimeTrendAdjustmentPercentage() < 0)
          buffer.insert(0, "Adjusted for time with a decrease ");
        else
          buffer.insert(0, "Adjusted for time with an increase ");
        break;
      case CALCULATED_LOGLINEAR_PERC :
        PrintCalculatedTimeTrend(fp, SetHandler); break;
      case STRATIFIED_RANDOMIZATION  :
        buffer = "Adjusted for time by stratified randomization."; break;
      default :
        throw prg_error("Unknown time trend adjustment type '%d'\n.",
                        "PrintAdjustments()", gParameters.GetTimeTrendAdjustmentType());
    }
    if (buffer.size())
      PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
    //display spatial adjustments
    switch (gParameters.GetSpatialAdjustmentType()) {
      case NO_SPATIAL_ADJUSTMENT :
        break;
      case SPATIALLY_STRATIFIED_RANDOMIZATION :
        buffer = "Adjusted for purely spatial clusters by stratified randomization.";
        PrintFormat.PrintAlignedMarginsDataString(fp, buffer); break;
      default :
        throw prg_error("Unknown time trend adjustment type '%d'\n.",
                        "PrintAdjustments()", gParameters.GetSpatialAdjustmentType());
    }
    //display space-time adjustments
    if (gParameters.UseAdjustmentForRelativeRisksFile()) {
        buffer = "Adjusted for known relative risks.";
        PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("PrintAdjustments()","ParametersPrint");
    throw;
  }
}

/** Prints 'Analysis' tab parameters to file stream. */
void ParametersPrint::PrintAnalysisParameters(FILE* fp) const {
    AnalysisType eAnalysisType = gParameters.GetAnalysisType();
    SettingContainer_t settings;
    std::string buffer;

    try {
        settings.push_back(std::make_pair("Type of Analysis",GetAnalysisTypeAsString()));
        settings.push_back(std::make_pair("Probability Model",GetProbabilityModelTypeAsString()));
        settings.push_back(std::make_pair("Scan for Areas with",GetAreaScanRateTypeAsString()));
        if (eAnalysisType != PURELYSPATIAL) {
            buffer = "Time Aggregation Units";
            switch (gParameters.GetTimeAggregationUnitsType()) {
                case YEAR  : settings.push_back(std::make_pair(buffer,"Year")); break;
                case MONTH : settings.push_back(std::make_pair(buffer,"Month")); break;
                case DAY   : settings.push_back(std::make_pair(buffer,"Day")); break;
                case GENERIC : settings.push_back(std::make_pair(buffer,"Generic"));  break;
                default : throw prg_error("Unknown date precision type '%d'.\n","PrintAnalysisParameters()", gParameters.GetTimeAggregationUnitsType());
            }
            printString(buffer, "%i", gParameters.GetTimeAggregationLength());
            settings.push_back(std::make_pair("Time Aggregation Length",buffer));
        }
        WriteSettingsContainer(settings, "Analysis", fp);
    } catch (prg_exception& x) {
        x.addTrace("PrintAnalysisParameters()","ParametersPrint");
        throw;
    }
}

/** Prints analysis type related information, in a particular format, to passed ascii file. */
void ParametersPrint::PrintAnalysisSummary(FILE* fp) const {
  try {
    switch (gParameters.GetAnalysisType()) {
      case PURELYSPATIAL             : fprintf(fp, "Purely Spatial"); break;
      case PURELYTEMPORAL            : fprintf(fp, "Retrospective Purely Temporal"); break;
      case SPACETIME                 : fprintf(fp, "Retrospective Space-Time"); break;
      case PROSPECTIVESPACETIME      : fprintf(fp, "Prospective Space-Time"); break;
      case SPATIALVARTEMPTREND       : fprintf(fp, "Spatial Variation in Temporal Trends"); break;
      case PROSPECTIVEPURELYTEMPORAL : fprintf(fp, "Prospective Purely Temporal"); break;
      default : throw prg_error("Unknown analysis type '%d'.\n","PrintAnalysisSummary()", gParameters.GetAnalysisType());
    }
    if (gParameters.getPerformPowerEvaluation() && gParameters.getPowerEvaluationMethod() == PE_WITH_ANALYSIS) {
        fprintf(fp, " analysis and power evaluation\n");
    } else {
        fprintf(fp, gParameters.getPerformPowerEvaluation() ? " power evaluation\n" : " analysis\n");
    }
    fprintf(fp, "scanning for ");
    if (gParameters.GetAnalysisType() == PURELYSPATIAL && gParameters.GetRiskType() == MONOTONERISK)
      fprintf(fp, "monotone ");
    std::string s(GetAreaScanRateTypeAsString());
    std::transform(s.begin(), s.end(), s.begin(), (int(*)(int)) tolower);
    fprintf(fp, "clusters with %s\n", s.c_str());
    switch (gParameters.GetProbabilityModelType()) {
      case POISSON              : fprintf(fp, "using the Discrete Poisson model.\n"); break;
      case BERNOULLI            : fprintf(fp, "using the Bernoulli model.\n"); break;
      case SPACETIMEPERMUTATION : fprintf(fp, "using the Space-Time Permutation model.\n"); break;
      case CATEGORICAL          : fprintf(fp, "using the Multinomial model.\n"); break;
      case ORDINAL              : fprintf(fp, "using the Ordinal model.\n"); break;
      case EXPONENTIAL          : fprintf(fp, "using the Exponential model.\n"); break;
      case NORMAL               : fprintf(fp, "using the Normal model.\n"); break;
      case RANK                 : fprintf(fp, "using the Rank model.\n"); break;
      case HOMOGENEOUSPOISSON   : fprintf(fp, "using the Continuous Poisson model.\n"); break;
      default : throw prg_error("Unknown probability model type '%d'.\n",
                                "PrintAnalysisSummary()", gParameters.GetProbabilityModelType());
    }

    if (gParameters.GetIsSpaceTimeAnalysis()) {
      if (gParameters.GetIncludePurelySpatialClusters() && gParameters.GetIncludePurelyTemporalClusters())
        fprintf(fp, "Analysis includes purely spatial and purely temporal clusters.\n");
      else if (gParameters.GetIncludePurelySpatialClusters())
        fprintf(fp, "Analysis includes purely spatial clusters.\n");
      else if (gParameters.GetIncludePurelyTemporalClusters())
        fprintf(fp, "Analysis includes purely temporal clusters.\n");
    }

    if (gParameters.GetNumDataSets() > 1) {
      switch (gParameters.GetMultipleDataSetPurposeType()) {
        case MULTIVARIATE : fprintf(fp, "Multivariate scan using %u data sets.\n", gParameters.GetNumDataSets()); break;
        case ADJUSTMENT   : fprintf(fp, "Adjusted using %u data sets.\n", gParameters.GetNumDataSets()); break;
        default : throw prg_error("Unknown purpose for multiple data sets type '%d'.\n",
                                  "PrintAnalysisSummary()", gParameters.GetMultipleDataSetPurposeType());
      }
    }
    if (gParameters.GetIsIterativeScanning() && gParameters.GetProbabilityModelType() != HOMOGENEOUSPOISSON)
      fprintf(fp, "Iterative analysis performed.\n");
  }
  catch (prg_exception& x) {
    x.addTrace("PrintAnalysisSummary()","ParametersPrint");
    throw;
  }
}

/** Prints calculated time trend adjustment parameters, in a particular format, to passed ascii file. */
void ParametersPrint::PrintCalculatedTimeTrend(FILE* fp, const DataSetHandler& SetHandler) const {
  unsigned int                  t;
  std::string                   buffer, work;
  std::deque<unsigned int>      TrendIncrease, TrendDecrease;

  if (gParameters.GetTimeTrendAdjustmentType() != CALCULATED_LOGLINEAR_PERC)
    return;

  //NOTE: Each dataset has own calculated time trend.

  if (SetHandler.GetNumDataSets() == 1) {
    if (SetHandler.GetDataSet(0).getCalculatedTimeTrendPercentage() < 0)
      printString(buffer, "Adjusted for time trend with an annual decrease ");
    else
      printString(buffer, "Adjusted for time trend with an annual increase ");
    printString(work, "of %g%%.", fabs(SetHandler.GetDataSet(0).getCalculatedTimeTrendPercentage()));
    buffer += work;
  }
  else {//multiple datasets print
    //count number of increasing and decreasing trends
    for (t=0; t < SetHandler.GetNumDataSets(); ++t) {
       if (SetHandler.GetDataSet(t).getCalculatedTimeTrendPercentage() < 0)
         TrendDecrease.push_back(t);
       else
         TrendIncrease.push_back(t);
    }
    //now print
    buffer = "Adjusted for time trend with an annual ";
    //print increasing trends first
    if (TrendIncrease.size()) {
       printString(work, "increase of %0.2f%%",
                         fabs(SetHandler.GetDataSet(TrendIncrease.front()).getCalculatedTimeTrendPercentage()));
       buffer += work;
       for (t=1; t < TrendIncrease.size(); ++t) {
          printString(work, (t < TrendIncrease.size() - 1) ? ", %g%%" : " and %g%%",
                            fabs(SetHandler.GetDataSet(TrendIncrease[t]).getCalculatedTimeTrendPercentage()));
          buffer += work;
       }
       printString(work, " for data set%s %u", (TrendIncrease.size() == 1 ? "" : "s"), TrendIncrease.front() + 1);
       buffer += work;
       for (t=1; t < TrendIncrease.size(); ++t) {
          printString(work, (t < TrendIncrease.size() - 1 ? ", %u" : " and %u"), TrendIncrease[t] + 1);
          buffer += work;
       }
       printString(work, (TrendIncrease.size() > 1 ? " respectively" : ""));
       buffer += work;
       printString(work, (TrendDecrease.size() > 0 ? " and an annual " : "."));
       buffer += work;
    }
    //print decreasing trends
    if (TrendDecrease.size()) {
      printString(work, "decrease of %0.2f%%",
                        fabs(SetHandler.GetDataSet(TrendDecrease.front()).getCalculatedTimeTrendPercentage()));
      buffer += work;
      for (t=1; t < TrendDecrease.size(); ++t) {
         printString(work, (t < TrendDecrease.size() - 1) ? ", %g%%" : " and %0.2f%%",
                           fabs(SetHandler.GetDataSet(TrendDecrease[t]).getCalculatedTimeTrendPercentage()));
         buffer += work;
      }
      printString(work, " for data set%s %u", (TrendDecrease.size() == 1 ? "" : "s"), TrendDecrease.front() + 1);
      buffer += work;
      for (t=1; t < TrendDecrease.size(); ++t) {
         printString(work, (t < TrendDecrease.size() - 1 ? ", %u" : " and %u"), TrendDecrease[t] + 1);
         buffer += work;
      }
      printString(work, (TrendDecrease.size() > 1 ? " respectively." : "."));
      buffer += work;
    }
  }
  AsciiPrintFormat PrintFormat;
  PrintFormat.SetMarginsAsOverviewSection();
  PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
}

/** Prints 'Spatial Output' tab parameters to file stream. */
void ParametersPrint::PrintSpatialOutputParameters(FILE* fp) const {
    SettingContainer_t settings;
    std::string buffer, worker;

    try {
        // skip these settings for purely temporal analysis
        if (gParameters.GetIsPurelyTemporalAnalysis()) return;

        if (gParameters.GetCoordinatesType() == LATLON && gParameters.getOutputKMLFile()) {
            settings.push_back(std::make_pair("Automatically Launch Google Earth",(gParameters.getLaunchKMLViewer() ? "Yes" : "No")));
            settings.push_back(std::make_pair("Compress KML File into KMZ File",(gParameters.getCompressClusterKML() ? "Yes" : "No")));
            settings.push_back(std::make_pair("Include All Location IDs in the Clusters",(gParameters.getIncludeLocationsKML() ? "Yes" : "No")));
            printString(buffer, "%u", gParameters.getLocationsThresholdKML());
            settings.push_back(std::make_pair("Cluster Location Threshold - Separate KML",buffer));
        }

        // skip these settings when performing power evaluations without running an analysis
        if ((gParameters.getPerformPowerEvaluation() && gParameters.getPowerEvaluationMethod() != PE_WITH_ANALYSIS)) return;

        settings.push_back(std::make_pair("Report Hierarchical Clusters", (gParameters.getReportHierarchicalClusters() ? "Yes" : "No")));
        if (gParameters.getReportHierarchicalClusters()) {
            buffer = "Criteria for Reporting Secondary Clusters";
            switch (gParameters.GetCriteriaSecondClustersType()) {
                case NOGEOOVERLAP          :
                    settings.push_back(std::make_pair(buffer,"No Geographical Overlap")); break;
                case NOCENTROIDSINOTHER    :
                    settings.push_back(std::make_pair(buffer,"No Cluster Centroids in Other Clusters")); break;
                case NOCENTROIDSINMORELIKE :
                    settings.push_back(std::make_pair(buffer,"No Cluster Centroids in More Likely Clusters")); break;
                case NOCENTROIDSINLESSLIKE :
                    settings.push_back(std::make_pair(buffer,"No Cluster Centroids in Less Likely Clusters")); break;
                case NOPAIRSINEACHOTHERS   :
                    settings.push_back(std::make_pair(buffer,"No Pairs of Centroids Both in Each Others Clusters")); break;
                case NORESTRICTIONS        :
                    settings.push_back(std::make_pair(buffer,"No Restrictions = Most Likely Cluster for Each Centroid")); break;
                default : throw prg_error("Unknown secondary clusters type '%d'.\n", "PrintSpatialOutputParameters()", gParameters.GetCriteriaSecondClustersType());
            }
        }
        if (gParameters.GetAnalysisType() == PURELYSPATIAL) {
            settings.push_back(std::make_pair("Report Gini Optimized Cluster Collection", (gParameters.getReportGiniOptimizedClusters() ? "Yes" : "No")));
            if (gParameters.getReportGiniOptimizedClusters()) {
                buffer = "Gini Index Based Collection Reporting";
                switch (gParameters.getGiniIndexReportType()) {
                    case OPTIMAL_ONLY : settings.push_back(std::make_pair(buffer,"Optimal Only")); break;
                    case ALL_VALUES    : settings.push_back(std::make_pair(buffer,"All Values")); break;
                    default : throw prg_error("Unknown index based cluster reporting type '%d'.\n","PrintSpatialOutputParameters()", gParameters.getGiniIndexReportType());
                }
                settings.push_back(std::make_pair("Report Gini Index Cluster Coefficents", (gParameters.getReportGiniIndexCoefficents() ? "Yes" : "No")));
                printString(buffer, "%g", gParameters.getExecuteSpatialWindowStops()[0]);
                for (size_t i=1; i < gParameters.getExecuteSpatialWindowStops().size(); ++i) {
                    printString(worker, ", %g", gParameters.getExecuteSpatialWindowStops()[i]);
                    buffer += worker;
                }
                settings.push_back(std::make_pair("Spatial Cluster Maxima",buffer));
            }
        }
        settings.push_back(std::make_pair("Restrict Reporting to Smaller Clusters", (gParameters.GetRestrictingMaximumReportedGeoClusterSize() ? "Yes" : "No")));
        if (gParameters.GetRestrictingMaximumReportedGeoClusterSize()) {
            if (!(gParameters.GetAnalysisType() == PROSPECTIVESPACETIME && gParameters.GetAdjustForEarlierAnalyses())) {
                printString(buffer, "Only clusters smaller than %g percent of population at risk reported.", gParameters.GetMaxSpatialSizeForType(PERCENTOFPOPULATION, true));
                settings.push_back(std::make_pair("Reported Clusters",buffer));
            }
            if (gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, true)) {
                printString(buffer, "Only clusters smaller than %g percent of population defined in max circle file reported.", gParameters.GetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, true));
                settings.push_back(std::make_pair("Reported Clusters",buffer));
            }
            if (gParameters.GetRestrictMaxSpatialSizeForType(MAXDISTANCE, true)) {
                printString(buffer, "Only clusters smaller than %g%s reported.",
                            gParameters.GetMaxSpatialSizeForType(MAXDISTANCE, true), (gParameters.GetCoordinatesType() == CARTESIAN ? " Cartesian units" : " km"));
                settings.push_back(std::make_pair("Reported Clusters",buffer));
            }
        }
        WriteSettingsContainer(settings, "Spatial Output", fp);
    } catch (prg_exception& x) {
        x.addTrace("PrintSpatialOutputParameters()","ParametersPrint");
        throw;
    }
}

/** Prints 'Data Checking' parameters to file stream. */
void ParametersPrint::PrintDataCheckingParameters(FILE* fp) const {
    SettingContainer_t settings;
    std::string buffer;

    try {
        if (gParameters.GetProbabilityModelType() != HOMOGENEOUSPOISSON) {
            buffer = "Temporal Data Check";
            switch (gParameters.GetStudyPeriodDataCheckingType()) {
                case STRICTBOUNDS     :
                    settings.push_back(std::make_pair(buffer,"Check to ensure that all cases and controls are within the specified temporal study period.")); 
                    break;
                case RELAXEDBOUNDS    : settings.push_back(std::make_pair(buffer,"Ignore cases and controls that are outside the specified temporal study period.")); 
                    break;
                default : throw prg_error("Unknown study period check type '%d'.\n", "PrintDataCheckingParameters()", gParameters.GetStudyPeriodDataCheckingType());
            }
        }
        if (!gParameters.GetIsPurelyTemporalAnalysis()) {
            buffer = "Geographical Data Check";
            switch (gParameters.GetCoordinatesDataCheckingType()) {
                case STRICTCOORDINATES  :
                    settings.push_back(std::make_pair(buffer,"Check to ensure that all observations (cases, controls and populations) are within the specified geographical area."));
                    break;
                case RELAXEDCOORDINATES :
                    settings.push_back(std::make_pair(buffer,"Ignore observations that are outside the specified geographical area."));
                    break;
                default : throw prg_error("Unknown geographical coordinates check type '%d'.\n", "PrintDataCheckingParameters()", gParameters.GetCoordinatesDataCheckingType());
            }
        }
        WriteSettingsContainer(settings, "Data Checking", fp);
    } catch (prg_exception& x) {
        x.addTrace("PrintDataCheckingParameters()","ParametersPrint");
        throw;
    }
}

/** Prints 'Elliptic Scan' parameters to file stream. */
void ParametersPrint::PrintEllipticScanParameters(FILE* fp) const {
    SettingContainer_t settings;
    std::string buffer, worker;

    try {
        if (!gParameters.GetIsPurelyTemporalAnalysis() && !gParameters.UseLocationNeighborsFile() && gParameters.GetSpatialWindowType() == ELLIPTIC) {
            printString(buffer, "%g", gParameters.GetEllipseShapes()[0]);
            for (size_t i=1; i < gParameters.GetEllipseShapes().size(); ++i) {
                printString(worker, ", %g", gParameters.GetEllipseShapes()[i]);
                buffer += worker;
            }
            settings.push_back(std::make_pair("Ellipse Shapes",buffer));
            printString(buffer, "%i", gParameters.GetEllipseRotations()[0]);
            for (size_t i=1; i < gParameters.GetEllipseRotations().size(); ++i) {
                printString(worker, ", %i", gParameters.GetEllipseRotations()[i]);
                buffer += worker;
            }
            settings.push_back(std::make_pair("Number of Angles for Each Ellipse Shape",buffer));
            WriteSettingsContainer(settings, "Elliptic Scan", fp);
        }
    } catch (prg_exception& x) {
        x.addTrace("PrintEllipticScanParameters()","ParametersPrint");
        throw;
    }
}

/** Prints 'Inference' tab parameters to file stream. */
void ParametersPrint::PrintInferenceParameters(FILE* fp) const {
    SettingContainer_t settings;
    std::string buffer;

    try {
        buffer = "P-Value Reporting";
        switch (gParameters.GetPValueReportingType()) {
            case DEFAULT_PVALUE :
                settings.push_back(std::make_pair(buffer,"Default Combination"));
                break;
            case STANDARD_PVALUE :
                settings.push_back(std::make_pair(buffer,"Standard Monte Carlo"));
                settings.push_back(std::make_pair("Report Gumbel Based P-Values",(gParameters.GetReportGumbelPValue() ? "Yes" : "No")));
                break;
            case TERMINATION_PVALUE :
                settings.push_back(std::make_pair(buffer,"Sequential Monte Carlo Early Termination"));
                printString(buffer, "%u", gParameters.GetEarlyTermThreshold());
                settings.push_back(std::make_pair("Termination Cutoff",buffer));
                settings.push_back(std::make_pair("Report Gumbel Based P-Values",(gParameters.GetReportGumbelPValue() ? "Yes" : "No")));
                break;
            case GUMBEL_PVALUE :
                settings.push_back(std::make_pair(buffer,"Gumbel Approximation"));
                break;
        }
        printString(buffer, "%u", gParameters.GetNumReplicationsRequested());
        settings.push_back(std::make_pair("Number of Replications",buffer));
        if (gParameters.GetIsProspectiveAnalysis()) {
            settings.push_back(std::make_pair("Adjusted for Earlier Analyses",(gParameters.GetAdjustForEarlierAnalyses() ? "Yes" : "No")));
            if (gParameters.GetAdjustForEarlierAnalyses())
                settings.push_back(std::make_pair("Prospective Start Time",gParameters.GetProspectiveStartDate()));
        }
        if (gParameters.GetProbabilityModelType() != HOMOGENEOUSPOISSON && !gParameters.getPerformPowerEvaluation()) {
            settings.push_back(std::make_pair("Adjusting for More Likely Clusters",(gParameters.GetIsIterativeScanning() ? "Yes" : "No")));
            if (gParameters.GetIsIterativeScanning()) {
                printString(buffer, "%u", gParameters.GetNumIterativeScansRequested());
                settings.push_back(std::make_pair("Maximum number of iterations",buffer));
                printString(buffer, "%g", gParameters.GetIterativeCutOffPValue());
                settings.push_back(std::make_pair("Stop when p-value greater",buffer));
            }
        }
        WriteSettingsContainer(settings, "Inference", fp);
    } catch (prg_exception& x) {
        x.addTrace("PrintInferenceParameters()","ParametersPrint");
        throw;
    }
}

/** Prints 'Input' tab parameters to file stream. */
void ParametersPrint::PrintInputParameters(FILE* fp) const {
    DatePrecisionType ePrecision;
    const char * sDataSetLabel = (gParameters.GetNumDataSets() == 1 ? "" : " (data set 1)");
    const char * sBlankDataSetLabel = (gParameters.GetNumDataSets() == 1 ? "" : "            ");
    SettingContainer_t settings;
    std::string buffer;

    try {
        if (gParameters.GetProbabilityModelType() != HOMOGENEOUSPOISSON ||
            (gParameters.getPerformPowerEvaluation() && gParameters.getPowerEvaluationMethod() == PE_ONLY_SPECIFIED_CASES)) {
            settings.push_back(std::make_pair("Case File",gParameters.GetCaseFileName(1)));
            settings.back().first += sDataSetLabel;
        }
        switch (gParameters.GetProbabilityModelType()) {
            case POISSON :
                if (!gParameters.UsePopulationFile()) break;
                settings.push_back(std::make_pair("Population File",gParameters.GetPopulationFileName(1)));
                settings.back().first += sDataSetLabel; break;
            case BERNOULLI :
                settings.push_back(std::make_pair("Control File",gParameters.GetControlFileName(1)));
                settings.back().first += sDataSetLabel; break;
            case SPACETIMEPERMUTATION :
            case CATEGORICAL          :
            case ORDINAL              :
            case EXPONENTIAL          :
            case NORMAL               :
            case RANK                 :
            case HOMOGENEOUSPOISSON   :  break;
            default : 
                throw prg_error("Unknown probability model type '%d'.\n", "PrintInputParameters()", gParameters.GetProbabilityModelType());
        }
        if (gParameters.GetProbabilityModelType() != HOMOGENEOUSPOISSON) {
            //Display precision, keeping in mind the v4 behavior.
            if (gParameters.GetPrecisionOfTimesType() == NONE)
                ePrecision = NONE;
            else if (gParameters.GetCreationVersion().iMajor == 4)
                ePrecision = (gParameters.GetAnalysisType() == PURELYSPATIAL ? YEAR : gParameters.GetTimeAggregationUnitsType());
            else
                ePrecision =  gParameters.GetPrecisionOfTimesType();
            buffer = "Time Precision";
            switch (ePrecision) {
                case YEAR    : settings.push_back(std::make_pair(buffer,"Year")); break;
                case MONTH   : settings.push_back(std::make_pair(buffer,"Month")); break;
                case DAY     : settings.push_back(std::make_pair(buffer,"Day")); break;
                case GENERIC : settings.push_back(std::make_pair(buffer,"Generic")); break;
                default      : settings.push_back(std::make_pair(buffer,"None")); break;
            }
            settings.push_back(std::make_pair("Start Time",gParameters.GetStudyPeriodStartDate()));
            settings.push_back(std::make_pair("End Time",gParameters.GetStudyPeriodEndDate()));
        }
        if (gParameters.UseCoordinatesFile())
            settings.push_back(std::make_pair("Coordinates File",gParameters.GetCoordinatesFileName()));
        if (gParameters.UseSpecialGrid())
            settings.push_back(std::make_pair("Grid File",gParameters.GetSpecialGridFileName()));
        if (gParameters.GetSimulationType() == FILESOURCE)
            settings.push_back(std::make_pair("Simulated Data Import File",gParameters.GetSimulationDataSourceFilename()));
        if ((gParameters.UseCoordinatesFile() || gParameters.UseSpecialGrid())) {
            buffer = "Coordinates";
            switch (gParameters.GetCoordinatesType()) {
                case CARTESIAN : settings.push_back(std::make_pair(buffer,"Cartesian")); break;
                case LATLON    : settings.push_back(std::make_pair(buffer,"Latitude/Longitude")); break;
                default : throw prg_error("Unknown coordinated type '%d'.\n", "PrintInputParameters()", gParameters.GetCoordinatesType());
            }
        }
        WriteSettingsContainer(settings, "Input", fp);
    } catch (prg_exception& x) {
        x.addTrace("PrintInputParameters()","ParametersPrint");
        throw;
    }
}

/** Prints 'Multiple Data Set' tab parameters to file stream. */
void ParametersPrint::PrintMultipleDataSetParameters(FILE* fp) const {
    SettingContainer_t settings;
    std::string buffer;

    try {
        if (gParameters.GetNumDataSets() == 1) return;
        for (unsigned int t=1; t < gParameters.GetNumDataSets(); ++t) {
            printString(buffer, "Case File (data set %i)", t + 1);
            settings.push_back(std::make_pair(buffer,gParameters.GetCaseFileName(t + 1)));
            switch (gParameters.GetProbabilityModelType()) {
                case POISSON :
                    if (!gParameters.UsePopulationFile()) break;
                    printString(buffer, "Population File (data set %i)", t + 1);
                    settings.push_back(std::make_pair(buffer,gParameters.GetPopulationFileName(t + 1))); break;
                case BERNOULLI :
                    printString(buffer, "Control File (data set %i)", t + 1);
                    settings.push_back(std::make_pair(buffer,gParameters.GetControlFileName(t + 1))); break;
                case SPACETIMEPERMUTATION :
                case CATEGORICAL          :
                case ORDINAL              :
                case EXPONENTIAL          :
                case NORMAL               :
                case RANK                 :
                case HOMOGENEOUSPOISSON   : break;
                default : 
                    throw prg_error("Unknown probability model type '%d'.\n", "PrintMultipleDataSetParameters()", gParameters.GetProbabilityModelType());
            }
        }
        switch (gParameters.GetMultipleDataSetPurposeType()) {
            case MULTIVARIATE : settings.push_back(std::make_pair("Purpose of Multiple Data Sets","Multivariate Analysis")); break;
            case ADJUSTMENT    : settings.push_back(std::make_pair("Purpose of Multiple Data Sets","Adjustment")); break;
            default : throw prg_error("Unknown purpose for multiple data sets type '%d'.\n", "PrintMultipleDataSetParameters()", gParameters.GetMultipleDataSetPurposeType());
        }
        WriteSettingsContainer(settings, "Multiple Data Sets", fp);
    } catch (prg_exception& x) {
        x.addTrace("PrintMultipleDataSetParameters()","ParametersPrint");
        throw;
    }
}

/** Prints 'Output' tab parameters to file stream. */
void ParametersPrint::PrintOutputParameters(FILE* fp) const {
    FileName AdditionalOutputFile(gParameters.GetOutputFileName().c_str());
    SettingContainer_t settings;
    std::string buffer;
    bool canReportClusterFiles = (!gParameters.getPerformPowerEvaluation() || (gParameters.getPerformPowerEvaluation() && gParameters.getPowerEvaluationMethod() == PE_WITH_ANALYSIS));

    try {
        settings.push_back(std::make_pair("Results File",gParameters.GetOutputFileName()));
        if (canReportClusterFiles && gParameters.GetOutputClusterLevelAscii()) {
            AdditionalOutputFile.setExtension(".col.txt");
            settings.push_back(std::make_pair("Cluster File",AdditionalOutputFile.getFullPath(buffer)));
        }
        if (gParameters.GetCoordinatesType() == LATLON && gParameters.getOutputKMLFile()) {
            AdditionalOutputFile.setExtension(gParameters.getCompressClusterKML() ? ".kmz" : ".kml");
            settings.push_back(std::make_pair("Google Earth File",AdditionalOutputFile.getFullPath(buffer)));
        }
        if (gParameters.GetCoordinatesType() == LATLON && gParameters.getOutputShapeFiles()) {
            AdditionalOutputFile.setExtension(".col.shp");
            settings.push_back(std::make_pair("Shapefile",AdditionalOutputFile.getFullPath(buffer)));
        }
        if (canReportClusterFiles && gParameters.GetOutputClusterLevelDBase()) {
            AdditionalOutputFile.setExtension(".col.dbf");
            settings.push_back(std::make_pair("Cluster File",AdditionalOutputFile.getFullPath(buffer)));
        }
        // cluster case information files
        if (canReportClusterFiles && gParameters.GetOutputClusterCaseAscii()) {
            AdditionalOutputFile.setExtension(".sci.txt");
            settings.push_back(std::make_pair("Stratified Cluster File",AdditionalOutputFile.getFullPath(buffer)));
        }
        if (canReportClusterFiles && gParameters.GetOutputClusterCaseDBase()) {
            AdditionalOutputFile.setExtension(".sci.dbf");
            settings.push_back(std::make_pair("Stratified Cluster File",AdditionalOutputFile.getFullPath(buffer)));
        }
        // area specific files
        if (canReportClusterFiles && gParameters.GetOutputAreaSpecificAscii()) {
            AdditionalOutputFile.setExtension(".gis.txt");
            settings.push_back(std::make_pair("Location File",AdditionalOutputFile.getFullPath(buffer)));
        }
        if (canReportClusterFiles && gParameters.GetOutputAreaSpecificDBase()) {
            AdditionalOutputFile.setExtension(".gis.dbf");
            settings.push_back(std::make_pair("Location File",AdditionalOutputFile.getFullPath(buffer)));
        }
        // relative risk files
        if (gParameters.GetOutputRelativeRisksAscii()) {
            AdditionalOutputFile.setExtension(".rr.txt");
            settings.push_back(std::make_pair("Relative Risks File",AdditionalOutputFile.getFullPath(buffer)));
        }
        if (gParameters.GetOutputRelativeRisksDBase()) {
            AdditionalOutputFile.setExtension(".rr.dbf");
            settings.push_back(std::make_pair("Relative Risks File",AdditionalOutputFile.getFullPath(buffer)));
        }
        // loglikelihood ratio files
        if (gParameters.GetOutputSimLoglikeliRatiosAscii()) {
            AdditionalOutputFile.setExtension(".llr.txt");
            settings.push_back(std::make_pair("Simulated LLRs File",AdditionalOutputFile.getFullPath(buffer)));
        }
        if (gParameters.GetOutputSimLoglikeliRatiosDBase()) {
            AdditionalOutputFile.setExtension(".llr.dbf");
            settings.push_back(std::make_pair("Simulated LLRs File",AdditionalOutputFile.getFullPath(buffer)));
        }
        if (gParameters.getOutputTemporalGraphFile() && (gParameters.GetProbabilityModelType() == POISSON || gParameters.GetProbabilityModelType() == BERNOULLI)) {
            AdditionalOutputFile.setFullPath(gParameters.GetOutputFileName().c_str());
            TemporalChartGenerator::getFilename(AdditionalOutputFile);
            settings.push_back(std::make_pair("Temporal Graph File",AdditionalOutputFile.getFullPath(buffer)));
        }
        WriteSettingsContainer(settings, "Output", fp);
    } catch (prg_exception& x) {
        x.addTrace("PrintOutputParameters()","ParametersPrint");
        throw;
    }
}

/** Prints 'Power Simulations' parameters to file stream. */
void ParametersPrint::PrintPowerEvaluationsParameters(FILE* fp) const {
    SettingContainer_t settings;
    std::string buffer;

    try {
        if (gParameters.GetProbabilityModelType() == POISSON && gParameters.GetAnalysisType() != SPATIALVARTEMPTREND) {
            settings.push_back(std::make_pair("Perform Power Evaluations", (gParameters.getPerformPowerEvaluation() ? "Yes" : "No")));
            if (!gParameters.getPerformPowerEvaluation()) return;
            buffer = "Power Method";
            switch (gParameters.getPowerEvaluationMethod()) {
                case PE_WITH_ANALYSIS: 
                    settings.push_back(std::make_pair(buffer,"Standard Analysis and Power Evaluation Together")); break;
                case PE_ONLY_CASEFILE: 
                    settings.push_back(std::make_pair(buffer,"Only Power Evaluation, Using Total Cases from Case File")); break;
                case PE_ONLY_SPECIFIED_CASES: 
                    settings.push_back(std::make_pair(buffer,"Only Power Evaluation, Using Defined Total Cases")); 
                    printString(buffer, "%i", gParameters.getPowerEvaluationCaseCount());
                    settings.push_back(std::make_pair("Power Evaluation Total Cases",buffer)); 
                    break;
                default: throw prg_error("Unknown power evaluation method type '%d'.\n", "PrintPowerEvaluationsParameters()", gParameters.getPowerEvaluationMethod());
            }
            buffer = "Critical Values";
            switch (gParameters.getPowerEvaluationCriticalValueType()) {
                case CV_MONTECARLO: 
                    settings.push_back(std::make_pair(buffer,"Monte Carlo")); break;
                case CV_GUMBEL: 
                    settings.push_back(std::make_pair(buffer,"Gumbel")); break;
                case CV_POWER_VALUES: 
                    settings.push_back(std::make_pair(buffer,"User Defined"));
                    printString(buffer, "%lf", gParameters.getPowerEvaluationCriticalValue05());
                    settings.push_back(std::make_pair("Critical Value .05",buffer));
                    printString(buffer, "%lf", gParameters.getPowerEvaluationCriticalValue01());
                    settings.push_back(std::make_pair("Critical Value .01",buffer));
                    printString(buffer, "%lf", gParameters.getPowerEvaluationCriticalValue001());
                    settings.push_back(std::make_pair("Critical Value .001",buffer));
                    break;
                default: throw prg_error("Unknown critical values type '%d'.\n", "PrintPowerEvaluationsParameters()", gParameters.getPowerEvaluationCriticalValueType());
            }
            buffer = "Power Estimation";
            switch (gParameters.getPowerEstimationType()) {
                case CV_MONTECARLO: 
                    settings.push_back(std::make_pair(buffer,"Monte Carlo")); break;
                case CV_GUMBEL: 
                    settings.push_back(std::make_pair(buffer,"Gumbel")); break;
                default: throw prg_error("Unknown critical values type '%d'.\n", "PrintPowerEvaluationsParameters()", gParameters.getPowerEstimationType());
            }
            printString(buffer, "%u", gParameters.getNumPowerEvalReplicaPowerStep());
            settings.push_back(std::make_pair("Number of Replications",buffer));
            switch (gParameters.GetPowerEvaluationSimulationType()) {
                case STANDARD         : 
                    settings.push_back(std::make_pair("Alternative Hypothesis File",gParameters.getPowerEvaluationAltHypothesisFilename()));
                    break;
                case FILESOURCE       :
                    settings.push_back(std::make_pair("Power Step Randomization Method","File Source"));
                    settings.push_back(std::make_pair("Randomization Source File",gParameters.getPowerEvaluationSimulationDataSourceFilename())); break;
                case HA_RANDOMIZATION :
                default : throw prg_error("Unknown simulation type '%d'.\n", "PrintPowerEvaluationsParameters()", gParameters.GetPowerEvaluationSimulationType());
            }
            // Since reporting the power evaluations' simulation data is not settable in the gui, only report if toggled on.
            if (gParameters.getOutputPowerEvaluationSimulationData()) {
                settings.push_back(std::make_pair("Output Power Step Simulation Data","Yes"));
                if (gParameters.getOutputPowerEvaluationSimulationData()) {
                    settings.push_back(std::make_pair("Power Step Simulation Data Filename", gParameters.getPowerEvaluationSimulationDataOutputFilename()));
                }
            }
        }
        WriteSettingsContainer(settings, "Power Evaluation", fp);
    } catch (prg_exception& x) {
        x.addTrace("PrintPowerEvaluationsParameters()","ParametersPrint");
        throw;
    }
}

/** Prints 'Power Simulations' parameters to file stream. */
void ParametersPrint::PrintPowerSimulationsParameters(FILE* fp) const {
    SettingContainer_t settings;
    std::string buffer;

    try {
        buffer = "Simulation Method";
        switch (gParameters.GetSimulationType()) {
            case STANDARD         : break;
            case FILESOURCE       :
                settings.push_back(std::make_pair(buffer,"File Source")); break;
                settings.push_back(std::make_pair("Randomization File",gParameters.GetSimulationDataSourceFilename())); break;
                break;
            case HA_RANDOMIZATION :
            default : throw prg_error("Unknown simulation type '%d'.\n", "PrintPowerSimulationsParameters()", gParameters.GetSimulationType());
        };
        if (gParameters.GetOutputSimulationData()) {
            settings.push_back(std::make_pair("Output Simulation Data","Yes"));
            settings.push_back(std::make_pair("Simulation Data Output",gParameters.GetSimulationDataOutputFilename()));
        }
        WriteSettingsContainer(settings, "Power Simulations", fp);
    } catch (prg_exception& x) {
        x.addTrace("PrintPowerSimulationsParameters()","ParametersPrint");
        throw;
    }
}

/** Prints polygon paramters to file stream. */
void ParametersPrint::PrintPolygonParameters(FILE* fp) const {
    SettingContainer_t settings;
    std::string buffer, worker;

    try {
        if (gParameters.GetProbabilityModelType() == HOMOGENEOUSPOISSON) {
            for (size_t t=0; t < gParameters.getObservableRegions().size(); ++t) {
                printString(buffer, "Polygon %i", t + 1);
                settings.push_back(std::make_pair(buffer,""));
                InequalityContainer_t list = ConvexPolygonBuilder::parse(gParameters.getObservableRegions().at(t));
                for (size_t y=0; y < list.size(); ++y) {
                    printString(worker, "%s%s", list.at(y).toString().c_str(), (y < list.size() - 1 ? ", " : ""));
                    settings.back().second += worker;
                }
            }
            WriteSettingsContainer(settings, "Polygons", fp);
        }
    } catch (prg_exception& x) {
        x.addTrace("PrintPolygonParameters()","ParametersPrint");
        throw;
    }
}

/** Prints 'Run Options' parameters to file stream. */
void ParametersPrint::PrintRunOptionsParameters(FILE* fp) const {
    SettingContainer_t settings;
    std::string buffer;

    try {
        if (gParameters.GetNumRequestedParallelProcesses() == 0)
            settings.push_back(std::make_pair("Processer Usage","All Available Proccessors"));
        else {
            printString(buffer, "At Most %u Proccessors", gParameters.GetNumRequestedParallelProcesses());
            settings.push_back(std::make_pair("Processer Usage",buffer));
        }
        settings.push_back(std::make_pair("Suppress Warnings", (gParameters.GetSuppressingWarnings() ? "Yes" : "No")));
        settings.push_back(std::make_pair("Logging Analysis", (gParameters.GetIsLoggingHistory() ? "Yes" : "No")));
        if (gParameters.GetIsRandomlyGeneratingSeed())
            settings.push_back(std::make_pair("Use Random Seed",(gParameters.GetIsRandomlyGeneratingSeed() ? "Yes" : "No")));
        if (gParameters.GetRandomizationSeed() != RandomNumberGenerator::glDefaultSeed) {
            printString(buffer, "%ld\n", gParameters.GetRandomizationSeed());
            settings.push_back(std::make_pair("Randomization Seed",buffer));
        }
        if (gParameters.GetExecutionType() != AUTOMATIC) {
            buffer = "Execution Type";
            switch (gParameters.GetExecutionType()) {
                case AUTOMATIC    : settings.push_back(std::make_pair(buffer,"Automatic Determination")); break;
                case SUCCESSIVELY : settings.push_back(std::make_pair(buffer,"Successively")); break;
                case CENTRICALLY  : settings.push_back(std::make_pair(buffer,"Centrically")); break;
                default : throw prg_error("Unknown execution type '%d'.\n",
                                          "PrintRunOptionsParameters()", gParameters.GetExecutionType());
            }
        }
        WriteSettingsContainer(settings, "Run Options", fp);
    } catch (prg_exception& x) {
        x.addTrace("PrintRunOptionsParameters()","ParametersPrint");
        throw;
    }
}

/** Prints 'Spatial Neighbors' tab parameters to file stream. */
void ParametersPrint::PrintSpatialNeighborsParameters(FILE* fp) const {
    SettingContainer_t settings;
    std::string buffer;

    try {
        if (!(gParameters.GetIsPurelyTemporalAnalysis() || gParameters.GetProbabilityModelType() == HOMOGENEOUSPOISSON)) {
            settings.push_back(std::make_pair("Use Non-Euclidian Neighbors file",(gParameters.UseLocationNeighborsFile() ? "Yes" : "No")));
            if (gParameters.UseLocationNeighborsFile())
                settings.push_back(std::make_pair("Non-Euclidian Neighbors file",gParameters.GetLocationNeighborsFileName()));
            settings.push_back(std::make_pair("Use Meta Locations File",(gParameters.UseLocationNeighborsFile() ? "Yes" : "No")));
            if (gParameters.UseMetaLocationsFile())
                settings.push_back(std::make_pair("Meta Locations File",gParameters.getMetaLocationsFilename()));
        }
        if (!(gParameters.GetIsPurelyTemporalAnalysis() || gParameters.UseLocationNeighborsFile() || gParameters.GetProbabilityModelType() == HOMOGENEOUSPOISSON)) {
            buffer = "Multiple Coordinates Type";
            switch (gParameters.GetMultipleCoordinatesType()) {
                case ONEPERLOCATION :
                    settings.push_back(std::make_pair(buffer,"Allow only one set of coordinates per location ID."));
                    break;
                case ATLEASTONELOCATION :
                    settings.push_back(std::make_pair(buffer,"Include location ID in the scanning window if at least one set of coordinates is included."));
                    break;
                case ALLLOCATIONS :
                    settings.push_back(std::make_pair(buffer,"Include location ID in the scanning window if and only if all sets of coordinates are in the window."));
                    break;
                default : throw prg_error("Unknown multiple coordinates type %d.\n", "PrintSpatialNeighborsParameters()", gParameters.GetMultipleCoordinatesType());
            }
        }
        WriteSettingsContainer(settings, "Spatial Neighbors", fp);
    } catch (prg_exception& x) {
        x.addTrace("PrintSpatialNeighborsParameters()","ParametersPrint");
        throw;
    }
}

/** Prints 'Space And Time Adjustments' tab parameters to file stream. */
void ParametersPrint::PrintSpaceAndTimeAdjustmentsParameters(FILE* fp) const {
    bool bPrintingTemporalAdjustment = (gParameters.GetAnalysisType() == PURELYTEMPORAL ||
                                        gParameters.GetAnalysisType() == SPACETIME ||
                                        gParameters.GetAnalysisType() == PROSPECTIVEPURELYTEMPORAL ||
                                        gParameters.GetAnalysisType() == PROSPECTIVESPACETIME ||
                                        gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) &&
                                        gParameters.GetProbabilityModelType() == POISSON;
    bool bPrintingSpatialAdjustment =  (gParameters.GetAnalysisType() == SPACETIME ||
                                        gParameters.GetAnalysisType() == PROSPECTIVESPACETIME) &&
                                        gParameters.GetProbabilityModelType() == POISSON;

    SettingContainer_t settings;
    std::string buffer, worker;
    try {
        if (bPrintingTemporalAdjustment) {
            switch (gParameters.GetTimeTrendAdjustmentType()) {
                case NOTADJUSTED               :
                    settings.push_back(std::make_pair("Temporal Adjustment","None"));break;
                case NONPARAMETRIC             :
                    settings.push_back(std::make_pair("Temporal Adjustment","Nonparametric"));break;
                case LOGLINEAR_PERC            :
                    printString(buffer, "Log linear with %g%% per year", gParameters.GetTimeTrendAdjustmentPercentage());
                    settings.push_back(std::make_pair("Temporal Adjustment",buffer));break;
                case CALCULATED_LOGLINEAR_PERC :
                    settings.push_back(std::make_pair("Temporal Adjustment","Log linear with automatically calculated trend"));break;
                case STRATIFIED_RANDOMIZATION  :
                    settings.push_back(std::make_pair("Temporal Adjustment","Nonparametric, with time stratified randomization"));break;
                default : throw prg_error("Unknown time trend adjustment type '%d'.\n",
                                          "PrintSpaceAndTimeAdjustmentsParameters()", gParameters.GetTimeTrendAdjustmentType());
            }
        }
        if (gParameters.GetAnalysisType() != PURELYSPATIAL) {
            settings.push_back(std::make_pair("Adjust for Weekly Trends, Nonparametric",(gParameters.getAdjustForWeeklyTrends() ? "Yes" : "No")));
        }
        if (bPrintingSpatialAdjustment) {
            switch (gParameters.GetSpatialAdjustmentType()) {
                case NO_SPATIAL_ADJUSTMENT              :
                    settings.push_back(std::make_pair("Spatial Adjustment","None")); break;
                case SPATIALLY_STRATIFIED_RANDOMIZATION :
                    settings.push_back(std::make_pair("Spatial Adjustment","Spatial adjustment by stratified randomization")); break;
                default : throw prg_error("Unknown spatial adjustment type '%d'.\n", "PrintSpaceAndTimeAdjustmentsParameters()", gParameters.GetSpatialAdjustmentType());
            }
        }
        if (gParameters.GetProbabilityModelType() == POISSON) {
            settings.push_back(std::make_pair("Adjust for known relative risks",(gParameters.UseAdjustmentForRelativeRisksFile() ? "Yes" : "No")));
            if (gParameters.UseAdjustmentForRelativeRisksFile())
                settings.push_back(std::make_pair("Adjustments File",gParameters.GetAdjustmentsByRelativeRisksFilename()));
            //since SVTT time trend type is defaulted to Linear and not GUI, only report as quadratic when set
            if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND && gParameters.getTimeTrendType() == QUADRATIC)
                settings.push_back(std::make_pair("Time Trend Type (SVTT)","Quadratic"));
        }
        WriteSettingsContainer(settings, "Space And Time Adjustments", fp);
    } catch (prg_exception& x) {
        x.addTrace("PrintSpaceAndTimeAdjustmentsParameters()","ParametersPrint");
        throw;
    }
}

/** Prints 'Spatial Window' tab parameters to file stream. */
void ParametersPrint::PrintSpatialWindowParameters(FILE* fp) const {
    SettingContainer_t settings;
    std::string buffer, worker;

    try {
        if (gParameters.GetIsPurelyTemporalAnalysis()) return;

        if (!(gParameters.GetAnalysisType() == PROSPECTIVESPACETIME && gParameters.GetAdjustForEarlierAnalyses())) {
            printString(buffer, "%g percent of population at risk", gParameters.GetMaxSpatialSizeForType(PERCENTOFPOPULATION, false));
            settings.push_back(std::make_pair("Maximum Spatial Cluster Size",buffer));
        }
        if (gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false)) {
            printString(buffer, "%g percent of population defined in max circle file", gParameters.GetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false));
            settings.push_back(std::make_pair("Maximum Spatial Cluster Size", buffer));
        }
        if (gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false) || gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, true))
            settings.push_back(std::make_pair("Max Circle Size File",gParameters.GetMaxCirclePopulationFileName()));
        if (gParameters.GetRestrictMaxSpatialSizeForType(MAXDISTANCE, false)) {
            printString(buffer, "%g%s", gParameters.GetMaxSpatialSizeForType(MAXDISTANCE, false), (gParameters.GetCoordinatesType() == CARTESIAN ? " Cartesian units" : " km"));
            settings.push_back(std::make_pair("Maximum Spatial Cluster Size", buffer));
        }
        if (gParameters.GetProbabilityModelType() != SPACETIMEPERMUTATION && gParameters.GetIsSpaceTimeAnalysis()) {
            settings.push_back(std::make_pair("Include Purely Temporal Clusters", (gParameters.GetIncludePurelyTemporalClusters() ? "Yes" : "No")));
        }
        if (!gParameters.UseLocationNeighborsFile()) {
            switch (gParameters.GetSpatialWindowType()) {
                case CIRCULAR : settings.push_back(std::make_pair("Window Shape", "Circular")); break;
                case ELLIPTIC :
                    settings.push_back(std::make_pair("Window Shape", "Elliptic"));
                    switch (gParameters.GetNonCompactnessPenaltyType()) {
                        case NOPENALTY     : settings.push_back(std::make_pair("Non-Compactness Penalty", "None")); break;
                        case MEDIUMPENALTY : settings.push_back(std::make_pair("Non-Compactness Penalty", "Meduim")); break;
                        case STRONGPENALTY : settings.push_back(std::make_pair("Non-Compactness Penalty", "Strong")); break;
                        default : 
                            throw prg_error("Unknown non-compactness penalty type '%d'.\n", "PrintSpatialWindowParameters()", gParameters.GetNonCompactnessPenaltyType());
                    }
                    break;
                    default : throw prg_error("Unknown window shape type %d.\n", "PrintSpatialWindowParameters()", gParameters.GetSpatialWindowType());
            }
        }
        if (!gParameters.getPerformPowerEvaluation() &&  gParameters.GetAnalysisType() == PURELYSPATIAL &&
            (gParameters.GetProbabilityModelType() == POISSON || gParameters.GetProbabilityModelType() == BERNOULLI)) {
            settings.push_back(std::make_pair("Isotonic Scan", (gParameters.GetRiskType() == MONOTONERISK ? "Yes" : "No")));
        }
        WriteSettingsContainer(settings, "Spatial Window", fp);
    } catch (prg_exception& x) {
        x.addTrace("PrintSpatialWindowParameters()","ParametersPrint");
        throw;
    }
}

/** Prints 'System' parameters to file stream. */
void ParametersPrint::PrintSystemParameters(FILE* fp) const {
    const CParameters::CreationVersion & IniVersion = gParameters.GetCreationVersion();
    CParameters::CreationVersion Current = {atoi(VERSION_MAJOR), atoi(VERSION_MINOR), atoi(VERSION_RELEASE)};
    SettingContainer_t settings;
    std::string buffer;

    if (IniVersion.iMajor != Current.iMajor ||
        IniVersion.iMinor != Current.iMinor ||
        IniVersion.iRelease != Current.iRelease) {
        printString(buffer, "%u.%u.%u", IniVersion.iMajor, IniVersion.iMinor, IniVersion.iRelease);
        settings.push_back(std::make_pair("Parameters Version",buffer));
        WriteSettingsContainer(settings, "System", fp);
    }
}

/** Prints 'Temporal Ouput' tab parameters to file stream. */
void ParametersPrint::PrintTemporalOutputParameters(FILE* fp) const {
    SettingContainer_t settings;
    std::string buffer;

    try {
        if (!(gParameters.GetIsPurelyTemporalAnalysis() && (gParameters.GetProbabilityModelType() == POISSON || gParameters.GetProbabilityModelType() == BERNOULLI))) return;

        // This feature is not present in gui at the moment, so only show if toggled on.
        if (!gParameters.getOutputTemporalGraphFile()) return; 

        settings.push_back(std::make_pair("Create Temporal Graph",(gParameters.getOutputTemporalGraphFile() ? "Yes" : "No")));
        if (gParameters.getOutputTemporalGraphFile()) {
            FileName outputFile(gParameters.GetOutputFileName().c_str());
            outputFile.setFullPath(gParameters.GetOutputFileName().c_str());
            TemporalChartGenerator::getFilename(outputFile);
            settings.push_back(std::make_pair("Temporal Graph File", outputFile.getFullPath(buffer)));
        }
        WriteSettingsContainer(settings, "Temporal Output", fp);
    } catch (prg_exception& x) {
        x.addTrace("PrintTemporalOutputParameters()","ParametersPrint");
        throw;
    }
}

/** Prints 'Temporal Window' tab parameters to file stream. */
void ParametersPrint::PrintTemporalWindowParameters(FILE* fp) const {
    SettingContainer_t settings;
    std::string buffer, worker;

    try {
        // skip this section if purely spatial or svtt analyses, these settings are not relevant
        if (gParameters.GetAnalysisType() == PURELYSPATIAL || gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) return;

        printString(buffer, "%i %s",
                    gParameters.getMinimumTemporalClusterSize(),
                    GetDatePrecisionAsString(gParameters.GetTimeAggregationUnitsType(), worker, gParameters.getMinimumTemporalClusterSize() != 1, true));
        settings.push_back(std::make_pair("Minimum Temporal Cluster Size", buffer)); 
        switch (gParameters.GetMaximumTemporalClusterSizeType()) {
            case PERCENTAGETYPE :
                printString(buffer, "%g percent of study period", gParameters.GetMaximumTemporalClusterSize());
                settings.push_back(std::make_pair("Maximum Temporal Cluster Size", buffer)); 
                break;
            case TIMETYPE       :
                printString(buffer, "%g %s",
                            gParameters.GetMaximumTemporalClusterSize(),
                            GetDatePrecisionAsString(gParameters.GetTimeAggregationUnitsType(), worker, gParameters.GetMaximumTemporalClusterSize() != 1, true));
                settings.push_back(std::make_pair("Maximum Temporal Cluster Size", buffer)); break;
                break;
            default : throw prg_error("Unknown maximum temporal cluster size type '%d'.\n",
                                      "PrintTemporalWindowParameters()", gParameters.GetMaximumTemporalClusterSizeType());
        }
        if (gParameters.GetProbabilityModelType() != SPACETIMEPERMUTATION && gParameters.GetIsSpaceTimeAnalysis()) {
            settings.push_back(std::make_pair("Include Purely Spatial Clusters",(gParameters.GetIncludePurelySpatialClusters() ? "Yes" : "No")));
        }
        if (gParameters.GetAnalysisType() == PURELYTEMPORAL || gParameters.GetAnalysisType() == SPACETIME) {
            switch (gParameters.GetIncludeClustersType()) {
                case ALIVECLUSTERS   :
                    settings.push_back(std::make_pair("Clusters to Include", "Only those including the study end date"));
                    break;
                case ALLCLUSTERS     : /*fprintf(fp, "All\n");
                                  -- geIncludeClustersType parameter no longer visible in GUI,
                                     defaulted to ALLCLUSTERS, so don't print setting */ break;
                case CLUSTERSINRANGE :
                    printString(buffer, "%s to %s", gParameters.GetStartRangeStartDate().c_str(), gParameters.GetStartRangeEndDate().c_str());
                    settings.push_back(std::make_pair("Flexible Temporal Window Start Range", buffer));
                    printString(buffer, "%s to %s", gParameters.GetEndRangeStartDate().c_str(), gParameters.GetEndRangeEndDate().c_str());
                    settings.push_back(std::make_pair("Flexible Temporal Window End Range", buffer));
                    break;
                default : throw prg_error("Unknown inclusion cluster type '%d'.\n", "PrintTemporalWindowParameters()", gParameters.GetIncludeClustersType());
            }
        }
        WriteSettingsContainer(settings, "Temporal Window", fp);
    } catch (prg_exception& x) {
        x.addTrace("PrintTemporalWindowParameters()","ParametersPrint");
        throw;
    }
}

/** Writes settings container to file stream. */
void ParametersPrint::WriteSettingsContainer(const SettingContainer_t& settings, const std::string& section, FILE* fp) const {
  try {
      if (!settings.size()) return;

      //print section label
      fprintf(fp, "\n");
      fprintf(fp, section.c_str());
      fprintf(fp, "\n");
      for (size_t t=0; t < section.size(); ++t)
          fprintf(fp, "-");
      fprintf(fp, "\n");

      SettingContainer_t::const_iterator itr=settings.begin();
      //first calculate maximum label length
      size_t tMaxLabel=0;
      for (; itr != settings.end(); ++itr) {
            tMaxLabel = std::max(tMaxLabel, itr->first.size());
      }
      //print settings
      for (itr=settings.begin(); itr != settings.end(); ++itr) {
          fprintf(fp, "  ");
          fprintf(fp, itr->first.c_str());
          for (size_t t=itr->first.size(); t < tMaxLabel; ++t)
            fprintf(fp, " ");
          fprintf(fp, " : ");
          fprintf(fp, itr->second.c_str());
          fprintf(fp, "\n");
      }
  }
  catch (prg_exception& x) {
    x.addTrace("WriteSettingsContainer()","ParametersPrint");
    throw;
  }
}

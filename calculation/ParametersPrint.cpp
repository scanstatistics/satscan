//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ParametersPrint.h"
#include "DataSetHandler.h"

/** constructor*/
ParametersPrint::ParametersPrint(const CParameters& Parameters) : gParameters(Parameters) {}

/** destructor */
ParametersPrint::~ParametersPrint() {}

/** Returns analysis type as string. */
const char * ParametersPrint::GetAnalysisTypeAsString() const {
  const char * sAnalysisType;

  try {
    switch (gParameters.GetAnalysisType()) {
      case PURELYSPATIAL             : sAnalysisType = "Purely Spatial"; break;
      case PURELYTEMPORAL            : sAnalysisType = "Retrospective Purely Temporal"; break;
      case SPACETIME                 : sAnalysisType = "Retrospective Space-Time"; break;
      case PROSPECTIVESPACETIME      : sAnalysisType = "Prospective Space-Time"; break;
      case SPATIALVARTEMPTREND       : sAnalysisType = "Spatial Variation of Temporal Trends"; break;
      case PROSPECTIVEPURELYTEMPORAL : sAnalysisType = "Prospective Purely Temporal"; break;
      default : ZdException::Generate("Unknown analysis type '%d'.\n", "GetAnalysisTypeAsString()", gParameters.GetAnalysisType());
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("GetAnalysisTypeAsString()","ParametersPrint");
    throw;
  }
  return sAnalysisType;
}

/** Returns area scan type as string based upon probability model type. */
const char * ParametersPrint::GetAreaScanRateTypeAsString() const {
  try {
    switch (gParameters.GetProbabilityModelType()) {
      case POISSON :  if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
                         switch (gParameters.GetAreaScanRateType()) {
                           case HIGH       : return "Increasing Rates";
                           case LOW        : return "Decreasing Rates";
                           case HIGHANDLOW : return "Increasing or Decreasing Rates";
                           default : ZdException::Generate("Unknown area scan rate type '%d'.\n", "GetAreaScanRateTypeAsString()", gParameters.GetAreaScanRateType());
                         }
                         break;
                      }
      case BERNOULLI :
      case SPACETIMEPERMUTATION :
         switch (gParameters.GetAreaScanRateType()) {
            case HIGH       : return "High Rates";
            case LOW        : return "Low Rates";
            case HIGHANDLOW : return "High or Low Rates";
            default : ZdException::Generate("Unknown area scan rate type '%d'.\n", "GetAreaScanRateTypeAsString()", gParameters.GetAreaScanRateType());
         }
         break;
      case ORDINAL :
      case NORMAL :
         switch (gParameters.GetAreaScanRateType()) {
            case HIGH       : return "High Values";
            case LOW        : return "Low Values";
            case HIGHANDLOW : return "High or Low Values";
            default : ZdException::Generate("Unknown area scan rate type '%d'.\n", "GetAreaScanRateTypeAsString()", gParameters.GetAreaScanRateType());
         }
         break;
      case EXPONENTIAL :
         switch (gParameters.GetAreaScanRateType()) {
            case HIGH       : return "Short Survival";
            case LOW        : return "Long Survival";
            case HIGHANDLOW : return "Short or Long Survival";
            default : ZdException::Generate("Unknown area scan rate type '%d'.\n", "GetAreaScanRateTypeAsString()", gParameters.GetAreaScanRateType());
         }
         break;
      default : ZdGenerateException("Unknown probability model '%d'.", "GetAreaScanRateTypeAsString()", gParameters.GetProbabilityModelType());
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("GetAreaScanRateTypeAsString()","ParametersPrint");
    throw;
  }
  return "?";
}

/** Returns probability model type as a character array. */
const char * ParametersPrint::GetProbabilityModelTypeAsString() const {
  const char * sProbabilityModel;

  try {
    switch (gParameters.GetProbabilityModelType()) {
      case POISSON              : sProbabilityModel = "Poisson"; break;
      case BERNOULLI            : sProbabilityModel = "Bernoulli"; break;
      case SPACETIMEPERMUTATION : sProbabilityModel = "Space-Time Permutation"; break;
      case ORDINAL              : sProbabilityModel = "Ordinal"; break;
      case EXPONENTIAL          : sProbabilityModel = "Exponential"; break;
      case NORMAL               : sProbabilityModel = "Normal"; break;
      case RANK                 : sProbabilityModel = "Rank"; break;
      default : ZdException::Generate("Unknown probability model type '%d'.\n", "GetProbabilityModelTypeAsString()", gParameters.GetProbabilityModelType());
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("GetProbabilityModelTypeAsString()","ParametersPrint");
    throw;
  }
  return sProbabilityModel;
}

/** Prints parameters, in a particular format, to passed ascii file. */
void ParametersPrint::Print(FILE* fp) const {
  try {
    AsciiPrintFormat::PrintSectionSeparatorString(fp, 0, 2);
    fprintf(fp, "PARAMETER SETTINGS\n");
    //print 'Input' tab settings
    PrintInputParameters(fp);
    //print 'Analysis' tab settings
    PrintAnalysisParameters(fp);
    //print 'Output' tab settings
    PrintOutputParameters(fp);
    //print 'Multiple Data Sets' tab settings
    PrintMultipleDataSetParameters(fp);
    //print 'Data Checking' tab settings
    PrintDataCheckingParameters(fp);
    //print 'Neighbors File' tab settings
    PrintNeighborsFileParameters(fp);
    //print 'Multiple Coordinates Per Location' tab settings
    PrintMultipleCoordinatesParameters(fp);
    //print 'Spatial Window' tab settings
    PrintSpatialWindowParameters(fp);
    //print 'Temporal Window' tab settings
    PrintTemporalWindowParameters(fp);
    //print 'Space and Time Adjustments' tab settings
    PrintSpaceAndTimeAdjustmentsParameters(fp);
    //print 'Inference' tab settings
    PrintInferenceParameters(fp);
    //print 'Clusters Reported' tab settings
    PrintClustersReportedParameters(fp);
    //print 'Elliptic Scan' settings
    PrintEllipticScanParameters(fp);
    //print 'Power Simulations' settings
    PrintPowerSimulationsParameters(fp);
    //print 'RunOptions' settings
    PrintRunOptionsParameters(fp);
    //print 'System' parameters
    PrintSystemParameters(fp);
    AsciiPrintFormat::PrintSectionSeparatorString(fp, 0, 1);
  }
  catch (ZdException &x) {
    x.AddCallpath("Print()","ParametersPrint");
    throw;
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
        ZdException::Generate("Unknown time trend adjustment type '%d'\n.",
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
        ZdException::Generate("Unknown time trend adjustment type '%d'\n.",
                              "PrintAdjustments()", gParameters.GetSpatialAdjustmentType());
    }
    //display space-time adjustments
    if (gParameters.UseAdjustmentForRelativeRisksFile()) {
        buffer = "Adjusted for known relative risks.";
        PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
    }    
  }
  catch (ZdException &x) {
    x.AddCallpath("PrintAdjustments()","ParametersPrint");
    throw;
  }
}

/** Prints 'Analysis' tab parameters to file stream. */
void ParametersPrint::PrintAnalysisParameters(FILE* fp) const {
  AnalysisType eAnalysisType = gParameters.GetAnalysisType();

  try {
    fprintf(fp, "\nAnalysis\n--------\n");
    fprintf(fp, "  Type of Analysis         : %s\n", GetAnalysisTypeAsString());
    fprintf(fp, "  Probability Model        : %s\n", GetProbabilityModelTypeAsString());
    fprintf(fp, "  Scan for Areas with      : %s\n", GetAreaScanRateTypeAsString());
    if (eAnalysisType != PURELYSPATIAL) {
     fprintf(fp, "\n  Time Aggregation Units   : ");
      switch (gParameters.GetTimeAggregationUnitsType()) {
        case YEAR  : fprintf(fp, "Year\n"); break;
        case MONTH : fprintf(fp, "Month\n"); break;
        case DAY   : fprintf(fp, "Day\n"); break;
        default : ZdException::Generate("Unknown date precision type '%d'.\n",
                                        "PrintAnalysisParameters()", gParameters.GetTimeAggregationUnitsType());
      }
      fprintf(fp, "  Time Aggregation Length  : %i\n", gParameters.GetTimeAggregationLength());
    }
    fprintf(fp, "\n  Number of Replications   : %u\n", gParameters.GetNumReplicationsRequested());
  }
  catch (ZdException &x) {
    x.AddCallpath("PrintAnalysisParameters()","ParametersPrint");
    throw;
  }
}

/** Prints analysis type related information, in a particular format, to passed ascii file. */
void ParametersPrint::PrintAnalysisSummary(FILE* fp) const {
  try {
    switch (gParameters.GetAnalysisType()) {
      case PURELYSPATIAL             : fprintf(fp, "Purely Spatial analysis\n"); break;
      case PURELYTEMPORAL            : fprintf(fp, "Retrospective Purely Temporal analysis\n"); break;
      case SPACETIME                 : fprintf(fp, "Retrospective Space-Time analysis\n"); break;
      case PROSPECTIVESPACETIME      : fprintf(fp, "Prospective Space-Time analysis\n"); break;
      case SPATIALVARTEMPTREND       : fprintf(fp, "Spatial Variation of Temporal Trends analysis\n"); break;
      case PROSPECTIVEPURELYTEMPORAL : fprintf(fp, "Prospective Purely Temporal analysis\n"); break;
      default : ZdException::Generate("Unknown analysis type '%d'.\n",
                                      "PrintAnalysisSummary()", gParameters.GetAnalysisType());
    }
    fprintf(fp, "scanning for ");
    if (gParameters.GetAnalysisType() == PURELYSPATIAL && gParameters.GetRiskType() == MONOTONERISK)
      fprintf(fp, "monotone ");
    std::string s(GetAreaScanRateTypeAsString());
    std::transform(s.begin(), s.end(), s.begin(), (int(*)(int)) tolower);
    fprintf(fp, "clusters with %s\n", s.c_str());
    switch (gParameters.GetProbabilityModelType()) {
      case POISSON              : fprintf(fp, "using the Poisson model.\n"); break;
      case BERNOULLI            : fprintf(fp, "using the Bernoulli model.\n"); break;
      case SPACETIMEPERMUTATION : fprintf(fp, "using the Space-Time Permutation model.\n"); break;
      case ORDINAL              : fprintf(fp, "using the Ordinal model.\n"); break;
      case EXPONENTIAL          : fprintf(fp, "using the Exponential model.\n"); break;
      case NORMAL               : fprintf(fp, "using the Normal model.\n"); break;
      case RANK                 : fprintf(fp, "using the Rank model.\n"); break;
      default : ZdException::Generate("Unknown probability model type '%d'.\n",
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
        default : ZdException::Generate("Unknown purpose for multiple data sets type '%d'.\n",
                                        "PrintAnalysisSummary()", gParameters.GetMultipleDataSetPurposeType());
      }
    }
    if (gParameters.GetIsIterativeScanning())
      fprintf(fp, "Iterative analysis performed.\n");
  }
  catch (ZdException &x) {
    x.AddCallpath("PrintAnalysisSummary()","ParametersPrint");
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

/** Prints 'Clusters Reported' tab parameters to file stream. */
void ParametersPrint::PrintClustersReportedParameters(FILE* fp) const {
  try {
    if (!gParameters.GetIsPurelyTemporalAnalysis() && !gParameters.UseLocationNeighborsFile()) {
      fprintf(fp, "\nClusters Reported\n-----------------\n");
      fprintf(fp, "  Criteria for Reporting Secondary Clusters : ");
      switch (gParameters.GetCriteriaSecondClustersType()) {
         case NOGEOOVERLAP          : fprintf(fp, "No Geographical Overlap\n"); break;
         case NOCENTROIDSINOTHER    : fprintf(fp, "No Cluster Centroids in Other Clusters\n"); break;
         case NOCENTROIDSINMORELIKE : fprintf(fp, "No Cluster Centroids in More Likely Clusters\n"); break;
         case NOCENTROIDSINLESSLIKE : fprintf(fp, "No Cluster Centroids in Less Likely Clusters\n"); break;
         case NOPAIRSINEACHOTHERS   : fprintf(fp, "No Pairs of Centroids Both in Each Others Clusters\n"); break;
         case NORESTRICTIONS        : fprintf(fp, "No Restrictions = Most Likely Cluster for Each Centroid\n"); break;
         default : ZdException::Generate("Unknown secondary clusters type '%d'.\n",
                                         "PrintClustersReportedParameters()", gParameters.GetCriteriaSecondClustersType());
      }
      if (gParameters.GetRestrictingMaximumReportedGeoClusterSize()) {
        if (!(gParameters.GetAnalysisType() == PROSPECTIVESPACETIME && gParameters.GetAdjustForEarlierAnalyses()))
          fprintf(fp, "  Only clusters smaller than %g%% of population at risk reported.\n", gParameters.GetMaxSpatialSizeForType(PERCENTOFPOPULATION, true));
        if (gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, true))
           fprintf(fp, "  Only clusters smaller than %g%% of population defined in max circle file reported.\n", gParameters.GetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, true));
        if (gParameters.GetRestrictMaxSpatialSizeForType(MAXDISTANCE, true))
           fprintf(fp, "  Only clusters smaller than %g%s reported.\n",
                   gParameters.GetMaxSpatialSizeForType(MAXDISTANCE, true), (gParameters.GetCoordinatesType() == CARTESIAN ? " Cartesian units" : " km"));
     }
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("PrintClustersReportedParameters()","ParametersPrint");
    throw;
  }
}

/** Prints 'Data Checking' parameters to file stream. */
void ParametersPrint::PrintDataCheckingParameters(FILE* fp) const {
  fprintf(fp, "\nData Checking\n-------------\n");
  fprintf(fp, "  Study Period Check             : ");
  switch (gParameters.GetStudyPeriodDataCheckingType()) {
    case STRICTBOUNDS     : fprintf(fp, "Check to ensure that cases and controls are within the Study Period.\n"); break;
    case RELAXEDBOUNDS    : fprintf(fp, "Ignore cases and controls that are outside the Study Period.\n"); break;
    default : ZdException::Generate("Unknown study period check type '%d'.\n", "PrintDataCheckingParameters()", gParameters.GetStudyPeriodDataCheckingType());
  }
  if (!gParameters.GetIsPurelyTemporalAnalysis()) {
    fprintf(fp, "  Geographical Coordinates Check : ");
    switch (gParameters.GetCoordinatesDataCheckingType()) {
      case STRICTCOORDINATES  : fprintf(fp, "Check to ensure that all locations in the case, control\n"
                                            "                                   and population files are present in the %s file.\n",
                                            (gParameters.UseLocationNeighborsFile() ? "neighbors" : "coordinates")); break;
      case RELAXEDCOORDINATES : fprintf(fp, "Ignore data in the case, control and population files that \n"
                                            "                                   do not correspond to a location ID listed in the %s file.\n",
                                            (gParameters.UseLocationNeighborsFile() ? "neighbors" : "coordinates")); break;
      default : ZdException::Generate("Unknown geographical coordinates check type '%d'.\n", "PrintDataCheckingParameters()", gParameters.GetCoordinatesDataCheckingType());
    }
  }  
}

/** Prints 'Elliptic Scan' parameters to file stream. */
void ParametersPrint::PrintEllipticScanParameters(FILE* fp) const {
  if (!gParameters.GetIsPurelyTemporalAnalysis() && !gParameters.UseLocationNeighborsFile() && gParameters.GetSpatialWindowType() == ELLIPTIC) {
    fprintf(fp, "\nElliptic Scan\n-------------\n");
    fprintf(fp, "  Ellipse Shapes                           : ");
    fprintf(fp, "%g", gParameters.GetEllipseShapes()[0]);
    for (size_t i=1; i < gParameters.GetEllipseShapes().size(); ++i)
       fprintf(fp, ", %g", gParameters.GetEllipseShapes()[i]);
    fprintf(fp, "\n  Number of Angles for Each Ellipse Shape  : ");
    fprintf(fp, "%i", gParameters.GetEllipseRotations()[0]);
    for (size_t i=1; i < gParameters.GetEllipseRotations().size(); ++i)
       fprintf(fp, ", %i", gParameters.GetEllipseRotations()[i]);
    fprintf(fp, "\n");
  }
}

/** Prints 'Inference' tab parameters to file stream. */
void ParametersPrint::PrintInferenceParameters(FILE* fp) const {
  fprintf(fp, "\nInference\n---------\n");
  fprintf(fp, "  Early Termination             : %s\n", (gParameters.GetTerminateSimulationsEarly() ? "Yes" : "No"));
  if (gParameters.GetIsProspectiveAnalysis()) {
    fprintf(fp, "  Adjusted for Earlier Analyses : %s\n", (gParameters.GetAdjustForEarlierAnalyses() ? "Yes" : "No"));
    if (gParameters.GetAdjustForEarlierAnalyses())
     fprintf(fp, "  Prospective Start Date        : %s\n", gParameters.GetProspectiveStartDate().c_str());
  }
  fprintf(fp, "  Report Critical Values        : %s\n", (gParameters.GetReportCriticalValues() ? "Yes" : "No"));
  fprintf(fp, "  Iterative Scan                : %s\n", (gParameters.GetIsIterativeScanning() ? "Yes" : "No"));
  if (gParameters.GetIsIterativeScanning()) {
    fprintf(fp, "  Number of Scans               : %u\n", gParameters.GetNumIterativeScansRequested());
    fprintf(fp, "  P-value Cutoff                : %g\n", gParameters.GetIterativeCutOffPValue());
  }  
}

/** Prints 'Input' tab parameters to file stream. */
void ParametersPrint::PrintInputParameters(FILE* fp) const {
  DatePrecisionType     ePrecision;
  const char          * sDataSetLabel = (gParameters.GetNumDataSets() == 1 ? "" : "(data set 1)");
  const char          * sBlankDataSetLabel = (gParameters.GetNumDataSets() == 1 ? "" : "            ");

  try {
    fprintf(fp, "\nInput\n-----\n");
    fprintf(fp, "  Case File         %s : %s\n", sDataSetLabel, gParameters.GetCaseFileName(1).c_str());
    switch (gParameters.GetProbabilityModelType()) {
      case POISSON :
         if (!gParameters.UsePopulationFile()) break;
         fprintf(fp, "  Population File   %s : %s\n", sDataSetLabel,  gParameters.GetPopulationFileName(1).c_str()); break;
      case BERNOULLI :
         fprintf(fp, "  Control File      %s : %s\n", sDataSetLabel, gParameters.GetControlFileName(1).c_str()); break;
      case SPACETIMEPERMUTATION :
      case ORDINAL              :
      case EXPONENTIAL          :
      case NORMAL               :
      case RANK                 : break;
      default : ZdException::Generate("Unknown probability model type '%d'.\n",
                                      "PrintInputParameters()", gParameters.GetProbabilityModelType());
    }
    if (gParameters.UseCoordinatesFile())
      fprintf(fp, "  Coordinates File  %s : %s\n", sBlankDataSetLabel, gParameters.GetCoordinatesFileName().c_str());
    if (gParameters.UseSpecialGrid())
      fprintf(fp, "  Grid File         %s : %s\n", sBlankDataSetLabel, gParameters.GetSpecialGridFileName().c_str());
    if (gParameters.GetSimulationType() == FILESOURCE)
      fprintf(fp, "  Simulated Data Import File  : %s\n", gParameters.GetSimulationDataSourceFilename().c_str());
    fprintf(fp, "\n  Time Precision     : ");
    //Display precision, keeping in mind the v4 behavior.
    if (gParameters.GetPrecisionOfTimesType() == NONE)
      ePrecision = NONE;
    else if (gParameters.GetCreationVersion().iMajor == 4)
      ePrecision = (gParameters.GetAnalysisType() == PURELYSPATIAL ? YEAR : gParameters.GetTimeAggregationUnitsType());
    else
      ePrecision =  gParameters.GetPrecisionOfTimesType();
    switch (ePrecision) {
      case YEAR  : fprintf(fp, "Year\n"); break;
      case MONTH : fprintf(fp, "Month\n"); break;
      case DAY   : fprintf(fp, "Day\n"); break;
      default    : fprintf(fp, "None\n"); break;;
    }
    fprintf(fp, "  Start Date         : %s\n", gParameters.GetStudyPeriodStartDate().c_str());
    fprintf(fp, "  End Date           : %s\n", gParameters.GetStudyPeriodEndDate().c_str());
    if ((gParameters.UseCoordinatesFile() || gParameters.UseSpecialGrid())) {
      fprintf(fp, "  Coordinates        : ");
      switch (gParameters.GetCoordinatesType()) {
        case CARTESIAN : fprintf(fp, "Cartesian\n"); break;
        case LATLON    : fprintf(fp, "Latitude/Longitude\n"); break;
        default : ZdException::Generate("Unknown coordinated type '%d'.\n",
                                        "PrintInputParameters()", gParameters.GetCoordinatesType());
      }
    }  
  }
  catch (ZdException &x) {
    x.AddCallpath("PrintInputParameters()","ParametersPrint");
    throw;
  }
}

/** Prints 'Multiple Coordinates Per Location' tab parameters to file stream. */
void ParametersPrint::PrintMultipleCoordinatesParameters(FILE* fp) const {
  if (gParameters.GetIsPurelyTemporalAnalysis())
    return;

  fprintf(fp, "\nMultiple Coordinates Per Location\n---------------------------------\n");
  fprintf(fp, "  Multiple Coordinates Type : ");
  switch (gParameters.GetMultipleCoordinatesType()) {
    case ONEPERLOCATION : fprintf(fp, "Allow only one coordinate per location.\n"); break;
    case ATLEASTONELOCATION : fprintf(fp, "Include location in the scanning window if at least one coordinate is in the circle.\n"); break;
    case ALLLOCATIONS : fprintf(fp, "Include location in the scanning window if all coordinates are in the circle.\n"); break;
    default : ZdException::Generate("Unknown multiple coordinates type %d.\n", "PrintSpatialWindowParameters()", gParameters.GetMultipleCoordinatesType());
  }
}

/** Prints 'Multiple Data Set' tab parameters to file stream. */
void ParametersPrint::PrintMultipleDataSetParameters(FILE* fp) const {
  try {
    if (gParameters.GetNumDataSets() == 1)
      return;

    fprintf(fp, "\nMultiple Data Sets\n------------------\n");
    for (unsigned int t=1; t < gParameters.GetNumDataSets(); ++t) {
       fprintf(fp, "  Case File        (data set %i) : %s\n", t + 1,  gParameters.GetCaseFileName(t + 1).c_str());
       switch (gParameters.GetProbabilityModelType()) {
         case POISSON :
           if (!gParameters.UsePopulationFile()) break;
           fprintf(fp, "  Population File  (data set %i) : %s\n", t + 1, gParameters.GetPopulationFileName(t + 1).c_str()); break;
         case BERNOULLI :
           fprintf(fp, "  Control File     (data set %i) : %s\n", t + 1, gParameters.GetControlFileName(t + 1).c_str()); break;
         case SPACETIMEPERMUTATION :
         case ORDINAL              :
         case EXPONENTIAL          :
         case NORMAL               :
         case RANK                 : break;
         default :
           ZdException::Generate("Unknown probability model type '%d'.\n",
                                 "PrintMultipleDataSetParameters()", gParameters.GetProbabilityModelType());
       }
    }
    fprintf(fp, "\n  Purpose of Multiple Data Sets : ");
    switch (gParameters.GetMultipleDataSetPurposeType()) {
      case MULTIVARIATE : fprintf(fp, "Multivariate Analysis\n"); break;
      case ADJUSTMENT    : fprintf(fp, "Adjustment\n"); break;
      default : ZdException::Generate("Unknown purpose for multiple data sets type '%d'.\n",
                                      "PrintMultipleDataSetParameters()", gParameters.GetMultipleDataSetPurposeType());
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("PrintMultipleDataSetParameters()","ParametersPrint");
    throw;
  }
}

/** Prints 'Run Options' parameters to file stream. */
void ParametersPrint::PrintNeighborsFileParameters(FILE* fp) const {
  try {
    if (gParameters.GetIsPurelyTemporalAnalysis()) return;

    fprintf(fp, "\nNon-Eucledian Neighbors\n-----------------------\n");
    fprintf(fp, "  Use Neighbors File      : %s\n", (gParameters.UseLocationNeighborsFile() ? "Yes" : "No"));
    if (gParameters.UseLocationNeighborsFile())
      fprintf(fp, "  Neighbors File          : %s\n", gParameters.GetLocationNeighborsFileName().c_str());
    fprintf(fp, "  Use Meta Locations File : %s\n", (gParameters.UseMetaLocationsFile() ? "Yes" : "No"));
    if (gParameters.UseMetaLocationsFile())
      fprintf(fp, "  Meta Locations File     : %s\n", gParameters.getMetaLocationsFilename().c_str());
  }
  catch (ZdException &x) {
    x.AddCallpath("PrintRunOptionsParameters()","ParametersPrint");
    throw;
  }
}

/** Prints 'Output' tab parameters to file stream. */
void ParametersPrint::PrintOutputParameters(FILE* fp) const {
  ZdFileName            AdditionalOutputFile(gParameters.GetOutputFileName().c_str());
  
  try {
    fprintf(fp, "\nOutput\n------\n");
    fprintf(fp, "  Results File          : %s\n", gParameters.GetOutputFileName().c_str());
    // cluster information files
    if (gParameters.GetOutputClusterLevelAscii()) {
      AdditionalOutputFile.SetExtension(".col.txt");
      fprintf(fp, "  Cluster File          : %s\n", AdditionalOutputFile.GetFullPath());
    }
    if (gParameters.GetOutputClusterLevelDBase()) {
      AdditionalOutputFile.SetExtension(".col.dbf");
      fprintf(fp, "  Cluster File          : %s\n", AdditionalOutputFile.GetFullPath());
    }
    // cluster case information files
    if (gParameters.GetOutputClusterCaseAscii()) {
      AdditionalOutputFile.SetExtension(".cci.txt");
      fprintf(fp, "  Cluster Case File     : %s\n", AdditionalOutputFile.GetFullPath());
    }
    if (gParameters.GetOutputClusterCaseDBase()) {
      AdditionalOutputFile.SetExtension(".cci.dbf");
      fprintf(fp, "  Cluster Case File     : %s\n", AdditionalOutputFile.GetFullPath());
    }
    // area specific files
    if (gParameters.GetOutputAreaSpecificAscii()) {
      AdditionalOutputFile.SetExtension(".gis.txt");
      fprintf(fp, "  Location File         : %s\n", AdditionalOutputFile.GetFullPath());
    }
    if (gParameters.GetOutputAreaSpecificDBase()) {
      AdditionalOutputFile.SetExtension(".gis.dbf");
      fprintf(fp, "  Location File         : %s\n", AdditionalOutputFile.GetFullPath());
    }
    // relative risk files
    if (gParameters.GetOutputRelativeRisksAscii()) {
      AdditionalOutputFile.SetExtension(".rr.txt");
      fprintf(fp, "  Relative Risks File   : %s\n", AdditionalOutputFile.GetFullPath());
    }
    if (gParameters.GetOutputRelativeRisksDBase()) {
      AdditionalOutputFile.SetExtension(".rr.dbf");
      fprintf(fp, "  Relative Risks File   : %s\n", AdditionalOutputFile.GetFullPath());
    }
    // loglikelihood ratio files
    if (gParameters.GetOutputSimLoglikeliRatiosAscii()) {
      AdditionalOutputFile.SetExtension(".llr.txt");
      fprintf(fp, "  Simulated LLRs File   : %s\n", AdditionalOutputFile.GetFullPath());
    }
    if (gParameters.GetOutputSimLoglikeliRatiosDBase()) {
      AdditionalOutputFile.SetExtension(".llr.dbf");
      fprintf(fp, "  Simulated LLRs File   : %s\n", AdditionalOutputFile.GetFullPath());
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("PrintOutputParameters()","ParametersPrint");
    throw;
  }
}

/** Prints 'Power Simulations' parameters to file stream. */
void ParametersPrint::PrintPowerSimulationsParameters(FILE* fp) const {
  bool bPrintingPowerCalculations = gParameters.GetIsPowerCalculated();
  bool bPrintingSimulationType = gParameters.GetSimulationType() != STANDARD;
  bool bPrintingSimulationData = gParameters.GetOutputSimulationData();

  try {
    if (bPrintingPowerCalculations || bPrintingSimulationType || bPrintingSimulationData) {
      fprintf(fp, "\nPower Simulations\n-----------------\n");
      if (bPrintingPowerCalculations) {
        fprintf(fp, "  P-values Prespecified LLRs : Yes\n");
        fprintf(fp, "  LLR1                       : %lf\n", gParameters.GetPowerCalculationX());
        fprintf(fp, "  LLR2                       : %lf\n", gParameters.GetPowerCalculationY());
      }
      if (gParameters.GetSimulationType() != STANDARD) {
        fprintf(fp, "  Simulation Method          : ");
        switch (gParameters.GetSimulationType()) {
          //case STANDARD         : fprintf(fp, "Null Randomization\n"); break;
          case HA_RANDOMIZATION : fprintf(fp, "HA Randomization\n"); break;
          case FILESOURCE       : fprintf(fp, "File Source\n");
                                  fprintf(fp, "  Simulation Data Source     : %s\n",
                                          gParameters.GetSimulationDataSourceFilename().c_str());
                                  break;
          default : ZdException::Generate("Unknown simulation type '%d'.\n",
                                          "PrintPowerSimulationsParameters()", gParameters.GetSimulationType());
        };
      }
      if (bPrintingSimulationData) {
        fprintf(fp, "  Output Simulation Data     : Yes\n");
        fprintf(fp, "  Simulation Data Output     : %s\n", gParameters.GetSimulationDataOutputFilename().c_str());
      }
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("PrintPowerSimulationsParameters()","ParametersPrint");
    throw;
  }
}

/** Prints 'Run Options' parameters to file stream. */
void ParametersPrint::PrintRunOptionsParameters(FILE* fp) const {
  try {
    fprintf(fp, "\nRun Options\n-----------\n");
    fprintf(fp, "  Processer Usage     : ");
    if (gParameters.GetNumRequestedParallelProcesses() == 0)
      fprintf(fp, "All Available Proccessors\n");
    else
      fprintf(fp, "At Most %u Proccessors\n", gParameters.GetNumRequestedParallelProcesses());
    if (gParameters.GetIsRandomlyGeneratingSeed())
      fprintf(fp, "  Use Random Seed     : %s\n", (gParameters.GetIsRandomlyGeneratingSeed() ? "Yes" : "No"));
    if (gParameters.GetRandomizationSeed() != RandomNumberGenerator::glDefaultSeed)
      fprintf(fp, "  Randomization Seed  : %ld\n", gParameters.GetRandomizationSeed());
    if (gParameters.GetExecutionType() != AUTOMATIC) {
      fprintf(fp, "  Execution Type      : ");
      switch (gParameters.GetExecutionType()) {
        case AUTOMATIC    : fprintf(fp, "Automatic Determination\n"); break;
        case SUCCESSIVELY : fprintf(fp, "Successively\n"); break;
        case CENTRICALLY  : fprintf(fp, "Centrically\n"); break;
        default : ZdException::Generate("Unknown execution type '%d'.\n",
                                        "PrintRunOptionsParameters()", gParameters.GetExecutionType());
      };
    }
    fprintf(fp, "  Logging Analysis    : %s\n", (gParameters.GetIsLoggingHistory() ? "Yes" : "No"));
    fprintf(fp, "  Suppress Warnings   : %s\n", (gParameters.GetSuppressingWarnings() ? "Yes" : "No"));
  }
  catch (ZdException &x) {
    x.AddCallpath("PrintRunOptionsParameters()","ParametersPrint");
    throw;
  }
}

/** Prints 'Space And Time Adjustments' tab parameters to file stream. */
void ParametersPrint::PrintSpaceAndTimeAdjustmentsParameters(FILE* fp) const {
  bool bPrintingAdjustmentsFileParameters = (gParameters.GetSimulationType() == HA_RANDOMIZATION ||
                                             gParameters.UseAdjustmentForRelativeRisksFile());
  bool bPrintingTemporalAdjustment = (gParameters.GetAnalysisType() == PURELYTEMPORAL ||
                                      gParameters.GetAnalysisType() == SPACETIME ||
                                      gParameters.GetAnalysisType() == PROSPECTIVEPURELYTEMPORAL ||
                                      gParameters.GetAnalysisType() == PROSPECTIVESPACETIME ||
                                      gParameters.GetAnalysisType() == SPATIALVARTEMPTREND)
                                      && gParameters.GetProbabilityModelType() == POISSON;
  bool bPrintingSpatialAdjustment =  (gParameters.GetAnalysisType() == SPACETIME ||
                                      gParameters.GetAnalysisType() == PROSPECTIVESPACETIME)
                                      && gParameters.GetProbabilityModelType() == POISSON;

  try {
    if (!(bPrintingAdjustmentsFileParameters || bPrintingTemporalAdjustment || bPrintingSpatialAdjustment))
       return;

    fprintf(fp, "\nSpace And Time Adjustments\n--------------------------\n");
    if (bPrintingAdjustmentsFileParameters)
      fprintf(fp, "  Adjustments File    : %s\n", gParameters.GetAdjustmentsByRelativeRisksFilename().c_str());
    if (bPrintingAdjustmentsFileParameters && (bPrintingTemporalAdjustment || bPrintingSpatialAdjustment))
      fprintf(fp, "\n");
    if (bPrintingTemporalAdjustment) {
      fprintf(fp, "  Temporal Adjustment : ");
      switch (gParameters.GetTimeTrendAdjustmentType()) {
        case NOTADJUSTED               : fprintf(fp, "None\n"); break;
        case NONPARAMETRIC             : fprintf(fp, "Nonparametric\n"); break;
        case LOGLINEAR_PERC            : fprintf(fp, "Log linear with %g%% per year\n", gParameters.GetTimeTrendAdjustmentPercentage()); break;
        case CALCULATED_LOGLINEAR_PERC : fprintf(fp, "Log linear with automatically calculated trend\n"); break;
        case STRATIFIED_RANDOMIZATION  : fprintf(fp, "Nonparametric, with time stratified randomization\n"); break;
        default : ZdException::Generate("Unknown time trend adjustment type '%d'.\n",
                                        "PrintSpaceAndTimeAdjustmentsParameters()", gParameters.GetTimeTrendAdjustmentType());
      }
    }
    if (bPrintingSpatialAdjustment) {
      fprintf(fp, "  Spatial Adjustment  : ");
      switch (gParameters.GetSpatialAdjustmentType()) {
        case NO_SPATIAL_ADJUSTMENT              : fprintf(fp, "None\n"); break;
        case SPATIALLY_STRATIFIED_RANDOMIZATION : fprintf(fp, "Spatial adjustment by stratified randomization\n"); break;
        default : ZdException::Generate("Unknown spatial adjustment type '%d'.\n",
                                        "PrintSpaceAndTimeAdjustmentsParameters()", gParameters.GetSpatialAdjustmentType());
      }
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("PrintSpaceAndTimeAdjustmentsParameters()","ParametersPrint");
    throw;
  }
}

/** Prints 'Spatial Window' tab parameters to file stream. */
void ParametersPrint::PrintSpatialWindowParameters(FILE* fp) const {
  try {
    if (gParameters.GetIsPurelyTemporalAnalysis())
      return;

    fprintf(fp, "\nSpatial Window\n--------------\n");
    if (gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false) || gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, true))
      fprintf(fp, "  Max Circle Size File                  : %s\n", gParameters.GetMaxCirclePopulationFileName().c_str());
    if (!(gParameters.GetAnalysisType() == PROSPECTIVESPACETIME && gParameters.GetAdjustForEarlierAnalyses()) && !gParameters.UseLocationNeighborsFile())
      fprintf(fp, "  Maximum Spatial Cluster Size          : %g%% of population at risk\n", gParameters.GetMaxSpatialSizeForType(PERCENTOFPOPULATION, false));
    if (gParameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false))
      fprintf(fp, "  Maximum Spatial Cluster Size          : %g%% of population defined in max circle file\n", gParameters.GetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false));
    if (gParameters.GetRestrictMaxSpatialSizeForType(MAXDISTANCE, false))
      fprintf(fp, "  Maximum Spatial Cluster Size          : %g%s\n",
              gParameters.GetMaxSpatialSizeForType(MAXDISTANCE, false), (gParameters.GetCoordinatesType() == CARTESIAN ? " Cartesian units\n" : " km\n"));
    if (gParameters.GetProbabilityModelType() != SPACETIMEPERMUTATION && gParameters.GetIsSpaceTimeAnalysis()) {
      fprintf(fp, "  Include Purely Temporal Clusters      : ");
      fprintf(fp, (gParameters.GetIncludePurelyTemporalClusters() ? "Yes\n" : "No\n"));
    }
    if (!gParameters.UseLocationNeighborsFile()) {
      fprintf(fp, "  Window Shape                          : ");
      switch (gParameters.GetSpatialWindowType()) {
        case CIRCULAR : fprintf(fp, "Circular\n"); break;
        case ELLIPTIC : fprintf(fp, "Elliptic\n");
                        fprintf(fp, "  Non-Compactness Penalty               : ");
                        switch (gParameters.GetNonCompactnessPenaltyType()) {
                          case NOPENALTY     : fprintf(fp, "None\n"); break;
                          case MEDIUMPENALTY : fprintf(fp, "Meduim\n"); break;
                          case STRONGPENALTY : fprintf(fp, "Strong\n"); break;
                          default : ZdException::Generate("Unknown non-compactness penalty type '%d'.\n",
                                                           "PrintSpatialWindowParameters()", gParameters.GetNonCompactnessPenaltyType());
                        }
                        break;
        default : ZdException::Generate("Unknown window shape type %d.\n", "PrintSpatialWindowParameters()", gParameters.GetSpatialWindowType());
      }
    }
    if (gParameters.GetAnalysisType() == PURELYSPATIAL)
      fprintf(fp, "  Isotonic Scan                         : %s\n", (gParameters.GetRiskType() == MONOTONERISK ? "Yes" : "No"));
  }
  catch (ZdException &x) {
    x.AddCallpath("PrintSpatialWindowParameters()","ParametersPrint");
    throw;
  }
}

/** Prints 'System' parameters to file stream. */
void ParametersPrint::PrintSystemParameters(FILE* fp) const {
  const CParameters::CreationVersion  & IniVersion = gParameters.GetCreationVersion();
  CParameters::CreationVersion          Current = {atoi(VERSION_MAJOR), atoi(VERSION_MINOR), atoi(VERSION_RELEASE)};

  if (IniVersion.iMajor != Current.iMajor ||
      IniVersion.iMinor != Current.iMinor ||
      IniVersion.iRelease != Current.iRelease) {
    fprintf(fp, "\nSystem\n------\n");
    fprintf(fp, "  Parameters Version : %u.%u.%u\n", IniVersion.iMajor, IniVersion.iMinor, IniVersion.iRelease);
  }
}

/** Prints 'Spatial Window' tab parameters to file stream. */
void ParametersPrint::PrintTemporalWindowParameters(FILE* fp) const {
  std::string sBuffer;

  try {
    if (gParameters.GetAnalysisType() == PURELYSPATIAL || gParameters.GetAnalysisType() == SPATIALVARTEMPTREND)
      return;

    fprintf(fp, "\nTemporal Window\n---------------\n");
    fprintf(fp, "  Maximum Temporal Cluster Size         : %g", gParameters.GetMaximumTemporalClusterSize());
    switch (gParameters.GetMaximumTemporalClusterSizeType()) {
      case PERCENTAGETYPE : fprintf(fp, "%% of study period\n"); break;
      case TIMETYPE       : fprintf(fp, " %s\n",
                            GetDatePrecisionAsString(gParameters.GetTimeAggregationUnitsType(), sBuffer, gParameters.GetMaximumTemporalClusterSize() != 1, true)); break;
      default : ZdException::Generate("Unknown maximum temporal cluster size type '%d'.\n",
                                      "PrintTemporalWindowParameters()", gParameters.GetMaximumTemporalClusterSizeType());
    }
    if (gParameters.GetProbabilityModelType() != SPACETIMEPERMUTATION && gParameters.GetIsSpaceTimeAnalysis()) {
      fprintf(fp, "  Include Purely Spatial Clusters       : ");
      fprintf(fp, (gParameters.GetIncludePurelySpatialClusters() ? "Yes\n" : "No\n"));
    }
    if (gParameters.GetAnalysisType() == PURELYTEMPORAL || gParameters.GetAnalysisType() == SPACETIME) {
      switch (gParameters.GetIncludeClustersType()) {
         case ALIVECLUSTERS   : fprintf(fp, "  Clusters to Include                   : ");
                                fprintf(fp, "Only those including the study end date\n"); break;
         case ALLCLUSTERS     : /*fprintf(fp, "All\n");
                                  -- geIncludeClustersType parameter no longer visible in GUI,
                                     defaulted to ALLCLUSTERS, so don't print setting */ break;
         case CLUSTERSINRANGE : fprintf(fp, "  Flexible Temporal Window Definition   : Start time from %s to %s.\n",
                                        gParameters.GetStartRangeStartDate().c_str(),
                                        gParameters.GetStartRangeEndDate().c_str());
                                fprintf(fp, "                                          End time from %s to %s.\n",
                                        gParameters.GetEndRangeStartDate().c_str(), gParameters.GetEndRangeEndDate().c_str()); break;
         default : ZdException::Generate("Unknown inclusion cluster type '%d'.\n",
                                         "PrintTemporalWindowParameters()", gParameters.GetIncludeClustersType());
      };
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("PrintTemporalWindowParameters()","ParametersPrint");
    throw;
  }
}


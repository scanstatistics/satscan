//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "cluster.h"
#include "OrdinalLikelihoodCalculation.h"
#include "CategoricalClusterData.h"
#include "NormalClusterData.h"
#include "UniformTimeClusterData.h"
#include "AbstractAnalysis.h"
#include "SSException.h"
#include <iostream>
#include <fstream>
#include "WeightedNormalRandomizer.h"
#include "ClusterSupplement.h"
#include "LoglikelihoodRatioUnifier.h"
#include "RankRandomizer.h"
#include "GisUtils.h"

unsigned int CCluster::MIN_RANK_RPT_GUMBEL = 10;

/** constructor */
CCluster::CCluster() {
  Initialize();
}

/** destructor */
CCluster::~CCluster() {}

/** initializes cluster data  */
void CCluster::Initialize(tract_t nCenter) {
  m_Center = nCenter;
  _central_identifier = -1;
  _num_identifiers = 0;
  m_CartesianRadius= -1;
  m_nRatio         = 0;
  m_nRank          = 1;
  m_NonCompactnessPenalty = 1;
  m_nFirstInterval = 0;
  m_nLastInterval  = 0;
  m_iEllipseOffset = 0;
  gpCachedReportLines = 0;
  _gini_cluster = false;
  _hierarchical_cluster = false;
  _ratio_sets.clear();
  _span_of_locations = -1;
}

/** overloaded assignment operator */
CCluster& CCluster::operator=(const CCluster& rhs) {
  m_Center                = rhs.m_Center;
  _central_identifier  = rhs._central_identifier;
  _num_identifiers = rhs._num_identifiers;
  m_CartesianRadius       = rhs.m_CartesianRadius;
  m_nRatio                = rhs.m_nRatio;
  _ratio_sets             = rhs._ratio_sets;
  m_nRank                 = rhs.m_nRank;
  m_NonCompactnessPenalty = rhs.m_NonCompactnessPenalty;
  m_nFirstInterval        = rhs.m_nFirstInterval;
  m_nLastInterval         = rhs.m_nLastInterval;
  m_iEllipseOffset        = rhs.m_iEllipseOffset;
  if (rhs.gpCachedReportLines) {
      gpCachedReportLines = new ReportCache_t(*rhs.gpCachedReportLines);
  }
  _gini_cluster = rhs._gini_cluster;
  _hierarchical_cluster = rhs._hierarchical_cluster;
  _span_of_locations = rhs._span_of_locations;
  return *this;
}

/** Adds report line to cluster report cache. */
void CCluster::cacheReportLine(std::string& label, std::string& value, unsigned int setIdx) const {
  if (!gpCachedReportLines)
      gpCachedReportLines = new ReportCache_t();
  gpCachedReportLines->push_back(std::make_pair(label, std::make_pair(value, setIdx) ));
}

/** returns the area rate for cluster */ 
AreaRateType CCluster::getAreaRateForCluster(const CSaTScanData& DataHub) const {
    const CParameters& parameters = DataHub.GetParameters();

    if (parameters.GetProbabilityModelType() == CATEGORICAL) {
        // There is no concept of high versus low clusters with Multinomial model ... all clusters are high.
        return HIGH; 
    } else if (parameters.GetAreaScanRateType() != HIGHANDLOW && !parameters.getIsWeightedNormalCovariates()) {
        // If not scanning for both high and low rates simultaneously, just use the parameter settings.
        return parameters.GetAreaScanRateType();
    } else if (DataHub.GetNumDataSets() > 1 && parameters.GetMultipleDataSetPurposeType() == ADJUSTMENT) {
        // When we do the adjustments with multiple data sets, some data sets in a cluster could have O/E>1
        // and other data sets O/E<1. The cluster is still either high or low but we need to determine 
        // whether the intermediate ratio is positive (high) or negative (low) in the unifier object.
        std::auto_ptr<AbstractLikelihoodCalculator> Calculator(AbstractAnalysis::GetNewLikelihoodCalculator(DataHub));
        GetClusterData()->getRatioUnified(*Calculator);
        const AdjustmentUnifier * pUnifier = 0;
        if ((pUnifier = dynamic_cast<const AdjustmentUnifier*>(&Calculator->GetUnifier())) == 0)
            throw prg_error("Cluster data object could not be dynamically casted to AbstractCategoricalClusterData type.\n", "getAreaRateForCluster()");
        return pUnifier->GetRawLoglikelihoodRatio() >= 0 ? HIGH : LOW;
    } else {
        // For a multivariate analysis with multiple data sets, all data sets in a cluster should either
        // have O/E>1 or all of them should have O/E<1.
        switch (parameters.GetProbabilityModelType()) {
            case POISSON :
            case BERNOULLI :
            case SPACETIMEPERMUTATION :
            case HOMOGENEOUSPOISSON:
            case EXPONENTIAL:
            case RANK:
            case UNIFORMTIME:
                // Cluster is high rate if observed / expected > 1.0 and is low rate if observed / expected < 1.0.
                return GetObservedDivExpected(DataHub) >= 1.0 ? HIGH : LOW;
            case ORDINAL: {
                // Several Obs/Exp values are provided in the output file. It is high if the Obs/Exp are presented in increasing order, otherwise low.
                const AbstractCategoricalClusterData * pClusterData = 0;
                if ((pClusterData = dynamic_cast<const AbstractCategoricalClusterData*>(GetClusterData())) == 0)
                    throw prg_error("Cluster data object could not be dynamically casted to AbstractCategoricalClusterData type.\n", "getAreaRateForCluster()");                
                bool increasing = true;
                OrdinalLikelihoodCalculator Calculator(DataHub);
                DataSetIndexes_t setIndexes(getDataSetIndexesComprisedInRatio(DataHub));
                for (DataSetIndexes_t::const_iterator itr_Index=setIndexes.begin(); itr_Index != setIndexes.end(); ++itr_Index) {
                    //retrieve collection of ordinal categories in combined state
                    std::vector<OrdinalCombinedCategory> categories;
                    pClusterData->GetOrdinalCombinedCategories(Calculator, categories, *itr_Index);
                    //if container is empty, data set did not contribute to the loglikelihood ratio, so skip reporting it
                    if (!categories.size()) continue;
                    // iterate through the data sets' categories to determine if ode is increasing or decreasing
                    measure_t ode=0;
                    for (std::vector<OrdinalCombinedCategory>::iterator itrCategory=categories.begin(); itrCategory != categories.end(); ++itrCategory) {
                        count_t tObserved=0;
                        measure_t tExpected=0;
                        for (size_t m=0; m < itrCategory->GetNumCombinedCategories(); ++m) {
                            tObserved += GetObservedCountOrdinal(*itr_Index, itrCategory->GetCategoryIndex(m));
                            tExpected += GetExpectedCountOrdinal(DataHub, *itr_Index, itrCategory->GetCategoryIndex(m));
                        }
                        if (categories.size()  == 1) {
                            // all categories combined into one
                            return (measure_t)tObserved/tExpected >= 1.0 ? HIGH : LOW;
                        } else if (itrCategory == categories.begin()) {
                            // first time through, store ode for subsequent comparison
                            ode = (measure_t)tObserved/tExpected; 
                        } else {
                            // compare previous ode with current ode, then break loop
                            increasing = (measure_t)tObserved/tExpected >= ode;
                            break;
                        }
                    }
                    // break loop, we found a category with relevant information in this data set
                    break;
                }
                return increasing ? HIGH : LOW;
            }
            case NORMAL: {
                // High if the mean inside the cluster is higher than the mean outside the cluster, otherwise low.
                const DataSetHandler& handler = DataHub.GetDataSetHandler();
                DataSetIndexes_t setIndexes(getDataSetIndexesComprisedInRatio(DataHub));
                unsigned int firstSetIdx = *(setIndexes.begin());
                double meanInside=0, meanOutside=0;
                if (DataHub.GetParameters().getIsWeightedNormal()) {
                    const AbstractWeightedNormalRandomizer * pRandomizer=0;
                    std::vector<tract_t> identifierIndexes;
                    getIdentifierIndexes(DataHub, identifierIndexes, true);
                    AbstractWeightedNormalRandomizer::ClusterStatistics statistics;

                    if ((pRandomizer = dynamic_cast<const AbstractWeightedNormalRandomizer*>(handler.GetRandomizer(firstSetIdx))) == 0)
                        throw prg_error("Randomizer could not be dynamically casted to AbstractWeightedNormalRandomizer type.\n", "getAreaRateForCluster()");
                    statistics = pRandomizer->getClusterStatistics(m_nFirstInterval, m_nLastInterval, identifierIndexes);
                    meanInside = statistics.gtMeanIn;
                    meanOutside = statistics.gtMeanOut;
                } else {
                    count_t tObserved = GetObservedCount(firstSetIdx);
                    measure_t tExpected = GetExpectedCount(DataHub, firstSetIdx);

                    meanInside = (tObserved ? tExpected/tObserved : 0);
                    count_t tCasesOutside = handler.GetDataSet(firstSetIdx).getTotalCases() - tObserved;
                    meanOutside = (tCasesOutside ? (handler.GetDataSet(firstSetIdx).getTotalMeasure() - tExpected)/tCasesOutside : 0);
                }
                return meanInside >= meanOutside ? HIGH : LOW;
            }
            default: throw prg_error("Unknown probability model type '%d'.", "getAreaRateForCluster()", parameters.GetProbabilityModelType());
        }
    }
}

CCluster::ReportCache_t & CCluster::getReportLinesCache() const {
  if (!gpCachedReportLines)
      gpCachedReportLines = new ReportCache_t();
  return *gpCachedReportLines;
}

/** prints cluster information to file stream, via AsciiPrintFormat object. If 'saveToCache' is true, adds label/value to cache.

    Caching the data provides a mechanism to store cluster information to be displayed later, in other types of reports (e.g. KML file).
    The logic that creates the label/value pairs can be involved. Rather than repeating this logic in other classes, this function stores
    the result of that logic in a cache for later retrieval.

    The fields currently marked for caching are specific to reporting in the KML file. This caching implementation is not ideal, but I don't
    have a better solution at the moment. When/or if we have another cluster report file that needs to report a different collection of fields,
    we'll need to come up with a better solution.
*/
void CCluster::printClusterData(FILE* fp, const AsciiPrintFormat& PrintFormat, const char * label, std::string& value, bool saveToCache, unsigned int setIdx) const {
    PrintFormat.PrintSectionLabel(fp, label, false, true);
    PrintFormat.PrintAlignedMarginsDataString(fp, value);
    if (saveToCache) {
        std::string s(label);
        cacheReportLine(s, value, setIdx);
    }
}

/** converts passed angle to degrees */
const double CCluster::ConvertAngleToDegrees(double dAngle) const {
    double degrees = 180.00 * (dAngle / (double)M_PI);
    return degrees > 90.00 ? degrees - 180.00 : degrees;
}

/** Calls class cluster data object, deallocating any class members that were used to
    iterate through data during evaluation process. */
void CCluster::DeallocateEvaluationAssistClassMembers() {
  GetClusterData()->DeallocateEvaluationAssistClassMembers();
}

/** Writes cluster properties to file stream in format required by result output file  */
void CCluster::Display(FILE* fp, const CSaTScanData& DataHub, const ClusterSupplementInfo& supplementInfo, const SimulationVariables& simVars) const {
    try {
        AsciiPrintFormat PrintFormat = getAsciiPrintFormat();
        std::string buffer, work;
        unsigned int iReportedCluster = supplementInfo.getClusterReportIndex(*this);

        PrintFormat.SetMarginsAsClusterSection(iReportedCluster);
        fprintf(fp, "%u.", iReportedCluster);
        DisplayCensusTracts(fp, DataHub, PrintFormat);
        if (DataHub.GetParameters().getClusterMonikerPrefix().size()) {
            printString(buffer, "%sC%u", DataHub.GetParameters().getClusterMonikerPrefix().c_str(), iReportedCluster);
            PrintFormat.PrintSectionLabel(fp, "Moniker", false, true);
            PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
        }
        if (DataHub.GetParameters().getReportGiniOptimizedClusters()) {
            if (supplementInfo.getOverlappingClusters(*this, buffer).size() == 0) buffer = "No Overlap";
            printClusterData(fp, PrintFormat, "Overlap with clusters", buffer, false);
        }

		// Display cluster coordinates but not when:
		// - using non-euclidean neighbors
		// - using a network file to define locations and the user didn't specify all the coordinates in coordinates file
        if (!(DataHub.GetParameters().UseLocationNeighborsFile() || GetClusterType() == PURELYTEMPORALCLUSTER ||
			  (DataHub.GetParameters().getUseLocationsNetworkFile() && !DataHub.networkCanReportLocationCoordinates()))) {
            if (DataHub.GetParameters().GetCoordinatesType() == CARTESIAN)
                DisplayCoordinates(fp, DataHub, PrintFormat);
            else
                DisplayLatLongCoords(fp, DataHub, PrintFormat);
            double span = getLocationsSpan(DataHub);
            buffer = "N/A";
            printClusterData(
                fp, PrintFormat, "Span", span >= 0.0 ? printString(buffer, "%s%s", getValueAsString(span, work).c_str(), DataHub.GetParameters().GetCoordinatesType() == LATLON ? " km" : "") : buffer, false
            );
        }
        if (DataHub.GetParameters().getReportGiniOptimizedClusters()) {
            buffer = isGiniCluster() ? "Yes" : "No";
            printClusterData(fp, PrintFormat, "Gini Cluster", buffer, false);
        }
        DisplayTimeFrame(fp, DataHub, PrintFormat);
        if (DataHub.GetParameters().GetProbabilityModelType() == ORDINAL || DataHub.GetParameters().GetProbabilityModelType() == CATEGORICAL)
            DisplayClusterDataOrdinal(fp, DataHub, PrintFormat);
        else if (DataHub.GetParameters().GetProbabilityModelType() == EXPONENTIAL)
            DisplayClusterDataExponential(fp, DataHub, PrintFormat);
        else if (DataHub.GetParameters().GetProbabilityModelType() == NORMAL) {
            if (DataHub.GetParameters().getIsWeightedNormal())
                DisplayClusterDataWeightedNormal(fp, DataHub, PrintFormat);
            else
                DisplayClusterDataNormal(fp, DataHub, PrintFormat);
        } else if (DataHub.GetParameters().GetProbabilityModelType() == RANK) {
            DisplayClusterDataRank(fp, DataHub, PrintFormat);
        } else
            DisplayClusterDataStandard(fp, DataHub, PrintFormat);
        DisplayTimeTrend(fp, DataHub, PrintFormat);
        DisplayRatio(fp, DataHub, PrintFormat);
        DisplayMonteCarloInformation(fp, DataHub, iReportedCluster, PrintFormat, simVars);
    } catch (prg_exception& x) {
        x.addTrace("Display()","CCluster");
        throw;
    }
}

/** Prints annual cases to file stream is in format required by result output file. */
void CCluster::DisplayAnnualCaseInformation(FILE* fp, unsigned int iDataSetIndex, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  std::string buffer, buffer2;
  const CParameters& parameters = DataHub.GetParameters();

  if (parameters.GetProbabilityModelType() == POISSON && parameters.UsePopulationFile() && parameters.GetTimeTrendAdjustmentType() != TEMPORAL_STRATIFIED_RANDOMIZATION) {
    printString(buffer, "Annual cases / %.0f", DataHub.GetAnnualRatePop());
    buffer2 = getValueAsString(DataHub.GetAnnualRateAtStart(iDataSetIndex) * GetObservedDivExpected(DataHub, iDataSetIndex), buffer2, 1);
    printClusterData(fp, PrintFormat, buffer.c_str(), buffer2, false);
  }
}

/** Writes cluster location identifiers to file stream in format required by result output file  */
void CCluster::DisplayCensusTracts(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const {
    std::stringstream text;
    try {
        PrintFormat.PrintSectionLabel(fp, "Location IDs included", false, false);
        std::vector<tract_t> clusterLocations;
        if (Data.GetParameters().GetMultipleCoordinatesType() == ONEPERLOCATION) {
            Identifier::CombinedIdentifierNames_t identifiers;
            for (tract_t t = 1; t <= _num_identifiers; ++t) {
                tract_t tTract = Data.GetNeighbor(m_iEllipseOffset, m_Center, t, m_CartesianRadius);
                if (!Data.isNullifiedIdentifier(tTract)) {
                    Data.getIdentifierInfo().retrieveAll(tTract, identifiers);
                    for (unsigned int i = 0; i < identifiers.size(); ++i) {
                        if (text.rdbuf()->in_avail()) text << ", ";
                        text << identifiers[i];
                    }
                }
            }
        } else {
            const auto& locationData = Data.getLocationsManager().locations();
            CentroidNeighborCalculator::getLocationsAboutCluster(Data, *this, 0, &clusterLocations);
            for (auto index : clusterLocations) {
                if (text.rdbuf()->in_avail()) text << ", ";
                text << locationData[index].get()->name();
            }
        }
        PrintFormat.PrintAlignedMarginsDataString(fp, text.str());
    } catch (prg_exception& x) {
        x.addTrace("DisplayCensusTracts()","CCluster");
        throw;
    }
}

/** Prints observed cases, expected cases and observed/expected, for exponetial model,
    to file stream is in format required by result output file. */
void CCluster::DisplayClusterDataExponential(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  std::string buffer;
  DataSetIndexes_t setIndexes(getDataSetIndexesComprisedInRatio(DataHub));
  
  for (DataSetIndexes_t::const_iterator itr=setIndexes.begin(); itr != setIndexes.end(); ++itr) {
     unsigned int set_number = *itr + 1;
     //print data set number if analyzing more than data set
     if (DataHub.GetParameters().getNumFileSets() > 1) {
       set_number = DataHub.GetDataSetHandler().getDataSetRelativeIndex(*itr) + 1;
       printString(buffer, "Data Set #%ld", set_number);
       PrintFormat.PrintSectionLabelAtDataColumn(fp, buffer.c_str());
     }
     //print total individuals (censored and non-censored)
     GetPopulationAsString(buffer, DataHub.GetProbabilityModel().GetPopulation(*itr, *this, DataHub));
     printClusterData(fp, PrintFormat, "Total individuals", buffer, true, set_number);
     //print total cases (non-censored)
     printClusterData(fp, PrintFormat, "Number of cases", printString(buffer, "%ld", GetObservedCount(*itr)), true, set_number);
     //print expected cases
     printClusterData(fp, PrintFormat, "Expected cases", getValueAsString(GetExpectedCount(DataHub, *itr), buffer), true, set_number);
     DisplayObservedDivExpected(fp, *itr, DataHub, PrintFormat);
     //not printing censored information at Martin's directive, but leave in place for now
     ////print total censored cases
     //GetPopulationAsString(sBuffer, DataHub.GetProbabilityModel().GetPopulation(*itr, *this, DataHub) - GetObservedCount(*itr));
     //printClusterData(fp, PrintFormat, "Number censored cases", buffer, false);

     //NOTE: Not printing relative risk information for exponential model at this time.
  }
}

/** Prints observed cases, expected cases and observed/expected, for rank model,
    to file stream is in format required by result output file. */
void CCluster::DisplayClusterDataRank(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
    std::string buffer, work;
    double n1, n2, r1, r2;
    DataSetIndexes_t setIndexes(getDataSetIndexesComprisedInRatio(DataHub));
    const DataSetHandler& Handler = DataHub.GetDataSetHandler();

    for (DataSetIndexes_t::const_iterator itr=setIndexes.begin(); itr != setIndexes.end(); ++itr) {
        unsigned int set_number = *itr + 1;
        //print data set number if analyzing more than data set
        if (DataHub.GetParameters().getNumFileSets() > 1) {
            set_number = DataHub.GetDataSetHandler().getDataSetRelativeIndex(*itr) + 1;
            printString(buffer, "Data Set #%ld", set_number);
            PrintFormat.PrintSectionLabelAtDataColumn(fp, buffer.c_str());
        }
        //print total cases
        printClusterData(fp, PrintFormat, "Number of cases", printString(buffer, "%ld", GetObservedCount(*itr)), true, set_number);
        n1 = static_cast<double>(GetObservedCount(*itr));
        n2 = static_cast<double>(Handler.GetDataSet(*itr).getTotalCases()) - n1;
        r1 = GetExpectedCount(DataHub, *itr);
        r2 = Handler.GetDataSet(*itr).getTotalMeasure() - r1;
        printClusterData(fp, PrintFormat, "Average Rank Inside", printString(buffer, "%lf", (r1 + 1)/ n1), true, set_number);
        printClusterData(fp, PrintFormat, "Average Rank Outside", printString(buffer, "%lf", (r2 + 1) / n2), true, set_number);
        //get randomizer for data set to retrieve various information
        //const AbstractRankRandomizer                * pRandomizer = 0;
        //if ((pRandomizer = dynamic_cast<const AbstractRankRandomizer*>(Handler.GetRandomizer(*itr))) == 0)
        //    throw prg_error("Randomizer could not be dynamically casted to AbstractRankRandomizer type.\n", "DisplayClusterDataRank()");
        //printClusterData(fp, PrintFormat, "Average Category", printString(buffer, "%lf", pRandomizer->getAverageAtributeValue()), true, *itr_Index + 1);

        /*
        TODO:
        Consider implementing something similar to the AbstractWeightedNormalRandomizer methods
        ClusterStatistics          getClusterStatistics(int iIntervalStart, int iIntervalEnd, const std::vector<tract_t>& vTracts) const;
        ClusterLocationStatistics  getClusterLocationStatistics(int iIntervalStart, int iIntervalEnd, const std::vector<tract_t>& vTracts) const;
        */

        buffer = "";
        printClusterData(fp, PrintFormat, "Average Category", buffer, true, set_number);
        printClusterData(fp, PrintFormat, "Median Rank Inside", buffer, true, set_number);
        printClusterData(fp, PrintFormat, "Median Rank Outside", buffer, true, set_number);
        printClusterData(fp, PrintFormat, "Variance", buffer, false);
        printClusterData(fp, PrintFormat, "Standard deviation", buffer, false);
    }
}

/** Prints observed cases, expected cases and observed/expected, for Normal model,
    to file stream is in format required by result output file. */
void CCluster::DisplayClusterDataNormal(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  std::string buffer, work;
  double dEstimatedMeanInside, dEstimatedMeanOutside, dUnbiasedVariance;
  count_t tObserved;
  measure_t tExpected;
  const DataSetHandler& Handler = DataHub.GetDataSetHandler();
  DataSetIndexes_t setIndexes(getDataSetIndexesComprisedInRatio(DataHub));

  const AbstractNormalClusterData * pClusterData = 0;
  if ((pClusterData = dynamic_cast<const AbstractNormalClusterData*>(GetClusterData())) == 0)
    throw prg_error("Cluster data object could not be dynamically casted to AbstractNormalClusterData type.\n",
                    "DisplayClusterDataNormal()");
  
  for (DataSetIndexes_t::const_iterator itr=setIndexes.begin(); itr != setIndexes.end(); ++itr) {
     unsigned int set_number = *itr + 1;
     //print data set number if analyzing more than data set
     if (DataHub.GetParameters().getNumFileSets() > 1) {
       set_number = DataHub.GetDataSetHandler().getDataSetRelativeIndex(*itr) + 1;
       printString(buffer, "Data Set #%ld", set_number);
       PrintFormat.PrintSectionLabelAtDataColumn(fp, buffer.c_str());
     }
     //print total cases
     printClusterData(fp, PrintFormat, "Number of cases", printString(buffer, "%ld", GetObservedCount(*itr)), true, set_number);
     //print estimated mean inside
     tObserved = GetObservedCount(*itr);
     tExpected = GetExpectedCount(DataHub, *itr);
     dEstimatedMeanInside = (tObserved ? tExpected/tObserved : 0);
     printClusterData(fp, PrintFormat, "Mean inside", getValueAsString(dEstimatedMeanInside, buffer), true, set_number);
     //print estimated mean inside
     count_t tCasesOutside = Handler.GetDataSet(*itr).getTotalCases() - tObserved;
     dEstimatedMeanOutside = (tCasesOutside ? (Handler.GetDataSet(*itr).getTotalMeasure() - tExpected)/tCasesOutside : 0);
     printClusterData(fp, PrintFormat, "Mean outside", getValueAsString(dEstimatedMeanOutside, buffer), true, set_number);
     //print unexplained variance
     dUnbiasedVariance = GetUnbiasedVariance(GetObservedCount(*itr), GetExpectedCount(DataHub, *itr), pClusterData->GetMeasureAux(*itr),
                                             Handler.GetDataSet(*itr).getTotalCases(), Handler.GetDataSet(*itr).getTotalMeasure(),
                                             Handler.GetDataSet(*itr).getTotalMeasureAux());
     printClusterData(fp, PrintFormat, "Variance", getValueAsString(dUnbiasedVariance, buffer), false);
     //print common standard deviation
     buffer = getValueAsString(std::sqrt(dUnbiasedVariance), buffer);
     printClusterData(fp, PrintFormat, "Standard deviation", getValueAsString(std::sqrt(dUnbiasedVariance), buffer), false);
  }
}

/** Prints observed cases, expected cases and observed/expected, for Ordinal model,
    to file stream is in format required by result output file. */
void CCluster::DisplayClusterDataOrdinal(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  std::string work, buffer, work2;
  double dTotalCasesInClusterDataSet=0;
  OrdinalLikelihoodCalculator Calculator(DataHub);
  DataSetIndexes_t setIndexes(getDataSetIndexesComprisedInRatio(DataHub));

  const AbstractCategoricalClusterData * pClusterData = 0;
  if ((pClusterData = dynamic_cast<const AbstractCategoricalClusterData*>(GetClusterData())) == 0)
    throw prg_error("Cluster data object could not be dynamically casted to AbstractCategoricalClusterData type.\n", "DisplayClusterDataOrdinal()");

  for (DataSetIndexes_t::const_iterator itr=setIndexes.begin(); itr != setIndexes.end(); ++itr) {
     //retrieve collection of ordinal categories in combined state
     std::vector<OrdinalCombinedCategory> vCategoryContainer;
     pClusterData->GetOrdinalCombinedCategories(Calculator, vCategoryContainer, *itr);
     //if container is empty, data set did not contribute to the loglikelihood ratio, so skip reporting it
     if (!vCategoryContainer.size())
       continue;
     unsigned int set_number = *itr + 1;
     //print data set number if analyzing more than data set
     if (DataHub.GetParameters().getNumFileSets() > 1) {
       set_number = DataHub.GetDataSetHandler().getDataSetRelativeIndex(*itr) + 1;
       printString(buffer, "Data Set #%ld", set_number);
       PrintFormat.PrintSectionLabelAtDataColumn(fp, buffer.c_str());
     }
     //print total cases per data set
     dTotalCasesInClusterDataSet = DataHub.GetProbabilityModel().GetPopulation(*itr, *this, DataHub);
     printClusterData(fp, PrintFormat, "Total cases", GetPopulationAsString(buffer, dTotalCasesInClusterDataSet), true, set_number);

     //print category ordinal values
     const RealDataSet& thisDataSet = DataHub.GetDataSetHandler().GetDataSet(*itr);
     buffer = "";
     for (std::vector<OrdinalCombinedCategory>::iterator itrC=vCategoryContainer.begin(); itrC != vCategoryContainer.end(); ++itrC) {
       buffer += (itrC == vCategoryContainer.begin() ? "" : ", ");
       for (size_t m=0; m < itrC->GetNumCombinedCategories(); ++m) {
         printString(work, "%s%s%s",
                      (m == 0 ? "[" : ", "),
                      thisDataSet.getPopulationData().GetCategoryTypeLabel(itrC->GetCategoryIndex(m)).c_str(),
                      (m + 1 == itrC->GetNumCombinedCategories() ? "]" : ""));
         buffer += work;
       }
     }
     printClusterData(fp, PrintFormat, "Category", buffer, true, set_number);

     //print observed case data per category
     buffer = "";
     for (std::vector<OrdinalCombinedCategory>::iterator itrC=vCategoryContainer.begin(); itrC != vCategoryContainer.end(); ++itrC) {
       count_t tObserved=0;
       for (size_t m=0; m < itrC->GetNumCombinedCategories(); ++m)
          tObserved += GetObservedCountOrdinal(*itr, itrC->GetCategoryIndex(m));
       printString(work, "%s%ld", (itrC == vCategoryContainer.begin() ? "" : ", "), tObserved);
       buffer += work;
     }
     printClusterData(fp, PrintFormat, "Number of cases", buffer, true, set_number);
     //print expected case data per category
     buffer = "";
     for (std::vector<OrdinalCombinedCategory>::iterator itrC=vCategoryContainer.begin(); itrC != vCategoryContainer.end(); ++itrC) {
       measure_t tExpected=0;
       for (size_t m=0; m < itrC->GetNumCombinedCategories(); ++m)
          tExpected += GetExpectedCountOrdinal(DataHub, *itr, itrC->GetCategoryIndex(m));
       work2 = getValueAsString(tExpected, work2); 
       printString(work, "%s%s", (itrC == vCategoryContainer.begin() ? "" : ", "), work2.c_str());
       buffer += work;
     }
     printClusterData(fp, PrintFormat, "Expected cases", buffer, true, set_number);
     //print observed div expected case data per category
     buffer = "";
     for (std::vector<OrdinalCombinedCategory>::iterator itrC=vCategoryContainer.begin(); itrC != vCategoryContainer.end(); ++itrC) {
       count_t   tObserved=0;
       measure_t tExpected=0;
       for (size_t m=0; m < itrC->GetNumCombinedCategories(); ++m) {
          tObserved += GetObservedCountOrdinal(*itr, itrC->GetCategoryIndex(m));
          tExpected += GetExpectedCountOrdinal(DataHub, *itr, itrC->GetCategoryIndex(m));
       }
       work2 = getValueAsString((double)tObserved/tExpected, work2); 
       printString(work, "%s%s", (itrC == vCategoryContainer.begin() ? "" : ", "), work2.c_str());
       buffer += work;
     }
     printClusterData(fp, PrintFormat, "Observed / expected", buffer, false);
     //print relative data - note that we will possibly be combining categories
     buffer = "";
     for (std::vector<OrdinalCombinedCategory>::iterator itrC=vCategoryContainer.begin(); itrC != vCategoryContainer.end(); ++itrC) {
       double           tRelativeRisk=0;
       count_t          tObserved=0, tTotalCategoryCases=0;
       measure_t        tExpected=0;
       for (size_t m=0; m < itrC->GetNumCombinedCategories(); ++m) {
          tObserved += GetObservedCountOrdinal(*itr, itrC->GetCategoryIndex(m));
          tExpected += GetExpectedCountOrdinal(DataHub, *itr, itrC->GetCategoryIndex(m));
          tTotalCategoryCases += DataHub.GetDataSetHandler().GetDataSet(*itr).getPopulationData().GetNumCategoryTypeCases(itrC->GetCategoryIndex(m));
       }
       if ((tRelativeRisk = GetRelativeRisk(tObserved, tExpected, tTotalCategoryCases, tTotalCategoryCases)) == -1)
         printString(work, "%sinfinity", (itrC == vCategoryContainer.begin() ? "" : ", "));
       else {
         work2 = getValueAsString(tRelativeRisk, work2); 
         printString(work, "%s%s", (itrC == vCategoryContainer.begin() ? "" : ", "), work2.c_str());
       }
       buffer += work;
     }
     printClusterData(fp, PrintFormat, "Relative risk", buffer, true, set_number);
     //print percent cases in area per category
     buffer = "";
     for (std::vector<OrdinalCombinedCategory>::iterator itrC=vCategoryContainer.begin(); itrC != vCategoryContainer.end(); ++itrC) {
       count_t tObserved=0;
       for (size_t m=0; m < itrC->GetNumCombinedCategories(); ++m)
          tObserved += GetObservedCountOrdinal(*itr, itrC->GetCategoryIndex(m));
       printString(
           work, "%s%s", (itrC == vCategoryContainer.begin() ? "" : ", "), 
           getValueAsString(dTotalCasesInClusterDataSet ? 100.0 * tObserved / dTotalCasesInClusterDataSet : 0.0, work2, 1).c_str()
       );
       buffer += work;
     }     
     printClusterData(fp, PrintFormat, "Percent cases in area", buffer, true, set_number);
  }
}

/** Prints population, observed cases, expected cases and relative risk
    to file stream is in format required by result output file. */
void CCluster::DisplayClusterDataStandard(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  std::string buffer;
  DataSetIndexes_t setIndexes(getDataSetIndexesComprisedInRatio(DataHub));

  DisplayPopulation(fp, DataHub, PrintFormat);
  for (DataSetIndexes_t::const_iterator itr=setIndexes.begin(); itr != setIndexes.end(); ++itr) {
     unsigned int set_number = *itr + 1;
     //print data set number if analyzing more than data set
     if (DataHub.GetParameters().getNumFileSets() > 1) {
       set_number = DataHub.GetDataSetHandler().getDataSetRelativeIndex(*itr) + 1;
       printString(buffer, "Data Set #%ld", set_number);
       PrintFormat.PrintSectionLabelAtDataColumn(fp, buffer.c_str());
     }
     //print observed cases
     printClusterData(fp, PrintFormat, "Number of cases", printString(buffer, "%ld", GetObservedCount(*itr)), true, set_number);
     //print expected cases
     printClusterData(fp, PrintFormat, "Expected cases", getValueAsString(GetExpectedCount(DataHub, *itr), buffer), true, set_number);
     DisplayAnnualCaseInformation(fp, *itr, DataHub, PrintFormat);
     DisplayObservedDivExpected(fp, *itr,DataHub, PrintFormat);
     if (DataHub.GetParameters().GetProbabilityModelType() == POISSON  || 
         DataHub.GetParameters().GetProbabilityModelType() == BERNOULLI)
       DisplayRelativeRisk(fp, *itr, DataHub, PrintFormat);
     if (DataHub.GetParameters().GetProbabilityModelType() == BERNOULLI) {
        //percent cases in an area
        double clusterSetPopulation = DataHub.GetProbabilityModel().GetPopulation(*itr, *this, DataHub);
        double percentCases = (clusterSetPopulation ? 100.0 * GetObservedCount(*itr) / clusterSetPopulation: 0.0);
        printClusterData(fp, PrintFormat, "Percent cases in area", getValueAsString(percentCases, buffer,1), true, set_number);
     }
   }
}

/** Prints observed cases, expected cases and observed/expected, for Weighted Normal model,
    to file stream is in format required by result output file. */
void CCluster::DisplayClusterDataWeightedNormal(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  std::string buffer;
  const DataSetHandler& Handler = DataHub.GetDataSetHandler();
  DataSetIndexes_t setIndexes(getDataSetIndexesComprisedInRatio(DataHub));

  std::vector<tract_t> tractIndexes;
  getIdentifierIndexes(DataHub, tractIndexes, true);
  for (auto setIdx: setIndexes) {
      //get randomizer for data set to retrieve various information
      const AbstractWeightedNormalRandomizer * pRandomizer = 0;
      if ((pRandomizer = dynamic_cast<const AbstractWeightedNormalRandomizer*>(Handler.GetRandomizer(setIdx))) == 0)
        throw prg_error("Randomizer could not be dynamically casted to AbstractWeightedNormalRandomizer type.\n", "DisplayClusterDataWeightedNormal()");

     unsigned int set_number = setIdx + 1;
     //print data set number if analyzing more than data set
     if (DataHub.GetParameters().getNumFileSets() > 1) {
       set_number = DataHub.GetDataSetHandler().getDataSetRelativeIndex(setIdx) + 1;
       printString(buffer, "Data Set #%ld", set_number);
       PrintFormat.PrintSectionLabelAtDataColumn(fp, buffer.c_str());
     }
     AbstractWeightedNormalRandomizer::ClusterStatistics statistics;
     statistics = pRandomizer->getClusterStatistics(m_nFirstInterval, m_nLastInterval, tractIndexes);

     //print total cases
     printClusterData(fp, PrintFormat, "Number of cases", printString(buffer, "%ld", statistics.gtObservations), true, set_number);
     //print total cluster weight
     printClusterData(fp, PrintFormat, "Total weights", getValueAsString(statistics.gtWeight, buffer), true, set_number);
     //print mean inside
     printClusterData(fp, PrintFormat, "Mean inside", getValueAsString(statistics.gtMeanIn, buffer), true, set_number);
     //print mean outside
     printClusterData(fp, PrintFormat, "Mean outside", getValueAsString(statistics.gtMeanOut, buffer), true, set_number);
     //print cluster variance
     printClusterData(fp, PrintFormat, "Variance", getValueAsString(statistics.gtVariance, buffer), false);
     //print cluster standard deviation
     printClusterData(fp, PrintFormat, "Standard deviation", getValueAsString(std::sqrt(statistics.gtVariance), buffer), false);
     //print weighted mean inside
     printClusterData(fp, PrintFormat, "Weighted mean inside", getValueAsString(statistics.gtWeightedMeanIn, buffer), true, set_number);
     //print weighted mean outside
     printClusterData(fp, PrintFormat, "Weighted mean outside", getValueAsString(statistics.gtWeightedMeanOut, buffer), true, set_number);
     //print cluster weighted variance
     printClusterData(fp, PrintFormat, "Weighted variance", getValueAsString(statistics.gtWeightedVariance, buffer), false);
     //print cluster standard deviation
     printClusterData(fp, PrintFormat, "Weighted std deviation", getValueAsString(std::sqrt(statistics.gtWeightedVariance), buffer), false);
  }
}

/** Writes clusters cartesian coordinates and ellipse properties (if cluster is elliptical)
    in format required by result output file. */
void CCluster::DisplayCoordinates(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const {
  std::vector<double>   vCoordinates;
  std::string           buffer, work, work2;

  try {
	 Data.GetGInfo()->retrieveCoordinates(m_Center, vCoordinates);

    //print coordinates differently when ellipses are requested
    if (Data.GetParameters().GetSpatialWindowType() == CIRCULAR)  {
      for (size_t i=0; i < vCoordinates.size() - 1; ++i) {
         buffer += printString(work, "%s%g,", (i == 0 ? "(" : ""), vCoordinates[i]);
      }
      buffer += printString(work, "%g)", vCoordinates.back());
	  if (!Data.GetParameters().getUseLocationsNetworkFile()) {
          buffer += printString(work, " / %s", getValueAsString(m_CartesianRadius, work2).c_str());
	  }
	  printClusterData(fp, PrintFormat, (Data.GetParameters().getUseLocationsNetworkFile() ? "Coordinates" : "Coordinates / radius"), buffer, false);
    } else {//print ellipse settings
      for (size_t i=0; i < vCoordinates.size() - 1; ++i) {
          buffer += printString(work, "%s%g,", (i == 0 ? "(" : "" ), vCoordinates[i]);
      }
      buffer += printString(work, "%g)", vCoordinates.back());
      printClusterData(fp, PrintFormat, "Coordinates", buffer, false);
      //print ellipse particulars
      work = getValueAsString(m_CartesianRadius, work);
      printClusterData(fp, PrintFormat, "Semiminor axis", work, false);
      work = getValueAsString(m_CartesianRadius * Data.GetEllipseShape(GetEllipseOffset()), work);
      printClusterData(fp, PrintFormat, "Semimajor axis", work, false);
      work = getValueAsString(ConvertAngleToDegrees(Data.GetEllipseAngle(m_iEllipseOffset)), work);
      printClusterData(fp, PrintFormat, "Angle (degrees)", work, false);
      work = getValueAsString(Data.GetEllipseShape(m_iEllipseOffset), work);
      printClusterData(fp, PrintFormat, "Shape", work, false);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("DisplayCoordinates()","CCluster");
    throw;
  }
}

/** Writes clusters lat/long coordinates in format required by result output file. */
void CCluster::DisplayLatLongCoords(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const {
  std::vector<double>           vCoordinates;
  std::pair<double, double>     prLatitudeLongitude;
  std::string                   buffer, work, work2;

  try {
    Data.GetGInfo()->retrieveCoordinates(m_Center, vCoordinates);
    prLatitudeLongitude = ConvertToLatLong(vCoordinates);
	printString(buffer, "(%.6f %c, %.6f %c)", 
		fabs(prLatitudeLongitude.first), prLatitudeLongitude.first >= 0 ? 'N' : 'S', 
		fabs(prLatitudeLongitude.second), prLatitudeLongitude.second >= 0 ? 'E' : 'W'
	);
	if (!Data.GetParameters().getUseLocationsNetworkFile()) {
		printString(work2, " / %s km", getValueAsString(GetLatLongRadius(), work).c_str());
		buffer += work2;
	}
	printClusterData(fp, PrintFormat, (Data.GetParameters().getUseLocationsNetworkFile() ? "Coordinates" : "Coordinates / radius"), buffer, false);
  } catch (prg_exception& x) {
    x.addTrace("DisplayLatLongCoords()","CCluster");
    throw;
  }
}

/** Writes clusters monte carlo rank and p-value in format required by result output file. */
void CCluster::DisplayMonteCarloInformation(FILE* fp, const CSaTScanData& DataHub,
                                            unsigned int iReportedCluster,
                                            const AsciiPrintFormat& PrintFormat,
                                            const SimulationVariables& simVars) const {
  std::string                      format, replicas, buffer;
  const CParameters & parameters = DataHub.GetParameters();
      
  if (simVars.get_sim_count() == 0)
      return;

  if (DataHub.GetParameters().getReportClusterRank()) {
    //PrintFormat.PrintSectionLabel(fp, "Monte Carlo rank", false, true);
    printString(buffer, "%u/%ld", m_nRank, simVars.get_sim_count() + 1);
    //fprintf(fp, buffer.c_str());
    printClusterData(fp, PrintFormat, "Monte Carlo rank", buffer, false);
  }

  if (reportablePValue(parameters,simVars)) {
    // conditionally report cluster p-value as monte carlo or gumbel
    //PrintFormat.PrintSectionLabel(fp, "P-value", false, true);
    bool bReportsDefaultGumbel = (parameters.GetAnalysisType() == PURELYSPATIAL ||
                                  parameters.GetAnalysisType() == SPACETIME || 
                                  parameters.GetAnalysisType() == PROSPECTIVESPACETIME) 
                                  && 
                                 (parameters.GetProbabilityModelType() == POISSON || 
                                  parameters.GetProbabilityModelType() == BERNOULLI ||
                                  parameters.GetProbabilityModelType() == SPACETIMEPERMUTATION
								  );
    bReportsDefaultGumbel |= parameters.GetAnalysisType() == PURELYSPATIAL &&
                            (parameters.GetProbabilityModelType() == ORDINAL || parameters.GetProbabilityModelType() == CATEGORICAL);
    if (parameters.GetPValueReportingType() == GUMBEL_PVALUE || (bReportsDefaultGumbel && parameters.GetPValueReportingType() == DEFAULT_PVALUE && GetRank() < MIN_RANK_RPT_GUMBEL)) {
      std::pair<double,double> p = GetGumbelPValue(simVars);
      if (p.first == 0.0) {
        getValueAsString(p.second, buffer, 1).insert(0, "< ");
      } else {
        getValueAsString(p.first, buffer);
      }
      printClusterData(fp, PrintFormat, "P-value", buffer, true);
    } else {
      printString(replicas, "%u", simVars.get_sim_count());
      printString(format, "%%.%dlf", replicas.size());
      printString(buffer, format.c_str(), GetMonteCarloPValue(parameters,simVars, DataHub.GetParameters().GetIsIterativeScanning() || iReportedCluster == 1));
      //fprintf(fp, buffer.c_str());
      printClusterData(fp, PrintFormat, "P-value", buffer, true);
    }
    DisplayRecurrenceInterval(fp, DataHub, iReportedCluster, simVars, PrintFormat);
    //conditionally report gumbel p-value as supplement to reported p-value
    if (parameters.GetReportGumbelPValue() &&
        (parameters.GetPValueReportingType() == STANDARD_PVALUE || parameters.GetPValueReportingType() == TERMINATION_PVALUE)) {
         //PrintFormat.PrintSectionLabel(fp, "Gumbel P-value", false, true);
         std::pair<double,double> p = GetGumbelPValue(simVars);
         if (p.first == 0.0) {
           getValueAsString(p.second, buffer, 1).insert(0, "< ");
         } else {
           getValueAsString(p.first, buffer);
         }
         printClusterData(fp, PrintFormat, "Gumbel P-value", buffer, true);
    }
  }
}

/** Calculates recurrence interval and returns pair (recurrence in years, recurrence in days). */
CCluster::RecurrenceInterval_t CCluster::GetRecurrenceInterval(const CSaTScanData& Data, unsigned int iReportedCluster, const SimulationVariables& simVars) const {
    double dIntervals, dPValue, dAdjustedP_Value, dUnitsInOccurrence;
    const CParameters & parameters(Data.GetParameters());

    if (!parameters.GetIsProspectiveAnalysis())
        throw prg_error("GetRecurrenceInterval() called for non-prospective analysis.","GetRecurrenceInterval()");
    dIntervals = static_cast<double>(Data.GetNumTimeIntervals() - Data.GetProspectiveStartIndex() + 1);
    bool bReportsDefaultGumbel = (
        parameters.GetAnalysisType() == PURELYSPATIAL || parameters.GetAnalysisType() == SPACETIME || parameters.GetAnalysisType() == PROSPECTIVESPACETIME
    ) && (
        parameters.GetProbabilityModelType() == POISSON || parameters.GetProbabilityModelType() == BERNOULLI || parameters.GetProbabilityModelType() == SPACETIMEPERMUTATION
    );
    if ((bReportsDefaultGumbel && parameters.GetPValueReportingType() == DEFAULT_PVALUE && m_nRank < MIN_RANK_RPT_GUMBEL) || parameters.GetPValueReportingType() == GUMBEL_PVALUE) {
        std::pair<double,double> p = GetGumbelPValue(simVars);
        dPValue = std::max(p.first, p.second);
        dAdjustedP_Value = std::max(1.0 - pow(1.0 - dPValue, 1.0/dIntervals),p.second);
    } else {
        dPValue = GetMonteCarloPValue(Data.GetParameters(), simVars, parameters.GetIsIterativeScanning() || iReportedCluster == 1);
        dAdjustedP_Value = 1.0 - pow(1.0 - dPValue, 1.0/dIntervals);
    }
    // Special case the recurrance interval for GENERIC units - which don't translate to the prospective frequency units.
    if (parameters.GetTimeAggregationUnitsType() == GENERIC) {
        dUnitsInOccurrence = static_cast<double>(parameters.GetTimeAggregationLength()) / dAdjustedP_Value;
        return std::make_pair(std::max(dUnitsInOccurrence, 1.0), std::max(dUnitsInOccurrence, 1.0));
    }
    // Determine the number of units in occurrence - either per time aggregation or user selection.
    ProspectiveFrequency frequency = parameters.getProspectiveFrequencyType();
    dUnitsInOccurrence = static_cast<double>(parameters.getProspectiveFrequency()) / dAdjustedP_Value;
    if (frequency == SAME_TIMEAGGREGATION) {
        dUnitsInOccurrence = static_cast<double>(parameters.GetTimeAggregationLength()) / dAdjustedP_Value;
        switch (parameters.GetTimeAggregationUnitsType()) { //translate time aggregation units to prospective frequency
            case YEAR: frequency = YEARLY; break;
            case MONTH: frequency = MONTHLY; break;
            case DAY: frequency = DAILY; break;
            default: throw prg_error("Invalid enum for time aggregation type '%d'.", "GetRecurrenceInterval()", parameters.GetTimeAggregationUnitsType());
        }
    }
    // Now calculate recurrance interval as years and days based on frequency.
    switch (frequency) {
        case YEARLY: return std::make_pair(dUnitsInOccurrence, std::max(dUnitsInOccurrence * AVERAGE_DAYS_IN_YEAR, 1.0));
        case QUARTERLY: return std::make_pair(dUnitsInOccurrence / 4.0, std::max((dUnitsInOccurrence / 4.0) * AVERAGE_DAYS_IN_YEAR, 1.0));
        case MONTHLY: return std::make_pair(dUnitsInOccurrence / 12.0, std::max((dUnitsInOccurrence / 12.0) * AVERAGE_DAYS_IN_YEAR, 1.0));
        case WEEKLY:  return std::make_pair(dUnitsInOccurrence / 52.0, std::max((dUnitsInOccurrence / 52.0) * AVERAGE_DAYS_IN_YEAR, 1.0));
        case DAILY: return std::make_pair(dUnitsInOccurrence / AVERAGE_DAYS_IN_YEAR, std::max(dUnitsInOccurrence, 1.0));
        default: throw prg_error("Invalid enum '%d' for prospective analysis frequency type.", "GetRecurrenceInterval()", parameters.getProspectiveFrequencyType());
    }
}

/** Writes clusters null occurance rate in format required by result output file. */
void CCluster::DisplayRecurrenceInterval(FILE* fp, const CSaTScanData& Data, unsigned int iReportedCluster, const SimulationVariables& simVars, const AsciiPrintFormat& PrintFormat) const {
  std::string   buffer;

  try {
      if (reportableRecurrenceInterval(Data.GetParameters(), simVars)) {
         //PrintFormat.PrintSectionLabel(fp, "Recurrence interval", false, true);
         RecurrenceInterval_t ri = GetRecurrenceInterval(Data, iReportedCluster, simVars);
         if (ri.first < 1.0) {
            printString(
                buffer, "%.0lf %s%s", ri.second, 
                Data.GetParameters().GetTimeAggregationUnitsType() == GENERIC ? "unit" : "day", (ri.second < 1.5 ? "" : "s")
            );
         } else if (ri.first <= 10.0) {
            printString(
                buffer, "%.1lf %s%s", ri.first, 
                Data.GetParameters().GetTimeAggregationUnitsType() == GENERIC ? "unit" : "year", (ri.first < 1.05 ? "" : "s")
            );
         } else {
            printString(buffer, "%.0lf %s", ri.first, Data.GetParameters().GetTimeAggregationUnitsType() == GENERIC ? "units" : "years");
         }
         //print data to file stream
         printClusterData(fp, PrintFormat, "Recurrence interval", buffer, true);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("DisplayNullOccurrence()","CCluster");
    throw;
  }
}

/** Writes clusters population in format required by result output file. */
void CCluster::DisplayPopulation(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  unsigned int           i;
  std::string            work, buffer;
  const DataSetHandler & DataSets = DataHub.GetDataSetHandler();
  double                 dPopulation;

  switch (DataHub.GetParameters().GetProbabilityModelType()) {
    case POISSON :
      if (!DataHub.GetParameters().UsePopulationFile() || GetClusterType() == PURELYTEMPORALCLUSTER)
        break;
    case BERNOULLI :
      for (i=0; i < DataSets.GetNumDataSets(); ++i) {
        dPopulation = DataHub.GetProbabilityModel().GetPopulation(i, *this, DataHub);
        if (dPopulation < .5)
          printString(work, "%s%g", (i > 0 ? ", " : ""), dPopulation); // display all decimals for populations less than .5
        else if (dPopulation < 1)
          printString(work, "%s%.1f", (i > 0 ? ", " : ""), dPopulation); // display one decimal for populations less than 1
        else
          printString(work, "%s%.0f", (i > 0 ? ", " : ""), dPopulation); // else display no decimals
        buffer += work;
      }
      printClusterData(fp, PrintFormat, "Population", buffer, true);
      break;
    default : break;
  };
}

/** Writes clusters log likelihood ratio/test statistic in format required by result output file. */
void CCluster::DisplayRatio(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  std::string  buffer;
  const CParameters& params = DataHub.GetParameters();

  if (params.GetProbabilityModelType() == SPACETIMEPERMUTATION ||
      params.GetProbabilityModelType() == RANK ||
      params.GetProbabilityModelType() == UNIFORMTIME ||
      (params.GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION)) {
     printClusterData(fp, PrintFormat, "Test statistic", printString(buffer, "%lf", m_nRatio), false);
  } else {
    printClusterData(fp, PrintFormat, "Log likelihood ratio", printString(buffer, "%lf", m_nRatio / m_NonCompactnessPenalty), false);
    if (params.GetSpatialWindowType() == ELLIPTIC) {
      printString(buffer, "%lf", m_nRatio);
      printClusterData(fp, PrintFormat, "Test statistic", buffer, false);
    }
  }
}

/** Writes clusters relative risk in format required by result output file. */
void CCluster::DisplayRelativeRisk(FILE* fp, unsigned int iDataSetIndex, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  std::string buffer;
  double      dRelativeRisk;

  //PrintFormat.PrintSectionLabel(fp, "Relative risk", false, true);
  if ((dRelativeRisk = GetRelativeRisk(DataHub, iDataSetIndex)) == -1)
    buffer = "infinity";
  else
    buffer = getValueAsString(dRelativeRisk, buffer);

  printClusterData(fp, PrintFormat, "Relative risk", buffer, true, DataHub.GetDataSetHandler().getDataSetRelativeIndex(iDataSetIndex) + 1);
}

/** Writes clusters overall relative risk in format required by result output file. */
void CCluster::DisplayObservedDivExpected(FILE* fp, unsigned int iDataSetIndex, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  std::string buffer;
  printClusterData(fp, PrintFormat, "Observed / expected", getValueAsString(GetObservedDivExpected(DataHub, iDataSetIndex), buffer), true, 
      DataHub.GetDataSetHandler().getDataSetRelativeIndex(iDataSetIndex) + 1
  );
}

/** Prints clusters time frame in format required by result output file. */
void CCluster::DisplayTimeFrame(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  std::string  buffer, sStart, sEnd;

  //PrintFormat.PrintSectionLabel(fp, "Time frame", false, true);
  //fprintf(fp, "%s to %s\n", GetStartDate(sStart, DataHub).c_str(), GetEndDate(sEnd, DataHub).c_str());
  printString(buffer, "%s to %s", GetStartDate(sStart, DataHub).c_str(), GetEndDate(sEnd, DataHub).c_str());
  printClusterData(fp, PrintFormat, "Time frame", buffer, true);
}

/* Returns data sets comprised in LLR calculation. This method is for the most part only relevent for multiple data set using
   the multivariate purpose -- and this variable is set during cluster scannning. Otherwise it's either just the one data or
   all the data sets when using adjustment purpose. */
DataSetIndexes_t CCluster::getDataSetIndexesComprisedInRatio(const CSaTScanData& DataHub) const {
    DataSetIndexes_t setIndexes;
    if (_ratio_sets.size() == 0) {
        // This is a single data set or multiple set w/ adjustment, in which case it is all the data sets.
        for (size_t d=0; d < DataHub.GetNumDataSets(); ++d)
            setIndexes.insert(d);
    } else {
        // This will be the case if this analysis is multivariate.
        for (size_t t=0; t < _ratio_sets.size(); ++t) {
            if (_ratio_sets.test(t))
                setIndexes.insert(t);
        }
    }
    return setIndexes;
}

/* Returns the name of the most central location in the cluster.
   In the most general and common application, identifier and location reference the same label/name.*/
std::string& CCluster::GetClusterLocation(std::string& locationID, const CSaTScanData& DataHub) const {
    if (GetClusterType() == PURELYTEMPORALCLUSTER)
        locationID = "All";
    else if (DataHub.GetParameters().GetMultipleCoordinatesType() == ONEPERLOCATION)
        // The location name and the identifier name are indentical when using the typical one coordinate per observation.
        locationID = DataHub.getIdentifierInfo().getIdentifierNameAtIndex(mostCentralIdentifierIdx(), locationID);
    else {
        std::vector<tract_t> clusterLocations;
        CentroidNeighborCalculator::getLocationsAboutCluster(DataHub, *this, 0, &clusterLocations);
        locationID = DataHub.getLocationsManager().locations()[clusterLocations.front()]->name();
    }
    return locationID;
}

/** returns end date of defined cluster as formated string */
std::string& CCluster::GetEndDate(std::string& sDateString, const CSaTScanData& DataHub, const char * sep) const {
  DatePrecisionType eDatePrint = (DataHub.GetParameters().GetPrecisionOfTimesType() == GENERIC ? GENERIC : DAY);  
  return JulianToString(sDateString, DataHub.GetTimeIntervalStartTimes()[m_nLastInterval] - 1, eDatePrint, sep);
}

/** Returns number of expected cases in accumulated data. */
measure_t CCluster::GetExpectedCount(const CSaTScanData& DataHub, size_t tSetIndex) const {
    const CParameters& params = DataHub.GetParameters();
    if (params.GetProbabilityModelType() == UNIFORMTIME) {
        const AbstractUniformTimeClusterData * pClusterData = 0;
        if ((pClusterData = dynamic_cast<const AbstractUniformTimeClusterData*>(GetClusterData())) == 0)
            throw prg_error("Cluster data object could not be dynamically casted to AbstractNormalClusterData type.\n", "GetExpectedCount()");
        double M = 2.0;
        count_t casesInPeriod = pClusterData->GetCasesInPeriod(tSetIndex), cases = GetClusterData()->GetCaseCount(tSetIndex);
        measure_t measureInPeriod = pClusterData->GetMeasureInPeriod(tSetIndex), measure = GetClusterData()->GetMeasure(tSetIndex);
        if (cases < casesInPeriod)
            return measure * (static_cast<double>(casesInPeriod) - static_cast<double>(cases)) / (measureInPeriod - measure);
        else if (cases == casesInPeriod)
            return measure / (M * (measureInPeriod - measure));
        return 0.0; // should not happen
    } else if (params.GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION) {
        // Retrieve the identifier indexes for this cluster.
        std::vector<tract_t> identifierIndexes;
        getIdentifierIndexes(DataHub, identifierIndexes, true);
        // Calculate the expected by summing the expected within each interval of the cluster window.
        count_t * pcasesnc = DataHub.GetDataSetHandler().GetDataSet(tSetIndex).getCaseData_PT_NC();
        measure_t ** ppmeasure = DataHub.GetDataSetHandler().GetDataSet(tSetIndex).getMeasureData().GetArray();
        measure_t expected = 0.0, * pmeasurenc = DataHub.GetDataSetHandler().GetDataSet(tSetIndex).getMeasureData_PT_NC();
        for (int interval=m_nFirstInterval; interval < m_nLastInterval; ++interval) {
            measure_t clusterMeaureInterval = 0;
            for (auto neighbor : identifierIndexes)
                clusterMeaureInterval += ppmeasure[interval][neighbor] - (interval < (m_nLastInterval - 1) ? ppmeasure[interval + 1][neighbor] : 0.0);
            if (params.GetProbabilityModelType() == BERNOULLI) 
                expected += (pmeasurenc[interval] ? pcasesnc[interval] / pmeasurenc[interval] : 0.0) * clusterMeaureInterval;
            else
                expected += clusterMeaureInterval;
        }
        return expected;
    } else
        return GetClusterData()->GetMeasure(tSetIndex) * DataHub.GetMeasureAdjustment(tSetIndex);
}

/** Returns number of expected cases in accumulated data that is stratified by
    ordinal categories. */
measure_t CCluster::GetExpectedCountOrdinal(const CSaTScanData& DataHub, size_t tSetIndex, size_t iCategoryIndex) const {
  const RealDataSet& DataSet = DataHub.GetDataSetHandler().GetDataSet(tSetIndex);

  return DataHub.GetProbabilityModel().GetPopulation(tSetIndex, *this, DataHub) *
             DataSet.getPopulationData().GetNumCategoryTypeCases(iCategoryIndex) / DataSet.getTotalPopulation();

}

/** Returns collection of identifier indexes that define this cluster. If 'bAtomize' is true, breaks down meta locations into atomic indexes. */
std::vector<tract_t> & CCluster::getIdentifierIndexes(const CSaTScanData& DataHub, std::vector<tract_t>& indexes, bool bAtomize) const {
    indexes.clear();
    for (tract_t t=1; t <= _num_identifiers; ++t) {
        tract_t n = DataHub.GetNeighbor(m_iEllipseOffset, m_Center, t, m_CartesianRadius); 
        if (n < DataHub.GetNumIdentifiers() || !bAtomize) {
            if (!DataHub.isNullifiedIdentifier(n))
                indexes.push_back(n);
        } else {
            std::vector<tract_t> atomicIndexes;
            DataHub.getIdentifierInfo().getMetaManagerProxy().getIndexes(n - DataHub.GetNumIdentifiers(), atomicIndexes);
            for (auto idx: atomicIndexes) {
                if (!DataHub.isNullifiedIdentifier(idx))
                    indexes.push_back(idx);
            }
        }
    }
    return indexes;
}

/** Returns index of most central identifier. */
tract_t CCluster::mostCentralIdentifierIdx() const {
    if (_central_identifier == -1) throw prg_error("Most central identifier of cluster not calculated.","mostCentralIdentifierIdx()");
    return _central_identifier;
}

/** Returns the number of identifiers in cluster that are not nullified from previous iteration(s) of an iterative scan. */
tract_t CCluster::numNonNullifiedIdentifiersInCluster(const CSaTScanData& DataHub) const {
    tract_t tClusterLocationCount=0;
    for (tract_t t=1; t <= getNumIdentifiers(); ++t) {
        tract_t tLocation = DataHub.GetNeighbor(GetEllipseOffset(), GetCentroidIndex(), t);
        if (!DataHub.isNullifiedIdentifier(tLocation))
            ++tClusterLocationCount;
    }
    return tClusterLocationCount;
}

/** Returns number of observed cases in accumulated data. */
count_t CCluster::GetObservedCount(size_t tSetIndex) const {
  return GetClusterData()->GetCaseCount(tSetIndex);
}

/** Returns number of observed cases in accumulated data that is stratified by
    ordinal categories. */
count_t CCluster::GetObservedCountOrdinal(size_t tSetIndex, size_t iCategoryIndex) const {
  return GetClusterData()->GetCategoryCaseCount(iCategoryIndex, tSetIndex);
}

/** Returns relative risk of cluster. */
double CCluster::GetObservedDivExpected(const CSaTScanData& DataHub, size_t tSetIndex) const {
  measure_t     tExpected = GetExpectedCount(DataHub, tSetIndex);
  return (tExpected ? (double)GetObservedCount(tSetIndex)/tExpected : 0);
}

/** Returns the relative risk for tract as defined by cluster. */
double CCluster::GetObservedDivExpectedForTract(tract_t tTractIndex, const CSaTScanData& DataHub, size_t tSetIndex) const {
  measure_t tExpected = GetExpectedCountForTract(tTractIndex, DataHub, tSetIndex);
  return (tExpected ? (double)GetObservedCountForTract(tTractIndex, DataHub, tSetIndex)/tExpected : 0);
}

/** Returns observed cases divided by expected cases of accummulated data that
    is stratified by ordinal categories. Note that if categories where combined,
    then this function should not be used. Instead value should be gotten by adding
    together observed values and dividing by sum of expected values. */
double CCluster::GetObservedDivExpectedOrdinal(const CSaTScanData& DataHub, size_t tSetIndex, size_t iCategoryIndex) const {
   measure_t    tExpected = GetExpectedCountOrdinal(DataHub, tSetIndex, iCategoryIndex);

   return (tExpected ? GetObservedCountOrdinal(tSetIndex, iCategoryIndex) / tExpected : 0);
}

/** Returns Gumbel p-value. */
std::pair<double,double> CCluster::GetGumbelPValue(const SimulationVariables& simVars) const {
    return calculateGumbelPValue(simVars, GetRatio());
}

/* Returns the distance between the furthest locations within this cluster. */
double CCluster::getLocationsSpan(const CSaTScanData& DataHub) const {
    if (_span_of_locations == -1.0) _span_of_locations = GisUtils::calculateDistanceFurthestLocations(DataHub, *this);
    return _span_of_locations;
}

/** Returns population as string with varied precision, based upon value. */
std::string & CCluster::GetPopulationAsString(std::string& sString, double dPopulation) const {
  if (dPopulation < .5)
    printString(sString, "%g", dPopulation); // display all decimals for populations less than .5
  else if (dPopulation < 1)
    printString(sString, "%.1f", dPopulation); // display one decimal for populations less than 1
  else
    printString(sString, "%.0f", dPopulation); // else display no decimals

  return sString;
}

/** Returns cluster monte carlo p-value. */
double CCluster::GetMonteCarloPValue(const CParameters& parameters, const SimulationVariables& simVars, bool bMLC) const {
  if (simVars.get_sim_count() < parameters.GetNumReplicationsRequested()) {
    if (bMLC)
      return static_cast<double>(parameters.GetExecuteEarlyTermThreshold())/static_cast<double>(simVars.get_sim_count());
    else
      return static_cast<double>(GetRank() - 1)/static_cast<double>(simVars.get_sim_count());
  }
  else
    return static_cast<double>(m_nRank)/static_cast<double>(simVars.get_sim_count() + 1);
}

/** Returns cluster p-value to report. */
double CCluster::getReportingPValue(const CParameters& parameters, const SimulationVariables& simVars, bool bMLC) const {
    double p_value=1.0;

  //Check base p-value reportability.
  if (!reportablePValue(parameters, simVars))
      return p_value;
  
  switch (parameters.GetPValueReportingType()) {
    case STANDARD_PVALUE    : 
    case TERMINATION_PVALUE : 
        if (reportableMonteCarloPValue(parameters,simVars))
          return GetMonteCarloPValue(parameters, simVars, bMLC);
        break;
    case GUMBEL_PVALUE      :
        if (reportableGumbelPValue(parameters, simVars)) {
           std::pair<double,double> p = GetGumbelPValue(simVars);
           return std::max(p.first, p.second);
        }
        break;
    case DEFAULT_PVALUE     :
    default                 :
        {
            bool bReportsDefaultGumbel = (parameters.GetAnalysisType() == PURELYSPATIAL ||
                                          parameters.GetAnalysisType() == SPACETIME ||
                                          parameters.GetAnalysisType() == PROSPECTIVESPACETIME) 
                                          && 
                                         (parameters.GetProbabilityModelType() == POISSON ||
                                          parameters.GetProbabilityModelType() == BERNOULLI ||
                                          parameters.GetProbabilityModelType() == SPACETIMEPERMUTATION
										  );
            bReportsDefaultGumbel |= parameters.GetAnalysisType() == PURELYSPATIAL &&
                                    (parameters.GetProbabilityModelType() == ORDINAL ||
                                      parameters.GetProbabilityModelType() == CATEGORICAL
                                    );
            if (bReportsDefaultGumbel && reportableGumbelPValue(parameters, simVars)) {
                std::pair<double,double> p = GetGumbelPValue(simVars);
                return std::max(p.first, p.second);
            }
            if (reportableMonteCarloPValue(parameters,simVars))
                return GetMonteCarloPValue(parameters, simVars, bMLC);
        }
  }

  return p_value;
}

/** Returns relative risk for Bernoulli, ordinal and Poisson models given parameter data. */
double CCluster::GetRelativeRisk(const CSaTScanData& DataHub, size_t tSetIndex) const {
  const CParameters params = DataHub.GetParameters();
  double expected = GetExpectedCount(DataHub, tSetIndex);

  if (params.GetProbabilityModelType() == UNIFORMTIME) {
      return (expected == 0.0 ? -1 : static_cast<double>(GetObservedCount(tSetIndex)) / expected); // when expected == 0, relative risk goes to infinity
  } else if (params.GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION) {
      return GetRelativeRisk(
          GetObservedCount(tSetIndex), GetClusterData()->GetMeasure(tSetIndex), // We want the unadjusted measure here.
          DataHub.GetDataSetHandler().GetDataSet(tSetIndex).getCaseData_PT()[m_nFirstInterval] - DataHub.GetDataSetHandler().GetDataSet(tSetIndex).getCaseData_PT()[m_nLastInterval],
          DataHub.GetDataSetHandler().GetDataSet(tSetIndex).getMeasureData_PT()[m_nFirstInterval] - DataHub.GetDataSetHandler().GetDataSet(tSetIndex).getMeasureData_PT()[m_nLastInterval]
      );
  } else 
    return GetRelativeRisk(
        GetObservedCount(tSetIndex), expected,
        DataHub.GetDataSetHandler().GetDataSet(tSetIndex).getTotalCases(), DataHub.GetDataSetHandler().GetDataSet(tSetIndex).getTotalCases()
    );
}

/** Returns relative risk for Bernoulli, ordinal and Poisson models given parameter data.
Returns negative one if relative risk goes to infinity */
double CCluster::GetRelativeRisk(double dObserved, double dExpected, double dTotalCases, double dTotalMeasure) const {
  //when all cases are inside cluster, relative risk goes to infinity
  if (dTotalCases == dObserved) return -1;
  if (dExpected && dTotalCases - dExpected && ((dTotalCases - dObserved)/(dTotalMeasure - dExpected)))
    return (dObserved/dExpected)/((dTotalCases - dObserved)/(dTotalMeasure - dExpected));
  return 0.0;  
}

/** Returns relative risk for location contained in cluster. */
double CCluster::GetRelativeRiskForTract(tract_t tTractIndex, const CSaTScanData& DataHub, size_t tSetIndex) const {
    const CParameters params = DataHub.GetParameters();
    double expected = GetExpectedCountForTract(tTractIndex, DataHub, tSetIndex);

    if (DataHub.GetParameters().GetProbabilityModelType() == UNIFORMTIME)
        return (expected == 0.0 ? -1 : static_cast<double>(GetObservedCountForTract(tTractIndex, DataHub, tSetIndex)) / expected); // when expected == 0, relative risk goes to infinity
    else if (params.GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION) {
        return GetRelativeRisk(
            GetObservedCountForTract(tTractIndex, DataHub, tSetIndex),
            GetExpectedCountForTract(tTractIndex, DataHub, tSetIndex, false), // We want the unadjusted measure here.
            DataHub.GetDataSetHandler().GetDataSet(tSetIndex).getCaseData_PT()[m_nFirstInterval] - DataHub.GetDataSetHandler().GetDataSet(tSetIndex).getCaseData_PT()[m_nLastInterval],
            DataHub.GetDataSetHandler().GetDataSet(tSetIndex).getMeasureData_PT()[m_nFirstInterval] - DataHub.GetDataSetHandler().GetDataSet(tSetIndex).getMeasureData_PT()[m_nLastInterval]
        );
    } else
        return GetRelativeRisk(
            GetObservedCountForTract(tTractIndex, DataHub, tSetIndex), expected,
            DataHub.GetDataSetHandler().GetDataSet(tSetIndex).getTotalCases(), DataHub.GetDataSetHandler().GetDataSet(tSetIndex).getTotalCases()
        );
}

/** returns start date of defined cluster as formated string */
std::string& CCluster::GetStartDate(std::string& sDateString, const CSaTScanData& DataHub, const char * sep) const {
  DatePrecisionType eDatePrint = (DataHub.GetParameters().GetPrecisionOfTimesType() == GENERIC ? GENERIC : DAY);  
  return JulianToString(sDateString, DataHub.GetTimeIntervalStartTimes()[m_nFirstInterval], eDatePrint, sep);
}

/** Returns whether cluster meets cutoff. */
boost::logic::tribool CCluster::meetsCutoff(
    const CSaTScanData& Data, unsigned int iReportedCluster, const SimulationVariables& simVars, RecurrenceInterval_t ri_cutoff, double pvalue_cutoff
) const {
    boost::logic::tribool meets_cutoff(boost::logic::indeterminate);
    const CParameters& params = Data.GetParameters();

    // If no simulations executed, then we can't say this cluster meets cutoff.
    if (simVars.get_sim_count() == 0)
        return meets_cutoff;
    // Test edge situations with LLR and cluster rank.
    if (m_nRatio < MIN_CLUSTER_LLR_REPORT || m_nRank > simVars.get_sim_count()) {
        meets_cutoff = false;
        return meets_cutoff;
    }
    // Test whether cluster meet specified cutoffs.
    if (params.GetIsProspectiveAnalysis() && reportableRecurrenceInterval(params, simVars)) {
        RecurrenceInterval_t ri = GetRecurrenceInterval(Data, iReportedCluster, simVars);
        return ri.second >= ri_cutoff.second;
    }
    if (!params.GetIsProspectiveAnalysis() && reportablePValue(params, simVars)) {
        return macro_less_than_or_equal(
            getReportingPValue(params, simVars, params.GetIsIterativeScanning() || iReportedCluster == 1),
            pvalue_cutoff, DBL_CMP_TOLERANCE
        );
    }
    return meets_cutoff;
}


/** Returns whether cluster is significant. */
boost::logic::tribool CCluster::isSignificant(const CSaTScanData& Data, unsigned int iReportedCluster, const SimulationVariables& simVars) const {
    const double SIGNIFICANCE_MULTIPLIER = 100.0; // I'm not sure how Martin picked this number.
    boost::logic::tribool significant(boost::logic::indeterminate);
    const CParameters& params = Data.GetParameters();

    // If no simulations executed, then we can't say this cluster is significant.
    if (simVars.get_sim_count() == 0)
        return significant;
    // Test edge situations with LLR and cluster rank.
    if (m_nRatio < 0.001/* minimum LLR for reporting and consideration for significance */ || m_nRank > simVars.get_sim_count()) {
        significant = false;
        return significant;
    }
    // Test using default significance checks.
    if (reportableRecurrenceInterval(params, simVars)) {
        RecurrenceInterval_t ri = GetRecurrenceInterval(Data, iReportedCluster, simVars);
        // 365 days or 100*(# time aggregation units), whichever is larger (e.g., if time aggregation is 3 months, only report clusters with RI>300 months)
        if (params.GetTimeAggregationUnitsType() == GENERIC)
            significant = (ri.first > SIGNIFICANCE_MULTIPLIER);
        else {
            double frequency_length = static_cast<double>(params.getProspectiveFrequency()); // defaulted to one.
            ProspectiveFrequency frequency = params.getProspectiveFrequencyType();
            // Potientially translate time aggregation type to prospective frequency type.
            if (frequency == SAME_TIMEAGGREGATION) {
                /* When frequency is that of time aggregation, the assumed frequency is the time aggregation.
                   (e.g. 1 day aggregation assumes daily frequency, 20 day aggregation assmes a frequency of every 20 days, etc.) */
                frequency_length = static_cast<double>(params.GetTimeAggregationLength());
                switch (params.GetTimeAggregationUnitsType()) {
                    case YEAR: frequency = YEARLY; break;
                    case MONTH: frequency = MONTHLY; break;
                    case DAY: frequency = DAILY; break;
                    default: throw prg_error("Invalid enum for time aggregation type '%d'.", "significant()", params.GetTimeAggregationUnitsType());
                }
            }
            /* Now calculate significance per frequency unit and frequency length (typically one or that of time agregation length).
               For instance with prospective frequency as weekly, a significant cluster would be greater than 100 weeks. */
            switch (frequency) {
                case YEARLY   : significant = (ri.first > SIGNIFICANCE_MULTIPLIER * frequency_length); break;
                case QUARTERLY: significant = (ri.first * 4.0 > SIGNIFICANCE_MULTIPLIER * frequency_length); break;
                case MONTHLY  : significant = (ri.first * 12.0 > SIGNIFICANCE_MULTIPLIER * frequency_length); break;
                case WEEKLY   : significant = (ri.first * 52.0 > SIGNIFICANCE_MULTIPLIER * frequency_length); break;
                case DAILY    : significant = (ri.second > std::max(365.0, SIGNIFICANCE_MULTIPLIER * frequency_length)); break;
                default       : throw prg_error("Unknown prospective frequency type '%d'.", "isSignificant()", frequency);
            }
        }
    } else if (reportablePValue(params, simVars)) {
        // p-value less than 0.05 is significant
        significant = getReportingPValue(params, simVars, params.GetIsIterativeScanning() || iReportedCluster == 1) < 0.05;
    } //else {// If both recurrence interval and p-value are not reportable, so we do not have information to say whether this cluster is significant or not.}
    return significant;
}

/** Returns indication whether this cluster can report Gumbel p-value. */
bool CCluster::reportableGumbelPValue(const CParameters& parameters, const SimulationVariables& simVars) const {
  //Check base p-value reportability.
  if (!reportablePValue(parameters, simVars))
      return false;

  //If default combination, report only if rank meets threshold.
  if (parameters.GetPValueReportingType() == DEFAULT_PVALUE)
      return m_nRank < MIN_RANK_RPT_GUMBEL;

  //Otherwise default to CParameters::getIsReportingGumbel()
  return parameters.getIsReportingGumbelPValue();
}

/** Returns indication whether this cluster can report monte carlo p-value. */
bool CCluster::reportableMonteCarloPValue(const CParameters& parameters, const SimulationVariables& simVars) const {
  // if cluster p-value is reportable and parameters indicate reporting monte carlo p-value
  return reportablePValue(parameters, simVars) && parameters.getIsReportingStandardPValue();
}

/** Returns indication whether this cluster can report p-value. */
bool CCluster::reportablePValue(const CParameters& parameters, const SimulationVariables& simVars) {
  //Require at least 99 simulations requested.
  return parameters.GetNumReplicationsRequested() >= MIN_SIMULATION_RPT_PVALUE;
}

/** Returns indication whether this cluster can report recurrence interval. */
bool CCluster::reportableRecurrenceInterval(const CParameters& parameters, const SimulationVariables& simVars) {
  //Require at least 99 simulations requested.
  if (parameters.GetNumReplicationsRequested() < MIN_SIMULATION_RPT_PVALUE)
      return false;

  //Otherwise only reportable for prospective analyses
  return parameters.GetIsProspectiveAnalysis();
}

/** Set class member 'm_CartesianRadius' from neighbor information obtained from
    CSaTScanData object. */
void CCluster::SetCartesianRadius(const CSaTScanData& DataHub) {
  std::vector<double> ClusterCenter, TractCoords;

  if (ClusterDefined() && !DataHub.GetParameters().UseLocationNeighborsFile() && !DataHub.GetParameters().getUseLocationsNetworkFile()) {
    DataHub.GetGInfo()->retrieveCoordinates(GetCentroidIndex(), ClusterCenter);
    CentroidNeighborCalculator::getTractCoordinates(DataHub, *this, DataHub.GetNeighbor(m_iEllipseOffset, m_Center, _num_identifiers), TractCoords);
    if (m_iEllipseOffset) {
     CentroidNeighborCalculator::Transform(ClusterCenter[0], ClusterCenter[1], DataHub.GetEllipseAngle(m_iEllipseOffset),
                                           DataHub.GetEllipseShape(m_iEllipseOffset), &ClusterCenter[0], &ClusterCenter[1]);
     CentroidNeighborCalculator::Transform(TractCoords[0], TractCoords[1], DataHub.GetEllipseAngle(m_iEllipseOffset),
                                           DataHub.GetEllipseShape(m_iEllipseOffset), &TractCoords[0], &TractCoords[1]);
    }
    m_CartesianRadius = Coordinates::distanceBetween(ClusterCenter, TractCoords);
  }  
}

/** Sets centroid index of cluster as defined in CSaTScanData. */
void CCluster::SetCenter(tract_t nCenter) {
  m_Center = nCenter;
}

/** Set ellipse offset as defined in CSaTScanData. */
void CCluster::SetEllipseOffset(int iOffset, const CSaTScanData& DataHub) {
  m_iEllipseOffset = iOffset;
  SetNonCompactnessPenalty(DataHub.GetEllipseShape(iOffset), DataHub.GetParameters().GetNonCompactnessPenaltyPower());
}

/** Calculates the most central identifier of the cluster. */
void CCluster::setMostCentralIdentifier(const CSaTScanData& DataHub) {
    if (ClusterDefined()) {
        // When iterative scan performed, we want the most central, not nullified, identifier.
        _central_identifier = -1;
        for (tract_t t=1; t <= getNumIdentifiers() && _central_identifier == -1; ++t) {
            tract_t identifierIdx = DataHub.GetNeighbor(GetEllipseOffset(), GetCentroidIndex(), t);
            if (!DataHub.isNullifiedIdentifier(identifierIdx))
                _central_identifier = identifierIdx;
        }
        //special case -- when most central location is still equal to '-1', 
        //that means all locations in cluster have been nullified, reset cluster to not defined
        if (_central_identifier == -1) _num_identifiers = 0;
    }
}

/** Sets non compactness penalty for shape. */
void CCluster::SetNonCompactnessPenalty(double dEllipseShape, double dPower) {
  m_NonCompactnessPenalty = CalculateNonCompactnessPenalty(dEllipseShape, dPower);
}

/** Set class members 'm_CartesianRadius' and '_central_identifier' from
    neighbor information obtained from CentroidNeighbors object. */
void CCluster::SetNonPersistantNeighborInfo(const CSaTScanData& DataHub, const CentroidNeighbors& Neighbors) {
  std::vector<double> ClusterCenter, TractCoords;

  if (ClusterDefined()) {
    DataHub.GetGInfo()->retrieveCoordinates(GetCentroidIndex(), ClusterCenter);
    CentroidNeighborCalculator::getTractCoordinates(DataHub, *this, Neighbors.GetNeighborTractIndex(_num_identifiers - 1), TractCoords);
    if (m_iEllipseOffset) {
        CentroidNeighborCalculator::Transform(
            ClusterCenter[0], ClusterCenter[1], DataHub.GetEllipseAngle(m_iEllipseOffset), DataHub.GetEllipseShape(m_iEllipseOffset), &ClusterCenter[0], &ClusterCenter[1]
        );
        CentroidNeighborCalculator::Transform(
            TractCoords[0], TractCoords[1], DataHub.GetEllipseAngle(m_iEllipseOffset), DataHub.GetEllipseShape(m_iEllipseOffset), &TractCoords[0], &TractCoords[1]
        );
    }
    m_CartesianRadius = Coordinates::distanceBetween(ClusterCenter, TractCoords);
    //when iterative scan performed, we want the most central, not nullified, identifiers
    _central_identifier = -1;
    for (tract_t t=0; t < getNumIdentifiers() && _central_identifier == -1; ++t) {
       if (!DataHub.isNullifiedIdentifier(Neighbors.GetNeighborTractIndex(t)))
         _central_identifier = Neighbors.GetNeighborTractIndex(t);
    }
  }
}

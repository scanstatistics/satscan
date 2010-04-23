//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "cluster.h"
#include "ClusterLocationsWriter.h"
#include "OrdinalLikelihoodCalculation.h"
#include "CategoricalClusterData.h"
#include "NormalClusterData.h"
#include "AbstractAnalysis.h"
#include "SSException.h"
#include <iostream>
#include <fstream>
#include "WeightedNormalRandomizer.h"

unsigned int CCluster::MIN_RANK_RPT_GUMBEL = 10;

/** constructor */
CCluster::CCluster() {
  Initialize();
}

/** destructor */
CCluster::~CCluster() {}

/** initializes cluster data  */
void CCluster::Initialize(tract_t nCenter) {
  m_Center         = nCenter;
  m_MostCentralLocation = -1;
  m_nTracts        = 0;
  m_CartesianRadius= -1;
  m_nRatio         = 0;
  m_nRank          = 1;
  m_NonCompactnessPenalty = 1;
  m_nFirstInterval = 0;
  m_nLastInterval  = 0;
  m_iEllipseOffset = 0;
}

/** overloaded assignment operator */
CCluster& CCluster::operator=(const CCluster& rhs) {
  m_Center                = rhs.m_Center;
  m_MostCentralLocation   = rhs.m_MostCentralLocation;
  m_nTracts               = rhs.m_nTracts;
  m_CartesianRadius       = rhs.m_CartesianRadius;
  m_nRatio                = rhs.m_nRatio;
  m_nRank                 = rhs.m_nRank;
  m_NonCompactnessPenalty = rhs.m_NonCompactnessPenalty;
  m_nFirstInterval        = rhs.m_nFirstInterval;
  m_nLastInterval         = rhs.m_nLastInterval;
  m_iEllipseOffset        = rhs.m_iEllipseOffset;

  return *this;
}

/** */
const double CCluster::ConvertAngleToDegrees(double dAngle) const {
  double dDegrees;

  dDegrees = 180.00 * (dAngle / (double)M_PI);
  if (dDegrees > 90.00)
    dDegrees -= 180.00;

  return dDegrees;
}

/** Calls class cluster data object, deallocating any class members that were used to
    iterate through data during evaluation process. */
void CCluster::DeallocateEvaluationAssistClassMembers() {
  GetClusterData()->DeallocateEvaluationAssistClassMembers();
}

/** Writes cluster properties to file stream in format required by result output file  */
void CCluster::Display(FILE* fp, const CSaTScanData& DataHub, unsigned int iReportedCluster, const SimulationVariables& simVars) const {
  try {
    AsciiPrintFormat PrintFormat;
    PrintFormat.SetMarginsAsClusterSection(iReportedCluster);
    fprintf(fp, "%u.", iReportedCluster);
    DisplayCensusTracts(fp, DataHub, PrintFormat);
    if (!DataHub.GetParameters().UseLocationNeighborsFile()) {
      if (DataHub.GetParameters().GetCoordinatesType() == CARTESIAN)
        DisplayCoordinates(fp, DataHub, PrintFormat);
      else
        DisplayLatLongCoords(fp, DataHub, PrintFormat);
    }    
    DisplayTimeFrame(fp, DataHub, PrintFormat);
    if (DataHub.GetParameters().GetProbabilityModelType() == ORDINAL ||
        DataHub.GetParameters().GetProbabilityModelType() == CATEGORICAL)
      DisplayClusterDataOrdinal(fp, DataHub, PrintFormat);
    else if (DataHub.GetParameters().GetProbabilityModelType() == EXPONENTIAL)
      DisplayClusterDataExponential(fp, DataHub, PrintFormat);
    else if (DataHub.GetParameters().GetProbabilityModelType() == NORMAL) { 
        if (DataHub.GetParameters().getIsWeightedNormal())
            DisplayClusterDataWeightedNormal(fp, DataHub, PrintFormat);
        else
            DisplayClusterDataNormal(fp, DataHub, PrintFormat);
    }
    else
      DisplayClusterDataStandard(fp, DataHub, PrintFormat);
    DisplayRatio(fp, DataHub, PrintFormat);
    DisplayMonteCarloInformation(fp, DataHub, iReportedCluster, PrintFormat, simVars);
    DisplayTimeTrend(fp, DataHub, PrintFormat);
  }
  catch (prg_exception& x) {
    x.addTrace("Display()","CCluster");
    throw;
  }
}

/** Prints annual cases to file stream is in format required by result output file. */
void CCluster::DisplayAnnualCaseInformation(FILE* fp, unsigned int iDataSetIndex, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  std::string buffer;

  if (DataHub.GetParameters().GetProbabilityModelType() == POISSON && DataHub.GetParameters().UsePopulationFile()) {
    printString(buffer, "Annual cases / %.0f", DataHub.GetAnnualRatePop());
    PrintFormat.PrintSectionLabel(fp, buffer.c_str(), false, true);
    buffer = getValueAsString(DataHub.GetAnnualRateAtStart(iDataSetIndex) * GetObservedDivExpected(DataHub, iDataSetIndex), buffer, 1);
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
  }
}

/** Writes cluster location identifiers to file stream in format required by result output file  */
void CCluster::DisplayCensusTracts(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const {
  try {
    PrintFormat.PrintSectionLabel(fp, "Location IDs included", false, false);  
    DisplayCensusTractsInStep(fp, Data, 1, m_nTracts, PrintFormat);
  }
  catch (prg_exception& x) {
    x.addTrace("DisplayCensusTracts()","CCluster");
    throw;
  }
}

/** Writes clusters location information in format required by result output file. */
void CCluster::DisplayCensusTractsInStep(FILE* fp, const CSaTScanData& DataHub, tract_t nFirstTract, tract_t nLastTract, const AsciiPrintFormat& PrintFormat) const {
  std::string                                   locations;
  TractHandler::Location::StringContainer_t     vTractIdentifiers;

  try {
    for (tract_t i=nFirstTract; i <= nLastTract; ++i) {
       //get i'th neighbor tracts index
       tract_t tTract = DataHub.GetNeighbor(m_iEllipseOffset, m_Center, i, m_CartesianRadius);
       // Print location identifiers if location data has not been removed in iterative scan.
       if (!DataHub.GetIsNullifiedLocation(tTract)) {
         //get all locations ids for tract at index tTract -- might be more than one if combined
         DataHub.GetTInfo()->retrieveAllIdentifiers(tTract, vTractIdentifiers);
         for (unsigned int i=0; i < vTractIdentifiers.size(); ++i) {
            if (locations.size())
              locations += ", ";
            locations += vTractIdentifiers[i].c_str();
         }
       }
    }
    // There should be at least one location printed, else there is likely a bug in the iterative scan code.
    if (!locations.size())
      throw prg_error("Attempting to print cluster with no location identifiers.","DisplayCensusTractsInStep()");

    PrintFormat.PrintAlignedMarginsDataString(fp, locations);
  }
  catch (prg_exception& x) {
    x.addTrace("DisplayCensusTractsInStep()","CCluster");
    throw;
  }
}

/** Prints observed cases, expected cases and observed/expected, for exponetial model,
    to file stream is in format required by result output file. */
void CCluster::DisplayClusterDataExponential(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  std::string                                   buffer;
  std::vector<unsigned int>                     vComprisedDataSetIndexes;
  std::vector<unsigned int>::iterator           itr_Index;
  std::auto_ptr<AbstractLikelihoodCalculator>   Calculator(AbstractAnalysis::GetNewLikelihoodCalculator(DataHub));

  GetClusterData()->GetDataSetIndexesComprisedInRatio(m_nRatio/m_NonCompactnessPenalty, *Calculator.get(), vComprisedDataSetIndexes);
  for (itr_Index=vComprisedDataSetIndexes.begin(); itr_Index != vComprisedDataSetIndexes.end(); ++itr_Index) {
     //print data set number if analyzing more than data set
     if (DataHub.GetDataSetHandler().GetNumDataSets() > 1) {
       printString(buffer, "Data Set #%ld", *itr_Index + 1);
       PrintFormat.PrintSectionLabelAtDataColumn(fp, buffer.c_str());
     }
     //print total individuals (censored and non-censored)
     PrintFormat.PrintSectionLabel(fp, "Total individuals", false, true);
     GetPopulationAsString(buffer, DataHub.GetProbabilityModel().GetPopulation(*itr_Index, *this, DataHub));
     PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
     //print total cases (non-censored)
     PrintFormat.PrintSectionLabel(fp, "Number of cases", false, true);
     printString(buffer, "%ld", GetObservedCount(*itr_Index));
     PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
     //not printing censored information at Martin's directive, but leave in place for now
     ////print total censored cases
     //PrintFormat.PrintSectionLabel(fp, "Number censored cases", false, true);
     //GetPopulationAsString(sBuffer, DataHub.GetProbabilityModel().GetPopulation(*itr_Index, *this, DataHub) - GetObservedCount(*itr_Index));
     //PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
     //print expected cases label
     PrintFormat.PrintSectionLabel(fp, "Expected cases", false, true);
     //print expected cases
     buffer = getValueAsString(GetExpectedCount(DataHub, *itr_Index), buffer);
     PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
     DisplayObservedDivExpected(fp, *itr_Index, DataHub, PrintFormat);
     //NOTE: Not printing relative risk information for exponential model at this time.
  }
}

/** Prints observed cases, expected cases and observed/expected, for Normal model,
    to file stream is in format required by result output file. */
void CCluster::DisplayClusterDataNormal(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  std::string                                   buffer, work;
  std::vector<unsigned int>                     vComprisedDataSetIndexes;
  std::vector<unsigned int>::iterator           itr_Index;
  std::auto_ptr<AbstractLikelihoodCalculator>   Calculator(AbstractAnalysis::GetNewLikelihoodCalculator(DataHub));
  double                                        dEstimatedMeanInside, dEstimatedMeanOutside, dUnbiasedVariance;
  const AbstractNormalClusterData             * pClusterData=0;
  count_t                                       tObserved;
  measure_t                                     tExpected;

  if ((pClusterData = dynamic_cast<const AbstractNormalClusterData*>(GetClusterData())) == 0)
    throw prg_error("Cluster data object could not be dynamically casted to AbstractNormalClusterData type.\n",
                    "DisplayClusterDataNormal()");
  const DataSetHandler& Handler = DataHub.GetDataSetHandler();
  GetClusterData()->GetDataSetIndexesComprisedInRatio(m_nRatio/m_NonCompactnessPenalty, *Calculator.get(), vComprisedDataSetIndexes);
  for (itr_Index=vComprisedDataSetIndexes.begin(); itr_Index != vComprisedDataSetIndexes.end(); ++itr_Index) {
     //print data set number if analyzing more than data set
     if (Handler.GetNumDataSets() > 1) {
       printString(buffer, "Data Set #%ld", *itr_Index + 1);
       PrintFormat.PrintSectionLabelAtDataColumn(fp, buffer.c_str());
     }
     //print total cases
     PrintFormat.PrintSectionLabel(fp, "Number of cases", false, true);
     printString(buffer, "%ld", GetObservedCount(*itr_Index));
     PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
     tObserved = GetObservedCount(*itr_Index);
     tExpected = GetExpectedCount(DataHub, *itr_Index);
     //print estimated mean inside label
     PrintFormat.PrintSectionLabel(fp, "Mean inside", false, true);
     //print estimated mean inside
     dEstimatedMeanInside = (tObserved ? tExpected/tObserved : 0);
     buffer = getValueAsString(dEstimatedMeanInside, buffer);
     PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
     //print estimated mean outside label
     PrintFormat.PrintSectionLabel(fp, "Mean outside", false, true);
     //print estimated mean inside
     count_t tCasesOutside = Handler.GetDataSet(*itr_Index).getTotalCases() - tObserved;
     dEstimatedMeanOutside = (tCasesOutside ? (Handler.GetDataSet(*itr_Index).getTotalMeasure() - tExpected)/tCasesOutside : 0);
     buffer = getValueAsString(dEstimatedMeanOutside, buffer);
     PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
     //print unexplained variance label
     PrintFormat.PrintSectionLabel(fp, "Variance", false, true);
     dUnbiasedVariance = GetUnbiasedVariance(GetObservedCount(*itr_Index), GetExpectedCount(DataHub, *itr_Index), pClusterData->GetMeasureAux(*itr_Index),
                                             Handler.GetDataSet(*itr_Index).getTotalCases(), Handler.GetDataSet(*itr_Index).getTotalMeasure(),
                                             Handler.GetDataSet(*itr_Index).getTotalMeasureAux());
     buffer = getValueAsString(dUnbiasedVariance, buffer);
     PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
     //print common standard deviation
     PrintFormat.PrintSectionLabel(fp, "Standard deviation", false, true);
     buffer = getValueAsString(std::sqrt(dUnbiasedVariance), buffer);
     PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
  }
}

/** Prints observed cases, expected cases and observed/expected, for Ordinal model,
    to file stream is in format required by result output file. */
void CCluster::DisplayClusterDataOrdinal(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  std::string                                           work, buffer, work2;
  double                                                dTotalCasesInClusterDataSet=0;
  OrdinalLikelihoodCalculator                           Calculator(DataHub);
  std::vector<OrdinalCombinedCategory>                  vCategoryContainer;
  std::vector<OrdinalCombinedCategory>::iterator        itrCategory;
  const AbstractCategoricalClusterData                * pClusterData=0;
  std::vector<unsigned int>                             vComprisedDataSetIndexes;
  std::vector<unsigned int>::iterator                   itr_Index;

  if ((pClusterData = dynamic_cast<const AbstractCategoricalClusterData*>(GetClusterData())) == 0)
    throw prg_error("Cluster data object could not be dynamically casted to AbstractCategoricalClusterData type.\n",
                    "DisplayClusterDataOrdinal()");
  GetClusterData()->GetDataSetIndexesComprisedInRatio(m_nRatio/m_NonCompactnessPenalty, Calculator, vComprisedDataSetIndexes);

  for (itr_Index=vComprisedDataSetIndexes.begin(); itr_Index != vComprisedDataSetIndexes.end(); ++itr_Index) {
     //retrieve collection of ordinal categories in combined state
     pClusterData->GetOrdinalCombinedCategories(Calculator, vCategoryContainer, *itr_Index);
     //if container is empty, data set did not contribute to the loglikelihood ratio, so skip reporting it
     if (!vCategoryContainer.size())
       continue;
     //print data set number if analyzing more than one data set
     if (DataHub.GetDataSetHandler().GetNumDataSets() > 1) {
       printString(buffer, "Data Set #%ld", *itr_Index + 1);
       printString(buffer, "Data Set #%ld", *itr_Index + 1);
       PrintFormat.PrintSectionLabelAtDataColumn(fp, buffer.c_str());
     }
     //print total cases per data set
     PrintFormat.PrintSectionLabel(fp, "Total cases", false, true);
     dTotalCasesInClusterDataSet = DataHub.GetProbabilityModel().GetPopulation(*itr_Index, *this, DataHub);
     PrintFormat.PrintAlignedMarginsDataString(fp, GetPopulationAsString(buffer, dTotalCasesInClusterDataSet));
     //print category ordinal values
     PrintFormat.PrintSectionLabel(fp, "Category", false, true);
     const RealDataSet& thisDataSet = DataHub.GetDataSetHandler().GetDataSet(*itr_Index);
     buffer = "";
     for (itrCategory=vCategoryContainer.begin(); itrCategory != vCategoryContainer.end(); ++itrCategory) {
       buffer += (itrCategory == vCategoryContainer.begin() ? "" : ", ");
       for (size_t m=0; m < itrCategory->GetNumCombinedCategories(); ++m) {
         printString(work, "%s%g%s",
                      (m == 0 ? "[" : ", "),
                      thisDataSet.getPopulationData().GetOrdinalCategoryValue(itrCategory->GetCategoryIndex(m)),
                      (m + 1 == itrCategory->GetNumCombinedCategories() ? "]" : ""));
         buffer += work;
       }
     }
     PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
     //print observed case data per category
     PrintFormat.PrintSectionLabel(fp, "Number of cases", false, true);
     buffer = "";
     for (itrCategory=vCategoryContainer.begin(); itrCategory != vCategoryContainer.end(); ++itrCategory) {
       count_t tObserved=0;
       for (size_t m=0; m < itrCategory->GetNumCombinedCategories(); ++m)
          tObserved += GetObservedCountOrdinal(*itr_Index, itrCategory->GetCategoryIndex(m));
       printString(work, "%s%ld", (itrCategory == vCategoryContainer.begin() ? "" : ", "), tObserved);
       buffer += work;
     }
     PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
     //print expected case data per category
     PrintFormat.PrintSectionLabel(fp, "Expected cases", false, true);
     buffer = "";
     for (itrCategory=vCategoryContainer.begin(); itrCategory != vCategoryContainer.end(); ++itrCategory) {
       measure_t tExpected=0;
       for (size_t m=0; m < itrCategory->GetNumCombinedCategories(); ++m)
          tExpected += GetExpectedCountOrdinal(DataHub, *itr_Index, itrCategory->GetCategoryIndex(m));
       work2 = getValueAsString(tExpected, work2); 
       printString(work, "%s%s", (itrCategory == vCategoryContainer.begin() ? "" : ", "), work2.c_str());
       buffer += work;
     }
     PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
     //print observed div expected case data per category
     PrintFormat.PrintSectionLabel(fp, "Observed / expected", false, true);
     buffer = "";
     for (itrCategory=vCategoryContainer.begin(); itrCategory != vCategoryContainer.end(); ++itrCategory) {
       count_t   tObserved=0;
       measure_t tExpected=0;
       for (size_t m=0; m < itrCategory->GetNumCombinedCategories(); ++m) {
          tObserved += GetObservedCountOrdinal(*itr_Index, itrCategory->GetCategoryIndex(m));
          tExpected += GetExpectedCountOrdinal(DataHub, *itr_Index, itrCategory->GetCategoryIndex(m));
       }
       work2 = getValueAsString((double)tObserved/tExpected, work2); 
       printString(work, "%s%s", (itrCategory == vCategoryContainer.begin() ? "" : ", "), work2.c_str());
       buffer += work;
     }
     PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
     //print relative data - note that we will possibly be combining categories
     PrintFormat.PrintSectionLabel(fp, "Relative risk", false, true);
     buffer = "";
     for (itrCategory=vCategoryContainer.begin(); itrCategory != vCategoryContainer.end(); ++itrCategory) {
       double           tRelativeRisk=0;
       count_t          tObserved=0, tTotalCategoryCases=0;
       measure_t        tExpected=0;
       for (size_t m=0; m < itrCategory->GetNumCombinedCategories(); ++m) {
          tObserved += GetObservedCountOrdinal(*itr_Index, itrCategory->GetCategoryIndex(m));
          tExpected += GetExpectedCountOrdinal(DataHub, *itr_Index, itrCategory->GetCategoryIndex(m));
          tTotalCategoryCases += DataHub.GetDataSetHandler().GetDataSet(*itr_Index).getPopulationData().GetNumOrdinalCategoryCases(itrCategory->GetCategoryIndex(m));
       }
       if ((tRelativeRisk = GetRelativeRisk(tObserved, tExpected, tTotalCategoryCases)) == -1)
         printString(work, "%sinfinity", (itrCategory == vCategoryContainer.begin() ? "" : ", "));
       else {
         work2 = getValueAsString(tRelativeRisk, work2); 
         printString(work, "%s%s", (itrCategory == vCategoryContainer.begin() ? "" : ", "), work2.c_str());
       }
       buffer += work;
     }
     PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
  }
}

/** Prints population, observed cases, expected cases and relative risk
    to file stream is in format required by result output file. */
void CCluster::DisplayClusterDataStandard(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  std::string                                   buffer;
  std::vector<unsigned int>                     vComprisedDataSetIndexes;
  std::vector<unsigned int>::iterator           itr_Index;
  std::auto_ptr<AbstractLikelihoodCalculator>   Calculator(AbstractAnalysis::GetNewLikelihoodCalculator(DataHub));

  DisplayPopulation(fp, DataHub, PrintFormat);
  GetClusterData()->GetDataSetIndexesComprisedInRatio(m_nRatio/m_NonCompactnessPenalty, *Calculator.get(), vComprisedDataSetIndexes);
  for (itr_Index=vComprisedDataSetIndexes.begin(); itr_Index != vComprisedDataSetIndexes.end(); ++itr_Index) {
     //print data set number if analyzing more than data set
     if (DataHub.GetDataSetHandler().GetNumDataSets() > 1) {
       printString(buffer, "Data Set #%ld", *itr_Index + 1);
       PrintFormat.PrintSectionLabelAtDataColumn(fp, buffer.c_str());
     }
     //print observed cases label
     PrintFormat.PrintSectionLabel(fp, "Number of cases", false, true);
     printString(buffer, "%ld", GetObservedCount(*itr_Index));
     //print observed cases
     PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
     //print expected cases label
     PrintFormat.PrintSectionLabel(fp, "Expected cases", false, true);
     //print expected cases
     buffer = getValueAsString(GetExpectedCount(DataHub, *itr_Index), buffer);
     PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
     DisplayAnnualCaseInformation(fp, *itr_Index, DataHub, PrintFormat);
     DisplayObservedDivExpected(fp, *itr_Index ,DataHub, PrintFormat);
     if (DataHub.GetParameters().GetProbabilityModelType() == POISSON  || DataHub.GetParameters().GetProbabilityModelType() == BERNOULLI)
       DisplayRelativeRisk(fp, *itr_Index, DataHub, PrintFormat);
   }
}

/** Prints observed cases, expected cases and observed/expected, for Weighted Normal model,
    to file stream is in format required by result output file. */
void CCluster::DisplayClusterDataWeightedNormal(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  std::string                                   buffer;
  std::vector<unsigned int>                     vComprisedDataSetIndexes;
  std::auto_ptr<AbstractLikelihoodCalculator>   Calculator(AbstractAnalysis::GetNewLikelihoodCalculator(DataHub));
  const AbstractWeightedNormalRandomizer      * pRandomizer=0;
  std::vector<tract_t>                          tractIndexes;

  const DataSetHandler& Handler = DataHub.GetDataSetHandler();
  GetClusterData()->GetDataSetIndexesComprisedInRatio(m_nRatio/m_NonCompactnessPenalty, *Calculator.get(), vComprisedDataSetIndexes);
  getLocationIndexes(DataHub, tractIndexes, true);
  for (std::vector<unsigned int>::iterator itr_Index=vComprisedDataSetIndexes.begin(); itr_Index != vComprisedDataSetIndexes.end(); ++itr_Index) {
      const RealDataSet& dataSet = Handler.GetDataSet(*itr_Index);
      //get randomizer for data set to retrieve various information
      if ((pRandomizer = dynamic_cast<const AbstractWeightedNormalRandomizer*>(Handler.GetRandomizer(*itr_Index))) == 0)
        throw prg_error("Randomizer could not be dynamically casted to AbstractWeightedNormalRandomizer type.\n", "DisplayClusterDataWeightedNormal()");

     //print data set number if analyzing more than data set
     if (Handler.GetNumDataSets() > 1) {
       printString(buffer, "Data Set #%ld", *itr_Index + 1);
       PrintFormat.PrintSectionLabelAtDataColumn(fp, buffer.c_str());
     }
     AbstractWeightedNormalRandomizer::ClusterStatistics statistics;
     statistics = pRandomizer->getClusterStatistics(m_nFirstInterval, m_nLastInterval, tractIndexes);

     //print total cases
     PrintFormat.PrintSectionLabel(fp, "Number of cases", false, true);
     printString(buffer, "%ld", statistics.gtObservations);
     PrintFormat.PrintAlignedMarginsDataString(fp, buffer);

     //print total cluster weight
     PrintFormat.PrintSectionLabel(fp, "Total weights", false, true);
     buffer = getValueAsString(statistics.gtWeight, buffer);
     PrintFormat.PrintAlignedMarginsDataString(fp, buffer);

     //print mean inside
     PrintFormat.PrintSectionLabel(fp, "Mean inside", false, true);
     buffer = getValueAsString(statistics.gtMeanIn, buffer);
     PrintFormat.PrintAlignedMarginsDataString(fp, buffer);

     //print mean outside
     PrintFormat.PrintSectionLabel(fp, "Mean outside", false, true);
     buffer = getValueAsString(statistics.gtMeanOut, buffer);
     PrintFormat.PrintAlignedMarginsDataString(fp, buffer);

     //print cluster variance
     PrintFormat.PrintSectionLabel(fp, "Variance", false, true);
     buffer = getValueAsString(statistics.gtVariance, buffer);
     PrintFormat.PrintAlignedMarginsDataString(fp, buffer);

     //print cluster standard deviation
     PrintFormat.PrintSectionLabel(fp, "Standard deviation", false, true);
     buffer = getValueAsString(std::sqrt(statistics.gtVariance), buffer);
     PrintFormat.PrintAlignedMarginsDataString(fp, buffer);

     //print weighted mean inside
     PrintFormat.PrintSectionLabel(fp, "Weighted mean inside", false, true);
     buffer = getValueAsString(statistics.gtWeightedMeanIn, buffer);
     PrintFormat.PrintAlignedMarginsDataString(fp, buffer);

     //print weighted mean outside
     PrintFormat.PrintSectionLabel(fp, "Weighted mean outside", false, true);
     buffer = getValueAsString(statistics.gtWeightedMeanOut, buffer);
     PrintFormat.PrintAlignedMarginsDataString(fp, buffer);

     //print cluster weighted variance
     PrintFormat.PrintSectionLabel(fp, "Weighted variance", false, true);
     buffer = getValueAsString(statistics.gtWeightedVariance, buffer);
     PrintFormat.PrintAlignedMarginsDataString(fp, buffer);

     //print cluster standard deviation
     PrintFormat.PrintSectionLabel(fp, "Weighted std deviation", false, true);
     buffer = getValueAsString(std::sqrt(statistics.gtWeightedVariance), buffer);
     PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
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
      PrintFormat.PrintSectionLabel(fp, "Coordinates / radius", false, true);
      for (size_t i=0; i < vCoordinates.size() - 1; ++i) {
         printString(work, "%s%g,", (i == 0 ? "(" : "" ), vCoordinates[i]);
         buffer += work;
      }

      work2 = getValueAsString(m_CartesianRadius, work2);
      printString(work, "%g) / %s", vCoordinates.back(), work2.c_str());
      buffer += work;
      PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
    }
    else {//print ellipse settings
      PrintFormat.PrintSectionLabel(fp, "Coordinates", false, true);
      for (size_t i=0; i < vCoordinates.size() - 1; ++i) {
         printString(work, "%s%g,", (i == 0 ? "(" : "" ), vCoordinates[i]);
         buffer += work;
      }
      printString(work, "%g)", vCoordinates.back());
      buffer += work;
      PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
      //print ellipse particulars
      PrintFormat.PrintSectionLabel(fp, "Semiminor axis", false, true);
      work = getValueAsString(m_CartesianRadius, work);
      fprintf(fp, "%s\n", work.c_str());
      PrintFormat.PrintSectionLabel(fp, "Semimajor axis", false, true);
      work = getValueAsString(m_CartesianRadius * Data.GetEllipseShape(GetEllipseOffset()), work);
      fprintf(fp, "%s\n", work.c_str());
      PrintFormat.PrintSectionLabel(fp, "Angle (degrees)", false, true);
      work = getValueAsString(ConvertAngleToDegrees(Data.GetEllipseAngle(m_iEllipseOffset)), work);
      fprintf(fp, "%s\n", work.c_str());
      PrintFormat.PrintSectionLabel(fp, "Shape", false, true);
      work = getValueAsString(Data.GetEllipseShape(m_iEllipseOffset), work);
      fprintf(fp, "%s\n", work.c_str());
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
  char                          cNorthSouth, cEastWest;
  std::string                   buffer;

  try {
    Data.GetGInfo()->retrieveCoordinates(m_Center, vCoordinates);
    prLatitudeLongitude = ConvertToLatLong(vCoordinates);
    prLatitudeLongitude.first >= 0 ? cNorthSouth = 'N' : cNorthSouth = 'S';
    prLatitudeLongitude.second >= 0 ? cEastWest = 'E' : cEastWest = 'W';
    PrintFormat.PrintSectionLabel(fp, "Coordinates / radius", false, true);
    buffer = getValueAsString(GetLatLongRadius(), buffer);
    fprintf(fp, "(%.6f %c, %.6f %c) / %s km\n", fabs(prLatitudeLongitude.first), cNorthSouth, fabs(prLatitudeLongitude.second), cEastWest, buffer.c_str());
  }
  catch (prg_exception& x) {
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

  PrintFormat.PrintSectionLabel(fp, "Monte Carlo rank", false, true);
  fprintf(fp, "%u/%ld\n", m_nRank, simVars.get_sim_count() + 1);

  if (reportablePValue(parameters,simVars)) {
    // conditionally report cluster p-value as monte carlo or gumbel
    PrintFormat.PrintSectionLabel(fp, "P-value", false, true);
    if (parameters.GetPValueReportingType() == GUMBEL_PVALUE ||
        (parameters.GetPValueReportingType() == DEFAULT_PVALUE && GetRank() < MIN_RANK_RPT_GUMBEL)) {
      std::pair<double,double> p = GetGumbelPValue(simVars);
      if (p.first == 0.0) {
        getValueAsString(p.second, buffer).insert(0, "< ");
      } else {
        getValueAsString(p.first, buffer);
      }
      PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
    } else {
      printString(replicas, "%u", simVars.get_sim_count());
      printString(format, "%%.%dlf\n", replicas.size());
      fprintf(fp, format.c_str(), GetMonteCarloPValue(parameters,simVars, iReportedCluster == 1));
    }
    DisplayRecurrenceInterval(fp, DataHub, iReportedCluster, simVars, PrintFormat);
    //conditionally report gumbel p-value as supplement to reported p-value
    if (parameters.GetReportGumbelPValue() &&
        (parameters.GetPValueReportingType() == STANDARD_PVALUE || parameters.GetPValueReportingType() == TERMINATION_PVALUE)) {
         PrintFormat.PrintSectionLabel(fp, "Gumbel P-value", false, true);
         std::pair<double,double> p = GetGumbelPValue(simVars);
         if (p.first == 0.0) {
           getValueAsString(p.second, buffer).insert(0, "< ");
         } else {
           getValueAsString(p.first, buffer);
         }
         PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
    }
  }
}

/** Returns recurrence interval interms of years and interms of days. */
CCluster::RecurrenceInterval_t CCluster::GetRecurrenceInterval(const CSaTScanData& Data, 
                                                               unsigned int iReportedCluster, 
                                                               const SimulationVariables& simVars) const {
  double        dIntervals, dPValue, dAdjustedP_Value, dUnitsInOccurrence;
  const CParameters & parameters(Data.GetParameters());

  if (!parameters.GetIsProspectiveAnalysis())
     throw prg_error("GetRecurrenceInterval() called for non-prospective analysis.","GetRecurrenceInterval()");

  dIntervals = static_cast<double>(Data.GetNumTimeIntervals() - Data.GetProspectiveStartIndex() + 1);
  if ((parameters.GetPValueReportingType() == DEFAULT_PVALUE && m_nRank < MIN_RANK_RPT_GUMBEL) ||
       parameters.GetPValueReportingType() == GUMBEL_PVALUE) {
      std::pair<double,double> p = GetGumbelPValue(simVars);
      dPValue = std::max(p.first, p.second);
      dAdjustedP_Value = std::max(1.0 - pow(1.0 - dPValue, 1.0/dIntervals),p.second);
  } else {
      dPValue = GetMonteCarloPValue(Data.GetParameters(), simVars, iReportedCluster == 1);
      dAdjustedP_Value = 1.0 - pow(1.0 - dPValue, 1.0/dIntervals);
  }
  dUnitsInOccurrence = static_cast<double>(parameters.GetTimeAggregationLength())/dAdjustedP_Value;
  
  switch (parameters.GetTimeAggregationUnitsType()) {
      case YEAR   : return std::make_pair(dUnitsInOccurrence, std::max(dUnitsInOccurrence * AVERAGE_DAYS_IN_YEAR,1.0));
      case MONTH  : return std::make_pair(dUnitsInOccurrence/12.0, std::max((dUnitsInOccurrence/12.0) * AVERAGE_DAYS_IN_YEAR,1.0));
      case DAY    : return std::make_pair(dUnitsInOccurrence/AVERAGE_DAYS_IN_YEAR, std::max(dUnitsInOccurrence,1.0));
      default     : throw prg_error("Invalid time interval index \"%d\" for prospective analysis.",
                                    "GetRecurrenceInterval()", parameters.GetTimeAggregationUnitsType());
  }
}

/** Writes clusters null occurance rate in format required by result output file. */
void CCluster::DisplayRecurrenceInterval(FILE* fp, const CSaTScanData& Data, unsigned int iReportedCluster, const SimulationVariables& simVars, const AsciiPrintFormat& PrintFormat) const {
  std::string   buffer;

  try {
      if (reportableRecurrenceInterval(Data.GetParameters(), simVars)) {
         PrintFormat.PrintSectionLabel(fp, "Recurrence interval", false, true);
         RecurrenceInterval_t ri = GetRecurrenceInterval(Data, iReportedCluster, simVars);
         if (ri.first < 1.0) {
            printString(buffer, "%.0lf day%s", ri.second, (ri.second < 1.5 ? "" : "s"));
         } else if (ri.first <= 10.0) {
            printString(buffer, "%.1lf year%s", ri.first, (ri.first < 1.05 ? "" : "s"));
         } else {
            printString(buffer, "%.0lf years", ri.first);
         }
         //print data to file stream
         PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
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
      PrintFormat.PrintSectionLabel(fp, "Population", false, true);
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
      PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
      break;
    default : break;
  };
}

/** Writes clusters log likelihood ratio/test statistic in format required by result output file. */
void CCluster::DisplayRatio(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  if (DataHub.GetParameters().GetProbabilityModelType() == SPACETIMEPERMUTATION) {
     PrintFormat.PrintSectionLabel(fp, "Test statistic", false, true);
     fprintf(fp, "%lf\n", m_nRatio);
  }
  else {
    PrintFormat.PrintSectionLabel(fp, "Log likelihood ratio", false, true);
    fprintf(fp, "%lf\n", m_nRatio/m_NonCompactnessPenalty);
    if (DataHub.GetParameters().GetSpatialWindowType() == ELLIPTIC) {
      PrintFormat.PrintSectionLabel(fp, "Test statistic", false, true);
      fprintf(fp, "%lf\n", m_nRatio);
    }
  }
}

/** Writes clusters relative risk in format required by result output file. */
void CCluster::DisplayRelativeRisk(FILE* fp, unsigned int iDataSetIndex, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  std::string buffer;
  double      dRelativeRisk;

  PrintFormat.PrintSectionLabel(fp, "Relative risk", false, true);
  if ((dRelativeRisk = GetRelativeRisk(DataHub, iDataSetIndex)) == -1)
    buffer = "infinity";
  else
    buffer = getValueAsString(dRelativeRisk, buffer);
  PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
}

/** Writes clusters overall relative risk in format required by result output file. */
void CCluster::DisplayObservedDivExpected(FILE* fp, unsigned int iDataSetIndex, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  std::string buffer;

  PrintFormat.PrintSectionLabel(fp, "Observed / expected", false, true);
  buffer = getValueAsString(GetObservedDivExpected(DataHub, iDataSetIndex), buffer);
  PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
}

/** Prints clusters time frame in format required by result output file. */
void CCluster::DisplayTimeFrame(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  std::string  sStart, sEnd;

  PrintFormat.PrintSectionLabel(fp, "Time frame", false, true);
  fprintf(fp, "%s to %s\n", GetStartDate(sStart, DataHub).c_str(), GetEndDate(sEnd, DataHub).c_str());
}

/** returns end date of defined cluster as formated string */
std::string& CCluster::GetEndDate(std::string& sDateString, const CSaTScanData& DataHub) const {
  return JulianToString(sDateString, DataHub.GetTimeIntervalStartTimes()[m_nLastInterval] - 1, DataHub.GetParameters().GetPrecisionOfTimesType());
}

/** Returns number of expected cases in accumulated data. */
measure_t CCluster::GetExpectedCount(const CSaTScanData& DataHub, size_t tSetIndex) const {
  return DataHub.GetMeasureAdjustment(tSetIndex) * GetClusterData()->GetMeasure(tSetIndex);
}

/** Returns number of expected cases in accumulated data that is stratified by
    ordinal categories. */
measure_t CCluster::GetExpectedCountOrdinal(const CSaTScanData& DataHub, size_t tSetIndex, size_t iCategoryIndex) const {
  const RealDataSet& DataSet = DataHub.GetDataSetHandler().GetDataSet(tSetIndex);

  return DataHub.GetProbabilityModel().GetPopulation(tSetIndex, *this, DataHub) *
             DataSet.getPopulationData().GetNumOrdinalCategoryCases(iCategoryIndex) / DataSet.getTotalPopulation();

}

/** Returns collection of location indexes that define this cluster. If 'bAtomize' is true, breaks
    down meta locations into atomic indexes. */
std::vector<tract_t> & CCluster::getLocationIndexes(const CSaTScanData& DataHub, std::vector<tract_t>& indexes, bool bAtomize) const {
   indexes.clear();
   std::vector<tract_t> atomicIndexes;
   std::vector<tract_t>::iterator itr;

   for (tract_t t=1; t <= m_nTracts; ++t) {
      tract_t n = DataHub.GetNeighbor(m_iEllipseOffset, m_Center, t, m_CartesianRadius); 
      if (n < DataHub.GetNumTracts() || !bAtomize) {
          if (!DataHub.GetIsNullifiedLocation(n))
            indexes.push_back(n);
      } else {
        DataHub.GetTInfo()->getMetaManagerProxy().getIndexes(n - DataHub.GetNumTracts(), atomicIndexes);
        for (itr=atomicIndexes.begin(); itr != atomicIndexes.end(); ++itr) {
            if (!DataHub.GetIsNullifiedLocation(*itr))
                indexes.push_back(*itr);
        }
      }
   }
      
   return indexes;
}

/** Returns index of most central location. */
tract_t CCluster::GetMostCentralLocationIndex() const {
  if (m_MostCentralLocation == -1)
    throw prg_error("Most central location of cluster not calculated.","GetMostCentralLocationIndex()");
  return m_MostCentralLocation;
}

/** Returns the number of tract in cluster that are not nullified from previous iteration
    of an iterative scan.*/
tract_t CCluster::GetNumNonNullifiedTractsInCluster(const CSaTScanData& DataHub) const {
  tract_t tClusterLocationCount=0;

  for (tract_t t=1; t <= GetNumTractsInCluster(); ++t) {
     tract_t tLocation = DataHub.GetNeighbor(GetEllipseOffset(), GetCentroidIndex(), t);
     if (!DataHub.GetIsNullifiedLocation(tLocation))
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

/** Returns relative risk of cluster.
    NOTE: Currently this only reports the relative risk of first data set. */
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
    double beta = std::sqrt(simVars.get_variance()) * std::sqrt(6.0)/PI;
    double mu = simVars.get_mean() - EULER * beta;

	double p = 1 - std::exp(-std::exp((mu - GetRatio())/beta));
	//double llr = mu - beta * std::log(std::log( 1 /( 1 - p )));

    // Determine the alternative minimum p-value. Very strong clusters will cause 
    // the calculated p-value to be computed as zero in above statement.    
    double min = (double)0.1 / std::pow(10.0, std::numeric_limits<double>::digits10 + 1.0);

    return std::make_pair(p,min);
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
        if (reportableGumbelPValue(parameters, simVars)) {
           std::pair<double,double> p = GetGumbelPValue(simVars);
           return std::max(p.first, p.second);
        }
        if (reportableMonteCarloPValue(parameters,simVars))
          return GetMonteCarloPValue(parameters, simVars, bMLC);
  }

  return p_value;
}

/** Returns relative risk for Bernoulli, ordinal and Poisson models given parameter data. */
double CCluster::GetRelativeRisk(const CSaTScanData& DataHub, size_t tSetIndex) const {
  return GetRelativeRisk(GetObservedCount(tSetIndex),
                         GetExpectedCount(DataHub, tSetIndex),
                         DataHub.GetDataSetHandler().GetDataSet(tSetIndex).getTotalCases());
}

/** Returns relative risk for Bernoulli, ordinal and Poisson models given parameter data.
    Returns negative one if relative risk goes to infinity */
double CCluster::GetRelativeRisk(double dObserved, double dExpected, double dTotalCases) const {
  //when all cases are inside cluster, relative risk goes to infinity
  if (dTotalCases == dObserved) return -1;

  if (dExpected && dTotalCases - dExpected && ((dTotalCases - dObserved)/(dTotalCases - dExpected)))
    return (dObserved/dExpected)/((dTotalCases - dObserved)/(dTotalCases - dExpected));
  return 0;  
}

/** Returns relative risk for location contained in cluster. */
double CCluster::GetRelativeRiskForTract(tract_t tTractIndex, const CSaTScanData& DataHub, size_t tSetIndex) const {
  return GetRelativeRisk(GetObservedCountForTract(tTractIndex, DataHub, tSetIndex),
                         GetExpectedCountForTract(tTractIndex, DataHub, tSetIndex),
                         DataHub.GetDataSetHandler().GetDataSet(tSetIndex).getTotalCases());
}

/** returns start date of defined cluster as formated string */
std::string& CCluster::GetStartDate(std::string& sDateString, const CSaTScanData& DataHub) const {
  return JulianToString(sDateString, DataHub.GetTimeIntervalStartTimes()[m_nFirstInterval], DataHub.GetParameters().GetPrecisionOfTimesType());
}

/** Prints name and coordinates of locations contained in cluster to ASCII file.
    Note: This is a debug function and can be helpful when used with Excel to get
    visual of cluster using scatter plotting. */
void CCluster::PrintClusterLocationsToFile(const CSaTScanData& DataHub, const std::string& sFilename) const {
  tract_t                       i, tTract;
  std::ofstream                 outfilestream(sFilename.c_str(), std::ios::ate);

  try {
    if (!outfilestream)
      throw prg_error("Could not open file for write:'%s'.\n", "PrintClusterLocationsToFile()", sFilename.c_str());

    outfilestream.setf(std::ios_base::fixed, std::ios_base::floatfield);

    std::vector<double> vCoords;
    if (DataHub.GetParameters().UseSpecialGrid()) {
      DataHub.GetGInfo()->retrieveCoordinates(GetCentroidIndex(), vCoords);
      outfilestream << "Central_Grid_Point";
      for (size_t t=0; t < vCoords.size(); ++t)
       outfilestream << " " << vCoords.at(t);
      outfilestream << std::endl;
    }                                                                                 

    for (i=1; i <= m_nTracts; ++i) {
       tTract = DataHub.GetNeighbor(m_iEllipseOffset, m_Center, i, m_CartesianRadius);
       // Print location identifiers if location data has not been removed in iterative scan.
       if (!DataHub.GetIsNullifiedLocation(tTract)) {
         const std::string&sLocationID = DataHub.GetTInfo()->getLocations().at(tTract)->getIndentifier();
         CentroidNeighborCalculator::getTractCoordinates(DataHub, *this, tTract, vCoords);
         outfilestream << sLocationID.c_str();
         for (size_t t=0; t < vCoords.size(); ++t)
           outfilestream << " " << vCoords.at(t);
         outfilestream << std::endl;
       }
    }
    outfilestream << std::endl;
  }
  catch (prg_exception& x) {
    x.addTrace("PrintClusterLocationsToFile()","CCluster");
    throw;
  }
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
bool CCluster::reportablePValue(const CParameters& parameters, const SimulationVariables& simVars) const {
  //Require at least 99 simulations requested.
  return parameters.GetNumReplicationsRequested() >= MIN_SIMULATION_RPT_PVALUE;
}

/** Returns indication whether this cluster can report recurrence interval. */
bool CCluster::reportableRecurrenceInterval(const CParameters& parameters, const SimulationVariables& simVars) const {
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

  if (ClusterDefined() && !DataHub.GetParameters().UseLocationNeighborsFile()) {
    DataHub.GetGInfo()->retrieveCoordinates(GetCentroidIndex(), ClusterCenter);
    CentroidNeighborCalculator::getTractCoordinates(DataHub, *this, DataHub.GetNeighbor(m_iEllipseOffset, m_Center, m_nTracts), TractCoords);
    if (m_iEllipseOffset) {
     CentroidNeighborCalculator::Transform(ClusterCenter[0], ClusterCenter[1], DataHub.GetEllipseAngle(m_iEllipseOffset),
                                           DataHub.GetEllipseShape(m_iEllipseOffset), &ClusterCenter[0], &ClusterCenter[1]);
     CentroidNeighborCalculator::Transform(TractCoords[0], TractCoords[1], DataHub.GetEllipseAngle(m_iEllipseOffset),
                                           DataHub.GetEllipseShape(m_iEllipseOffset), &TractCoords[0], &TractCoords[1]);
    }
    m_CartesianRadius = std::sqrt(DataHub.GetTInfo()->getDistanceSquared(ClusterCenter, TractCoords));
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

/** Set class member 'm_MostCentralLocation' from neighbor information obtained
    from CSaTScanData object. */
void CCluster::SetMostCentralLocationIndex(const CSaTScanData& DataHub) {
  if (ClusterDefined()) {
    //when iterative scan performed, we want the most central, not nullified, location
    m_MostCentralLocation = -1;
    for (tract_t t=1; t <= GetNumTractsInCluster() && m_MostCentralLocation == -1; ++t) {
       tract_t tLocation = DataHub.GetNeighbor(GetEllipseOffset(), GetCentroidIndex(), t);
       if (!DataHub.GetIsNullifiedLocation(tLocation))
         m_MostCentralLocation = tLocation;
    }
  }
}

/** Sets non compactness penalty for shape. */
void CCluster::SetNonCompactnessPenalty(double dEllipseShape, double dPower) {
  m_NonCompactnessPenalty = CalculateNonCompactnessPenalty(dEllipseShape, dPower);
}

/** Set class members 'm_CartesianRadius' and 'm_MostCentralLocation' from
    neighbor information obtained from CentroidNeighbors object. */
void CCluster::SetNonPersistantNeighborInfo(const CSaTScanData& DataHub, const CentroidNeighbors& Neighbors) {
  std::vector<double> ClusterCenter, TractCoords;

  if (ClusterDefined()) {
    DataHub.GetGInfo()->retrieveCoordinates(GetCentroidIndex(), ClusterCenter);
    CentroidNeighborCalculator::getTractCoordinates(DataHub, *this, Neighbors.GetNeighborTractIndex(m_nTracts - 1), TractCoords);
    if (m_iEllipseOffset) {
     CentroidNeighborCalculator::Transform(ClusterCenter[0], ClusterCenter[1], DataHub.GetEllipseAngle(m_iEllipseOffset),
                                           DataHub.GetEllipseShape(m_iEllipseOffset), &ClusterCenter[0], &ClusterCenter[1]);
     CentroidNeighborCalculator::Transform(TractCoords[0], TractCoords[1], DataHub.GetEllipseAngle(m_iEllipseOffset),
                                           DataHub.GetEllipseShape(m_iEllipseOffset), &TractCoords[0], &TractCoords[1]);
    }
    m_CartesianRadius = std::sqrt(DataHub.GetTInfo()->getDistanceSquared(ClusterCenter, TractCoords));
    //when iterative scan performed, we want the most central, not nullified, location
    m_MostCentralLocation = -1;
    for (tract_t t=0; t < GetNumTractsInCluster() && m_MostCentralLocation == -1; ++t) {
       if (!DataHub.GetIsNullifiedLocation(Neighbors.GetNeighborTractIndex(t)))
         m_MostCentralLocation = Neighbors.GetNeighborTractIndex(t);
    }
  }
}

/** Writes location information to stsAreaSpecificData object for each tract
    contained in cluster. */
void CCluster::Write(LocationInformationWriter& LocationWriter, const CSaTScanData& DataHub,
                     unsigned int iReportedCluster, const SimulationVariables& simVars) const {
  tract_t       tTract;
  int           i;

  try {
    LocationWriter.WritePrep(*this, DataHub);
    for (i=1; i <= m_nTracts; ++i) {
       tTract = DataHub.GetNeighbor(m_iEllipseOffset, m_Center, i, m_CartesianRadius);
       if (tTract >= DataHub.GetNumTracts() && DataHub.GetTInfo()->getMetaManagerProxy().getNumMetaLocations()) {
         //When the location index exceeds number of tracts and the meta neighbors manager contains
         //entries, we need to resolve meta location into it's real location indexes.
         std::vector<tract_t> indexes;
         DataHub.GetTInfo()->getMetaManagerProxy().getIndexes(tTract - DataHub.GetNumTracts(), indexes);
         for (size_t t=0; t < indexes.size(); ++t)
            LocationWriter.Write(*this, DataHub, iReportedCluster, indexes[t], simVars);
       }
       else
         LocationWriter.Write(*this, DataHub, iReportedCluster, tTract, simVars);
    }   
  }
  catch (prg_exception& x) {
    x.addTrace("Write()","CCluster");
    throw;
  }
}

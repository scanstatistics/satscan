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
#include <iostream>
#include <fstream>

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
  if (!(GetClusterType() == PURELYSPATIALMONOTONECLUSTER || GetClusterType() == SPATIALVARTEMPTRENDCLUSTER))
    GetClusterData()->DeallocateEvaluationAssistClassMembers();
}

/** Writes cluster properties to file stream in format required by result output file  */
void CCluster::Display(FILE* fp, const CSaTScanData& DataHub, unsigned int iReportedCluster, unsigned int iNumSimsCompleted) const {
  try {
    AsciiPrintFormat PrintFormat;
    PrintFormat.SetMarginsAsClusterSection(iReportedCluster);
    fprintf(fp, "%u.", iReportedCluster);
    DisplayCensusTracts(fp, DataHub, PrintFormat);
    if (DataHub.GetParameters().GetCoordinatesType() == CARTESIAN)
      DisplayCoordinates(fp, DataHub, PrintFormat);
    else
      DisplayLatLongCoords(fp, DataHub, PrintFormat);
    DisplayTimeFrame(fp, DataHub, PrintFormat);
    if (DataHub.GetParameters().GetProbabilityModelType() == ORDINAL)
      DisplayClusterDataOrdinal(fp, DataHub, PrintFormat);
    else if (DataHub.GetParameters().GetProbabilityModelType() == EXPONENTIAL)
      DisplayClusterDataExponential(fp, DataHub, PrintFormat);
    else if (DataHub.GetParameters().GetProbabilityModelType() == NORMAL)
      DisplayClusterDataNormal(fp, DataHub, PrintFormat);
    else
      DisplayClusterDataStandard(fp, DataHub, PrintFormat);
    DisplayRatio(fp, DataHub, PrintFormat);
    DisplayMonteCarloInformation(fp, DataHub, PrintFormat, iNumSimsCompleted);
    DisplayNullOccurrence(fp, DataHub, iNumSimsCompleted, PrintFormat);
    DisplayTimeTrend(fp, PrintFormat);
  }
  catch (ZdException &x) {
    x.AddCallpath("Display()","CCluster");
    throw;
  }
}

/** Prints annual cases to file stream is in format required by result output file. */
void CCluster::DisplayAnnualCaseInformation(FILE* fp, unsigned int iDataSetIndex, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  ZdString               sBuffer;

  if (DataHub.GetParameters().GetProbabilityModelType() == POISSON && DataHub.GetParameters().UsePopulationFile()) {
    sBuffer.printf("Annual cases / %.0f", DataHub.GetAnnualRatePop());
    PrintFormat.PrintSectionLabel(fp, sBuffer.GetCString(), false, true);
    sBuffer.printf("%.1f", DataHub.GetAnnualRateAtStart(iDataSetIndex) * GetObservedDivExpected(DataHub, iDataSetIndex));
    PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
  }
}

/** Writes cluster location identifiers to file stream in format required by result output file  */
void CCluster::DisplayCensusTracts(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const {
  try {
    PrintFormat.PrintSectionLabel(fp, "Location IDs included", false, false);  
    DisplayCensusTractsInStep(fp, Data, 1, m_nTracts, PrintFormat);
  }
  catch (ZdException &x) {
    x.AddCallpath("DisplayCensusTracts()","CCluster");
    throw;
  }
}

/** Writes clusters location information in format required by result output file. */
void CCluster::DisplayCensusTractsInStep(FILE* fp, const CSaTScanData& DataHub, tract_t nFirstTract, tract_t nLastTract, const AsciiPrintFormat& PrintFormat) const {

  unsigned int                  k;
  tract_t                       i, tTract;
  ZdString                      sLocations;
  std::vector<std::string>      vTractIdentifiers;

  try {
    for (i=nFirstTract; i <= nLastTract; ++i) {
       //get i'th neighbor tracts index
       tTract = DataHub.GetNeighbor(m_iEllipseOffset, m_Center, i, m_CartesianRadius);
       // Print location identifiers if location data has not been removed in sequential scan.
       if (!DataHub.GetIsNullifiedLocation(tTract)) {
         //get all locations ids for tract at index tTract -- might be more than one if combined
         DataHub.GetTInfo()->tiGetTractIdentifiers(tTract, vTractIdentifiers);
         for (k=0; k < vTractIdentifiers.size(); ++k) {
            if (sLocations.GetLength())
              sLocations << ", ";
            sLocations << vTractIdentifiers[k].c_str();
         }
       }
    }
    // There should be at least one location printed, else there is likely a bug in the sequential scan code.
    if (!sLocations.GetLength())
      ZdGenerateException("Attempting to print cluster with no location identifiers.","DisplayCensusTractsInStep()");

    PrintFormat.PrintAlignedMarginsDataString(fp, sLocations);
  }
  catch (ZdException &x) {
    x.AddCallpath("DisplayCensusTractsInStep()","CCluster");
    throw;
  }
}

/** Prints observed cases, expected cases and observed/expected, for exponetial model,
    to file stream is in format required by result output file. */
void CCluster::DisplayClusterDataExponential(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  ZdString                                      sBuffer;
  std::vector<unsigned int>                     vComprisedDataSetIndexes;
  std::vector<unsigned int>::iterator           itr_Index;
  std::auto_ptr<AbstractLikelihoodCalculator>   Calculator(AbstractAnalysis::GetNewLikelihoodCalculator(DataHub));

  GetClusterData()->GetDataSetIndexesComprisedInRatio(m_nRatio, *Calculator.get(), vComprisedDataSetIndexes);
  for (itr_Index=vComprisedDataSetIndexes.begin(); itr_Index != vComprisedDataSetIndexes.end(); ++itr_Index) {
     //print data set number if analyzing more than data set
     if (DataHub.GetDataSetHandler().GetNumDataSets() > 1) {
       sBuffer.printf("Data Set #%ld", *itr_Index + 1);
       PrintFormat.PrintSectionLabelAtDataColumn(fp, sBuffer.GetCString());
     }
     //print total individuals (censored and non-censored)
     PrintFormat.PrintSectionLabel(fp, "Total individuals", false, true);
     GetPopulationAsString(sBuffer, DataHub.GetProbabilityModel().GetPopulation(*itr_Index, *this, DataHub));
     PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
     //print total cases (non-censored)
     PrintFormat.PrintSectionLabel(fp, "Number of cases", false, true);
     sBuffer.printf("%ld", GetObservedCount(*itr_Index));
     PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
     //not printing censored information at Martin's directive, but leave in place for now
     ////print total censored cases
     //PrintFormat.PrintSectionLabel(fp, "Number censored cases", false, true);
     //GetPopulationAsString(sBuffer, DataHub.GetProbabilityModel().GetPopulation(*itr_Index, *this, DataHub) - GetObservedCount(*itr_Index));
     //PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
     //print expected cases label
     PrintFormat.PrintSectionLabel(fp, "Expected cases", false, true);
     //print expected cases
     sBuffer.printf("%.2f", GetExpectedCount(DataHub, *itr_Index));
     PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
     DisplayObservedDivExpected(fp, *itr_Index, DataHub, PrintFormat);
     //NOTE: Not printing relative risk information for exponential model at this time.
  }
}

/** Prints observed cases, expected cases and observed/expected, for Normal model,
    to file stream is in format required by result output file. */
void CCluster::DisplayClusterDataNormal(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  ZdString                                      sBuffer;
  std::vector<unsigned int>                     vComprisedDataSetIndexes;
  std::vector<unsigned int>::iterator           itr_Index;
  std::auto_ptr<AbstractLikelihoodCalculator>   Calculator(AbstractAnalysis::GetNewLikelihoodCalculator(DataHub));
  double                                        dEstimatedMeanInside, dEstimatedMeanOutside, dCommonVariance;
  const AbstractNormalClusterData             * pClusterData=0;
  count_t                                       tObserved;
  measure_t                                     tExpected;

  if ((pClusterData = dynamic_cast<const AbstractNormalClusterData*>(GetClusterData())) == 0)
    ZdGenerateException("Cluster data object could not be dynamically casted to AbstractNormalClusterData type.\n",
                        "DisplayClusterDataNormal()");
  const DataSetHandler& Handler = DataHub.GetDataSetHandler();
  GetClusterData()->GetDataSetIndexesComprisedInRatio(m_nRatio, *Calculator.get(), vComprisedDataSetIndexes);
  for (itr_Index=vComprisedDataSetIndexes.begin(); itr_Index != vComprisedDataSetIndexes.end(); ++itr_Index) {
     //print data set number if analyzing more than data set
     if (Handler.GetNumDataSets() > 1) {
       sBuffer.printf("Data Set #%ld", *itr_Index + 1);
       PrintFormat.PrintSectionLabelAtDataColumn(fp, sBuffer.GetCString());
     }
     //print total cases
     PrintFormat.PrintSectionLabel(fp, "Total cases", false, true);
     sBuffer.printf("%ld", GetObservedCount(*itr_Index));
     PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
     tObserved = GetObservedCount(*itr_Index);
     tExpected = GetExpectedCount(DataHub, *itr_Index);
     //print estimated mean inside label
     PrintFormat.PrintSectionLabel(fp, "Mean inside", false, true);
     //print estimated mean inside
     dEstimatedMeanInside = (tObserved ? tExpected/tObserved : 0);
     sBuffer.printf("%.2f", dEstimatedMeanInside);
     PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
     //print estimated mean outside label
     PrintFormat.PrintSectionLabel(fp, "Mean outside", false, true);
     //print estimated mean inside
     count_t tCasesOutside = Handler.GetDataSet(*itr_Index).GetTotalCases() - tObserved;
     dEstimatedMeanOutside = (tCasesOutside ? (Handler.GetDataSet(*itr_Index).GetTotalMeasure() - tExpected)/tCasesOutside : 0);
     sBuffer.printf("%.2f", dEstimatedMeanOutside);
     PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
     //print common variance label
     PrintFormat.PrintSectionLabel(fp, "Common variance", false, true);
     dCommonVariance = GetVariance(GetObservedCount(*itr_Index), GetExpectedCount(DataHub, *itr_Index), pClusterData->GetMeasureSq(*itr_Index),
                                   Handler.GetDataSet(*itr_Index).GetTotalCases(), Handler.GetDataSet(*itr_Index).GetTotalMeasure(),
                                   Handler.GetDataSet(*itr_Index).GetTotalMeasureSq());
     sBuffer.printf("%.2f", dCommonVariance);
     PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
     //print common standard deviation
     PrintFormat.PrintSectionLabel(fp, "Common standard dev.", false, true);
     sBuffer.printf("%.2f", std::sqrt(dCommonVariance));
     PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
  }
}

/** Prints observed cases, expected cases and observed/expected, for Ordinal model,
    to file stream is in format required by result output file. */
void CCluster::DisplayClusterDataOrdinal(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  ZdString                                              sWork, sBuffer;
  double                                                dTotalCasesInClusterDataSet=0;
  OrdinalLikelihoodCalculator                           Calculator(DataHub);
  std::vector<OrdinalCombinedCategory>                  vCategoryContainer;
  std::vector<OrdinalCombinedCategory>::iterator        itrCategory;
  const AbstractCategoricalClusterData                * pClusterData=0;
  std::vector<unsigned int>                             vComprisedDataSetIndexes;
  std::vector<unsigned int>::iterator                   itr_Index;

  if ((pClusterData = dynamic_cast<const AbstractCategoricalClusterData*>(GetClusterData())) == 0)
    ZdGenerateException("Cluster data object could not be dynamically casted to AbstractCategoricalClusterData type.\n",
                        "DisplayClusterDataOrdinal()");
  GetClusterData()->GetDataSetIndexesComprisedInRatio(m_nRatio, Calculator, vComprisedDataSetIndexes);

  for (itr_Index=vComprisedDataSetIndexes.begin(); itr_Index != vComprisedDataSetIndexes.end(); ++itr_Index) {
     //retrieve collection of ordinal categories in combined state
     pClusterData->GetOrdinalCombinedCategories(Calculator, vCategoryContainer, *itr_Index);
     //if container is empty, data set did not contribute to the loglikelihood ratio, so skip reporting it
     if (!vCategoryContainer.size())
       continue;
     //print data set number if analyzing more than one data set
     if (DataHub.GetDataSetHandler().GetNumDataSets() > 1) {
       sBuffer.printf("Data Set #%ld", *itr_Index + 1);
       PrintFormat.PrintSectionLabelAtDataColumn(fp, sBuffer.GetCString());
     }
     //print total cases per data set
     PrintFormat.PrintSectionLabel(fp, "Total cases", false, true);
     dTotalCasesInClusterDataSet = DataHub.GetProbabilityModel().GetPopulation(*itr_Index, *this, DataHub);
     PrintFormat.PrintAlignedMarginsDataString(fp, GetPopulationAsString(sBuffer, dTotalCasesInClusterDataSet));
     //print category ordinal values
     PrintFormat.PrintSectionLabel(fp, "Category", false, true);
     const RealDataSet& thisDataSet = DataHub.GetDataSetHandler().GetDataSet(*itr_Index);
     sBuffer = "";
     for (itrCategory=vCategoryContainer.begin(); itrCategory != vCategoryContainer.end(); ++itrCategory) {
       sBuffer << (itrCategory == vCategoryContainer.begin() ? "" : ", ");
       for (size_t m=0; m < itrCategory->GetNumCombinedCategories(); ++m) {
         sWork.printf("%s%g%s",
                      (m == 0 ? "[" : ", "),
                      thisDataSet.GetPopulationData().GetOrdinalCategoryValue(itrCategory->GetCategoryIndex(m)),
                      (m + 1 == itrCategory->GetNumCombinedCategories() ? "]" : ""));
         sBuffer << sWork;
       }
     }
     PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
     //print observed case data per category
     PrintFormat.PrintSectionLabel(fp, "Number of cases", false, true);
     sBuffer = "";
     for (itrCategory=vCategoryContainer.begin(); itrCategory != vCategoryContainer.end(); ++itrCategory) {
       count_t tObserved=0;
       for (size_t m=0; m < itrCategory->GetNumCombinedCategories(); ++m)
          tObserved += GetObservedCountOrdinal(*itr_Index, itrCategory->GetCategoryIndex(m));
       sWork.printf("%s%ld", (itrCategory == vCategoryContainer.begin() ? "" : ", "), tObserved);
       sBuffer << sWork;
     }
     PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
     //print expected case data per category
     PrintFormat.PrintSectionLabel(fp, "Expected cases", false, true);
     sBuffer = "";
     for (itrCategory=vCategoryContainer.begin(); itrCategory != vCategoryContainer.end(); ++itrCategory) {
       measure_t tExpected=0;
       for (size_t m=0; m < itrCategory->GetNumCombinedCategories(); ++m)
          tExpected += GetExpectedCountOrdinal(DataHub, *itr_Index, itrCategory->GetCategoryIndex(m));
       sWork.printf("%s%.2f", (itrCategory == vCategoryContainer.begin() ? "" : ", "), tExpected);
       sBuffer << sWork;
     }
     PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
     //print observed div expected case data per category
     PrintFormat.PrintSectionLabel(fp, "Observed / expected", false, true);
     sBuffer = "";
     for (itrCategory=vCategoryContainer.begin(); itrCategory != vCategoryContainer.end(); ++itrCategory) {
       count_t   tObserved=0;
       measure_t tExpected=0;
       for (size_t m=0; m < itrCategory->GetNumCombinedCategories(); ++m) {
          tObserved += GetObservedCountOrdinal(*itr_Index, itrCategory->GetCategoryIndex(m));
          tExpected += GetExpectedCountOrdinal(DataHub, *itr_Index, itrCategory->GetCategoryIndex(m));
       }
       sWork.printf("%s%.3f", (itrCategory == vCategoryContainer.begin() ? "" : ", "),
                    (tExpected ? (double)tObserved/tExpected  : 0));
       sBuffer << sWork;
     }
     PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
     //print relative data - note that we will possibly be combining categories
     PrintFormat.PrintSectionLabel(fp, "Relative risk", false, true);
     sBuffer = "";
     for (itrCategory=vCategoryContainer.begin(); itrCategory != vCategoryContainer.end(); ++itrCategory) {
       double           tRelativeRisk=0;
       count_t          tObserved=0, tTotalCategoryCases=0;
       measure_t        tExpected=0;
       for (size_t m=0; m < itrCategory->GetNumCombinedCategories(); ++m) {
          tObserved += GetObservedCountOrdinal(*itr_Index, itrCategory->GetCategoryIndex(m));
          tExpected += GetExpectedCountOrdinal(DataHub, *itr_Index, itrCategory->GetCategoryIndex(m));
          tTotalCategoryCases += DataHub.GetDataSetHandler().GetDataSet(*itr_Index).GetPopulationData().GetNumOrdinalCategoryCases(itrCategory->GetCategoryIndex(m));
       }
       if ((tRelativeRisk = GetRelativeRisk(tObserved, tExpected, tTotalCategoryCases)) == -1)
         sWork.printf("%sinfinity", (itrCategory == vCategoryContainer.begin() ? "" : ", "));
       else
         sWork.printf("%s%.3f", (itrCategory == vCategoryContainer.begin() ? "" : ", "), tRelativeRisk);
       sBuffer << sWork;
     }
     PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
  }
}

/** Prints population, observed cases, expected cases and relative risk
    to file stream is in format required by result output file. */
void CCluster::DisplayClusterDataStandard(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  ZdString                                      sBuffer;
  std::vector<unsigned int>                     vComprisedDataSetIndexes;
  std::vector<unsigned int>::iterator           itr_Index;
  std::auto_ptr<AbstractLikelihoodCalculator>   Calculator(AbstractAnalysis::GetNewLikelihoodCalculator(DataHub));

  DisplayPopulation(fp, DataHub, PrintFormat);

  if (GetClusterType() == PURELYSPATIALMONOTONECLUSTER || GetClusterType() == SPATIALVARTEMPTRENDCLUSTER)
    vComprisedDataSetIndexes.push_back(0);
  else
    GetClusterData()->GetDataSetIndexesComprisedInRatio(m_nRatio, *Calculator.get(), vComprisedDataSetIndexes);


  for (itr_Index=vComprisedDataSetIndexes.begin(); itr_Index != vComprisedDataSetIndexes.end(); ++itr_Index) {
     //print data set number if analyzing more than data set
     if (DataHub.GetDataSetHandler().GetNumDataSets() > 1) {
       sBuffer.printf("Data Set #%ld", *itr_Index + 1);
       PrintFormat.PrintSectionLabelAtDataColumn(fp, sBuffer.GetCString());
     }
     //print observed cases label
     PrintFormat.PrintSectionLabel(fp, "Number of cases", false, true);
     sBuffer.printf("%ld", GetObservedCount(*itr_Index));
     //print observed cases
     PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
     //print expected cases label
     PrintFormat.PrintSectionLabel(fp, "Expected cases", false, true);
     //print expected cases
     sBuffer.printf("%.2f", GetExpectedCount(DataHub, *itr_Index));
     PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
     DisplayAnnualCaseInformation(fp, *itr_Index, DataHub, PrintFormat);
     DisplayObservedDivExpected(fp, *itr_Index ,DataHub, PrintFormat);
     if (DataHub.GetParameters().GetProbabilityModelType() == POISSON  || DataHub.GetParameters().GetProbabilityModelType() == BERNOULLI)
       DisplayRelativeRisk(fp, *itr_Index, DataHub, PrintFormat);
   }
}

/** Writes clusters cartesian coordinates and ellipse properties (if cluster is elliptical)
    in format required by result output file. */
void CCluster::DisplayCoordinates(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const {
  std::vector<double>   vCoordinates;
  int                   i;
  ZdString              sBuffer, sWork;

  try {
    Data.GetGInfo()->giRetrieveCoords(m_Center, vCoordinates);

    //print coordinates differently when ellipses are requested
    if (Data.GetParameters().GetSpatialWindowType() == CIRCULAR)  {
      PrintFormat.PrintSectionLabel(fp, "Coordinates / radius", false, true);
      for (i=0; i < Data.GetParameters().GetDimensionsOfData() - 1; ++i) {
         sWork.printf("%s%g,", (i == 0 ? "(" : "" ), vCoordinates[i]);
         sBuffer << sWork;
      }
      //to keep radius value consistant with previous versions, down cast double to float
      float radius = static_cast<float>(m_CartesianRadius);
      sWork.printf("%g) / %-5.2f", vCoordinates[Data.GetParameters().GetDimensionsOfData() - 1], radius);
      sBuffer << sWork;
      PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
    }
    else {//print ellipse settings
      PrintFormat.PrintSectionLabel(fp, "Coordinates", false, true);
      for (i=0; i < Data.GetParameters().GetDimensionsOfData() - 1; ++i) {
         sWork.printf("%s%g,", (i == 0 ? "(" : "" ), vCoordinates[i]);
         sBuffer << sWork;
      }
      sWork.printf("%g)", vCoordinates[Data.GetParameters().GetDimensionsOfData() - 1]);
      sBuffer << sWork;
      PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
      //print ellipse particulars
      PrintFormat.PrintSectionLabel(fp, "Semiminor axis", false, true);
      fprintf(fp, "%-g\n", (float)m_CartesianRadius);
      PrintFormat.PrintSectionLabel(fp, "Semimajor axis", false, true);
      fprintf(fp, "%-g\n", (float)m_CartesianRadius * Data.GetEllipseShape(GetEllipseOffset()));
      PrintFormat.PrintSectionLabel(fp, "Angle (degrees)", false, true);
      fprintf(fp, "%-g\n", ConvertAngleToDegrees(Data.GetEllipseAngle(m_iEllipseOffset)));
      PrintFormat.PrintSectionLabel(fp, "Shape", false, true);
      fprintf(fp, "%-g\n", Data.GetEllipseShape(m_iEllipseOffset));
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("DisplayCoordinates()","CCluster");
    throw;
  }
}

/** Writes clusters lat/long coordinates in format required by result output file. */
void CCluster::DisplayLatLongCoords(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const {
  std::vector<double>           vCoordinates;
  std::pair<double, double>     prLatitudeLongitude;
  char                          cNorthSouth, cEastWest;

  try {
    Data.GetGInfo()->giRetrieveCoords(m_Center, vCoordinates);
    prLatitudeLongitude = ConvertToLatLong(vCoordinates);
    prLatitudeLongitude.first >= 0 ? cNorthSouth = 'N' : cNorthSouth = 'S';
    prLatitudeLongitude.second >= 0 ? cEastWest = 'E' : cEastWest = 'W';
    PrintFormat.PrintSectionLabel(fp, "Coordinates / radius", false, true);
    fprintf(fp, "(%.6f %c, %.6f %c) / %5.2lf km\n", fabs(prLatitudeLongitude.first), cNorthSouth, fabs(prLatitudeLongitude.second), cEastWest, GetLatLongRadius());
  }
  catch (ZdException &x) {
    x.AddCallpath("DisplayLatLongCoords()","CCluster");
    throw;
  }
}

/** Writes clusters monte carlo rank and p-value in format required by result output file. */
void CCluster::DisplayMonteCarloInformation(FILE* fp, const CSaTScanData& DataHub,
                                            const AsciiPrintFormat& PrintFormat,
                                            unsigned int iNumSimsCompleted) const {
  ZdString      sFormat, sReplicas;
  float         fPValue;  
                                            
  if (iNumSimsCompleted) {
    PrintFormat.PrintSectionLabel(fp, "Monte Carlo rank", false, true);
    fprintf(fp, "%u/%ld\n", m_nRank, iNumSimsCompleted+1);
  }
  if (iNumSimsCompleted > 98) {
    PrintFormat.PrintSectionLabel(fp, "P-value", false, true);
    fPValue = (float)GetPValue(iNumSimsCompleted);
    sReplicas << iNumSimsCompleted;
    sFormat.printf("%%.%df\n", sReplicas.GetLength());
    fprintf(fp, sFormat.GetCString(), fPValue);
  }
}

/** Writes clusters null occurance rate in format required by result output file. */
void CCluster::DisplayNullOccurrence(FILE* fp, const CSaTScanData& Data, unsigned int iNumSimulations, const AsciiPrintFormat& PrintFormat) const {
  float         fUnitsInOccurrence, fYears, fMonths, fDays, fIntervals, fAdjustedP_Value;
  ZdString      sBuffer;
  const float   AVERAGE_DAYS_IN_YEAR(365.25);

  try {
    if (Data.GetParameters().GetIsProspectiveAnalysis() && Data.GetParameters().GetNumReplicationsRequested() > 98) {
      PrintFormat.PrintSectionLabel(fp, "Recurrence interval", false, true);
      fIntervals = Data.GetNumTimeIntervals() - Data.GetProspectiveStartIndex() + 1;
      fAdjustedP_Value = 1 - pow(1 - GetPValue(iNumSimulations), 1/fIntervals);
      fUnitsInOccurrence = (float)Data.GetParameters().GetTimeAggregationLength()/fAdjustedP_Value;
      switch (Data.GetParameters().GetTimeAggregationUnitsType()) {
        case YEAR   : sBuffer.printf("%.1f year%s\n", fUnitsInOccurrence, (fUnitsInOccurrence > 1 ? "s" : ""));
                      break;
        case MONTH  : fYears = floor(fUnitsInOccurrence/12);
                      fMonths = fUnitsInOccurrence - (fYears * 12);
                      //we don't want to print "Once in 5 years and 12 months", so round months for it
                      if (fMonths >= 11.5) {
                        ++fYears;
                        fMonths = 0;
                      }
                      else if (fMonths < .5)
                        fMonths = 0;
                      //Print correctly formatted statement.
                      if (fMonths == 0)
                        sBuffer.printf("%.0f year%s", fYears, (fYears == 1 ? "" : "s"));
                      else if (fYears == 0)
                        sBuffer.printf("%.0f month%s", fMonths, (fMonths < 1.5 ? "" : "s"));
                      else /*Having both zero month and year should never happen.*/
                        sBuffer.printf("%.0f year%s %0.f month%s", fYears, (fYears == 1 ? "" : "s"), fMonths, (fMonths < 1.5 ? "" : "s"));
                      break;
        case DAY    : fYears = floor(fUnitsInOccurrence/AVERAGE_DAYS_IN_YEAR);
                      fDays = fUnitsInOccurrence - (fYears * AVERAGE_DAYS_IN_YEAR);
                      //we don't want to print "Once in 5 years and 0 days", so round days for it
                      if (fDays < .5)
                        fDays = 0;
                      //Print correctly formatted statement.
                      if (fDays == 0)
                        sBuffer.printf("%.0f year%s", fYears, (fYears == 1 ? "" : "s"));
                      else if (fYears == 0)
                        sBuffer.printf("%.0f day%s", fDays, (fDays < 1.5 ? "" : "s"));
                      else /*Having both zero day and year should never happen.*/
                        sBuffer.printf("%.0f year%s %.0f day%s", fYears, (fYears == 1 ? "" : "s"), fDays, (fDays < 1.5 ? "" : "s"));
                      break;
        default     : ZdGenerateException("Invalid time interval index \"%d\" for prospective analysis.",
                                          "DisplayNullOccurrence()", Data.GetParameters().GetTimeAggregationUnitsType());
      }
      //print data to file stream
      PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("DisplayNullOccurrence()","CCluster");
    throw;
  }
}

/** Writes clusters population in format required by result output file. */
void CCluster::DisplayPopulation(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  unsigned int           i;
  ZdString               sWork, sBuffer;
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
          sWork.printf("%s%g", (i > 0 ? ", " : ""), dPopulation); // display all decimals for populations less than .5
        else if (dPopulation < 1)
          sWork.printf("%s%.1f", (i > 0 ? ", " : ""), dPopulation); // display one decimal for populations less than 1
        else
          sWork.printf("%s%.0f", (i > 0 ? ", " : ""), dPopulation); // else display no decimals
        sBuffer << sWork;
      }
      PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
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
  ZdString      sBuffer;
  double        dRelativeRisk;

  PrintFormat.PrintSectionLabel(fp, "Relative risk", false, true);
  if ((dRelativeRisk = GetRelativeRisk(DataHub, iDataSetIndex)) == -1)
    sBuffer = "infinity";
  else
    sBuffer.printf("%.3f", dRelativeRisk);
  PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
}

/** Writes clusters overall relative risk in format required by result output file. */
void CCluster::DisplayObservedDivExpected(FILE* fp, unsigned int iDataSetIndex, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  ZdString      sBuffer;

  PrintFormat.PrintSectionLabel(fp, "Observed / expected", false, true);
  sBuffer.printf("%.3f", GetObservedDivExpected(DataHub, iDataSetIndex));
  PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
}

/** Prints clusters time frame in format required by result output file. */
void CCluster::DisplayTimeFrame(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  ZdString  sStart, sEnd;

  PrintFormat.PrintSectionLabel(fp, "Time frame", false, true);
  fprintf(fp, "%s - %s\n", GetStartDate(sStart, DataHub).GetCString(), GetEndDate(sEnd, DataHub).GetCString());
}

/** returns end date of defined cluster as formated string */
ZdString& CCluster::GetEndDate(ZdString& sDateString, const CSaTScanData& DataHub) const {
  return JulianToString(sDateString, DataHub.GetTimeIntervalStartTimes()[m_nLastInterval] - 1);
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
             DataSet.GetPopulationData().GetNumOrdinalCategoryCases(iCategoryIndex) / DataSet.GetTotalPopulation();

}

/** Returns index of most central location. */
tract_t CCluster::GetMostCentralLocationIndex() const {
  if (m_MostCentralLocation == -1)
    ZdGenerateException("Most central location of cluster not calculated.","GetMostCentralLocationIndex()");
  return m_MostCentralLocation;
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

/** Returns population as string with varied precision, based upon value. */
ZdString & CCluster::GetPopulationAsString(ZdString& sString, double dPopulation) const {
  if (dPopulation < .5)
    sString.printf("%g", dPopulation); // display all decimals for populations less than .5
  else if (dPopulation < 1)
    sString.printf("%.1f", dPopulation); // display one decimal for populations less than 1
  else
    sString.printf("%.0f", dPopulation); // else display no decimals

  return sString;
}

/** Returns p-value, rank / (number of simulations completed + 1), of cluster. */
double CCluster::GetPValue(unsigned int uiNumSimulationsCompleted) const {
  return (double)m_nRank/(double)(uiNumSimulationsCompleted+1);
}

/** Returns relative risk for Bernoulli, ordinal and Poisson models given parameter data. */
double CCluster::GetRelativeRisk(const CSaTScanData& DataHub, size_t tSetIndex) const {
  return GetRelativeRisk(GetObservedCount(tSetIndex),
                         GetExpectedCount(DataHub, tSetIndex),
                         DataHub.GetDataSetHandler().GetDataSet(tSetIndex).GetTotalCases());
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
                         DataHub.GetDataSetHandler().GetDataSet(tSetIndex).GetTotalCases());
}

/** returns start date of defined cluster as formated string */
ZdString& CCluster::GetStartDate(ZdString& sDateString, const CSaTScanData& DataHub) const {
  return JulianToString(sDateString, DataHub.GetTimeIntervalStartTimes()[m_nFirstInterval]);
}

/** Prints name and coordinates of locations contained in cluster to ASCII file.
    Note: This is a debug function and can be helpful when used with Excel to get
    visual of cluster using scatter plotting. */
void CCluster::PrintClusterLocationsToFile(const CSaTScanData& DataHub, const std::string& sFilename) const {
  unsigned int                  k;
  tract_t                       i, tTract;
  std::string                   sLocationID;
  std::ofstream                 outfilestream(sFilename.c_str(), ios::ate);

  try {
    if (!outfilestream)
      ZdGenerateException("Error: Could not open file for write:'%s'.\n", "PrintClusterLocationsToFile()", sFilename.c_str());

    outfilestream.setf(std::ios_base::fixed, std::ios_base::floatfield);

    std::vector<double> vCoords;
    if (DataHub.GetParameters().UseSpecialGrid()) {
      DataHub.GetGInfo()->giRetrieveCoords(GetCentroidIndex(), vCoords);
      outfilestream << "Central_Grid_Point";
      for (size_t t=0; t < vCoords.size(); ++t)
       outfilestream << " " << vCoords.at(t);
      outfilestream << std::endl;
    }                                                                                 

    for (i=1; i <= m_nTracts; ++i) {
       tTract = DataHub.GetNeighbor(m_iEllipseOffset, m_Center, i, m_CartesianRadius);
       // Print location identifiers if location data has not been removed in sequential scan.
       if (!DataHub.GetIsNullifiedLocation(tTract)) {
         DataHub.GetTInfo()->tiGetTid(tTract, sLocationID);
         DataHub.GetTInfo()->tiRetrieveCoords(tTract, vCoords);
         outfilestream << sLocationID.c_str();
         for (size_t t=0; t < vCoords.size(); ++t)
           outfilestream << " " << vCoords.at(t);
         outfilestream << std::endl;
       }
    }
    outfilestream << std::endl;
  }
  catch (ZdException &x) {
    x.AddCallpath("PrintClusterLocationsToFile()","CCluster");
    throw;
  }
}

/** Set class member 'm_CartesianRadius' from neighbor information obtained from
    CSaTScanData object. */
void CCluster::SetCartesianRadius(const CSaTScanData& DataHub) {
  std::vector<double> vCoordsOfCluster;
  std::vector<double> vCoordsOfNeighborCluster;

  if (ClusterDefined()) {
    DataHub.GetGInfo()->giRetrieveCoords(GetCentroidIndex(), vCoordsOfCluster);
    DataHub.GetTInfo()->tiRetrieveCoords(DataHub.GetNeighbor(m_iEllipseOffset, m_Center, m_nTracts), vCoordsOfNeighborCluster);
    if (m_iEllipseOffset) {
     CentroidNeighborCalculator::Transform(vCoordsOfCluster[0], vCoordsOfCluster[1], DataHub.GetEllipseAngle(m_iEllipseOffset),
                                           DataHub.GetEllipseShape(m_iEllipseOffset), &vCoordsOfCluster[0], &vCoordsOfCluster[1]);
     CentroidNeighborCalculator::Transform(vCoordsOfNeighborCluster[0], vCoordsOfNeighborCluster[1], DataHub.GetEllipseAngle(m_iEllipseOffset),
                                           DataHub.GetEllipseShape(m_iEllipseOffset), &vCoordsOfNeighborCluster[0], &vCoordsOfNeighborCluster[1]);
    }
    m_CartesianRadius = std::sqrt(DataHub.GetTInfo()->tiGetDistanceSq(vCoordsOfCluster, vCoordsOfNeighborCluster));
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
  m_MostCentralLocation = DataHub.GetNeighbor(GetEllipseOffset(), GetCentroidIndex(), 1);
}

/** Sets non compactness penalty for shape. */
void CCluster::SetNonCompactnessPenalty(double dEllipseShape, double dPower) {
  m_NonCompactnessPenalty = CalculateNonCompactnessPenalty(dEllipseShape, dPower);
}

/** Set class members 'm_CartesianRadius' and 'm_MostCentralLocation' from
    neighbor information obtained from CentroidNeighbors object. */
void CCluster::SetNonPersistantNeighborInfo(const CSaTScanData& DataHub, const CentroidNeighbors& Neighbors) {
  std::vector<double> vCoordsOfCluster;
  std::vector<double> vCoordsOfNeighborCluster;

  if (ClusterDefined()) {
    DataHub.GetGInfo()->giRetrieveCoords(GetCentroidIndex(), vCoordsOfCluster);
    DataHub.GetTInfo()->tiRetrieveCoords(Neighbors.GetNeighborTractIndex(m_nTracts - 1), vCoordsOfNeighborCluster);
    if (m_iEllipseOffset) {
     CentroidNeighborCalculator::Transform(vCoordsOfCluster[0], vCoordsOfCluster[1], DataHub.GetEllipseAngle(m_iEllipseOffset),
                                           DataHub.GetEllipseShape(m_iEllipseOffset), &vCoordsOfCluster[0], &vCoordsOfCluster[1]);
     CentroidNeighborCalculator::Transform(vCoordsOfNeighborCluster[0], vCoordsOfNeighborCluster[1], DataHub.GetEllipseAngle(m_iEllipseOffset),
                                           DataHub.GetEllipseShape(m_iEllipseOffset), &vCoordsOfNeighborCluster[0], &vCoordsOfNeighborCluster[1]);
    }
    m_CartesianRadius = std::sqrt(DataHub.GetTInfo()->tiGetDistanceSq(vCoordsOfCluster, vCoordsOfNeighborCluster));
    m_MostCentralLocation = Neighbors.GetNeighborTractIndex(0);
  }
}

/** Sets scanning area rate. */
void CCluster::SetRate(int nRate) {
  switch (nRate) {
    case HIGH       : m_pfRateOfInterest = HighRate;      break;
    case LOW        : m_pfRateOfInterest = LowRate;       break;
    case HIGHANDLOW : m_pfRateOfInterest = HighOrLowRate; break;
    default         : ;
  }
}

/** Writes location information to stsAreaSpecificData object for each tract
    contained in cluster. */
void CCluster::Write(LocationInformationWriter& LocationWriter, const CSaTScanData& DataHub,
                     unsigned int iReportedCluster, unsigned int iNumSimsCompleted) const {
  tract_t       tTract;
  int           i;

  try {
    for (i=1; i <= m_nTracts; i++) {
       tTract = DataHub.GetNeighbor(m_iEllipseOffset, m_Center, i, m_CartesianRadius);
       LocationWriter.Write(*this, DataHub, iReportedCluster, tTract, iNumSimsCompleted);
    }   
  }
  catch (ZdException &x) {
    x.AddCallpath("Write()","CCluster");
    throw;
  }
}

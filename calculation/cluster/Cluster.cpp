//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "cluster.h"
#include "stsAreaSpecificData.h"

/** constructor */
CCluster::CCluster() {
  Initialize();
}

/** destructor */
CCluster::~CCluster() {}

/** initializes cluster data  */
void CCluster::Initialize(tract_t nCenter) {
  m_Center         = nCenter;
  m_nTracts        = 0;
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
  m_nTracts               = rhs.m_nTracts;
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

/** Writes cluster properties to file stream in format required by result output file  */
void CCluster::Display(FILE* fp, const CSaTScanData& DataHub, unsigned int iReportedCluster, unsigned int iNumSimsCompleted) const {
  try {
    AsciiPrintFormat PrintFormat;
    PrintFormat.SetMarginsAsClusterSection(iReportedCluster);
    fprintf(fp, "%u.", iReportedCluster);
    DisplayCensusTracts(fp, DataHub, PrintFormat);
    DisplaySteps(fp, PrintFormat);
    if (DataHub.GetParameters().GetCoordinatesType() == CARTESIAN)
      DisplayCoordinates(fp, DataHub, PrintFormat);
    else
      DisplayLatLongCoords(fp, DataHub, PrintFormat);
    DisplayTimeFrame(fp, DataHub, PrintFormat);
    DisplayPopulation(fp, DataHub, PrintFormat);
    DisplayCaseInformation(fp, DataHub, PrintFormat);
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
void CCluster::DisplayAnnualCaseInformation(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  unsigned int                  i;
  ZdString                      sWork, sBuffer;
  const DataStreamHandler     & Streams = DataHub.GetDataStreamHandler();

  if (DataHub.GetParameters().GetProbabilityModelType() == POISSON && DataHub.GetParameters().UsePopulationFile()) {
    sBuffer.printf("Annual cases / %.0f", DataHub.GetAnnualRatePop());
    PrintFormat.PrintSectionLabel(fp, sBuffer.GetCString(), false, true);
    sBuffer.printf("%.1f", DataHub.GetAnnualRateAtStart(0) * GetRelativeRisk(DataHub.GetMeasureAdjustment(0), 0));
    for (i=1; i < Streams.GetNumDataSets(); ++i) {
       sWork.printf(", %.1f", DataHub.GetAnnualRateAtStart(i) * GetRelativeRisk(DataHub.GetMeasureAdjustment(i), i));
       sBuffer << sWork;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
  }
}

/** Prints number of observed and expected cases to file stream is in format
    required by result output file. */
void CCluster::DisplayCaseInformation(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  unsigned int                  i, j, k;
  ZdString                      sWork, sBuffer, sNullString;
  const DataStreamHandler     & DataSets = DataHub.GetDataStreamHandler();

  if (DataHub.GetParameters().GetProbabilityModelType() == ORDINAL) {
    for (i=0; i < DataSets.GetNumDataSets(); ++i) {
       const RealDataStream& RealSet = DataSets.GetStream(i);
       //print data set number if more than data set 
       if (DataSets.GetNumDataSets() > 1) {
         sWork.printf("Data Set #%ld", i + 1);
         PrintFormat.PrintSectionLabel(fp, sWork.GetCString(), false, true);
         PrintFormat.PrintAlignedMarginsDataString(fp, sNullString);
       }
       //print category ordinal values
       PrintFormat.PrintSectionLabel(fp, "Category", false, true);
       sBuffer.printf("%g", RealSet.GetPopulationData().GetOrdinalCategoryValue(0));
       for (k=1; k < RealSet.GetPopulationData().GetNumOrdinalCategories(); ++k) {
         sWork.printf(", %g", RealSet.GetPopulationData().GetOrdinalCategoryValue(k));
         sBuffer << sWork;
       }
       PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
       //print observed case data per category
       PrintFormat.PrintSectionLabel(fp, "Number of cases", false, true);
       sBuffer.printf("%ld", GetClusterData()->GetCategoryCaseCount(0, i));
       for (k=1; k < RealSet.GetPopulationData().GetNumOrdinalCategories(); ++k) {
         sWork.printf(", %ld", GetClusterData()->GetCategoryCaseCount(k, i));
         sBuffer << sWork;
       }
       PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
       //print expected case data per category
       //$$ This process should be similar to Bernoulli.
       //$$
       //$$ category 'z'
       //$$ RemaingCases = 0;
       //$$ for (k=1; k < RealSet.GetPopulationData().GetNumOrdinalCategories(); ++k)
       //$$   if (k != z)
       //$$     RemaingCases += GetClusterData()->GetCategoryCaseCount(k, i);
       //$$ (RealSet.GetPopulationData().GetNumOrdinalCategoryCases(z) * RealSet.GetTotalCases()) * RemaingCases;
       //$$
       PrintFormat.PrintSectionLabel(fp, "Expected cases", false, true);
       sBuffer = "?";
       PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
       //print observed div expected case data per category
       //$$ Waiting on Martin for details.
       PrintFormat.PrintSectionLabel(fp, "Observed / expected", false, true);
       sBuffer = "?";
       PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
    }
  }
  else {
    PrintFormat.PrintSectionLabel(fp, "Number of cases", false, true);
    sBuffer.printf("%ld", GetCaseCount(0));
    for (i=1; i < DataSets.GetNumDataSets(); ++i) {
       sWork.printf(", %ld", GetCaseCount(i));
       sBuffer << sWork;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
    //print expected cases label
    PrintFormat.PrintSectionLabel(fp, "Expected cases", false, true);
    //print expected cases
    sBuffer.printf("%.2f", DataHub.GetMeasureAdjustment(0) * GetMeasure(0));
    for (i=1; i < DataSets.GetNumDataSets(); ++i) {
       sWork.printf(", %.2f", DataHub.GetMeasureAdjustment(i) * GetMeasure(i));
       sBuffer << sWork;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
    DisplayAnnualCaseInformation(fp, DataHub, PrintFormat);
    DisplayRelativeRisk(fp, DataHub, PrintFormat);
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
       tTract = DataHub.GetNeighbor(m_iEllipseOffset, m_Center, i);
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

/** Writes clusters cartesian coordinates and ellipse properties (if cluster is elliptical)
    in format required by result output file. */
void CCluster::DisplayCoordinates(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const {
  double      * pCoords = 0, * pCoords2 = 0;
  float         nRadius;
  int           i, j, count=0;
  ZdString      sBuffer, sWork;

  try {
    Data.GetGInfo()->giGetCoords(m_Center, &pCoords);
    Data.GetTInfo()->tiGetCoords(Data.GetNeighbor(m_iEllipseOffset, m_Center, m_nTracts), &pCoords2);
    nRadius = (float)sqrt((Data.GetTInfo())->tiGetDistanceSq(pCoords, pCoords2));

    //print coordinates differently for the circles and ellipses
    if (m_iEllipseOffset == 0)  {//print coordinates for circle
      PrintFormat.PrintSectionLabel(fp, "Coordinates / radius", false, true);
      for (i=0; i < Data.GetParameters().GetDimensionsOfData() - 1; ++i) {
         sWork.printf("%s%g,", (i == 0 ? "(" : "" ), pCoords[i]);
         sBuffer << sWork;
      }
      sWork.printf("%g) / %-5.2f", pCoords[Data.GetParameters().GetDimensionsOfData() - 1], nRadius);
      sBuffer << sWork;
      PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
      //print ellipse particulars - circle with shape of '1'
      if (Data.GetParameters().GetNumRequestedEllipses()) {
        PrintFormat.PrintSectionLabel(fp, "Ellipse Parameters", false, true);
        fprintf(fp, "\n");
        PrintFormat.PrintSectionLabel(fp, "Angle (degrees)", false, true);
        fprintf(fp, "n/a\n");
        PrintFormat.PrintSectionLabel(fp, "Shape", false, true);
        fprintf(fp, "1.0\n");
      }
    }
    else {//print ellipse settings
      PrintFormat.PrintSectionLabel(fp, "Coordinates", false, true);
      for (i=0; i < Data.GetParameters().GetDimensionsOfData() - 1; ++i) {
         sWork.printf("%s%g,", (i == 0 ? "(" : "" ), pCoords[i]);
         sBuffer << sWork;
      }
      sWork.printf("%g)", pCoords[Data.GetParameters().GetDimensionsOfData() - 1]);
      sBuffer << sWork;
      PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
      //print ellipse particulars
      PrintFormat.PrintSectionLabel(fp, "Ellipse Semiminor axis", false, true);
      fprintf(fp, "%-g\n", nRadius);
      PrintFormat.PrintSectionLabel(fp, "Ellipse Parameters", false, true);
      fprintf(fp, "\n");
      PrintFormat.PrintSectionLabel(fp, "Angle (degrees)", false, true);
      fprintf(fp, "%-g\n", ConvertAngleToDegrees(Data.GetEllipseAngle(m_iEllipseOffset)));
      PrintFormat.PrintSectionLabel(fp, "Shape", false, true);
      fprintf(fp, "%-g\n", Data.GetEllipseShape(m_iEllipseOffset));
    }
    free(pCoords);
    free(pCoords2);
  }
  catch (ZdException &x) {
    free(pCoords);
    free(pCoords2);
    x.AddCallpath("DisplayCoordinates()","CCluster");
    throw;
  }
}

/** Writes clusters lat/long coordinates in format required by result output file. */
void CCluster::DisplayLatLongCoords(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const {
  double  dRadius, * pCoords = 0, * pCoords2 = 0, EARTH_RADIUS = 6367/*radius of earth in km*/;
  float   Latitude, Longitude;
  char    cNorthSouth, cEastWest;

  try {
    Data.GetGInfo()->giGetCoords(m_Center, &pCoords);
    Data.GetTInfo()->tiGetCoords(Data.GetNeighbor(0, m_Center, m_nTracts), &pCoords2);
    dRadius = 2 * EARTH_RADIUS * asin(sqrt(Data.GetTInfo()->tiGetDistanceSq(pCoords, pCoords2))/(2 * EARTH_RADIUS));

    ConvertToLatLong(&Latitude, &Longitude, pCoords);
    Latitude >= 0 ? cNorthSouth = 'N' : cNorthSouth = 'S';
    Longitude >= 0 ? cEastWest = 'E' : cEastWest = 'W';
    PrintFormat.PrintSectionLabel(fp, "Coordinates / radius", false, true);
    fprintf(fp, "(%.6f %c, %.6f %c) / %5.2lf km\n",
            fabs(Latitude), cNorthSouth, fabs(Longitude), cEastWest, dRadius);
    free(pCoords);
    free(pCoords2);
  }
  catch (ZdException &x) {
    free(pCoords);
    free(pCoords2);
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

  try {
    if (Data.GetParameters().GetIsProspectiveAnalysis() && Data.GetParameters().GetNumReplicationsRequested() > 98) {
      PrintFormat.PrintSectionLabel(fp, "Null Occurrence", false, true);
      fIntervals = Data.GetNumTimeIntervals() - Data.GetProspectiveStartIndex() + 1;
      fAdjustedP_Value = 1 - pow(1 - GetPValue(iNumSimulations), 1/fIntervals);
      fUnitsInOccurrence = (float)Data.GetParameters().GetTimeAggregationLength()/fAdjustedP_Value;
      switch (Data.GetParameters().GetTimeAggregationUnitsType()) {
        case YEAR   : sBuffer.printf("Once in %.1f year%s\n", fUnitsInOccurrence, (fUnitsInOccurrence > 1 ? "s" : ""));
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
                        sBuffer.printf("Once in %.0f year%s", fYears, (fYears == 1 ? "" : "s"));
                      else if (fYears == 0)
                        sBuffer.printf("Once in %.0f month%s", fMonths, (fMonths < 1.5 ? "" : "s"));
                      else /*Having both zero month and year should never happen.*/
                        sBuffer.printf("Once in %.0f year%s and %0.f month%s", fYears, (fYears == 1 ? "" : "s"), fMonths, (fMonths < 1.5 ? "" : "s"));
                      break;
        case DAY    : fYears = floor(fUnitsInOccurrence/AVERAGE_DAYS_IN_YEAR);
                      fDays = fUnitsInOccurrence - (fYears * AVERAGE_DAYS_IN_YEAR);
                      //we don't want to print "Once in 5 years and 0 days", so round days for it
                      if (fDays < .5)
                        fDays = 0;
                      //Print correctly formatted statement.
                      if (fDays == 0)
                        sBuffer.printf("Once in %.0f year%s", fYears, (fYears == 1 ? "" : "s"));
                      else if (fYears == 0)
                        sBuffer.printf("Once in %.0f day%s", fDays, (fDays < 1.5 ? "" : "s"));
                      else /*Having both zero day and year should never happen.*/
                        sBuffer.printf("Once in %.0f year%s and %.0f day%s", fYears, (fYears == 1 ? "" : "s"), fDays, (fDays < 1.5 ? "" : "s"));
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
void CCluster::DisplayPopulation(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const {
  unsigned int                  i;
  ZdString                      sWork, sBuffer;
  const DataStreamHandler     & Streams = Data.GetDataStreamHandler();
  double                        dPopulation;

  if ((Data.GetParameters().GetProbabilityModelType() == POISSON && Data.GetParameters().UsePopulationFile())
      || Data.GetParameters().GetProbabilityModelType() == BERNOULLI) {
    PrintFormat.PrintSectionLabel(fp, "Population", false, true);

    for (i=0; i < Streams.GetNumDataSets(); ++i) {
      dPopulation = Data.GetProbabilityModel().GetPopulation(i, m_iEllipseOffset, m_Center,
                                                             m_nTracts, m_nFirstInterval, m_nLastInterval);
      if (dPopulation < .5)
        sWork.printf("%s%g", (i > 0 ? ", " : ""), dPopulation); // display all decimals for populations less than .5
      else if (dPopulation < 1)
        sWork.printf("%s%.1f", (i > 0 ? ", " : ""), dPopulation); // display one decimal for populations less than 1
      else
        sWork.printf("%s%.0f", (i > 0 ? ", " : ""), dPopulation); // else display no decimals
      sBuffer << sWork;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
  }
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
    if (DataHub.GetParameters().GetNonCompactnessPenalty()) {
      PrintFormat.PrintSectionLabel(fp, "Test statistic", false, true);
      fprintf(fp, "%lf\n", m_nRatio);
    }
  }
}

/** Writes clusters overall relative risk in format required by result output file. */
void CCluster::DisplayRelativeRisk(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  const DataStreamHandler & Streams = DataHub.GetDataStreamHandler();
  unsigned int              i;
  ZdString                  sBuffer, sWork;

  PrintFormat.PrintSectionLabel(fp, "Observed / expected", false, true);
  sBuffer.printf("%.3f", GetRelativeRisk(DataHub.GetMeasureAdjustment(0), 0));
  for (i=1; i < Streams.GetNumDataSets(); ++i) {
    sWork.printf(", %.3f", GetRelativeRisk(DataHub.GetMeasureAdjustment(i), i));
    sBuffer << sWork;
  }
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

/** Returns p-value, rank / (number of simulations completed + 1), of cluster. */
const double CCluster::GetPValue(unsigned int uiNumSimulationsCompleted) const {
  return (double)m_nRank/(double)(uiNumSimulationsCompleted+1);
}

/** Returns relative risk of cluster.
    NOTE: Currently this only reports the relative risk of first data stream. */
const double CCluster::GetRelativeRisk(double nMeasureAdjustment, unsigned int iStream) const {
  double        dRelativeRisk=0;

  if (GetMeasure(iStream) * nMeasureAdjustment)
    dRelativeRisk = ((double)GetCaseCount(iStream))/(GetMeasure(iStream) * nMeasureAdjustment);

  return dRelativeRisk;
}

/** Returns the relative risk for tract as defined by cluster. */
double CCluster::GetRelativeRiskForTract(tract_t tTract, const CSaTScanData& DataHub, unsigned int iStream) const {
  double        dRelativeRisk=0;
  count_t       tCaseCount;
  measure_t     tMeasure;

  tCaseCount = GetCaseCountForTract(tTract, DataHub, iStream);
  tMeasure = GetMeasureForTract(tTract, DataHub, iStream);

  if (tMeasure)
    dRelativeRisk = ((double)(tCaseCount))/tMeasure;
  return dRelativeRisk;
}

/** returns start date of defined cluster as formated string */
ZdString& CCluster::GetStartDate(ZdString& sDateString, const CSaTScanData& DataHub) const {
  return JulianToString(sDateString, DataHub.GetTimeIntervalStartTimes()[m_nFirstInterval]);
}

/** Sets centroid index of cluster as defined in CSaTScanData. */
void CCluster::SetCenter(tract_t nCenter) {
  m_Center = nCenter;
}

/** Set ellipse offset as defined in CSaTScanData. */
void CCluster::SetEllipseOffset(int iOffset) {
   m_iEllipseOffset = iOffset;
}

/** Sets non compactness penalty for shape. */
void CCluster::SetNonCompactnessPenalty(double dEllipseShape) {
  m_NonCompactnessPenalty = CalculateNonCompactnessPenalty(dEllipseShape);
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
void CCluster::Write(stsAreaSpecificData& AreaData, const CSaTScanData& DataHub,
                     unsigned int iReportedCluster, unsigned int iNumSimsCompleted) const {
  tract_t       tTract;
  int           i;

  try {
    for (i=1; i <= m_nTracts; i++) {
       tTract = DataHub.GetNeighbor(m_iEllipseOffset, m_Center, i);
       AreaData.RecordClusterData(*this, DataHub, iReportedCluster, tTract, iNumSimsCompleted);
    }   
  }
  catch (ZdException &x) {
    x.AddCallpath("Write(stsAreaSpecificData*)","CCluster");
    throw;
  }
}

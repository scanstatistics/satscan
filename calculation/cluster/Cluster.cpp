//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "cluster.h"
#include "stsAreaSpecificData.h"

/** constructor */
ClusterPrintFormat::ClusterPrintFormat(int iLeftMargin, int iRightMargin, char cDeliminator)
                   :giLeftMargin(iLeftMargin), giRightMargin(iRightMargin), gcDeliminator(cDeliminator) {
  gsSpacesOnLeft[0] = '\0';
}

/** destructor */
ClusterPrintFormat::~ClusterPrintFormat() {}

/** Adjusts left margin give cluster number */
void ClusterPrintFormat::SetLeftMargin(unsigned int iClusterNumber) {
  // Adjust for spacing of cluster number
  strcpy(gsSpacesOnLeft, "  ");
  int n = (int)floor(((double)iClusterNumber)/10);
  while (n > 0) {
      strcat(gsSpacesOnLeft, " ");
      ++giLeftMargin;
      n = (int)floor(((double)n)/10);
  }
}


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
  m_DuczmalCorrection = 1;
  m_nFirstInterval = 0;
  m_nLastInterval  = 0;
  m_iEllipseOffset = 0;
}

/** overloaded assignment operator */
CCluster& CCluster::operator=(const CCluster& rhs) {
  m_Center              = rhs.m_Center;
  m_nTracts             = rhs.m_nTracts;
  m_nRatio              = rhs.m_nRatio;
  m_nRank               = rhs.m_nRank;
  m_DuczmalCorrection   = rhs.m_DuczmalCorrection;
  m_nFirstInterval      = rhs.m_nFirstInterval;
  m_nLastInterval       = rhs.m_nLastInterval;
  m_iEllipseOffset      = rhs.m_iEllipseOffset;

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
void CCluster::Display(FILE* fp, const CParameters& Parameters, const CSaTScanData& Data,
                       unsigned int iReportedCluster, measure_t nMinMeasure, unsigned int iNumSimsCompleted) const {
  ClusterPrintFormat PrintFormat(26, 67, ',');

  try {
    fprintf(fp, "%u.", iReportedCluster);
    PrintFormat.SetLeftMargin(iReportedCluster);
    DisplaySteps(fp, PrintFormat);
    fprintf(fp, "Location IDs ");
    DisplayCensusTracts(fp, Data, nMinMeasure, PrintFormat);
    if (Parameters.GetCoordinatesType() == CARTESIAN)
      DisplayCoordinates(fp, Data, PrintFormat);
    else
      DisplayLatLongCoords(fp, Data, PrintFormat);
    DisplayTimeFrame(fp, Data, PrintFormat);
    if (Parameters.GetProbabiltyModelType() == POISSON || Parameters.GetProbabiltyModelType() == BERNOULLI)
      DisplayPopulation(fp, Data, PrintFormat);
    fprintf(fp, "%sNumber of cases.......: %ld", PrintFormat.GetSpacesOnLeft(), GetCaseCount(0));
    fprintf(fp, "          (%.2f expected)\n", Data.GetMeasureAdjustment() * GetMeasure(0));
    if (Parameters.GetProbabiltyModelType() == POISSON)
      fprintf(fp, "%sAnnual cases / %.0f.: %.1f\n", PrintFormat.GetSpacesOnLeft(), Data.GetAnnualRatePop(),
              Data.GetAnnualRateAtStart()*GetRelativeRisk(Data.GetMeasureAdjustment()));
    DisplayRelativeRisk(fp, Data.GetMeasureAdjustment(), PrintFormat);
    //Print Loglikelihood/Test Statistic
    if (Parameters.GetProbabiltyModelType() == SPACETIMEPERMUTATION)
      fprintf(fp, "%sTest statistic........: %f\n", PrintFormat.GetSpacesOnLeft(), m_nRatio);
    else {
      fprintf(fp, "%sLog likelihood ratio..: %f\n", PrintFormat.GetSpacesOnLeft(), m_nRatio/m_DuczmalCorrection);
      if (Parameters.GetDuczmalCorrectEllipses())
        fprintf(fp, "%sTest statistic........: %f\n", PrintFormat.GetSpacesOnLeft(), m_nRatio);
    }
    if (iNumSimsCompleted)
      fprintf(fp, "%sMonte Carlo rank......: %u/%ld\n", PrintFormat.GetSpacesOnLeft(), m_nRank, iNumSimsCompleted+1);
    if (iNumSimsCompleted > 98) {
        fprintf(fp, "%sP-value...............: ", PrintFormat.GetSpacesOnLeft());
        DisplayPValue(fp, iNumSimsCompleted, PrintFormat);
        fprintf(fp, "\n");
    }
    DisplayNullOccurrence(fp, Data, iNumSimsCompleted, PrintFormat);
    DisplayTimeTrend(fp, PrintFormat);
  }
  catch (ZdException &x) {
    x.AddCallpath("Display()","CCluster");
    throw;
  }
}

/** Writes cluster location identifiers to file stream in format required by result output file  */
void CCluster::DisplayCensusTracts(FILE* fp, const CSaTScanData& Data, measure_t nMinMeasure,
                                   const ClusterPrintFormat& PrintFormat) const {
  try {
    if (PrintFormat.GetLeftMargin() > 0)
      fprintf(fp, "included.: ");
    DisplayCensusTractsInStep(fp, Data, 1, m_nTracts, nMinMeasure, PrintFormat);
  }
  catch (ZdException &x) {
    x.AddCallpath("DisplayCensusTracts()","CCluster");
    throw;
  }
}

/** Writes clusters location information in format required by result output file. */
void CCluster::DisplayCensusTractsInStep(FILE* fp, const CSaTScanData& Data,
                                         tract_t nFirstTract, tract_t nLastTract,
                                         measure_t nMinMeasure, const ClusterPrintFormat& PrintFormat) const {
                                         
  int                                  pos  = PrintFormat.GetLeftMargin(), nCount=0;
  tract_t                              tTract;
  std::vector<std::string>             vTractIdentifiers;
  measure_t                         ** ppMeasure(Data.GetDataStreamHandler().GetStream(0/*for now*/).GetMeasureArray());

  try {
    for (int i=nFirstTract; i <= nLastTract; i++) {
       tTract = Data.GetNeighbor(m_iEllipseOffset, m_Center, i);
       if (ppMeasure[0][tTract] > nMinMeasure) {
         Data.GetTInfo()->tiGetTractIdentifiers(tTract, vTractIdentifiers);
         for (int k=0; k < (int)vTractIdentifiers.size(); k++) {
            pos += strlen(vTractIdentifiers[k].c_str()) + 2;
            if (nCount > 1 && pos > PrintFormat.GetRightMargin()) { 
              pos = PrintFormat.GetLeftMargin() + strlen(vTractIdentifiers[k].c_str()) + 2;
              fprintf(fp, "\n");
              for (int j=0; j < PrintFormat.GetLeftMargin(); ++j)
                 fprintf(fp, " ");
            }
            fprintf(fp, "%s", vTractIdentifiers[k].c_str());
            if (k < (int)vTractIdentifiers.size() - 1)
              fprintf(fp, "%c ", PrintFormat.GetDeliminator());
            nCount++;
         }
         if (i < nLastTract)
           fprintf(fp, "%c ", PrintFormat.GetDeliminator());
       }
    }
    fprintf(fp, "\n");
  }
  catch (ZdException &x) {
    x.AddCallpath("DisplayCensusTractsInStep()","CCluster");
    throw;
  }
}

/** Writes clusters cartesian coordinates and ellipse properties (if cluster is elliptical)
    in format required by result output file. */
void CCluster::DisplayCoordinates(FILE* fp, const CSaTScanData& Data, const ClusterPrintFormat& PrintFormat) const {
  double      * pCoords = 0, * pCoords2 = 0;
  float         nRadius;
  int           i, j, count=0;

  try {
    Data.GetGInfo()->giGetCoords(m_Center, &pCoords);
    Data.GetTInfo()->tiGetCoords(Data.GetNeighbor(m_iEllipseOffset, m_Center, m_nTracts), &pCoords2);
    nRadius = (float)sqrt((Data.GetTInfo())->tiGetDistanceSq(pCoords, pCoords2));

    if (Data.GetParameters().GetDimensionsOfData() < 5) {
      //print coordinates differently for the circles and ellipses
      if (m_iEllipseOffset == 0)  {
        fprintf(fp, "%sCoordinates / radius..: (", PrintFormat.GetSpacesOnLeft());
      	for (i=0; i<(Data.GetParameters().GetDimensionsOfData())-1; i++)
      	   fprintf(fp, "%g,",pCoords[i]);
      	fprintf(fp, "%g) / %-5.2f\n",pCoords[(Data.GetParameters().GetDimensionsOfData())-1],nRadius);
        if (Data.GetParameters().GetNumRequestedEllipses()) {
          //print circle as ellipse with shape of '1' when analysis has ellipses
          fprintf(fp, "%sEllipse Parameters....:\n", PrintFormat.GetSpacesOnLeft());
          fprintf(fp, "%sAngle (degrees).......: n/a\n", PrintFormat.GetSpacesOnLeft());
          fprintf(fp, "%sShape.................: 1.0\n", PrintFormat.GetSpacesOnLeft());
        }
      }
      else {//print ellipse settings
        fprintf(fp, "%sCoordinates...........: (", PrintFormat.GetSpacesOnLeft());
        for (i=0; i<(Data.GetParameters().GetDimensionsOfData())-1; i++)
      	   fprintf(fp, "%g,",pCoords[i]);
        fprintf(fp, "%g)\n",pCoords[(Data.GetParameters().GetDimensionsOfData())-1]);
        fprintf(fp, "%sEllipse Semiminor axis: %-g\n", PrintFormat.GetSpacesOnLeft(), nRadius);
        fprintf(fp, "%sEllipse Parameters....:\n", PrintFormat.GetSpacesOnLeft());
        fprintf(fp, "%sAngle (degrees).......: %-g\n", PrintFormat.GetSpacesOnLeft(), ConvertAngleToDegrees(Data.GetAnglesArray()[m_iEllipseOffset-1]));
        fprintf(fp, "%sShape.................: %-g\n", PrintFormat.GetSpacesOnLeft(), Data.GetShapesArray()[m_iEllipseOffset-1]);
      }
    }
    else {/* More than four dimensions: need to wrap output */
      fprintf(fp, "%sCoordinates...........: (", PrintFormat.GetSpacesOnLeft());
      for (i=0; i<(Data.GetParameters().GetDimensionsOfData())-1; i++) {
         if (count < 4) { // This is a magic number: if 5 dimensions they all print on one line; if more, 4 per line
           fprintf(fp, "%g,",pCoords[i]);
    	   count++;
         }
         else { /*Start a new line */
           fprintf(fp,"\n");
           for (j=0; j < PrintFormat.GetLeftMargin() + 1; j++)
              fprintf(fp, " ");
           fprintf(fp, "%g,",pCoords[i]);
           count = 1;
         }
      }
      fprintf(fp, "%g)\n",pCoords[(Data.GetParameters().GetDimensionsOfData())-1]);
      if (m_iEllipseOffset == 0) {
        fprintf(fp, "%sRadius................: %-5.2f\n", PrintFormat.GetSpacesOnLeft(), nRadius);
        if (Data.GetParameters().GetNumRequestedEllipses()) {
          //print circle as ellipse with shape of '1' when analysis has ellipses
          fprintf(fp, "%sEllipse Parameters....:\n", PrintFormat.GetSpacesOnLeft());
          fprintf(fp, "%sAngle (degrees).......: n/a\n", PrintFormat.GetSpacesOnLeft());
          fprintf(fp, "%sShape.................: 1.0\n", PrintFormat.GetSpacesOnLeft());
        }
      }
      else {
        fprintf(fp, "%sEllipse Semiminor axis: %-g\n", PrintFormat.GetSpacesOnLeft(), nRadius);
        fprintf(fp, "%sEllipse Parameters....:\n", PrintFormat.GetSpacesOnLeft());
        fprintf(fp, "%sAngle (degrees).......: %-g\n", PrintFormat.GetSpacesOnLeft(), ConvertAngleToDegrees(Data.GetAnglesArray()[m_iEllipseOffset-1]));
        fprintf(fp, "%sShape.................: %-g\n", PrintFormat.GetSpacesOnLeft(), Data.GetShapesArray()[m_iEllipseOffset-1]);
      }
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
void CCluster::DisplayLatLongCoords(FILE* fp, const CSaTScanData& Data, const ClusterPrintFormat& PrintFormat) const {
  double *pCoords = 0, *pCoords2 = 0;
  float   Latitude, Longitude, nRadius;
  char    cNorthSouth, cEastWest;

  try {
    (Data.GetGInfo())->giGetCoords(m_Center, &pCoords);
    (Data.GetTInfo())->tiGetCoords(Data.GetNeighbor(0, m_Center, m_nTracts), &pCoords2);
    nRadius = (float)(sqrt((Data.GetTInfo())->tiGetDistanceSq(pCoords, pCoords2)));
    ConvertToLatLong(&Latitude, &Longitude, pCoords);
    Latitude >= 0 ? cNorthSouth = 'N' : cNorthSouth = 'S';
    Longitude >= 0 ? cEastWest = 'E' : cEastWest = 'W';
    // use to be .3f
    fprintf(fp, "%sCoordinates / radius..: (%.6f %c, %.6f %c) / %5.2f km\n",
            PrintFormat.GetSpacesOnLeft(), fabs(Latitude), cNorthSouth, fabs(Longitude), cEastWest, nRadius);
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

/** Writes clusters null occurance rate in format required by result output file. */
void CCluster::DisplayNullOccurrence(FILE* fp, const CSaTScanData& Data, unsigned int iNumSimulations, const ClusterPrintFormat& PrintFormat) const {
  float         fUnitsInOccurrence, fYears, fMonths, fDays, fIntervals, fAdjustedP_Value;

  try {
    if (Data.GetParameters().GetIsProspectiveAnalysis() && Data.GetParameters().GetNumReplicationsRequested() > 98) {
      fprintf(fp, "%sNull Occurrence.......: ", PrintFormat.GetSpacesOnLeft());
      fIntervals = Data.m_nTimeIntervals - Data.m_nProspectiveIntervalStart + 1;
      fAdjustedP_Value = 1 - pow(1 - GetPValue(iNumSimulations), 1/fIntervals);
      fUnitsInOccurrence = (float)Data.GetParameters().GetTimeIntervalLength()/fAdjustedP_Value;
      switch (Data.GetParameters().GetTimeIntervalUnitsType()) {
        case YEAR   : fprintf(fp, "Once in %.1f year%s\n", fUnitsInOccurrence, (fUnitsInOccurrence > 1 ? "s" : ""));
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
                        fprintf(fp, "Once in %.0f year%s\n", fYears, (fYears == 1 ? "" : "s"));
                      else if (fYears == 0)
                        fprintf(fp, "Once in %.0f month%s\n", fMonths, (fMonths < 1.5 ? "" : "s"));
                      else /*Having both zero month and year should never happen.*/
                        fprintf(fp, "Once in %.0f year%s and %0.f month%s\n", fYears, (fYears == 1 ? "" : "s"), fMonths, (fMonths < 1.5 ? "" : "s"));
                      break;
        case DAY    : fYears = floor(fUnitsInOccurrence/AVERAGE_DAYS_IN_YEAR);
                      fDays = fUnitsInOccurrence - (fYears * AVERAGE_DAYS_IN_YEAR);
                      //we don't want to print "Once in 5 years and 0 days", so round days for it
                      if (fDays < .5)
                        fDays = 0;
                      //Print correctly formatted statement.
                      if (fDays == 0)
                        fprintf(fp, "Once in %.0f year%s\n", fYears, (fYears == 1 ? "" : "s"));
                      else if (fYears == 0)
                        fprintf(fp, "Once in %.0f day%s\n", fDays, (fDays < 1.5 ? "" : "s"));
                      else /*Having both zero day and year should never happen.*/
                        fprintf(fp, "Once in %.0f year%s and %.0f day%s\n", fYears, (fYears == 1 ? "" : "s"), fDays, (fDays < 1.5 ? "" : "s"));
                      break;
        default     : ZdGenerateException("Invalid time interval index \"%d\" for prospective analysis.",
                                          "DisplayNullOccurrence()", Data.GetParameters().GetTimeIntervalUnitsType());
      }
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("DisplayNullOccurrence()","CCluster");
    throw;
  }
}

/** Writes clusters population in format required by result output file. */
void CCluster::DisplayPopulation(FILE* fp, const CSaTScanData& Data, const ClusterPrintFormat& PrintFormat) const {
  double        dPopulation;
  const char  * sFormat;

  dPopulation = Data.GetProbabilityModel().GetPopulation(m_iEllipseOffset, m_Center, m_nTracts, m_nFirstInterval, m_nLastInterval);
  if (dPopulation < .5)
    sFormat = "%sPopulation............: %-g\n"; // display all decimals for populations less than .5
  else if (dPopulation < 1)
    sFormat = "%sPopulation............: %-10.1f\n"; // display one decimal for populations less than 1
  else
    sFormat = "%sPopulation............: %-10.0f\n"; // else display no decimals

  fprintf(fp, sFormat, PrintFormat.GetSpacesOnLeft(), dPopulation);
}

/** Prints clusters p-value in format required by result output file. */
void CCluster::DisplayPValue(FILE* fp, unsigned int uiNumSimualtionsCompleted, const ClusterPrintFormat& PrintFormat) const {
  ZdString      sFormat, sReplicas;
  float         pVal = (float)GetPValue(uiNumSimualtionsCompleted);

  sReplicas << uiNumSimualtionsCompleted;
  sFormat.printf("%%.%df", sReplicas.GetLength());
  fprintf(fp, sFormat.GetCString(), pVal);
}

/** Writes clusters overall relative risk in format required by result output file. */
void CCluster::DisplayRelativeRisk(FILE* fp, double nMeasureAdjustment, const ClusterPrintFormat& PrintFormat) const {
  fprintf(fp, "%sOverall relative risk.: %.3f\n", PrintFormat.GetSpacesOnLeft(), GetRelativeRisk(nMeasureAdjustment));
}

/** Prints clusters time frame in format required by result output file. */
void CCluster::DisplayTimeFrame(FILE* fp, const CSaTScanData& DataHub, const ClusterPrintFormat& PrintFormat) const {
  ZdString  sStart, sEnd;

  fprintf(fp, "%sTime frame............: %s - %s\n", PrintFormat.GetSpacesOnLeft(),
          GetStartDate(sStart, DataHub).GetCString(), GetEndDate(sEnd, DataHub).GetCString());
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
const double CCluster::GetRelativeRisk(double nMeasureAdjustment) const {
  double        dRelativeRisk=0;

  if (GetMeasure(0) * nMeasureAdjustment)
    dRelativeRisk = ((double)GetCaseCount(0))/(GetMeasure(0) * nMeasureAdjustment);

  return dRelativeRisk;
}

/** Returns the relative risk for tract as defined by cluster. */
double CCluster::GetRelativeRiskForTract(tract_t tTract, const CSaTScanData& DataHub) const {
  double        dRelativeRisk=0;
  count_t       tCaseCount;
  measure_t     tMeasure;

  tCaseCount = GetCaseCountForTract(tTract, DataHub);
  tMeasure = GetMeasureForTract(tTract, DataHub);

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

/** Sets Duczmal Compactness Correction for shape. */
void CCluster::SetDuczmalCorrection(double dEllipseShape) {
  m_DuczmalCorrection = GetDuczmalCorrection(dEllipseShape);
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

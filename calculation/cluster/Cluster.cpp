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
  m_DuczmalCorrection = 1;
  m_nFirstInterval = 0;
  m_nLastInterval  = 0;
  m_nStartDate     = 0;
  m_nEndDate       = 0;
  m_iEllipseOffset = 0;         // use to be -1, but bombed when R = 1
  gfPValue = 0.0;
  gpAreaData = 0;
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
  m_nStartDate          = rhs.m_nStartDate;
  m_nEndDate            = rhs.m_nEndDate;
  m_iEllipseOffset      = rhs.m_iEllipseOffset;

  return *this;
}

const double CCluster::ConvertAngleToDegrees(double dAngle) const
{
   double dDegrees;
   
   dDegrees = 180.00 * (dAngle / (double)M_PI);
   if (dDegrees > 90.00)
     dDegrees -= 180.00;

   return dDegrees;
}

void CCluster::Display(FILE*     fp,
                       const     CParameters& Parameters,
                       const     CSaTScanData& Data,
                       int       nCluster,
                       measure_t nMinMeasure, int iNumSimulations)
{
   int    nLeftMargin = 26;
   int    nRightMargin = 67;
   char   cDeliminator = ',';
   char   szSpacesOnLeft [100];

   try
      {
      fprintf(fp, "%i.", nCluster);

      // Adjust for spacing of cluster number
      strcpy(szSpacesOnLeft, "  ");
      int n = (int)floor(((double)nCluster)/10);
      while (n > 0)
        {
        strcat(szSpacesOnLeft, " ");
        ++nLeftMargin;
        n = (int)floor(((double)n)/10);
        }

      DisplaySteps(fp, szSpacesOnLeft);
      //  fprintf(fp, "Census areas included");
      fprintf(fp, "Location IDs ");
      DisplayCensusTracts(fp, Data, -1, nMinMeasure,
                          iNumSimulations, 0, false, false,
                          nLeftMargin, nRightMargin, cDeliminator, szSpacesOnLeft);
    
      if (Parameters.GetCoordinatesType() == CARTESIAN)
      	DisplayCoordinates(fp, Data, nLeftMargin, nRightMargin, cDeliminator, szSpacesOnLeft);
      else
      	DisplayLatLongCoords(fp, Data, nLeftMargin, nRightMargin, cDeliminator, szSpacesOnLeft);

      DisplayTimeFrame(fp, szSpacesOnLeft, Parameters.GetAnalysisType());
      if (Parameters.GetProbabiltyModelType() == POISSON || Parameters.GetProbabiltyModelType() == BERNOULLI)
        DisplayPopulation(fp, Data, szSpacesOnLeft);

      fprintf(fp, "%sNumber of cases.......: %ld", szSpacesOnLeft, GetCaseCount(0));
      fprintf(fp, "          (%.2f expected)\n", Data.GetMeasureAdjustment() * GetMeasure(0));
    
      if (Parameters.GetProbabiltyModelType() == POISSON)
        fprintf(fp, "%sAnnual cases / %.0f.: %.1f\n",
                     szSpacesOnLeft, Data.GetAnnualRatePop(),
                     Data.GetAnnualRateAtStart()*GetRelativeRisk(Data.GetMeasureAdjustment()));

      DisplayRelativeRisk(fp, Data.GetMeasureAdjustment(), nLeftMargin, nRightMargin, cDeliminator, szSpacesOnLeft);

      //Print Loglikelihood/Test Statistic
      if (Parameters.GetProbabiltyModelType() == SPACETIMEPERMUTATION)
        fprintf(fp, "%sTest statistic........: %f\n", szSpacesOnLeft, m_nRatio);
      else {
        fprintf(fp, "%sLog likelihood ratio..: %f\n", szSpacesOnLeft, m_nRatio/m_DuczmalCorrection);
        if (Parameters.GetDuczmalCorrectEllipses())
          fprintf(fp, "%sTest statistic........: %f\n", szSpacesOnLeft, m_nRatio);
      }

      if (iNumSimulations)
        fprintf(fp, "%sMonte Carlo rank......: %ld/%ld\n", szSpacesOnLeft, m_nRank, iNumSimulations+1);

      if (iNumSimulations > 98)
        {
        fprintf(fp, "%sP-value...............: ", szSpacesOnLeft);
        DisplayPVal(fp, iNumSimulations, szSpacesOnLeft);
        fprintf(fp, "\n");
        }
      DisplayNullOccurrence(fp, Data, iNumSimulations, szSpacesOnLeft);
      DisplayTimeTrend(fp, szSpacesOnLeft);
      }
   catch (ZdException & x)
      {
      x.AddCallpath("Display()", "CCluster");
      throw;
      }
}

void CCluster::DisplayCensusTracts(FILE* fp, const CSaTScanData& Data,
                                   int nCluster, measure_t nMinMeasure, int iNumSimulations,
                                   long lReportHistoryRunNumber,
                                   bool bIncludeRelRisk, bool bIncludePVal, int nLeftMargin, int nRightMargin,
                                   char cDeliminator, char* szSpacesOnLeft, bool bFormat)
{
   try {
      if (nLeftMargin > 0 && fp != NULL)
         fprintf(fp, "included.: ");

       DisplayCensusTractsInStep(fp, Data, 1, m_nTracts, nCluster, nMinMeasure,
                            iNumSimulations, lReportHistoryRunNumber, bIncludeRelRisk, bIncludePVal,
                            nLeftMargin, nRightMargin, cDeliminator, szSpacesOnLeft, bFormat);
   }
   catch (ZdException & x) {
      x.AddCallpath("DisplayCensusTracts()", "CCluster");
      throw;
   }
}

void CCluster::DisplayCensusTractsInStep(FILE* fp, const CSaTScanData& Data,
                                         tract_t nFirstTract, tract_t nLastTract,
                                         int nCluster, measure_t nMinMeasure, int iNumSimulations,
                                         long lReportHistoryRunNumber, bool bIncludeRelRisk,
                                         bool bIncludePVal, int nLeftMargin, int nRightMargin,
                                         char cDeliminator, char* szSpacesOnLeft, bool bFormat)
{
  int                                  pos  = nLeftMargin, nCount=0;
  tract_t                              tTract;
  std::vector<std::string>             vTractIdentifiers;
  measure_t                         ** ppMeasure(Data.GetDataStreamHandler().GetStream(0/*for now*/).GetMeasureArray());   

  try {
    for (int i = nFirstTract; i <= nLastTract; i++) {
       //if (ppMeasure[0][Data.GetNeighbor(0, m_Center, i)]>nMinMeasure)       // access over-run here
       //if (ppMeasure[m_iEllipseOffset][Data.GetNeighbor(m_iEllipseOffset, m_Center, i)]>nMinMeasure) // access over-run here

       // the first dimension of ppMeasure is Time Interval !!!
       tTract = Data.GetNeighbor(m_iEllipseOffset, m_Center, i);
       if (ppMeasure[0][tTract] > nMinMeasure) {
         Data.GetTInfo()->tiGetTractIdentifiers(tTract, vTractIdentifiers);
         if (fp != NULL) {
            for (int k=0; k < (int)vTractIdentifiers.size(); k++) {
               pos += strlen(vTractIdentifiers[k].c_str()) + 2;
               if (nCount>1 && pos>nRightMargin) {   // strange and sad but true print formatting
                 pos = nLeftMargin + strlen(vTractIdentifiers[k].c_str()) + 2;
                 fprintf(fp, "\n");
                 for (int j=0; j<nLeftMargin; ++j)
                    fprintf(fp, " ");
               }
               if (bFormat)
                 fprintf(fp, "%s", vTractIdentifiers[k].c_str());
               else
                fprintf(fp, "%-29s", vTractIdentifiers[k].c_str());
               if (k < (int)vTractIdentifiers.size() - 1)
                 fprintf(fp, "%c ", cDeliminator);
               nCount++;
            }
#ifdef INCLUDE_RUN_HISTORY
            // run history number
            if (bIncludeRelRisk)
              fprintf(fp, "%-12d", lReportHistoryRunNumber);
#endif
                // tract name
//                if (bFormat)
//                   fprintf(fp, "%s", tid);
//                else
//                   fprintf(fp, "%-29s", tid);

            // cluster number
            if (nCluster > -1)
              fprintf(fp, "%i         ", nCluster);

            // relative risk
            if (bIncludeRelRisk) {
              fprintf(fp, "  %i", GetCaseCount(0));      // cluster level Observed
              fprintf(fp, "   %-12.2f", GetMeasure(0));    // cluster level expected
              fprintf(fp, "   %-12.3f", GetRelativeRisk(Data.GetMeasureAdjustment()));  // cluster level rel risk
            }
            if (bIncludePVal) {    // this is only displayed if Reps > 98
              fprintf(fp, "     ");
              DisplayPVal(fp, iNumSimulations, szSpacesOnLeft);
            }
            if (bIncludeRelRisk) {  // if we include the cluster rel risk, then we also include obs, exp, and rel_risk as well
              fprintf(fp, "\t %12i", GetCaseCountForTract(tTract, Data));      // area level obeserved clusters
              fprintf(fp, "\t %12.3f", GetMeasureForTract(tTract, Data));      // area level expected clusters
              fprintf(fp, "\t %12.3f", GetRelativeRiskForTract(tTract, Data));   // area level relative risk
              DisplayAnnualTimeTrendWithoutTitle(fp);
            }
            if (i < nLastTract)
              fprintf(fp, "%c ", cDeliminator);
         }     // end if fp
       }

       // record DBF output data - AJV
       if(gpAreaData)
          gpAreaData->RecordClusterData(*this, Data, nCluster, tTract, iNumSimulations);
    }

    if (fp != NULL)
      fprintf(fp, "\n");
  }
  catch (ZdException & x) {
    x.AddCallpath("DisplayCensusTractsInStep()", "CCluster");
    throw;
  }
}

void CCluster::DisplayCoordinates(FILE* fp, const CSaTScanData& Data, int nLeftMargin,
                                  int nRightMargin, char cDeliminator, char* szSpacesOnLeft) {
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
        fprintf(fp, "%sCoordinates / radius..: (", szSpacesOnLeft);
      	for (i=0; i<(Data.GetParameters().GetDimensionsOfData())-1; i++)
      	   fprintf(fp, "%g,",pCoords[i]);
      	fprintf(fp, "%g) / %-5.2f\n",pCoords[(Data.GetParameters().GetDimensionsOfData())-1],nRadius);
        if (Data.GetParameters().GetNumRequestedEllipses()) {
          //print circle as ellipse with shape of '1' when analysis has ellipses
          fprintf(fp, "%sEllipse Parameters....:\n", szSpacesOnLeft);
          fprintf(fp, "%sAngle (degrees).......: n/a\n", szSpacesOnLeft);
          fprintf(fp, "%sShape.................: 1.0\n", szSpacesOnLeft);
        }
      }
      else {//print ellipse settings
        fprintf(fp, "%sCoordinates...........: (", szSpacesOnLeft);
        for (i=0; i<(Data.GetParameters().GetDimensionsOfData())-1; i++)
      	   fprintf(fp, "%g,",pCoords[i]);
        fprintf(fp, "%g)\n",pCoords[(Data.GetParameters().GetDimensionsOfData())-1]);
        fprintf(fp, "%sEllipse Semiminor axis: %-g\n", szSpacesOnLeft, nRadius);
        fprintf(fp, "%sEllipse Parameters....:\n", szSpacesOnLeft);
        fprintf(fp, "%sAngle (degrees).......: %-g\n", szSpacesOnLeft, ConvertAngleToDegrees(Data.GetAnglesArray()[m_iEllipseOffset-1]));
        fprintf(fp, "%sShape.................: %-g\n", szSpacesOnLeft, Data.GetShapesArray()[m_iEllipseOffset-1]);
      }
    }
    else {/* More than four dimensions: need to wrap output */
      fprintf(fp, "%sCoordinates...........: (", szSpacesOnLeft);
      for (i=0; i<(Data.GetParameters().GetDimensionsOfData())-1; i++) {
         if (count < 4) { // This is a magic number: if 5 dimensions they all print on one line; if more, 4 per line
           fprintf(fp, "%g,",pCoords[i]);
    	   count++;
         }
         else { /*Start a new line */
           fprintf(fp,"\n");
           for (j=0; j < nLeftMargin+1; j++)
              fprintf(fp, " ");
           fprintf(fp, "%g,",pCoords[i]);
           count = 1;
         }
      }
      fprintf(fp, "%g)\n",pCoords[(Data.GetParameters().GetDimensionsOfData())-1]);
      if (m_iEllipseOffset == 0) {
        fprintf(fp, "%sRadius................: %-5.2f\n", szSpacesOnLeft, nRadius);
        if (Data.GetParameters().GetNumRequestedEllipses()) {
          //print circle as ellipse with shape of '1' when analysis has ellipses
          fprintf(fp, "%sEllipse Parameters....:\n", szSpacesOnLeft);
          fprintf(fp, "%sAngle (degrees).......: n/a\n", szSpacesOnLeft);
          fprintf(fp, "%sShape.................: 1.0\n", szSpacesOnLeft);
        }
      }
      else {
        fprintf(fp, "%sEllipse Semiminor axis: %-g\n", szSpacesOnLeft, nRadius);
        fprintf(fp, "%sEllipse Parameters....:\n", szSpacesOnLeft);
        fprintf(fp, "%sAngle (degrees).......: %-g\n", szSpacesOnLeft, ConvertAngleToDegrees(Data.GetAnglesArray()[m_iEllipseOffset-1]));
        fprintf(fp, "%sShape.................: %-g\n", szSpacesOnLeft, Data.GetShapesArray()[m_iEllipseOffset-1]);
      }
    }

    free(pCoords);
    free(pCoords2);
  }
  catch (ZdException & x) {
    free(pCoords);
    free(pCoords2);
    x.AddCallpath("DisplayCoordinates()", "CCluster");
    throw;
  }
}

//**********************************************************************
//  Since Ellipsoid option can NOT be run under the Lat/Long do not
//  need to update this function at this time...
//
//**********************************************************************
void CCluster::DisplayLatLongCoords(FILE* fp, const CSaTScanData& Data,
                                  int nLeftMargin, int nRightMargin,
                                  char cDeliminator, char* szSpacesOnLeft)
{
   double *pCoords = 0, *pCoords2 = 0;
   float   Latitude, Longitude, nRadius;
   char    cNorthSouth, cEastWest;

   try
      {
      (Data.GetGInfo())->giGetCoords(m_Center, &pCoords);
      (Data.GetTInfo())->tiGetCoords(Data.GetNeighbor(0, m_Center, m_nTracts), &pCoords2);

      nRadius = (float)(sqrt((Data.GetTInfo())->tiGetDistanceSq(pCoords, pCoords2)));

      ConvertToLatLong(&Latitude, &Longitude, pCoords);

      Latitude >= 0 ? cNorthSouth = 'N' : cNorthSouth = 'S';
      Longitude >= 0 ? cEastWest = 'E' : cEastWest = 'W';

    //  fprintf(fp, "  Coordinates / radius..........: (%.3f %c, %.3f %c) / %5.2f\n",
    //               fabs(Latitude), cNorthSouth, fabs(Longitude), cEastWest, nRadius);

      // use to be .3f
      fprintf(fp, "%sCoordinates / radius..: (%.6f %c, %.6f %c) / %5.2f km\n",
                   szSpacesOnLeft, fabs(Latitude), cNorthSouth, fabs(Longitude), cEastWest, nRadius);

      free(pCoords);
      free(pCoords2);
      }
   catch (ZdException & x)
      {
      free(pCoords);
      free(pCoords2);
      x.AddCallpath("DisplayLatLongCoords()", "CCluster");
      throw;
      }
}

/** Prints null occurrence rate for cluster given time interval units. */
void CCluster::DisplayNullOccurrence(FILE* fp, const CSaTScanData& Data, int iNumSimulations, char* szSpacesOnLeft) {
  float         fUnitsInOccurrence, fYears, fMonths, fDays, fIntervals, fAdjustedP_Value;

  try {
    if (Data.GetParameters().GetIsProspectiveAnalysis() && Data.GetParameters().GetNumReplicationsRequested() > 98) {
      fprintf(fp, "%sNull Occurrence.......: ", szSpacesOnLeft);
      fIntervals = Data.m_nTimeIntervals - Data.m_nProspectiveIntervalStart + 1;
      fAdjustedP_Value = 1 - pow(1 - GetPVal(iNumSimulations), 1/fIntervals);
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

void CCluster::DisplayPopulation(FILE* fp, const CSaTScanData& Data, char* szSpacesOnLeft) {
  double        dPopulation;
  const char  * sFormat;

  dPopulation = Data.GetProbabilityModel().GetPopulation(m_iEllipseOffset, m_Center, m_nTracts, m_nFirstInterval, m_nLastInterval);
  if (dPopulation < .5)
    sFormat = "%sPopulation............: %-g\n"; // display all decimals for populations less than .5
  else if (dPopulation < 1)
    sFormat = "%sPopulation............: %-10.1f\n"; // display one decimal for populations less than 1
  else
    sFormat = "%sPopulation............: %-10.0f\n"; // else display no decimals

  fprintf(fp, sFormat, szSpacesOnLeft, dPopulation);
}

void CCluster::DisplayPVal(FILE* fp, int nReplicas, char* szSpacesOnLeft) {
  ZdString      sFormat, sReplicas;
  float         pVal = (float)GetPVal(nReplicas);

  sReplicas = nReplicas;
  sFormat.printf("%%.%df", sReplicas.GetLength());
  gfPValue = pVal;
  fprintf(fp, sFormat.GetCString(), pVal);
}

void CCluster::DisplayRelativeRisk(FILE* fp, double nMeasureAdjustment, int nLeftMargin,
                                   int nRightMargin, char cDeliminator, char* szSpacesOnLeft) {
  //  fprintf(fp, "          (Relative risk: %.2f)\n", GetRelativeRisk());
  fprintf(fp, "%sOverall relative risk.: %.3f\n", szSpacesOnLeft, GetRelativeRisk(nMeasureAdjustment));
}

void CCluster::DisplayTimeFrame(FILE* fp, char* szSpacesOnLeft, int nAnalysisType) {
  char szStartDt[MAX_DT_STR];
  char szEndDt[MAX_DT_STR];

  fprintf(fp, "%sTime frame............: %s - %s\n", szSpacesOnLeft,
          JulianToChar(szStartDt, m_nStartDate), JulianToChar(szEndDt, m_nEndDate));
}

const double CCluster::GetRelativeRisk(double nMeasureAdjustment) const
{
  double        dRelativeRisk=0;

  if (GetMeasure(0) * nMeasureAdjustment)
    dRelativeRisk = ((double)GetCaseCount(0))/(GetMeasure(0) * nMeasureAdjustment);

  return dRelativeRisk;
}

/** Returns the relative risk for tract as defined by cluster. */
double CCluster::GetRelativeRiskForTract(tract_t tTract, const CSaTScanData & Data) const
{
  double        dRelativeRisk=0;
  count_t       tCaseCount;
  measure_t     tMeasure;

  tCaseCount = GetCaseCountForTract(tTract, Data);
  tMeasure = GetMeasureForTract(tTract, Data);

  if (tMeasure)
    dRelativeRisk = ((double)(tCaseCount))/tMeasure;
  return dRelativeRisk;
}

void CCluster::SetCenter(tract_t nCenter)
{
  m_Center = nCenter;
}

void CCluster::SetEllipseOffset(int iOffset)
{
   m_iEllipseOffset = iOffset;
}

/** Sets Duczmal Compactness Correction */
void CCluster::SetDuczmalCorrection(double dEllipseShape) {
  m_DuczmalCorrection = GetDuczmalCorrection(dEllipseShape);
}

void CCluster::SetRate(int nRate)
{
  switch (nRate)
  {
    case HIGH       : m_pfRateOfInterest = HighRate;      break;
    case LOW        : m_pfRateOfInterest = LowRate;       break;
    case HIGHANDLOW : m_pfRateOfInterest = HighOrLowRate; break;
    default         : ;
  }
}

void CCluster::SetStartAndEndDates(const Julian* pIntervalStartTimes, int nTimeIntervals)
{
  m_nStartDate = pIntervalStartTimes[m_nFirstInterval];
  m_nEndDate   = pIntervalStartTimes[m_nLastInterval]-1;
}

void CCluster::WriteCoordinates(FILE* fp, CSaTScanData* pData)
{
   double *pCoords = 0, *pCoords2 = 0;
   float nRadius;
   int i;

   try
      {
      (pData->GetGInfo())->giGetCoords(m_Center, &pCoords);
      //tiGetCoords(Data.GetNeighbor(0, m_Center, m_nTracts), &pCoords2);       DTG
      (pData->GetTInfo())->tiGetCoords(pData->GetNeighbor(m_iEllipseOffset, m_Center, m_nTracts), &pCoords2);

      nRadius = (float)sqrt((pData->GetTInfo())->tiGetDistanceSq(pCoords, pCoords2));

      if ( m_iEllipseOffset == 0 )
         {
         if (GetClusterType() == PURELYTEMPORAL)
            {
            for (i=0; i<=(pData->GetParameters().GetDimensionsOfData())-1; i++)
               fprintf(fp, " %14s","n/a");
            fprintf(fp, " %12s ","n/a");
            }
         else
            {
            for (i=0; i<=(pData->GetParameters().GetDimensionsOfData())-1; i++)
               fprintf(fp, " %14.6g",pCoords[i]);
            fprintf(fp, " %12.2f",nRadius);
            }

         //these are not applicable for circles....
         //SHAPE, ANGLE
         if (pData->GetParameters().GetNumRequestedEllipses() > 0)
            {
            if (GetClusterType() == PURELYTEMPORAL)
               fprintf(fp, " %8s %8s", "n/a", "n/a");
            else
               fprintf(fp, " %8.3f %8.3f", 1.0, 0.0);
            }
         }
      else
         {
         if (GetClusterType() == PURELYTEMPORAL)
            {
            for (i=0; i<=(pData->GetParameters().GetDimensionsOfData())-1; i++)
               fprintf(fp, " %14s", "n/a");
            fprintf(fp, " %12s","n/a");

            fprintf(fp, " %8s %8s", "n/a", "n/a");
            }
         else
            {
            for (i=0; i<=(pData->GetParameters().GetDimensionsOfData())-1; i++)
               fprintf(fp, " %14.6g",pCoords[i]);

            //just print the nRadius value here...
            //IT IS NOT RADIUS
            //IT IS SEMI-MINOR AXIS.....  FOR AN ELLIPSOID
            fprintf(fp, " %12.2f",nRadius);

            //PRINT SHAPE AND ANGLE
            fprintf(fp, " %8.3f", pData->GetShapesArray()[m_iEllipseOffset-1]);
            fprintf(fp, " %8.3f", ConvertAngleToDegrees(pData->GetAnglesArray()[m_iEllipseOffset-1]));
            }
         }

      // If Space-Time analysis...  put Start and End of Cluster
     /* if (pData->m_pParameters->GetAnalysisType() != PURELYSPATIAL)
         {
         JulianToChar(sStartDate, m_nStartDate);
         JulianToChar(sEndDate, m_nEndDate);
         fprintf(fp, " %11s %11s", sStartDate, sEndDate);
         }
      else //purely spacial - print study begin and end dates
         fprintf(fp, " %11s %11s", pData->m_pParameters->GetStudyPeriodStartDate().c_str(), pData->m_pParameters->GetStudyPeriodEndDate().c_str());
       */
      free(pCoords);
      free(pCoords2);
      }
   catch (ZdException & x)
      {
      free(pCoords);
      free(pCoords2);
      x.AddCallpath("WriteCoordinates()", "CCluster");
      throw;
      }
}

//**********************************************************************
//  Since Ellipsoid option can NOT be run under the Lat/Long do not
//  need to update this function at this time...
//
//**********************************************************************
void CCluster::WriteLatLongCoords(FILE* fp, CSaTScanData* pData)
{
   double *pCoords = 0, *pCoords2 = 0;
   float  Latitude, Longitude, nRadius;
   //char  cNorthSouth, cEastWest;

   try
      {
      (pData->GetGInfo())->giGetCoords(m_Center, &pCoords);
      (pData->GetTInfo())->tiGetCoords(pData->GetNeighbor(0, m_Center, m_nTracts), &pCoords2);

      nRadius = (float)sqrt((pData->GetTInfo())->tiGetDistanceSq(pCoords, pCoords2));

      ConvertToLatLong(&Latitude, &Longitude, pCoords);

     // Latitude >= 0 ? cNorthSouth = 'N' : cNorthSouth = 'S';
     // Longitude >= 0 ? cEastWest = 'E' : cEastWest = 'W';

      fprintf(fp, " %f %f %5.2f", Latitude, Longitude, nRadius);

      // use to be .3f
      //fprintf(fp, " %.6f %c, %.6f %c) / %5.2f\n",
      //             fabs(Latitude), cNorthSouth, fabs(Longitude), cEastWest, nRadius);

      free(pCoords);
      free(pCoords2);
      }
   catch (ZdException & x)
      {
      free(pCoords);
      free(pCoords2);
      x.AddCallpath("WriteLatLongCoords()", "CCluster");
      throw;
      }
}

#include "SaTScan.h"
#pragma hdrstop

#include "Cluster.h"
#include "stsAreaSpecificData.h"

/** constructor */
CCluster::CCluster(BasePrint *pPrintDirection) {
  Initialize();
  gpPrintDirection = pPrintDirection;
}

/** destructor */
CCluster::~CCluster() {}

/** returns newly cloned CCluster */
CCluster * CCluster::Clone() const {
  //Note: Replace this code with copy constructor...
  CCluster * pClone = new CCluster(gpPrintDirection);
  *pClone = *this;
  return pClone;
}

void CCluster::Initialize(tract_t nCenter)
{
  m_Center         = nCenter;

  m_nCases         = 0;
  m_nMeasure       = 0;
  m_nTracts        = 0;
  m_nRatio         = -DBL_MAX;//0;
  m_nLogLikelihood = 0;
  m_nRank          = 1;
  m_DuczmalCorrection = 1;

  m_nFirstInterval = 0;
  m_nLastInterval  = 0;
  m_nStartDate     = 0;
  m_nEndDate       = 0;

  m_nSteps       = 0;

  m_bClusterInit   = true;
  m_bClusterSet    = false;
  m_bClusterDefined= false;
  m_bLogLSet       = false;
  m_bRatioSet      = false;
  m_nClusterType   = 0;
  m_iEllipseOffset = 0;         // use to be -1, but bombed when R = 1

  gfPValue = 0.0;
  gpAreaData = 0;
}


CCluster& CCluster::operator =(const CCluster& cluster)
{
  m_Center         = cluster.m_Center;

  m_nCases         = cluster.m_nCases ;
  m_nMeasure       = cluster.m_nMeasure;
  m_nTracts        = cluster.m_nTracts;
  m_nRatio         = cluster.m_nRatio;
  m_nLogLikelihood = cluster.m_nLogLikelihood;
  m_nRank          = cluster.m_nRank;
  m_DuczmalCorrection = cluster.m_DuczmalCorrection;

  m_nFirstInterval = cluster.m_nFirstInterval;
  m_nLastInterval  = cluster.m_nLastInterval;
  m_nStartDate     = cluster.m_nStartDate;
  m_nEndDate       = cluster.m_nEndDate;

  m_nSteps       = cluster.m_nSteps;

  m_bClusterInit   = cluster.m_bClusterInit;
  m_bClusterSet    = cluster.m_bClusterSet;
  m_bClusterDefined= cluster.m_bClusterDefined;
  m_bLogLSet       = cluster.m_bLogLSet;
  m_bRatioSet      = cluster.m_bRatioSet;
  m_nClusterType   = cluster.m_nClusterType;
  m_iEllipseOffset = cluster.m_iEllipseOffset;

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
                       measure_t nMinMeasure)
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
      fprintf(fp, "Census areas ");
      DisplayCensusTracts(fp, Data, -1, nMinMeasure,
                          Parameters.GetNumReplicationsRequested(), 0, false, false,
                          nLeftMargin, nRightMargin, cDeliminator, szSpacesOnLeft);
    
      if (Parameters.GetCoordinatesType() == CARTESIAN)
      	DisplayCoordinates(fp, Data, nLeftMargin, nRightMargin, cDeliminator, szSpacesOnLeft);
      else
      	DisplayLatLongCoords(fp, Data, nLeftMargin, nRightMargin, cDeliminator, szSpacesOnLeft);

      DisplayTimeFrame(fp, szSpacesOnLeft, Parameters.GetAnalysisType());
      if (Parameters.GetProbabiltyModelType() != SPACETIMEPERMUTATION)
        DisplayPopulation(fp, Data, szSpacesOnLeft);

      fprintf(fp, "%sNumber of cases.......: %ld", szSpacesOnLeft, m_nCases);
      fprintf(fp, "          (%.2f expected)\n", Data.GetMeasureAdjustment()*m_nMeasure);
    
      if (Parameters.GetProbabiltyModelType() == POISSON)
        fprintf(fp, "%sAnnual cases / %.0f.: %.1f\n",
                     szSpacesOnLeft, Data.GetAnnualRatePop(),
                     Data.GetAnnualRateAtStart()*GetRelativeRisk(Data.GetMeasureAdjustment()));

      DisplayRelativeRisk(fp, Data.GetMeasureAdjustment(), nLeftMargin, nRightMargin, cDeliminator, szSpacesOnLeft);

      //Print Loglikelihood/Test Statistic
      if (Parameters.GetProbabiltyModelType() == SPACETIMEPERMUTATION) {
        if (m_iEllipseOffset != 0 /*i.e. is ellipse*/ && Parameters.GetDuczmalCorrectEllipses())
          fprintf(fp, "%sTest statistic........: %f\n", szSpacesOnLeft, GetDuczmalCorrectedLogLikelihoodRatio());
        else
          fprintf(fp, "%sTest statistic........: %f\n", szSpacesOnLeft, m_nRatio);
      }
      else {
        fprintf(fp, "%sLog likelihood ratio..: %f\n", szSpacesOnLeft, m_nRatio);
        if (Parameters.GetDuczmalCorrectEllipses())
          fprintf(fp, "%sTest statistic........: %f\n", szSpacesOnLeft, GetDuczmalCorrectedLogLikelihoodRatio());
      }

      if (Parameters.GetNumReplicationsRequested())
        fprintf(fp, "%sMonte Carlo rank......: %ld/%ld\n", szSpacesOnLeft, m_nRank, Parameters.GetNumReplicationsRequested()+1);

      if (Parameters.GetNumReplicationsRequested() > 99)
        {
        fprintf(fp, "%sP-value...............: ", szSpacesOnLeft);
        DisplayPVal(fp, Parameters.GetNumReplicationsRequested(), szSpacesOnLeft);
        fprintf(fp, "\n");
        }
      DisplayNullOccurrence(fp, Data, szSpacesOnLeft);
      }
   catch (ZdException & x)
      {
      x.AddCallpath("Display()", "CCluster");
      throw;
      }
}

void CCluster::DisplayCensusTracts(FILE* fp, const CSaTScanData& Data,
                                   int nCluster, measure_t nMinMeasure, int nReplicas,
                                   long lReportHistoryRunNumber,
                                   bool bIncludeRelRisk, bool bIncludePVal, int nLeftMargin, int nRightMargin,
                                   char cDeliminator, char* szSpacesOnLeft, bool bFormat)
{
   try {
      if (nLeftMargin > 0 && fp != NULL)
         fprintf(fp, "included.: ");

       DisplayCensusTractsInStep(fp, Data, 1, m_nTracts, nCluster, nMinMeasure,
                            nReplicas, lReportHistoryRunNumber, bIncludeRelRisk, bIncludePVal,
                            nLeftMargin, nRightMargin, cDeliminator, szSpacesOnLeft, bFormat);
   }
   catch (ZdException & x) {
      x.AddCallpath("DisplayCensusTracts()", "CCluster");
      throw;
   }
}

void CCluster::DisplayCensusTractsInStep(FILE* fp, const CSaTScanData& Data,
                                         tract_t nFirstTract, tract_t nLastTract,
                                         int nCluster, measure_t nMinMeasure, int nReplicas, long lReportHistoryRunNumber, bool bIncludeRelRisk,
                                         bool bIncludePVal, int nLeftMargin, int nRightMargin, char cDeliminator,
                                         char* szSpacesOnLeft, bool bFormat)
{
  int                                  pos  = nLeftMargin, nCount=0;
  tract_t                              tTract;
  std::vector<std::string>             vTractIdentifiers;

  try {
    for (int i = nFirstTract; i <= nLastTract; i++) {
       //if (Data.m_pMeasure[0][Data.GetNeighbor(0, m_Center, i)]>nMinMeasure)       // access over-run here
       //if (Data.m_pMeasure[m_iEllipseOffset][Data.GetNeighbor(m_iEllipseOffset, m_Center, i)]>nMinMeasure) // access over-run here

       // the first dimension of m_pMeasure is Time Interval !!!
       tTract = Data.GetNeighbor(m_iEllipseOffset, m_Center, i);
       if (Data.m_pMeasure[0][tTract] > nMinMeasure) {
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
              fprintf(fp, "  %i", m_nCases);      // cluster level Observed
              fprintf(fp, "   %-12.2f", m_nMeasure);    // cluster level expected
              fprintf(fp, "   %-12.3f", GetRelativeRisk(Data.GetMeasureAdjustment()));  // cluster level rel risk
            }
            if (bIncludePVal) {    // this is only displayed if Reps > 99
              fprintf(fp, "     ");
              DisplayPVal(fp, nReplicas, szSpacesOnLeft);
            }
            if (bIncludeRelRisk) {  // if we include the cluster rel risk, then we also include obs, exp, and rel_risk as well
              fprintf(fp, "\t %12i", GetCaseCountForTract(tTract, Data));      // area level obeserved clusters
              fprintf(fp, "\t %12.3f", GetMeasureForTract(tTract, Data));      // area level expected clusters
              fprintf(fp, "\t %12.3f", GetRelativeRiskForTract(tTract, Data));   // area level relative risk
            }
            if (i < nLastTract)
              fprintf(fp, "%c ", cDeliminator);
         }     // end if fp
       }

       // record DBF output data - AJV
       if(gpAreaData)
          gpAreaData->RecordClusterData(*this, Data, nCluster, tTract);
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

    if (Data.m_pParameters->GetDimensionsOfData() < 5) {
      //print coordinates differently for the circles and ellipses
      if (m_iEllipseOffset == 0)  {
        fprintf(fp, "%sCoordinates / radius..: (", szSpacesOnLeft);
      	for (i=0; i<(Data.m_pParameters->GetDimensionsOfData())-1; i++)
      	   fprintf(fp, "%g,",pCoords[i]);
      	fprintf(fp, "%g) / %-5.2f\n",pCoords[(Data.m_pParameters->GetDimensionsOfData())-1],nRadius);
        if (Data.m_pParameters->GetNumRequestedEllipses()) {
          //print circle as ellipse with shape of '1' when analysis has ellipses
          fprintf(fp, "%sEllipse Parameters....:\n", szSpacesOnLeft);
          fprintf(fp, "%sAngle (degrees).......: n/a\n", szSpacesOnLeft);
          fprintf(fp, "%sShape.................: 1.0\n", szSpacesOnLeft);
        }
      }
      else {//print ellipse settings
        fprintf(fp, "%sCoordinates...........: (", szSpacesOnLeft);
        for (i=0; i<(Data.m_pParameters->GetDimensionsOfData())-1; i++)
      	   fprintf(fp, "%g,",pCoords[i]);
        fprintf(fp, "%g)\n",pCoords[(Data.m_pParameters->GetDimensionsOfData())-1]);
        fprintf(fp, "%sEllipse Semiminor axis: %-g\n", szSpacesOnLeft, nRadius);
        fprintf(fp, "%sEllipse Parameters....:\n", szSpacesOnLeft);
        fprintf(fp, "%sAngle (degrees).......: %-g\n", szSpacesOnLeft, ConvertAngleToDegrees(Data.mdE_Angles[m_iEllipseOffset-1]));
        fprintf(fp, "%sShape.................: %-g\n", szSpacesOnLeft, Data.mdE_Shapes[m_iEllipseOffset-1]);
      }
    }
    else {/* More than four dimensions: need to wrap output */
      fprintf(fp, "%sCoordinates...........: (", szSpacesOnLeft);
      for (i=0; i<(Data.m_pParameters->GetDimensionsOfData())-1; i++) {
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
      fprintf(fp, "%g)\n",pCoords[(Data.m_pParameters->GetDimensionsOfData())-1]);
      if (m_iEllipseOffset == 0) {
        fprintf(fp, "%sRadius................: %-5.2f\n", szSpacesOnLeft, nRadius);
        if (Data.m_pParameters->GetNumRequestedEllipses()) {
          //print circle as ellipse with shape of '1' when analysis has ellipses
          fprintf(fp, "%sEllipse Parameters....:\n", szSpacesOnLeft);
          fprintf(fp, "%sAngle (degrees).......: n/a\n", szSpacesOnLeft);
          fprintf(fp, "%sShape.................: 1.0\n", szSpacesOnLeft);
        }
      }
      else {
        fprintf(fp, "%sEllipse Semiminor axis: %-g\n", szSpacesOnLeft, nRadius);
        fprintf(fp, "%sEllipse Parameters....:\n", szSpacesOnLeft);
        fprintf(fp, "%sAngle (degrees).......: %-g\n", szSpacesOnLeft, ConvertAngleToDegrees(Data.mdE_Angles[m_iEllipseOffset-1]));
        fprintf(fp, "%sShape.................: %-g\n", szSpacesOnLeft, Data.mdE_Shapes[m_iEllipseOffset-1]);
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
   float nRadius;
   float Latitude, Longitude;
   char  cNorthSouth, cEastWest;

   try
      {
      (Data.GetGInfo())->giGetCoords(m_Center, &pCoords);
      (Data.GetTInfo())->tiGetCoords(Data.GetNeighbor(0, m_Center, m_nTracts), &pCoords2);

      nRadius = (float)sqrt((Data.GetTInfo())->tiGetDistanceSq(pCoords, pCoords2));

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
void CCluster::DisplayNullOccurrence(FILE* fp, const CSaTScanData& Data, char* szSpacesOnLeft) {
  float         fDaysInOccurrence, fYears, fMonths, fDays;
  Julian        ProspectiveStartDate;

  try {

    if (Data.m_pParameters->GetAnalysisType() == PROSPECTIVESPACETIME && Data.m_pParameters->GetNumReplicationsRequested() > 99) {
      ProspectiveStartDate = CharToJulian(Data.m_pParameters->GetProspectiveStartDate().c_str());
      fprintf(fp, "%sNull Occurrence.......: ", szSpacesOnLeft);
      switch (Data.m_pParameters->GetTimeIntervalUnitsType()) {
        case YEAR   : fDaysInOccurrence = (TimeBetween(ProspectiveStartDate, Data.m_nEndDate, DAY) + AVERAGE_DAYS_IN_YEAR) / GetPVal(Data.m_pParameters->GetNumReplicationsRequested());
                      fYears = fDaysInOccurrence/AVERAGE_DAYS_IN_YEAR;
                      fprintf(fp, "Once in %.1f year%s\n", fYears, (fYears > 1 ? "s" : ""));
                      break;
        case MONTH  : fDaysInOccurrence = (TimeBetween(ProspectiveStartDate, Data.m_nEndDate, DAY) + AVERAGE_DAYS_IN_MONTH) / GetPVal(Data.m_pParameters->GetNumReplicationsRequested());
                      fYears = floor(fDaysInOccurrence/AVERAGE_DAYS_IN_YEAR);
                      fDays = fDaysInOccurrence - fYears * AVERAGE_DAYS_IN_YEAR;
                      fMonths = fDays/AVERAGE_DAYS_IN_MONTH;
                      /*Round now for values that cause months to goto 12 or down to 0.*/
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
        case DAY    : fDaysInOccurrence = (TimeBetween(ProspectiveStartDate, Data.m_nEndDate, DAY) + 1) / GetPVal(Data.m_pParameters->GetNumReplicationsRequested());
                      fYears = floor(fDaysInOccurrence/AVERAGE_DAYS_IN_YEAR);
                      fDays = fDaysInOccurrence - fYears * AVERAGE_DAYS_IN_YEAR;
                      /*Round now for values that cause days to go 365 or down to 0.*/
                      if (fDays >= 364.5) {
                        ++fYears;
                        fDays = 0;
                      }
                      else if (fDays < .5)
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
                                          "DisplayNullOccurrence()", Data.m_pParameters->GetTimeIntervalUnitsType());
      }
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("DisplayNullOccurrence()","CCluster");
    throw;
  }
}

void CCluster::DisplayPopulation(FILE* fp, const CSaTScanData& Data, char* szSpacesOnLeft)
{
  fprintf(fp, "%sPopulation............: %-10.0f\n",
  szSpacesOnLeft,
  Data.m_pModel->GetPopulation(m_iEllipseOffset, m_Center, m_nTracts, m_nFirstInterval, m_nLastInterval));
}

void CCluster::DisplayPVal(FILE* fp, int nReplicas, char* szSpacesOnLeft)
{
  float pVal = (float)GetPVal(nReplicas);
  gfPValue = pVal;

  if (nReplicas > 9999)
    fprintf(fp, "%.5f", pVal);
  else if (nReplicas > 999)
    fprintf(fp, "%.4f", pVal);
  else if (nReplicas > 99)
    fprintf(fp, "%.3f", pVal);
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

/** Duczmal compactness correction. For circles this should be no different than
    the loglikelihood ratio as m_DuczmalCorrection should be 1. */
double CCluster::GetDuczmalCorrectedLogLikelihoodRatio() const {
  return m_DuczmalCorrection * m_nRatio;
}

const double CCluster::GetRelativeRisk(double nMeasureAdjustment) const
{
  double        dRelativeRisk=0;

  if (m_nMeasure*nMeasureAdjustment)
    dRelativeRisk = ((double)(m_nCases))/(m_nMeasure*nMeasureAdjustment);

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

bool CCluster::RateIsOfInterest(count_t nTotalCases, measure_t nTotalMeasure)
{
  return(m_pfRateOfInterest(m_nCases,m_nMeasure,
                            nTotalCases, nTotalMeasure));
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

double CCluster::SetRatio(double nLogLikelihoodForTotal)
{
  m_bRatioSet = true;
  m_nRatio    = GetLogLikelihood() - nLogLikelihoodForTotal;

  return m_nRatio;

}

void CCluster::SetRatioAndDates(const CSaTScanData& Data)
{
  if (ClusterDefined())
    {
    m_bClusterSet = true;
    SetRatio(Data.m_pModel->GetLogLikelihoodForTotal());
    SetStartAndEndDates(Data.m_pIntervalStartTimes, Data.m_nTimeIntervals);
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
            for (i=0; i<=(pData->m_pParameters->GetDimensionsOfData())-1; i++)
               fprintf(fp, " %14s","n/a");
            fprintf(fp, " %12s ","n/a");
            }
         else
            {
            for (i=0; i<=(pData->m_pParameters->GetDimensionsOfData())-1; i++)
               fprintf(fp, " %14.6g",pCoords[i]);
            fprintf(fp, " %12.2f",nRadius);
            }

         //these are not applicable for circles....
         //SHAPE, ANGLE
         if (pData->m_nNumEllipsoids > 0)
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
            for (i=0; i<=(pData->m_pParameters->GetDimensionsOfData())-1; i++)
               fprintf(fp, " %14s", "n/a");
            fprintf(fp, " %12s","n/a");

            fprintf(fp, " %8s %8s", "n/a", "n/a");
            }
         else
            {
            for (i=0; i<=(pData->m_pParameters->GetDimensionsOfData())-1; i++)
               fprintf(fp, " %14.6g",pCoords[i]);

            //just print the nRadius value here...
            //IT IS NOT RADIUS
            //IT IS SEMI-MINOR AXIS.....  FOR AN ELLIPSOID
            fprintf(fp, " %12.2f",nRadius);

            //PRINT SHAPE AND ANGLE
            fprintf(fp, " %8.3f", pData->mdE_Shapes[m_iEllipseOffset-1]);
            fprintf(fp, " %8.3f", ConvertAngleToDegrees(pData->mdE_Angles[m_iEllipseOffset-1]));
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
   float nRadius;
   float Latitude, Longitude;
   //char  cNorthSouth, cEastWest;

   try
      {
      (pData->GetGInfo())->giGetCoords(m_Center, &pCoords);
      (pData->GetTInfo())->tiGetCoords(pData->GetNeighbor(0, m_Center, m_nTracts), &pCoords2);

      nRadius = (float)sqrt((pData->GetTInfo())->tiGetDistanceSq(pCoords, pCoords2));

      ConvertToLatLong(&Latitude, &Longitude, pCoords);

     // Latitude >= 0 ? cNorthSouth = 'N' : cNorthSouth = 'S';
     // Longitude >= 0 ? cEastWest = 'E' : cEastWest = 'W';

      fprintf(fp, " %lf %lf %5.2f", Latitude, Longitude, nRadius);

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

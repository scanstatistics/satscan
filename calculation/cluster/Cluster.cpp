#include "SaTScan.h"
#pragma hdrstop

#include "Cluster.h"
#include "stsAreaSpecificDBF.h"

CCluster::CCluster(BasePrint *pPrintDirection)
{
  Initialize();
  gpPrintDirection = pPrintDirection;
}

CCluster::~CCluster()
{
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
  gpAreaDBFReport = 0;
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

double CCluster::ConvertAngleToDegrees(double dAngle)
{
   double dDegrees;
   
   try
      {
      dDegrees = 180.00 * (dAngle / (double)M_PI);
      if (dDegrees > 90.00)
         dDegrees -= 180.00;
      }
   catch (SSException & x)
      {
      x.AddCallpath("ConvertAngleToDegrees()", "CCluster");
      throw;
      }
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
        nLeftMargin += n;
        n = (int)floor(((double)n)/10);
        }

      DisplaySteps(fp, szSpacesOnLeft);
      //  fprintf(fp, "Census areas included");
      fprintf(fp, "Census areas ");
      DisplayCensusTracts(fp, Data, -1, nMinMeasure,
                          Parameters.m_nReplicas, false, false,
                          nLeftMargin, nRightMargin, cDeliminator, szSpacesOnLeft);
    
      if (Parameters.m_nCoordType == CARTESIAN)
      	DisplayCoordinates(fp, Data, nLeftMargin, nRightMargin, cDeliminator, szSpacesOnLeft);
      else
      	DisplayLatLongCoords(fp, Data, nLeftMargin, nRightMargin, cDeliminator, szSpacesOnLeft);

      DisplayTimeFrame(fp, szSpacesOnLeft, Parameters.m_nAnalysisType);
      if (Parameters.m_nModel != SPACETIMEPERMUTATION)
        DisplayPopulation(fp, Data, szSpacesOnLeft);

      fprintf(fp, "%sNumber of cases.......: %ld", szSpacesOnLeft, m_nCases);
      fprintf(fp, "          (%.2f expected)\n", Data.GetMeasureAdjustment()*m_nMeasure);
    
      if (Parameters.m_nModel == POISSON)
        fprintf(fp, "%sAnnual cases / %.0f.: %.1f\n",
                     szSpacesOnLeft, Data.GetAnnualRatePop(),
                     Data.GetAnnualRateAtStart()*GetRelativeRisk(Data.GetMeasureAdjustment()));

      DisplayRelativeRisk(fp, Data.GetMeasureAdjustment(), nLeftMargin, nRightMargin, cDeliminator, szSpacesOnLeft);

      // For space-time permutation, ratio is technically no longer a likelihood ratio test statistic.
      fprintf(fp, "%s%s: %f\n", szSpacesOnLeft,
              (Parameters.m_nModel == SPACETIMEPERMUTATION ? "Test statistic........" : "Log likelihood ratio.."),
              m_nRatio);
      fprintf(fp, "%sMonte Carlo rank......: %ld/%ld\n",
                   szSpacesOnLeft, m_nRank, Parameters.m_nReplicas+1);

      if (Parameters.m_nReplicas > 99)
        {
        fprintf(fp, "%sP-value...............: ", szSpacesOnLeft);
        DisplayPVal(fp, Parameters.m_nReplicas, szSpacesOnLeft);
        fprintf(fp, "\n");
        }
      }
   catch (SSException & x)
      {
      x.AddCallpath("Display()", "CCluster");
      throw;
      }
}

void CCluster::DisplayCensusTracts(FILE* fp, const CSaTScanData& Data,
                                   int nCluster, measure_t nMinMeasure, int nReplicas, bool bIncludeRelRisk,
                                   bool bIncludePVal, int nLeftMargin, int nRightMargin, char cDeliminator,
                                   char* szSpacesOnLeft, bool bFormat)
{
   try {
      if (nLeftMargin > 0 && fp != NULL)
         fprintf(fp, "included.: ");

       DisplayCensusTractsInStep(fp, Data, 1, m_nTracts, nCluster, nMinMeasure,
                            nReplicas, bIncludeRelRisk, bIncludePVal,
                            nLeftMargin, nRightMargin, cDeliminator, szSpacesOnLeft, bFormat);
   }
   catch (SSException & x) {
      x.AddCallpath("DisplayCensusTracts()", "CCluster");
      throw;
   }
}

void CCluster::DisplayCensusTractsInStep(FILE* fp, const CSaTScanData& Data,
                                         tract_t nFirstTract, tract_t nLastTract,
                                         int nCluster, measure_t nMinMeasure, int nReplicas, bool bIncludeRelRisk,
                                         bool bIncludePVal, int nLeftMargin, int nRightMargin, char cDeliminator,
                                         char* szSpacesOnLeft, bool bFormat)
{
   int   pos  = nLeftMargin;
   char* tid;
   int   nCount=0;

   try {
      for (int i = nFirstTract; i <= nLastTract; i++)
      {
           //if (Data.m_pMeasure[0][Data.GetNeighbor(0, m_Center, i)]>nMinMeasure)       // access over-run here
           //if (Data.m_pMeasure[m_iEllipseOffset][Data.GetNeighbor(m_iEllipseOffset, m_Center, i)]>nMinMeasure) // access over-run here

           // the first dimension of m_pMeasure is Time Interval !!!
           tract_t tTract = Data.GetNeighbor(m_iEllipseOffset, m_Center, i);
           if (Data.m_pMeasure[0][tTract]>nMinMeasure) {
             nCount++;
             tid = (Data.GetTInfo())->tiGetTid(tTract);

             if(fp != NULL) {
                pos += strlen(tid) + 2;

                if (nCount>1 && pos>nRightMargin)
                {
                  pos = nLeftMargin + strlen(tid) + 2;
                  fprintf(fp, "\n");
                  for (int j=0; j<nLeftMargin; ++j)
                    fprintf(fp, " ");
                }

                if (nCluster > -1)
                  fprintf(fp, "%i         ", nCluster);
                if (bFormat)
                   fprintf(fp, "%s", tid);
                else
                   fprintf(fp, "%-29s", tid);

                if (bIncludeRelRisk)
                  fprintf(fp, "   %-12.3f", GetRelativeRisk(Data.GetMeasureAdjustment()));
                if (bIncludePVal)
                {
                  fprintf(fp, "     ");
                  DisplayPVal(fp, nReplicas, szSpacesOnLeft);
                }

                fprintf(fp, "\t %12i", GetCaseCountForTract(tTract, Data));
                fprintf(fp, "\t %12.3f", GetMeasureForTract(tTract, Data));
                fprintf(fp, "\t %12.3f", GetRelativeRiskForTract(tTract, Data));

                if (i < nLastTract)
                  fprintf(fp, "%c ", cDeliminator);
              }     // end if fp
           }

       // record DBF output data - AJV
       if(gpAreaDBFReport && Data.m_pParameters->GetOutputAreaSpecificDBF())
          gpAreaDBFReport->RecordClusterData(*this, Data, nCluster, i);
      }
      if(fp != NULL)
         fprintf(fp, "\n");
   }
   catch (SSException & x)
      {
      x.AddCallpath("DisplayCensusTractsInStep()", "CCluster");
      throw;
      }
}

void CCluster::DisplayCoordinates(FILE* fp, const CSaTScanData& Data,
                                  int nLeftMargin, int nRightMargin,
                                  char cDeliminator, char* szSpacesOnLeft)
{
   double *pCoords = 0, *pCoords2 = 0;
   float nRadius;
   int i;

   try
      {
      (Data.GetGInfo())->giGetCoords(m_Center, &pCoords);
      //tiGetCoords(Data.GetNeighbor(0, m_Center, m_nTracts), &pCoords2);       DTG
      (Data.GetTInfo())->tiGetCoords(Data.GetNeighbor(m_iEllipseOffset, m_Center, m_nTracts), &pCoords2);
    
      nRadius = (float)sqrt((Data.GetTInfo())->tiGetDistanceSq(pCoords, pCoords2));
    
      //fprintf(fp, "  Coordinates / radius..........: (%g,%g) / %5.2f\n",
                   //x1, y1, nRadius);
    
      if(Data.m_pParameters->m_nDimension < 5)
      {
         if ( m_iEllipseOffset == 0 )
            {
            fprintf(fp, "%sCoordinates / radius..: (", szSpacesOnLeft);
      	    for (i=0; i<(Data.m_pParameters->m_nDimension)-1; i++)
      	       fprintf(fp, "%g,",pCoords[i]);
      	    fprintf(fp, "%g) / %-5.2f\n",pCoords[(Data.m_pParameters->m_nDimension)-1],nRadius);
            }
         else
            {
            fprintf(fp, "%sCoordinates...........: (", szSpacesOnLeft);
            for (i=0; i<(Data.m_pParameters->m_nDimension)-1; i++)
      	       fprintf(fp, "%g,",pCoords[i]);
            fprintf(fp, "%g)\n",pCoords[(Data.m_pParameters->m_nDimension)-1]);
            fprintf(fp, "%sEllipse Semiminor axis: %-6.3f\n", szSpacesOnLeft, nRadius);
            //Print the ellipse dimensions....
            fprintf(fp, "%sEllipse Parameters....:\n", szSpacesOnLeft);
            fprintf(fp, "%sAngle (degrees).......: %-6.3f\n", szSpacesOnLeft, ConvertAngleToDegrees(Data.mdE_Angles[m_iEllipseOffset-1]));
            fprintf(fp, "%sShape.................: %-6.3f\n", szSpacesOnLeft, Data.mdE_Shapes[m_iEllipseOffset-1]);
            //fprintf(fp, "%s              Width...: \n", szSpacesOnLeft);
            //fprintf(fp, "%s              Length..: \n", szSpacesOnLeft);
            }
      }
      else /* More than four dimensions: need to wrap output */
      {
        fprintf(fp, "%sCoordinates...........: (", szSpacesOnLeft);
        int count = 0;
        for (i=0; i<(Data.m_pParameters->m_nDimension)-1; i++)
        {
          if (count < 4) // This is a magic number: if 5 dimensions they
          							 // all print on one line; if more, 4 per line
          {
    	 fprintf(fp, "%g,",pCoords[i]);
    	 count++;
          }
          else /*Start a new line */
          {
    //      	fprintf(fp,"\n                                   ");
            fprintf(fp,"\n");
            for (int j=0; j<nLeftMargin+1; j++)
              fprintf(fp, " ");
            fprintf(fp, "%g,",pCoords[i]);
            count = 1;
          }
        }
        fprintf(fp, "%g)\n",pCoords[(Data.m_pParameters->m_nDimension)-1]);
        if (m_iEllipseOffset == 0)
           fprintf(fp, "%sRadius................: %-5.2f\n", szSpacesOnLeft, nRadius);
        else
           {
           fprintf(fp, "%sEllipse Semiminor axis: %-6.3f\n", szSpacesOnLeft, nRadius);
           fprintf(fp, "%sEllipse Parameters....:\n", szSpacesOnLeft);
           fprintf(fp, "%sAngle (degrees).......: %-6.3f\n", szSpacesOnLeft, ConvertAngleToDegrees(Data.mdE_Angles[m_iEllipseOffset-1]));
           fprintf(fp, "%sShape.................: %-6.3f\n", szSpacesOnLeft, Data.mdE_Shapes[m_iEllipseOffset-1]);
           //fprintf(fp, "%s              Width...: \n", szSpacesOnLeft);
           //fprintf(fp, "%s              Length..: \n", szSpacesOnLeft);
           }
      }

      free(pCoords);
      free(pCoords2);
      }
   catch (SSException & x)
      {
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
   catch (SSException & x)
      {
      free(pCoords);
      free(pCoords2);
      x.AddCallpath("DisplayLatLongCoords()", "CCluster");
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
   try
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
   catch (SSException & x)
      {
      x.AddCallpath("DisplayPVal()", "CCluster");
      throw;
      }
}

void CCluster::DisplayRelativeRisk(FILE* fp,
                                   double nMeasureAdjustment,
                                   int nLeftMargin, int nRightMargin,
                                   char cDeliminator, char* szSpacesOnLeft)
{
   try
      {
      //  fprintf(fp, "          (Relative risk: %.2f)\n", GetRelativeRisk());
      fprintf(fp, "%sOverall relative risk.: %.3f\n", szSpacesOnLeft, GetRelativeRisk(nMeasureAdjustment));
      }
   catch (SSException & x)
      {
      x.AddCallpath("DisplayRelativeRisk()", "CCluster");
      throw;
      }
}

void CCluster::DisplayTimeFrame(FILE* fp, char* szSpacesOnLeft, int nAnalysisType)
{
  char szStartDt[MAX_DT_STR];
  char szEndDt[MAX_DT_STR];

   try
      {
      fprintf(fp, "%sTime frame............: %s - %s\n",
               szSpacesOnLeft,
               JulianToChar(szStartDt, m_nStartDate),
               JulianToChar(szEndDt, m_nEndDate));
      }
   catch (SSException & x)
      {
      x.AddCallpath("DisplayTimeFrame()", "CCluster");
      throw;
      }
}

/** Returns the number of case for tract as defined by cluster. */
count_t CCluster::GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data) const
{
  count_t       tCaseCount=0;

  if (m_nFirstInterval != m_nLastInterval)
     tCaseCount = Data.m_pCases[m_nFirstInterval][tTract];

  return tCaseCount;
}

/** Returns the measure for tract as defined by cluster. */
measure_t CCluster::GetMeasureForTract(tract_t tTract, const CSaTScanData& Data) const
{
  measure_t     tMeasure=0;

  if (m_nFirstInterval != m_nLastInterval)
     tMeasure = Data.m_pMeasure[m_nFirstInterval][tTract];

  return tMeasure;
}

double CCluster::GetRelativeRisk(double nMeasureAdjustment)
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

  if (tMeasure*Data.GetMeasureAdjustment())
    dRelativeRisk = ((double)(tCaseCount))/(tMeasure*Data.GetMeasureAdjustment());
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
   try
      {
      m_bRatioSet = true;
      m_nRatio    = GetLogLikelihood() - nLogLikelihoodForTotal;
      }
   catch (SSException & x)
      {
      x.AddCallpath("SetRatio()", "CCluster");
      throw;
      }
  return m_nRatio;

}

void CCluster::SetRatioAndDates(const CSaTScanData& Data)
{
   try
      {
      if (ClusterDefined())
         {
         m_bClusterSet = true;
         SetRatio(Data.m_pModel->GetLogLikelihoodForTotal());
         SetStartAndEndDates(Data.m_pIntervalStartTimes,
                        Data.m_nTimeIntervals);
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("SetRatioAndDates()", "CCluster");
      throw;
      }
}

void CCluster::SetStartAndEndDates(const Julian* pIntervalStartTimes,
                                   int nTimeIntervals)
{
   try
      {
      m_nStartDate = pIntervalStartTimes[m_nFirstInterval];
      m_nEndDate   = pIntervalStartTimes[m_nLastInterval]-1;
      }
   catch (SSException & x)
      {
      x.AddCallpath("SetStartAndEndDates()", "CCluster");
      throw;
      }
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
            for (i=0; i<=(pData->m_pParameters->m_nDimension)-1; i++)
               fprintf(fp, " %14s","n/a");
            fprintf(fp, " %12s ","n/a");
            }
         else
            {
            for (i=0; i<=(pData->m_pParameters->m_nDimension)-1; i++)
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
            for (i=0; i<=(pData->m_pParameters->m_nDimension)-1; i++)
               fprintf(fp, " %14s", "n/a");
            fprintf(fp, " %12s","n/a");

            fprintf(fp, " %8s %8s", "n/a", "n/a");
            }
         else
            {
            for (i=0; i<=(pData->m_pParameters->m_nDimension)-1; i++)
               fprintf(fp, " %14.6g",pCoords[i]);

            //just print the nRadius value here...
            //IT IS NOT RADIUS
            //IT IS SEMI-MINOR AXIS.....  FOR AN ELLIPSOID
            fprintf(fp, " %12.2f",nRadius);

            //PRINT SHAPE AND ANGLE
            fprintf(fp, " %8.3f", ConvertAngleToDegrees(pData->mdE_Angles[m_iEllipseOffset-1]));
            fprintf(fp, " %8.3f", pData->mdE_Shapes[m_iEllipseOffset-1]);
            }
         }

      // If Space-Time analysis...  put Start and End of Cluster
     /* if (pData->m_pParameters->m_nAnalysisType != PURELYSPATIAL)
         {
         JulianToChar(sStartDate, m_nStartDate);
         JulianToChar(sEndDate, m_nEndDate);
         fprintf(fp, " %11s %11s", sStartDate, sEndDate);
         }
      else //purely spacial - print study begin and end dates
         fprintf(fp, " %11s %11s", pData->m_pParameters->m_szStartDate, pData->m_pParameters->m_szEndDate);
       */
      free(pCoords);
      free(pCoords2);
      }
   catch (SSException & x)
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
   catch (SSException & x)
      {
      free(pCoords);
      free(pCoords2);
      x.AddCallpath("WriteLatLongCoords()", "CCluster");
      throw;
      }
}

// Cluster.cpp

#include <stdio.h>
#include <float.h>
#include "Cluster.h"
#include "Param.h"
#include "Cats.h"
#include "tinfo.h"
#include "ginfo.h"
//#include "LogLikelihood.h"
#include "Model.h"
#include "LatLong.h" //???

CCluster::CCluster()
{
  Initialize();
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
}

void CCluster::SetCenter(tract_t nCenter)
{
  m_Center = nCenter;
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

  return *this;
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
  DisplayPopulation(fp, Data, szSpacesOnLeft);

  fprintf(fp, "%sNumber of cases.......: %ld", szSpacesOnLeft, m_nCases);
  fprintf(fp, "          (%.2f expected)\n", Data.GetMeasureAdjustment()*m_nMeasure);

  if (Parameters.m_nModel == POISSON)
    fprintf(fp, "%sAnnual cases / %.0f.: %.1f\n",
                 szSpacesOnLeft, Data.GetAnnualRatePop(),
                 Data.GetAnnualRateAtStart()*GetRelativeRisk(Data.GetMeasureAdjustment()));

  DisplayRelativeRisk(fp, Data.GetMeasureAdjustment(), nLeftMargin, nRightMargin, cDeliminator, szSpacesOnLeft);

  fprintf(fp, "%sLog likelihood ratio..: %f\n", szSpacesOnLeft, m_nRatio);
  fprintf(fp, "%sMonte Carlo rank......: %ld/%ld\n",
               szSpacesOnLeft, m_nRank, Parameters.m_nReplicas+1);

  if (Parameters.m_nReplicas > 99)
  {
    fprintf(fp, "%sP-value...............: ", szSpacesOnLeft);
    DisplayPVal(fp, Parameters.m_nReplicas, szSpacesOnLeft);
    fprintf(fp, "\n");
  }

}

void CCluster::DisplayRelativeRisk(FILE* fp,
                                   double nMeasureAdjustment,
                                   int nLeftMargin, int nRightMargin,
                                   char cDeliminator, char* szSpacesOnLeft)
{
//  fprintf(fp, "          (Relative risk: %.2f)\n", GetRelativeRisk());
  fprintf(fp, "%sOverall relative risk.: %.3f\n",
               szSpacesOnLeft, GetRelativeRisk(nMeasureAdjustment));
}

void CCluster::DisplayCensusTracts(FILE* fp, const CSaTScanData& Data,
                                   int nCluster, measure_t nMinMeasure,
                                   int nReplicas, bool bIncludeRelRisk, bool bIncludePVal,
                                   int nLeftMargin, int nRightMargin,
                                   char cDeliminator, char* szSpacesOnLeft)
{
  if (nLeftMargin > 0)
    fprintf(fp, "included.: ");

  DisplayCensusTractsInStep(fp, Data, 1, m_nTracts, nCluster, nMinMeasure,
                            nReplicas, bIncludeRelRisk, bIncludePVal,
                            nLeftMargin, nRightMargin, cDeliminator, szSpacesOnLeft);
}

void CCluster::DisplayCensusTractsInStep(FILE* fp, const CSaTScanData& Data,
                                         tract_t nFirstTract, tract_t nLastTract,
                                         int nCluster, measure_t nMinMeasure,
                                         int nReplicas,
                                         bool bIncludeRelRisk, bool bIncludePVal,
                                         int nLeftMargin, int nRightMargin,
                                         char cDeliminator, char* szSpacesOnLeft)
{
  int   pos  = nLeftMargin;
  char* tid;
  int   nCount=0;

  for (int i = nFirstTract; i <= nLastTract; i++)
  {
    if (Data.m_pMeasure[0][Data.GetNeighbor(m_Center, i)]>nMinMeasure)
    {
      nCount++;
      tid = tiGetTid(Data.GetNeighbor(m_Center, i));
      pos += strlen(tid) + 2;

      if (nCount>1 && pos>nRightMargin)
      {
        pos = nLeftMargin + strlen(tid) + 2;
        fprintf(fp, "\n");
        for (int j=0; j<nLeftMargin; j++)
          fprintf(fp, " ");
      }

      if (nCluster > -1)
        fprintf(fp, "%i         ", nCluster);
      fprintf(fp, "%s", tid);

      if (bIncludeRelRisk)
        fprintf(fp, "          %.3f", GetRelativeRisk(Data.GetMeasureAdjustment()));
      if (bIncludePVal)
      {
        fprintf(fp, "          ");
        DisplayPVal(fp, nReplicas, szSpacesOnLeft);
      }

      if (i < nLastTract)
        fprintf(fp, "%c ", cDeliminator);

    }
  }
  fprintf(fp, "\n");
}

void CCluster::DisplayCoordinates(FILE* fp, const CSaTScanData& Data,
                                  int nLeftMargin, int nRightMargin,
                                  char cDeliminator, char* szSpacesOnLeft)
{
  float *pCoords, *pCoords2;
  float nRadius;
  int i;

  giGetCoords(m_Center, &pCoords);
  tiGetCoords(Data.GetNeighbor(m_Center, m_nTracts), &pCoords2);

  nRadius = (float)sqrt(tiGetDistanceSq(pCoords, pCoords2));

  //fprintf(fp, "  Coordinates / radius..........: (%g,%g) / %5.2f\n",
               //x1, y1, nRadius);

  if(Data.m_pParameters->m_nDimension < 5)
  {
  	fprintf(fp, "%sCoordinates / radius..: (", szSpacesOnLeft);
  	for (i=0; i<(Data.m_pParameters->m_nDimension)-1; i++)
  	{
  		fprintf(fp, "%g,",pCoords[i]);
  	}
  	fprintf(fp, "%g) / %-5.2f\n",pCoords[(Data.m_pParameters->m_nDimension)-1],nRadius);
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
    fprintf(fp, "%sRadius................: %-5.2f\n",
                 szSpacesOnLeft, nRadius);
  }

  free(pCoords);
  free(pCoords2);
}

//void CCluster::DisplayLatLongCoords(FILE* fp, const CSaTScanData& Data,
//                                  int nLeftMargin, int nRightMargin,
//                                  char cDeliminator)
void CCluster::DisplayLatLongCoords(FILE* fp, const CSaTScanData& Data,
                                  int nLeftMargin, int nRightMargin,
                                  char cDeliminator, char* szSpacesOnLeft)
{
  float *pCoords, *pCoords2;
  float nRadius;
  float Latitude, Longitude;
  char  cNorthSouth, cEastWest;

  giGetCoords(m_Center, &pCoords);
  tiGetCoords(Data.GetNeighbor(m_Center, m_nTracts), &pCoords2);

  nRadius = (float)sqrt(tiGetDistanceSq(pCoords, pCoords2));

  ConvertToLatLong(&Latitude, &Longitude, pCoords);

  Latitude >= 0 ? cNorthSouth = 'N' : cNorthSouth = 'S';
  Longitude >= 0 ? cEastWest = 'E' : cEastWest = 'W';

//  fprintf(fp, "  Coordinates / radius..........: (%.3f %c, %.3f %c) / %5.2f\n",
//               fabs(Latitude), cNorthSouth, fabs(Longitude), cEastWest, nRadius);
  fprintf(fp, "%sCoordinates / radius..: (%.3f %c, %.3f %c) / %5.2f\n",
               szSpacesOnLeft, fabs(Latitude), cNorthSouth, fabs(Longitude), cEastWest, nRadius);

  free(pCoords);
  free(pCoords2);
}

void CCluster::DisplayPopulation(FILE* fp, const CSaTScanData& Data, char* szSpacesOnLeft)
{
  fprintf(fp, "%sPopulation............: %-10.0f\n",
  szSpacesOnLeft,
  Data.m_pModel->GetPopulation(m_Center, m_nTracts, m_nFirstInterval, m_nLastInterval));
}

void CCluster::DisplayTimeFrame(FILE* fp, char* szSpacesOnLeft, int nAnalysisType)
{
  char szStartDt[MAX_DT_STR];
  char szEndDt[MAX_DT_STR];

  fprintf(fp, "%sTime frame............: %s - %s\n",
               szSpacesOnLeft,
               JulianToChar(szStartDt, m_nStartDate),
               JulianToChar(szEndDt, m_nEndDate));

}

void CCluster::SetStartAndEndDates(const Julian* pIntervalStartTimes,
                                   int nTimeIntervals)
{
  m_nStartDate = pIntervalStartTimes[m_nFirstInterval];
  m_nEndDate   = pIntervalStartTimes[m_nLastInterval]-1;
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

bool CCluster::RateIsOfInterest(count_t nTotalCases, measure_t nTotalMeasure)
{
  return(m_pfRateOfInterest(m_nCases,m_nMeasure,
                            nTotalCases, nTotalMeasure));
}

void CCluster::SetRatioAndDates(const CSaTScanData& Data)
{
  if (ClusterDefined())
  {
    m_bClusterSet = true;
    SetRatio(Data.m_pModel->GetLogLikelihoodForTotal());
    SetStartAndEndDates(Data.m_pIntervalStartTimes,
                        Data.m_nTimeIntervals);
  }
}

double CCluster::GetRelativeRisk(double nMeasureAdjustment)
{
  return ((double)(m_nCases))/(m_nMeasure*nMeasureAdjustment);
}

void CCluster::DisplayPVal(FILE* fp, int nReplicas, char* szSpacesOnLeft)
{
  float pVal = (float)GetPVal(nReplicas);

  if (nReplicas > 9999)
    fprintf(fp, "%.5f", pVal);
  else if (nReplicas > 999)
    fprintf(fp, "%.4f", pVal);
  else if (nReplicas > 99)
    fprintf(fp, "%.3f", pVal);
}

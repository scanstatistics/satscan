// PSMCluster.c

#include <stdio.h>
#include "PSMCluster.h"
#include "param.h"
#include "tinfo.h"
#include "ginfo.h"
#include "model.h"
#include "LatLong.h"

CPSMonotoneCluster::CPSMonotoneCluster()
                   :CPurelySpatialCluster()
{
  m_nMaxCircles        = 0;
  m_pCasesList         = NULL;
  m_pMeasureList       = NULL;
  m_pFirstNeighborList = NULL;
  m_pLastNeighborList  = NULL;

  Initialize(0);
}

CPSMonotoneCluster::~CPSMonotoneCluster()
{
  if (m_pCasesList != NULL)
    free(m_pCasesList);
  if (m_pMeasureList != NULL)
    free(m_pMeasureList);
  if (m_pFirstNeighborList != NULL)
    free(m_pFirstNeighborList);
  if (m_pLastNeighborList != NULL)
    free(m_pLastNeighborList);
}

void CPSMonotoneCluster::Initialize(tract_t nCenter=0)
{
  CCluster::Initialize(nCenter);

  m_nClusterType = PURELYSPATIALMONOTONE;

  for (int i=0; i<m_nMaxCircles; i++)
  {
    m_pCasesList[i]         = 0;
    m_pMeasureList[i]       = 0;
    m_pFirstNeighborList[i] = 0;
    m_pLastNeighborList[i]  = 0;
  }

}

void CPSMonotoneCluster::AllocateForMaxCircles(tract_t nCircles)
{
  m_nMaxCircles        = nCircles;
  m_pCasesList         = (count_t*)  Smalloc(m_nMaxCircles * sizeof(count_t));
  m_pMeasureList       = (measure_t*)Smalloc(m_nMaxCircles * sizeof(measure_t));
  m_pFirstNeighborList = (tract_t*)  Smalloc(m_nMaxCircles * sizeof(tract_t));
  m_pLastNeighborList  = (tract_t*)  Smalloc(m_nMaxCircles * sizeof(tract_t));
}

CPSMonotoneCluster& CPSMonotoneCluster::operator =(const CPSMonotoneCluster& cluster)
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
  m_nMaxCircles    = cluster.m_nSteps;

  if (m_pCasesList != NULL)
    free(m_pCasesList);
  if (m_pMeasureList != NULL)
    free(m_pMeasureList);
  if (m_pFirstNeighborList != NULL)
    free(m_pFirstNeighborList);
  if (m_pLastNeighborList != NULL)
    free(m_pLastNeighborList);

  m_pCasesList         = (count_t*)  Smalloc(m_nSteps * sizeof(count_t));
  m_pMeasureList       = (measure_t*)Smalloc(m_nSteps * sizeof(measure_t));
  m_pFirstNeighborList = (tract_t*)  Smalloc(m_nSteps * sizeof(tract_t));
  m_pLastNeighborList  = (tract_t*)  Smalloc(m_nSteps * sizeof(tract_t));

  for (int i=0; i<m_nMaxCircles; i++)
  {
    m_pCasesList[i]         = cluster.m_pCasesList[i];
    m_pMeasureList[i]       = cluster.m_pMeasureList[i];
    m_pFirstNeighborList[i] = cluster.m_pFirstNeighborList[i];
    m_pLastNeighborList[i]  = cluster.m_pLastNeighborList[i];
  }

  m_bClusterInit   = cluster.m_bClusterInit;
  m_bClusterDefined= cluster.m_bClusterDefined;
  m_bClusterSet    = cluster.m_bClusterSet;
  m_bLogLSet       = cluster.m_bLogLSet;
  m_bRatioSet      = cluster.m_bRatioSet;
  m_nClusterType   = cluster.m_nClusterType;

  return *this;
}

void CPSMonotoneCluster::AddNeighbor(const CSaTScanData& Data, count_t** pCases, tract_t n)
{
  tract_t nNeighbor = Data.GetNeighbor(m_Center, n);

  m_nSteps++;

  m_nCases   += pCases[0][nNeighbor];
  m_nMeasure += Data.m_pMeasure[0][nNeighbor];

  m_pCasesList[m_nSteps-1]         = pCases[0][nNeighbor];
  m_pMeasureList[m_nSteps-1]       = Data.m_pMeasure[0][nNeighbor];
  m_pFirstNeighborList[m_nSteps-1] = n;
  m_pLastNeighborList[m_nSteps-1]  = n;
}

void CPSMonotoneCluster::CheckCircle(tract_t n)
{
  if (n != 0)
    if (!m_pfRateOfInterest(m_pCasesList[n-1],m_pMeasureList[n-1],
                            m_pCasesList[n],  m_pMeasureList[n]))
    {
      ConcatLastCircles();
      CheckCircle(n-1);
    }
}

void CPSMonotoneCluster::ConcatLastCircles()
{
  m_nSteps--;
  m_pCasesList[m_nSteps-1]        += m_pCasesList[m_nSteps];
  m_pMeasureList[m_nSteps-1]      += m_pMeasureList[m_nSteps];
  m_pLastNeighborList[m_nSteps-1]  = m_pLastNeighborList[m_nSteps];

  m_pCasesList[m_nSteps]         = 0;
  m_pMeasureList[m_nSteps]       = 0;
  m_pFirstNeighborList[m_nSteps] = 0;
  m_pLastNeighborList[m_nSteps]  = 0;
}

void CPSMonotoneCluster::RemoveRemainder()
{
  m_nSteps--;
  m_pCasesList[m_nSteps]         = 0;
  m_pMeasureList[m_nSteps]       = 0;
  m_pFirstNeighborList[m_nSteps] = 0;
  m_pLastNeighborList[m_nSteps]  = 0;

  if (m_nSteps==0)
    Initialize(m_Center);
}

void CPSMonotoneCluster::AddRemainder(count_t nTotalCases, measure_t nTotalMeasure)
{
  m_nSteps++;

  m_pCasesList[m_nSteps-1]   = nTotalCases - m_nCases;
  m_pMeasureList[m_nSteps-1] = nTotalMeasure - m_nMeasure;
}

void CPSMonotoneCluster::SetCasesAndMeasures()
{
  m_nCases   = 0;
  m_nMeasure = 0;

  for (int i=0; i<m_nSteps; i++)
  {
    m_nCases   += m_pCasesList[i];
    m_nMeasure += m_pMeasureList[i];
  }
}

double CPSMonotoneCluster::SetRatio(double nLikelihoodForTotal)
{
  m_bRatioSet = true;
  m_nRatio    = m_nLogLikelihood - nLikelihoodForTotal;

  return m_nRatio;
}

/*double CPSMonotoneCluster::SetLogLikelihood()
{
  m_nLogLikelihood = 0;
  m_bLogLSet       = true;

  for (int i=0; i<m_nSteps; i++)
  {
    if (m_pCasesList[i] != 0)
      m_nLogLikelihood += m_pCasesList[i]*log(m_pCasesList[i]/m_pMeasureList[i]);
  }

  return m_nLogLikelihood;
}
*/
double CPSMonotoneCluster::GetRatio()
{
  if (m_bRatioSet)
    return m_nRatio;
  else
    return -1;
}

/*double CPSMonotoneCluster::GetLogLikelihood()
{
  if (m_bLogLSet)
    return m_nLogLikelihood;
  else
    return -1;
}
*/
void CPSMonotoneCluster::DefineTopCluster(const CSaTScanData& Data, count_t** pCases)
{
//  int i = 1;
//  AddNeighbor(Data, pCases, i);

//  while (i<=Data.m_NeighborCounts[m_Center] &&
//         m_nMeasure <= Data.m_nMaxCircleSize)
  for (int i=1; i<=Data.m_NeighborCounts[m_Center]; i++)
  {
    AddNeighbor(Data, pCases, i);
    CheckCircle(GetLastCircleIndex());
//    i++;
//    if (i<=Data.m_NeighborCounts[m_Center])
//      AddNeighbor(Data, pCases, i);
  }

  if (Data.m_nTotalCases != m_nCases)
  {
    AddRemainder(Data.m_nTotalCases, Data.m_nTotalMeasure);
    CheckCircle(GetLastCircleIndex());
  }

  m_nLogLikelihood = Data.m_pModel->CalcMonotoneLogLikelihood(*this);
  m_bLogLSet        = true;

  SetRatio(Data.m_pModel->GetLogLikelihoodForTotal());

  if (Data.m_nTotalCases != m_nCases)
    RemoveRemainder();

  // Recalc Total Cases, Measure, and Tracts to  account for
  // outer tracts absorbed into remainder
  SetCasesAndMeasures();
  SetTotalTracts();
}

void CPSMonotoneCluster::SetTotalTracts()
{
  m_nTracts = 0;
  for (int i=0; i<m_nSteps; i++)
    m_nTracts += m_pLastNeighborList[i]-m_pFirstNeighborList[i]+1;
}

void CPSMonotoneCluster::DisplayCensusTracts(FILE* fp, const CSaTScanData& Data,
                                             int nCluster, measure_t nMinMeasure,
                                             int nReplicas,
                                             bool bIncludeRelRisk, bool bIncludePVal,
                                             int nLeftMargin, int nRightMargin,
                                             char cDeliminator, char* szSpacesOnLeft)
{
//  fprintf(fp, "");
//KR  fprintf(fp, ".:\n");

  for (int i=0; i<m_nSteps; i++)
  {
    if (i>0)
      for (int j=1; j<=nLeftMargin-11; j++)
        fprintf(fp," ");

    if (nLeftMargin != 0)
      fprintf(fp, "- Step %i.: ",i+1);
//KR      fprintf(fp, "               - Step %i.: ",i+1);

    DisplayCensusTractsInStep(fp, Data, m_pFirstNeighborList[i],
                              m_pLastNeighborList[i], nCluster, nMinMeasure,
                              nReplicas, bIncludeRelRisk, bIncludePVal,
                              nLeftMargin, nRightMargin, cDeliminator, szSpacesOnLeft);
  }
}

void CPSMonotoneCluster::DisplayRelativeRisk(FILE* fp, double nMeasureAdjustment,
                                             int nLeftMargin, int nRightMargin,
                                             char cDeliminator, char* szSpacesOnLeft)
{
  //double nExpectedThisStep;

  CCluster::DisplayRelativeRisk(fp, nMeasureAdjustment, nLeftMargin, nRightMargin, cDeliminator, szSpacesOnLeft);

  if (m_nSteps == 1)
    return;

  fprintf(fp, "%sRelative risk by step.: ", szSpacesOnLeft);

  int   pos  = nLeftMargin;

  for (int i=0; i<m_nSteps; i++)
  {
    pos += 7;

    if (pos > nRightMargin)
    {
      pos = nLeftMargin;
      fprintf(fp, "\n");
      for (int j=0; j<nLeftMargin; j++)
        fprintf(fp, " ");
    }

    fprintf(fp, "%.3f", GetRelativeRisk(i, nMeasureAdjustment));

    if (i < m_nSteps-1)
      fprintf(fp, "%c ", cDeliminator);
  }

  fprintf(fp, "\n");
}

double CPSMonotoneCluster::GetRelativeRisk(tract_t nStep, double nMeasureAdjustment)
{
  return ((double)(m_pCasesList[nStep]))/(m_pMeasureList[nStep] * nMeasureAdjustment);
}

void CPSMonotoneCluster::DisplaySteps(FILE* fp, char* szSpacesOnLeft)
{
  fprintf(fp, "Steps in risk function: %i\n%s", m_nSteps, szSpacesOnLeft);
}

void CPSMonotoneCluster::DisplayCoordinates(FILE* fp, const CSaTScanData& Data,
                                            int nLeftMargin, int nRightMargin,
                                            char cDeliminator, char* szSpacesOnLeft)
{
  float *pCoords, *pCoords2;
  float nRadius;
  int   pos = nLeftMargin, i;
 
  giGetCoords(m_Center, &pCoords);
  //fprintf(fp, "  Coordinates...................: (%g,%g)\n",x1,y1);
  if(Data.m_pParameters->m_nDimension < 5)
  {
  	fprintf(fp, "%sCoordinates...........: (", szSpacesOnLeft);
  	for (i=0; i<(Data.m_pParameters->m_nDimension)-1; i++)
  	{
  		fprintf(fp, "%g,",pCoords[i]);
  	}
  	fprintf(fp, "%g)\n",pCoords[(Data.m_pParameters->m_nDimension)-1]);
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
  }
  fprintf(fp, "%sRadius for each step..: ", szSpacesOnLeft);

  for (i=0; i<m_nSteps; i++)
  {
    tiGetCoords(Data.GetNeighbor(m_Center, m_pLastNeighborList[i]), &pCoords2);
    nRadius = (float)sqrt(tiGetDistanceSq(pCoords, pCoords2));
    free(pCoords2);
    pos += 10;

    if (pos > nRightMargin)
    {
      pos = nLeftMargin;
      fprintf(fp, "\n");
      for (int j=0; j<nLeftMargin; j++)
        fprintf(fp, " ");
    }

    fprintf(fp,"%4.2f", nRadius);

    if (i < m_nSteps-1)
      fprintf(fp, "%c ", cDeliminator);

  }

  fprintf(fp, "\n");

  free(pCoords);
}

void CPSMonotoneCluster::DisplayLatLongCoords(FILE* fp, const CSaTScanData& Data,
                                  int nLeftMargin, int nRightMargin,
                                  char cDeliminator, char* szSpacesOnLeft)
{
  float *pCoords, *pCoords2;
  float nRadius;
  int   pos = nLeftMargin;
  float Latitude, Longitude;
  char  cNorthSouth, cEastWest;

  giGetCoords(m_Center, &pCoords);
  tiGetCoords(Data.GetNeighbor(m_Center, m_nTracts), &pCoords2);

  nRadius = (float)sqrt(tiGetDistanceSq(pCoords, pCoords2));

  ConvertToLatLong(&Latitude, &Longitude, pCoords);

  Latitude >= 0 ? cNorthSouth = 'N' : cNorthSouth = 'S';
  Longitude >= 0 ? cEastWest = 'W' : cEastWest = 'E';

  fprintf(fp, "%sCoordinates...........: (%.3f %c, %.3f %c)\n",
               szSpacesOnLeft, fabs(Latitude), cNorthSouth, fabs(Longitude), cEastWest, nRadius);
  fprintf(fp, "%sRadius for each step..: ", szSpacesOnLeft);

  for (int i=0; i<m_nSteps; i++)
  {
    tiGetCoords(Data.GetNeighbor(m_Center, m_pLastNeighborList[i]), &pCoords2);
    nRadius = (float)sqrt(tiGetDistanceSq(pCoords, pCoords2));
    free(pCoords2);
    pos += 10;

    if (pos > nRightMargin)
    {
      pos = nLeftMargin;
      fprintf(fp, "\n");
      for (int j=0; j<nLeftMargin; j++)
        fprintf(fp, " ");
    }

    fprintf(fp,"%5.2f", nRadius);

    if (i < m_nSteps-1)
      fprintf(fp, "%c ", cDeliminator);

  }

  fprintf(fp, "\n");

  free(pCoords);
}





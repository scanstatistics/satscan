// STCluster.cpp

#include "STCluster.h"
#include "Param.h"
#include "TIAll.h"
#include "TIAlive.h"

CSpaceTimeCluster::CSpaceTimeCluster(int nTIType, int nIntervals, int nIntervalCut)
                  :CCluster()
{
  switch (nTIType)
  {
    case (ALLCLUSTERS)   : TI = new CTIAll(nIntervals, nIntervalCut);   break;
    case (ALIVECLUSTERS) : TI = new CTIAlive(nIntervals, nIntervalCut); break;
    default              : break;
  }

  // need to keep both of these?
  m_nTotalIntervals = nIntervals;
  m_nIntervalCut    = nIntervalCut;

  m_pCumCases   = (count_t*) Smalloc((m_nTotalIntervals)*sizeof(count_t));
  m_pCumMeasure = (measure_t*) Smalloc((m_nTotalIntervals)*sizeof(measure_t));

  Initialize(0);
}

CSpaceTimeCluster::~CSpaceTimeCluster()
{
  delete TI;

  if (m_pCumCases != NULL)
    free(m_pCumCases);
  if (m_pCumMeasure != NULL)
    free(m_pCumMeasure);
}

void CSpaceTimeCluster::Initialize(tract_t nCenter = 0)
{
  CCluster::Initialize(nCenter);

  m_nClusterType = SPACETIME;

  for (int i=0; i<m_nTotalIntervals; i++)
  {
    m_pCumCases[i]   = 0;
    m_pCumMeasure[i] = 0;
  }

}

void CSpaceTimeCluster::DeAllocCumulativeCounts()
{
  if (m_pCumCases != NULL)
  {
    free(m_pCumCases);
    m_pCumCases = NULL;
  }

  if (m_pCumMeasure != NULL)
  {
    free(m_pCumMeasure);
    m_pCumMeasure = NULL;
  }
}

CSpaceTimeCluster& CSpaceTimeCluster::operator =(const CSpaceTimeCluster& cluster)
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

  m_nTotalIntervals = cluster.m_nTotalIntervals;
  m_nIntervalCut    = cluster.m_nIntervalCut;

  for (int i = 0; i<m_nTotalIntervals; i++)
  {
    m_pCumCases[i]   = cluster.m_pCumCases[i];
    m_pCumMeasure[i] = cluster.m_pCumMeasure[i];
  }

  m_nSteps           = cluster.m_nSteps;

  m_bClusterInit   = cluster.m_bClusterInit;
  m_bClusterDefined= cluster.m_bClusterDefined;
  m_bClusterSet    = cluster.m_bClusterSet;
  m_bLogLSet       = cluster.m_bLogLSet;
  m_bRatioSet      = cluster.m_bRatioSet;
  m_nClusterType   = cluster.m_nClusterType;

  return *this;
}

void CSpaceTimeCluster::AddNeighbor(const CSaTScanData& Data, count_t** pCases, tract_t n)
{
  m_nTracts = n;
  tract_t nNeighbor = Data.GetNeighbor(m_Center, n);

  for (int i=0; i<m_nTotalIntervals; i++)
  {
    m_pCumCases[i]   += pCases[i][nNeighbor];
    m_pCumMeasure[i] += Data.m_pMeasure[i][nNeighbor];
  }

}

void CSpaceTimeCluster::InitTimeIntervalIndeces()
{
  TI->Initialize();
}

bool CSpaceTimeCluster::SetNextTimeInterval()
{
  m_bClusterDefined = true;
  return(TI->GetNextTimeInterval(m_pCumCases,
                                 m_pCumMeasure,
                                 m_nCases,
                                 m_nMeasure,
                                 m_nFirstInterval,
                                 m_nLastInterval));
}

/*void CSpaceTimeCluster::Display(FILE* pFile)
{
  fprintf(pFile, "\nCenter Grid Point #%i\n", m_Center);
  fprintf(pFile, "Cases=%i, Measure=%f, Tracts=%i, Ratio=%f, Rank=%i\n",
          m_nCases, m_nMeasure, m_nTracts, m_nRatio, m_nRank);
  fprintf(pFile, "Start Interval=%i, Last Interval=%i\n\n",
          m_nFirstInterval, m_nLastInterval);
}
*/

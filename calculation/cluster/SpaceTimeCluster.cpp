#include "SaTScan.h"
#pragma hdrstop
#include "SpaceTimeCluster.h"

CSpaceTimeCluster::CSpaceTimeCluster(int nTIType, int nIntervals, int nIntervalCut, BasePrint *pPrintDirection)
                  :CCluster(pPrintDirection)
{
   try
      {
      TI = 0;
      m_pCumCases = 0;
      m_pCumMeasure = 0;

      switch (nTIType)
      {
        case (ALLCLUSTERS)   : TI = new CTIAll(nIntervals, nIntervalCut);   break;
        case (ALIVECLUSTERS) : TI = new CTIAlive(nIntervals, nIntervalCut); break;
        default              : break;
      }

      // need to keep both of these?
      m_nTotalIntervals = nIntervals;
      m_nIntervalCut    = nIntervalCut;

      m_pCumCases   = (count_t*) Smalloc((m_nTotalIntervals)*sizeof(count_t), gpPrintDirection);
      m_pCumMeasure = (measure_t*) Smalloc((m_nTotalIntervals)*sizeof(measure_t), gpPrintDirection);

      Initialize(0);
      }
   catch (SSException & x)
      {
      x.AddCallpath("CSpaceTimeCluster()", "CSpaceTimeCluster");
      throw;
      }
}

CSpaceTimeCluster::~CSpaceTimeCluster()
{
  try
     {
     delete TI;
     DeAllocCumulativeCounts();
     }
  catch(...){}
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

  memcpy(m_pCumCases, cluster.m_pCumCases, m_nTotalIntervals*sizeof(count_t));
  memcpy(m_pCumMeasure, cluster.m_pCumMeasure, m_nTotalIntervals*sizeof(measure_t));

  m_nSteps           = cluster.m_nSteps;

  m_bClusterInit   = cluster.m_bClusterInit;
  m_bClusterDefined= cluster.m_bClusterDefined;
  m_bClusterSet    = cluster.m_bClusterSet;
  m_bLogLSet       = cluster.m_bLogLSet;
  m_bRatioSet      = cluster.m_bRatioSet;
  m_nClusterType   = cluster.m_nClusterType;

  m_iEllipseOffset = cluster.m_iEllipseOffset;

  *TI = *(cluster.TI);

  return *this;
}

void CSpaceTimeCluster::AddNeighbor(int iEllipse, const CSaTScanData& Data, count_t** pCases, tract_t n)
{
   int i;

   try
      {
      m_nTracts = n;
      tract_t nNeighbor = Data.GetNeighbor(iEllipse, m_Center, n);

      for (i=0; i<m_nTotalIntervals; i++)
        {
        m_pCumCases[i]   += pCases[i][nNeighbor];
        m_pCumMeasure[i] += Data.m_pMeasure[i][nNeighbor];
        }
      }
   catch (SSException & x)
      {
      x.AddCallpath("AddNeighbor()", "CSpaceTimeCluster");
      throw;
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

/** Returns the number of case for tract as defined by cluster. */
count_t CSpaceTimeCluster::GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data) const
{
  return TI->GetCaseCountForTract(tTract, Data.m_pCases);
}

void CSpaceTimeCluster::GetMeasure()
{
     TI->GetNextTimeIntervalProsp(m_pCumCases,
                                 m_pCumMeasure,
                                 m_nCases,
                                 m_nMeasure);
}

/** Returns the measure for tract as defined by cluster. */
measure_t CSpaceTimeCluster::GetMeasureForTract(tract_t tTract, const CSaTScanData& Data) const
{
  return TI->GetMeasureForTract(tTract, Data.m_pMeasure);
}

void CSpaceTimeCluster::Initialize(tract_t nCenter = 0)
{
   try
      {
      CCluster::Initialize(nCenter);

      m_nClusterType = SPACETIME;

      memset(m_pCumCases, 0, sizeof(count_t) * m_nTotalIntervals);
      memset(m_pCumMeasure, 0, sizeof(measure_t) * m_nTotalIntervals);
      }
   catch (SSException & x)
      {
      x.AddCallpath("Initialize()", "CSpaceTimeCluster");
      throw;
      }
}

void CSpaceTimeCluster::InitTimeIntervalIndeces()
{
   try
      {
      TI->Initialize();
      }
   catch (SSException & x)
      {
      x.AddCallpath("InitTimeIntervalIndeces()", "CSpaceTimeCluster");
      throw;
      }
}

void CSpaceTimeCluster::InitTimeIntervalIndeces(int nLow, int nHigh)
{
   try
      {
      TI->InitializeRange(nLow, nHigh);
      }
   catch (SSException & x)
      {
      x.AddCallpath("InitTimeIntervalIndeces()", "CSpaceTimeCluster");
      throw;
      }
}

bool CSpaceTimeCluster::SetNextProspTimeInterval()
{
   bool bRetVal;

   try
      {
      m_bClusterDefined = true;
      bRetVal = (TI->GetNextTimeIntervalProsp(m_pCumCases,
                                 m_pCumMeasure,
                                 m_nCases,
                                 m_nMeasure));
      }
   catch (SSException & x)
      {
      x.AddCallpath("SetNextProspTimeInterval()", "CSpaceTimeCluster");
      throw;
      }
   return bRetVal;
}

bool CSpaceTimeCluster::SetNextTimeInterval()
{
   bool bRetVal;

   m_bClusterDefined = true;
   bRetVal = (TI->GetNextTimeInterval(m_pCumCases,
                                 m_pCumMeasure,
                                 m_nCases,
                                 m_nMeasure,
                                 m_nFirstInterval,
                                 m_nLastInterval));
   return bRetVal;
}
//------------------------------------------------------------------------------
/*void CSpaceTimeCluster::Display(FILE* pFile)
{
  fprintf(pFile, "\nCenter Grid Point #%i\n", m_Center);
  fprintf(pFile, "Cases=%i, Measure=%f, Tracts=%i, Ratio=%f, Rank=%i\n",
          m_nCases, m_nMeasure, m_nTracts, m_nRatio, m_nRank);
  fprintf(pFile, "Start Interval=%i, Last Interval=%i\n\n",
          m_nFirstInterval, m_nLastInterval);
}
*/

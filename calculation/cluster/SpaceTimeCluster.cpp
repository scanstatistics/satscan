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
  delete TI;

  if (m_pCumCases != NULL)
    free(m_pCumCases);
  if (m_pCumMeasure != NULL)
    free(m_pCumMeasure);
}

void CSpaceTimeCluster::Initialize(tract_t nCenter = 0)
{
   try
      {
      CCluster::Initialize(nCenter);
    
      m_nClusterType = SPACETIME;
    
      //for (int i=0; i<m_nTotalIntervals; i++)
      //  {
      //  m_pCumCases[i]   = 0;
      //  m_pCumMeasure[i] = 0;
      //  }
      memset(m_pCumCases, 0, sizeof(count_t) * m_nTotalIntervals);
      memset(m_pCumMeasure, 0, sizeof(measure_t) * m_nTotalIntervals);
      }
   catch (SSException & x)
      {
      x.AddCallpath("Initialize()", "CSpaceTimeCluster");
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

void CSpaceTimeCluster::GetMeasure()
{
     TI->GetNextTimeIntervalProsp(m_pCumCases,
                                 m_pCumMeasure,
                                 m_nCases,
                                 m_nMeasure);
     /*TI->GetNextTimeIntervalProsp(m_pCumCases,
                                 m_pCumMeasure,
                                 m_nCases,
                                 m_nMeasure,
                                 m_nFirstInterval,
                                 m_nLastInterval); */
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
      /*bRetVal = (TI->GetNextTimeIntervalProsp(m_pCumCases,
                                 m_pCumMeasure,
                                 m_nCases,
                                 m_nMeasure,
                                 m_nFirstInterval,
                                 m_nLastInterval)); */
      }
   catch (SSException & x)
      {
      x.AddCallpath("SetNextProspTimeInterval()", "CSpaceTimeCluster");
      throw;
      }
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

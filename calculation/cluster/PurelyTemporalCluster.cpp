#pragma hdrstop
#include "PurelyTemporalCluster.h"

CPurelyTemporalCluster::CPurelyTemporalCluster(int nTIType, int nIntervals, int nIntervalCut, BasePrint *pPrintDirection)
                  :CCluster(pPrintDirection)
{
   try
      {
      m_TI = 0;
      switch (nTIType)
         {
         case (ALLCLUSTERS)   : m_TI = new CTIAll(nIntervals, nIntervalCut);   break;
         case (ALIVECLUSTERS) : m_TI = new CTIAlive(nIntervals, nIntervalCut); break;
         default              : break;
         }
      m_nTotalIntervals = nIntervals;
      m_nIntervalCut    = nIntervalCut;
      Initialize(0);
      }
   catch (SSException & x)
      {
      x.AddCallpath("CPurelyTemporalCluster()", "CPurelyTemporalCluster");
      throw;
      }
}

CPurelyTemporalCluster::~CPurelyTemporalCluster()
{
   delete m_TI;
}

void CPurelyTemporalCluster::Initialize(tract_t nCenter)
{
   CCluster::Initialize(nCenter);
   m_nClusterType = PURELYTEMPORAL;
}

CPurelyTemporalCluster& CPurelyTemporalCluster::operator =(const CPurelyTemporalCluster& cluster)
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

  m_nSteps          = cluster.m_nSteps;

  m_bClusterInit   = cluster.m_bClusterInit;
  m_bClusterDefined= cluster.m_bClusterDefined;
  m_bClusterSet    = cluster.m_bClusterSet;
  m_bLogLSet       = cluster.m_bLogLSet;
  m_bRatioSet      = cluster.m_bRatioSet;
  m_nClusterType   = cluster.m_nClusterType;

//  *m_TI            = *(cluster.m_TI);

  return *this;
}

void CPurelyTemporalCluster::InitTimeIntervalIndeces()
{
   try
      {
      m_TI->Initialize();
      }
   catch (SSException & x)
      {
      x.AddCallpath("InitTimeIntervalIndeces()", "CPurelyTemporalCluster");
      throw;
      }
}

bool CPurelyTemporalCluster::SetNextTimeInterval(const count_t*& pCases,
                                                 const measure_t*& pMeasure)
{
   bool bValue;

  m_bClusterDefined = true;
  bValue = (m_TI->GetNextTimeInterval(pCases,
                                   pMeasure,
                                   m_nCases,
                                   m_nMeasure,
                                   m_nFirstInterval,
                                   m_nLastInterval));
   return bValue;
}

void CPurelyTemporalCluster::DisplayCensusTracts(FILE* fp, const CSaTScanData& Data,
                                                 int nCluster,  measure_t nMinMeasure,
                                                 int nReplicas,
                                                 bool bIncludeRelRisk, bool bIncludePVal,
                                                 int nLeftMargin, int nRightMargin,
                                                 char cDeliminator, char* szSpacesOnLeft,
                                                 bool bFormat)
{
   //char* szTID;
   try
      {
      if (nLeftMargin > 0)
         fprintf(fp, "included.: All\n");
      // Code to print tracts for purely temporal clusters
      /*else
        {
        for (int i=0; i<Data.m_nTracts; i++)
           {
           szTID = tiGetTid(i);
           fprintf(fp, "%i         ", nCluster);
           fprintf(fp, "%s\n", szTID);
           }
        }
        */
      }
   catch (SSException & x)
      {
      x.AddCallpath("DisplayCensusTracts()", "CPurelyTemporalCluster");
      throw;
      }
}



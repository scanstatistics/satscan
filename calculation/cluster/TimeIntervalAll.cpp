#include "SaTScan.h"
#pragma hdrstop
#include "TimeIntervalAll.h"
#include "Cluster.h"

/** constructor */
CTIAll::CTIAll(int nTotal, int nCut) : CTimeIntervals(nTotal, nCut) {}

/** copy constructor */
CTIAll::CTIAll(const CTIAll& rhs) : CTimeIntervals(rhs) {}

/** returns newly cloned CTIAll */
CTIAll * CTIAll::Clone() const {
  return new CTIAll(*this);
}

/** Returns the number of cases that tract attributed to accumulated case count. */
count_t CTIAll::GetCaseCountForTract(const CCluster & Cluster, tract_t tTract, count_t** pCases) const
{
   count_t      tCaseCount;

   if (Cluster.m_nLastInterval == giNumIntervals)
     tCaseCount = pCases[Cluster.m_nFirstInterval][tTract];
   else
     tCaseCount  = pCases[Cluster.m_nFirstInterval][tTract] - pCases[Cluster.m_nLastInterval][tTract];

   return tCaseCount;
}

/** Returns the measure that tract attributed to accumulated measure. */
measure_t CTIAll::GetMeasureForTract(const CCluster & Cluster, tract_t tTract, measure_t** pMeasure) const
{
   measure_t      tMeasure;

   if (Cluster.m_nLastInterval == giNumIntervals)
     tMeasure = pMeasure[Cluster.m_nFirstInterval][tTract];
   else
     tMeasure  = pMeasure[Cluster.m_nFirstInterval][tTract] - pMeasure[Cluster.m_nLastInterval][tTract];

   return tMeasure;
}


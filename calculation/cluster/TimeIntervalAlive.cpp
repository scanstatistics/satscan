#include "SaTScan.h"
#pragma hdrstop
#include "TimeIntervalAlive.h"

CTIAlive::CTIAlive(int nTotal, int nCut)
         :CTimeIntervals(nTotal, nCut)
{
}

void CTIAlive::Initialize()
{
   //  m_nStart = m_nCut-1;
   //  m_nStop  = m_nTotal-1;
   m_nStart = m_nTotal-m_nCut-1;
   m_nStop  = m_nTotal;
}

/** Returns the number of cases that tract attributed to accumulated case count. */
count_t CTIAlive::GetCaseCountForTract(tract_t tTract, count_t** pCases) const
{
   count_t      tCaseCount;

   if (m_nStart == m_nStop)
     tCaseCount = 0;
   else
     tCaseCount = pCases[m_nStart][tTract];

   return tCaseCount;
}

/** Returns the measure that tract attributed to accumulated measure. */
measure_t CTIAlive::GetMeasureForTract(tract_t tTract, measure_t** pMeasure) const
{
   measure_t      tMeasure;

   if (m_nStart == m_nStop)
     tMeasure = 0;
   else
     tMeasure = pMeasure[m_nStart][tTract];

   return tMeasure;
}

bool CTIAlive::GetNextTimeInterval(const count_t* pCases,
                                   const measure_t* pMeasure,
                                   count_t& nCases,
                                   measure_t& nMeasure,
                                   int& nStart,
                                   int& nStop)
{
      m_nStart++;
      if (m_nStart == m_nStop)
         {
         nCases = -1;
         nMeasure = -1;
         nStart = -1;
         nStop = -1;
         return false;
         }

      //  m_bClusterSet = true;
      nCases   = pCases[m_nStart];
      nMeasure = pMeasure[m_nStart];
      nStart   = m_nStart;
      nStop    = m_nStop;
  return true;
}

/*bool CTIAlive::GetNextTimeIntervalProsp(const count_t*& pCases,
                                   const measure_t*& pMeasure,
                                   count_t& nCases,
                                   measure_t& nMeasure,
                                   int& nStart,
                                   int& nStop) */
bool CTIAlive::GetNextTimeIntervalProsp(const count_t* pCases,
                                   const measure_t* pMeasure,
                                   count_t& nCases,
                                   measure_t& nMeasure)
{
  return false;
}

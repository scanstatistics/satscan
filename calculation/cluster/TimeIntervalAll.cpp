#include "SaTScan.h"
#pragma hdrstop
#include "TimeIntervalAll.h"

CTIAll::CTIAll(int nTotal, int nCut)
         :CTimeIntervals(nTotal, nCut)
{
}

void CTIAll::Initialize()
{
   m_nStart = -1;
   m_nStop  = 1;
}

void CTIAll::InitializeRange(int nLow, int nHigh)
{
   m_nStart = nLow - 1;
   m_nStop  = nHigh;
}

/** Returns the number of cases that tract attributed to accumulated case count. */
count_t CTIAll::GetCaseCountForTract(tract_t tTract, count_t** pCases) const
{
   count_t      tCaseCount;

   if (m_nStop == m_nTotal)
     tCaseCount = pCases[m_nStart][tTract];
   else
     tCaseCount  = pCases[m_nStart][tTract] - pCases[m_nStop][tTract];

   return tCaseCount;
}

/** Returns the measure that tract attributed to accumulated measure. */
measure_t CTIAll::GetMeasureForTract(tract_t tTract, measure_t** pMeasure) const
{
   measure_t      tMeasure;

   if (m_nStop == m_nTotal)
     tMeasure = pMeasure[m_nStart][tTract];
   else
     tMeasure  = pMeasure[m_nStart][tTract] - pMeasure[m_nStop][tTract];

   return tMeasure;
}

bool CTIAll::GetNextTimeInterval(const count_t*& pCases,
                                   const measure_t*& pMeasure,
                                   count_t& nCases,
                                   measure_t& nMeasure,
                                   int& nStart,
                                   int& nStop)
{

      if (m_nStart+1 == m_nStop)
         {
         if (m_nStop == m_nTotal)
            {
            nCases = -1;
            nMeasure = -1;
            nStart = -1;
            nStop = -1;
            return false;
            }
         m_nStop++;
         m_nStart = (m_nStop > m_nCut ? m_nStop-m_nCut : 0);
         }
      else
         m_nStart++;

      if (m_nStop == m_nTotal)
      {
        nCases   = pCases[m_nStart];
        nMeasure = pMeasure[m_nStart];
      }
      else
      {
        nCases   = pCases[m_nStart]-pCases[m_nStop];
        nMeasure = pMeasure[m_nStart]-pMeasure[m_nStop];
      }

      nStart   = m_nStart;
      nStop    = m_nStop;

  return true;
}


bool CTIAll::GetNextTimeIntervalProsp(const count_t*& pCases,
                                   const measure_t*& pMeasure,
                                   count_t& nCases,
                                   measure_t& nMeasure)
{
      //**********************************************************************
      // DID NOT KNOW WHEN TO STOP EVALUATING INTERVALS...
      //**********************************************************************
      if (m_nStart+1 == m_nStop)
         return false;
      m_nStart++;

      if (m_nStop == m_nTotal)
        {
        nCases   = pCases[m_nStart];
        nMeasure = pMeasure[m_nStart];
        }
      else
        {
        nCases   = pCases[m_nStart]-pCases[m_nStop];
        nMeasure = pMeasure[m_nStart]-pMeasure[m_nStop];
        }
  return true;
}

/*bool CTIAll::GetNextTimeIntervalProsp(const count_t*& pCases,
                                   const measure_t*& pMeasure,
                                   count_t& nCases,
                                   measure_t& nMeasure,
                                   int& nStart,
                                   int& nStop)
{
   try
      {
      //**********************************************************************
      // DID NOT KNOW WHEN TO STOP EVALUATING INTERVALS...
      //**********************************************************************
      if (m_nStart+1 == m_nStop)
         return false;
      m_nStart++;

      if (m_nStop == m_nTotal)
      {
        nCases   = pCases[m_nStart];
        nMeasure = pMeasure[m_nStart];
      }
      else
      {
        nCases   = pCases[m_nStart]-pCases[m_nStop];
        nMeasure = pMeasure[m_nStart]-pMeasure[m_nStop];
      }

      nStart   = m_nStart;
      nStop    = m_nStop;
      }
   catch (SSException & x)
      {
      x.AddCallpath("GetNextTimeIntervalProsp()", "CTIAll");
      throw;
      }
  return true;
} */




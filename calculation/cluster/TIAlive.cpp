// TIAlive.cpp

#include "TIAlive.h"

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

bool CTIAlive::GetNextTimeInterval(const count_t*& pCases,
                                   const measure_t*& pMeasure,
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


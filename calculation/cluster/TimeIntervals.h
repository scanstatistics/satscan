//*****************************************************************************
#ifndef __TIMEINTERVALS_H
#define __TIMEINTERVALS_H
//*****************************************************************************
#include "SaTScan.h"

class CTimeIntervals
{
  public:
    CTimeIntervals(int nTotal, int nCut) {m_nTotal=nTotal; m_nCut=nCut;};
    virtual ~CTimeIntervals() {};

    virtual void        Initialize() {};
    virtual void        InitializeRange(int nLow, int nHigh) { };
    virtual count_t     GetCaseCountForTract(tract_t tTract, count_t** pCases) const = 0;
    virtual measure_t   GetMeasureForTract(tract_t tTract,  measure_t** pMeasure) const = 0;
    virtual bool        GetNextTimeInterval(const count_t* pCases, const measure_t* pMeasure,
                                            count_t& nCases, measure_t& nMeasure,
                                            int& nStart, int& nStop)
                                            {return false;}
    virtual bool GetNextTimeIntervalProsp(const count_t* pCases, const measure_t* pMeasure,
                                          count_t& nCases, measure_t& nMeasure) {return false;};
    /*virtual bool GetNextTimeIntervalProsp(const count_t*& pCases,
                                     const measure_t*& pMeasure,
                                     count_t& nCases,
                                     measure_t& nMeasure,
                                     int& nStart,
                                     int& nStop) {return false;};*/
  protected:
    int m_nStart;
    int m_nStop;
    int m_nTotal;
    int m_nCut;
};
#endif

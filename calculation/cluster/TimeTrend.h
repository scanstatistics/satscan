//*****************************************************************************
#ifndef __TIMETREND_H
#define __TIMETREND_H
//*****************************************************************************
#include "SaTScan.h"

#define TREND_ZERO 0.00000001
#define TREND_OK 0
#define TREND_INF 1
#define TREND_UNDEF 2

class CTimeTrend
{
  public:
    CTimeTrend();
    virtual ~CTimeTrend();

    CTimeTrend& operator =(const CTimeTrend& line);

    void Initialize();

    virtual void CalculateAndSet(count_t*   pCases,
                                 measure_t* pMeasure,
                                 int        nTimeIntervals,
                                 double     nConverge);

    double Alpha(double nSC, double nSME) const;
    double F(double nSC, double nSTC, double nSME, double nSTME) const;
    double S(double nSC, double nSTC, double nSTME, double nST2ME) const;
    double Alpha(count_t nCases,
                 measure_t* pMeasure,
                 int nTimeIntervals,
                 double nBeta) const;

    double SetAnnualTimeTrend(int nIntervalUnits, double nIntervalLen);
    double GetAnnualTimeTrend() {return m_nAnnualTT;};
    void SetTrendStatus();

    bool   IsNegative();

    double m_nAlpha;
    double m_nBeta;
    int    m_nStatus;

  private:
    double m_nAnnualTT;
    double m_nSC;
    double m_nSTC;
    double m_nSME;
    double m_nSTME;
    double m_nST2ME;

};

#endif

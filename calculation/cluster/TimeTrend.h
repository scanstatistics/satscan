//*****************************************************************************
#ifndef __TIMETREND_H
#define __TIMETREND_H
//*****************************************************************************
#include "SaTScan.h"

/** Calculates temporal trend. */
class CTimeTrend {
  public:
    enum Status                 {TREND_UNDEF=0,        /** trend undefined */
                                 TREND_INF,            /** trend is infinite */
                                 TREND_NOTCONVERGED,   /** trend did not converge */
                                 TREND_NEGATIVE,       /** negative beta calculated */
                                 TREND_CONVERGED       /** trend converged */
                                 };

  private:
    double                      gdAnnualTimeTrend;
    double                      gdAlpha;
    double                      gdBeta;
    Status                      gStatus;
    static const double         TREND_ZERO;
    static const unsigned int   MAX_BETA_TEST_ITERATIONS;
    static const unsigned int   MAX_CONVERGENCE_ITERATIONS;

    double                      Alpha(double nSC, double nSME) const;
    double                      F(double nSC, double nSTC, double nSME, double nSTME) const;
    double                      S(double nSC, double nSTC, double nSTME, double nST2ME) const;

  public:
    CTimeTrend();
    virtual ~CTimeTrend();

    double                      Alpha(count_t nCases, const measure_t* pMeasure, int nTimeIntervals, double nBeta) const;
    virtual Status              CalculateAndSet(const count_t* pCases, const measure_t* pMeasure, int nTimeIntervals, double nConverge);
    double                      GetAlpha() const {return gdAlpha;}
    double                      GetAnnualTimeTrend() const {return gdAnnualTimeTrend;}
    double                      GetBeta() const {return gdBeta;}
    Status                      GetStatus() const {return gStatus;}
    void                        Initialize();
    bool                        IsNegative() const;
    double                      SetAnnualTimeTrend(DatePrecisionType eDatePrecision, double nIntervalLen);
};
//*****************************************************************************
#endif

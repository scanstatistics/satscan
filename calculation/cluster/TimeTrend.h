//*****************************************************************************
#ifndef __TIMETREND_H
#define __TIMETREND_H
//*****************************************************************************
#include "SaTScan.h"

/** Calculates temporal trend. */
class CTimeTrend {
  public:
    enum Status                 {UNDEFINED=0,        /** trend undefined */
                                 NEGATIVE_INFINITY,  /** trend is infinite */
                                 POSITIVE_INFINITY,  /** trend is infinite */
                                 NOT_CONVERGED,      /** trend did not converge */
                                 CONVERGED           /** trend converged */
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

    static const double         NEGATIVE_INFINITY_INDICATOR;
    static const double         POSITIVE_INFINITY_INDICATOR;

    double                      Alpha(count_t nCases, const measure_t* pMeasure, int nTimeIntervals, double nBeta) const;
    virtual Status              CalculateAndSet(const count_t* pCases, const measure_t* pMeasure, int nTimeIntervals, double nConverge);
    double                      GetAlpha() const {return gdAlpha;}
    double                      GetAnnualTimeTrend() const {return gdAnnualTimeTrend;}
    double                      GetBeta() const {return gdBeta;}
    Status                      GetStatus() const {return gStatus;}
    void                        Initialize();
    double                      SetAnnualTimeTrend(DatePrecisionType eAggregationPrecision, double dTimeAggregationLength);
};
//*****************************************************************************
#endif

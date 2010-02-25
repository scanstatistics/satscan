//*****************************************************************************
#ifndef __TIMETREND_H
#define __TIMETREND_H
//*****************************************************************************
#include "SaTScan.h"
#include "Parameters.h"

/** Calculates temporal trend. */
class AbstractTimeTrend {
  public:
    enum Status                 {UNDEFINED=0,        /** trend undefined */
                                 NEGATIVE_INFINITY,  /** trend is infinite */
                                 POSITIVE_INFINITY,  /** trend is infinite */
                                 NOT_CONVERGED,      /** trend did not converge */
                                 CONVERGED,          /** trend converged */
                                 SINGULAR_MATRIX     /** singular matrix detected */
                                 };

  protected:
    double                      gdAnnualTimeTrend;
    double                      gdAlpha;
    double                      gdBeta;
    mutable double              gbGlobalAlpha;
    Status                      gStatus;
    static const double         TREND_ZERO;
    static const unsigned int   MAX_BETA_TEST_ITERATIONS;
    static const unsigned int   MAX_CONVERGENCE_ITERATIONS;

  public:
    AbstractTimeTrend();
    virtual ~AbstractTimeTrend();

    static const double         NEGATIVE_INFINITY_INDICATOR;
    static const double         POSITIVE_INFINITY_INDICATOR;

    virtual AbstractTimeTrend * clone() const = 0;

    virtual Status              CalculateAndSet(const count_t* pCases, const measure_t* pMeasure, int nTimeIntervals, double nConverge) = 0;
    double                      GetAnnualTimeTrend() const {return gdAnnualTimeTrend;}
    double                      GetAlpha() const {return gdAlpha;}
    double                      GetBeta() const {return gdBeta;}
    double                      GetGlobalAlpha() const {return gbGlobalAlpha;}
    Status                      GetStatus() const {return gStatus;}
    virtual TimeTrendType       getType() const = 0;
    virtual void                Initialize();
    double                      SetAnnualTimeTrend(DatePrecisionType eAggregationPrecision, double dTimeAggregationLength);

	static AbstractTimeTrend *  getTimeTrend(const CParameters& parameters);
};

/** Calculates linear time trend. */
class LinearTimeTrend : public AbstractTimeTrend {
  private:
    double                      Alpha(double nSC, double nSME) const;
    double                      F(double nSC, double nSTC, double nSME, double nSTME) const;
    double                      S(double nSC, double nSTC, double nSTME, double nST2ME) const;

  public:
    LinearTimeTrend();
    virtual ~LinearTimeTrend();

    virtual LinearTimeTrend   * clone() const {return new LinearTimeTrend(*this);}

    double                      Alpha(count_t nCases, const measure_t* pMeasure, int nTimeIntervals, double nBeta) const;
    virtual Status              CalculateAndSet(const count_t* pCases, const measure_t* pMeasure, int nTimeIntervals, double nConverge);
    virtual TimeTrendType       getType() const {return LINEAR;};
};


/** Calculates quadratic time trend. */
class QuadraticTimeTrend : public AbstractTimeTrend {
  private:
    double                      gdBeta2;

  public:
    QuadraticTimeTrend();
    virtual ~QuadraticTimeTrend();

    virtual QuadraticTimeTrend* clone() const {return new QuadraticTimeTrend(*this);}

    double                      Alpha(count_t nCases, const measure_t* pMeasure, int nTimeIntervals, double nBeta, double nBeta2) const;
    virtual Status              CalculateAndSet(const count_t* pCases, const measure_t* pMeasure, int nTimeIntervals, double nConverge=0.00001);
    double                      GetBeta2() const {return gdBeta2;}
    virtual TimeTrendType       getType() const {return QUADRATIC;};
    virtual void                Initialize();
};
//*****************************************************************************
#endif

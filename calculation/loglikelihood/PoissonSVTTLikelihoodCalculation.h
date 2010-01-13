//******************************************************************************
#ifndef PoissonSVTTLikelihoodCalculationH
#define PoissonSVTTLikelihoodCalculationH
//******************************************************************************
#include "LikelihoodCalculation.h"
#include "Parameters.h"
#include "TimeTrend.h"

typedef bool (*SVTT_TREND_FUNCPTR) (const AbstractTimeTrend&, const AbstractTimeTrend&);

class AbstractPoissonSVTTLikelihoodCalculator : public AbstractLikelihoodCalculator {
  protected:
    const CParameters & gParameters;   /** const reference to CParameters object */
    std::vector<double> gvDataSetLogLikelihoodUnderNull;
    double              CalcLogLikelihoodSpatialOnly(count_t n, measure_t u) const;

  public:
    AbstractPoissonSVTTLikelihoodCalculator(const CSaTScanData& DataHub);
    virtual ~AbstractPoissonSVTTLikelihoodCalculator();

    virtual double      CalcSVTTLogLikelihood(size_t tSetIndex, SVTTClusterData& ClusterData, const AbstractTimeTrend& GlobalTimeTrend) const = 0;
    virtual double      CalculateFullStatistic(double dMaximizingValue, size_t tDataSetIndex=0) const;
    //virtual double      CalculateMaximizingValue(count_t n, measure_t u, size_t tDataSetIndex=0) const;
    virtual double      GetLogLikelihoodForTotal(size_t tSetIndex=0) const;
};

/** Poisson log likelihood calculator. */
class PoissonSVTTLikelihoodCalculator : public AbstractPoissonSVTTLikelihoodCalculator {
  private:
    SVTT_TREND_FUNCPTR  gpTrendOfInterest;
    double              CalcLogLikelihood(const count_t* pCases, const measure_t* pMeasure,
                                          count_t pTotalCases, double nAlpha,
                                          double nBeta, int nStatus) const;
  public:
    PoissonSVTTLikelihoodCalculator(const CSaTScanData& DataHub);
    virtual ~PoissonSVTTLikelihoodCalculator();

    virtual double      CalcSVTTLogLikelihood(size_t tSetIndex, SVTTClusterData& ClusterData, const AbstractTimeTrend& GlobalTimeTrend) const;
};

/** Poisson log likelihood calculator. */
class QuadraticPoissonSVTTLikelihoodCalculator : public AbstractPoissonSVTTLikelihoodCalculator {
  private:
    SVTT_TREND_FUNCPTR  gpTrendOfInterest;
    double              CalcLogLikelihood(const count_t* pCases, const measure_t* pMeasure, count_t pTotalCases, 
                                          double nAlpha, double nBeta, double nBeta2, int nStatus) const;
  public:
    QuadraticPoissonSVTTLikelihoodCalculator(const CSaTScanData& DataHub);
    virtual ~QuadraticPoissonSVTTLikelihoodCalculator();

    virtual double      CalcSVTTLogLikelihood(size_t tSetIndex, SVTTClusterData& ClusterData, const AbstractTimeTrend& GlobalTimeTrend) const;
};
//******************************************************************************
#endif

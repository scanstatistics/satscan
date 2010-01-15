//******************************************************************************
#ifndef PoissonSVTTLikelihoodCalculationH
#define PoissonSVTTLikelihoodCalculationH
//******************************************************************************
#include "LikelihoodCalculation.h"
#include "Parameters.h"
#include "TimeTrend.h"

typedef bool (*SVTT_TREND_FUNCPTR) (const AbstractTimeTrend&, const AbstractTimeTrend&);

class AbstractPoissonTrendLikelihoodCalculator : public AbstractLikelihoodCalculator {
  protected:
    const CParameters & gParameters;   /** const reference to CParameters object */
    std::vector<double> gvDataSetLogLikelihoodUnderNull;
    double              CalcLogLikelihoodSpatialOnly(count_t n, measure_t u) const;

  public:
    AbstractPoissonTrendLikelihoodCalculator(const CSaTScanData& DataHub);
    virtual ~AbstractPoissonTrendLikelihoodCalculator();

    virtual double      CalcSVTTLogLikelihood(size_t tSetIndex, SVTTClusterData& ClusterData, const AbstractTimeTrend& GlobalTimeTrend) const = 0;
    virtual double      CalculateFullStatistic(double dMaximizingValue, size_t tDataSetIndex=0) const;
    //virtual double      CalculateMaximizingValue(count_t n, measure_t u, size_t tDataSetIndex=0) const;
    virtual double      GetLogLikelihoodForTotal(size_t tSetIndex=0) const;
};

/** Poisson log likelihood calculator. */
class PoissonLinearTrendLikelihoodCalculator : public AbstractPoissonTrendLikelihoodCalculator {
  private:
    SVTT_TREND_FUNCPTR  gpTrendOfInterest;
    double              CalcLogLikelihood(const count_t* pCases, const measure_t* pMeasure,
                                          count_t pTotalCases, double nAlpha,
                                          double nBeta, int nStatus) const;
  public:
    PoissonLinearTrendLikelihoodCalculator(const CSaTScanData& DataHub);
    virtual ~PoissonLinearTrendLikelihoodCalculator();

    virtual double      CalcSVTTLogLikelihood(size_t tSetIndex, SVTTClusterData& ClusterData, const AbstractTimeTrend& GlobalTimeTrend) const;
};

/** Poisson log likelihood calculator. */
class PoissonQuadraticTrendLikelihoodCalculator : public AbstractPoissonTrendLikelihoodCalculator {
  private:
    SVTT_TREND_FUNCPTR  gpTrendOfInterest;
    double              CalcLogLikelihood(const count_t* pCases, const measure_t* pMeasure, count_t pTotalCases, 
                                          double nAlpha, double nBeta, double nBeta2, int nStatus) const;
  public:
    PoissonQuadraticTrendLikelihoodCalculator(const CSaTScanData& DataHub);
    virtual ~PoissonQuadraticTrendLikelihoodCalculator();

    virtual double      CalcSVTTLogLikelihood(size_t tSetIndex, SVTTClusterData& ClusterData, const AbstractTimeTrend& GlobalTimeTrend) const;
};
//******************************************************************************
#endif

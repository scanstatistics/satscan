//******************************************************************************
#ifndef PoissonSVTTLikelihoodCalculationH
#define PoissonSVTTLikelihoodCalculationH
//******************************************************************************
#include "LikelihoodCalculation.h"
#include "Parameters.h"

typedef bool (*SVTT_TREND_FUNCPTR) (const CTimeTrend&, const CTimeTrend&);

/** Poisson log likelihood calculator. */
class PoissonSVTTLikelihoodCalculator : public AbstractLikelihoodCalculator {
  private:
    SVTT_TREND_FUNCPTR  gpTrendOfInterest;
    const CParameters & gParameters;   /** const reference to CParameters object */
    std::vector<double> gvDataSetLogLikelihoodUnderNull;
    double              CalcLogLikelihoodSpatialOnly(count_t n, measure_t u) const;
    double              CalcSVTTLogLikelihood(count_t* pCases, measure_t* pMeasure,
                                              count_t pTotalCases, double nAlpha,
                                              double nBeta, int nStatus) const;
  public:
    PoissonSVTTLikelihoodCalculator(const CSaTScanData& DataHub);
    virtual ~PoissonSVTTLikelihoodCalculator();

    virtual double      CalcSVTTLogLikelihood(size_t tSetIndex, SVTTClusterData& ClusterData, const CTimeTrend& GlobalTimeTrend) const;
    //virtual double      CalculateFullStatistic(double dMaximizingValue, size_t tDataSetIndex=0) const;
    //virtual double      CalculateMaximizingValue(count_t n, measure_t u, size_t tDataSetIndex=0) const;
    virtual double      GetLogLikelihoodForTotal(size_t tSetIndex=0) const;
};
//******************************************************************************
#endif

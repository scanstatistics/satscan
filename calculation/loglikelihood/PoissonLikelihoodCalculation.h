//******************************************************************************
#ifndef __PoissonLikelihoodCalculation_H
#define __PoissonLikelihoodCalculation_H
//******************************************************************************
#include "LikelihoodCalculation.h"
#include "Parameters.h"

/** Poisson log likelihood calculator. */
class PoissonLikelihoodCalculator : public AbstractLikelihoodCalculator {
  private:
    const CParameters & gParameters;   /** const reference to CParameters object */
    std::vector<double> gvDataSetLogLikelihoodUnderNull;
    double              CalcSVTTLogLikelihood(count_t* pCases, measure_t* pMeasure,
                                              count_t pTotalCases, double nAlpha,
                                              double nBeta, int nStatus) const;
  public:
    PoissonLikelihoodCalculator(const CSaTScanData& DataHub);
    virtual ~PoissonLikelihoodCalculator();

    virtual double      CalcLogLikelihood(count_t n, measure_t u) const;
    virtual double      CalcLogLikelihoodRatio(count_t tCases, measure_t tMeasure, size_t tSetIndex=0) const;
    virtual double      CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster) const;
    virtual double      CalcSVTTLogLikelihood(size_t tSetIndex, CSVTTCluster* Cluster, const CTimeTrend& GlobalTimeTrend) const;
    virtual double      CalculateFullStatistic(double dMaximizingValue, size_t tDataSetIndex=0) const;
    virtual double      CalculateMaximizingValue(count_t n, measure_t u, size_t tDataSetIndex=0) const;
    virtual double      GetLogLikelihoodForTotal(size_t tSetIndex=0) const;
};
//******************************************************************************
#endif

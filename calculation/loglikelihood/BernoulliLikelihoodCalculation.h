//*****************************************************************************
#ifndef __BernoulliLikelihoodCalculation_H
#define __BernoulliLikelihoodCalculation_H
//*****************************************************************************
#include "LikelihoodCalculation.h"

/** Bernoulli log likelihood calculator. */
class BernoulliLikelihoodCalculator : public AbstractLikelihoodCalculator {
  public:
    BernoulliLikelihoodCalculator(const CSaTScanData& DataHub);
    virtual ~BernoulliLikelihoodCalculator();

    virtual double      CalcLogLikelihood(count_t n, measure_t u) const;
    virtual double      CalcLogLikelihoodRatio(count_t tCases, measure_t tMeasure, count_t tTotalCases, measure_t tTotalMeasure) const;
    virtual double      GetLogLikelihoodForTotal() const;
    virtual double      CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster) const;
};
//*****************************************************************************
#endif

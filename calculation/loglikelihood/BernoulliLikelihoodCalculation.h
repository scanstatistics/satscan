//*****************************************************************************
#ifndef __BernoulliLikelihoodCalculation_H
#define __BernoulliLikelihoodCalculation_H
//*****************************************************************************
#include "LikelihoodCalculation.h"

class BernoulliLikelihoodCalculator : public AbstractLikelihoodCalculator {
  public:
    BernoulliLikelihoodCalculator(count_t tTotalCases, measure_t tTotalMeasure);
    virtual ~BernoulliLikelihoodCalculator();

    virtual double      CalcLogLikelihood(count_t n, measure_t u);
    virtual double      CalcLogLikelihoodRatio(count_t tCases, measure_t tMeasure, count_t tTotalCases, measure_t tTotalMeasure);
    virtual double      GetLogLikelihoodForTotal() const;
    virtual double      CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster);
};
//*****************************************************************************
#endif

//*****************************************************************************
#ifndef __ExponentialLikelihoodCalculation_H
#define __ExponentialLikelihoodCalculation_H
//*****************************************************************************
#include "LikelihoodCalculation.h"

class ExponentialLikelihoodCalculator : public AbstractLikelihoodCalculator {
  public:
    ExponentialLikelihoodCalculator(count_t tTotalCases, measure_t tTotalMeasure);
    virtual ~ExponentialLikelihoodCalculator();

    virtual double      CalcLogLikelihood(count_t n, measure_t u);
    virtual double      CalcLogLikelihoodRatio(count_t tCases, measure_t tMeasure, count_t tTotalCases, measure_t tTotalMeasure);
    virtual double      GetLogLikelihoodForTotal() const;
};
//*****************************************************************************
#endif

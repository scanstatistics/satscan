//*****************************************************************************
#ifndef __NormalLikelihoodCalculation_H
#define __NormalLikelihoodCalculation_H
//*****************************************************************************
#include "LikelihoodCalculation.h"

class NormalLikelihoodCalculator : public AbstractLikelihoodCalculator {
  public:
    NormalLikelihoodCalculator(count_t tTotalCases, measure_t tTotalMeasure);
    virtual ~NormalLikelihoodCalculator();

    virtual double      CalcLogLikelihood(count_t n, measure_t u);
    virtual double      CalcLogLikelihoodRatio(count_t tCases, measure_t tMeasure, count_t tTotalCases, measure_t tTotalMeasure);
    virtual double      CalcLogLikelihoodRatioEx(count_t tCases, measure_t tMeasure, measure_t tMeasure2, count_t tTotalCases, measure_t tTotalMeasure);
    virtual double      GetLogLikelihoodForTotal() const;
};
//*****************************************************************************
#endif

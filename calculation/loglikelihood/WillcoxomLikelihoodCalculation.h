//*****************************************************************************
#ifndef __WillcoxomLikelihoodCalculation_H
#define __WillcoxomLikelihoodCalculation_H
//*****************************************************************************
#include "LikelihoodCalculation.h"

class WillcoxomLikelihoodCalculator : public AbstractLikelihoodCalculator {
  public:
    WillcoxomLikelihoodCalculator(count_t tTotalCases, measure_t tTotalMeasure);
    virtual ~WillcoxomLikelihoodCalculator();

    virtual double      CalcLogLikelihood(count_t n, measure_t u);
    virtual double      CalcLogLikelihoodRatio(count_t tCases, measure_t tMeasure, count_t tTotalCases, measure_t tTotalMeasure);
    virtual double      GetLogLikelihoodForTotal() const;
};
//*****************************************************************************
#endif
 
//*****************************************************************************
#ifndef __WilcoxonLikelihoodCalculation_H
#define __WilcoxonLikelihoodCalculation_H
//*****************************************************************************
#include "LikelihoodCalculation.h"

class WilcoxonLikelihoodCalculator : public AbstractLikelihoodCalculator {
  public:
    WilcoxonLikelihoodCalculator(count_t tTotalCases, measure_t tTotalMeasure);
    virtual ~WilcoxonLikelihoodCalculator();

    virtual double      CalcLogLikelihood(count_t n, measure_t u);
    virtual double      CalcLogLikelihoodRatio(count_t tCases, measure_t tMeasure, count_t tTotalCases, measure_t tTotalMeasure);
    virtual double      GetLogLikelihoodForTotal() const;
};
//*****************************************************************************
#endif
 
//*****************************************************************************
#ifndef __NormalLikelihoodCalculation_H
#define __NormalLikelihoodCalculation_H
//*****************************************************************************
#include "LikelihoodCalculation.h"

/** Normal log likelihood calculator. Note that this class is currently just
    a shell. All defined functions, when called, will through exceptions.
    Waiting for further implementation details...                             */
class NormalLikelihoodCalculator : public AbstractLikelihoodCalculator {
  public:
    NormalLikelihoodCalculator(const CSaTScanData& Data);
    virtual ~NormalLikelihoodCalculator();

    virtual double      CalcLogLikelihood(count_t n, measure_t u) const;
    virtual double      CalcLogLikelihoodRatio(count_t tCases, measure_t tMeasure, count_t tTotalCases, measure_t tTotalMeasure) const;
    virtual double      CalcLogLikelihoodRatioEx(count_t tCases, measure_t tMeasure, measure_t tMeasure2, count_t tTotalCases, measure_t tTotalMeasure) const;
    virtual double      GetLogLikelihoodForTotal() const;
};
//*****************************************************************************
#endif

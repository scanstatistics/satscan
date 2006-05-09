//*****************************************************************************
#ifndef __NormalLikelihoodCalculation_H
#define __NormalLikelihoodCalculation_H
//*****************************************************************************
#include "LikelihoodCalculation.h"

/** Normal log likelihood calculator. Note that this class is currently just
    a shell. All defined functions, when called, will through exceptions.
    Waiting for further implementation details...                             */
class NormalLikelihoodCalculator : public AbstractLikelihoodCalculator {
  private:
    std::vector<measure_t> gvDataSetMeasureSqTotals;

  public:
    NormalLikelihoodCalculator(const CSaTScanData& Data);
    virtual ~NormalLikelihoodCalculator();

    virtual double      CalcLogLikelihoodRatioNormal(count_t tCases, measure_t tMeasure, measure_t tMeasure2, size_t tSetIndex=0) const;
    virtual double      CalculateFullStatistic(double dMaximizingValue, size_t tSetIndex=0) const;
    virtual double      CalculateMaximizingValueNormal(count_t n, measure_t u, measure_t u2, size_t tSetIndex=0) const;
};
//*****************************************************************************
#endif

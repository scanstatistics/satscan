//******************************************************************************
#ifndef __OrdinalLikelihoodCalculation_H
#define __OrdinalLikelihoodCalculation_H
//******************************************************************************
#include "LikelihoodCalculation.h"

/** Ordinal log likelihood calculator. */
class OrdinalLikelihoodCalculator : public AbstractLikelihoodCalculator {
  public:
    OrdinalLikelihoodCalculator(const CSaTScanData& Data);
    virtual ~OrdinalLikelihoodCalculator();

    virtual double      CalcLogLikelihood(count_t n, measure_t u) const;
    virtual double      CalcLogLikelihoodRatio(count_t tCases, measure_t tMeasure, count_t tTotalCases, measure_t tTotalMeasure) const;
    virtual double      CalcLogLikelihoodRatioOrdinal(const std::vector<count_t>& vOrdinalCases, const std::vector<count_t>& vOrdinalTotalCases) const;
    virtual double      GetLogLikelihoodForTotal() const;
};
//******************************************************************************
#endif


//******************************************************************************
#ifndef __BernoulliLikelihoodCalculation_H
#define __BernoulliLikelihoodCalculation_H
//******************************************************************************
#include "LikelihoodCalculation.h"

/** Bernoulli log likelihood calculator. */
class BernoulliLikelihoodCalculator : public AbstractLikelihoodCalculator {
  private:
    std::vector<double> gvDataSetLogLikelihoodUnderNull;

  public:
    BernoulliLikelihoodCalculator(const CSaTScanData& DataHub);
    virtual ~BernoulliLikelihoodCalculator();

    virtual double      CalcLogLikelihood(count_t n, measure_t u) const;
    virtual double      CalcLogLikelihoodRatio(count_t tCases, measure_t tMeasure, size_t tSetIndex=0) const;
    virtual double      CalcMonotoneLogLikelihood(tract_t tSteps, const std::vector<count_t>& vCasesList, const std::vector<measure_t>& vMeasureList) const;
    virtual double      CalculateFullStatistic(double dMaximizingValue, size_t tDataSetIndex=0) const;
    virtual double      CalculateMaximizingValue(count_t n, measure_t u, size_t tDataSetIndex=0) const;
    virtual double      GetLogLikelihoodForTotal(size_t tSetIndex=0) const;
};
//******************************************************************************
#endif

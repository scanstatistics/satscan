//******************************************************************************
#ifndef __PoissonLikelihoodCalculation_H
#define __PoissonLikelihoodCalculation_H
//******************************************************************************
#include "LikelihoodCalculation.h"
#include "Parameters.h"

/** Poisson log likelihood calculator. */
class PoissonLikelihoodCalculator : public AbstractLikelihoodCalculator {
  private:
    const CParameters & gParameters;   /** const reference to CParameters object */
    std::vector<double> gvDataSetLogLikelihoodUnderNull;
    bool _time_stratified;

  public:
    PoissonLikelihoodCalculator(const CSaTScanData& DataHub);
    virtual ~PoissonLikelihoodCalculator();

    virtual double      CalcLogLikelihood(count_t n, measure_t u) const;
    virtual double      CalcLogLikelihoodTimeStratified(count_t n, measure_t u, count_t N, measure_t U) const;
    virtual double      CalcLogLikelihoodRatio(count_t tCases, measure_t tMeasure, size_t tSetIndex=0) const;
    virtual double      CalcMonotoneLogLikelihood(tract_t tSteps, const std::vector<count_t>& vCasesList, const std::vector<measure_t>& vMeasureList) const;
    virtual double      CalculateFullStatistic(double dMaximizingValue, size_t tDataSetIndex=0) const;
    virtual double      CalculateMaximizingValue(count_t n, measure_t u, size_t tDataSetIndex=0) const;
    virtual double      GetLogLikelihoodForTotal(size_t tSetIndex=0) const;
};
//******************************************************************************
#endif

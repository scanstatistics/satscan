//******************************************************************************
#ifndef __BernoulliLikelihoodCalculation_H
#define __BernoulliLikelihoodCalculation_H
//******************************************************************************
#include "LikelihoodCalculation.h"
#include "MultipleDimensionArrayHandler.h"
#include "boost/shared_ptr.hpp"

/** Bernoulli log likelihood calculator. */
class BernoulliLikelihoodCalculator : public AbstractLikelihoodCalculator {
private:
    typedef TwoDimensionArrayHandler<double> TwoDimDoubleArray_t;
    boost::shared_ptr<TwoDimDoubleArray_t>   _datasetLogLikelihoodUnderNull;
    std::vector<count_t**>   _pp_counts;
    std::vector<measure_t**> _pp_measure;
    bool _time_stratified;

  public:
    BernoulliLikelihoodCalculator(const CSaTScanData& DataHub);
    virtual ~BernoulliLikelihoodCalculator();

    virtual double      CalcLogLikelihood(count_t n, measure_t u) const;
    virtual double      CalcLogLikelihoodRatio(count_t tCases, measure_t tMeasure, size_t tSetIndex=0) const;
    virtual double      CalcMonotoneLogLikelihood(tract_t tSteps, const std::vector<count_t>& vCasesList, const std::vector<measure_t>& vMeasureList) const;
    virtual double      CalculateFullStatistic(double dMaximizingValue, size_t tDataSetIndex=0) const;
    virtual double      CalculateMaximizingValue(count_t n, measure_t u, size_t tDataSetIndex=0) const;
    virtual double      CalcLogLikelihoodTimeStratified(count_t n, measure_t u, count_t N, measure_t U) const;
    virtual double      CalcLogLikelihoodBernoulliSpatialStratified(count_t n, measure_t u, tract_t tract, size_t setIdx = 0) const;

    virtual double      GetLogLikelihoodForTotal(size_t tSetIndex=0) const;
};
//******************************************************************************
#endif

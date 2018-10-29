//*****************************************************************************
#ifndef __UniformTimeLikelihoodCalculation_H
#define __UniformTimeLikelihoodCalculation_H
//*****************************************************************************
#include "LikelihoodCalculation.h"

/** Uniform time log likelihood calculator.*/
class UniformTimeLikelihoodCalculator : public AbstractLikelihoodCalculator {
  public:
      UniformTimeLikelihoodCalculator(const CSaTScanData& Data);
      virtual ~UniformTimeLikelihoodCalculator();

    virtual double      CalcLogLikelihoodUniformTime(count_t cases, measure_t measure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex=0) const;
    virtual double      CalcLogLikelihoodRatioUniformTime(count_t tCases, measure_t tMeasure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex=0) const;
    virtual double      CalculateFullStatistic(double dMaximizingValue, size_t tSetIndex=0) const;
    virtual double      CalculateMaximizingValue(count_t cases, measure_t u, size_t tDataSetIndex) const;
    virtual double      CalculateMaximizingValueUniformTime(count_t cases, measure_t measure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex=0) const;
    virtual double      GetLogLikelihoodForTotal(size_t tSetIndex) const;
};
//*****************************************************************************
#endif

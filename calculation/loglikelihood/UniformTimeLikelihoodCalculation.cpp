//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "UniformTimeLikelihoodCalculation.h"
#include "SaTScanData.h"

/** constructor */
UniformTimeLikelihoodCalculator::UniformTimeLikelihoodCalculator(const CSaTScanData& DataHub)
                           :AbstractLikelihoodCalculator(DataHub) {}

/** destructor */
UniformTimeLikelihoodCalculator::~UniformTimeLikelihoodCalculator() {}

/** Calculates the full test statistic given passed maximizing value and data set index. */
double UniformTimeLikelihoodCalculator::CalculateFullStatistic(double dMaximizingValue, size_t tSetIndex) const {
  return dMaximizingValue;
}

/** Calculates the maximizing value from cases and pre-calculated u. */
double UniformTimeLikelihoodCalculator::CalculateMaximizingValue(count_t cases, measure_t u, size_t tDataSetIndex) const {
    return (u - static_cast<double>(cases)) + static_cast<double>(cases) * (log(static_cast<double>(cases)) - log(u));
}

/** Calculates the maximizing value given observed cases, expected cases, expected cases squared and data set index.
    For the Normal calculator, the maximizing value is the negative variance. */
double UniformTimeLikelihoodCalculator::CalculateMaximizingValueUniformTime(count_t cases, measure_t measure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) const {
    /** 
        cases = cases in the cluster window
        casesInPeriod = total number of cases in the location for the whole study period
        measure = length of the cluster window=measure of the study window
        measureInPeriod = length of the study period=total measure of the location
    */
    double M = 2.0;
    double u = 0.0;
    if (cases < casesInPeriod)
        u = measure * static_cast<double>(casesInPeriod - cases) / (measureInPeriod - measure);
    else if (cases == casesInPeriod)
        u = measure / (M * (measureInPeriod - measure));

    return (u - static_cast<double>(cases)) + static_cast<double>(cases) * (log(static_cast<double>(cases)) - log(u));
}

/** calculates the Normal log likelihood ratio given the number of observed, expected cases, and expected cases squared */
double UniformTimeLikelihoodCalculator::CalcLogLikelihoodUniformTime(count_t cases, measure_t measure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) const {
    /** 
        cases = cases in the cluster window
        casesInPeriod = total number of cases in the location for the whole study period
        measure = length of the cluster window=measure of the study window
        measureInPeriod = length of the study period=total measure of the location
    */
    double M = 2.0;
    double u = 0.0;
	if (cases == 0)
		return 0.0;
    else if (cases < casesInPeriod)
        u = measure * static_cast<double>(casesInPeriod - cases) / (measureInPeriod - measure);
    else if (cases == casesInPeriod)
        u = measure / (M * (measureInPeriod - measure));

    return (u - static_cast<double>(cases)) + static_cast<double>(cases) * (log(static_cast<double>(cases)) - log(u));
}

/** calculates the Normal log likelihood ratio given the number of observed, expected cases, and expected cases squared */
double UniformTimeLikelihoodCalculator::CalcLogLikelihoodRatioUniformTime(count_t cases, measure_t measure, count_t casesInPeriod, measure_t measureInPeriod, size_t tSetIndex) const {
    /**
    cases = cases in the cluster window
    casesInPeriod = total number of cases in the location for the whole study period
    measure = length of the cluster window=measure of the study window
    measureInPeriod = length of the study period=total measure of the location
    */
    double M = 2.0;
    double u = 0.0;
	if (cases == 0)
		return 0.0;
    else if (cases < casesInPeriod)
        u = measure * static_cast<double>(casesInPeriod - cases) / (measureInPeriod - measure);
    else if (cases == casesInPeriod)
        u = measure / (M * (measureInPeriod - measure));

    return (u - static_cast<double>(cases)) + static_cast<double>(cases) * (log(static_cast<double>(cases)) - log(u));
}

/** Correct?  */
double UniformTimeLikelihoodCalculator::GetLogLikelihoodForTotal(size_t tSetIndex) const {
    return 0.0;
}
//*****************************************************************************
#ifndef __WilcoxonLikelihoodCalculation_H
#define __WilcoxonLikelihoodCalculation_H
//*****************************************************************************
#include "LikelihoodCalculation.h"

/* 
For any Mann-Whitney U test, the theoretical range of U is from 0 (complete separation between groups,
H0 most likely false and H1 most likely true) to n1*n2 (little evidence in support of H1).
In every test, U1+U2 is always equal to n1*n2. 
*/
#define MannWhitneyU(n1, n2, R) (n1 * n2 + (n1 * (n1 + 1.0) / 2.0) - R)

/* 
If n > 20, this table cannot be used. A p can be computed for U, using the normal distribution approximation. 
We won't need to use this because we're running the monte carlos replications.
*/
#define CriticalValueU(U, n1, n2) (std::abs(U - (n1 * n2/2.0)) / std::sqrt((n1 * n2 * (n1 + n2 + 1) / 12.0)))

/** Wilcoxon log likelihood calculator. Note that this class is currently just
    a shell. All defined functions, when called, will through exceptions.
    Waiting for further implementation details...                             */
class WilcoxonLikelihoodCalculator : public AbstractLikelihoodCalculator {
  public:
    WilcoxonLikelihoodCalculator(const CSaTScanData& DataHub);
    virtual ~WilcoxonLikelihoodCalculator();

    virtual double      CalcLogLikelihood(count_t n, measure_t u) const;
    virtual double      CalcLogLikelihoodRatio(count_t tCases, measure_t tMeasure, size_t tSetIndex = 0) const;
    virtual double      CalculateFullStatistic(double dMaximizingValue, size_t tDataSetIndex = 0) const;
    virtual double      CalculateMaximizingValue(count_t n, measure_t u, size_t tDataSetIndex = 0) const;
    virtual double      GetLogLikelihoodForTotal(size_t tSetIndex = 0) const;
};
//*****************************************************************************
#endif

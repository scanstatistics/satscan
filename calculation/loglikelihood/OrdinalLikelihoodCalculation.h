//******************************************************************************
#ifndef __OrdinalLikelihoodCalculation_H
#define __OrdinalLikelihoodCalculation_H
//******************************************************************************
#include "LikelihoodCalculation.h"

/** Ordinal log likelihood calculator. */
class OrdinalLikelihoodCalculator : public AbstractLikelihoodCalculator {
  private:
    typedef bool                        (OrdinalLikelihoodCalculator:: *COMPARE_RATIOS_METHOD) (double, double) const;

    mutable std::vector<double>         gvP;  /** gvP[k] is the probability that a case inside the window belongs to category k */
    mutable std::vector<double>         gvQ;  /** gvQ[k] is the probability that a case outside the window belongs to category k */
    std::vector<double>                 gvDataSetLogLikelihoodUnderNull;
    std::vector<std::vector<double> >   gvDataSetTotalCasesPerCategory;
    bool                                gbScanHighRates;           /** indicates whether to scan for high rates */
    bool                                gbScanLowRates;            /** indicates whether to scan for low rates */

    bool                CalculateLoglikelihoodInsideAndOutside(const std::vector<count_t>& vOrdinalCases, const std::vector<double>& vOrdinalTotalCases, COMPARE_RATIOS_METHOD pCompareMethod) const;
    bool                CompareRatioForHighScanningArea(double dPk, double dPi) const {return dPk < dPi;}
    bool                CompareRatioForLowScanningArea(double dPk, double dPi) const {return dPk > dPi;}

  public:
    OrdinalLikelihoodCalculator(const CSaTScanData& DataHub);
    virtual ~OrdinalLikelihoodCalculator();

    virtual double      CalcLogLikelihood(count_t n, measure_t u) const;
    virtual double      CalcLogLikelihoodRatio(count_t tCases, measure_t tMeasure, count_t tTotalCases, measure_t tTotalMeasure) const;
    virtual double      CalcLogLikelihoodRatioOrdinal(const std::vector<count_t>& vOrdinalCases, size_t tSetIndex=0) const;
    virtual double      CalcLogLikelihoodRatioOrdinalHighRate(const std::vector<count_t>& vOrdinalCases, size_t tSetIndex=0) const;
    virtual double      CalcLogLikelihoodRatioOrdinalLowRate(const std::vector<count_t>& vOrdinalCases, size_t tSetIndex=0) const;
    virtual double      GetLogLikelihoodForTotal() const;
};
//******************************************************************************
#endif


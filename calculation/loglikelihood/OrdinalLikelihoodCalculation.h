//******************************************************************************
#ifndef __OrdinalLikelihoodCalculation_H
#define __OrdinalLikelihoodCalculation_H
//******************************************************************************
#include "LikelihoodCalculation.h"

/** Representation of ordinal categories that are possibly combined. */
class OrdinalCombinedCategory {
  private:
    std::vector<int>    gvCatorgiesIndexes;

  public:
    OrdinalCombinedCategory(int iInitialCategoryIndex);
    ~OrdinalCombinedCategory();

    void                Combine(int iCategoryIndex);
    int                 GetCategoryIndex(size_t tIndex) const;
    size_t              GetNumCombinedCategories() const {return gvCatorgiesIndexes.size();}
};

/** Ordinal log likelihood calculator. */
class OrdinalLikelihoodCalculator : public AbstractLikelihoodCalculator {
  private:
    typedef bool                        (OrdinalLikelihoodCalculator:: *COMPARE_RATIOS_METHOD) (double, double, double, double) const;

    mutable std::vector<double>         gvP;  /** gvP[k] is the probability that a case inside the window belongs to category k */
    mutable std::vector<double>         gvQ;  /** gvQ[k] is the probability that a case outside the window belongs to category k */
    std::vector<double>                 gvDataSetLogLikelihoodUnderNull;
    std::vector<std::vector<double> >   gvDataSetTotalCasesPerCategory;
    bool                                gbScanHighRates;           /** indicates whether to scan for high rates */
    bool                                gbScanLowRates;            /** indicates whether to scan for low rates */

    void                CalculateCombinedCategories(const std::vector<count_t>& vOrdinalCases, std::vector<OrdinalCombinedCategory>& vOrdinalCategories) const;
    double              CalculateLogLikelihood(const std::vector<count_t>& vOrdinalCases, size_t tSetIndex) const;
    bool                CalculateProbabilitiesInsideAndOutsideOfCluster(const std::vector<count_t>& vOrdinalCases, const std::vector<double>& vOrdinalTotalCases, COMPARE_RATIOS_METHOD pCompareMethod) const;
    bool                CompareRatioForHighScanningArea(double dPk, double dQk, double dPi,double dQi) const;
    bool                CompareRatioForLowScanningArea(double dPk, double dQk, double dPi, double dQi) const;

  public:
    OrdinalLikelihoodCalculator(const CSaTScanData& DataHub);
    virtual ~OrdinalLikelihoodCalculator();

    virtual double      CalcLogLikelihoodRatioOrdinal(const std::vector<count_t>& vOrdinalCases, size_t tSetIndex=0) const;
    virtual double      CalcLogLikelihoodRatioOrdinalHighRate(const std::vector<count_t>& vOrdinalCases, size_t tSetIndex=0) const;
    virtual double      CalcLogLikelihoodRatioOrdinalLowRate(const std::vector<count_t>& vOrdinalCases, size_t tSetIndex=0) const;
    virtual double      CalculateFullStatistic(double dMaximizingValue, size_t tDataSetIndex=0) const;
    virtual double      CalculateMaximizingValueOrdinal(const std::vector<count_t>& vOrdinalCases, size_t tSetIndex=0) const;
    virtual void        CalculateOrdinalCombinedCategories(const std::vector<count_t>& vOrdinalCases, std::vector<OrdinalCombinedCategory>& vOrdinalCategories, size_t tSetIndex=0) const;
};
//******************************************************************************
#endif


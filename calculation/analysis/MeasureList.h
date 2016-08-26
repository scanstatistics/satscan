//*****************************************************************************
#ifndef __MEASURELIST_H
#define __MEASURELIST_H
//*****************************************************************************

class CSaTScanData;
class AbstractLikelihoodCalculator;

/** Abstract base class which defines interface for the process of calculating
    the greatest log likelihood ratio for a simulation. The intent of this class
    is to make the performance of monte carlo simulations faster than process used
    to calculate most likely clusters. There are caveats to its use:
      - analyses with multiple data sets can not use this method to perform simulations
      - analyses that use the Normal probability model can not use this method to
        perform simulations
      - this method might actually cause a decrease in performance with a large
        number of cases versus centroids/locations; future feature could be a
        conditional usage of this class at runtime. */
class CMeasureList {
  private:
    void                           Setup();

  protected:
    const CSaTScanData           & gSaTScanData;
    AbstractLikelihoodCalculator & gLikelihoodCalculator;
    std::vector<int>               gvCalculationBoundries;
    std::vector<double>            gvMaximumLogLikelihoodRatios;

    void                           AddMaximumLogLikelihood(double dMaxLogLikelihood, int iIteration);
    virtual void                   CalculateBernoulliMaximumLogLikelihood(int iIteration) = 0;
    virtual void                   CalculateMaximumLogLikelihood(int iIteration) = 0;
    virtual void                   SetMeasures() = 0;

  public:
            CMeasureList(const CSaTScanData & hub, AbstractLikelihoodCalculator & LikelihoodCalculator);
    virtual ~CMeasureList();

    virtual void                   AddMeasure(count_t n, measure_t u) = 0;
    virtual void                   Display(FILE* pFile) const = 0;
    double                         GetMaximumLogLikelihoodRatio();
    void                           Reset();
    void                           SetForNextIteration(int iIteration);
};

/** Redefines class methods to scan for areas with more than expected cases. */
class CMinMeasureList : public CMeasureList {
  private:
    void                        Init();
    void                        Setup();

  protected:
    measure_t                 * gpMinMeasures;

    virtual void                CalculateBernoulliMaximumLogLikelihood(int iIteration);
    virtual void                CalculateMaximumLogLikelihood(int iIteration);
    virtual void                SetMeasures();

  public:
            CMinMeasureList(const CSaTScanData & hub, AbstractLikelihoodCalculator & LikelihoodCalculator);
    virtual ~CMinMeasureList();

    inline virtual void         AddMeasure(count_t n, measure_t u);
    virtual void                Display(FILE* pFile) const;
};

inline void CMinMeasureList::AddMeasure(count_t n, measure_t u) {
  if (gpMinMeasures[n] > u)
    gpMinMeasures[n] = u;
}

/** Redefines class methods to scan for areas with less than expected cases. */
class CMaxMeasureList : public CMeasureList {
  private:
    void                        Init();
    void                        Setup();

  protected:
    measure_t                 * gpMaxMeasures;

    virtual void                CalculateBernoulliMaximumLogLikelihood(int iIteration);
    virtual void                CalculateMaximumLogLikelihood(int iIteration);
    virtual void                SetMeasures();

  public:
            CMaxMeasureList(const CSaTScanData & SaTScanData, AbstractLikelihoodCalculator & LikelihoodCalculator);
    virtual ~CMaxMeasureList();

    inline virtual void         AddMeasure(count_t n, measure_t u);
    virtual void                Display(FILE* pFile) const;
};

inline void CMaxMeasureList::AddMeasure(count_t n, measure_t u) {
  if (gpMaxMeasures[n] < u)
    gpMaxMeasures[n] = u;
}

/** Redefines class methods to scan for area with both less than or greater than expected cases simultaneously. */
class CMinMaxMeasureList : public CMeasureList {
  private:
    void                        Init();
    void                        Setup();

  protected:
    measure_t                 * gpMinMeasures;
    measure_t                 * gpMaxMeasures;

    virtual void                CalculateBernoulliMaximumLogLikelihood(int iIteration);
    virtual void                CalculateMaximumLogLikelihood(int iIteration);
    virtual void                SetMeasures();

  public:
            CMinMaxMeasureList(const CSaTScanData & SaTScanData, AbstractLikelihoodCalculator & LikelihoodCalculator);
    virtual ~CMinMaxMeasureList();

    inline virtual void         AddMeasure(count_t n, measure_t u);
    virtual void                Display(FILE* pFile) const;
};

inline void CMinMaxMeasureList::AddMeasure(count_t n, measure_t u) {
  if (gpMinMeasures[n] > u)
    gpMinMeasures[n] = u;
  if (gpMaxMeasures[n] < u)
    gpMaxMeasures[n] = u;
}

/* Risk level calculation - abstraction. */
class RiskCalculate {
protected:
    const double _measure_adjustment;

public:
    RiskCalculate(double measure_adjustment) : _measure_adjustment(measure_adjustment) {}
    virtual ~RiskCalculate() {}

    virtual double getRisk(count_t n, measure_t u) const = 0;
};

/* Risk level calculation - observed / expected. */
class ObservedDividedExpectedCalc : public RiskCalculate {
public:
    ObservedDividedExpectedCalc(double measure_adjustment) : RiskCalculate(measure_adjustment) {}
    virtual ~ObservedDividedExpectedCalc() {}

    virtual double getRisk(count_t n, measure_t u) const {
        u *= _measure_adjustment; // apply measure adjustment --  applicable only to Bernoulli
        return (u ? static_cast<double>(n) / u : 0.0);
    }
};

/* Risk level calculation - relative risk. */
class RelativeRiskCalc : public RiskCalculate {
protected:
    const double _total_cases;

public:
    RelativeRiskCalc(double total_cases, double measure_adjustment) : RiskCalculate(measure_adjustment), _total_cases(total_cases) {}
    virtual ~RelativeRiskCalc() {}

    virtual double getRisk(count_t n, measure_t u) const {
        if (_total_cases == n) return std::numeric_limits<double>::max(); // could use std::numeric_limits<double>::infinity()
        u *= _measure_adjustment; // apply measure adjustment --  applicable only to Bernoulli
        if (u && _total_cases - u && ((_total_cases - u) / (_total_cases - u)))
            return (n / u) / ((_total_cases - n) / (_total_cases - u));
        return 0.0;
    }
};

/** Extends class CMinMeasureList to include risk level check. */
class RiskMinMeasureList : public CMinMeasureList {
protected:
    std::auto_ptr<RiskCalculate> _risk_calc;
    const double _risk_threshold;

public:
    RiskMinMeasureList(const CSaTScanData& hub, AbstractLikelihoodCalculator& calculator, double risk_threshold);
    virtual ~RiskMinMeasureList() {}

    virtual void AddMeasure(count_t n, measure_t u) {
        if (gpMinMeasures[n] > u && _risk_calc->getRisk(n, u) > _risk_threshold)
            gpMinMeasures[n] = u;
    }
};

/** Extends class CMaxMeasureList to include risk level check. */
class RiskMaxMeasureList : public CMaxMeasureList {
protected:
    std::auto_ptr<RiskCalculate> _risk_calc;
    const double _risk_threshold;

public:
    RiskMaxMeasureList(const CSaTScanData& hub, AbstractLikelihoodCalculator& calculator, double risk_threshold);
    virtual ~RiskMaxMeasureList() {}

    virtual void AddMeasure(count_t n, measure_t u) {
        if (gpMaxMeasures[n] < u && _risk_calc->getRisk(n, u) < _risk_threshold)
            gpMaxMeasures[n] = u;
    }
};

/** Redefines class methods to scan for area with both less than or greater than expected cases simultaneously - while including risk level check. */
class RiskMinMaxMeasureList : public CMinMaxMeasureList {
protected:
    std::auto_ptr<RiskCalculate> _risk_calc;
    const double _low_risk_threshold;
    const double _high_risk_threshold;

public:
    RiskMinMaxMeasureList(const CSaTScanData& hub, AbstractLikelihoodCalculator& calculator, double low_risk_threshold, double high_risk_threshold);
    virtual ~RiskMinMaxMeasureList() {}

    virtual void AddMeasure(count_t n, measure_t u) {
        double risk = _risk_calc->getRisk(n, u);
        if (gpMinMeasures[n] > u) 
            if (risk > _high_risk_threshold)
                gpMinMeasures[n] = u;
        if (gpMaxMeasures[n] < u)
            if (risk < _low_risk_threshold)
                gpMaxMeasures[n] = u;
    }
};
//*****************************************************************************
#endif

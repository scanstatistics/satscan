//*****************************************************************************
#ifndef __MEASURELIST_H
#define __MEASURELIST_H
//*****************************************************************************
#include "SaTScanData.h"
#include "LikelihoodCalculation.h"

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
            CMeasureList(const CSaTScanData & SaTScanData, AbstractLikelihoodCalculator & LikelihoodCalculator);
    virtual ~CMeasureList();

    virtual void                   AddMeasure(count_t n, measure_t u) = 0;
    virtual void                   Display(FILE* pFile) const = 0;
    double                         GetMaximumLogLikelihoodRatio();
    void                           Reset();
    void                           SetForNextIteration(int iIteration);
};

/** Redefines base class methods to scan for areas with more than expected cases. */
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
            CMinMeasureList(const CSaTScanData & SaTScanData, AbstractLikelihoodCalculator & LikelihoodCalculator);
    virtual ~CMinMeasureList();

    inline virtual void         AddMeasure(count_t n, measure_t u);
    virtual void                Display(FILE* pFile) const;
};

inline void CMinMeasureList::AddMeasure(count_t n, measure_t u) {
  if (gpMinMeasures[n] > u)
    gpMinMeasures[n] = u;
}

/** Redefines base class methods to scan for areas with less than expected cases. */
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

/** Redefines base class methods to scan for area with both less than or greater
    than expected cases simultaneously. */
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
//*****************************************************************************
#endif

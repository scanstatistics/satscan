//*****************************************************************************
#ifndef __MEASURELIST_H
#define __MEASURELIST_H
//*****************************************************************************
#include "SaTScanData.h"
#include <malloc.h>

class CMeasureList {
  private:
    void                        Setup();

  protected:
    BasePrint                 & gPrintDirection;
    const CSaTScanData        & gSaTScanData;
    std::vector<int>            gvCalculationBoundries;
    std::vector<double>         gvMaximumLogLikelihoodRatios;

    void                        AddMaximumLogLikelihood(double dMaxLogLikelihood, int iIteration);
    virtual void                CalculateMaximumLogLikelihood(int iIteration) = 0;
    virtual void                SetMeasures() = 0;

  public:
            CMeasureList(const CSaTScanData & SaTScanData, BasePrint & PrintDirection);
    virtual ~CMeasureList();

    virtual void                AddMeasure(count_t n, measure_t u) = 0;
    virtual void                Display(FILE* pFile) const = 0;
    double                      GetMaximumLogLikelihoodRatio();
    void                        SetForNextIteration(int iIteration);
};

class CMinMeasureList : public CMeasureList {
  private:
    void                        Init();
    void                        Setup();

  protected:
    measure_t                 * gpMinMeasures;

    virtual void                CalculateMaximumLogLikelihood(int iIteration);
    virtual void                SetMeasures();

  public:
            CMinMeasureList(const CSaTScanData & SaTScanData, BasePrint & PrintDirection);
    virtual ~CMinMeasureList();

    inline virtual void         AddMeasure(count_t n, measure_t u);
    virtual void                Display(FILE* pFile) const;
};

inline void CMinMeasureList::AddMeasure(count_t n, measure_t u) {
  if (gpMinMeasures[n] > u)
    gpMinMeasures[n] = u;
}

class CMaxMeasureList : public CMeasureList {
  private:
    void                        Init();
    void                        Setup();

  protected:
    measure_t                 * gpMaxMeasures;

    virtual void                CalculateMaximumLogLikelihood(int iIteration);
    virtual void                SetMeasures();

  public:
            CMaxMeasureList(const CSaTScanData & SaTScanData, BasePrint & PrintDirection);
    virtual ~CMaxMeasureList();

    inline virtual void         AddMeasure(count_t n, measure_t u);
    virtual void                Display(FILE* pFile) const;
};

inline void CMaxMeasureList::AddMeasure(count_t n, measure_t u) {
  if (gpMaxMeasures[n] < u)
    gpMaxMeasures[n] = u;
}

class CMinMaxMeasureList : public CMeasureList {
  private:
    void                        Init();
    void                        Setup();

  protected:
    measure_t                 * gpMinMeasures;
    measure_t                 * gpMaxMeasures;

    virtual void                CalculateMaximumLogLikelihood(int iIteration);
    virtual void                SetMeasures();

  public:
            CMinMaxMeasureList(const CSaTScanData & SaTScanData, BasePrint & PrintDirection);
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

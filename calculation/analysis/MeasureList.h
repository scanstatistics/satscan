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

    virtual void                SetMeasures() = 0;

  public:
            CMeasureList(const CSaTScanData & SaTScanData, BasePrint & PrintDirection);
    virtual ~CMeasureList();

    virtual void                AddMeasure(count_t n, measure_t u) = 0;
    virtual void                Display(FILE* pFile) const = 0;
    virtual double              GetMaxLogLikelihood(double dMaxLogLikelihood, int iEllipseOffset = 0) = 0;
    void                        SetForNextIteration(int iIteration, double & dMaxLogLikelihood);
};

class CMinMeasureList : public CMeasureList {
  private:
    void                        Init();
    void                        Setup();

  protected:
    measure_t                 * gpMinMeasures;

    virtual void                SetMeasures();

  public:
            CMinMeasureList(const CSaTScanData & SaTScanData, BasePrint & PrintDirection);
    virtual ~CMinMeasureList();

    inline virtual void         AddMeasure(count_t n, measure_t u);
    virtual void                Display(FILE* pFile) const;
    virtual double              GetMaxLogLikelihood(double dMaxLogLikelihood, int iEllipseOffset = 0);
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

    virtual void                SetMeasures();

  public:
            CMaxMeasureList(const CSaTScanData & SaTScanData, BasePrint & PrintDirection);
    virtual ~CMaxMeasureList();

    inline virtual void         AddMeasure(count_t n, measure_t u);
    virtual void                Display(FILE* pFile) const;
    virtual double              GetMaxLogLikelihood(double dMaxLogLikelihood, int iEllipseOffset = 0);
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

    virtual void                SetMeasures();

  public:
            CMinMaxMeasureList(const CSaTScanData & SaTScanData, BasePrint & PrintDirection);
    virtual ~CMinMaxMeasureList();

    inline virtual void         AddMeasure(count_t n, measure_t u);
    virtual void                Display(FILE* pFile) const;
    virtual double              GetMaxLogLikelihood(double dMaxLogLikelihood, int iEllipseOffset = 0);
};

inline void CMinMaxMeasureList::AddMeasure(count_t n, measure_t u) {
  if (gpMinMeasures[n] > u)
    gpMinMeasures[n] = u;
  if (gpMaxMeasures[n] < u)
    gpMaxMeasures[n] = u;
}
//*****************************************************************************
#endif

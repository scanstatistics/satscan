//*****************************************************************************
#ifndef __MEASURELIST_H
#define __MEASURELIST_H
//*****************************************************************************
#include "SaTScanData.h"
#include <malloc.h>

class CMeasureList {
  protected:
    count_t     m_nListSize;
    BasePrint * gpPrintDirection;

  public:
            CMeasureList(count_t N, BasePrint *pPrintDirection);
    virtual ~CMeasureList();

    virtual     void   AddMeasure(count_t n, measure_t u) = 0;
    virtual     void   Display(FILE* pFile) {/*stub - no action*/};
    virtual     double GetMaxLogLikelihood(const CSaTScanData& Data) = 0;
};

class CMinMeasureList : public CMeasureList {
  protected:
    measure_t           * m_pMinMeasures;

  public:
            CMinMeasureList(count_t N, measure_t U, BasePrint *pPrintDirection);
    virtual ~CMinMeasureList();

    inline virtual      void   AddMeasure(count_t n, measure_t u);
    virtual void        Display(FILE* pFile);
    virtual double      GetMaxLogLikelihood(const CSaTScanData& Data);
};

inline void CMinMeasureList::AddMeasure(count_t n, measure_t u) {
  if (m_pMinMeasures[n] > u)
    m_pMinMeasures[n] = u;
}

class CMaxMeasureList : public CMeasureList {
  protected:
    measure_t         * m_pMaxMeasures;

  public:
            CMaxMeasureList(count_t N, measure_t U, BasePrint *pPrintDirection);
    virtual ~CMaxMeasureList();

    inline virtual void AddMeasure(count_t n, measure_t u);
    virtual void        Display(FILE* pFile);
    virtual double      GetMaxLogLikelihood(const CSaTScanData& Data);
};

inline void CMaxMeasureList::AddMeasure(count_t n, measure_t u) {
  if (m_pMaxMeasures[n] < u)
    m_pMaxMeasures[n] = u;
}

class CMinMaxMeasureList : public CMeasureList {
  protected:
    measure_t         * m_pMinMeasures;
    measure_t         * m_pMaxMeasures;

  public:
            CMinMaxMeasureList(count_t N, measure_t U, BasePrint *pPrintDirection);
    virtual ~CMinMaxMeasureList();

    inline virtual void AddMeasure(count_t n, measure_t u);
    virtual void        Display(FILE* pFile);
    virtual double      GetMaxLogLikelihood(const CSaTScanData& Data);
};

inline void CMinMaxMeasureList::AddMeasure(count_t n, measure_t u) {
  if (m_pMinMeasures[n] > u)
    m_pMinMeasures[n] = u;
  if (m_pMaxMeasures[n] < u)
    m_pMaxMeasures[n] = u;
}
//*****************************************************************************
#endif

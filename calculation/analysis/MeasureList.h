// MeasureList.h

#ifndef __MEASURELIST_H
#define __MEASURELIST_H

#include "spatscan.h"
#include "data.h"
#include "baseprint.h"

class CMeasureList
{
  public:
    CMeasureList(count_t N, BasePrint *pPrintDirection);
    virtual ~CMeasureList();

    virtual void   AddMeasure(count_t n, measure_t u) = 0;
//    virtual double GetMaxLogLikelihood(count_t N, measure_t U, double& nMaxLogLikelihood)   = 0;
    virtual double GetMaxLogLikelihood(const CSaTScanData& Data)   = 0;
    virtual void   Display(FILE* pFile) {};

  protected:
    count_t m_nListSize;
    BasePrint *gpPrintDirection;
};

class CMinMeasureList : public CMeasureList
{
  public:
    CMinMeasureList(count_t N, measure_t U, BasePrint *pPrintDirection);
    virtual ~CMinMeasureList();

    virtual void   AddMeasure(count_t n, measure_t u);
//    virtual double GetMaxLogLikelihood(count_t N, measure_t U, double& nMaxLogLikelihood);
    virtual double GetMaxLogLikelihood(const CSaTScanData& Data);
    virtual void   Display(FILE* pFile);

  protected:
    measure_t* m_pMinMeasures;

};

class CMaxMeasureList : public CMeasureList
{
  public:
    CMaxMeasureList(count_t N, measure_t U, BasePrint *pPrintDirection);
    virtual ~CMaxMeasureList();

    virtual void   AddMeasure(count_t n, measure_t u);
//    virtual double GetMaxLogLikelihood(count_t N, measure_t U, double& nMaxLogLikelihood);
    virtual double GetMaxLogLikelihood(const CSaTScanData& Data);
    virtual void   Display(FILE* pFile);

  protected:
    measure_t* m_pMaxMeasures;

};

class CMinMaxMeasureList : public CMeasureList
{
  public:
    CMinMaxMeasureList(count_t N, measure_t U, BasePrint *pPrintDirection);
    virtual ~CMinMaxMeasureList();

    virtual void   AddMeasure(count_t n, measure_t u);
//    virtual double GetMaxLogLikelihood(count_t N, measure_t U, double& nMaxLogLikelihood);
    virtual double GetMaxLogLikelihood(const CSaTScanData& Data);
    virtual void   Display(FILE* pFile);

  protected:
    measure_t* m_pMinMeasures;
    measure_t* m_pMaxMeasures;

};

#endif

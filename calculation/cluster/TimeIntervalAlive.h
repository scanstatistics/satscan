//*****************************************************************************
#ifndef __TIMEINTERVALALIVE_H
#define __TIMEINTERVALALIVE_H
//*****************************************************************************
#include "TimeIntervals.h"

class CTIAlive : public CTimeIntervals
{
  public:
    CTIAlive(int nTotal, int nCut);
    virtual ~CTIAlive() {};

    inline virtual void         Initialize();
    virtual count_t             GetCaseCountForTract(tract_t tTract, count_t** pCases) const;
    virtual measure_t           GetMeasureForTract(tract_t tTract,  measure_t** pMeasure) const;
    inline virtual bool         GetNextTimeInterval(const count_t* pCases,
                                                    const measure_t* pMeasure,
                                                    count_t& nCases,
                                                    measure_t& nMeasure,
                                                    int& nStart, int& nStop);
    inline virtual bool         GetNextTimeIntervalProsp(const count_t* pCases,
                                                         const measure_t* pMeasure,
                                                         count_t& nCases,
                                                         measure_t& nMeasure);
   /*virtual bool GetNextTimeIntervalProsp(const count_t*& pCases,
                                     const measure_t*& pMeasure,
                                     count_t& nCases,
                                     measure_t& nMeasure,
                                     int& nStart,
                                     int& nStop); */
};

//*****************************************************************************
#endif

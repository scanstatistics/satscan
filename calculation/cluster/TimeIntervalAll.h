//*****************************************************************************
#ifndef __TIMEINTERVALIALL_H
#define __TIMEINTERVALIALL_H
//*****************************************************************************
#include "TimeIntervals.h"

class CTIAll : public CTimeIntervals
{
  public:
    CTIAll(int nTotal, int nCut);
    virtual ~CTIAll() {};

    inline virtual void         Initialize();
    void                        InitializeRange(int nLow, int nHigh);
    virtual count_t             GetCaseCountForTract(tract_t tTract, count_t** pCases) const;
    virtual measure_t           GetMeasureForTract(tract_t tTract, measure_t** pMeasure) const;
    virtual bool                GetNextTimeInterval(const count_t*& pCases,
                                                    const measure_t*& pMeasure,
                                                    count_t& nCases, measure_t& nMeasure,
                                                    int& nStart, int& nStop);
    inline virtual bool         GetNextTimeIntervalProsp(const count_t*& pCases,
                                                         const measure_t*& pMeasure,
                                                         count_t& nCases, measure_t& nMeasure);
   /*virtual bool GetNextTimeIntervalProsp(const count_t*& pCases,
                                     const measure_t*& pMeasure,
                                     count_t& nCases,
                                     measure_t& nMeasure,
                                     int& nStart,
                                     int& nStop);
*/
};
//*****************************************************************************
#endif

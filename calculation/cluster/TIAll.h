// TIAll.h

#ifndef __TIALL_H
#define __TIALL_H

#include "TimeIntervals.h"

class CTIAll : public CTimeIntervals
{
  public:
    CTIAll(int nTotal, int nCut);
    virtual ~CTIAll() {};

    virtual void Initialize();
    virtual bool GetNextTimeInterval(const count_t*& pCases,
                                     const measure_t*& pMeasure,
                                     count_t& nCases,
                                     measure_t& nMeasure,
                                     int& nStart,
                                     int& nStop);

};

#endif

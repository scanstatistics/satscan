// TIAlive.h

#ifndef __TIALIVE_H
#define __TIALIVE_H

#include "TimeIntervals.h"

class CTIAlive : public CTimeIntervals
{
  public:
    CTIAlive(int nTotal, int nCut);
    virtual ~CTIAlive() {};

    virtual void Initialize();
    virtual bool GetNextTimeInterval(const count_t*& pCases,
                                     const measure_t*& pMeasure,
                                     count_t&   nCases,
                                     measure_t& nMeasure,
                                     int&       nStart,
                                     int&       nStop);

};

#endif

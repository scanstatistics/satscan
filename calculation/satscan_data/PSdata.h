// dataPS.h

#ifndef __DATAPS_H
#define __DATAPS_H

#include "data.h"

class CPurelySpatialData : public CSaTScanData
{
  public:
    CPurelySpatialData(CParameters* pParameters, BasePrint *pPrintDirection);
    virtual ~CPurelySpatialData();

//    int* m_pCases;
//    int* m_pMeasures;

  protected:
    virtual void SetNumTimeIntervals() {m_nTimeIntervals = 1;};

    //    virtual void AllocateForCasesAndMeasures();

//    int* AllocateCountsArray();
};


#endif

//*****************************************************************************
#ifndef __PURELYSPATIALDATA_H
#define __PURELYSPATIALDATA_H
//*****************************************************************************
#include "SaTScanData.h"

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

//******************************************************************************
#endif

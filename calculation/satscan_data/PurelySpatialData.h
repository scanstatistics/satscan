//*****************************************************************************
#ifndef __PURELYSPATIALDATA_H
#define __PURELYSPATIALDATA_H
//*****************************************************************************
#include "SaTScanData.h"

class CPurelySpatialData : public CSaTScanData {
  private:
    virtual void        SetProbabilityModel();

  protected:
    virtual void        SetNumTimeIntervals() {m_nTimeIntervals = 1;};

  public:
    CPurelySpatialData(const CParameters* pParameters, BasePrint *pPrintDirection);
    virtual ~CPurelySpatialData();
};

//******************************************************************************
#endif

//*****************************************************************************
#ifndef __PURELYSPATIALDATA_H
#define __PURELYSPATIALDATA_H
//*****************************************************************************
#include "SaTScanData.h"

/** Data hub class that derives from base class to define alternate functionality
    for purely spatial analyses. */
class CPurelySpatialData : public CSaTScanData {
  private:
    virtual void        SetProbabilityModel();

  protected:
    virtual void        SetIntervalStartTimes();

  public:
    CPurelySpatialData(const CParameters* pParameters, BasePrint *pPrintDirection);
    virtual ~CPurelySpatialData();
};
//******************************************************************************
#endif

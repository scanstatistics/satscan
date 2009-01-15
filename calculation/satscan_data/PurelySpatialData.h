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
    virtual void        SetIntervalCut() {m_nIntervalCut = 1;}
    virtual void        SetIntervalStartTimes();
    virtual void        SetTimeIntervalRangeIndexes() {/* nop */}

  public:
    CPurelySpatialData(const CParameters& Parameters, BasePrint& PrintDirection);
    virtual ~CPurelySpatialData();
};
//******************************************************************************
#endif

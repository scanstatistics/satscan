//**************************************************************************
#ifndef __SPACETIMEDATA_H
#define __SPACETIMEDATA_H
//**************************************************************************
#include "SaTScanData.h"

class CSpaceTimeData : public CSaTScanData {
  protected:
    virtual bool        CalculateMeasure();
    virtual void        ReadDataFromFiles();
    virtual void        SetIntervalCut();
    virtual void        SetProbabilityModel();

  public:
    CSpaceTimeData(CParameters* pParameters, BasePrint *pPrintDirection);
    virtual ~CSpaceTimeData();

    virtual void        AllocSimCases();
    virtual void        DeAllocSimCases();
    virtual void        MakeData(int iSimulationNumber);
};

//**************************************************************************
#endif
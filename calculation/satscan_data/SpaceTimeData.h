//**************************************************************************
#ifndef __SPACETIMEDATA_H
#define __SPACETIMEDATA_H
//**************************************************************************
#include "SaTScanData.h"

class CSpaceTimeData : public CSaTScanData
{
  public:
    CSpaceTimeData(CParameters* pParameters, BasePrint *pPrintDirection);
    virtual ~CSpaceTimeData();

//    void SetPurelyTemporalSimCases();

    virtual void AllocSimCases();
    virtual void DeAllocSimCases();
    virtual void MakeData(int iSimulationNumber);
    
  protected:
    virtual void SetIntervalCut();

    virtual void ReadDataFromFiles();
    virtual bool CalculateMeasure();

};

//**************************************************************************
#endif
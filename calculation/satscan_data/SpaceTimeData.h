//**************************************************************************
#ifndef __SPACETIMEDATA_H
#define __SPACETIMEDATA_H
//**************************************************************************
#include "SaTScanData.h"

class CSpaceTimeData : public CSaTScanData {
  protected:
    virtual bool        CalculateMeasure(DataStream & thisStream);
    virtual void        ReadDataFromFiles();
    virtual void        SetIntervalCut();
    virtual void        SetProbabilityModel();

  public:
    CSpaceTimeData(CParameters* pParameters, BasePrint *pPrintDirection);
    virtual ~CSpaceTimeData();

    virtual void        AllocateSimulationStructures();
    virtual void        MakeData(int iSimulationNumber, DataStreamGateway & DataGateway);
};

//**************************************************************************
#endif
// dataST.h

#ifndef __DATAST_H
#define __DATAST_H

#include "data.h"

class CSpaceTimeData : public CSaTScanData
{
  public:
    CSpaceTimeData(CParameters* pParameters, BasePrint *pPrintDirection);
    virtual ~CSpaceTimeData();

//    void SetPurelyTemporalSimCases();

    virtual void AllocSimCases();
    virtual void DeAllocSimCases();
    virtual void MakeData();
    
  protected:
    virtual void SetIntervalCut();

    virtual void ReadDataFromFiles();
    virtual bool CalculateMeasure();

};


#endif
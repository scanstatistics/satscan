//*********************************************************************************
#ifndef __PURELYTEMPORALDATA_H
#define __PURELYTEMPORALDATA_H
//*********************************************************************************
#include "SaTScanData.h"

class CPurelyTemporalData : public CSaTScanData
{
  public:
    CPurelyTemporalData(CParameters* pParameters, BasePrint *pPrintDirection);
    virtual ~CPurelyTemporalData();

    virtual void DisplayCases(FILE* pFile);
    virtual void DisplaySimCases(FILE* pFile);
    virtual void DisplayMeasure(FILE* pFile);

//  protected:
    virtual void ReadDataFromFiles();
    virtual bool CalculateMeasure();
    virtual void AllocSimCases();
    virtual void DeAllocSimCases();
    virtual void MakeData(int iSimulationNumber);

};

//*********************************************************************************
#endif

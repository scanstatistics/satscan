//*********************************************************************************
#ifndef __PURELYTEMPORALDATA_H
#define __PURELYTEMPORALDATA_H
//*********************************************************************************
#include "SaTScanData.h"

class CPurelyTemporalData : public CSaTScanData {
  private:
    virtual void        SetProbabilityModel();

  public:
    CPurelyTemporalData(CParameters* pParameters, BasePrint *pPrintDirection);
    virtual ~CPurelyTemporalData();

    virtual void        AdjustNeighborCounts();
    virtual void        AllocSimCases();
    virtual bool        CalculateMeasure();
    virtual void        DeAllocSimCases();
    virtual void        DisplayCases(FILE* pFile);
    virtual void        DisplayMeasure(FILE* pFile);
    virtual void        DisplaySimCases(FILE* pFile);
    virtual bool        FindNeighbors() {return true;/** no action - all locations included */}
    virtual tract_t     GetNeighbor(int iEllipse, tract_t t, unsigned int nearness) const;
    virtual void        MakeData(int iSimulationNumber);
    virtual void        PrintNeighbors();
    virtual void        ReadDataFromFiles();
};

//*********************************************************************************
#endif

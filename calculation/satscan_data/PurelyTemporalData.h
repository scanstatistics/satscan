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
    virtual bool        CalculateMeasure(DataStream & thisStream);
    virtual void        DisplayCases(FILE* pFile);
    virtual void        DisplayMeasure(FILE* pFile);
    virtual void        DisplaySimCases(FILE* pFile);
    virtual bool        FindNeighbors(bool bSimulations) {return true;/** no action - all locations included */}
    virtual tract_t     GetNeighbor(int iEllipse, tract_t t, unsigned int nearness) const;
    virtual void        RandomizeData(int iSimulationNumber);
    virtual void        ReadDataFromFiles();
};

//*********************************************************************************
#endif

//*********************************************************************************
#ifndef __PURELYTEMPORALDATA_H
#define __PURELYTEMPORALDATA_H
//*********************************************************************************
#include "SaTScanData.h"

class CPurelyTemporalData : public CSaTScanData {
  private:
    virtual void        SetProbabilityModel();

  public:
    CPurelyTemporalData(const CParameters* pParameters, BasePrint *pPrintDirection);
    virtual ~CPurelyTemporalData();

    virtual void        AdjustNeighborCounts();
    virtual void        CalculateMeasure(RealDataStream & thisStream);
    virtual void        DisplayCases(FILE* pFile);
    virtual void        DisplayMeasure(FILE* pFile);
    virtual void        DisplaySimCases(FILE* pFile);
    virtual void        FindNeighbors(bool bSimulations) {/** no action - all locations included */}
    virtual tract_t     GetNeighbor(int iEllipse, tract_t t, unsigned int nearness) const;
    virtual void        RandomizeData(SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber);
    virtual void        RandomizeIsolatedData(RandomizerContainer_t& RandomizerContainer,
                                              SimulationDataContainer_t& SimDataContainer,
                                              unsigned int iSimulationNumber) const;
    virtual void        ReadDataFromFiles();
};

//*********************************************************************************
#endif

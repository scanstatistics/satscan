//**************************************************************************
#ifndef __SPACETIMEDATA_H
#define __SPACETIMEDATA_H
//**************************************************************************
#include "SaTScanData.h"

class CSpaceTimeData : public CSaTScanData {
  protected:
    virtual void        CalculateMeasure(RealDataStream& thisStream);
    virtual void        ReadDataFromFiles();
    virtual void        SetIntervalCut();
    virtual void        SetProbabilityModel();

  public:
    CSpaceTimeData(const CParameters* pParameters, BasePrint *pPrintDirection);
    virtual ~CSpaceTimeData();

    virtual void        RandomizeData(SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber);
    virtual void        RandomizeIsolatedData(RandomizerContainer_t& RandomizerContainer,
                                              SimulationDataContainer_t& SimDataContainer,
                                              unsigned int iSimulationNumber) const;
};

//**************************************************************************
#endif

//**************************************************************************
#ifndef __SPACETIMEDATA_H
#define __SPACETIMEDATA_H
//**************************************************************************
#include "SaTScanData.h"

/** Data hub class that derives from base class to define alternate functionality
    for space-time analyses.
    The code to generate data structures for 'number of cases', 'number of
    expected cases', 'simulated cases', etc. is designed to modify a multiple
    dimension array (time intervals by tracts) in a cumulative manner(in respect
    to time). The primary purpose of this class is to direct datasets to set
    corresponding temporal structures from data of multiple dimension array IF
    purely temporal clusters were requested. */
class CSpaceTimeData : public CSaTScanData {
  protected:
    virtual void        CalculateMeasure(RealDataSet& DataSet);
    virtual void        PostDataRead();
    virtual void        SetIntervalCut();
    virtual void        SetProbabilityModel();

  public:
    CSpaceTimeData(const CParameters& Parameters, BasePrint& PrintDirection);
    virtual ~CSpaceTimeData();

    virtual void        RandomizeData(RandomizerContainer_t& RandomizerContainer,
                                      SimulationDataContainer_t& SimDataContainer,
                                      unsigned int iSimulationNumber) const;
};
//**************************************************************************
#endif

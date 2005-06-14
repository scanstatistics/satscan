//*********************************************************************************
#ifndef __PURELYTEMPORALDATA_H
#define __PURELYTEMPORALDATA_H
//*********************************************************************************
#include "SaTScanData.h"

/** Data hub class that derives from base class to define alternate functionality
    for purely temporal analyses.
    The code to generate data structures for 'number of cases', 'number of
    expected cases', 'simulated cases', etc. is designed to modify a multiple
    dimension array (time intervals by tracts) in a cumulative manner(in respect
    to time). The primary purpose of this class is to direct dataset to set
    corresponding temporal structures from data of multiple dimension array. */
class CPurelyTemporalData : public CSaTScanData {
  private:
    virtual void        SetProbabilityModel();

  public:
    CPurelyTemporalData(const CParameters& Parameters, BasePrint& PrintDirection);
    virtual ~CPurelyTemporalData();

    virtual void        AdjustNeighborCounts();
    virtual void        CalculateMeasure(RealDataSet& DataSet);
    virtual void        DisplayCases(FILE* pFile);
    virtual void        DisplayMeasure(FILE* pFile);
    virtual void        DisplaySimCases(FILE* pFile);
    virtual void        FindNeighbors() {/** no action - all locations included */}
    virtual tract_t     GetNeighbor(int iEllipse, tract_t t, unsigned int nearness, double dMaxRadius) const;
    virtual void        RandomizeData(RandomizerContainer_t& RandomizerContainer,
                                      SimulationDataContainer_t& SimDataContainer,
                                      unsigned int iSimulationNumber) const;
    virtual void        ReadDataFromFiles();
};
//*********************************************************************************
#endif

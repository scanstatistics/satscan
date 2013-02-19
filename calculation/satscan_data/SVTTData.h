//*****************************************************************************
#ifndef __SVTTDATA_H
#define __SVTTDATA_H
//*****************************************************************************
#include "SaTScanData.h"

/** Data hub class that derives from base class to define alternate functionality
    for spatial variation in temporal trends analyses.
    The code to generate data structures for 'number of cases', 'number of
    expected cases', 'simulated cases', etc. is designed to modify a multiple
    dimension array (time intervals by tracts) in a cumulative manner(in respect
    to time). The primary purpose of this class is to direct datasets to set
    corresponding temporal and non cumulative structures from data of multiple
    dimension array. */
class CSVTTData : public CSaTScanData {
  private:
    virtual void        DisplayCounts(FILE* pFile, 
                                      count_t** pCounts, const char* szVarName, 
                                      count_t** pCountsNC, const char* szVarNameNC, 
                                      count_t*  pCountsTI, const char* szVarNameTI, const char* szTitle=NULL) const;
    virtual void        SetProbabilityModel();

  protected:
    virtual void        ReadDataFromFiles();
    virtual void        RemoveClusterSignificance(const CCluster& Cluster);
    virtual void        SetIntervalStartTimes();

  public:
    CSVTTData(const CParameters& Parameters, BasePrint& PrintDirection);
    virtual ~CSVTTData();

    virtual void        DisplayCases(FILE* pFile) const;
    virtual void        DisplaySimCases(SimulationDataContainer_t& Container, FILE* pFile) const;
    virtual void        DisplayMeasures(FILE* pFile) const;
    virtual void        DisplayRelativeRisksForEachTract() const;
    virtual void        RandomizeData(RandomizerContainer_t& RandomizerContainer,
                                      SimulationDataContainer_t& SimDataContainer,
                                      unsigned int iSimulationNumber) const;
};
//*****************************************************************************
#endif

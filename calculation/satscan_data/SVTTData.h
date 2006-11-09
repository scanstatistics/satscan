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
    virtual void        DisplayCounts(FILE* pFile, count_t** pCounts,
                                      char* szVarName, count_t** pCountsNC,
                                      char* szVarNameNC, count_t*  pCountsTI,
                                      char* szVarNameTI, char* szTitle=NULL);
    virtual void        SetProbabilityModel();

  protected:
    virtual void        ReadDataFromFiles();
    virtual void        RemoveClusterSignificance(const CCluster& Cluster);

  public:
    CSVTTData(const CParameters& Parameters, BasePrint& PrintDirection);
    virtual ~CSVTTData();

    virtual void        DisplayCases(FILE* pFile);
    virtual void        DisplaySimCases(FILE* pFile);
    virtual void        DisplayMeasures(FILE* pFile);
    virtual void        DisplayRelativeRisksForEachTract() const;
    virtual void        RandomizeData(RandomizerContainer_t& RandomizerContainer,
                                      SimulationDataContainer_t& SimDataContainer,
                                      unsigned int iSimulationNumber) const;
};
//*****************************************************************************
#endif

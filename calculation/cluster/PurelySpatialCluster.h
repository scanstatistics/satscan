//*****************************************************************************
#ifndef __PURELYSPATIALCLUSTER_H
#define __PURELYSPATIALCLUSTER_H
//*****************************************************************************
#include "Cluster.h"
#include "UtilityFunctions.h"

class CPurelySpatialCluster : public CCluster
{
  public:

    CPurelySpatialCluster(BasePrint *pPrintDirection);
    CPurelySpatialCluster(int nRate, tract_t nCircles);
    ~CPurelySpatialCluster();

    virtual void Initialize(tract_t nCenter);
    virtual void DisplayTimeFrame(FILE* fp, char* szSpacesOnLeft, int nAnalysisType);
    virtual void SetStartAndEndDates(const Julian* pIntervalStartTimes, int nTimeIntervals);
    virtual void AddNeighbor(int iEllipse, const CSaTScanData& Data, count_t** pCases, tract_t n);
};

//*****************************************************************************
#endif
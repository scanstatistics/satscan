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
    virtual ~CPurelySpatialCluster();

    virtual void                        AddNeighbor(int iEllipse, const CSaTScanData& Data, count_t** pCases, tract_t n);
    virtual CPurelySpatialCluster     * Clone() const;
    virtual void                        DisplayTimeFrame(FILE* fp, char* szSpacesOnLeft, int nAnalysisType);
    virtual count_t                     GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual measure_t                   GetMeasureForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual void                        Initialize(tract_t nCenter);
    virtual void                        SetStartAndEndDates(const Julian* pIntervalStartTimes, int nTimeIntervals);
};
//*****************************************************************************
#endif
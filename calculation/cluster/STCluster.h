// STCluster.h

#ifndef __STCLUSTER_H
#define __STCLUSTER_H

#include "Cluster.h"
#include "TimeIntervals.h"

class CSpaceTimeCluster : public CCluster
{
  public:
    CSpaceTimeCluster(int nTIType, int nIntervals, int nMaxIntervals);
    virtual ~CSpaceTimeCluster();

    CSpaceTimeCluster& operator =(const CSpaceTimeCluster& cluster);

    virtual void Initialize(tract_t nCenter);

    virtual void InitTimeIntervalIndeces();
    void DeAllocCumulativeCounts();

    virtual bool SetNextTimeInterval();

    virtual void AddNeighbor(const CSaTScanData& Data, count_t** pCases, tract_t n);

  protected:
    count_t*   m_pCumCases;
    measure_t* m_pCumMeasure;
    int        m_nTotalIntervals;
    int        m_nIntervalCut;

    CTimeIntervals* TI;
};

#endif

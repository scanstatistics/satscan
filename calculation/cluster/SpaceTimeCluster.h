//*****************************************************************************
#ifndef __SPACETIMECLUSTER_H
#define __SPACETIMECLUSTER_H
//*****************************************************************************
#include "Cluster.h"
#include "TimeIntervalAll.h"
#include "TimeIntervalAlive.h"

class CSpaceTimeCluster : public CCluster
{
  public:
    CSpaceTimeCluster(int nTIType, int nIntervals, int nMaxIntervals, BasePrint *pPrintDirection);
    virtual ~CSpaceTimeCluster();

    CSpaceTimeCluster& operator =(const CSpaceTimeCluster& cluster);

    virtual void Initialize(tract_t nCenter);
    virtual void InitTimeIntervalIndeces(int nLow, int nHigh);
    virtual void InitTimeIntervalIndeces();
    void DeAllocCumulativeCounts();
    inline virtual bool SetNextTimeInterval();
    virtual bool SetNextProspTimeInterval();
    virtual void AddNeighbor(int iEllipse, const CSaTScanData& Data, count_t** pCases, tract_t n);
    void GetMeasure();

  protected:
    count_t*   m_pCumCases;
    measure_t* m_pCumMeasure;
    int        m_nTotalIntervals;
    int        m_nIntervalCut;

    CTimeIntervals* TI;
};
//*****************************************************************************
#endif

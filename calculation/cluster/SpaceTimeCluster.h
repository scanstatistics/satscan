//*****************************************************************************
#ifndef __SPACETIMECLUSTER_H
#define __SPACETIMECLUSTER_H
//*****************************************************************************
#include "Cluster.h"
#include "TimeIntervalAll.h"
#include "TimeIntervalAlive.h"

class CSpaceTimeCluster : public CCluster {
  private:
    void                        Init() {TI=0;m_pCumCases=0;m_pCumMeasure=0;}
    void                        Setup(IncludeClustersType eTIType, int nIntervals, int nIntervalCut);
    
  protected:
    count_t                   * m_pCumCases;
    measure_t                 * m_pCumMeasure;
    int                         m_nTotalIntervals;
    int                         m_nIntervalCut;
    IncludeClustersType         m_nTIType;
    CTimeIntervals            * TI;

  public:
                CSpaceTimeCluster(IncludeClustersType eTIType, int nIntervals, int nIntervalCut, BasePrint *pPrintDirection);
    virtual     ~CSpaceTimeCluster();

    CSpaceTimeCluster         & operator =(const CSpaceTimeCluster& cluster);

    virtual void                AddNeighbor(int iEllipse, const CSaTScanData& Data, count_t** pCases, tract_t n);
    virtual CSpaceTimeCluster * Clone() const;
    void                        DeAllocCumulativeCounts();
    virtual count_t             GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual measure_t           GetMeasureForTract(tract_t tTract, const CSaTScanData& Data) const;
    void                        GetMeasure();
    virtual void                Initialize(tract_t nCenter);
    virtual void                InitTimeIntervalIndeces();
    virtual void                InitTimeIntervalIndeces(int nLow, int nHigh);
    virtual bool                SetNextProspTimeInterval();
    inline virtual bool         SetNextTimeInterval();
};
//*****************************************************************************
#endif

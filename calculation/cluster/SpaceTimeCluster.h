//*****************************************************************************
#ifndef __SPACETIMECLUSTER_H
#define __SPACETIMECLUSTER_H
//*****************************************************************************
#include "Cluster.h"
#include "TimeIntervalAll.h"
#include "TimeIntervalAlive.h"
#include "TimeIntervalRange.h"

class CSpaceTimeCluster : public CCluster {
  private:
    void                        Init() {TI=0;m_pCumCases=0;m_pCumMeasure=0;}
    void                        Setup(IncludeClustersType eIncludeClustersType, const CSaTScanData & Data);
    void                        Setup(const CSpaceTimeCluster& rhs);

  protected:
    count_t                   * m_pCumCases;
    measure_t                 * m_pCumMeasure;
    int                         m_nTotalIntervals;
    int                         m_nIntervalCut;
    IncludeClustersType         m_nTIType;
    CTimeIntervals            * TI;

  public:
    CSpaceTimeCluster(IncludeClustersType eIncludeClustersType, const CSaTScanData & Data, BasePrint & PrintDirection);
    CSpaceTimeCluster(const CSpaceTimeCluster& rhs);
    virtual ~CSpaceTimeCluster();

    CSpaceTimeCluster         & operator =(const CSpaceTimeCluster& cluster);

    inline virtual void         AssignAsType(const CCluster& rhs) {*this = (CSpaceTimeCluster&)rhs;}
    virtual void                AddNeighbor(int iEllipse, const CSaTScanData& Data, count_t** pCases, tract_t n);
    virtual CSpaceTimeCluster * Clone() const;
    inline virtual void         CompareTopCluster(CSpaceTimeCluster & TopShapeCluster, const CSaTScanData & Data);
    inline virtual void         ComputeBestMeasures(CMeasureList & MeasureList);
    void                        DeAllocCumulativeCounts();
    virtual count_t             GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual measure_t           GetMeasureForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual void                Initialize(tract_t nCenter);
};
//*****************************************************************************
#endif

//*****************************************************************************
#ifndef __SPACETIMECLUSTER_H
#define __SPACETIMECLUSTER_H
//*****************************************************************************
#include "Cluster.h"
#include "TimeIntervalAll.h"
#include "TimeIntervalAlive.h"
#include "TimeIntervalRange.h"

class SpaceTimeClusterStreamData : public AbstractTemporalClusterStreamData {
   private:
     int                giAllocationSize;

     void               Init() {gpCases=0;gpMeasure=0;gpSqMeasure=0;}
     void               Setup();

   public:
    SpaceTimeClusterStreamData(int unsigned iAllocationSize);
    SpaceTimeClusterStreamData(const SpaceTimeClusterStreamData& rhs);
    virtual ~SpaceTimeClusterStreamData();

    SpaceTimeClusterStreamData        & operator=(const SpaceTimeClusterStreamData& rhs);
    
    virtual SpaceTimeClusterStreamData* Clone() const;
    virtual void                        InitializeData();
};

/** cluster class for space-time analysis (retrospective and prospective) */
class CSpaceTimeCluster : public CCluster {
  private:
    void                        Init();
    void                        Setup(IncludeClustersType eIncludeClustersType, const CSaTScanData & Data);
    void                        Setup(const CSpaceTimeCluster& rhs);

  protected:
    StreamDataContainer_t       gStreamData;
    int                         m_nTotalIntervals;
    int                         m_nIntervalCut;
    IncludeClustersType         m_nTIType;
    CTimeIntervals            * TI;

  public:
    CSpaceTimeCluster(IncludeClustersType eIncludeClustersType, const CSaTScanData & Data, BasePrint & PrintDirection);
    CSpaceTimeCluster(const CSpaceTimeCluster& rhs);
    virtual ~CSpaceTimeCluster();

    CSpaceTimeCluster         & operator =(const CSpaceTimeCluster& cluster);

    virtual count_t             GetCaseCount(unsigned int iStream) const {return gStreamData[iStream]->gCases;}
    virtual measure_t           GetMeasure(unsigned int iStream) const {return gStreamData[iStream]->gMeasure;}
    virtual void                SetCaseCount(unsigned int iStream, count_t tCount) {gStreamData[iStream]->gCases = tCount;}
    virtual void                SetMeasure(unsigned int iStream, measure_t tMeasure) {gStreamData[iStream]->gMeasure = tMeasure;}


    inline virtual void         AssignAsType(const CCluster& rhs) {*this = (CSpaceTimeCluster&)rhs;}
    void                        AddNeighbor(tract_t tNeighbor, const DataStreamGateway & DataGateway);
    void                        AddNeighbor(tract_t tNeighbor, const DataStreamInterface & Interface, size_t tStream);
    virtual CSpaceTimeCluster * Clone() const;
    inline virtual void         CompareTopCluster(CSpaceTimeCluster & TopShapeCluster, const CSaTScanData & Data);
    inline virtual void         ComputeBestMeasures(CMeasureList & MeasureList);
    virtual count_t             GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual measure_t           GetMeasureForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual void                Initialize(tract_t nCenter);
};
//*****************************************************************************
#endif

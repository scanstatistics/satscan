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
     void               Init() {gpCases=0;gpMeasure=0;gpSqMeasure=0;}
     void               Setup();

   protected:
     int                giAllocationSize;

   public:
    SpaceTimeClusterStreamData(int unsigned iAllocationSize);
    SpaceTimeClusterStreamData(const SpaceTimeClusterStreamData& rhs);
    virtual ~SpaceTimeClusterStreamData();

    virtual SpaceTimeClusterStreamData  * Clone() const;
    SpaceTimeClusterStreamData          & operator=(const SpaceTimeClusterStreamData& rhs);

    virtual void                        InitializeData();
};

class SpaceTimeClusterStreamDataEx : public SpaceTimeClusterStreamData {
   private:
     void               Setup();

   public:
     SpaceTimeClusterStreamDataEx(int unsigned iAllocationSize);
     SpaceTimeClusterStreamDataEx(const SpaceTimeClusterStreamDataEx& rhs);
     virtual ~SpaceTimeClusterStreamDataEx();

     virtual SpaceTimeClusterStreamDataEx * Clone() const;
     SpaceTimeClusterStreamDataEx         & operator=(const SpaceTimeClusterStreamDataEx& rhs);

     virtual void       InitializeData();
};

/** cluster class for space-time analysis (retrospective and prospective) */
class CSpaceTimeCluster : public CCluster {
  typedef void (CSpaceTimeCluster::* ADDNEIGHBOR)(tract_t tNeighbor, const DataStreamInterface & Interface, size_t tStream=0);

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
    ADDNEIGHBOR                 fAddNeighborData;

  public:
    CSpaceTimeCluster(IncludeClustersType eIncludeClustersType, const CSaTScanData & Data, BasePrint & PrintDirection);
    CSpaceTimeCluster(const CSpaceTimeCluster& rhs);
    virtual ~CSpaceTimeCluster();

    CSpaceTimeCluster         & operator =(const CSpaceTimeCluster& cluster);

    inline virtual void         AssignAsType(const CCluster& rhs) {*this = (CSpaceTimeCluster&)rhs;}
    void                        AddNeighbor(tract_t tNeighbor, const DataStreamGateway & DataGateway);
    void                        AddNeighborData(tract_t tNeighbor, const DataStreamInterface & Interface, size_t tStream=0);
    void                        AddNeighborDataEx(tract_t tNeighbor, const DataStreamInterface & Interface, size_t tStream=0);
    virtual CSpaceTimeCluster * Clone() const;
    inline virtual void         CompareTopCluster(CSpaceTimeCluster & TopShapeCluster, const CSaTScanData & Data);
    inline virtual void         ComputeBestMeasures(CMeasureList & MeasureList);
    virtual count_t             GetCaseCount(unsigned int iStream) const {return gStreamData[iStream]->gCases;}
    virtual count_t             GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual measure_t           GetMeasure(unsigned int iStream) const {return gStreamData[iStream]->gMeasure;}
    virtual measure_t           GetMeasureForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual void                Initialize(tract_t nCenter);
    virtual void                SetCaseCount(unsigned int iStream, count_t tCount) {gStreamData[iStream]->gCases = tCount;}
    virtual void                SetMeasure(unsigned int iStream, measure_t tMeasure) {gStreamData[iStream]->gMeasure = tMeasure;}
};
//*****************************************************************************
#endif

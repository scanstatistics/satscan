//*****************************************************************************
#ifndef __SPACETIMECLUSTER_H
#define __SPACETIMECLUSTER_H
//*****************************************************************************
#include "cluster.h"
//#include "TimeIntervalAll.h"
//#include "TimeIntervalAlive.h"
#include "TimeIntervalRange.h"
#include "ClusterDataFactory.h"

/** cluster class for space-time analysis (retrospective and prospective) */
class CSpaceTimeCluster : public CCluster {
  private:
    void                                  Init() {gpClusterData=0;}
    void                                  Setup(const AbstractClusterDataFactory * pClusterFactory, const AbtractDataStreamGateway & DataGateway);

  protected:
    AbstractTemporalClusterData         * gpClusterData;

  public:
    CSpaceTimeCluster(const AbstractClusterDataFactory * pClusterFactory, const AbtractDataStreamGateway & DataGateway, BasePrint & PrintDirection);
    CSpaceTimeCluster(const CSpaceTimeCluster& rhs);
    virtual ~CSpaceTimeCluster();

    inline virtual void                   AssignAsType(const CCluster& rhs) {*this = (CSpaceTimeCluster&)rhs;}
    virtual CSpaceTimeCluster           * Clone() const;
    CSpaceTimeCluster                   & operator =(const CSpaceTimeCluster& cluster);

    void                                  AddNeighborDataAndCompare(const AbtractDataStreamGateway & DataGateway,
                                                                    const CSaTScanData * pData,
                                                                    CSpaceTimeCluster & TopCluster,
                                                                    CTimeIntervals * pTimeIntervals);
    virtual count_t                       GetCaseCount(unsigned int iStream) const {return gpClusterData->GetCaseCount(iStream);}
    virtual count_t                       GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual AbstractClusterData         * GetClusterData() {return gpClusterData;}
    virtual int                           GetClusterType() const {return SPACETIME;}
    virtual measure_t                     GetMeasure(unsigned int iStream) const {return gpClusterData->GetMeasure(iStream);}
    virtual measure_t                     GetMeasureForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual void                          Initialize(tract_t nCenter=0);
    virtual void                          SetCaseCount(unsigned int iStream, count_t tCount) {gpClusterData->SetCaseCount(iStream, tCount);}
    virtual void                          SetMeasure(unsigned int iStream, measure_t tMeasure) {gpClusterData->SetMeasure(iStream, tMeasure);}
};
//*****************************************************************************
#endif

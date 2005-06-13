//*****************************************************************************
#ifndef __SPACETIMECLUSTER_H
#define __SPACETIMECLUSTER_H
//*****************************************************************************
#include "cluster.h"
#include "TimeIntervalRange.h"
#include "ClusterDataFactory.h"

/** cluster class for space-time analysis (retrospective and prospective) */
class CSpaceTimeCluster : public CCluster {
  private:
    void                                  Init() {gpClusterData=0;}
    void                                  Setup(const AbstractClusterDataFactory * pClusterFactory, const AbtractDataSetGateway & DataGateway);

  protected:
    AbstractTemporalClusterData         * gpClusterData;

  public:
    CSpaceTimeCluster(const AbstractClusterDataFactory * pClusterFactory, const AbtractDataSetGateway & DataGateway);
    CSpaceTimeCluster(const CSpaceTimeCluster& rhs);
    virtual ~CSpaceTimeCluster();

    inline virtual void                   AssignAsType(const CCluster& rhs) {*this = (CSpaceTimeCluster&)rhs;}
    virtual CSpaceTimeCluster           * Clone() const;
    CSpaceTimeCluster                   & operator =(const CSpaceTimeCluster& cluster);

    virtual AbstractClusterData         * GetClusterData() {return gpClusterData;}
    virtual const AbstractClusterData   * GetClusterData() const {return gpClusterData;}
    virtual ClusterType                   GetClusterType() const {return SPACETIMECLUSTER;}
    virtual measure_t                     GetExpectedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex=0) const;
    virtual count_t                       GetObservedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex=0) const;
    void                                  CalculateTopClusterAboutCentroidDefinition(const AbtractDataSetGateway & DataGateway,
                                                                                     const CentroidNeighbors& CentroidDef,
                                                                                     CSpaceTimeCluster & TopCluster,
                                                                                     CTimeIntervals& TimeIntervals);
    virtual void                          Initialize(tract_t nCenter=0);
};
//*****************************************************************************
#endif


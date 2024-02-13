//*****************************************************************************
#ifndef __SPACETIMECLUSTER_H
#define __SPACETIMECLUSTER_H
//*****************************************************************************
#include "cluster.h"
#include "TimeIntervalRange.h"
#include "ClusterDataFactory.h"
#include "IntermediateClustersContainer.h"

/** cluster class for space-time analysis (retrospective and prospective) */
class CSpaceTimeCluster : public CCluster {
  protected:
    AbstractTemporalClusterData         * gpClusterData;

  public:
    CSpaceTimeCluster(const AbstractClusterDataFactory * pClusterFactory, const AbstractDataSetGateway & DataGateway);
    CSpaceTimeCluster(const CSpaceTimeCluster& rhs);
    virtual ~CSpaceTimeCluster();

    virtual CSpaceTimeCluster           * Clone() const;
    CSpaceTimeCluster                   & operator =(const CSpaceTimeCluster& cluster);

    virtual AbstractClusterData         * GetClusterData() {return gpClusterData;}
    virtual const AbstractClusterData   * GetClusterData() const {return gpClusterData;}
    virtual ClusterType                   GetClusterType() const {return SPACETIMECLUSTER;}
    virtual measure_t                     GetExpectedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex=0, bool adjusted=true) const;
    virtual count_t                       GetObservedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex=0) const;
	virtual count_t                       GetCountForTractOutside(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex = 0) const;
    void                                  CalculateTopClusterAboutCentroidDefinition(const AbstractDataSetGateway& DataGateway,
                                                                                     const CentroidNeighbors& CentroidDef,
                                                                                     CClusterSet& clusterSet,
                                                                                     CTimeIntervals& TimeIntervals);
    virtual void                          CopyEssentialClassMembers(const CCluster& rhs);
    virtual void                          Initialize(tract_t nCenter=0);
};
//*****************************************************************************
#endif


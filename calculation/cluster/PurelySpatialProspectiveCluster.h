//*****************************************************************************
#ifndef __PURELYSPATIALPROSPECTIVECLUSTER_H
#define __PURELYSPATIALPROSPECTIVECLUSTER_H
//*****************************************************************************
#include "Cluster.h"
#include "ClusterDataFactory.h"
#include "UtilityFunctions.h"

class CMeasureList; /** class declaration defined in other unit */

/** cluster class for purely spatial cluster used in replications of prospective
    space-time analysis */
class CPurelySpatialProspectiveCluster : public CCluster {
  private:
    AbstractTemporalClusterData              * gpClusterData;

    void                        	       Init() {gpClusterData=0;}
    void			               Setup(const AbstractClusterDataFactory * pClusterFactory, const AbtractDataStreamGateway & DataGateway, const CSaTScanData & Data);

  public:
    CPurelySpatialProspectiveCluster(const AbstractClusterDataFactory * pClusterFactory, const AbtractDataStreamGateway & DataGateway,
                                     const CSaTScanData & Data, BasePrint *pPrintDirection);
    CPurelySpatialProspectiveCluster(const CPurelySpatialProspectiveCluster& rhs);
    virtual ~CPurelySpatialProspectiveCluster();

     CPurelySpatialProspectiveCluster        & operator=(const CPurelySpatialProspectiveCluster& cluster);

    void                                       AddNeighborAndCompare(const AbtractDataStreamGateway & DataGateway,
                                                             const CSaTScanData * pData,
                                                             CPurelySpatialProspectiveCluster & TopCluster,
                                                             CModel & Model);
    inline virtual void                        AssignAsType(const CCluster& rhs) {*this = (CPurelySpatialProspectiveCluster&)rhs;}
    virtual CPurelySpatialProspectiveCluster * Clone() const;
    virtual count_t                            GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual AbstractClusterData              * GetClusterData() {return gpClusterData;}
    virtual int                                GetClusterType() const {return PURELYSPATIAL;}
    virtual measure_t                          GetMeasureForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual void                               Initialize(tract_t nCenter);
    virtual void                               SetStartAndEndDates(const Julian* pIntervalStartTimes, int nTimeIntervals);
};
//*****************************************************************************
#endif
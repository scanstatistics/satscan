//*****************************************************************************
#ifndef __PURELYSPATIALPROSPECTIVECLUSTER_H
#define __PURELYSPATIALPROSPECTIVECLUSTER_H
//*****************************************************************************
#include "cluster.h"
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
    CPurelySpatialProspectiveCluster(const AbstractClusterDataFactory * pClusterFactory,
                                     const AbtractDataStreamGateway & DataGateway,
                                     const CSaTScanData & Data);
    CPurelySpatialProspectiveCluster(const CPurelySpatialProspectiveCluster& rhs);
    virtual ~CPurelySpatialProspectiveCluster();

     CPurelySpatialProspectiveCluster        & operator=(const CPurelySpatialProspectiveCluster& cluster);

    void                                       AddNeighborAndCompare(tract_t tEllipseOffset,
                                                                     tract_t tCentroid,
                                                                     const AbtractDataStreamGateway & DataGateway,
                                                                     const CSaTScanData * pData,
                                                                     CPurelySpatialProspectiveCluster & TopCluster,
                                                                     AbstractLikelihoodCalculator & Calculator);
    inline virtual void                        AssignAsType(const CCluster& rhs) {*this = (CPurelySpatialProspectiveCluster&)rhs;}
    virtual CPurelySpatialProspectiveCluster * Clone() const;
    virtual count_t                            GetCaseCount(unsigned int iStream) const {return gpClusterData->GetCaseCount(iStream);}
    virtual count_t                            GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data, unsigned int iStream=0) const;
    virtual AbstractClusterData              * GetClusterData() {return gpClusterData;}
    virtual ClusterType                        GetClusterType() const {return PURELYSPATIALPROSPECTIVECLUSTER;}
    virtual ZdString                         & GetEndDate(ZdString& sDateString, const CSaTScanData& DataHub) const;
    virtual measure_t                          GetMeasure(unsigned int iStream) const {return gpClusterData->GetMeasure(iStream);}
    virtual measure_t                          GetMeasureForTract(tract_t tTract, const CSaTScanData& Data, unsigned int iStream=0) const;
    virtual ZdString                         & GetStartDate(ZdString& sDateString, const CSaTScanData& DataHub) const;
    virtual void                               Initialize(tract_t nCenter);
};
//*****************************************************************************
#endif

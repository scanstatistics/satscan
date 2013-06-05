//*****************************************************************************
#ifndef __PURELYSPATIALPROSPECTIVECLUSTER_H
#define __PURELYSPATIALPROSPECTIVECLUSTER_H
//*****************************************************************************
#include "cluster.h"
#include "ClusterDataFactory.h"
#include "UtilityFunctions.h"
#include "IntermediateClustersContainer.h"

class CMeasureList; /** class declaration defined in other unit */

/** cluster class for purely spatial cluster used in replications of prospective
    space-time analysis */
class CPurelySpatialProspectiveCluster : public CCluster {
  private:
    AbstractTemporalClusterData              * gpClusterData;

    void                        	       Init() {gpClusterData=0;}
    void			               Setup(const AbstractClusterDataFactory * pClusterFactory, const AbstractDataSetGateway & DataGateway, const CSaTScanData & Data);

  public:
    CPurelySpatialProspectiveCluster(const AbstractClusterDataFactory * pClusterFactory,
                                     const AbstractDataSetGateway & DataGateway,
                                     const CSaTScanData & Data);
    CPurelySpatialProspectiveCluster(const CPurelySpatialProspectiveCluster& rhs);
    virtual ~CPurelySpatialProspectiveCluster();

    virtual CPurelySpatialProspectiveCluster * Clone() const;
    CPurelySpatialProspectiveCluster         & operator=(const CPurelySpatialProspectiveCluster& cluster);

    void                                       CalculateTopClusterAboutCentroidDefinition(const AbstractDataSetGateway& DataGateway,
                                                                                          const CentroidNeighbors& CentroidDef,
                                                                                          CClusterSet& clusterSet,
                                                                                          AbstractLikelihoodCalculator& Calculator);
    virtual void                               CopyEssentialClassMembers(const CCluster& rhs);
    virtual AbstractClusterData              * GetClusterData() {return gpClusterData;}
    virtual const AbstractClusterData        * GetClusterData() const {return gpClusterData;}
    virtual ClusterType                        GetClusterType() const {return PURELYSPATIALPROSPECTIVECLUSTER;}
    virtual std::string                      & GetEndDate(std::string& sDateString, const CSaTScanData& DataHub, const char * sep="/") const;
    virtual measure_t                          GetExpectedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex=0) const;
    virtual count_t                            GetObservedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex=0) const;
    virtual std::string                      & GetStartDate(std::string& sDateString, const CSaTScanData& DataHub, const char * sep="/") const;
    virtual void                               Initialize(tract_t nCenter);
};
//*****************************************************************************
#endif

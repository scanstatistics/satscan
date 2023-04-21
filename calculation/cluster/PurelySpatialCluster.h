//*****************************************************************************
#ifndef __PURELYSPATIALCLUSTER_H
#define __PURELYSPATIALCLUSTER_H
//*****************************************************************************
#include "cluster.h"
#include "UtilityFunctions.h"
#include "MeasureList.h"
#include "ClusterDataFactory.h"
#include "IntermediateClustersContainer.h"

//class CPurelySpatialCluster;
//typedef ClusterSet<CPurelySpatialCluster> clusters_set_t;

/** cluster class for purely spatial analysis and purely spatial cluster
    of space-time analysis */
class CPurelySpatialCluster : public CCluster {
  private:
    AbstractSpatialClusterData        * gpClusterData;

  public:
    CPurelySpatialCluster(const AbstractClusterDataFactory * pClusterFactory, const AbstractDataSetGateway & DataGateway);
    CPurelySpatialCluster(const CPurelySpatialCluster& rhs);
    virtual ~CPurelySpatialCluster();

    CPurelySpatialCluster                    & operator=(const CPurelySpatialCluster& rhs);
    virtual CPurelySpatialCluster            * Clone() const;
    virtual inline AbstractClusterData       * GetClusterData() {return gpClusterData;}
    virtual inline const AbstractClusterData * GetClusterData() const {return gpClusterData;}


    void                                CalculateTopClusterAboutCentroidDefinition(const AbstractDataSetGateway& DataGateway,
                                                                                   const CentroidNeighbors& CentroidDef,
                                                                                   CClusterSet& clusterSet,
                                                                                   AbstractLikelihoodCalculator& Calculator);
    virtual void                        CopyEssentialClassMembers(const CCluster& rhs);
    virtual void                        DisplayTimeFrame(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const;
    virtual ClusterType                 GetClusterType() const {return PURELYSPATIALCLUSTER;}
    virtual std::string               & GetEndDate(std::string& sDateString, const CSaTScanData& DataHub, const char * sep="/") const;
    virtual measure_t                   GetExpectedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex=0, bool adjusted=true) const;
    virtual count_t                     GetObservedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex=0) const;
    virtual std::string               & GetStartDate(std::string& sDateString, const CSaTScanData& DataHub, const char * sep="/") const;
    virtual void                        Initialize(tract_t nCenter=0);
};
//*****************************************************************************
#endif

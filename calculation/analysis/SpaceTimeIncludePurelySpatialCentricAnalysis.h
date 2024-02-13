//******************************************************************************
#ifndef __SpaceTimeIncludePurelySpatialCentricAnalysis_H
#define __SpaceTimeIncludePurelySpatialCentricAnalysis_H
//******************************************************************************
#include "SpaceTimeCentricAnalysis.h"
#include "PurelySpatialCluster.h"

/** Defines inteface for space-time analysis that includes purely spatial clusters,
    evaluating real and simulated data for each centroid separate than other centroids. */
class  SpaceTimeIncludePurelySpatialCentricAnalysis : public SpaceTimeCentricAnalysis {
  protected:
    CClusterSetCollections _top_ps_Clusters; // collection of clusters representing top cluster - used to evaluate real data
    boost::shared_ptr<CPurelySpatialCluster> _ps_cluster_compare; // instance of spatial cluster - used to evaluate real data
    boost::shared_ptr<SpatialData> _ps_cluster_data;
    boost::shared_ptr<ProspectiveSpatialData> _psp_cluster_data;
    boost::shared_ptr<AbstractClusterData> _ps_cluster_data_cl_eval;

    virtual void                        CalculateRatiosAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const DataSetGatewayContainer_t& vDataGateways);
    virtual void                        CalculateTopClusterAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const AbstractDataSetGateway& DataGateway);
    virtual const SharedClusterVector_t GetTopCalculatedClusters();
    virtual void                        MonteCarloAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const DataSetGatewayContainer_t& vDataGateways);
    void                                MonteCarloProspectiveAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const DataSetGatewayContainer_t& vDataGateways);

  public:
    SpaceTimeIncludePurelySpatialCentricAnalysis(const CParameters& Parameters, const CSaTScanData& Data, BasePrint& PrintDirection,
                                                 const AbstractDataSetGateway& RealDataGateway, const DataSetGatewayContainer_t& vSimDataGateways);
    virtual ~SpaceTimeIncludePurelySpatialCentricAnalysis() {}
};

/** Defines inteface for space-time analysis that includes purely temporal and spatial
    clusters, evaluating real and simulated data for each centroid separate than other centroids. */
class SpaceTimeIncludePureCentricAnalysis : public SpaceTimeIncludePurelySpatialCentricAnalysis {
public:
    SpaceTimeIncludePureCentricAnalysis(const CParameters& Parameters, const CSaTScanData& Data, BasePrint& PrintDirection,
        const AbstractDataSetGateway& RealDataGateway, const ptr_vector<AbstractDataSetGateway>& vSimDataGateways);
    virtual ~SpaceTimeIncludePureCentricAnalysis() {}
};
//******************************************************************************
#endif


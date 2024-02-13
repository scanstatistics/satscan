//******************************************************************************
#ifndef __PurelySpatialCentricAnalysis_H
#define __PurelySpatialCentricAnalysis_H
//******************************************************************************
#include "AbstractCentricAnalysis.h"
#include "ClusterData.h"
#include "IntermediateClustersContainer.h"
#include "PurelySpatialCluster.h"

/** Defines purely spatial analysis class which evaluates real and simulated data
    for each centroid separate than other centroids. */
class PurelySpatialCentricAnalysis : public AbstractCentricAnalysis {
    private:
        CClusterSetCollections _top_clusters; // collection of clusters representing top cluster - used to evaluate real data
        boost::shared_ptr<CPurelySpatialCluster> _cluster_compare; // instance of purely spatial cluster - used to evaluate real data
        boost::shared_ptr<SpatialData> _cluster_data; // concrete instance of spatial cluster data object - used by simulation process
        boost::shared_ptr<AbstractSpatialClusterData> _cluster_data_sim; // abstract instance of spatial cluster data object - used by simulation process

    protected:
        virtual void                        CalculateRatiosAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const DataSetGatewayContainer_t& vDataGateways);
        virtual void                        CalculateTopClusterAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const AbstractDataSetGateway& DataGateway);
        virtual const SharedClusterVector_t GetTopCalculatedClusters();
        virtual void                        MonteCarloAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const DataSetGatewayContainer_t& vDataGateways);

    public:
        PurelySpatialCentricAnalysis(const CParameters& Parameters, const CSaTScanData& Data, BasePrint& PrintDirection,
                                     const AbstractDataSetGateway& RealDataGateway, const DataSetGatewayContainer_t& vSimDataGateways);
        virtual ~PurelySpatialCentricAnalysis() {}
};
//******************************************************************************
#endif

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
  typedef IntermediateClustersContainer<CPurelySpatialCluster> intermediate_clusters_t;

  private:
    intermediate_clusters_t                   gTopShapeClusters;     /** collection of clusters representing top cluster for each shape */
    std::auto_ptr<CPurelySpatialCluster>      gClusterComparator;    /** instance of purely spatial cluster
                                                                         - used to evaluate real data */
    std::auto_ptr<SpatialData>                gClusterData;          /** concrete instance of spatial cluster data object
                                                                         - used by simulation process */
    std::auto_ptr<AbstractSpatialClusterData> gAbstractClusterData;  /** abstract instance of spatial cluster data object
                                                                        - used by simulation process */

    void                     Setup(const AbtractDataSetGateway& RealDataGateway, const DataSetGatewayContainer_t& vSimDataGateways);

  public:
    PurelySpatialCentricAnalysis(const CParameters& Parameters, const CSaTScanData& Data, BasePrint& PrintDirection,
                                 const AbtractDataSetGateway& RealDataGateway, const DataSetGatewayContainer_t& vSimDataGateways);
    virtual ~PurelySpatialCentricAnalysis();

    virtual void             CalculateRatiosAboutCentroidDefinition(CentroidDefinitionContainer_t& vCentroid, const DataSetGatewayContainer_t& vDataGateways);
    virtual const CCluster & CalculateTopClusterAboutCentroidDefinition(CentroidDefinitionContainer_t& vCentroid, const AbtractDataSetGateway& DataGateway);
    virtual void             MonteCarloAboutCentroidDefinition(CentroidDefinitionContainer_t& vCentroid, const DataSetGatewayContainer_t& vDataGateways);
};
//******************************************************************************
#endif

//******************************************************************************
#ifndef __SpaceTimeIncludePurelySpatialCentricAnalysis_H
#define __SpaceTimeIncludePurelySpatialCentricAnalysis_H
//******************************************************************************
#include "SpaceTimeCentricAnalysis.h"
#include "PurelySpatialCluster.h"

/** Defines inteface for purely spatial analysis which evaluates real and simulated
    data for each centroid separate than other centroids. */
class  SpaceTimeIncludePurelySpatialCentricAnalysis : public SpaceTimeCentricAnalysis {
  typedef IntermediateClustersContainer<CPurelySpatialCluster>  intermediate_ps_clusters_t;

  private:
    void                     Setup(const AbtractDataSetGateway& RealDataGateway, const DataSetGatewayContainer_t& vSimDataGateways);

  protected:
    intermediate_ps_clusters_t                       gPSTopShapeClusters;   /** collection of clusters representing top cluster for each shape */
    std::auto_ptr<CPurelySpatialCluster>             gPSClusterComparator;  /** instance of spatial cluster
                                                                               - used to evaluate real data */
    std::auto_ptr<SpatialData>                       gPSClusterData;
    std::auto_ptr<ProspectiveSpatialData>            gPSPClusterData;
    std::auto_ptr<AbstractClusterData>               gPSAbstractClusterData;

    void                     MonteCarloProspectiveAboutCentroidDefinition(CentroidDefinitionContainer_t& vCentroid, const DataSetGatewayContainer_t& vDataGateways);

  public:
    SpaceTimeIncludePurelySpatialCentricAnalysis(const CParameters& Parameters, const CSaTScanData& Data, BasePrint& PrintDirection,
                                                 const AbtractDataSetGateway& RealDataGateway, const DataSetGatewayContainer_t& vSimDataGateways);
    virtual ~SpaceTimeIncludePurelySpatialCentricAnalysis();

    virtual void             CalculateRatiosAboutCentroidDefinition(CentroidDefinitionContainer_t& vCentroid, const DataSetGatewayContainer_t& vDataGateways);
    virtual const CCluster & CalculateTopClusterAboutCentroidDefinition(CentroidDefinitionContainer_t& vCentroid, const AbtractDataSetGateway& DataGateway);
    virtual void             MonteCarloAboutCentroidDefinition(CentroidDefinitionContainer_t& vCentroid, const DataSetGatewayContainer_t& vDataGateways);
};
//******************************************************************************
#endif

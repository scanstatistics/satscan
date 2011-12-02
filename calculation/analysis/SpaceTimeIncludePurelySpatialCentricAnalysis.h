//******************************************************************************
#ifndef __SpaceTimeIncludePurelySpatialCentricAnalysis_H
#define __SpaceTimeIncludePurelySpatialCentricAnalysis_H
//******************************************************************************
#include "SpaceTimeCentricAnalysis.h"
#include "PurelySpatialCluster.h"

/** Defines inteface for space-time analysis that includes purely spatial clusters,
    evaluating real and simulated data for each centroid separate than other centroids. */
class  SpaceTimeIncludePurelySpatialCentricAnalysis : public SpaceTimeCentricAnalysis {
  private:
    void                     Setup(const AbstractDataSetGateway& RealDataGateway, const DataSetGatewayContainer_t& vSimDataGateways);

  protected:
    CClusterSetCollections                           _top_PS_Clusters;      /** collection of clusters representing top cluster
                                                                                - used to evaluate real data */
    std::auto_ptr<CPurelySpatialCluster>             gPSClusterComparator;  /** instance of spatial cluster
                                                                               - used to evaluate real data */
    std::auto_ptr<SpatialData>                       gPSClusterData;
    std::auto_ptr<ProspectiveSpatialData>            gPSPClusterData;
    std::auto_ptr<AbstractClusterData>               gPSAbstractClusterData;

    virtual void                        CalculateRatiosAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const DataSetGatewayContainer_t& vDataGateways);
    virtual void                        CalculateTopClusterAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const AbstractDataSetGateway& DataGateway);
    virtual const SharedClusterVector_t GetTopCalculatedClusters();
    virtual void                        MonteCarloAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const DataSetGatewayContainer_t& vDataGateways);
    void                                MonteCarloProspectiveAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const DataSetGatewayContainer_t& vDataGateways);

  public:
    SpaceTimeIncludePurelySpatialCentricAnalysis(const CParameters& Parameters, const CSaTScanData& Data, BasePrint& PrintDirection,
                                                 const AbstractDataSetGateway& RealDataGateway, const DataSetGatewayContainer_t& vSimDataGateways);
    virtual ~SpaceTimeIncludePurelySpatialCentricAnalysis();
};
//******************************************************************************
#endif


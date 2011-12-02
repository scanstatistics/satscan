//******************************************************************************
#ifndef SVTTCentricAnalysisH
#define SVTTCentricAnalysisH
//******************************************************************************
#include "AbstractCentricAnalysis.h"
#include "SVTTCluster.h"
#include "IntermediateClustersContainer.h"

/** Defines purely spatial analysis class which evaluates real and simulated data
    for each centroid separate than other centroids. */
class SpatialVarTempTrendCentricAnalysis : public AbstractCentricAnalysis {
  private:
    CClusterSetCollections           _topClusters;          /** collection of clusters representing top cluster
                                                                - used to evaluate real data */
    std::auto_ptr<CSVTTCluster>      gClusterComparator;    /** instance of purely spatial cluster
                                                                         - used to evaluate real data */
    std::auto_ptr<SVTTClusterData>   gClusterData;          /** concrete instance of spatial cluster data object
                                                               - used by simulation process */

    void                                Setup(const AbstractDataSetGateway& RealDataGateway, const DataSetGatewayContainer_t& vSimDataGateways);

  protected:


    virtual void                        CalculateRatiosAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const DataSetGatewayContainer_t& vDataGateways);
    virtual void                        CalculateTopClusterAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const AbstractDataSetGateway& DataGateway);
    virtual const SharedClusterVector_t GetTopCalculatedClusters();
    virtual void                        MonteCarloAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const DataSetGatewayContainer_t& vDataGateways);

  public:
    SpatialVarTempTrendCentricAnalysis(const CParameters& Parameters, const CSaTScanData& Data, BasePrint& PrintDirection,
                                 const AbstractDataSetGateway& RealDataGateway, const DataSetGatewayContainer_t& vSimDataGateways);
    virtual ~SpatialVarTempTrendCentricAnalysis();
};
//******************************************************************************
#endif

//******************************************************************************
#ifndef SpaceTimeCentricAnalysisH
#define SpaceTimeCentricAnalysisH
//******************************************************************************
#include "AbstractCentricAnalysis.h"
#include "ClusterData.h"
#include "IntermediateClustersContainer.h"
#include "SpaceTimeCluster.h"

/** Defines centric space-time analysis which evaluates real and simulated
    data for each centroid separate than other centroids. */
class SpaceTimeCentricAnalysis : public AbstractCentricAnalysis {
  typedef IntermediateClustersContainer<CSpaceTimeCluster> intermediate_clusters_t;

  private:
    void                     Setup(const AbtractDataSetGateway& RealDataGateway, const DataSetGatewayContainer_t& vSimDataGateways);

  protected:
    intermediate_clusters_t                    gTopShapeClusters;     /** collection of clusters representing top cluster for each shape */
    std::auto_ptr<CSpaceTimeCluster>           gClusterComparator;    /** instance of space-time cluster
                                                                         - used to evaluate real data */
    std::auto_ptr<SpaceTimeData>               gClusterData;          /** concrete instance of space-time cluster data object
                                                                         - used by simulation process */
    std::auto_ptr<AbstractTemporalClusterData> gAbstractClusterData;  /** abstract instance of temporal data object
                                                                        - used by simulation process */
    std::auto_ptr<CTimeIntervals>              gTimeIntervals_R;      /** iterates through temporal windows of cluster data */
    std::auto_ptr<CTimeIntervals>              gTimeIntervals_S;      /** iterates through temporal windows of cluster data */

  public:
    SpaceTimeCentricAnalysis(const CParameters& Parameters, const CSaTScanData& Data, BasePrint& PrintDirection,
                             const AbtractDataSetGateway& RealDataGateway, const DataSetGatewayContainer_t& vSimDataGateways);
    virtual ~SpaceTimeCentricAnalysis();

    virtual void             CalculatePurelyTemporalCluster(MostLikelyClustersContainer& TopClustersContainer, const AbtractDataSetGateway& DataGateway);
    virtual void             CalculateRatiosAboutCentroidDefinition(CentroidDefinitionContainer_t& vCentroid, const DataSetGatewayContainer_t& vDataGateways);
    virtual const CCluster & CalculateTopClusterAboutCentroidDefinition(CentroidDefinitionContainer_t& vCentroid, const AbtractDataSetGateway& DataGateway);
    virtual void             MonteCarloAboutCentroidDefinition(CentroidDefinitionContainer_t& vCentroid, const DataSetGatewayContainer_t& vDataGateways);
};
//******************************************************************************
#endif

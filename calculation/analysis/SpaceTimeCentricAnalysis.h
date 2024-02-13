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
  protected:
    CClusterSetCollections _top_clusters; // collection of clusters representing top cluster - used to evaluate real data
    boost::shared_ptr<CSpaceTimeCluster> _cluster_compare; // instance of space-time cluster - used to evaluate real data
    boost::shared_ptr<SpaceTimeData> _cluster_data; // concrete instance of space-time cluster data object - used by simulation process
    boost::shared_ptr<AbstractTemporalClusterData> _cluster_data_pt; // abstract instance of temporal data object - used by simulation process
    boost::shared_ptr<CTimeIntervals> _time_intervals_r; // iterates through temporal windows of cluster data
    boost::shared_ptr<CTimeIntervals> _time_Intervals_s; // iterates through temporal windows of cluster data

    virtual void                        CalculateRatiosAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const DataSetGatewayContainer_t& vDataGateways);
    virtual void                        CalculateTopClusterAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const AbstractDataSetGateway& DataGateway);
    virtual void                        ExecuteAboutPurelyTemporalCluster(const AbstractDataSetGateway& DataGateway, const DataSetGatewayContainer_t& vSimDataGateways);
    virtual const SharedClusterVector_t GetTopCalculatedClusters();
    virtual void                        MonteCarloAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const DataSetGatewayContainer_t& vDataGateways);

  public:
    SpaceTimeCentricAnalysis(const CParameters& Parameters, const CSaTScanData& Data, BasePrint& PrintDirection,
                             const AbstractDataSetGateway& RealDataGateway, const DataSetGatewayContainer_t& vSimDataGateways);
    virtual ~SpaceTimeCentricAnalysis() {}
};

/** Defines inteface for space-time analysis that includes purely temporal clusters
    evaluating real and simulated data for each centroid separate than other centroids. */
class SpaceTimeIncludePurelyTemporalCentricAnalysis : public SpaceTimeCentricAnalysis {
public:
    SpaceTimeIncludePurelyTemporalCentricAnalysis(const CParameters& Parameters, const CSaTScanData& Data, BasePrint& PrintDirection,
        const AbstractDataSetGateway& RealDataGateway, const DataSetGatewayContainer_t& vSimDataGateways);
    virtual ~SpaceTimeIncludePurelyTemporalCentricAnalysis() {}
};
//******************************************************************************
#endif

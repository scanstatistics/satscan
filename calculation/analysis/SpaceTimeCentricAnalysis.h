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
  private:
    void                     Setup(const AbstractDataSetGateway& RealDataGateway, const DataSetGatewayContainer_t& vSimDataGateways);

  protected:
    CClusterSetCollections                     _topClusters;          /** collection of clusters representing top cluster
                                                                         - used to evaluate real data */
    std::auto_ptr<CSpaceTimeCluster>           gClusterComparator;    /** instance of space-time cluster
                                                                         - used to evaluate real data */
    std::auto_ptr<SpaceTimeData>               gClusterData;          /** concrete instance of space-time cluster data object
                                                                         - used by simulation process */
    std::auto_ptr<AbstractTemporalClusterData> gAbstractClusterData;  /** abstract instance of temporal data object
                                                                        - used by simulation process */
    std::auto_ptr<CTimeIntervals>              gTimeIntervals_R;      /** iterates through temporal windows of cluster data */
    std::auto_ptr<CTimeIntervals>              gTimeIntervals_S;      /** iterates through temporal windows of cluster data */

    virtual void                        CalculateRatiosAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const DataSetGatewayContainer_t& vDataGateways);
    virtual void                        CalculateTopClusterAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const AbstractDataSetGateway& DataGateway);
    virtual void                        ExecuteAboutPurelyTemporalCluster(const AbstractDataSetGateway& DataGateway, const DataSetGatewayContainer_t& vSimDataGateways);
    virtual const SharedClusterVector_t GetTopCalculatedClusters();
    virtual void                        MonteCarloAboutCentroidDefinition(const CentroidNeighbors& CentroidDef, const DataSetGatewayContainer_t& vDataGateways);

  public:
    SpaceTimeCentricAnalysis(const CParameters& Parameters, const CSaTScanData& Data, BasePrint& PrintDirection,
                             const AbstractDataSetGateway& RealDataGateway, const DataSetGatewayContainer_t& vSimDataGateways);
    virtual ~SpaceTimeCentricAnalysis();
};
//******************************************************************************
#endif

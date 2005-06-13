//******************************************************************************
#ifndef __SpaceTimeIncludePurelyTemporalCentricAnalysis_H
#define __SpaceTimeIncludePurelyTemporalCentricAnalysis_H
//******************************************************************************
#include "SpaceTimeCentricAnalysis.h"
#include "PurelyTemporalCluster.h"

/** Defines inteface for purely spatial analysis which evaluates real and simulated
    data for each centroid separate than other centroids. */
class SpaceTimeIncludePurelyTemporalCentricAnalysis : public SpaceTimeCentricAnalysis {
  private:
    void                     Setup(const DataSetGatewayContainer_t& vSimDataGateways);

  protected:
    std::auto_ptr<CPurelyTemporalCluster>      gClusterComparator;    /** instance of space-time cluster
                                                                         - used to evaluate real data */
    std::auto_ptr<AbstractTemporalClusterData> gPTClusterData;        /** abstract instance of temporal data object
                                                                        - used by simulation process */

  public:
    SpaceTimeIncludePurelyTemporalCentricAnalysis(const CParameters& Parameters, const CSaTScanData& Data, BasePrint& PrintDirection,
                             const AbtractDataSetGateway& RealDataGateway, const DataSetGatewayContainer_t& vSimDataGateways);
    virtual ~SpaceTimeIncludePurelyTemporalCentricAnalysis();

    virtual void             CalculateRatiosAboutCentroidDefinition(CentroidDefinitionContainer_t& vCentroid, const DataSetGatewayContainer_t& vDataGateways);
    virtual void             MonteCarloAboutCentroidDefinition(CentroidDefinitionContainer_t& vCentroid, const DataSetGatewayContainer_t& vDataGateways);
};
//******************************************************************************
#endif
 
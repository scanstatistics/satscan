//******************************************************************************
#ifndef __SpaceTimeIncludePureCentricAnalysis_H
#define __SpaceTimeIncludePureCentricAnalysis_H
//******************************************************************************
#include "SpaceTimeIncludePurelySpatialCentricAnalysis.h"
#include "PurelyTemporalCluster.h"

/** Defines inteface for purely spatial analysis which evaluates real and simulated
    data for each centroid separate than other centroids. */
class SpaceTimeIncludePureCentricAnalysis : public SpaceTimeIncludePurelySpatialCentricAnalysis {
  private:
    void                     Setup(const ZdPointerVector<AbtractDataSetGateway>& vSimDataGateways);

  protected:
    std::auto_ptr<CPurelyTemporalCluster>      gClusterComparator;    /** instance of space-time cluster
                                                                         - used to evaluate real data */
    std::auto_ptr<AbstractTemporalClusterData> gPTClusterData;        /** abstract instance of temporal data object
                                                                        - used by simulation process */
    void                     MonteCarloProspectiveAboutCentroidDefinition(std::vector<CentroidNeighbors>& vCentroid, const ZdPointerVector<AbtractDataSetGateway>& vDataGateways);

  public:
    SpaceTimeIncludePureCentricAnalysis(const CParameters& Parameters, const CSaTScanData& Data, BasePrint& PrintDirection,
                             const AbtractDataSetGateway& RealDataGateway, const ZdPointerVector<AbtractDataSetGateway>& vSimDataGateways);
    virtual ~SpaceTimeIncludePureCentricAnalysis();

    virtual void             CalculateRatiosAboutCentroidDefinition(std::vector<CentroidNeighbors>& vCentroid, const ZdPointerVector<AbtractDataSetGateway>& vDataGateways);
    virtual void             MonteCarloAboutCentroidDefinition(std::vector<CentroidNeighbors>& vCentroid, const ZdPointerVector<AbtractDataSetGateway>& vDataGateways);
};
//******************************************************************************
#endif

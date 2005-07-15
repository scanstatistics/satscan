//******************************************************************************
#ifndef __SpaceTimeIncludePureCentricAnalysis_H
#define __SpaceTimeIncludePureCentricAnalysis_H
//******************************************************************************
#include "SpaceTimeIncludePurelySpatialCentricAnalysis.h"
#include "PurelyTemporalCluster.h"

/** Defines inteface for space-time analysis that includes purely temporal and spatial
    clusters, evaluating real and simulated data for each centroid separate than other centroids. */
class SpaceTimeIncludePureCentricAnalysis : public SpaceTimeIncludePurelySpatialCentricAnalysis {
  public:
    SpaceTimeIncludePureCentricAnalysis(const CParameters& Parameters, const CSaTScanData& Data, BasePrint& PrintDirection,
                             const AbstractDataSetGateway& RealDataGateway, const ZdPointerVector<AbstractDataSetGateway>& vSimDataGateways);
    virtual ~SpaceTimeIncludePureCentricAnalysis();
};
//******************************************************************************
#endif


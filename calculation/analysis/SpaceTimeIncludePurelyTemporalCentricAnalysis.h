//******************************************************************************
#ifndef __SpaceTimeIncludePurelyTemporalCentricAnalysis_H
#define __SpaceTimeIncludePurelyTemporalCentricAnalysis_H
//******************************************************************************
#include "SpaceTimeCentricAnalysis.h"
#include "PurelyTemporalCluster.h"

/** Defines inteface for space-time analysis that includes purely temporal clusters
    evaluating real and simulated data for each centroid separate than other centroids. */
class SpaceTimeIncludePurelyTemporalCentricAnalysis : public SpaceTimeCentricAnalysis {
  public:
    SpaceTimeIncludePurelyTemporalCentricAnalysis(const CParameters& Parameters, const CSaTScanData& Data, BasePrint& PrintDirection,
                             const AbtractDataSetGateway& RealDataGateway, const DataSetGatewayContainer_t& vSimDataGateways);
    virtual ~SpaceTimeIncludePurelyTemporalCentricAnalysis();
};
//******************************************************************************
#endif


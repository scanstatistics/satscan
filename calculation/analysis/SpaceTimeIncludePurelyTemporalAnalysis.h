//*****************************************************************************
#ifndef __SPACETIMEPURELYTEMPORALANALYSIS_H
#define __SPACETIMEPURELYTEMPORALANALYSIS_H
//*****************************************************************************
#include "SpaceTimeAnalysis.h"
#include "PurelyTemporalCluster.h"
#include "SpaceTimeCluster.h"
#include "PurelyTemporalData.h"
#include "SpaceTimeData.h"
#include "MeasureList.h"

/** Derives from base class to re-define methods for a space-time analysis that
    includes purely temporal clusters. */
class C_ST_PT_Analysis : public CSpaceTimeAnalysis {
  private:
    CPurelyTemporalCluster      * gpTopPurelyTemporalCluster;
    CPurelyTemporalCluster      * gpPurelyTemporalClusterComparator;
    TemporalData                * gpPTClusterData;

    void                          Init();

  protected:
    virtual void                AllocateSimulationObjects(const AbtractDataSetGateway & DataGateway);  

  public:
    C_ST_PT_Analysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection);
    virtual ~C_ST_PT_Analysis();

    virtual void                FindTopClusters(const AbtractDataSetGateway & DataGateway, MostLikelyClustersContainer& TopClustersContainer);
    virtual double              FindTopRatio(const AbtractDataSetGateway & DataGateway);
    virtual double              MonteCarlo(const DataSetInterface & Interface);
};
//*****************************************************************************
#endif

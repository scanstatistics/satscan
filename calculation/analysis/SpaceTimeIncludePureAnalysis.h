//*************************************************************************
#ifndef __SPACETIMEINCLUDEPUREANALYSIS_H
#define __SPACETIMEINCLUDEPUREANALYSIS_H
//*************************************************************************
#include "SpaceTimeIncludePurelySpatialAnalysis.h"
#include "SpaceTimeIncludePurelyTemporalAnalysis.h"

/** Derives from base class to re-define methods for a space-time analysis
    which includes purely spatial and purely temporal clusters. */
class C_ST_PS_PT_Analysis : public C_ST_PS_Analysis {
  private:
    CPurelyTemporalCluster    * gpTopPurelyTemporalCluster; /** cluster object utilized in calculating simulation llr */
    CPurelyTemporalCluster    * gpPTClusterComparator;      /** cluster object utilized in calculating simulation llr */
    TemporalData              * gpPTClusterData;            /** cluster data object utilized in calculating simulation llr */

    void                        Init();

  protected:
    virtual void                AllocateSimulationObjects(const AbtractDataSetGateway& DataGateway);
    double                      MonteCarloProspective(const DataSetInterface& Interface);

  public:
    C_ST_PS_PT_Analysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection);
    virtual ~C_ST_PS_PT_Analysis();

    virtual void                FindTopClusters(const AbtractDataSetGateway& DataGateway, MostLikelyClustersContainer& TopClustersContainer);
    virtual double              FindTopRatio(const AbtractDataSetGateway& DataGateway);
    virtual double              MonteCarlo(const DataSetInterface& Interface);
};
//*************************************************************************
#endif

//*****************************************************************************
#ifndef __PURELYSPATIALMONOTONEANALYSIS_H
#define __PURELYSPATIALMONOTONEANALYSIS_H
//*****************************************************************************
#include "PurelySpatialAnalysis.h"
#include "PurelySpatialMonotoneCluster.h"

/** Derives from base class to re-define methods for monotone purely spatial analysis. */
class CPSMonotoneAnalysis : public CAnalysis {
  private:
    boost::shared_ptr<CPSMonotoneCluster> _compare_cluster;    /** cluster object utilized to find top cluster in real data */
    boost::shared_ptr<CPSMonotoneCluster> _aux_compare_cluster; /** cluster object utilized to find top cluster in real data */
    CClusterSetCollections _top_clusters;

  protected:
    virtual const SharedClusterVector_t CalculateTopClusters(tract_t tCenter, const AbstractDataSetGateway& DataGateway);
    virtual double                      MonteCarlo(const DataSetInterface& Interface);
    virtual double                      MonteCarlo(tract_t tCenter, const AbstractDataSetGateway & DataGateway);

  public:
    CPSMonotoneAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection);
    virtual ~CPSMonotoneAnalysis() {}

    virtual void                        AllocateSimulationObjects(const AbstractDataSetGateway& DataGateway);
    virtual void                        AllocateTopClustersObjects(const AbstractDataSetGateway& DataGateway);
};
//*****************************************************************************
#endif

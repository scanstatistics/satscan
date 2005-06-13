//******************************************************************************
#ifndef __PURELYSPATIALANALYSIS_H
#define __PURELYSPATIALANALYSIS_H
//******************************************************************************
#include "Analysis.h"
#include "PurelySpatialCluster.h"
#include "IntermediateClustersContainer.h"

/** forward class declarations */
class SpatialData;

/** Derives from base class to re-define methods for a purely spatial analysis. */
class CPurelySpatialAnalysis : public CAnalysis {
  typedef IntermediateClustersContainer<CPurelySpatialCluster> intermediate_clusters_t;

  private:
    intermediate_clusters_t     gTopShapeClusters;   /** collection of clusters representing top cluster for each shape */
    CPurelySpatialCluster     * gpClusterComparator; /** cluster object utilized to find top cluster */
    SpatialData               * gpClusterData;       /** cluster data object utilized in calculated simulation ratio */
    CMeasureList              * gpMeasureList;       /** measure list object utilized in calculated simulation ratio */

  protected:
    virtual const CCluster    & CalculateTopCluster(tract_t nCenter, const AbtractDataSetGateway& DataGateway);

  public:
    CPurelySpatialAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection);
    virtual ~CPurelySpatialAnalysis();

    virtual void                AllocateTopClustersObjects(const AbtractDataSetGateway& DataGateway);
    virtual void                AllocateSimulationObjects(const AbtractDataSetGateway& DataGateway);
    virtual double              MonteCarlo(const DataSetInterface& Interface);
};
//******************************************************************************
#endif

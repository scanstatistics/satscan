//*****************************************************************************
#ifndef __PURELYSPATIALANALYSIS_H
#define __PURELYSPATIALANALYSIS_H
//*****************************************************************************
#include "Analysis.h"
#include "ClusterData.h"
#include "PurelySpatialData.h"
#include "PurelySpatialCluster.h"
#include "MeasureList.h"

/** Derives from base class to re-define methods for a purely spatial analysis. */
class CPurelySpatialAnalysis : public CAnalysis {
  private:
    TopClustersContainer      * gpTopShapeClusters;  /** collection of clusters representing top cluster for each shape */
    CPurelySpatialCluster     * gpClusterComparator; /** cluster object utilized to find top cluster */
    SpatialData               * gpClusterData;       /** cluster data object utilized in calculated simulation ratio */
    CMeasureList              * gpMeasureList;       /** measure list object utilized in calculated simulation ratio */

    void                        Init();
    void                        Setup();

  protected:
    virtual void                AllocateTopClustersObjects(const AbtractDataStreamGateway& DataGateway);
    virtual void                AllocateSimulationObjects(const AbtractDataStreamGateway& DataGateway);
    virtual const CCluster    & CalculateTopCluster(tract_t nCenter, const AbtractDataStreamGateway& DataGateway);

  public:
    CPurelySpatialAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection);
    virtual ~CPurelySpatialAnalysis();

    virtual double              MonteCarlo(const DataStreamInterface& Interface);
};
//*****************************************************************************
#endif

//******************************************************************************
#ifndef __SPACETIMEANALYSIS_H
#define __SPACETIMEANALYSIS_H
//******************************************************************************
#include "Analysis.h"
#include "SpaceTimeCluster.h"
#include "PurelyTemporalCluster.h"
#include "MeasureList.h"
#include "IntermediateClustersContainer.h"

/** forward class declarations */
class SpaceTimeData;

/** Derives from base class to re-define methods for a space-time analysis. */
class CSpaceTimeAnalysis : public CAnalysis {
  typedef IntermediateClustersContainer<CSpaceTimeCluster> intermediate_clusters_t;

  protected:
    intermediate_clusters_t    gTopShapeClusters; /** collection of clusters representing top cluster for each shape */
    CSpaceTimeCluster        * gpClusterComparator; /** cluster object utilized to find top cluster */
    SpaceTimeData            * gpClusterData;       /** cluster data object utilized in calculated simulation ratio */
    CMeasureList             * gpMeasureList;       /** measure list object utilized in calculated simulation ratio */
    CTimeIntervals           * gpTimeIntervals;     /** iterates through temporal windows of cluster data */

    virtual void               AllocateSimulationObjects(const AbtractDataSetGateway& DataGateway);
    virtual void               AllocateTopClustersObjects(const AbtractDataSetGateway& DataGateway);
    virtual const CCluster   & CalculateTopCluster(tract_t tCenter, const AbtractDataSetGateway& DataGateway);

  public:
    CSpaceTimeAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection);
    virtual ~CSpaceTimeAnalysis();

    virtual double             MonteCarlo(const DataSetInterface& Interface);
};
//***********************************************************************************
#endif

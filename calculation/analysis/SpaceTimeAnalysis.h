//***********************************************************************************
#ifndef __SPACETIMEANALYSIS_H
#define __SPACETIMEANALYSIS_H
//***********************************************************************************
#include "Analysis.h"
#include "ClusterData.h"
#include "SpaceTimeCluster.h"
#include "MeasureList.h"
#include "ProbabilityModel.h"

/** Derives from base class to re-define methods for a space-time analysis. */
class CSpaceTimeAnalysis : public CAnalysis {
  private:
    void                                Init();
    void                                Setup();

  protected:
    TopClustersContainer              * gpTopShapeClusters;  /** collection of clusters representing top cluster for each shape */
    CSpaceTimeCluster                 * gpClusterComparator; /** cluster object utilized to find top cluster */
    SpaceTimeData                     * gpClusterData;       /** cluster data object utilized in calculated simulation ratio */
    CMeasureList                      * gpMeasureList;       /** measure list object utilized in calculated simulation ratio */
    CTimeIntervals                    * gpTimeIntervals;     /** iterates through temporal windows of cluster data */

    virtual void                        AllocateSimulationObjects(const AbtractDataStreamGateway& DataGateway);
    virtual void                        AllocateTopClustersObjects(const AbtractDataStreamGateway& DataGateway);
    virtual const CCluster &            CalculateTopCluster(tract_t tCenter, const AbtractDataStreamGateway& DataGateway);

  public:
    CSpaceTimeAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection);
    virtual ~CSpaceTimeAnalysis();

    virtual double                      MonteCarlo(const DataStreamInterface& Interface);
};
//***********************************************************************************
#endif

//***********************************************************************************
#ifndef __SPACETIMEANALYSIS_H
#define __SPACETIMEANALYSIS_H
//***********************************************************************************
#include "Analysis.h"
#include "SpaceTimeCluster.h"
#include "MeasureList.h"
#include "ProbabilityModel.h"

class CSpaceTimeAnalysis : public CAnalysis {
  private:
    void                                Init();
    void                                Setup();

  protected:
    TopClustersContainer              * gpTopShapeClusters;
    CSpaceTimeCluster                 * gpClusterComparator;
    SpaceTimeData                     * gpClusterData;
    CMeasureList                      * gpMeasureList;
    CTimeIntervals                    * gpTimeIntervals;

    virtual void                        AllocateSimulationObjects(const AbtractDataStreamGateway & DataGateway);
    virtual void                        AllocateTopClustersObjects(const AbtractDataStreamGateway & DataGateway);
    virtual const CCluster &            CalculateTopCluster(tract_t tCenter, const AbtractDataStreamGateway & DataGateway);

  public:
    CSpaceTimeAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection);
    virtual ~CSpaceTimeAnalysis();

    virtual double                      MonteCarlo(const DataStreamInterface & Interface);
};
//***********************************************************************************
#endif

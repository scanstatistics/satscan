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
    virtual double                      MonteCarlo(const DataStreamInterface & Interface);
    virtual double                      MonteCarloProspective(const DataStreamInterface & Interface);

  public:
    CSpaceTimeAnalysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~CSpaceTimeAnalysis();
};
//***********************************************************************************
#endif


//*****************************************************************************
#ifndef __PURELYSPATIALANALYSIS_H
#define __PURELYSPATIALANALYSIS_H
//*****************************************************************************
#include "Analysis.h"
#include "PurelySpatialData.h"
#include "PurelySpatialCluster.h"
#include "MeasureList.h"

class CPurelySpatialAnalysis : public CAnalysis {
  private:
    TopClustersContainer      * gpTopShapeClusters;
    CPurelySpatialCluster     * gpClusterComparator;
    SpatialData               * gpClusterData;
    CMeasureList              * gpMeasureList;

    void                        Init();
    void                        Setup();

  protected:
    virtual void                AllocateTopClustersObjects(const AbtractDataStreamGateway & DataGateway);
    virtual void                AllocateSimulationObjects(const AbtractDataStreamGateway & DataGateway);
    virtual const CCluster    & CalculateTopCluster(tract_t nCenter, const AbtractDataStreamGateway & DataGateway);

  public:
    CPurelySpatialAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection);
    virtual ~CPurelySpatialAnalysis();

    virtual double              MonteCarlo(const DataStreamInterface & Interface);
};
//*****************************************************************************
#endif

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

    virtual double              MonteCarlo(const DataStreamInterface & Interface);
    virtual double              MonteCarloProspective(const DataStreamInterface & Interface);
    void                        Init();
    void                        Setup();

  protected:
    virtual void                AllocateSimulationObjects(const AbtractDataStreamGateway & DataGateway);
    virtual const CCluster    & CalculateTopCluster(tract_t nCenter, const AbtractDataStreamGateway & DataGateway);
    virtual void                AllocateTopClustersObjects(const AbtractDataStreamGateway & DataGateway);

  public:
    CPurelySpatialAnalysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~CPurelySpatialAnalysis();
};
//*****************************************************************************
#endif



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

    virtual double              MonteCarlo(const DataStreamInterface & Interface);
    virtual double              MonteCarloProspective(const DataStreamInterface & Interface);
    void                        Init() {gpTopShapeClusters=0;}
    void                        Setup();

  protected:
    virtual void                CalculateTopCluster(tract_t nCenter, const DataStreamGateway & DataGateway, bool bSimulation);
    virtual CCluster          & GetTopCalculatedCluster();
    virtual void                SetTopClusters(const DataStreamGateway & DataGateway, bool bSimulation);

  public:
    CPurelySpatialAnalysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~CPurelySpatialAnalysis();
};
//*****************************************************************************
#endif



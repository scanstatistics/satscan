//*****************************************************************************
#ifndef __SVTTANALYSIS_H
#define __SVTTANALYSIS_H
//*****************************************************************************
#include "Analysis.h"
#include "SaTScanData.h"

/** spatial variation and temporal tends analysis class */
class CSpatialVarTempTrendAnalysis : public CAnalysis {
  private:
    TopClustersContainer      * gpTopShapeClusters;

    void                        Init() {gpTopShapeClusters=0;}
    virtual double              MonteCarlo(const DataStreamInterface & Interface);
    virtual double              MonteCarloProspective(const DataStreamInterface & Interface);
    void                        Setup();

  protected:
    virtual void                CalculateTopCluster(tract_t tCenter, const DataStreamGateway & DataGateway, bool bSimulation);
    virtual CCluster          & GetTopCalculatedCluster();
    virtual void                SetTopClusters(const DataStreamGateway & DataGateway, bool bSimulation);
    virtual void                SetTopClusters(const DataStreamInterface & Interface, bool bSimulation);

  public:
    CSpatialVarTempTrendAnalysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~CSpatialVarTempTrendAnalysis();
};

#endif



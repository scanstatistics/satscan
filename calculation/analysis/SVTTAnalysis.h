//*****************************************************************************
#ifndef __SVTTANALYSIS_H
#define __SVTTANALYSIS_H
//*****************************************************************************
#include "Analysis.h"
#include "SaTScanData.h"

class CSpatialVarTempTrendAnalysis : public CAnalysis {
  private:
    void                        Init() {gpTopShapeClusters=0;}
    void                        Setup();

  protected:
    virtual double              MonteCarlo();
    virtual double              MonteCarloProspective();

    TopClustersContainer      * gpTopShapeClusters;

  public:
    CSpatialVarTempTrendAnalysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~CSpatialVarTempTrendAnalysis();

    virtual CCluster          * GetTopCluster(tract_t nCenter);
};

#endif



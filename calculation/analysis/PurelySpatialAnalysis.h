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
    void                                Init() {gpTopShapeClusters=0;}
    void                                Setup();

  protected:
    TopClustersContainer              * gpTopShapeClusters;

    virtual CCluster                  * GetTopCluster(tract_t nCenter);
    virtual double                      MonteCarlo();
    virtual double                      MonteCarloProspective();

  public:
    CPurelySpatialAnalysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~CPurelySpatialAnalysis();
};
//*****************************************************************************
#endif



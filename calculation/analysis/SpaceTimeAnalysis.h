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
    void                                Init() {gpTopShapeClusters=0;}
    void                                Setup();

  protected:
    TopClustersContainer              * gpTopShapeClusters;

    virtual CCluster                  * GetTopCluster(tract_t nCenter);
    virtual double                      MonteCarlo();
    virtual double                      MonteCarloProspective();

  public:
    CSpaceTimeAnalysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~CSpaceTimeAnalysis();
};
//***********************************************************************************
#endif


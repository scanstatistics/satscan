//***********************************************************************************
#ifndef __SPACETTIMEINCLUDEPURELYSPATIALANALYSIS_H
#define __SPACETTIMEINCLUDEPURELYSPATIALANALYSIS_H
//***********************************************************************************
#include "SpaceTimeAnalysis.h"
#include "PurelySpatialAnalysis.h"
#include "SpaceTimeCluster.h"
#include "PurelySpatialCluster.h"
#include "ProbabilityModel.h"

class C_ST_PS_Analysis : public CSpaceTimeAnalysis {
  private:
    void                                Init() {gp_PS_TopShapeClusters=0;gp_ST_TopShapeClusters=0;}
    void                                Setup();

  protected:
    TopClustersContainer              * gp_PS_TopShapeClusters;
    TopClustersContainer              * gp_ST_TopShapeClusters;

  public:
    C_ST_PS_Analysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~C_ST_PS_Analysis();

    virtual CCluster                  * GetTopCluster(tract_t nCenter);
    virtual double                      MonteCarlo();
    virtual double                      MonteCarloProspective();
};
//***********************************************************************************
#endif

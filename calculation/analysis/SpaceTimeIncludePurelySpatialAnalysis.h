//***********************************************************************************
#ifndef __SPACETTIMEINCLUDEPURELYSPATIALANALYSIS_H
#define __SPACETTIMEINCLUDEPURELYSPATIALANALYSIS_H
//***********************************************************************************
#include "SpaceTimeAnalysis.h"
#include "PurelySpatialAnalysis.h"
#include "SpaceTimeCluster.h"
#include "PurelySpatialCluster.h"
#include "ProbabilityModel.h"
#include "PurelySpatialProspectiveCluster.h"

class C_ST_PS_Analysis : public CSpaceTimeAnalysis {
  private:
    TopClustersContainer              * gp_PS_TopShapeClusters;
    TopClustersContainer              * gp_ST_TopShapeClusters;

    void                                Init() {gp_PS_TopShapeClusters=0;gp_ST_TopShapeClusters=0;}
    virtual double                      MonteCarlo(const DataStreamInterface & Interface);
    virtual double                      MonteCarloProspective(const DataStreamInterface & Interface);
    void                                Setup();

  protected:
    virtual void                        CalculateTopCluster(tract_t tCenter, const DataStreamGateway & DataGateway, bool bSimulation);
    CCluster                          & GetTopCalculatedCluster();
    virtual void                        SetTopClusters(const DataStreamGateway & DataGateway, bool bSimulation);
    
  public:
    C_ST_PS_Analysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~C_ST_PS_Analysis();
};
//***********************************************************************************
#endif

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
    TopClustersContainer              * gpTopShapeClusters;

    void                                Init() {gpTopShapeClusters=0;}
    virtual double                      MonteCarlo(const DataStreamInterface & Interface);
    virtual double                      MonteCarloProspective(const DataStreamInterface & Interface);
    void                                Setup();

  protected:
    virtual void                        CalculateTopCluster(tract_t tCenter, const DataStreamGateway & DataGateway, bool bSimulation);
    virtual CCluster                  & GetTopCalculatedCluster();
    virtual void                        SetTopClusters(const DataStreamGateway & DataGateway, bool bSimulation);

  public:
    CSpaceTimeAnalysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~CSpaceTimeAnalysis();
};
//***********************************************************************************
#endif


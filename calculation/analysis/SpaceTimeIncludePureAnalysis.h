//*************************************************************************
#ifndef __SPACETIMEINCLUDEPUREANALYSIS_H
#define __SPACETIMEINCLUDEPUREANALYSIS_H
//*************************************************************************
#include "SpaceTimeIncludePurelySpatialAnalysis.h"
#include "SpaceTimeIncludePurelyTemporalAnalysis.h"

class C_ST_PS_PT_Analysis : public C_ST_PS_Analysis {
  private:
    CPurelyTemporalCluster    * gpTopCluster;

    virtual void                CalculatePurelyTemporalCluster(const DataStreamGateway & DataGateway, bool bSimulation);
    void                        Init() {gpTopCluster=0;}
    virtual double              MonteCarlo(const DataStreamInterface & Interface);
    virtual double              MonteCarloProspective(const DataStreamInterface & Interface);

  protected:
    virtual void                CalculateTopCluster(tract_t tCenter, const DataStreamGateway & DataGateway, bool bSimluation);
    virtual bool                FindTopClusters();
    virtual CCluster          & GetTopCalculatedCluster();

  public:
    C_ST_PS_PT_Analysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~C_ST_PS_PT_Analysis();
};
//*************************************************************************
#endif

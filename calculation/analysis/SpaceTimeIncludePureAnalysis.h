//*************************************************************************
#ifndef __SPACETIMEINCLUDEPUREANALYSIS_H
#define __SPACETIMEINCLUDEPUREANALYSIS_H
//*************************************************************************
#include "SpaceTimeIncludePurelySpatialAnalysis.h"
#include "SpaceTimeIncludePurelyTemporalAnalysis.h"

class C_ST_PS_PT_Analysis : public C_ST_PS_Analysis {
  private:
    CPurelyTemporalCluster      * gpTopPurelyTemporalCluster;
    CPurelyTemporalCluster      * gpPTClusterComparator;
    TemporalData                * gpPTClusterData;
    
    void                        Init();

  protected:
    virtual void                AllocateSimulationObjects(const AbtractDataStreamGateway & DataGateway);  
    virtual bool                FindTopClusters(const AbtractDataStreamGateway & DataGateway);
    virtual double              FindTopRatio(const AbtractDataStreamGateway & DataGateway);
    virtual double              MonteCarlo(const DataStreamInterface & Interface);
    virtual double              MonteCarloProspective(const DataStreamInterface & Interface);

  public:
    C_ST_PS_PT_Analysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~C_ST_PS_PT_Analysis();
};
//*************************************************************************
#endif

//*************************************************************************
#ifndef __SPACETIMEINCLUDEPUREANALYSIS_H
#define __SPACETIMEINCLUDEPUREANALYSIS_H
//*************************************************************************
#include "SpaceTimeIncludePurelySpatialAnalysis.h"
#include "SpaceTimeIncludePurelyTemporalAnalysis.h"

class C_ST_PS_PT_Analysis : public C_ST_PS_Analysis
{
  public:
    C_ST_PS_PT_Analysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~C_ST_PS_PT_Analysis();

    virtual bool   FindTopClusters();
//    virtual void   MakeData();
    virtual double MonteCarlo();
    virtual double MonteCarloProspective();
    CPurelyTemporalCluster* GetTopPTCluster();
};
//*************************************************************************
#endif

//***********************************************************************************
#ifndef __SPACETIMEANALYSIS_H
#define __SPACETIMEANALYSIS_H
//***********************************************************************************
#include "Analysis.h"
#include "SpaceTimeCluster.h"
#include "MeasureList.h"
#include "ProbabilityModel.h"

class CSpaceTimeAnalysis : public CAnalysis
{
  public:
    CSpaceTimeAnalysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~CSpaceTimeAnalysis();

    virtual CCluster* GetTopCluster(tract_t nCenter);
    virtual double    MonteCarlo();
    virtual double    MonteCarloProspective();
};
//***********************************************************************************
#endif


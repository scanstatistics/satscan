//***********************************************************************************
#ifndef __SPACETTIMEINCLUDEPURELYSPATIALANALYSIS_H
#define __SPACETTIMEINCLUDEPURELYSPATIALANALYSIS_H
//***********************************************************************************
#include "SpaceTimeAnalysis.h"
#include "PurelySpatialAnalysis.h"
#include "SpaceTimeCluster.h"
#include "PurelySpatialCluster.h"
#include "ProbabilityModel.h"

class C_ST_PS_Analysis : public CSpaceTimeAnalysis
{
  public:
    C_ST_PS_Analysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~C_ST_PS_Analysis();

    virtual CCluster* GetTopCluster(tract_t nCenter);
    virtual double    MonteCarlo();
    virtual double    MonteCarloProspective();
};
//***********************************************************************************
#endif

// ST+PSanalysis.h

#ifndef __ST_PS_ANALYSIS_H
#define __ST_PS_ANALYSIS_H

#include "STAnalysis.h"
#include "PSAnalysis.h"

class C_ST_PS_Analysis : public CSpaceTimeAnalysis
{
  public:
    C_ST_PS_Analysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~C_ST_PS_Analysis();

    virtual CCluster* GetTopCluster(tract_t nCenter);
    virtual double    MonteCarlo();
    virtual double    MonteCarloProspective();
};

#endif

// STAnalysis.h

#ifndef __STANALYSIS_H
#define __STANALYSIS_H

#include "analysis.h"

class CSpaceTimeAnalysis : public CAnalysis
{
  public:
    CSpaceTimeAnalysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~CSpaceTimeAnalysis();

    virtual CCluster* GetTopCluster(tract_t nCenter);
    virtual double    MonteCarlo();
    virtual double    MonteCarloProspective();
};

#endif


// PSanalysis.h

#ifndef __PSANALYSIS_H
#define __PSANALYSIS_H

#include "analysis.h"
#include "PSData.h"

class CPurelySpatialAnalysis : public CAnalysis
{
  public:
    CPurelySpatialAnalysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~CPurelySpatialAnalysis();

    virtual CCluster* GetTopCluster(tract_t nCenter);

  protected:
    virtual double    MonteCarlo();
    virtual double    MonteCarloProspective();
};

#endif



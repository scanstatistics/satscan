// PSMAnalysis.h

#ifndef __PSMANALYSIS_H
#define __PSMANALYSIS_H

#include "PSanalysis.h"

class CPSMonotoneAnalysis : public CPurelySpatialAnalysis
{
  public:
    CPSMonotoneAnalysis(CParameters* pParameters, CSaTScanData* pData);
    virtual ~CPSMonotoneAnalysis();

    virtual CCluster* GetTopCluster(tract_t nCenter);

  protected:
    virtual double MonteCarlo();
};

#endif


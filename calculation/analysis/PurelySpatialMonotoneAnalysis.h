//*****************************************************************************
#ifndef __PURELYSPATIALMONOTONEANALYSIS_H
#define __PURELYSPATIALMONOTONEANALYSIS_H
//*****************************************************************************
#include "PurelySpatialAnalysis.h"
#include "PurelySpatialMonotoneCluster.h"

class CPSMonotoneAnalysis : public CPurelySpatialAnalysis
{
  public:
    CPSMonotoneAnalysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~CPSMonotoneAnalysis();

    virtual CCluster* GetTopCluster(tract_t nCenter);

  protected:
    virtual double MonteCarlo();
    virtual double MonteCarloProspective();
};

//*****************************************************************************
#endif


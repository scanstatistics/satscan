//*****************************************************************************
#ifndef __PURELYSPATIALANALYSIS_H
#define __PURELYSPATIALANALYSIS_H
//*****************************************************************************
#include "Analysis.h"
#include "PurelySpatialData.h"
#include "PurelySpatialCluster.h"
#include "MeasureList.h"

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
//*****************************************************************************
#endif



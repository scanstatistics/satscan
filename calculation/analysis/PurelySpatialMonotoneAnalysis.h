//*****************************************************************************
#ifndef __PURELYSPATIALMONOTONEANALYSIS_H
#define __PURELYSPATIALMONOTONEANALYSIS_H
//*****************************************************************************
#include "PurelySpatialAnalysis.h"
#include "PurelySpatialMonotoneCluster.h"

class CPSMonotoneAnalysis : public CPurelySpatialAnalysis {
  private:
    CPSMonotoneCluster* gpMaxCluster;

    void                Init() {gpMaxCluster=0;}
    virtual double      MonteCarlo(const DataStreamInterface & Interface);
    virtual double      MonteCarloProspective(const DataStreamInterface & Interface);

  protected:
    virtual void        CalculateTopCluster(tract_t tCenter, const DataStreamGateway & DataGateway, bool bSimulation);
    virtual CCluster  & GetTopCalculatedCluster();
    virtual void        SetTopClusters(const DataStreamGateway & DataGateway, bool bSimulation);
    
  public:
    CPSMonotoneAnalysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~CPSMonotoneAnalysis();
};

//*****************************************************************************
#endif


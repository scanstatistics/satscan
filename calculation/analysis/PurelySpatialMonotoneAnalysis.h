//*****************************************************************************
#ifndef __PURELYSPATIALMONOTONEANALYSIS_H
#define __PURELYSPATIALMONOTONEANALYSIS_H
//*****************************************************************************
#include "PurelySpatialAnalysis.h"
#include "PurelySpatialMonotoneCluster.h"

class CPSMonotoneAnalysis : public CPurelySpatialAnalysis {
  private:
    CPSMonotoneCluster     * gpMaxCluster;

    void                     Init() {gpMaxCluster=0;}

  protected:
    virtual void             AllocateSimulationObjects(const AbtractDataStreamGateway & DataGateway);
    virtual void             AllocateTopClustersObjects(const AbtractDataStreamGateway & DataGateway);
    virtual const CCluster & CalculateTopCluster(tract_t tCenter, const AbtractDataStreamGateway & DataGateway);
    virtual double           MonteCarlo(const DataStreamInterface & Interface);

  public:
    CPSMonotoneAnalysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~CPSMonotoneAnalysis();
};
//*****************************************************************************
#endif

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

  public:
    CPSMonotoneAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection);
    virtual ~CPSMonotoneAnalysis();

    virtual double           MonteCarlo(const DataStreamInterface & Interface);    
};
//*****************************************************************************
#endif

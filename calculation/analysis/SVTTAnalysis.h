//*****************************************************************************
#ifndef __SVTTANALYSIS_H
#define __SVTTANALYSIS_H
//*****************************************************************************
#include "Analysis.h"
#include "SaTScanData.h"

/** spatial variation and temporal tends analysis class */
class CSpatialVarTempTrendAnalysis : public CAnalysis {
  private:
    TopClustersContainer      * gpTopShapeClusters;

    void                        Init() {gpTopShapeClusters=0;}
    void                        Setup();

  protected:
    virtual void                AllocateTopClustersObjects(const AbtractDataStreamGateway & DataGateway);
    virtual void                AllocateSimulationObjects(const AbtractDataStreamGateway & DataGateway);
    virtual const CCluster    & CalculateTopCluster(tract_t tCenter, const AbtractDataStreamGateway & DataGateway);

  public:
    CSpatialVarTempTrendAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection);
    virtual ~CSpatialVarTempTrendAnalysis();

    virtual double              MonteCarlo(const DataStreamInterface & Interface);
};

#endif

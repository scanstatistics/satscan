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
    virtual void                AllocateTopClustersObjects(const AbtractDataSetGateway & DataGateway);
    virtual void                AllocateSimulationObjects(const AbtractDataSetGateway & DataGateway);
    virtual const CCluster    & CalculateTopCluster(tract_t tCenter, const AbtractDataSetGateway & DataGateway);

  public:
    CSpatialVarTempTrendAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection);
    virtual ~CSpatialVarTempTrendAnalysis();

    virtual double              MonteCarlo(const DataSetInterface & Interface);
};

#endif

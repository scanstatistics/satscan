//*****************************************************************************
#ifndef __PURELYSPATIALMONOTONEANALYSIS_H
#define __PURELYSPATIALMONOTONEANALYSIS_H
//*****************************************************************************
#include "PurelySpatialAnalysis.h"
#include "PurelySpatialMonotoneCluster.h"

/** Derives from base class to re-define methods for monotone purely spatial analysis. */
class CPSMonotoneAnalysis : public CPurelySpatialAnalysis {
  private:
    CPSMonotoneCluster     * gpMaxCluster; /** cluster object utilized to find top cluster in real data */

    void                     Init() {gpMaxCluster=0;}

  protected:
    virtual void             AllocateSimulationObjects(const AbtractDataSetGateway& DataGateway);
    virtual void             AllocateTopClustersObjects(const AbtractDataSetGateway& DataGateway);
    virtual const CCluster & CalculateTopCluster(tract_t tCenter, const AbtractDataSetGateway& DataGateway);

  public:
    CPSMonotoneAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection);
    virtual ~CPSMonotoneAnalysis();

    virtual double           MonteCarlo(const DataSetInterface& Interface);
};
//*****************************************************************************
#endif

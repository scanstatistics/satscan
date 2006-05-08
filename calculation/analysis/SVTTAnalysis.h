//*****************************************************************************
#ifndef __SVTTANALYSIS_H
#define __SVTTANALYSIS_H
//*****************************************************************************
#include "Analysis.h"
#include "SaTScanData.h"
#include "SVTTCluster.h"
#include "IntermediateClustersContainer.h"

/** spatial variation and temporal tends analysis class */
class CSpatialVarTempTrendAnalysis : public CAnalysis {
  private:
    IntermediateClustersContainer<CSVTTCluster>  gTopClusters;

  protected:
    virtual void                AllocateTopClustersObjects(const AbstractDataSetGateway & DataGateway);
    virtual void                AllocateSimulationObjects(const AbstractDataSetGateway & DataGateway);
    virtual const CCluster    & CalculateTopCluster(tract_t tCenter, const AbstractDataSetGateway & DataGateway);

  public:
    CSpatialVarTempTrendAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection);
    virtual ~CSpatialVarTempTrendAnalysis();

    virtual double              MonteCarlo(const DataSetInterface & Interface);
    virtual double              MonteCarlo(tract_t tCenter, const AbstractDataSetGateway & DataGateway);
};

#endif

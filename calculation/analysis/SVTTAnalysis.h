//******************************************************************************
#ifndef __SVTTANALYSIS_H
#define __SVTTANALYSIS_H
//******************************************************************************
#include "Analysis.h"
#include "SaTScanData.h"
#include "SVTTCluster.h"
#include "IntermediateClustersContainer.h"

/** spatial variation and temporal tends analysis class */
class CSpatialVarTempTrendAnalysis : public CAnalysis {
  private:
    boost::shared_ptr<CSVTTCluster> _cluster_compare; // cluster object utilized to find top cluster
    boost::shared_ptr<SVTTClusterData> _cluster_data;
    CClusterSetCollections _top_clusters;

  protected:
    virtual void                        AllocateTopClustersObjects(const AbstractDataSetGateway & DataGateway);
    virtual void                        AllocateSimulationObjects(const AbstractDataSetGateway & DataGateway);
    virtual const SharedClusterVector_t CalculateTopClusters(tract_t tCenter, const AbstractDataSetGateway & DataGateway);

  public:
    CSpatialVarTempTrendAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection);
    virtual ~CSpatialVarTempTrendAnalysis() {}

    virtual double                      MonteCarlo(const DataSetInterface & Interface);
    virtual double                      MonteCarlo(tract_t tCenter, const AbstractDataSetGateway & DataGateway);
};
//******************************************************************************
#endif


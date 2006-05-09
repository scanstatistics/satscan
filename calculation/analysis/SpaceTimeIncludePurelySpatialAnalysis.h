//***********************************************************************************
#ifndef __SPACETTIMEINCLUDEPURELYSPATIALANALYSIS_H
#define __SPACETTIMEINCLUDEPURELYSPATIALANALYSIS_H
//***********************************************************************************
#include "SpaceTimeAnalysis.h"
#include "PurelySpatialAnalysis.h"
#include "SpaceTimeCluster.h"
#include "PurelySpatialCluster.h"
#include "ProbabilityModel.h"
#include "PurelySpatialProspectiveCluster.h"

class SpatialData;
class ProspectiveSpatialData;

/** Derives from base class to re-define methods for a space-time analysis that
    include purely spatial clusters. */
class C_ST_PS_Analysis : public CSpaceTimeAnalysis {
  typedef IntermediateClustersContainer<CPurelySpatialCluster>            intermediate_ps_clusters_t;
  typedef IntermediateClustersContainer<CPurelySpatialProspectiveCluster> intermediate_psp_clusters_t;
    
  protected:
//    SpatialData                       * gpPSClusterData;
    std::auto_ptr<CPurelySpatialCluster>            gPSClusterComparator;
    intermediate_ps_clusters_t                      gPSTopShapeClusters;
//    ProspectiveSpatialData            * gpPSPClusterData;
    std::auto_ptr<CPurelySpatialProspectiveCluster> gPSPClusterComparator;
    intermediate_psp_clusters_t                     gPSPTopShapeClusters;
    std::auto_ptr<AbstractSpatialClusterData>       gAbstractPSClusterData; /** cluster data object utilized in calculated simulation ratio */
    std::auto_ptr<AbstractTemporalClusterData>      gAbstractPSPClusterData;

    virtual void                        AllocateSimulationObjects(const AbstractDataSetGateway & DataGateway);
    virtual void                        AllocateTopClustersObjects(const AbstractDataSetGateway & DataGateway);
    virtual const CCluster            & CalculateTopCluster(tract_t tCenter, const AbstractDataSetGateway & DataGateway);
    virtual const CCluster            & GetTopCalculatedCluster();
    virtual double                      MonteCarlo(const DataSetInterface & Interface);
    virtual double                      MonteCarlo(tract_t tCenter, const AbstractDataSetGateway & DataGateway);
    double                              MonteCarloProspective(const DataSetInterface & Interface);
    double                              MonteCarloProspective(tract_t tCenter, const AbstractDataSetGateway & DataGateway);

  public:
    C_ST_PS_Analysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection);
    virtual ~C_ST_PS_Analysis();
};
//***********************************************************************************
#endif

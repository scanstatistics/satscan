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

/** Derives from base class to re-define methods for a space-time analysis that
    include purely spatial clusters. */
class C_ST_PS_Analysis : public CSpaceTimeAnalysis {
  private:
    void                                Init();
    void                                Setup();

  protected:
    TopClustersContainer              * gpPSTopShapeClusters;   
    CPurelySpatialCluster             * gpPSClusterComparator;
    CPurelySpatialProspectiveCluster  * gpPSPClusterComparator;
    SpatialData                       * gpPSClusterData;
    ProspectiveSpatialData            * gpPSPClusterData;

    virtual void                        AllocateSimulationObjects(const AbtractDataSetGateway & DataGateway);
    virtual void                        AllocateTopClustersObjects(const AbtractDataSetGateway & DataGateway);
    virtual const CCluster            & CalculateTopCluster(tract_t tCenter, const AbtractDataSetGateway & DataGateway);
    virtual const CCluster            & GetTopCalculatedCluster();
    double                              MonteCarloProspective(const DataSetInterface & Interface);

  public:
    C_ST_PS_Analysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection);
    virtual ~C_ST_PS_Analysis();

    virtual double                      MonteCarlo(const DataSetInterface & Interface);
};
//***********************************************************************************
#endif

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

class C_ST_PS_Analysis : public CSpaceTimeAnalysis {
  private:
    void                                Init();
    virtual double                      MonteCarlo(const DataStreamInterface & Interface);
    virtual double                      MonteCarloProspective(const DataStreamInterface & Interface);
    void                                Setup();

  protected:
    TopClustersContainer              * gpPSTopShapeClusters;
    CPurelySpatialCluster             * gpPSClusterComparator;
    CPurelySpatialProspectiveCluster  * gpPSPClusterComparator;
    SpatialData                       * gpPSClusterData;
    ProspectiveSpatialData            * gpPSPClusterData;    

    virtual void                        AllocateSimulationObjects(const AbtractDataStreamGateway & DataGateway);  
    virtual void                        AllocateTopClustersObjects(const AbtractDataStreamGateway & DataGateway);  
    virtual const CCluster            & CalculateTopCluster(tract_t tCenter, const AbtractDataStreamGateway & DataGateway);
    virtual const CCluster            & GetTopCalculatedCluster();
    
  public:
    C_ST_PS_Analysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~C_ST_PS_Analysis();
};
//***********************************************************************************
#endif

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
    protected:
        std::auto_ptr<CPurelySpatialCluster> _ps_cluster_compare;
        std::auto_ptr<CPurelySpatialProspectiveCluster> _psp_cluster_compare;
        std::auto_ptr<AbstractSpatialClusterData> _ps_cluster_data; // cluster data object utilized in calculated simulation ratio
        std::auto_ptr<AbstractTemporalClusterData> _psp_cluster_data;
        CClusterSetCollections _ps_top_clusters;
        CClusterSetCollections _psp_top_clusters;

        virtual void                        AllocateSimulationObjects(const AbstractDataSetGateway & DataGateway);
        virtual void                        AllocateTopClustersObjects(const AbstractDataSetGateway & DataGateway);
        virtual const SharedClusterVector_t CalculateTopClusters(tract_t tCenter, const AbstractDataSetGateway & DataGateway);
        virtual double                      MonteCarlo(const DataSetInterface & Interface);
        virtual double                      MonteCarlo(tract_t tCenter, const AbstractDataSetGateway & DataGateway);
        double                              MonteCarloProspective(const DataSetInterface & Interface);
        double                              MonteCarloProspective(tract_t tCenter, const AbstractDataSetGateway & DataGateway);

    public:
        C_ST_PS_Analysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection);
        virtual ~C_ST_PS_Analysis() {}
};
//***********************************************************************************
#endif

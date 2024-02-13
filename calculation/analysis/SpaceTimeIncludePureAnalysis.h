//*************************************************************************
#ifndef __SPACETIMEINCLUDEPUREANALYSIS_H
#define __SPACETIMEINCLUDEPUREANALYSIS_H
//*************************************************************************
#include "SpaceTimeIncludePurelySpatialAnalysis.h"

/** Derives from base class to re-define methods for a space-time analysis
    which includes purely spatial and purely temporal clusters. */
class C_ST_PS_PT_Analysis : public C_ST_PS_Analysis {
    private:
        boost::shared_ptr<AbstractTemporalClusterData> _pt_cluster_data;

    protected:
        virtual void                AllocateSimulationObjects(const AbstractDataSetGateway& DataGateway);
        virtual double              MonteCarlo(const DataSetInterface& Interface);
        virtual double              MonteCarlo(tract_t tCenter, const AbstractDataSetGateway & DataGateway);
        double                      MonteCarloProspective(const DataSetInterface& Interface);
        double                      MonteCarloProspective(tract_t tCenter, const AbstractDataSetGateway & DataGateway);

    public:
        C_ST_PS_PT_Analysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection);
        virtual ~C_ST_PS_PT_Analysis() {}

        virtual void                FindTopClusters(const AbstractDataSetGateway& DataGateway, MLC_Collections_t& TopClustersContainers);
};
//*************************************************************************
#endif

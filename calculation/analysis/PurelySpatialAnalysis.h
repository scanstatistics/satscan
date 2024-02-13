//******************************************************************************
#ifndef __PURELYSPATIALANALYSIS_H
#define __PURELYSPATIALANALYSIS_H
//******************************************************************************
#include "Analysis.h"
#include "PurelySpatialCluster.h"

/** forward class declarations */
class SpatialData;

/** Derives from base class to re-define methods for a purely spatial analysis. */
class CPurelySpatialAnalysis : public CAnalysis {
    private:
        boost::shared_ptr<CPurelySpatialCluster> _cluster_compare; /** cluster object utilized to find top cluster */
        boost::shared_ptr<AbstractSpatialClusterData> _cluster_data; /** cluster data object utilized in calculated simulation ratio */
        boost::shared_ptr<CMeasureList> _measure_list; /** measure list object utilized in calculated simulation ratio */
        CClusterSetCollections _top_clusters; /** collection of clusters representing top cluster for each shape */

    protected:
        virtual const SharedClusterVector_t CalculateTopClusters(tract_t nCenter, const AbstractDataSetGateway& DataGateway);
        virtual double                      MonteCarlo(const DataSetInterface& Interface);
        virtual double                      MonteCarlo(tract_t tCenter, const AbstractDataSetGateway & DataGateway);

    public:
        CPurelySpatialAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection);
        virtual ~CPurelySpatialAnalysis() {}

        virtual void                        AllocateTopClustersObjects(const AbstractDataSetGateway& DataGateway);
        virtual void                        AllocateSimulationObjects(const AbstractDataSetGateway& DataGateway);
};
//******************************************************************************
#endif

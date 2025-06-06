//******************************************************************************
#ifndef __PURELYSPATIALBRUTEFORCEANALYSIS_H
#define __PURELYSPATIALBRUTEFORCEANALYSIS_H
//******************************************************************************
#include "AbstractBruteForceAnalysis.h"
#include "PurelySpatialHomogeneousPoissonCluster.h"
#include "IntermediateClustersContainer.h"

/** forward class declarations */
class SpatialData;

/** Derives from base class to re-define methods for a purely spatial analysis. */
class CPurelySpatialBruteForceAnalysis : public AbstractBruteForceAnalysis {
    private:
        boost::shared_ptr<PurelySpatialHomogeneousPoissonCluster> _cluster_compare; /** cluster object utilized to find top cluster */
        boost::shared_ptr<SpatialHomogeneousData> _cluster_data; /** cluster data object utilized in calculated simulation ratio */
        boost::shared_ptr<CMeasureList> _measure_list; /** measure list object utilized in calculated simulation ratio */
        CClusterSetCollections _top_clusters; /** collection of clusters representing top cluster for each shape */

    protected:
        virtual const SharedClusterVector_t       CalculateTopClusters(tract_t nCenter, const AbstractDataSetGateway& DataGateway);
        virtual double                            MonteCarlo(const DataSetInterface& Interface);
        virtual double                            MonteCarlo(tract_t tCenter, const AbstractDataSetGateway & DataGateway);

    public:
        CPurelySpatialBruteForceAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection);
        virtual ~CPurelySpatialBruteForceAnalysis() {}

        virtual void                              AllocateAdditionalSimulationObjects(RandomizerContainer_t& Container);
        virtual void                              AllocateTopClustersObjects(const AbstractDataSetGateway& DataGateway);
        virtual void                              AllocateSimulationObjects(const AbstractDataSetGateway& DataGateway);
};
//******************************************************************************
#endif

//******************************************************************************
#ifndef __SPACETIMEPURELYTEMPORALANALYSIS_H
#define __SPACETIMEPURELYTEMPORALANALYSIS_H
//******************************************************************************
#include "SpaceTimeAnalysis.h"
#include "PurelyTemporalCluster.h"
#include "SpaceTimeCluster.h"
#include "PurelyTemporalData.h"
#include "SpaceTimeData.h"
#include "MeasureList.h"

/** Derives from base class to re-define methods for a space-time analysis that
    includes purely temporal clusters. */
class C_ST_PT_Analysis : public CSpaceTimeAnalysis {
    private:
        boost::shared_ptr<AbstractTemporalClusterData> _pt_cluster_data;

    protected:
        virtual void                AllocateSimulationObjects(const AbstractDataSetGateway & DataGateway);
        virtual double              MonteCarlo(const DataSetInterface & Interface);
        virtual double              MonteCarlo(tract_t tCenter, const AbstractDataSetGateway & DataGateway);

    public:
        C_ST_PT_Analysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection);
        virtual ~C_ST_PT_Analysis() {}

        virtual void                FindTopClusters(const AbstractDataSetGateway & DataGateway, MLC_Collections_t& TopClustersContainers);
};
//******************************************************************************
#endif

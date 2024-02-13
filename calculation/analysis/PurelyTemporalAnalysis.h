//******************************************************************************
#ifndef __PURELYTEMPORALANALYSIS_H
#define __PURELYTEMPORALANALYSIS_H
//******************************************************************************
#include "Analysis.h"
#include "ClusterData.h"
#include "MeasureList.h"
#include "TimeIntervalRange.h"
#include "PurelyTemporalData.h"
#include "PurelyTemporalCluster.h"

/** Derives from base class to re-define methods for a purely temporal analysis. */
class CPurelyTemporalAnalysis : public CAnalysis {
    private:
        boost::shared_ptr<CMeasureList> _measure_list; /** measure list object utilized in calculating simulation llr */
        boost::shared_ptr<CTimeIntervals> _time_intervals; /** iterates through temporal windows of cluster data */
        boost::shared_ptr<AbstractTemporalClusterData> _cluster_data;

    protected:
        virtual void                        AllocateSimulationObjects(const AbstractDataSetGateway& DataGateway);
        virtual void                        AllocateTopClustersObjects(const AbstractDataSetGateway& DataGateway) {/*nop*/}
        virtual const SharedClusterVector_t CalculateTopClusters(tract_t tCenter, const AbstractDataSetGateway& DataGateway);
        virtual double                      MonteCarlo(const DataSetInterface& Interface);
        virtual double                      MonteCarlo(tract_t tCenter, const AbstractDataSetGateway & DataGateway);

    public:
        CPurelyTemporalAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection);
        virtual ~CPurelyTemporalAnalysis() {}

        virtual void                        FindTopClusters(const AbstractDataSetGateway& DataGateway, MLC_Collections_t& TopClustersContainers);
};
//******************************************************************************
#endif


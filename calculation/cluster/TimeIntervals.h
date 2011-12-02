//*****************************************************************************
#ifndef __TIMEINTERVALS_H
#define __TIMEINTERVALS_H
//*****************************************************************************
#include "SaTScan.h"
#include "Parameters.h"
#include "cluster.h"
#include "AbstractClusterData.h"
#include "MaxWindowLengthIndicator.h"
#include "IntermediateClustersContainer.h"

class CMeasureList; /** forward class declaration */
class CCluster; /** forward class declaration */
class CSaTScanData; /** forward class declaration */

/** Abstract base class which defines methods of iterating through temporal
    windows, evaluating the strength of a clustering.*/
class CTimeIntervals {
    private:
        void Setup(IncludeClustersType eIncludeClustersType);

    protected:
        IntervalRange_t _interval_range;  /* (startrange_start, startrange_end, endrange_start, endrange_end) */
        IntervalRange_t _init_interval_range;  /* (startrange_start, startrange_end, endrange_start, endrange_end) */
        const CSaTScanData & gDataHub; /** data hub */
        const GInfo & _gInfo;
        AbstractLikelihoodCalculator & gLikelihoodCalculator; /** log likelihood calculator */
        std::auto_ptr<AbstractMaxWindowLengthIndicator> gpMaxWindowLengthIndicator; /** indicates maximum temporal window length */
        int giNumIntervals; /* number of total time intervals */
        int giMaxWindowLength; /* maximum window length */
        RATE_FUNCPTRTYPE fRateOfInterest;

    public:
        CTimeIntervals(const CSaTScanData& DataHub, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType);

        virtual void CompareClusterSet(CCluster& Running, CClusterSet& ClusterSet) = 0;
        virtual void CompareMeasures(AbstractTemporalClusterData& StreamData, CMeasureList& MeasureList) = 0;
        virtual double ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData) = 0;
        void resetIntervalRange() {_interval_range = _init_interval_range; }
        void setIntervalRange(tract_t centerpoint);
};
#endif

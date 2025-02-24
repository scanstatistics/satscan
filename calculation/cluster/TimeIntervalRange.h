//******************************************************************************
#ifndef __TIMEINTERVALRANGE_H
#define __TIMEINTERVALRANGE_H
//******************************************************************************
#include "SaTScan.h"
#include "JulianDates.h"
#include "TimeIntervals.h"
#include "DataSet.h"

class CSaTScanData; /** forward class declaration */
class CCluster; /** forward class declaration */
class AbstractTemporalClusterData; /** forward class declaration */

/** Temporal window evaluator. */
class TemporalDataEvaluator : public CTimeIntervals {
    private:
        typedef double (AbstractLikelihoodCalculator::*MAXIMIZE_FUNCPTR) (count_t,measure_t,size_t) const;
        MAXIMIZE_FUNCPTR gpCalculationMethod;
        double gdDefaultMaximizingValue;

    public:
        TemporalDataEvaluator(const CSaTScanData& Data, AbstractLikelihoodCalculator& Calculator,
                              IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType);

        virtual void CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
        virtual void CompareClusterSet(CCluster& Running, CClusterSet& ClusterSet);
        virtual double ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData);
};

/** Temporal window evaluator used when adjusting for temporal trends nonparametrically. */
class TimeStratifiedTemporalDataEvaluator : public CTimeIntervals {
    private:
        count_t * _pt_counts;
        measure_t * _pt_measure;

    public:
        TimeStratifiedTemporalDataEvaluator(
            const CSaTScanData& Data, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType
        );

        virtual void CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
        virtual void CompareClusterSet(CCluster& Running, CClusterSet& ClusterSet);
        virtual double ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData);
};

/** Temporal window evaluator for the batched model and time stratified adjustment. */
class TimeStratifiedBatchedTemporalDataEvaluator : public CTimeIntervals {
private:
    typedef double (AbstractLikelihoodCalculator::* MAXIMIZE_FUNCPTR) (count_t, measure_t, measure_t, measure_t, const boost::dynamic_bitset<>&, size_t) const;
    MAXIMIZE_FUNCPTR gpCalculationMethod;

public:
    TimeStratifiedBatchedTemporalDataEvaluator(const CSaTScanData& Data, AbstractLikelihoodCalculator& Calculator,
        IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType);

    virtual void CompareClusterSet(CCluster& Running, CClusterSet& ClusterSet);
    virtual void CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
    virtual double ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData);
};

/** Temporal window evaluator for the batched model and time stratified adjustment.
    This class is specialized for scanning windows when the time stratified adjustment length is larger
    than the time aggregation length. Technically, this class replaces TimeStratifiedBatchedTemporalDataEvaluator,
    and should produce the same results, but that class is slightly faster. */
class TimeStratifiedBatchedTemporalDataEvaluatorEnhanced : public CTimeIntervals {
private:
    typedef double (AbstractLikelihoodCalculator::* MAXIMIZE_FUNCPTR) (count_t, measure_t, measure_t, measure_t, const boost::dynamic_bitset<>&, size_t) const;
    MAXIMIZE_FUNCPTR gpCalculationMethod;

public:
    TimeStratifiedBatchedTemporalDataEvaluatorEnhanced(const CSaTScanData& Data, AbstractLikelihoodCalculator& Calculator,
        IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType);

    virtual void CompareClusterSet(CCluster& Running, CClusterSet& ClusterSet);
    virtual void CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
    virtual double ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData);
};

/** Temporal window evaluator used when adjusting for temporal trends nonparametrically with multiple data sets. */
class MultiSetTimeStratifiedTemporalDataEvaluator : public CTimeIntervals {
    private:
        std::vector<count_t*>  _pt_counts;
        std::vector<measure_t*> _pt_measure;

    public:
        MultiSetTimeStratifiedTemporalDataEvaluator(const CSaTScanData& Data, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType);

        virtual void CompareClusterSet(CCluster& Running, CClusterSet& ClusterSet);
        virtual void CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
        virtual double ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData);
};

/** Temporal window evaluator used with Bernoulli model and adjusting for purely spatial clusters nonparametrically. */
class BernoulliSpatialStratifiedTemporalDataEvaluator : public CTimeIntervals {
private:
    count_t    ** _pp_counts;
    measure_t  ** _pp_measure;

public:
    BernoulliSpatialStratifiedTemporalDataEvaluator(const CSaTScanData& Data, AbstractLikelihoodCalculator& Calculator,
        IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType);

    virtual void CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
    virtual void CompareClusterSet(CCluster& Running, CClusterSet& ClusterSet);
    virtual double ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData);
};

/** Temporal window evaluator for seasonal analysis. */
class ClosedLoopTemporalDataEvaluator : public CTimeIntervals {
    private:
        typedef double (AbstractLikelihoodCalculator::*MAXIMIZE_FUNCPTR) (count_t,measure_t,size_t) const;
        MAXIMIZE_FUNCPTR gpCalculationMethod;
        double gdDefaultMaximizingValue;
        int _extended_period_start;

    public:
        ClosedLoopTemporalDataEvaluator(const CSaTScanData& Data, AbstractLikelihoodCalculator& Calculator,
                                        IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType);

        virtual void CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
        virtual void CompareClusterSet(CCluster& Running, CClusterSet& ClusterSet);
        virtual double ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData);
};

/** Temporal window evaluator for multiple data sets. */
class MultiSetTemporalDataEvaluator : public CTimeIntervals {
    public:
        MultiSetTemporalDataEvaluator(const CSaTScanData& Data, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType);

        virtual void CompareClusterSet(CCluster& Running, CClusterSet& ClusterSet);
        virtual void CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
        virtual double ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData);
};

/** Temporal window evaluator for multiple data sets using uniform model. */
class MultiSetUniformTimeTemporalDataEvaluator : public CTimeIntervals {
public:
    MultiSetUniformTimeTemporalDataEvaluator(const CSaTScanData& Data, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType);

    virtual void CompareClusterSet(CCluster& Running, CClusterSet& ClusterSet);
    virtual void CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
    virtual double ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData);
};

/** Temporal window evaluator for seasonal analysis and multiple data sets. */
class ClosedLoopMultiSetTemporalDataEvaluator : public CTimeIntervals {
    private:
        int _extended_period_start;

    public:
        ClosedLoopMultiSetTemporalDataEvaluator(const CSaTScanData& Data, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType);

        virtual void CompareClusterSet(CCluster& Running, CClusterSet& ClusterSet);
        virtual void CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
        virtual double ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData);
};

/** Temporal window evaluator for the normal model. */
class NormalTemporalDataEvaluator : public CTimeIntervals {
    private:
        typedef double (AbstractLikelihoodCalculator::*MAXIMIZE_FUNCPTR) (count_t,measure_t,measure_t,size_t) const;
        MAXIMIZE_FUNCPTR gpCalculationMethod;
        double gdDefaultMaximizingValue;

    public:
        NormalTemporalDataEvaluator(const CSaTScanData& Data, AbstractLikelihoodCalculator& Calculator,
                                    IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType);

        virtual void CompareClusterSet(CCluster& Running, CClusterSet& ClusterSet);
        virtual void CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
        virtual double ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData);
};

/** Temporal window evaluator for the uniform time model. */
class UniformTimeTemporalDataEvaluator : public CTimeIntervals {
private:
    typedef double (AbstractLikelihoodCalculator::*MAXIMIZE_FUNCPTR) (count_t, measure_t, count_t, measure_t, size_t) const;
    MAXIMIZE_FUNCPTR gpCalculationMethod;
    double gdDefaultMaximizingValue;

public:
    UniformTimeTemporalDataEvaluator(const CSaTScanData& Data, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType);

    virtual void CompareClusterSet(CCluster& Running, CClusterSet& ClusterSet);
    virtual void CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
    virtual double ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData);
};

/** Temporal window evaluator for the normal model and seasonal analysis. */
class ClosedLoopNormalTemporalDataEvaluator : public CTimeIntervals {
    private:
        typedef double (AbstractLikelihoodCalculator::*MAXIMIZE_FUNCPTR) (count_t,measure_t,measure_t,size_t) const;
        MAXIMIZE_FUNCPTR gpCalculationMethod;
        double gdDefaultMaximizingValue;
        int _extended_period_start;

    public:
        ClosedLoopNormalTemporalDataEvaluator(const CSaTScanData& Data, AbstractLikelihoodCalculator& Calculator,
                                              IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType);

        virtual void CompareClusterSet(CCluster& Running, CClusterSet& ClusterSet);
        virtual void CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
        virtual double ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData);
};

/** Temporal window evaluator for the normal model and multiple data sets. */
class MultiSetNormalTemporalDataEvaluator : public CTimeIntervals {
    public:
        MultiSetNormalTemporalDataEvaluator(const CSaTScanData& DataHub, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType);

        virtual void CompareClusterSet(CCluster& Running, CClusterSet& ClusterSet);
        virtual void CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
        virtual double ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData);
};

/** Temporal window evaluator for the normal model, seasonal analysis and multiple data sets. */
class ClosedLoopMultiSetNormalTemporalDataEvaluator : public CTimeIntervals {
    private:
        int _extended_period_start;

    public:
        ClosedLoopMultiSetNormalTemporalDataEvaluator(const CSaTScanData& DataHub, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType);

        virtual void CompareClusterSet(CCluster& Running, CClusterSet& ClusterSet);
        virtual void CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
        virtual double ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData);
};

/** Temporal window evaluator for the categorical model. */
class CategoricalTemporalDataEvaluator : public CTimeIntervals {
    private:
        typedef double (AbstractLikelihoodCalculator::*MAXIMIZE_FUNCPTR) (const std::vector<count_t>&,size_t) const;
        MAXIMIZE_FUNCPTR gpCalculationMethod;
        double gdDefaultMaximizingValue;

    public:
        CategoricalTemporalDataEvaluator(const CSaTScanData& DataHub, AbstractLikelihoodCalculator& Calculator,
                                         IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType);

        virtual void CompareClusterSet(CCluster& Running, CClusterSet& ClusterSet);
        virtual void CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
        virtual double ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData);
};

/** Temporal window evaluator for the categorical model and seasona analysis. */
class ClosedLoopCategoricalTemporalDataEvaluator : public CTimeIntervals {
    private:
        typedef double (AbstractLikelihoodCalculator::*MAXIMIZE_FUNCPTR) (const std::vector<count_t>&,size_t) const;
        MAXIMIZE_FUNCPTR gpCalculationMethod;
        double gdDefaultMaximizingValue;
        int _extended_period_start;

    public:
        ClosedLoopCategoricalTemporalDataEvaluator(const CSaTScanData& DataHub, AbstractLikelihoodCalculator& Calculator,
                                                   IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType);

        virtual void CompareClusterSet(CCluster& Running, CClusterSet& ClusterSet);
        virtual void CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
        virtual double ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData);
};

/** Temporal window evaluator for the categorical model and multiple data sets. */
class MultiSetCategoricalTemporalDataEvaluator : public CTimeIntervals {
    public:
        MultiSetCategoricalTemporalDataEvaluator(const CSaTScanData& DataHub, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType);

        virtual void CompareClusterSet(CCluster& Running, CClusterSet& ClusterSet);
        virtual void CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
        virtual double ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData);
};

/** Temporal window evaluator for the categorical model, seasonal analysis and multiple data sets. */
class ClosedLoopMultiSetCategoricalTemporalDataEvaluator : public CTimeIntervals {
    private:
        int _extended_period_start;

    public:
        ClosedLoopMultiSetCategoricalTemporalDataEvaluator(const CSaTScanData& DataHub, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType);

        virtual void CompareClusterSet(CCluster& Running, CClusterSet& ClusterSet);
        virtual void CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
        virtual double ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData);
};

/** Temporal window evaluator for the batched model. */
class BatchedTemporalDataEvaluator : public CTimeIntervals {
private:
    typedef double (AbstractLikelihoodCalculator::* MAXIMIZE_FUNCPTR) (count_t, measure_t, measure_t, measure_t, const boost::dynamic_bitset<>&, size_t) const;
    MAXIMIZE_FUNCPTR gpCalculationMethod;

public:
    BatchedTemporalDataEvaluator(const CSaTScanData& Data, AbstractLikelihoodCalculator& Calculator,
        IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType);

    virtual void CompareClusterSet(CCluster& Running, CClusterSet& ClusterSet);
    virtual void CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
    virtual double ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData);
};

/** Temporal window evaluator for the batched model and multiple data sets. */
class MultiSetBatchedTemporalDataEvaluator : public CTimeIntervals {
public:
    MultiSetBatchedTemporalDataEvaluator(const CSaTScanData& DataHub, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType);

    virtual void CompareClusterSet(CCluster& Running, CClusterSet& ClusterSet);
    virtual void CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
    virtual double ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData);
};

/** Temporal window evaluator for the batched model, time stratified adjustment and multiple data sets. */
class MultiSetTimeStratifiedBatchedTemporalDataEvaluator : public CTimeIntervals {
private:
    typedef double (AbstractLikelihoodCalculator::* MAXIMIZE_FUNCPTR) (count_t, measure_t, measure_t, measure_t, const boost::dynamic_bitset<>&, size_t) const;
    MAXIMIZE_FUNCPTR gpCalculationMethod;
    double gdDefaultMaximizingValue;

public:
    MultiSetTimeStratifiedBatchedTemporalDataEvaluator(const CSaTScanData& Data, AbstractLikelihoodCalculator& Calculator,
        IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType);

    virtual void CompareClusterSet(CCluster& Running, CClusterSet& ClusterSet);
    virtual void CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
    virtual double ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData);
};

/** Temporal window evaluator for the batched model, time stratified adjustment and multiple data sets.
    This class is specialized for scanning windows when the time stratified adjustment length is larger
    than the time aggregation length. Technically, this class replaces MultiSetTimeStratifiedBatchedTemporalDataEvaluator,
    and should produce the same results, but that class is slightly faster. */
class MultiSetTimeStratifiedBatchedTemporalDataEvaluatorEnhanced : public CTimeIntervals {
private:
    typedef double (AbstractLikelihoodCalculator::* MAXIMIZE_FUNCPTR) (count_t, measure_t, measure_t, measure_t, const boost::dynamic_bitset<>&, size_t) const;
    MAXIMIZE_FUNCPTR gpCalculationMethod;
    double gdDefaultMaximizingValue;

public:
    MultiSetTimeStratifiedBatchedTemporalDataEvaluatorEnhanced(const CSaTScanData& Data, AbstractLikelihoodCalculator& Calculator,
        IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType);

    virtual void CompareClusterSet(CCluster& Running, CClusterSet& ClusterSet);
    virtual void CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
    virtual double ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData);
};

/** Temporal window evaluator for the batched model and seasonal analysis. */
class ClosedLoopBatchedTemporalDataEvaluator : public CTimeIntervals {
private:
    typedef double (AbstractLikelihoodCalculator::* MAXIMIZE_FUNCPTR) (count_t, measure_t, measure_t, measure_t, const boost::dynamic_bitset<>&, size_t) const;
    MAXIMIZE_FUNCPTR gpCalculationMethod;
    double gdDefaultMaximizingValue;
    int _extended_period_start;

public:
    ClosedLoopBatchedTemporalDataEvaluator(const CSaTScanData& Data, AbstractLikelihoodCalculator& Calculator,
        IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType);

    virtual void CompareClusterSet(CCluster& Running, CClusterSet& ClusterSet);
    virtual void CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
    virtual double ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData);
};

/** Temporal window evaluator for the batched model, seasonal analysis and multiple data sets. */
class ClosedLoopMultiSetBatchedTemporalDataEvaluator : public CTimeIntervals {
private:
    int _extended_period_start;

public:
    ClosedLoopMultiSetBatchedTemporalDataEvaluator(const CSaTScanData& DataHub, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType);

    virtual void CompareClusterSet(CCluster& Running, CClusterSet& ClusterSet);
    virtual void CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
    virtual double ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData);
};
//******************************************************************************
#endif

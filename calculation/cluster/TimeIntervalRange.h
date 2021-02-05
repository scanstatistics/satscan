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

/** Temporal window evaluator used with Bernoulli model and adjusting for temporal trends nonparametrically. */
class BernoulliTimeStratifiedTemporalDataEvaluator : public CTimeIntervals {
private:
    count_t   *  _pt_counts_nc;
    measure_t * _pt_measure_nc;
    /* feature which saves execution time by stopping weakened clusters */
    bool        _stop_weakened_clusters;

public:
    BernoulliTimeStratifiedTemporalDataEvaluator(const CSaTScanData& Data, AbstractLikelihoodCalculator& Calculator,
        IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType);

    virtual void CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
    virtual void CompareClusterSet(CCluster& Running, CClusterSet& ClusterSet);
    virtual double ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData);
};

/** Temporal window evaluator used with Bernoulli model and adjusting for temporal trends nonparametrically with multiple data sets. */
class MultiSetBernoulliTimeStratifiedTemporalDataEvaluator : public CTimeIntervals {
private:
    std::vector<count_t*>  _pt_counts_nc;
    std::vector<measure_t*> _pt_measure_nc;
public:
    MultiSetBernoulliTimeStratifiedTemporalDataEvaluator(const CSaTScanData& Data, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType);

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
//******************************************************************************
#endif

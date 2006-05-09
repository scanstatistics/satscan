//******************************************************************************
#ifndef __TIMEINTERVALRANGE_H
#define __TIMEINTERVALRANGE_H
//******************************************************************************
#include "SaTScan.h"
#include "JulianDates.h"
#include "TimeIntervals.h"
#include "DataSet.h"

class CSaTScanData;                /** forward class declaration */
class CCluster;                    /** forward class declaration */
class AbstractTemporalClusterData; /** forward class declaration */

/** Class which defines methods of iterating through temporal windows,
    evaluating the strength of a clustering.*/
class TemporalDataEvaluator : public CTimeIntervals {
  private:
    typedef double (AbstractLikelihoodCalculator::*MAXIMIZE_FUNCPTR) (count_t,measure_t,size_t) const;
    MAXIMIZE_FUNCPTR            gpCalculationMethod;
    double                      gdDefaultMaximizingValue;

  public:
    TemporalDataEvaluator(const CSaTScanData& Data, AbstractLikelihoodCalculator& Calculator,
                          IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType);
    virtual ~TemporalDataEvaluator();

    virtual void                CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
    virtual void                CompareClusters(CCluster& Running, CCluster& TopCluster);
    virtual double              ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData);
};

/** Class which defines methods of iterating through temporal windows,
    evaluating the strength of a clustering. Redefines method CompareClusters()
    to incorporate multiple data sets in the calculation of a log likelihood
    ratio. The alogrithm for using the TMeasureList object with multiple data
    sets is not defined so method CompareMeasures() throws an exception. */
class MultiSetTemporalDataEvaluator : public CTimeIntervals {
  public:
    MultiSetTemporalDataEvaluator(const CSaTScanData& Data, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType);
    virtual ~MultiSetTemporalDataEvaluator() {}

    virtual void                CompareClusters(CCluster& Running, CCluster& TopCluster);
    virtual void                CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
    virtual double              ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData);
};

/** Class which defines methods of iterating through temporal windows,
    evaluating the strength of a clustering. Redefines method CompareClusters()
    to incorporate a second measure variable in the calculation of a log
    likelihood ratio for categorical data. The alogrithm for using the
    TMeasureList object with this second variable is not defined so method
    CompareMeasures() throws an exception. */
class NormalTemporalDataEvaluator : public CTimeIntervals {
  private:
    typedef double (AbstractLikelihoodCalculator::*MAXIMIZE_FUNCPTR) (count_t,measure_t,measure_t,size_t) const;
    MAXIMIZE_FUNCPTR            gpCalculationMethod;
    double                      gdDefaultMaximizingValue;

  public:
    NormalTemporalDataEvaluator(const CSaTScanData& Data, AbstractLikelihoodCalculator& Calculator,
                                IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType);
    virtual ~NormalTemporalDataEvaluator() {}

    virtual void                CompareClusters(CCluster& Running, CCluster& TopCluster);
    virtual void                CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
    virtual double              ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData);
};

/** Class which defines methods of iterating through temporal windows,
    evaluating the strength of a clustering. Redefines method CompareClusters()
    to incorporate multiple data sets in the calculation of a log likelihood
    ratio for categorical data. The alogrithm for using the TMeasureList object
    with multiple data sets is not defined so method CompareMeasures() throws
    an exception. */
class MultiSetNormalTemporalDataEvaluator : public CTimeIntervals {
  public:
    MultiSetNormalTemporalDataEvaluator(const CSaTScanData& DataHub, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType);
    virtual ~MultiSetNormalTemporalDataEvaluator() {}

    virtual void                CompareClusters(CCluster& Running, CCluster& TopCluster);
    virtual void                CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
    virtual double              ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData);
};

/** Class which defines methods of iterating through temporal windows,
    evaluating the strength of a clustering. Redefines method CompareClusters()
    to calculate log likelihood ratio for categorical data. The alogrithm for
    using the TMeasureList object with multiple data sets is not defined so
    method CompareMeasures() throws an exception. */
class CategoricalTemporalDataEvaluator : public CTimeIntervals {
  private:
    typedef double (AbstractLikelihoodCalculator::*MAXIMIZE_FUNCPTR) (const std::vector<count_t>&,size_t) const;
    MAXIMIZE_FUNCPTR            gpCalculationMethod;
    double                      gdDefaultMaximizingValue;

  public:
    CategoricalTemporalDataEvaluator(const CSaTScanData& DataHub, AbstractLikelihoodCalculator& Calculator,
                                     IncludeClustersType eIncludeClustersType, ExecutionType eExecutionType);
    virtual ~CategoricalTemporalDataEvaluator() {}

    virtual void                CompareClusters(CCluster& Running, CCluster& TopCluster);
    virtual void                CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
    virtual double              ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData);
};

/** Class which defines methods of iterating through temporal windows,
    evaluating the strength of a clustering. Redefines method CompareClusters()
    to incorporate multiple data sets in the calculation of a log likelihood
    ratio for categorical data. The alogrithm for using the TMeasureList object
    with multiple data sets is not defined so method CompareMeasures() throws
    an exception. */
class MultiSetCategoricalTemporalDataEvaluator : public CTimeIntervals {
  public:
    MultiSetCategoricalTemporalDataEvaluator(const CSaTScanData& DataHub, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType);
    virtual ~MultiSetCategoricalTemporalDataEvaluator() {}

    virtual void                CompareClusters(CCluster& Running, CCluster& TopCluster);
    virtual void                CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
    virtual double              ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData);
};
//******************************************************************************
#endif


//******************************************************************************
#ifndef __TIMEINTERVALRANGE_H
#define __TIMEINTERVALRANGE_H
//******************************************************************************
#include "SaTScan.h"
#include "JulianDates.h"
#include "TimeIntervals.h"
#include "DataSet.h"
#include "MaxWindowLengthIndicator.h"

class CSaTScanData;                /** forward class declaration */
class CCluster;                    /** forward class declaration */
class AbstractTemporalClusterData; /** forward class declaration */

/** Class which defines methods of iterating through temporal windows,
    evaluating the strength of a clustering.*/
class TemporalDataEvaluator : public CTimeIntervals {
  private:
    void                               Init() {gpMaxWindowLengthIndicator=0;}
    void                               Setup(const CSaTScanData& Data, IncludeClustersType  eIncludeClustersType);

  protected:
    int				       giStartRange_Start;         /** start date index of start range */
    int				       giStartRange_End;           /** end date index of start range */
    int				       giEndRange_Start;           /** start date index of end range */
    int				       giEndRange_End;             /** end date index of end range */
    const CSaTScanData               & gData;                      /** data hub */
    AbstractLikelihoodCalculator     & gLikelihoodCalculator;      /** log likelihood calculator */
    AbstractMaxWindowLengthIndicator * gpMaxWindowLengthIndicator; /** indicates maximum temporal window length */

  public:
    TemporalDataEvaluator(const CSaTScanData& Data, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType);
    virtual ~TemporalDataEvaluator();

    virtual void                CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
    virtual void                CompareClusters(CCluster& Running, CCluster& TopCluster);
    virtual IncludeClustersType GetType() const {return CLUSTERSINRANGE;}
};

/** Class which defines methods of iterating through temporal windows,
    evaluating the strength of a clustering. Redefines method CompareClusters()
    to incorporate multiple data sets in the calculation of a log likelihood
    ratio. The alogrithm for using the TMeasureList object with multiple data
    sets is not defined so method CompareMeasures() throws an exception. */
class MultiSetTemporalDataEvaluator : public TemporalDataEvaluator {
  public:
    MultiSetTemporalDataEvaluator(const CSaTScanData& Data, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType);
    virtual ~MultiSetTemporalDataEvaluator() {}

    virtual void                           CompareClusters(CCluster& Running, CCluster& TopCluster);
    virtual void                           CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
};

/** Class which defines methods of iterating through temporal windows,
    evaluating the strength of a clustering. Redefines method CompareClusters()
    to incorporate a second measure variable in the calculation of a log
    likelihood ratio. The alogrithm for using the TMeasureList object with this
    second variable is not defined so method CompareMeasures() throws an exception. */
class NormalTemporalDataEvaluator : public TemporalDataEvaluator {
  public:
    NormalTemporalDataEvaluator(const CSaTScanData& Data, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType);
    virtual ~NormalTemporalDataEvaluator() {}

    virtual void                      CompareClusters(CCluster& Running, CCluster& TopCluster);
    virtual void                      CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
};

/** Class which defines methods of iterating through temporal windows,
    evaluating the strength of a clustering. Redefines method CompareClusters()
    to incorporate a second measure variable in the calculation of a log
    likelihood ratio. The alogrithm for using the CMeasureList object with this
    second variable is not defined so method CompareMeasures() throws an exception. */
class CategoricalTemporalDataEvaluator : public TemporalDataEvaluator {
  public:
    CategoricalTemporalDataEvaluator(const CSaTScanData& DataHub, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType);
    virtual ~CategoricalTemporalDataEvaluator() {}

    virtual void                      CompareClusters(CCluster& Running, CCluster& TopCluster);
    virtual void                      CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
};

/** Class which defines methods of iterating through temporal windows,
    evaluating the strength of a clustering. Redefines method CompareClusters()
    to incorporate multiple data sets in the calculation of a log likelihood
    ratio. The alogrithm for using the TMeasureList object with multiple data
    sets is not defined so method CompareMeasures() throws an exception. */
class MultiSetCategoricalTemporalDataEvaluator : public TemporalDataEvaluator {
  public:
    MultiSetCategoricalTemporalDataEvaluator(const CSaTScanData& DataHub, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType);
    virtual ~MultiSetCategoricalTemporalDataEvaluator() {}

    virtual void                           CompareClusters(CCluster& Running, CCluster& TopCluster);
    virtual void                           CompareMeasures(AbstractTemporalClusterData& ClusterData, CMeasureList& MeasureList);
};
//******************************************************************************
#endif


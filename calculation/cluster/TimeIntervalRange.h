//*****************************************************************************
#ifndef __TIMEINTERVALRANGE_H
#define __TIMEINTERVALRANGE_H
//*****************************************************************************
#include "SaTScan.h"
#include "JulianDates.h"
#include "TimeIntervals.h"
#include "DataStream.h"
#include "ClusterData.h"
#include "NormalClusterData.h"
#include "MultipleStreamClusterData.h"
#include "MaxWindowLengthIndicator.h"

class CSaTScanData;
class CCluster;

class TimeIntervalRange : public CTimeIntervals {
  private:
    void                        Init() {gpMaxWindowLengthIndicator=0;}
    void                        Setup(const CSaTScanData& Data, IncludeClustersType  eIncludeClustersType);
    void                        ValidateWindowRanges(const CSaTScanData& Data);

  protected:
    int				giStartRange_Start;
    int				giStartRange_End;
    int				giEndRange_Start;
    int				giEndRange_End;
    const CSaTScanData        & gData;
    AbstractLikelihoodCalculator & gLikelihoodCalculator;
    AbstractMaxWindowLengthIndicator * gpMaxWindowLengthIndicator;

  public:
    TimeIntervalRange(const CSaTScanData& Data, AbstractLikelihoodCalculator & Calculator, IncludeClustersType  eIncludeClustersType);
    TimeIntervalRange(const TimeIntervalRange & rhs);
    virtual ~TimeIntervalRange();

    TimeIntervalRange         & operator=(const TimeIntervalRange& rhs);
    virtual TimeIntervalRange * Clone() const;


    virtual IncludeClustersType GetType() const {return CLUSTERSINRANGE;}
    virtual count_t             GetCaseCountForTract(const CCluster & Cluster, tract_t tTract, count_t** pCases) const;
    virtual measure_t           GetMeasureForTract(const CCluster & Cluster, tract_t tTract, measure_t** pMeasure) const;

    virtual void                CompareMeasures(AbstractTemporalClusterData * pStreamData, CMeasureList * pMeasureList);
    virtual void                CompareClusters(CCluster & Running, CCluster & TopCluster);
};

class NormalTimeIntervalRange : public TimeIntervalRange {
  public:
    NormalTimeIntervalRange(const CSaTScanData& Data, AbstractLikelihoodCalculator & Calculator, IncludeClustersType  eIncludeClustersType);
    NormalTimeIntervalRange(const NormalTimeIntervalRange & rhs);
    virtual ~NormalTimeIntervalRange() {}

    NormalTimeIntervalRange         & operator=(const NormalTimeIntervalRange& rhs);
    virtual NormalTimeIntervalRange * Clone() const;

    virtual void                CompareClusters(CCluster & Running, CCluster & TopCluster);
    virtual void                CompareMeasures(AbstractTemporalClusterData * pStreamData, CMeasureList * pMeasureList);
};

class MultiStreamTimeIntervalRange : public TimeIntervalRange {
  public:
    MultiStreamTimeIntervalRange(const CSaTScanData& Data, AbstractLikelihoodCalculator & Calculator, IncludeClustersType  eIncludeClustersType);
    MultiStreamTimeIntervalRange(const MultiStreamTimeIntervalRange & rhs);
    virtual ~MultiStreamTimeIntervalRange() {}

    MultiStreamTimeIntervalRange         & operator=(const MultiStreamTimeIntervalRange& rhs);
    virtual MultiStreamTimeIntervalRange * Clone() const;

    virtual void                CompareClusters(CCluster & Running, CCluster & TopCluster);
    virtual void                CompareMeasures(AbstractTemporalClusterData * pStreamData, CMeasureList * pMeasureList);
};
//*****************************************************************************
#endif

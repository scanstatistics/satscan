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

class CSaTScanData; /** forward class declaration */
class CCluster;     /** forward class declaration */

/** Class which defines methods of iterating through temporal windows,
    evaluating the strength of a clustering.*/
class TimeIntervalRange : public CTimeIntervals {
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
    TimeIntervalRange(const CSaTScanData& Data, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType);
    TimeIntervalRange(const TimeIntervalRange& rhs);
    virtual ~TimeIntervalRange();

    TimeIntervalRange         & operator=(const TimeIntervalRange& rhs);
    virtual TimeIntervalRange * Clone() const;

    virtual void                CompareMeasures(TemporalData& StreamData, CMeasureList& MeasureList);
    virtual void                CompareClusters(CCluster& Running, CCluster& TopCluster);
    virtual IncludeClustersType GetType() const {return CLUSTERSINRANGE;}
};

/** Class which defines methods of iterating through temporal windows,
    evaluating the strength of a clustering. Redefines method CompareClusters()
    to incorporate a second measure variable in the calculation of a log
    likelihood ratio. The alogrithm for using the TMeasureList object with this
    second variable is not defined so method CompareMeasures() throws an exception. */
class NormalTimeIntervalRange : public TimeIntervalRange {
  public:
    NormalTimeIntervalRange(const CSaTScanData& Data, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType);
    NormalTimeIntervalRange(const NormalTimeIntervalRange& rhs);
    virtual ~NormalTimeIntervalRange() {}

    NormalTimeIntervalRange         & operator=(const NormalTimeIntervalRange& rhs);
    virtual NormalTimeIntervalRange * Clone() const;

    virtual void                      CompareClusters(CCluster& Running, CCluster& TopCluster);
    virtual void                      CompareMeasures(TemporalData& StreamData, CMeasureList& MeasureList);
};

/** Class which defines methods of iterating through temporal windows,
    evaluating the strength of a clustering. Redefines method CompareClusters()
    to incorporate multiple data sets in the calculation of a log likelihood
    ratio. The alogrithm for using the TMeasureList object with multiple data
    sets is not defined so method CompareMeasures() throws an exception. */
class MultiStreamTimeIntervalRange : public TimeIntervalRange {
  public:
    MultiStreamTimeIntervalRange(const CSaTScanData& Data, AbstractLikelihoodCalculator & Calculator, IncludeClustersType eIncludeClustersType);
    MultiStreamTimeIntervalRange(const MultiStreamTimeIntervalRange& rhs);
    virtual ~MultiStreamTimeIntervalRange() {}

    MultiStreamTimeIntervalRange         & operator=(const MultiStreamTimeIntervalRange& rhs);
    virtual MultiStreamTimeIntervalRange * Clone() const;

    virtual void                           CompareClusters(CCluster& Running, CCluster& TopCluster);
    virtual void                           CompareMeasures(TemporalData& StreamData, CMeasureList& MeasureList);
};
//*****************************************************************************
#endif

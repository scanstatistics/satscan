//*****************************************************************************
#ifndef __TIMEINTERVALRANGE_H
#define __TIMEINTERVALRANGE_H
//*****************************************************************************
#include "SaTScan.h"
#include "JulianDates.h"
#include "TimeIntervals.h"

class CSaTScanData;
class CCluster;

class TimeIntervalRange : public CTimeIntervals {
  private:
    void                        Setup(const CSaTScanData& Data);
    void                        ValidateWindowRanges(const CSaTScanData& Data);

  protected:
    int				giStartRange_Start;
    int				giStartRange_End;
    int				giEndRange_Start;
    int				giEndRange_End;
    const CSaTScanData        & gData;

  public:
    TimeIntervalRange(const CSaTScanData& Data);
    TimeIntervalRange(const TimeIntervalRange & rhs);
    ~TimeIntervalRange() {}

    TimeIntervalRange         & operator=(const TimeIntervalRange& rhs);
    virtual TimeIntervalRange * Clone() const;
    void                        CompareClusters(CCluster & Running,
                                                CCluster & TopShapeCluster,
                                                const CSaTScanData& Data,
                                                const count_t* pCases,
                                                const measure_t* pMeasure);
    void                        ComputeBestMeasures(const count_t* pCases,
                                                    const measure_t* pMeasure,
                                                    CMeasureList & MeasureList);
    virtual count_t             GetCaseCountForTract(const CCluster & Cluster, tract_t tTract, count_t** pCases) const;
    virtual measure_t           GetMeasureForTract(const CCluster & Cluster, tract_t tTract, measure_t** pMeasure) const;
    virtual IncludeClustersType GetType() const {return CLUSTERSINRANGE;}
};
//*****************************************************************************
#endif

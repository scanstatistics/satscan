//*****************************************************************************
#ifndef __TIMEINTERVALS_H
#define __TIMEINTERVALS_H
//*****************************************************************************
#include "SaTScan.h"
#include "Parameters.h"

class CMeasureList;
class CCluster;
class CSaTScanData;

class CTimeIntervals {
  protected:
    int                         giNumIntervals;         /* number of total time intervals */
    int                         giMaxWindowLength;      /* maximum window length          */

  public:
    CTimeIntervals(int nTotal, int nCut) {giNumIntervals=nTotal; giMaxWindowLength=nCut;}
    CTimeIntervals(const CTimeIntervals& rhs) {giNumIntervals = rhs.giNumIntervals;
                                               giMaxWindowLength = rhs.giMaxWindowLength;}
    virtual ~CTimeIntervals() {};

    virtual CTimeIntervals    * Clone() const = 0;
    virtual void                ComputeBestMeasures(const count_t* pCases,
                                                    const measure_t* pMeasure,
                                                    CMeasureList & MeasureList) = 0;
    virtual void                CompareClusters(CCluster & Running,
                                                CCluster & TopShapeCluster,
                                                const CSaTScanData& Data,
                                                const count_t* pCases,
                                                const measure_t* pMeasure) = 0;
    virtual void                Initialize() {/*stub - no action */}
    virtual count_t             GetCaseCountForTract(const CCluster & Cluster, tract_t tTract, count_t** pCases) const = 0;
    virtual measure_t           GetMeasureForTract(const CCluster & Cluster, tract_t tTract,  measure_t** pMeasure) const = 0;
    virtual IncludeClustersType GetType() const = 0;
};
#endif

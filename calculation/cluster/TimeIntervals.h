//*****************************************************************************
#ifndef __TIMEINTERVALS_H
#define __TIMEINTERVALS_H
//*****************************************************************************
#include "SaTScan.h"
#include "Parameters.h"
#include "Cluster.h"

class CMeasureList;
class CCluster;
class CSaTScanData;

class CTimeIntervals {
  public:
    typedef void (CTimeIntervals::* COMPARECLUSTERS)(CCluster & Running, CCluster & TopShapeCluster, StreamDataContainer_t & StreamData);

  protected:
    int                         giNumIntervals;         /* number of total time intervals */
    int                         giMaxWindowLength;      /* maximum window length          */
    RATE_FUNCPTRTYPE            fRateOfInterest;

  public:
    CTimeIntervals(int nTotal, int nCut, AreaRateType eType=HIGH) {
                         giNumIntervals=nTotal;
                         giMaxWindowLength=nCut;
                         switch(eType) {
                           case LOW        : fRateOfInterest = LowRate;       break;
                           case HIGHANDLOW : fRateOfInterest = HighOrLowRate; break;
                           default         : fRateOfInterest = HighRate;
                         };
                   }
    CTimeIntervals(const CTimeIntervals& rhs) {giNumIntervals = rhs.giNumIntervals;
                                               giMaxWindowLength = rhs.giMaxWindowLength;}
    virtual ~CTimeIntervals() {};

    virtual CTimeIntervals    * Clone() const = 0;

    virtual void                CompareClusters(CCluster & Running, CCluster & TopShapeCluster, StreamDataContainer_t & StreamData) = 0;
    virtual void                CompareClustersEx(CCluster & Running, CCluster & TopShapeCluster, StreamDataContainer_t & StreamData) = 0;
    virtual void                CompareDataStreamClusters(CCluster & Running, CCluster & TopShapeCluster, StreamDataContainer_t & StreamData) = 0;
    virtual void                CompareDataStreamClustersEx(CCluster & Running, CCluster & TopShapeCluster, StreamDataContainer_t & StreamData) = 0;
    virtual void                ComputeBestMeasures(AbstractTemporalClusterStreamData * pStreamData, CMeasureList & MeasureList) = 0;
    virtual void                Initialize() {/*stub - no action */}
    virtual count_t             GetCaseCountForTract(const CCluster & Cluster, tract_t tTract, count_t** pCases) const = 0;
    virtual measure_t           GetMeasureForTract(const CCluster & Cluster, tract_t tTract,  measure_t** pMeasure) const = 0;
    virtual IncludeClustersType GetType() const = 0;
};
#endif

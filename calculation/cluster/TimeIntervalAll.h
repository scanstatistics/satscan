//*****************************************************************************
#ifndef __TIMEINTERVALIALL_H
#define __TIMEINTERVALIALL_H
//*****************************************************************************
#include "TimeIntervals.h"

class CTIAll : public CTimeIntervals {
  public:
    CTIAll(int nTotal, int nCut);
    CTIAll(const CTIAll& rhs);
    virtual ~CTIAll() {};

    virtual CTIAll            * Clone() const;
    virtual count_t             GetCaseCountForTract(const CCluster & Cluster, tract_t tTract, count_t** pCases) const;
    virtual measure_t           GetMeasureForTract(const CCluster & Cluster, tract_t tTract, measure_t** pMeasure) const;
    virtual IncludeClustersType GetType() const {return ALLCLUSTERS;}
    virtual void                CompareClusters(CCluster & Running, CCluster & TopShapeCluster, StreamDataContainer_t & StreamData);
    virtual void                CompareClustersEx(CCluster & Running, CCluster & TopShapeCluster, StreamDataContainer_t & StreamData);
    virtual void                CompareDataStreamClusters(CCluster & Running, CCluster & TopShapeCluster, StreamDataContainer_t & StreamData);
    virtual void                CompareDataStreamClustersEx(CCluster & Running, CCluster & TopShapeCluster, StreamDataContainer_t & StreamData);
    virtual void                ComputeBestMeasures(AbstractTemporalClusterStreamData * pStreamData, CMeasureList & MeasureList);
};
//*****************************************************************************
#endif

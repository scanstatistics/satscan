//*****************************************************************************
#ifndef __TIMEINTERVALALIVE_H
#define __TIMEINTERVALALIVE_H
//*****************************************************************************
#include "TimeIntervals.h"

class CTIAlive : public CTimeIntervals {
  protected:
    const CSaTScanData        & gData;

  public:
    CTIAlive(const CSaTScanData& Data);
    CTIAlive(const CTIAlive& rhs);
    virtual ~CTIAlive() {};

    virtual CTIAlive          * Clone() const;
    virtual void                CompareClusters(CCluster & Running, CCluster & TopShapeCluster, const count_t* pCases,
                                                const measure_t* pMeasure, const measure_t* pMeasureSquared);
    virtual void                CompareDataStreamClusters(CCluster & Running, CCluster & TopShapeCluster, StreamDataContainer_t & StreamData);
    virtual void                ComputeBestMeasures(const count_t* pCases,
                                                    const measure_t* pMeasure,
                                                    const measure_t* pMeasureSquared,
                                                    CMeasureList & MeasureList);
    virtual count_t             GetCaseCountForTract(const CCluster & Cluster, tract_t tTract, count_t** pCases) const;
    virtual measure_t           GetMeasureForTract(const CCluster & Cluster, tract_t tTract,  measure_t** pMeasure) const;
    virtual IncludeClustersType GetType() const {return ALIVECLUSTERS;}
};

//*****************************************************************************
#endif

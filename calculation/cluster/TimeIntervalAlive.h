//*****************************************************************************
#ifndef __TIMEINTERVALALIVE_H
#define __TIMEINTERVALALIVE_H
//*****************************************************************************
#include "TimeIntervals.h"

class CTIAlive : public CTimeIntervals {
  public:
    CTIAlive(int nTotal, int nCut);
    CTIAlive(const CTIAlive& rhs);
    virtual ~CTIAlive() {};

    virtual CTIAlive          * Clone() const;
    virtual void                CompareClusters(CCluster & Running,
                                                CCluster & TopShapeCluster,
                                                const CSaTScanData& Data,
                                                const count_t* pCases,
                                                const measure_t* pMeasure);
    virtual void                ComputeBestMeasures(const count_t* pCases,
                                                    const measure_t* pMeasure,
                                                    CMeasureList & MeasureList);
    virtual count_t             GetCaseCountForTract(const CCluster & Cluster, tract_t tTract, count_t** pCases) const;
    virtual measure_t           GetMeasureForTract(const CCluster & Cluster, tract_t tTract,  measure_t** pMeasure) const;
    virtual IncludeClustersType GetType() const {return ALIVECLUSTERS;}
};

//*****************************************************************************
#endif

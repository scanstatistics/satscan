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
    virtual void                ComputeBestMeasures(const count_t* pCases, const measure_t* pMeasure,
                                                    CMeasureList & MeasureList) {/* should be pure virtual */}
    virtual void                CompareClusters(CCluster & Running,
                                                CCluster & TopShapeCluster,
                                                const CSaTScanData& Data,
                                                const count_t* pCases,
                                                const measure_t* pMeasure) {
                                   ZdGenerateException("CompareClusters() not implemented.","CTimeIntervals");
                                }

};
//*****************************************************************************
#endif

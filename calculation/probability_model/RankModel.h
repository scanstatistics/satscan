//---------------------------------------------------------------------------
#ifndef __RANKMODEL_H
#define __RANKMODEL_H
//---------------------------------------------------------------------------
#include "ProbabilityModel.h"

/** survival data model. */
class CRankModel : public CModel {
  public:
    CRankModel(CParameters & Parameters, CSaTScanData & Data, BasePrint & PrintDirection);
    virtual ~CRankModel();

    virtual double              CalcLogLikelihood(count_t n, measure_t u);
    virtual double              CalcLogLikelihoodRatio(count_t tCases, measure_t tMeasure, count_t tTotalCases, measure_t tTotalMeasure);
    virtual double              CalcLogLikelihoodRatioEx(count_t tCases, measure_t tMeasure, measure_t tMeasure2, count_t tTotalCases, measure_t tTotalMeasure);
    virtual double              CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster);
    virtual bool                CalculateMeasure(DataStream & thisStream) {/* no action here */ return true;}
    virtual double              CalcSVTTLogLikelihoodRatio(size_t tStream, CSVTTCluster* Cluster, CTimeTrend GlobalTimeTrend);
    virtual double              GetLogLikelihoodForTotal() const;
    virtual double              GetPopulation(int m_iEllipseOffset, tract_t nCenter, tract_t nTracts,
                                              int nStartInterval, int nStopInterval);
};
//---------------------------------------------------------------------------
#endif

//*****************************************************************************
#ifndef __POISSONMODEL_H
#define __POISSONMODEL_H
//*****************************************************************************
#include "SaTScan.h"
#include "ProbabilityModel.h"
#include "CalculateMeasure.h"
#include "RandomDistribution.h"
#include "PurelySpatialMonotoneCluster.h"
#include <iostream>
#include <fstream>
#include "SVTTCluster.h"

class CPoissonModel : public CModel {
  private:
    void                        AdjustForNonParameteric(DataStream & thisStream, measure_t ** pNonCumulativeMeasure);
    void                        AdjustForLLPercentage(DataStream & thisStream, measure_t ** pNonCumulativeMeasure, double nPercentage);
    void                        AdjustForLogLinear(DataStream & thisStream, measure_t ** pNonCumulativeMeasure);
    void                        AdjustMeasure(DataStream & thisStream, measure_t ** ppNonCumulativeMeasure);
    void                        AssignMeasure(DataStream & thisStream, measure_t ** ppNonCumulativeMeasure);
    virtual double              CalcSVTTLogLikelihood(count_t* pCases, measure_t* pMeasure,
                                                      count_t pTotalCases, double nAlpha,
                                                      double nBeta, int nStatus);

  public:
    CPoissonModel(CParameters& Parameters, CSaTScanData& Data, BasePrint& PrintDirection);
    virtual ~CPoissonModel();

    virtual double              CalcLogLikelihood(count_t n, measure_t u);
    virtual double              CalcLogLikelihoodRatio(count_t tCases, measure_t tMeasure, count_t tTotalCases, measure_t tTotalMeasure);
    virtual double              CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster);
    virtual double              CalcSVTTLogLikelihoodRatio(size_t tStream, CSVTTCluster* Cluster, CTimeTrend GlobalTimeTrend);
    virtual bool                CalculateMeasure(DataStream & thisStream);
    virtual double              GetLogLikelihoodForTotal() const;
    virtual double              GetPopulation(int m_iEllipseOffset, tract_t nCenter,
                                              tract_t nTracts, int nStartInterval, int nStopInterval);
};
//*****************************************************************************
#endif


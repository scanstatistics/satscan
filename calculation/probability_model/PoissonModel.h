//*****************************************************************************
#ifndef __POISSONMODEL_H
#define __POISSONMODEL_H
//*****************************************************************************
#include "ProbabilityModel.h"
#include "CalculateMeasure.h"
#include "RandomDistribution.h"
#include "PurelySpatialMonotoneCluster.h"
#include <iostream>
#include <fstream>
#include "SVTTCluster.h"

class CPoissonModel : public CModel {
  private:
    static const double         gTimeTrendConvergence; /* time trend convergence variable */

    void                        AdjustForNonParameteric(RealDataSet& DataSet, measure_t ** pNonCumulativeMeasure);
    void                        AdjustForLLPercentage(RealDataSet& DataSet, measure_t ** pNonCumulativeMeasure, double nPercentage);
    void                        AdjustForLogLinear(RealDataSet& DataSet, measure_t ** pNonCumulativeMeasure);
    void                        AdjustMeasure(RealDataSet& DataSet, measure_t ** ppNonCumulativeMeasure);
    void                        AssignMeasure(RealDataSet& DataSet, TwoDimMeasureArray_t& NonCumulativeMeasureHandler);
    void                        StratifiedSpatialAdjustment(RealDataSet& DataSet, measure_t ** ppNonCumulativeMeasure);

  public:
    CPoissonModel(const CParameters& Parameters, CSaTScanData& DataHub, BasePrint& PrintDirection);
    virtual ~CPoissonModel();

    virtual void                CalculateMeasure(RealDataSet& DataSet);
    virtual double              GetPopulation(size_t iSetIndex, int m_iEllipseOffset, tract_t nCenter,
                                              tract_t nTracts, int nStartInterval, int nStopInterval) const;
};
//*****************************************************************************
#endif

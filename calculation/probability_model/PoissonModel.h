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

  public:
    CPoissonModel(CParameters& Parameters, CSaTScanData& Data, BasePrint& PrintDirection);
    virtual ~CPoissonModel();

    virtual bool                CalculateMeasure(DataStream & thisStream);
    virtual double              GetPopulation(int m_iEllipseOffset, tract_t nCenter,
                                              tract_t nTracts, int nStartInterval, int nStopInterval);
};
//*****************************************************************************
#endif


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
    void                        AdjustForNonParameteric(RealDataStream & thisStream, measure_t ** pNonCumulativeMeasure);
    void                        AdjustForLLPercentage(RealDataStream & thisStream, measure_t ** pNonCumulativeMeasure, double nPercentage);
    void                        AdjustForLogLinear(RealDataStream & thisStream, measure_t ** pNonCumulativeMeasure);
    void                        AdjustMeasure(RealDataStream & thisStream, measure_t ** ppNonCumulativeMeasure);
    void                        AssignMeasure(RealDataStream & thisStream, TwoDimMeasureArray_t& NonCumulativeMeasureHandler);
    void                        StratifiedSpatialAdjustment(RealDataStream& thisStream, measure_t ** ppNonCumulativeMeasure);

  public:
    CPoissonModel(const CParameters& Parameters, CSaTScanData& Data, BasePrint& PrintDirection);
    virtual ~CPoissonModel();

    virtual void                CalculateMeasure(RealDataStream & thisStream);
    virtual double              GetPopulation(unsigned int iStream, int m_iEllipseOffset, tract_t nCenter,
                                              tract_t nTracts, int nStartInterval, int nStopInterval) const;
};
//*****************************************************************************
#endif

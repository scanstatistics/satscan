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
    const CParameters         & gParameters;
    CSaTScanData              & gDataHub; 

    void                        AdjustForNonParameteric(RealDataSet& DataSet);
    void                        AdjustForLLPercentage(RealDataSet& DataSet, double nPercentage);
    void                        AdjustForLogLinear(RealDataSet& DataSet);
    void                        AdjustMeasure(RealDataSet& Set, const TwoDimMeasureArray_t& PopMeasure);
    void                        StratifiedSpatialAdjustment(RealDataSet& DataSet);

  public:
    CPoissonModel(CSaTScanData& DataHub);
    virtual ~CPoissonModel();

    virtual void                CalculateMeasure(RealDataSet& DataSet);
    virtual double              GetPopulation(size_t tSetIndex, const CCluster& Cluster, const CSaTScanData& DataHub) const;
};
//*****************************************************************************
#endif

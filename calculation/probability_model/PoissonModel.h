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
    mutable std::vector<double> _alpha;

    void                        AdjustForNonParameteric(RealDataSet& DataSet);
    void                        AdjustForTrend(RealDataSet& DataSet, double beta, double beta2);
    void                        AdjustForCalculatedTrend(RealDataSet& DataSet);
    void                        AdjustMeasure(RealDataSet& Set, const TwoDimMeasureArray_t& PopMeasure);
    void                        StratifiedSpatialAdjustment(RealDataSet& DataSet);

  public:
    CPoissonModel(CSaTScanData& DataHub);
    virtual ~CPoissonModel();

    boost::shared_ptr<TwoDimMeasureArray_t> calculateMeasure(RealDataSet& Set, PopulationData * pAltPopulationData=0);
    virtual void                CalculateMeasure(RealDataSet& Set, const CSaTScanData& DataHub);
    virtual double              GetPopulation(size_t tSetIndex, const CCluster& Cluster, const CSaTScanData& DataHub) const;
    virtual double              GetLocationPopulation(size_t tSetIndex, tract_t tractIdx, const CCluster& Cluster, const CSaTScanData& DataHub) const;
};
//*****************************************************************************
#endif

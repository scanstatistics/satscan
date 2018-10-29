//*****************************************************************************
#ifndef __UNIFORMTIMEMODEL_H
#define __UNIFORMTIMEMODEL_H
//*****************************************************************************
#include "ProbabilityModel.h"
//#include "CalculateMeasure.h"
//#include "RandomDistribution.h"
#include <iostream>
#include <fstream>

class CSaTScanData;

class UniformTimeModel : public CModel {
  private:
    const CParameters         & gParameters;
    CSaTScanData              & gDataHub; 
    mutable std::vector<double> _alpha;

    void                                    StratifiedSpatialAdjustment(RealDataSet& DataSet);

  public:
    UniformTimeModel(CSaTScanData& DataHub);
    virtual ~UniformTimeModel();

    boost::shared_ptr<TwoDimMeasureArray_t> calculateMeasure(RealDataSet& Set, PopulationData * pAltPopulationData=0);
    virtual void                            CalculateMeasure(RealDataSet& Set, const CSaTScanData& DataHub);
    virtual double                          GetPopulation(size_t tSetIndex, const CCluster& Cluster, const CSaTScanData& DataHub) const;
    virtual double                          GetLocationPopulation(size_t tSetIndex, tract_t tractIdx, const CCluster& Cluster, const CSaTScanData& DataHub) const;
};
//*****************************************************************************
#endif

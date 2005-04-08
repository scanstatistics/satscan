//******************************************************************************
#ifndef __BERNOULLIMODEL_H
#define __BERNOULLIMODEL_H
//******************************************************************************
#include "ProbabilityModel.h"
#include "PurelySpatialMonotoneCluster.h"
#include "RandomDistribution.h"
#include "RandomNumberGenerator.h"

class CBernoulliModel : public CModel {
  public:
    CBernoulliModel();
    virtual ~CBernoulliModel();

    virtual void                CalculateMeasure(RealDataSet& DataSet);
    virtual double              GetPopulation(size_t tSetIndex, const CCluster& Cluster, const CSaTScanData& DataHub) const;
};

//******************************************************************************
#endif

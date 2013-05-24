//******************************************************************************
#ifndef __HOMOGENEOUSPOISSONMODEL_H
#define __HOMOGENEOUSPOISSONMODEL_H
//******************************************************************************
#include "ProbabilityModel.h"

/** Homogeneous Poisson data model. */
class HomogenousPoissonModel : public CModel {
  public:
    HomogenousPoissonModel();
    virtual ~HomogenousPoissonModel();

    virtual void                CalculateMeasure(RealDataSet& DataSet, const CSaTScanData& DataHub);
    virtual double              GetPopulation(size_t tSetIndex, const CCluster& Cluster, const CSaTScanData& DataHub) const;
};
//******************************************************************************
#endif


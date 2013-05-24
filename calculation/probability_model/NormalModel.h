//******************************************************************************
#ifndef __NormalModel_H
#define __NormalModel_H
//******************************************************************************
#include "ProbabilityModel.h"

/** normal data model. */
class CNormalModel : public CModel {
  public:
    CNormalModel();
    virtual ~CNormalModel();

    virtual void                CalculateMeasure(RealDataSet& DataSet, const CSaTScanData& DataHub);
    virtual double              GetPopulation(size_t tSetIndex, const CCluster& Cluster, const CSaTScanData& DataHub) const;
};
//******************************************************************************
#endif


//******************************************************************************
#ifndef __ExponentialModel_H
#define __ExponentialModel_H
//******************************************************************************
#include "ProbabilityModel.h"

/** ExponentialModel data model. */
class ExponentialModel : public CModel {
  public:
    ExponentialModel();
    virtual ~ExponentialModel();

    virtual void                CalculateMeasure(RealDataSet& DataSet, const CSaTScanData& DataHub);
    virtual double              GetPopulation(size_t tSetIndex, const CCluster& Cluster, const CSaTScanData& DataHub) const;
};
//******************************************************************************
#endif

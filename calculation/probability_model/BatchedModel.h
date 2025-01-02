//******************************************************************************
#ifndef __BatchedModel_H
#define __BatchedModel_H
//******************************************************************************
#include "ProbabilityModel.h"

/** BatchedModel data model. */
class BatchedModel : public CModel {
  public:
    BatchedModel() {}
    virtual ~BatchedModel() {}

    virtual void CalculateMeasure(RealDataSet& DataSet, const CSaTScanData& DataHub) { /* nop */}
    virtual double GetPopulation(size_t tSetIndex, const CCluster& Cluster, const CSaTScanData& DataHub) const;
};
//******************************************************************************
#endif

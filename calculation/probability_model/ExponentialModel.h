//******************************************************************************
#ifndef __ExponentialModel_H
#define __ExponentialModel_H
//******************************************************************************
#include "ProbabilityModel.h"

/** ExponentialModel data model. */
class ExponentialModel : public CModel {
  public:
    ExponentialModel(const CParameters& Parameters, CSaTScanData& DataHub, BasePrint& PrintDirection);
    virtual ~ExponentialModel();

    virtual void                CalculateMeasure(RealDataSet& DataSet);
    virtual double              GetPopulation(size_t tSetIndex, const CCluster& Cluster) const;
};
//******************************************************************************
#endif

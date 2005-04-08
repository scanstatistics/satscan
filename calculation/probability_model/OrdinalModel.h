//******************************************************************************
#ifndef __OrdinalModel_H
#define __OrdinalModel_H
//******************************************************************************
#include "ProbabilityModel.h"

/** Probability model for ordinal data. Re-defines routines to calculate measure
    and population in defined cluster object. */
class OrdinalModel : public CModel {
  public:
    OrdinalModel();
    virtual ~OrdinalModel();

    virtual void                CalculateMeasure(RealDataSet& DataSet);
    virtual double              GetPopulation(size_t tSetIndex, const CCluster& Cluster, const CSaTScanData& DataHub) const;
};
//******************************************************************************
#endif


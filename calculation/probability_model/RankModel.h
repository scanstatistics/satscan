//******************************************************************************
#ifndef __RANKMODEL_H
#define __RANKMODEL_H
//******************************************************************************
#include "ProbabilityModel.h"

/** Rank data model. */
class CRankModel : public CModel {
  public:
    CRankModel(const CParameters& Parameters, CSaTScanData& DataHub, BasePrint& PrintDirection);
    virtual ~CRankModel();

    virtual void                CalculateMeasure(RealDataSet& DataSet);
    virtual double              GetPopulation(size_t tSetIndex, const CCluster& Cluster) const;
};
//******************************************************************************
#endif


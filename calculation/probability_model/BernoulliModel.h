// CBernoulliModel.h

#include "model.h"

#ifndef __BERNOULLIMODEL_H
#define __BERNOULLIMODEL_H

class CBernoulliModel : public CModel
{
  public:
    CBernoulliModel(CParameters* pParameters, CSaTScanData* pData);
    virtual ~CBernoulliModel();

    virtual bool   ReadData();
    virtual bool   CalculateMeasure();
    virtual double GetLogLikelihoodForTotal() const;
    virtual double CalcLogLikelihood(count_t n, measure_t u);
    virtual double CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster);
    virtual void   MakeData();
    virtual count_t*   MakeDataB(count_t nTotalCounts, count_t* RandCounts);
    virtual double GetPopulation(tract_t nCenter, tract_t nTracts,
                                 int nStartInterval, int nStopInterval);
};

#endif


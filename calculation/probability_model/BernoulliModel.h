// CBernoulliModel.h

#ifndef __BERNOULLIMODEL_H
#define __BERNOULLIMODEL_H

#include "model.h"
#include "ssexception.h"

class CBernoulliModel : public CModel
{
  public:
    CBernoulliModel(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~CBernoulliModel();

    virtual bool   ReadData();
    virtual bool   CalculateMeasure();
    virtual double GetLogLikelihoodForTotal() const;
    virtual double CalcLogLikelihood(count_t n, measure_t u);
    virtual double CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster);
    virtual void   MakeData();
    virtual count_t*   MakeDataB(count_t nTotalCounts, count_t* RandCounts);
    virtual double GetPopulation(int m_iEllipseOffset, tract_t nCenter, tract_t nTracts,
                                 int nStartInterval, int nStopInterval);
};

#endif


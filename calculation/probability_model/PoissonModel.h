// CPoissonModel.h

#include "model.h"
#include "spatscan.h"

#ifndef __POISSONMODEL_H
#define __POISSONMODEL_H

class CPoissonModel : public CModel
{
  public:
    CPoissonModel(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~CPoissonModel();

    virtual bool   ReadData();
    virtual bool   CalculateMeasure();
    virtual double GetLogLikelihoodForTotal() const;
    virtual double CalcLogLikelihood(count_t n, measure_t u);
    virtual double CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster);
    virtual void   MakeData();
    virtual double GetPopulation(int m_iEllipseOffset, tract_t nCenter, tract_t nTracts,
                                 int nStartInterval, int nStopInterval);

};

#endif


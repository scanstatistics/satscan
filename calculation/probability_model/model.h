// CModel.h

#ifndef __MODEL_H
#define __MODEL_H

#include "param.h"
#include "data.h"
#include "PSMCluster.h"

#define DEBUGMODEL 0

class CModel
{
  public:
    CModel(CParameters* pParameters, CSaTScanData* pData);
    virtual ~CModel();

    CParameters*  m_pParameters;
    CSaTScanData* m_pData;

    FILE*         m_pDebugModelFile;

    virtual bool   ReadData() = 0;
    virtual bool   CalculateMeasure() = 0;
    virtual double GetLogLikelihoodForTotal() const = 0;
    virtual double CalcLogLikelihood(count_t n, measure_t u) = 0;
    virtual double CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster) = 0;
    virtual void   MakeData() = 0;
    virtual double GetPopulation(tract_t nCenter, tract_t nTracts,
                                 int nStartInterval, int nStopInterval) = 0;
};

#endif


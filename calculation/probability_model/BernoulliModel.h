//*****************************************************************************
#ifndef __BERNOULLIMODEL_H
#define __BERNOULLIMODEL_H
//*****************************************************************************
#include "ProbabilityModel.h"
#include "PurelySpatialMonotoneCluster.h"
#include "RandomDistribution.h"
#include "RandomNumberGenerator.h"

class CBernoulliModel : public CModel
{
  private:
    RandomNumberGenerator       m_RandomNumberGenerator;

  public:
    CBernoulliModel(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~CBernoulliModel();

    virtual double              CalcLogLikelihood(count_t n, measure_t u);
    virtual double              CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster);
    virtual bool                CalculateMeasure();
    virtual double              GetLogLikelihoodForTotal() const;
    virtual double              GetPopulation(int m_iEllipseOffset, tract_t nCenter,
                                              tract_t nTracts, int nStartInterval, int nStopInterval);
    virtual void                MakeData();
    virtual count_t           * MakeDataB(count_t nTotalCounts, count_t* RandCounts);
    virtual bool                ReadData();
};

//*****************************************************************************
#endif


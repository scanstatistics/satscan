//*****************************************************************************
#ifndef __BERNOULLIMODEL_H
#define __BERNOULLIMODEL_H
//*****************************************************************************
#include "ProbabilityModel.h"
#include "PurelySpatialMonotoneCluster.h"
#include "RandomDistribution.h"
#include "RandomNumberGenerator.h"

class CBernoulliModel : public CModel {
  public:
    CBernoulliModel(CParameters& Parameters, CSaTScanData& Data, BasePrint& PrintDirection);
    virtual ~CBernoulliModel();

    virtual bool                CalculateMeasure(DataStream & thisStream);
    virtual double              GetPopulation(int m_iEllipseOffset, tract_t nCenter,
                                              tract_t nTracts, int nStartInterval, int nStopInterval);
};

//*****************************************************************************
#endif

//******************************************************************************
#ifndef __NormalModel_H
#define __NormalModel_H
//******************************************************************************
#include "ProbabilityModel.h"

/** normal data model. */
class CNormalModel : public CModel {
  public:
    CNormalModel(const CParameters& Parameters, CSaTScanData& DataHub, BasePrint& PrintDirection);
    virtual ~CNormalModel();

    virtual void                CalculateMeasure(RealDataSet& DataSet);
    virtual double              GetPopulation(size_t tSetIndex, int m_iEllipseOffset, tract_t nCenter,
                                              tract_t nTracts, int nStartInterval, int nStopInterval) const;
};
//******************************************************************************
#endif


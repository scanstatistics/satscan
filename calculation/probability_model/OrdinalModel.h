//******************************************************************************
#ifndef __OrdinalModel_H
#define __OrdinalModel_H
//******************************************************************************
#include "ProbabilityModel.h"

class OrdinalModel : public CModel {
  public:
    OrdinalModel(const CParameters& Parameters, CSaTScanData& DataHub, BasePrint& PrintDirection);
    virtual ~OrdinalModel();

    virtual void                CalculateMeasure(RealDataSet& DataSet);
    virtual double              GetPopulation(size_t tSetIndex, int m_iEllipseOffset, tract_t nCenter,
                                              tract_t nTracts, int nStartInterval, int nStopInterval) const;
};
//******************************************************************************
#endif


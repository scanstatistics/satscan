//---------------------------------------------------------------------------
#ifndef __NormalModel_H
#define __NormalModel_H
//---------------------------------------------------------------------------
#include "ProbabilityModel.h"

/** normal data model. */
class CNormalModel : public CModel {
  public:
    CNormalModel(CParameters & Parameters, CSaTScanData & Data, BasePrint & PrintDirection);
    virtual ~CNormalModel();

    virtual bool                CalculateMeasure(DataStream & thisStream) {/* no action here */ return true;}
    virtual double              GetPopulation(int m_iEllipseOffset, tract_t nCenter, tract_t nTracts,
                                              int nStartInterval, int nStopInterval);
};

#endif

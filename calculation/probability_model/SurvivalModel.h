//---------------------------------------------------------------------------
#ifndef __SurvivalModel_H
#define __SurvivalModel_H
//---------------------------------------------------------------------------
#include "ProbabilityModel.h"

/** survival data model. */
class CSurvivalModel : public CModel {
  public:
    CSurvivalModel(CParameters & Parameters, CSaTScanData & Data, BasePrint & PrintDirection);
    virtual ~CSurvivalModel();

    virtual bool                CalculateMeasure(DataStream & thisStream) {/* no action here */ return true;}
    virtual double              GetPopulation(int m_iEllipseOffset, tract_t nCenter, tract_t nTracts,
                                              int nStartInterval, int nStopInterval);
};
//---------------------------------------------------------------------------
#endif

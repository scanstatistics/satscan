//---------------------------------------------------------------------------
#ifndef __SurvivalModel_H
#define __SurvivalModel_H
//---------------------------------------------------------------------------
#include "ProbabilityModel.h"

/** survival data model. */
class CSurvivalModel : public CModel {
  public:
    CSurvivalModel(const CParameters & Parameters, CSaTScanData & Data, BasePrint & PrintDirection);
    virtual ~CSurvivalModel();

    virtual void                CalculateMeasure(RealDataStream & thisStream) {/* no action here */}
    virtual double              GetPopulation(int m_iEllipseOffset, tract_t nCenter, tract_t nTracts,
                                              int nStartInterval, int nStopInterval);
};
//---------------------------------------------------------------------------
#endif

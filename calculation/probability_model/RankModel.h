//---------------------------------------------------------------------------
#ifndef __RANKMODEL_H
#define __RANKMODEL_H
//---------------------------------------------------------------------------
#include "ProbabilityModel.h"

/** survival data model. */
class CRankModel : public CModel {
  public:
    CRankModel(CParameters & Parameters, CSaTScanData & Data, BasePrint & PrintDirection);
    virtual ~CRankModel();

    virtual bool                CalculateMeasure(DataStream & thisStream) {/* no action here */ return true;}
    virtual double              GetPopulation(int m_iEllipseOffset, tract_t nCenter, tract_t nTracts,
                                              int nStartInterval, int nStopInterval);
};
//---------------------------------------------------------------------------
#endif

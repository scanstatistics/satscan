//---------------------------------------------------------------------------
#ifndef __RANKMODEL_H
#define __RANKMODEL_H
//---------------------------------------------------------------------------
#include "ProbabilityModel.h"

/** survival data model. */
class CRankModel : public CModel {
  public:
    CRankModel(const CParameters & Parameters, CSaTScanData & Data, BasePrint & PrintDirection);
    virtual ~CRankModel();

    virtual void                CalculateMeasure(RealDataStream & thisStream) {/* no action here */}
    virtual double              GetPopulation(unsigned int iStream, int m_iEllipseOffset, tract_t nCenter,
                                              tract_t nTracts, int nStartInterval, int nStopInterval) const;
};
//---------------------------------------------------------------------------
#endif

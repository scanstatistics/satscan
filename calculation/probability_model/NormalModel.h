//---------------------------------------------------------------------------
#ifndef __NormalModel_H
#define __NormalModel_H
//---------------------------------------------------------------------------
#include "ProbabilityModel.h"

/** normal data model. */
class CNormalModel : public CModel {
  public:
    CNormalModel(const CParameters & Parameters, CSaTScanData & Data, BasePrint & PrintDirection);
    virtual ~CNormalModel();

    virtual void                CalculateMeasure(RealDataStream & thisStream) {/* no action here */}
    virtual double              GetPopulation(unsigned int iStream, int m_iEllipseOffset, tract_t nCenter,
                                              tract_t nTracts, int nStartInterval, int nStopInterval) const;
};

#endif

//*****************************************************************************
#ifndef __OrdinalModel_H
#define __OrdinalModel_H
//*****************************************************************************
#include "ProbabilityModel.h"

class OrdinalModel : public CModel {
  public:
    OrdinalModel(const CParameters & Parameters, CSaTScanData & Data, BasePrint & PrintDirection);
    virtual ~OrdinalModel();

    virtual void                CalculateMeasure(RealDataStream & thisStream) {/* no action here */}
    virtual double              GetPopulation(unsigned int iStream, int m_iEllipseOffset, tract_t nCenter,
                                              tract_t nTracts, int nStartInterval, int nStopInterval) const;
};
//*****************************************************************************
#endif
